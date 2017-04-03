#! /usr/bin/python

import numpy as np
import nest
import unittest
import time



class TestStringMethods(unittest.TestCase):

    # test circular buffer
    def test_circular_buffer(self):
        nest.ResetKernel()
        nest.CopyModel("spore_test_node", "test_circular_buffer", {"test_name":"test_circular_buffer"})
        nest.Create("test_circular_buffer",1)
        nest.Simulate(1)


if __name__ == '__main__':
    nest.Install("sporemodule")
    unittest.main()

