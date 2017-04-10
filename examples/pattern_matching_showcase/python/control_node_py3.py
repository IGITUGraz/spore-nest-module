#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import subprocess

subprocess.call([sys.executable, "control_node.py"]+sys.argv[1:])

