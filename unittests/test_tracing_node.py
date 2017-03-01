#! /usr/bin/python

import numpy as np
import nest
import pylab as pl
import unittest
import time


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

