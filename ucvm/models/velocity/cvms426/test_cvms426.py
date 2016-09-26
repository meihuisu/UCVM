"""
Defines the tests for the CVM-S4.26 model within UCVM.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   September 8, 2016
:modified:  September 8, 2016
"""
import unittest

from ucvm.src.framework.ucvm import UCVM


class CVMS426VelocityModelTest(unittest.TestCase):

    def setUp(self):
        self.data = {
            "none": []
        }

    def test_cvms426_basic_depth(self):
        self.assertTrue(UCVM.query(self.data["none"], "cvms426"))
