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


import numpy as np
import nest


def s_to_ms(s):
    return s * 1000.0


def main():
    RESOLUTION = s_to_ms(0.001)
    SIMULATION_TIME = s_to_ms(0.5)
    N_STEPS = int(SIMULATION_TIME / RESOLUTION)
    TRACE_LENGTH = N_STEPS + 1

    nest.ResetKernel()
    nest.SetKernelStatus({"resolution": RESOLUTION})
    nest.Install("sporemodule")
    nest.sli_func('InitSynapseUpdater', N_STEPS, 0)

    proxy = nest.Create("reward_in_proxy")
    nest.SetStatus(proxy, {'port_name': 'in', 'delay': 0.0})

    nest.Simulate(SIMULATION_TIME)
    traces = nest.GetStatus(proxy, ("trace"))[0]

    for i, trace in enumerate(traces):
        assert len(trace) == TRACE_LENGTH
        # NEST-inherent delay of one timestep forces first value to be unchanged
        assert trace[0] == 0.0, "Buffer on NEST side is initialized with 0."
        assert trace[N_STEPS] == 0.0, "Unused value is zero"

        # [0.0, received values ..., 0.0], #elements = N_STEPS + 1
        values = np.array(list(map(s_to_ms, trace[1:N_STEPS])))
        expect = np.arange(0.0, SIMULATION_TIME, RESOLUTION)[:-1] + s_to_ms(float(i))

        assert np.allclose(values, expect), "{}\n{}".format(values, expect)


if __name__ == "__main__":
    main()
