#! /usr/bin/python

import numpy as np
import nest
import unittest
import time


class TestStringMethods(unittest.TestCase):

    def do_exp_poisson_neuron_rate_test(self, I_e, target_rate):
        
        nest.ResetKernel()

        # create neuron and multimeter
        T = 10000
        tau_rise_exc = 2.0
        tau_fall_exc = 20.0
        tau_rise_inh = 1.0
        tau_fall_inh = 10.0

        n = nest.Create('poisson_dbl_exp_neuron',  params = {'tau_rise_exc': tau_rise_exc, 'tau_fall_exc': tau_fall_exc, 'tau_rise_inh': tau_rise_inh, 'tau_fall_inh': tau_fall_inh, 'I_e': I_e})

        m = nest.Create('spike_detector')

        nest.Connect(n, m)

        # simulate
        nest.Simulate(T)
        
        events = nest.GetStatus(m)[0]['events']
        
        rate = len(events['times'])*1000.0/T

        self.assertTrue( ( rate - target_rate )**2 < 0.00001, "poisson neuron rate test" )


    def test_spore_exp_poisson_neuron_rate_1(self):
        self.do_exp_poisson_neuron_rate_test(20.0, 154.7)

    def test_spore_exp_poisson_neuron_rate_2(self):
        self.do_exp_poisson_neuron_rate_test(10.0, 13.1)

    def test_spore_exp_poisson_neuron_rate_3(self):
        self.do_exp_poisson_neuron_rate_test(5.0, 3.7)

    def test_spore_exp_poisson_neuron_rate_4(self):
        self.do_exp_poisson_neuron_rate_test(3.0, 2.4)

    def test_spore_exp_poisson_neuron_rate_5(self):
        self.do_exp_poisson_neuron_rate_test(2.0, 2.0)

    def test_spore_exp_poisson_neuron_rate_6(self):
        self.do_exp_poisson_neuron_rate_test(1.0, 1.3)

    def test_spore_exp_poisson_neuron_rate_7(self):
        self.do_exp_poisson_neuron_rate_test(0.0, 1.1)

    def test_spore_exp_poisson_neuron_rate_8(self):
        self.do_exp_poisson_neuron_rate_test(-1.0, 0.6)

    def test_spore_exp_poisson_neuron_rate_9(self):
        self.do_exp_poisson_neuron_rate_test(-3.0, 0.4)

if __name__ == '__main__':
    nest.Install("sporemodule")
    unittest.main()
