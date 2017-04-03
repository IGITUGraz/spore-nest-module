#! /usr/bin/python

import unittest
import subprocess
import os


class PatternMatchingTest(unittest.TestCase):
    def test(self):
        showcase_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', 'examples', 'pattern_matching_showcase')
        print(showcase_path)
        subprocess.check_call(["mpirun", "-np", "3", "music", "run.music", "TEST_MODE"], cwd=showcase_path)

if __name__ == '__main__':
    unittest.main()
