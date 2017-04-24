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
    def spore_connection_test(self, resolution, interval, delay, exp_len):
        nest.ResetKernel()
        nest.SetKernelStatus({"resolution": resolution})
        nest.sli_func('InitSynapseUpdater', interval, delay)
        nest.CopyModel("spore_test_node", "test_tracing_node", {"test_name": "test_tracing_node"})
        nodes = nest.Create("test_tracing_node", 2)
        nest.CopyModel("spore_test_synapse", "test_synapse")
        nest.SetDefaults("test_synapse", {"weight_update_time": 100.0})
        nest.Connect([nodes[0]], [nodes[1]], "one_to_one", {"model": "test_synapse"})
        nest.Simulate(exp_len)

        conns = nest.GetConnections([nodes[0]], [nodes[1]], "test_synapse")
        results = nest.GetStatus(conns, ["recorder_times", "recorder_values"])

        offset = 1.0 if (resolution <= 1.0) else resolution

        self.assertAlmostEqual(np.mean(np.abs(results[0][0] - np.arange(0.0, exp_len - offset, resolution))), 0.0)
        self.assertAlmostEqual(np.mean(np.abs(results[0][1] - np.arange(0.0, (exp_len - offset) / resolution))), 0.0)

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
        self.spore_connection_test(1.0, 100, 50, 500)

    def test_spore_connection_7(self):
        self.spore_connection_test(0.1, 100, 100, 400)

    def test_spore_connection_9(self):
        self.spore_connection_test(5.0, 50, 50, 500)


if __name__ == '__main__':
    nest.Install("sporemodule")
    unittest.main()
