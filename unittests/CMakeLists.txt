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

# Test SPORE core functions
add_test( NAME circular_buffer COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/test_circular_buffer.py )
add_test( NAME tracing_node COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/test_tracing_node.py )
add_test( NAME connection COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/test_connection.py )

# Test models
add_test( NAME dbl_exp_neuron COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/test_dbl_exp_neuron.py )
add_test( NAME dbl_exp_neuron_io COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/test_dbl_exp_neuron_io.py )
add_test( NAME dbl_exp_neuron_rate COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/test_dbl_exp_neuron_rate.py )
add_test( NAME reward_synapse COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/test_reward_synapse.py )
add_test( NAME reward_synapse_io COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/test_reward_synapse_io.py )
add_test( NAME reward_synapse_stdp COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/test_reward_synapse_stdp.py )
add_test( NAME reward_in_proxy COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/test_reward_in_proxy/test.py )

# Integration Tests
add_test( NAME music_integration COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/test_music_integration.py )
