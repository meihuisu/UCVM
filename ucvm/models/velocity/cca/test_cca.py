"""
Defines the tests for the CCA model within UCVM.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   September 1, 2016
:modified:  September 1, 2016
"""
import unittest

from ucvm.src.framework.ucvm import UCVM


class CCAVelocityModelTest(unittest.TestCase):

    def setUp(self):
        self.data = {
            "none": []
        }

    def test_cca_basic_depth(self):
        self.assertTrue(UCVM.query(self.data["none"], "cca"))
