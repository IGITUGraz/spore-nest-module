#! /usr/bin/python

import music
import itertools
import numpy as np

setup = music.Setup()

def make_out_proxy(port_name):
    proxy = setup.publishContOutput(port_name)
    assert proxy.isConnected()
    assert proxy.width() > 0
    buf = np.zeros(proxy.width())
    proxy.map(buf, maxBuffered=1)
    return buf

buf = make_out_proxy("out")

for time in itertools.takewhile(lambda t: t < 0.5, setup.runtime(0.001)):
    buf[:] = [time + float(i) for i, _ in enumerate(range(len(buf)))]
