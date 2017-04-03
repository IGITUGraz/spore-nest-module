#! /usr/bin/python

import unittest
import subprocess
import os


class MusicIntegrationTest(unittest.TestCase):
    def test_pattern_matching(self):
        showcase_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', 'examples', 'pattern_matching_showcase')
        subprocess.check_call(["sh", "experiment.test.sh"], cwd=showcase_path)

if __name__ == '__main__':
    unittest.main()
