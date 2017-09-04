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
