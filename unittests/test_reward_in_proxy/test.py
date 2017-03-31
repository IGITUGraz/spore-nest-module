#! /usr/bin/python

import unittest
import subprocess
import os


class RewardInProxyTest(unittest.TestCase):
    def test(self):
        subprocess.check_call(["mpirun", "-np", "2", "music", "test.music"], cwd=os.path.dirname(os.path.realpath(__file__)))

if __name__ == '__main__':
    unittest.main()
