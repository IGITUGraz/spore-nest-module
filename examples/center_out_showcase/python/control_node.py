#! /usr/bin/env python
# -*- coding: utf-8 -*-

from config import *

import music
import numpy as np

import experiment_utils.reward as reward

import time
import collections
import logging
from snn_utils.comm.music.node import PyMusicNode
from snn_utils.comm.music import WindowedBuffer

logger = logging.getLogger(__name__)

# state definitions
state_wait = 0
state_move = 1
state_goal = 2
state_reward = 3

state_names = ["wait", "move", "goal", "reward"]

class ControlNode(PyMusicNode):
    def __init__(self):
        PyMusicNode.__init__(self, music_node_timestep, total_time=total_simulation_time, pre_run_barrier=True)
        self._set_buffer(WindowedBuffer(spike_rate_time_window))

    def _setup(self, music_setup):
        logger.info("Opening port for incoming spikes...")

        self._spike_buffers = self.publish_buffering_event_input(width_to_n_buffers=lambda size: n_control_neurons,
                                                                 idx_to_buffer=lambda idx: idx,
                                                                 **music_control_node_activity_in_port)

        logger.info("Opening port for outgoing reward signal...")

        self._event_pattern_out = self.publish_event_output(**music_control_node_pattern_out_port)

        self._pattern_id_out = self.publish_cont_output("pattern_id_out", **music_control_node_reward_out_ports)
        self._curr_reward_out = self.publish_cont_output("curr_reward_out", **music_control_node_reward_out_ports)
        self._mean_reward_out = self.publish_cont_output("mean_reward_out", **music_control_node_reward_out_ports)
        self._norm_reward_out = self.publish_cont_output("norm_reward_out", **music_control_node_reward_out_ports)
        self._cursor_pos_out = self.publish_cont_output(**music_control_node_activity_rates_out_ports)

        self._background_rate_pattern = np.ones(n_input_neurons) * background_rate
        self._input_rate_patterns = np.array(
            [self._background_rate_pattern] + [ControlNode.gen_rate_pattern() for i in range(n_patterns)])

    @staticmethod
    def gen_rate_pattern(shape=.2, scale=.8, min_val=background_rate, max_val=max_rate):
        return np.clip(max_val * np.random.gamma(shape, scale, n_input_neurons), min_val, max_val)

    @staticmethod
    def gen_duration(min_length=min_phase_duration, max_length=max_phase_duration):
        return min_length + np.random.rand() * (max_length - min_length)

    @staticmethod
    def gen_pattern_plan(pattern_duration, background_duration):
        """
        :param pattern_duration: how long to present the pattern [s]
        :param background_duration: how long to present pure background noise afterwards [s]
        :return: [(time, pattern_id), ...], background (id == 0), pattern #i (id == #i + 1)
        """
        plan = [(0.0, 0)]
        while plan[-1][0] < total_simulation_time:
            plan += [(plan[-1][0] + pattern_duration(), np.random.randint(n_patterns) + 1)]
            plan += [(plan[-1][0] + background_duration(), 0)]
        return plan

    @staticmethod
    def spikes_from_rate(rates):
        # rate [1/s] * timestep [s] (length of interval)
        return np.random.poisson(rates * music_node_timestep)


    def _enter_state(self, new_state, cur_time):

        self._current_state = new_state

        #logger.info( "entering state: " + state_names[self._current_state] )

        if new_state == state_wait:
            self._state_end_time = cur_time + self.gen_duration(min_wait_duration, max_wait_duration)
            self._current_pattern_id = 0
        elif new_state == state_move:
            self._state_end_time = cur_time + max_trial_duration
            self._current_pattern_id = 1
        elif new_state == state_goal:
            self._state_end_time = cur_time + hold_goal_duration
            self._current_pattern_id = 1
        elif new_state == state_reward:
            self._state_end_time = cur_time + reward_duration
            self._current_pattern_id = 0


    def _pre_run(self, music_setup):

        self._current_pattern_id = 0
        self._current_state = 0
        self._current_cursor_pos = np.array([0.0, 0.0])
        self._target_pos = np.array([20.0, 20.0])
        self._print_delay = 0

        self._last_real_time_stamp = time.clock()
        self._last_sim_time = 0
        self._last_real_time_stamps = collections.deque(maxlen=10000)
        self._last_sim_speedup = 0.0

        self._reward_generator = reward.RewardGenerator(reward_integration_time)

        map = np.random.randn(2,n_control_neurons)
        map = map - np.tile( map.mean(axis=1), [n_control_neurons,1] ).T
        map = map / np.tile( map.std(axis=1), [n_control_neurons,1] ).T

        self._output_map = map

        self._enter_state(state_wait, 0.0)


    def _run_single_cycle(self, curr_time):

        # update lever position
        for buf in self._spike_buffers:
            buf.update(curr_time)

        activity_rates_out = np.array([b.rate() for b in self._spike_buffers])
        self._current_cursor_pos = self._current_cursor_pos*0.95 + 0.05*np.inner( self._output_map, activity_rates_out )

        # update states
        if curr_time >= self._state_end_time:
            next_state_on_timeout = [state_move, state_wait, state_reward, state_wait]
            self._enter_state(next_state_on_timeout[self._current_state], curr_time)
        elif self._current_state == state_wait:
            self._current_cursor_pos = np.array([0.0, 0.0])
        elif self._current_state == state_move:
            if np.linalg.norm( self._current_cursor_pos - self._target_pos ) < threshold_1:
                self._enter_state(state_goal, curr_time)
        elif self._current_state == state_goal:
            if np.linalg.norm( self._current_cursor_pos - self._target_pos ) > threshold_2:
                self._enter_state(state_wait, curr_time)

        self._cursor_pos_out[:] = self._current_cursor_pos

        self._pattern_id_out[0] = self._current_pattern_id

        # send input spikes
        spikes = ControlNode.spikes_from_rate(self._input_rate_patterns[self._current_pattern_id])
        for neuron_idx, n_spikes in enumerate(spikes):
            for t in sorted(
                    [curr_time + np.random.rand() * music_node_timestep + network_node_time_step for _ in
                     range(n_spikes)]):
                self._event_pattern_out.insertEvent(t, neuron_idx, music.Index.GLOBAL)

        # update rewards

        #logger.info("current lever pos: " + str(self._current_cursor_pos))
        self._reward_generator.compute_reward(self._current_state == state_reward)
        self._curr_reward_out[0] = self._reward_generator.get_curr_reward()
        self._mean_reward_out[0] = self._reward_generator.get_mean_reward()
        self._norm_reward_out[0] = self._reward_generator.get_norm_reward()        


    def _post_cycle(self, curr_time, measured_cycle_time):
        # assess simulation performance
        curr_real_time_stamp = time.clock()
        self._last_real_time_stamps.append(curr_real_time_stamp)
        if len(self._last_real_time_stamps) > 1:
            real_time_span = self._last_real_time_stamps[-1] - self._last_real_time_stamps[0]
            sim_time_span = (len(self._last_real_time_stamps) - 1) * music_node_timestep
            sim_speedup = sim_time_span / real_time_span
            sim_speedup = (sim_speedup - self._last_sim_speedup) * 0.01 + self._last_sim_speedup
            self._last_sim_speedup = sim_speedup
        else:
            sim_speedup = np.NaN

        # log process information
        if self._print_delay == 0:
            self._print_delay = compute_node_print_interval
            print("t = {:.4f}: pattern = {: 2d}, reward = {:.4f} (~{:.4f}), speedup = {:.4f}, cursor pos = {}"
                  .format(curr_time, self._current_pattern_id, self._norm_reward_out[0], self._mean_reward_out[0],
                          sim_speedup, self._cursor_pos_out))
        else:
            self._print_delay -= 1


if __name__ == '__main__':
    ControlNode().main()
