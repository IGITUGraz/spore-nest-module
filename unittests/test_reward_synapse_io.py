#! /usr/bin/env python

#
# This file is part of SPORE.
#
# Copyright (C) 2016, the SPORE team (see AUTHORS).
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

import numpy as np
import nest
import unittest
import time


class TestStringMethods(unittest.TestCase):


    def assert_parameter_limits_min(self, param_name, limit ):
        
        nest.ResetKernel()
        nest.CopyModel("synaptic_sampling_rewardgradient_synapse", "test_synapse")

        try:
            nest.SetDefaults("test_synapse", { param_name : limit-1 })
            self.fail("Expected exception of type NESTError, but got nothing")
        except Exception as e:
            self.assertEqual(type(e).__name__, "NESTError", "Expected exception of type NESTError, but got: '"+type(e).__name__+"'")

        nest.ResetKernel()
        nest.CopyModel("synaptic_sampling_rewardgradient_synapse", "test_synapse")

        try:
            nest.SetDefaults("test_synapse", { param_name : limit })
        except Exception as e:
            self.fail("Expected no exception, but got: '"+type(e).__name__+"'")

        nest.ResetKernel()
        nest.CopyModel("synaptic_sampling_rewardgradient_synapse", "test_synapse")

        try:
            nest.SetDefaults("test_synapse", { param_name : limit+1 })
        except Exception as e:
            self.fail("Expected no exception, but got: '"+type(e).__name__+"'")


    def assert_parameter_limits_bigger(self, param_name, limit ):
        
        nest.ResetKernel()
        nest.CopyModel("synaptic_sampling_rewardgradient_synapse", "test_synapse")

        try:
            nest.SetDefaults("test_synapse", { param_name : limit-1 })
            self.fail("Expected exception of type NESTError, but got nothing")
        except Exception as e:
            self.assertEqual(type(e).__name__, "NESTError", "Expected exception of type NESTError, but got: '"+type(e).__name__+"'")

        nest.ResetKernel()
        nest.CopyModel("synaptic_sampling_rewardgradient_synapse", "test_synapse")

        try:
            nest.SetDefaults("test_synapse", { param_name : limit })
            self.fail("Expected exception of type NESTError, but got nothing")
        except Exception as e:
            self.assertEqual(type(e).__name__, "NESTError", "Expected exception of type NESTError, but got: '"+type(e).__name__+"'")

        nest.ResetKernel()
        nest.CopyModel("synaptic_sampling_rewardgradient_synapse", "test_synapse")

        try:
            nest.SetDefaults("test_synapse", { param_name : limit+1 })
        except Exception as e:
            self.fail("Expected no exception, but got: '"+type(e).__name__+"'")

    def assert_parameter_no_limits(self, param_name, val ):
        
        nest.ResetKernel()
        nest.CopyModel("synaptic_sampling_rewardgradient_synapse", "test_synapse")

        try:
            nest.SetDefaults("test_synapse", { param_name : val })
        except Exception as e:
            self.fail("Expected no exception, but got: '"+type(e).__name__+"'")


    # test connection
    def test_parameter_limits(self):
        self.assert_parameter_no_limits("learning_rate", 0.0)
        self.assert_parameter_limits_min("temperature", 0.0)
        self.assert_parameter_limits_min("gradient_noise", 0.0)
        self.assert_parameter_limits_bigger("psp_tau_rise", 0.0)
        self.assert_parameter_limits_bigger("psp_tau_fall", 0.0)
        self.assert_parameter_limits_bigger("integration_time", 0.0)
        self.assert_parameter_limits_bigger("episode_length", 0.0)
        self.assert_parameter_limits_bigger("weight_update_interval", 0.0)
        self.assert_parameter_no_limits("parameter_mapping_offset", 0.0)
        self.assert_parameter_no_limits("max_param", 0.0)
        self.assert_parameter_no_limits("min_param", 0.0)
        self.assert_parameter_limits_min("max_param_change", 0.0)
        self.assert_parameter_no_limits("direct_gradient_rate", 0.0)
        self.assert_parameter_no_limits("gradient_scale", 0.0)
        self.assert_parameter_limits_min("bap_trace_id", 0)
        self.assert_parameter_limits_min("dopa_trace_id", 0)
        self.assert_parameter_no_limits("simulate_retracted_synapses", False)
        self.assert_parameter_no_limits("verbose", False)


    # test connection
    def test_7_bad_property_error_reward_transmitter(self):
        
        nest.ResetKernel()
        nest.SetKernelStatus({"resolution":1.0})
        
        nest.sli_func('InitSynapseUpdater',100,100)
        nest.CopyModel("spore_test_node", "test_tracing_node", {"test_name":"test_tracing_node"})
        
        nodes = nest.Create("test_tracing_node",2)
        nest.CopyModel("synaptic_sampling_rewardgradient_synapse", "test_synapse")
        nest.SetDefaults("test_synapse", { "temperature" : 0.0, "synaptic_parameter" : 1.0 })
        nest.Connect( [nodes[0]], [nodes[1]], "one_to_one", { "model" : "test_synapse" } )
        
        try:
            nest.Simulate(1000)
            self.fail("Expected exception of type NESTError, but got nothing")
        except Exception as e:
            self.assertEqual(type(e).__name__, "NESTError", "Expected exception of type NESTError, but got: '"+type(e).__name__+"'")


if __name__ == '__main__':
    nest.Install("sporemodule")
    unittest.main()
