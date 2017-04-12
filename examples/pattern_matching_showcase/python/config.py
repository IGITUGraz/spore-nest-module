#
# This file is part of SPORE.
#
# Copyright (C) 2016 Institute for Theoretical Computer Science, Graz University of Technology
#
# SPORE is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# SPORE is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with SPORE.  If not, see <http://www.gnu.org/licenses/>.
#
# For more information see: https://github.com/IGITUGraz/spore-nest-module
#

# ##########################################################
#           GLOBAL CONFIGURATION FILE
# ##########################################################

import os
import math
import logging
import time
import sys


def s_to_ms(seconds):
    return seconds * 1000.0

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s | %(module)s | %(levelname)-8s | %(message)s',
                    datefmt='%s')

# ############################################## NEST TOPOLOGY #########################################################

n_patterns = 2
n_input_neurons = 50
n_hidden_neurons = n_patterns * 10
n_reward_channels = 1

# ############################################## MUSIC #################################################################

music_node_timestep = 0.015
standard_accepted_latency = 0
standard_delay = 0

music_control_node_activity_in_port = {
    'port_name': 'activity_in',
    'maxBuffered': 1,
    'base': 0,
    'accLatency': music_node_timestep + 0.001,
    'fallback_width': n_hidden_neurons,
}

music_control_node_reward_out_ports = {
    'maxBuffered': 1,
    'fallback_width': n_reward_channels,
}

music_control_node_activity_rates_out_ports = {
    'port_name': 'activity_rates_out',
    'maxBuffered': 1,
    'fallback_width': n_patterns,
}

music_control_node_pattern_out_port = {
    'port_name': 'pattern_out',
    'maxBuffered': 1,
    'base': 0,
}

music_network_node_pattern_in_port = {
    'port_name': 'pattern_in',
    'maxBuffered': 1,
    'accLatency': 0.0,
}

music_network_node_activity_out_port = {
    'port_name': 'activity_out',
}

music_network_node_reward_in_port = {
    'port_name': 'reward_in',
    'delay': 0.0,
}

music_zmq_proxy_config = {
    'communication': {
        'format': 'ujson',
        'port': 8001,
        'host': '127.0.0.1',
    },
    'cont_port_defaults': {
        'maxBuffered': 1,
        'delay': standard_delay,
        'interpolate': False,
    },
    'event_port_defaults': {
        'maxBuffered': 1,
        'accLatency': standard_accepted_latency,
        'base': 0,
    },
    'cont': {
        'pattern_id_in': {
            'keys': ['pattern_id'],
        },
        'activity_rates_in': {
            'keys': ['activity_rate_{}'.format(i) for i in range(n_patterns)],
        },
        'curr_reward_in': {
            'keys': ['curr'],
        },
        'norm_reward_in': {
            'keys': ['norm'],
        },
        'mean_reward_in': {
            'keys': ['mean'],
        }
    },
    'event': {
        'pattern_in': {
            'keys': n_input_neurons,
        },
        'activity_in': {
            'keys': n_hidden_neurons,
        }
    }
}

# ############################################## EXPERIMENT PARAMETERS #################################################

# all parameters in [s]

network_node_time_step = 0.001
total_simulation_time = 100000.0
synapse_update_interval = 0.1

if len(sys.argv) > 1 and "TEST_MODE" in sys.argv[1:]:
    total_simulation_time = 20.0

nest_n_threads = 2 # None -> auto; Integer -> according fixed number of threads

# EXPERIMENT

min_phase_duration = 0.5  # [s]
max_phase_duration = 1.0  # [s]
background_rate = 2  # [hz]
max_rate = 60  # [hz]
random_seed = 1338

# REWARD

spike_rate_time_window = 0.5  # [s]
spike_rate_buffer_clear_on_phase_change = True
reward_integration_time = 50  # [s]
reward_offset = 0.02  #
reward_target_scale = 0.2  #
reward_soft_threshold = 25.0  # [hz]

# ASSESSMENT

compute_node_print_interval = 1000

plotter_node_time_window = max_phase_duration * 15  # [s]

# #######################################################################
# ########################### NETWORK ###################################
# #######################################################################

synapse_init_mean = -0.5
synapse_init_std = 0.5

n_max_connections_input_hidden = 2
p_connection_input_hidden = 1.0

synapse_properties = {
    "temperature": 0.1,  # temperature of parameter posterior
    "gradient_noise": 0.0,  # temperature on reward gradient
    "episode_length": s_to_ms(1.0),  # episode length parameter of synapse
    "integration_time": s_to_ms(50.0),  # time to estimate expectations over
    "learning_rate": 0.005 * (1.0 / 100000.0),  # learning rate
    "parameter_mapping_offset": 3.0,  # offset theta_0
    "psp_tau_fall": 20.0,  # psp falling edge time constant
    "psp_tau_rise": 2.0,  # psp rising edge time constant
    "max_param": 5.0,  # maximum synaptic parameter
    "min_param": -2.0,  # minimum synaptic parameter
    "max_param_change": 40.0,  # maximum synaptic parameter change
    "gradient_scale": 1.0,  # activate synaptic gradient integration
    "direct_gradient_rate": 0.0,  # rate of direct effect of gradient on the synapse
    "prior_mean": 0.0,  # mean of parameter prior
    "prior_inverse_variance": 0.25,  # inverse variance of parameter prior
}

hidden_neuron_properties = {
    "dead_time": s_to_ms(0.005),  # refractory time after spike
    "target_rate": 5.0,  # target spiking rate
    "I_e": -3.0,  # membrane offset parameter
    "target_adaptation_speed": 0.1,  # adaptation speed of homestatic mechanism
    "c_1": 0.0,  # weight for linear part of firing probability
    "c_2": 1.0,  # weight for exponential part of firing probability
    "c_3": 1.0,  # temperature for exponential part of firing probability
    "with_reset": False  # don't reset membrane after spike
}

