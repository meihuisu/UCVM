"""
Defines the tests for the CVM-S4 model within UCVM.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   September 1, 2016
:modified:  October 21, 2016
"""
import unittest

from ucvm.src.framework.ucvm import UCVM
from ucvm.src.shared.properties import SeismicData, Point, VelocityProperties
from ucvm.src.shared.constants import UCVM_ELEVATION
from ucvm.src.shared.test import assert_velocity_properties, run_acceptance_test


class CVMS4VelocityModelTest(unittest.TestCase):

    def setUp(self):
        self.data = {
            "none": [],
            "one": [
                SeismicData(Point(-118, 34, 0))
            ]
        }

    def test_cvms4_basic_depth(self):
        """
        Tests UCVM's basic query capabilities with CVM-S4.
        :return: None
        """
        self.assertTrue(UCVM.query(self.data["none"], "cvms4", ["velocity"]))

        UCVM.query(self.data["one"], "cvms4", ["velocity"])

        assert_velocity_properties(
            self,
            self.data["one"][0],
            VelocityProperties(696.491, 213, 1974.976, None, None,
                               "cvms4", "cvms4", "cvms4", None, None)
        )

    def test_cvms4_acceptance(self):
        """
        Broad acceptance test. This is a built-in UCVM feature that parses the model's
        bottom left corner, the nx, ny, nz, etc. from the test grid file, and makes sure that
        our query matches the expected one.
        :return:
        """
        self.assertTrue(run_acceptance_test(self, "cvms4"))
