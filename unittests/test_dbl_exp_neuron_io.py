#! /usr/bin/env python
# -*- coding: utf-8 -*-

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

    def assert_parameter_limits_min(self, param_name, limit):

        nest.ResetKernel()

        try:
            nest.Create("poisson_dbl_exp_neuron", params={param_name: limit - 1})
            self.fail("Expected exception of type NESTError, but got nothing")
        except Exception as e:
            self.assertEqual(type(e).__name__, "NESTError",
                             "Expected exception of type NESTError, but got: '" + type(e).__name__ + "'")

        nest.ResetKernel()

        try:
            nest.Create("poisson_dbl_exp_neuron", params={param_name: limit})
        except Exception as e:
            self.fail("Expected no exception, but got: '" + type(e).__name__ + "'")

        nest.ResetKernel()

        try:
            nest.Create("poisson_dbl_exp_neuron", params={param_name: limit + 1})
        except Exception as e:
            self.fail("Expected no exception, but got: '" + type(e).__name__ + "'")

    def assert_parameter_limits_bigger(self, param_name, limit):

        nest.ResetKernel()

        try:
            nest.Create("poisson_dbl_exp_neuron", params={param_name: limit - 1})
            self.fail("Expected exception of type NESTError, but got nothing")
        except Exception as e:
            self.assertEqual(type(e).__name__, "NESTError",
                             "Expected exception of type NESTError, but got: '" + type(e).__name__ + "'")

        nest.ResetKernel()

        try:
            nest.Create("poisson_dbl_exp_neuron", params={param_name: limit})
            self.fail("Expected exception of type NESTError, but got nothing")
        except Exception as e:
            self.assertEqual(type(e).__name__, "NESTError",
                             "Expected exception of type NESTError, but got: '" + type(e).__name__ + "'")

        nest.ResetKernel()

        try:
            nest.Create("poisson_dbl_exp_neuron", params={param_name: limit + 1})
        except Exception as e:
            self.fail("Expected no exception, but got: '" + type(e).__name__ + "'")

    def assert_parameter_no_limits(self, param_name, val):

        nest.ResetKernel()

        try:
            nest.Create("poisson_dbl_exp_neuron", params={param_name: val})
        except Exception as e:
            self.fail("Expected no exception, but got: '" + type(e).__name__ + "'")

    # test connection
    def test_parameter_limits(self):
        self.assert_parameter_no_limits("V_m", 0.0)
        self.assert_parameter_limits_bigger("tau_rise_exc", 0.0)
        self.assert_parameter_limits_bigger("tau_fall_exc", 0.0)
        self.assert_parameter_limits_bigger("tau_rise_inh", 0.0)
        self.assert_parameter_limits_bigger("tau_fall_inh", 0.0)
        self.assert_parameter_no_limits("dead_time", 0.0)
        self.assert_parameter_no_limits("dead_time_random", False)
        self.assert_parameter_no_limits("dead_time_shape", 1)
        self.assert_parameter_no_limits("t_ref_remaining", 0.0)
        self.assert_parameter_no_limits("with_reset", False)
        self.assert_parameter_no_limits("I_e", 0.0)
        self.assert_parameter_no_limits("input_conductance", 0.0)
        self.assert_parameter_no_limits("c_1", 0.0)
        self.assert_parameter_no_limits("c_2", 0.0)
        self.assert_parameter_no_limits("c_3", 0.0)
        self.assert_parameter_no_limits("target_rate", 0.0)
        self.assert_parameter_no_limits("target_adaptation_speed", 0.0)


if __name__ == '__main__':
    nest.Install("sporemodule")
    unittest.main()
