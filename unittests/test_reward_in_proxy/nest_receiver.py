#! /usr/bin/python

import numpy as np
import nest

def s_to_ms(s):
    return s * 1000.0

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
    values = np.array(map(s_to_ms, trace[1:N_STEPS]))
    expect = np.arange(0.0, SIMULATION_TIME, RESOLUTION)[:-1] + s_to_ms(float(i))

    assert np.allclose(values, expect), "{}\n{}".format(values, expect)
