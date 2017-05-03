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
        nest.SetKernelStatus({"resolution": p_sim["resolution"]})
        nest.sli_func('InitSynapseUpdater', p_sim["interval"], p_sim["delay"])
        nest.CopyModel("spore_test_node", "test_tracing_node", {"test_name": "test_tracing_node"})
        nodes = nest.Create("test_tracing_node", p_sim["num_synapses"])
        synapse_properties["reward_transmitter"] = nodes[0]
        nest.CopyModel("synaptic_sampling_rewardgradient_synapse", "test_synapse")
        nest.SetDefaults("test_synapse", synapse_properties)
        nest.Connect([nodes[0]], nodes, "all_to_all", {"model": "test_synapse"})
        conns = nest.GetConnections([nodes[0]], nodes, "test_synapse")

        syn_param = np.arange(0, 1, 1.0 / 100)
        nest.SetStatus(conns, [{"recorder_interval": 100.0, "synaptic_parameter": p} for p in syn_param])

        frame = p_sim["frame"]

        num_syn = np.zeros(int(p_sim["exp_len"] / frame) + 1)

        for i, t in enumerate(np.arange(0, p_sim["exp_len"], frame)):
            nest.Simulate(frame)
            conns = nest.GetConnections([nodes[0]], nodes, "test_synapse")
            num_syn[i] = len(conns)

        self.assertAlmostEqual(np.sum((num_syn - np.array(target_values))**2), 0.0)

    def test_garbage_collector_0(self):
        p_sim = {"resolution": 1.0, "interval": 100, "delay": 100,
                 "exp_len": 5000.0, "num_synapses": 100, "frame": 30.0}

        synapse_properties = {"weight_update_interval": 100.0, "temperature": 0.0,
                              "synaptic_parameter": 1.0, "reward_transmitter": None,
                              "learning_rate": 0.0001, "episode_length": 100.0, "prior_mean": -1.0,
                              "max_param": 100.0, "min_param": -100.0, "max_param_change": 100.0,
                              "gradient_scale": 0.0, "delete_retracted_synapses": False, "integration_time": 10000.0}

        target_values = [100.] * 167

        self.spore_connection_test(p_sim, synapse_properties, target_values)

    def test_garbage_collector_1(self):
        p_sim = {"resolution": 1.0, "interval": 100, "delay": 100,
                 "exp_len": 5000.0, "num_synapses": 100, "frame": 30.0}

        synapse_properties = {"weight_update_interval": 100.0, "temperature": 0.0,
                              "synaptic_parameter": 1.0, "reward_transmitter": None,
                              "learning_rate": 0.0001, "episode_length": 100.0, "prior_mean": -1.0,
                              "max_param": 100.0, "min_param": -100.0, "max_param_change": 100.0,
                              "gradient_scale": 0.0, "delete_retracted_synapses": True, "integration_time": 10000.0}

        target_values = [100.,  100.,  100.,  100.,  100.,  100.,   98.,   98.,   98.,
                         98.,   98.,   98.,   98.,   96.,   96.,   96.,   96.,   96.,
                         96.,   94.,   94.,   94.,   94.,   94.,   94.,   94.,   92.,
                         92.,   92.,   92.,   92.,   92.,   92.,   90.,   90.,   90.,
                         90.,   90.,   90.,   88.,   88.,   88.,   88.,   88.,   88.,
                         88.,   86.,   86.,   86.,   86.,   86.,   86.,   86.,   83.,
                         83.,   83.,   83.,   83.,   83.,   81.,   81.,   81.,   81.,
                         81.,   81.,   81.,   78.,   78.,   78.,   78.,   78.,   78.,
                         78.,   76.,   76.,   76.,   76.,   76.,   76.,   73.,   73.,
                         73.,   73.,   73.,   73.,   73.,   71.,   71.,   71.,   71.,
                         71.,   71.,   71.,   68.,   68.,   68.,   68.,   68.,   68.,
                         66.,   66.,   66.,   66.,   66.,   66.,   66.,   63.,   63.,
                         63.,   63.,   63.,   63.,   63.,   60.,   60.,   60.,   60.,
                         60.,   60.,   57.,   57.,   57.,   57.,   57.,   57.,   57.,
                         54.,   54.,   54.,   54.,   54.,   54.,   54.,   52.,   52.,
                         52.,   52.,   52.,   52.,   49.,   49.,   49.,   49.,   49.,
                         49.,   49.,   45.,   45.,   45.,   45.,   45.,   45.,   45.,
                         42.,   42.,   42.,   42.,   42.,   42.,   39.,   39.,   39.,
                         39.,   39.,   39.,   39.,   36.]

        self.spore_connection_test(p_sim, synapse_properties, target_values)

    def test_garbage_collector_2(self):
        p_sim = {"resolution": 1.0, "interval": 100, "delay": 100,
                 "exp_len": 5000.0, "num_synapses": 100, "frame": 30.0}

        synapse_properties = {"weight_update_interval": 100.0, "temperature": 0.0,
                              "synaptic_parameter": 1.0, "reward_transmitter": None,
                              "learning_rate": 0.0001, "episode_length": 100.0, "prior_mean": -2.0,
                              "max_param": 100.0, "min_param": -100.0, "max_param_change": 100.0,
                              "gradient_scale": 0.0, "delete_retracted_synapses": True, "integration_time": 10000.0}

        target_values = [100.,  100.,  100.,  100.,  100.,  100.,   97.,   97.,   97.,
                         97.,   97.,   97.,   97.,   93.,   93.,   93.,   93.,   93.,
                         93.,   89.,   89.,   89.,   89.,   89.,   89.,   89.,   85.,
                         85.,   85.,   85.,   85.,   85.,   85.,   81.,   81.,   81.,
                         81.,   81.,   81.,   76.,   76.,   76.,   76.,   76.,   76.,
                         76.,   72.,   72.,   72.,   72.,   72.,   72.,   72.,   67.,
                         67.,   67.,   67.,   67.,   67.,   62.,   62.,   62.,   62.,
                         62.,   62.,   62.,   57.,   57.,   57.,   57.,   57.,   57.,
                         57.,   53.,   53.,   53.,   53.,   53.,   53.,   47.,   47.,
                         47.,   47.,   47.,   47.,   47.,   42.,   42.,   42.,   42.,
                         42.,   42.,   42.,   37.,   37.,   37.,   37.,   37.,   37.,
                         32.,   32.,   32.,   32.,   32.,   32.,   32.,   26.,   26.,
                         26.,   26.,   26.,   26.,   26.,   21.,   21.,   21.,   21.,
                         21.,   21.,   15.,   15.,   15.,   15.,   15.,   15.,   15.,
                         9.,    9.,    9.,    9.,    9.,    9.,    9.,    4.,    4.,
                         4.,    4.,    4.,    4.,    0.,    0.,    0.,    0.,    0.,
                         0.,    0.,    0.,    0.,    0.,    0.,    0.,    0.,    0.,
                         0.,    0.,    0.,    0.,    0.,    0.,    0.,    0.,    0.,
                         0.,    0.,    0.,    0.,    0.]

        self.spore_connection_test(p_sim, synapse_properties, target_values)

    def test_garbage_collector_3(self):
        p_sim = {"resolution": 0.2, "interval": 500, "delay": 500,
                 "exp_len": 5000.0, "num_synapses": 100, "frame": 30.0}

        synapse_properties = {"weight_update_interval": 100.0, "temperature": 0.0,
                              "synaptic_parameter": 1.0, "reward_transmitter": None,
                              "learning_rate": 0.0001, "episode_length": 100.0, "prior_mean": -2.0,
                              "max_param": 100.0, "min_param": -100.0, "max_param_change": 100.0,
                              "gradient_scale": 0.0, "delete_retracted_synapses": True, "integration_time": 10000.0}

        target_values = [100.,  100.,  100.,  100.,  100.,  100.,   97.,   97.,   97.,
                         97.,   97.,   97.,   97.,   93.,   93.,   93.,   93.,   93.,
                         93.,   89.,   89.,   89.,   89.,   89.,   89.,   89.,   85.,
                         85.,   85.,   85.,   85.,   85.,   85.,   81.,   81.,   81.,
                         81.,   81.,   81.,   76.,   76.,   76.,   76.,   76.,   76.,
                         76.,   72.,   72.,   72.,   72.,   72.,   72.,   72.,   67.,
                         67.,   67.,   67.,   67.,   67.,   62.,   62.,   62.,   62.,
                         62.,   62.,   62.,   57.,   57.,   57.,   57.,   57.,   57.,
                         57.,   53.,   53.,   53.,   53.,   53.,   53.,   47.,   47.,
                         47.,   47.,   47.,   47.,   47.,   42.,   42.,   42.,   42.,
                         42.,   42.,   42.,   37.,   37.,   37.,   37.,   37.,   37.,
                         32.,   32.,   32.,   32.,   32.,   32.,   32.,   26.,   26.,
                         26.,   26.,   26.,   26.,   26.,   21.,   21.,   21.,   21.,
                         21.,   21.,   15.,   15.,   15.,   15.,   15.,   15.,   15.,
                         9.,    9.,    9.,    9.,    9.,    9.,    9.,    4.,    4.,
                         4.,    4.,    4.,    4.,    0.,    0.,    0.,    0.,    0.,
                         0.,    0.,    0.,    0.,    0.,    0.,    0.,    0.,    0.,
                         0.,    0.,    0.,    0.,    0.,    0.,    0.,    0.,    0.,
                         0.,    0.,    0.,    0.,    0.]

        self.spore_connection_test(p_sim, synapse_properties, target_values)


if __name__ == '__main__':
    nest.Install("sporemodule")
    unittest.main()
