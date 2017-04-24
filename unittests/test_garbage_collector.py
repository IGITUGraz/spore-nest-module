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


class TestStringMethods(unittest.TestCase):

    # test connection
    def spore_connection_test(self, p_sim, synapse_properties, target_values):

        nest.ResetKernel()
        nest.SetKernelStatus({"resolution":p_sim["resolution"]})
        nest.sli_func('InitSynapseUpdater',p_sim["interval"],p_sim["delay"])
        nest.CopyModel("spore_test_node", "test_tracing_node", {"test_name":"test_tracing_node"})
        nodes = nest.Create("test_tracing_node",p_sim["num_synapses"])
        synapse_properties["reward_transmitter"] = nodes[0]
        nest.CopyModel("synaptic_sampling_rewardgradient_synapse", "test_synapse")
        nest.SetDefaults("test_synapse", synapse_properties )
        nest.Connect( [nodes[0]], nodes, "all_to_all", { "model" : "test_synapse" } )
        conns = nest.GetConnections([nodes[0]], nodes, "test_synapse")
        
        syn_param = np.arange(0,1,1.0/100)
        nest.SetStatus( conns,[ { "recorder_interval" : 100.0, "synaptic_parameter":p } for p in syn_param ] )

        frame = p_sim["frame"]

        num_syn = np.zeros(int(p_sim["exp_len"]/frame)+1)

        for i,t in enumerate(np.arange(0, p_sim["exp_len"], frame)):
            nest.Simulate(frame)
            conns = nest.GetConnections([nodes[0]], nodes, "test_synapse")
            num_syn[i] = len(conns)
        
        self.assertAlmostEqual( np.sum( ( num_syn - np.array(target_values) )**2 ), 0.0 )


    def test_garbage_collector_0(self):
        p_sim = { "resolution": 1.0, "interval": 100, "delay": 100,
                  "exp_len": 5000.0, "num_synapses": 100, "frame":30.0 }
        
        synapse_properties = { "weight_update_interval" : 100.0, "temperature" : 0.0,
                               "synaptic_parameter" : 1.0, "reward_transmitter" : None,
                               "learning_rate": 0.0001, "episode_length": 100.0, "prior_mean":-1.0,
                               "max_param": 100.0, "min_param": -100.0, "max_param_change": 100.0,
                               "gradient_scale": 0.0, "delete_retracted_synapses": False, "integration_time": 10000.0 }
                               
        target_values =  [ 100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,
                           100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,
                           100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,
                           100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,
                           100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,
                           100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,
                           100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,
                           100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,
                           100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,
                           100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,
                           100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,
                           100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,
                           100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,
                           100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,
                           100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,
                           100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,
                           100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,
                           100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,  100.,
                           100.,  100.,  100.,  100.,  100.]
                             
        self.spore_connection_test(p_sim, synapse_properties, target_values)


    def test_garbage_collector_1(self):
        p_sim = { "resolution": 1.0, "interval": 100, "delay": 100,
                  "exp_len": 5000.0, "num_synapses": 100, "frame":30.0 }
        
        synapse_properties = { "weight_update_interval" : 100.0, "temperature" : 0.0,
                               "synaptic_parameter" : 1.0, "reward_transmitter" : None,
                               "learning_rate": 0.0001, "episode_length": 100.0, "prior_mean":-1.0,
                               "max_param": 100.0, "min_param": -100.0, "max_param_change": 100.0,
                               "gradient_scale": 0.0, "delete_retracted_synapses": True, "integration_time": 10000.0 }
                               
        target_values =  [ 100.,  100.,  100.,  100.,  100.,  100.,   98.,   98.,   98.,
                            97.,   97.,   97.,   97.,   96.,   96.,   96.,   95.,   95.,
                            95.,   94.,   94.,   94.,   94.,   93.,   93.,   93.,   92.,
                            92.,   92.,   91.,   91.,   91.,   91.,   90.,   90.,   90.,
                            89.,   89.,   89.,   88.,   88.,   88.,   88.,   87.,   87.,
                            87.,   86.,   86.,   86.,   84.,   84.,   84.,   84.,   83.,
                            83.,   83.,   82.,   82.,   82.,   81.,   81.,   81.,   81.,
                            80.,   80.,   80.,   78.,   78.,   78.,   77.,   77.,   77.,
                            77.,   76.,   76.,   76.,   75.,   75.,   75.,   73.,   73.,
                            73.,   73.,   72.,   72.,   72.,   71.,   71.,   71.,   70.,
                            70.,   70.,   70.,   68.,   68.,   68.,   67.,   67.,   67.,
                            66.,   66.,   66.,   66.,   64.,   64.,   64.,   63.,   63.,
                            63.,   62.,   62.,   62.,   62.,   60.,   60.,   60.,   59.,
                            59.,   59.,   57.,   57.,   57.,   57.,   56.,   56.,   56.,
                            54.,   54.,   54.,   53.,   53.,   53.,   53.,   52.,   52.,
                            52.,   50.,   50.,   50.,   49.,   49.,   49.,   49.,   47.,
                            47.,   47.,   45.,   45.,   45.,   44.,   44.,   44.,   44.,
                            42.,   42.,   42.,   41.,   41.,   41.,   39.,   39.,   39.,
                            39.,   38.,   38.,   38.,   36.]
                             
        self.spore_connection_test(p_sim, synapse_properties, target_values)


    def test_garbage_collector_2(self):
        p_sim = { "resolution": 1.0, "interval": 100, "delay": 100,
                  "exp_len": 5000.0, "num_synapses": 100, "frame":30.0 }
        
        synapse_properties = { "weight_update_interval" : 100.0, "temperature" : 0.0,
                               "synaptic_parameter" : 1.0, "reward_transmitter" : None,
                               "learning_rate": 0.0001, "episode_length": 100.0, "prior_mean":-2.0,
                               "max_param": 100.0, "min_param": -100.0, "max_param_change": 100.0,
                               "gradient_scale": 0.0, "delete_retracted_synapses": True, "integration_time": 10000.0 }
                               
        target_values =  [ 100.,  100.,  100.,  100.,  100.,  100.,   97.,   97.,   97.,
                            95.,   95.,   95.,   95.,   93.,   93.,   93.,   91.,   91.,
                            91.,   89.,   89.,   89.,   89.,   87.,   87.,   87.,   85.,
                            85.,   85.,   83.,   83.,   83.,   83.,   81.,   81.,   81.,
                            78.,   78.,   78.,   76.,   76.,   76.,   76.,   74.,   74.,
                            74.,   72.,   72.,   72.,   69.,   69.,   69.,   69.,   67.,
                            67.,   67.,   65.,   65.,   65.,   62.,   62.,   62.,   62.,
                            60.,   60.,   60.,   57.,   57.,   57.,   55.,   55.,   55.,
                            55.,   53.,   53.,   53.,   50.,   50.,   50.,   47.,   47.,
                            47.,   47.,   45.,   45.,   45.,   42.,   42.,   42.,   40.,
                            40.,   40.,   40.,   37.,   37.,   37.,   35.,   35.,   35.,
                            32.,   32.,   32.,   32.,   29.,   29.,   29.,   26.,   26.,
                            26.,   24.,   24.,   24.,   24.,   21.,   21.,   21.,   18.,
                            18.,   18.,   15.,   15.,   15.,   15.,   12.,   12.,   12.,
                             9.,    9.,    9.,    6.,    6.,    6.,    6.,    4.,    4.,
                             4.,    1.,    1.,    1.,    0.,    0.,    0.,    0.,    0.,
                             0.,    0.,    0.,    0.,    0.,    0.,    0.,    0.,    0.,
                             0.,    0.,    0.,    0.,    0.,    0.,    0.,    0.,    0.,
                             0.,    0.,    0.,    0.,    0.]

        self.spore_connection_test(p_sim, synapse_properties, target_values)

if __name__ == '__main__':
    nest.Install("sporemodule")
    unittest.main()
