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

import nest
import unittest


class TestStringMethods(unittest.TestCase):

    # test tracing node
    def test_tracing_node_1(self):
        nest.ResetKernel()
        nest.SetKernelStatus({"resolution":1.0})
        nest.sli_func('InitSynapseUpdater',200,200)
        nest.CopyModel("spore_test_node", "test_tracing_node", {"test_name":"test_tracing_node", "test_time":300.0})
        nest.Create("test_tracing_node",1)
        nest.Simulate(400)

    def test_tracing_node_2(self):
        nest.ResetKernel()
        nest.SetKernelStatus({"resolution":0.1})
        nest.sli_func('InitSynapseUpdater',2000,2000)
        nest.CopyModel("spore_test_node", "test_tracing_node", {"test_name":"test_tracing_node", "test_time":300.0})
        nest.Create("test_tracing_node",1)
        nest.Simulate(400)

    def test_tracing_node_3(self):
        nest.ResetKernel()
        nest.SetKernelStatus({"resolution":10.0})
        nest.sli_func('InitSynapseUpdater',20,20)
        nest.CopyModel("spore_test_node", "test_tracing_node", {"test_name":"test_tracing_node", "test_time":300.0})
        nest.Create("test_tracing_node",1)
        nest.Simulate(400)
        
    def test_tracing_node_4(self):
        nest.ResetKernel()
        nest.SetKernelStatus({"resolution":5.0})
        nest.sli_func('InitSynapseUpdater',40,40)
        nest.CopyModel("spore_test_node", "test_tracing_node", {"test_name":"test_tracing_node", "test_time":300.0})
        nest.Create("test_tracing_node",1)
        nest.Simulate(400)

if __name__ == '__main__':
    nest.Install("sporemodule")
    unittest.main()
