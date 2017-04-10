#! /usr/bin/env python

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
        nest.CopyModel("synaptic_sampling_rewardgradient_synapse", "test_synapse")
        #nest.SetDefaults("test_synapse", { "weight_update_time" : 50.0 })
        nest.SetDefaults("test_synapse", { "temperature" : 0.0, "synaptic_parameter" : 1.0, "reward_transmitter" : nodes[0] })
        nest.Connect( [nodes[0]], [nodes[1]], "one_to_one", { "model" : "test_synapse" } )
        conns = nest.GetConnections([nodes[0]], [nodes[1]], "test_synapse")
        nest.SetStatus(conns,{ "recorder_interval" : 100.0 })
                
        nest.Simulate(exp_len)
        
        results = nest.GetStatus(conns, ["recorder_times","synaptic_parameter_values"])
        
        times = (0.0, 100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0, 900.0, 1000.0, 1100.0, 1200.0, 1300.0,
                 1400.0, 1500.0, 1600.0, 1700.0, 1800.0, 1900.0, 2000.0, 2100.0, 2200.0, 2300.0, 2400.0, 2500.0, 2600.0,
                 2700.0, 2800.0, 2900.0, 3000.0, 3100.0, 3200.0, 3300.0, 3400.0, 3500.0, 3600.0, 3700.0, 3800.0, 3900.0,
                 4000.0, 4100.0, 4200.0, 4300.0, 4400.0, 4500.0, 4600.0, 4700.0, 4800.0, 4900.0, 5000.0, 5100.0, 5200.0,
                 5300.0, 5400.0, 5500.0, 5600.0, 5700.0, 5800.0, 5900.0, 6000.0, 6100.0, 6200.0, 6300.0, 6400.0, 6500.0,
                 6600.0, 6700.0, 6800.0, 6900.0, 7000.0, 7100.0, 7200.0, 7300.0, 7400.0, 7500.0, 7600.0, 7700.0, 7800.0,
                 7900.0, 8000.0, 8100.0, 8200.0, 8300.0, 8400.0, 8500.0, 8600.0, 8700.0, 8800.0, 8900.0, 9000.0, 9100.0,
                 9200.0, 9300.0, 9400.0, 9500.0, 9600.0, 9700.0, 9800.0, 9900.0)

        values = (1.0, 0.99, 0.9801, 0.970299, 0.9605960100000001, 0.9509900499, 0.941480149401, 0.93206534790699,
                  0.9227446944279202, 0.9135172474836409, 0.9043820750088045, 0.8953382542587165, 0.8863848717161293,
                  0.8775210229989681, 0.8687458127689783, 0.8600583546412885, 0.8514577710948756, 0.8429431933839269,
                  0.8345137614500876, 0.8261686238355868, 0.8179069375972309, 0.8097278682212585, 0.801630589539046,
                  0.7936142836436555, 0.785678140807219, 0.7778213593991468, 0.7700431458051553, 0.7623427143471038,
                  0.7547192872036327, 0.7471720943315964, 0.7397003733882804, 0.7323033696543976, 0.7249803359578536,
                  0.717730532598275, 0.7105532272722923, 0.7034476949995694, 0.6964132180495737, 0.6894490858690779,
                  0.6825545950103872, 0.6757290490602833, 0.6689717585696805, 0.6622820409839837, 0.6556592205741438,
                  0.6491026283684024, 0.6426116020847183, 0.6361854860638712, 0.6298236312032325, 0.6235253948912002,
                  0.6172901409422882, 0.6111172395328653, 0.6050060671375367, 0.5989560064661613, 0.5929664464014998,
                  0.5870367819374848, 0.5811664141181099, 0.5753547499769288, 0.5696012024771595, 0.563905190452388,
                  0.5582661385478641, 0.5526834771623854, 0.5471566423907616, 0.5416850759668539, 0.5362682252071853,
                  0.5309055429551135, 0.5255964875255623, 0.5203405226503067, 0.5151371174238036, 0.5099857462495656,
                  0.50488588878707, 0.49983702989919926, 0.4948386596002073, 0.48989027300420523, 0.48499137027416317,
                  0.48014145657142154, 0.47534004200570734, 0.4705866415856503, 0.46588077516979376, 0.461221967418095,
                  0.4566097477439149, 0.45204365026647575, 0.447523213763811, 0.44304798162617287, 0.43861750180991116,
                  0.43423132679181203, 0.4298890135238939, 0.425590123388655, 0.42133422215476846, 0.4171208799332208,
                  0.4129496711338886, 0.4088201744225497, 0.4047319726783242, 0.400684652951541, 0.3966778064220256,
                  0.39271102835780536, 0.3887839180742273, 0.38489607889348504, 0.38104711810455016, 0.377236646923504,
                  0.3734642804542696, 0.3697296376497269)

        self.assertAlmostEqual( np.sum( ( np.array(results[0][0]) - np.array(times) )**2 ), 0.0 )
        self.assertAlmostEqual( np.sum( ( np.array(results[0][1]) - np.array(values) )**2 ), 0.0 )

    def test_reward_synapse(self):
        self.spore_connection_test(1.0, 100, 100, 10000.0)

if __name__ == '__main__':
    nest.Install("sporemodule")
    unittest.main()
