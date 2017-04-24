#! /usr/bin/env python
# -*- coding: utf-8 -*-

import sys

try:
    import nest
    nest.Install("sporemodule")
except:
    sys.exit(-1)

sys.exit(0)
