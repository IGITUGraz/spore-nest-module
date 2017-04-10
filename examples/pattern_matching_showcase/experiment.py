#! /usr/bin/env python

#
# This file is part of SPORE.
#
# Copyright (C) 2016 Institute for Theoretical Computer Science, Graz University of Technology
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

import os
import sys
import subprocess


plotter = None

if not "TEST_MODE" in sys.argv[1:]:
    # don't create plotter in test mode
    try:
        dev0 = open("/dev/null", "w")
        plotter = subprocess.Popen([sys.executable, "./python/interface.py"], stdout=dev0)

    except:
        print("Error when running the plotter interface.")

try:
    os.chdir("python")

    if (sys.version_info[0] == 3):
        music_filename = "run_py3.music"
    else:
        music_filename = "run.music"

    print("Starting MUSIC/NEST simulation")

    cmd = "mpirun -n 3 music `readlink -f "+ music_filename +"`"+("".join([" "+str(arg) for arg in sys.argv[1:]]))

    subprocess.call( cmd, shell=True )

finally:
    if plotter is not None:
        plotter.kill()

