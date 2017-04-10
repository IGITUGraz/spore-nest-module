#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import subprocess

sys.exit( subprocess.call([sys.executable, "nest_receiver.py"]+sys.argv[1:]) )

