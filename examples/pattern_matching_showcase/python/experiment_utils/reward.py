from config import *

import numpy as np


def sigmoid(x):
    return 1 / (1 + np.exp(-x))


class RewardGenerator(object):
    def __init__(self, normalization_time_window=0.0):
        assert(normalization_time_window >= 0.0)
        self._normalization_window = normalization_time_window

        if self._normalization_window > 0.0:
            self._gamma = np.exp(-1e-3 / self._normalization_window)
            self._mean_reward = 0.0
        else:
            self._gamma = np.NaN
            self._mean_reward = np.NaN

        self._curr_reward = 0.0
        self._norm_reward = np.NaN

    def compute_reward(self, target_population, population_rates):
        # target_population == 0 -> no target, reward = 0
        # target_population >  0  -> target desired, reward > 0
        reward = 0.0
        if target_population > 0:
            max_spike_rate = max(population_rates)
            activity_pattern = population_rates.index(max_spike_rate)

            if target_population == activity_pattern + 1:
                carry = max_spike_rate - sorted(population_rates)[-2]
                if carry > 0:
                    reward = sigmoid(reward_target_scale * (carry - reward_soft_threshold))

        self._curr_reward = reward

        if self._normalization_window > 0.0:
            reward = self._normalize_reward(reward)

        return reward

    def get_mean_reward(self):
        return self._mean_reward

    def get_curr_reward(self):
        return self._curr_reward

    def get_norm_reward(self):
        return self._norm_reward

    def _normalize_reward(self, reward):
        self._mean_reward = max(self._gamma * self._mean_reward + (1 - self._gamma) * reward, reward_offset)
        self._norm_reward = reward / self._mean_reward + reward_offset if reward > 0.0 else reward_offset
        return self._norm_reward

