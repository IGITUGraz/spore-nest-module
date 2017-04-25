#! /usr/bin/python

import unittest
import subprocess
import sys
import os


class RewardInProxyTest(unittest.TestCase):
    def test(self):
        if (sys.version_info[0] == 3):
            music_filename = "test_py3.music"
        else:
            music_filename = "test.music"
        subprocess.check_call(["mpirun", "-np", "2", "music", music_filename],
                              cwd=os.path.dirname(os.path.realpath(__file__)))


if __name__ == '__main__':
    unittest.main()
