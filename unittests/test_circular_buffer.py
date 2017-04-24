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

    # test circular buffer
    def test_circular_buffer(self):
        nest.ResetKernel()
        nest.CopyModel("spore_test_node", "test_circular_buffer", {"test_name": "test_circular_buffer"})
        nest.Create("test_circular_buffer", 1)
        nest.Simulate(1)


if __name__ == '__main__':
    nest.Install("sporemodule")
    unittest.main()
