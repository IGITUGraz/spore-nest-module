#
# This file is part of SPORE.
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

cmake_minimum_required( VERSION 2.8.12 )

add_test( NAME test_circular_buffer COMMAND python ${CMAKE_CURRENT_LIST_DIR}/test_circular_buffer.py )
add_test( NAME test_connection COMMAND python ${CMAKE_CURRENT_LIST_DIR}/test_connection.py )
add_test( NAME test_dbl_exp_neuron COMMAND python ${CMAKE_CURRENT_LIST_DIR}/test_dbl_exp_neuron.py )
add_test( NAME test_dbl_exp_neuron_io COMMAND python ${CMAKE_CURRENT_LIST_DIR}/test_dbl_exp_neuron_io.py )
add_test( NAME test_dbl_exp_neuron_rate COMMAND python ${CMAKE_CURRENT_LIST_DIR}/test_dbl_exp_neuron_rate.py )
add_test( NAME test_tracing_node COMMAND python ${CMAKE_CURRENT_LIST_DIR}/test_tracing_node.py )
add_test( NAME test_reward_synapse COMMAND python ${CMAKE_CURRENT_LIST_DIR}/test_reward_synapse.py )
add_test( NAME test_reward_synapse_io COMMAND python ${CMAKE_CURRENT_LIST_DIR}/test_reward_synapse_io.py )
add_test( NAME test_reward_in_proxy COMMAND python ${CMAKE_CURRENT_LIST_DIR}/test_reward_in_proxy/test.py )

# Integration Tests
add_test( NAME test_music_integration COMMAND python ${CMAKE_CURRENT_LIST_DIR}/test_music_integration.py )
