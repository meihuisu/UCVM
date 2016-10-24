"""
Defines the tests for the CVM-H 15.1.0 model within UCVM.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   October 11, 2016
:modified:  October 11, 2016
"""
import unittest

from ucvm.src.shared.properties import SeismicData, Point
from ucvm.src.shared.constants import UCVM_ELEVATION
from ucvm.src.shared.test import run_acceptance_test


class CVMH1510VelocityModelTest(unittest.TestCase):

    def setUp(self):
        self.data = {
            "none": [],
            "one": [
                SeismicData(Point(-118, 34, 0))
            ],
            "elevation": [
                SeismicData(Point(-118, 34, 276.99, UCVM_ELEVATION))
            ]
        }

    def test_cvmh_acceptance(self):
        """
        Broad acceptance test. This is a built-in UCVM feature that parses the model's
        bottom left corner, the nx, ny, nz, etc. from the test grid file, and makes sure that
        our query matches the expected one.
        :return:
        """
        self.assertTrue(run_acceptance_test(self, "cvmh1510"))
