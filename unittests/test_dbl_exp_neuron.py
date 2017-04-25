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


def spike_in(t, spike_times):
    """
    check if spike time is in the list.
    """
    return np.any([t - 0.01 < s and t + 0.01 > s for s in spike_times])


class TestStringMethods(unittest.TestCase):

    def test_spore_exp_poisson_neuron(self):
        nest.ResetKernel()

        # create neuron and multimeter
        w_exc = 3.0
        w_inh = -2.0
        T = 100
        tau_rise_exc = 2.0
        tau_fall_exc = 20.0
        tau_rise_inh = 1.0
        tau_fall_inh = 10.0
        delta_t = 0.1
        syn_delay = 1.0 - delta_t
        spike_times_ex = [10.0, 20.0, 50.0]
        spike_times_in = [15.0, 25.0]
        I_e = -3.0

        n = nest.Create('poisson_dbl_exp_neuron',  params={
                        'tau_rise_exc': tau_rise_exc, 'tau_fall_exc': tau_fall_exc, 'tau_rise_inh': tau_rise_inh, 'tau_fall_inh': tau_fall_inh, 'I_e': I_e})

        m = nest.Create('multimeter', params={'withtime': True, 'interval': 0.1, 'record_from': ['V_m']})

        # Create spike generators and connect
        gex = nest.Create('spike_generator', params={'spike_times': np.array(spike_times_ex)})
        gin = nest.Create('spike_generator',  params={'spike_times': np.array(spike_times_in)})

        nest.Connect(gex, n, 'one_to_one', {'weight':  w_exc})  # excitatory
        nest.Connect(gin, n, 'one_to_one', {'weight': w_inh, "receptor_type": 1})  # inhibitory
        nest.Connect(m, n)

        u_gd = np.zeros(int((T - 1.0) * (1.0 / delta_t)))
        u_rise_exc = 0
        u_fall_exc = 0
        u_rise_inh = 0
        u_fall_inh = 0

        for t in np.arange(0, T - 1.0, delta_t):

            u_rise_exc *= np.exp(-delta_t / tau_rise_exc)
            u_fall_exc *= np.exp(-delta_t / tau_fall_exc)
            u_rise_inh *= np.exp(-delta_t / tau_rise_inh)
            u_fall_inh *= np.exp(-delta_t / tau_fall_inh)

            if spike_in(t - syn_delay, spike_times_ex):
                u_rise_exc += w_exc
                u_fall_exc += w_exc

            if spike_in(t - syn_delay, spike_times_in):
                u_rise_inh += w_inh
                u_fall_inh += w_inh

            u_gd[int(t * (1.0 / delta_t))] = (tau_fall_exc / (tau_fall_exc - tau_rise_exc)) * (u_fall_exc -
                                                                                               u_rise_exc) + (tau_fall_inh / (tau_fall_inh - tau_rise_inh)) * (u_fall_inh - u_rise_inh) + I_e

        # simulate
        nest.Simulate(100)

        # obtain and display data
        events = nest.GetStatus(m)[0]['events']
        t = events['times']

        self.assertTrue(np.sum((events['V_m'] - u_gd)**2) < 0.00001, "simple poisson neuron test")


if __name__ == '__main__':
    nest.Install("sporemodule")
    unittest.main()
