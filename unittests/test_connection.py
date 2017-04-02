#! /usr/bin/python

import numpy as np
import nest
import unittest
import time



class TestStringMethods(unittest.TestCase):

    # test connection
    def spore_connection_test(self, resolution, interval, delay, exp_len):
        nest.ResetKernel()
        nest.SetKernelStatus({"resolution":resolution})
        nest.sli_func('InitSynapseUpdater',interval,delay)
        nest.CopyModel("spore_test_node", "test_tracing_node", {"test_name":"test_tracing_node"})
        nodes = nest.Create("test_tracing_node",2)
        nest.CopyModel("spore_test_synapse", "test_synapse")
        nest.SetDefaults("test_synapse", { "weight_update_time" : 100.0 })
        nest.Connect( [nodes[0]], [nodes[1]], "one_to_one", { "model" : "test_synapse" } )        
        nest.Simulate(exp_len)
        
        conns = nest.GetConnections([nodes[0]], [nodes[1]], "test_synapse")
        results = nest.GetStatus(conns, ["recorder_times","recorder_values"])
        
        offset = 1.0 if (resolution<=1.0) else resolution
       
        self.assertTrue( np.mean( np.abs( results[0][0] - np.arange(0.0,exp_len-offset,resolution) ) ) < 1e-8, "synapse temporal synchrony" )
        self.assertTrue( np.mean( np.abs( results[0][1] - np.arange(0.0,(exp_len-offset)/resolution) ) ) < 1e-8, "synapse trace values" )


    def test_spore_connection_1(self):
        self.spore_connection_test(1.0, 50, 50, 400)

    def test_spore_connection_2(self):
        self.spore_connection_test(1.0, 50, 50, 800)

    def test_spore_connection_3(self):
        self.spore_connection_test(1.0, 25, 25, 800)

    def test_spore_connection_4(self):
        self.spore_connection_test(1.0, 5, 5, 800)
        
    def test_spore_connection_5(self):
        self.spore_connection_test(1.0, 100, 100, 400)        

    def test_spore_connection_6(self):
        self.spore_connection_test(1.0, 100, 50, 450)        

    def test_spore_connection_7(self):
        self.spore_connection_test(0.1, 100, 100, 400)
        
    def test_spore_connection_9(self):
        self.spore_connection_test(5.0, 50, 50, 500)
        

if __name__ == '__main__':
    nest.Install("sporemodule")
    unittest.main()
    

