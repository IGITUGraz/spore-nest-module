#! /usr/bin/python

import numpy as np
import nest
import pylab as pl
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


    # test connection
    def test_parameter_limits(self):
        self.assert_parameter_limits_bigger("episode_length", 0.0)
        self.assert_parameter_limits_bigger("psp_facilitation_rate", 0.0)
        self.assert_parameter_limits_bigger("psp_depression_rate", 0.0)
        self.assert_parameter_limits_min("temperature", 0.0)
        self.assert_parameter_limits_min("gradient_noise", 0.0)
        self.assert_parameter_limits_min("max_param_change", 0.0)
        self.assert_parameter_limits_bigger("integration_time", 0.0)
        self.assert_parameter_limits_bigger("weight_update_time", 0.0)
        self.assert_parameter_limits_min("bap_trace_id", 0)
        self.assert_parameter_limits_min("dopa_trace_id", 0)


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
