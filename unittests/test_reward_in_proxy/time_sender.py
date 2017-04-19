#! /usr/bin/python

import music
import itertools
import numpy as np

def main():
    setup = music.Setup()

    def make_out_proxy(port_name):
        proxy = setup.publishContOutput(port_name)
        assert proxy.isConnected()
        assert proxy.width() > 0
        buf = np.zeros(proxy.width())
        proxy.map(buf, maxBuffered=1)
        return buf

    buf = make_out_proxy("out")

    def runtime(setup, timestep, stoptime):
        for t in itertools.takewhile(lambda t: t < stoptime,
                                    setup.runtime(timestep)):
            yield t

    for time in runtime(setup, timestep=0.001, stoptime=0.5):
        buf[:] = [time + float(i) for i, _ in enumerate(range(len(buf)))]

if __name__ == "__main__":
    main()
