"""
Defines the tests for the 1D model within UCVM.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   September 1, 2016
:modified:  September 1, 2016
"""
import unittest

from ucvm.src.framework.ucvm import UCVM
from ucvm.src.shared.properties import SeismicData, Point, VelocityProperties
from ucvm.src.shared.constants import UCVM_ELEVATION
from ucvm.src.shared.test import assert_velocity_properties


class OneDimensionalVelocityModelTest(unittest.TestCase):

    def setUp(self) -> None:
        """
        Set up all the initial SeismicData points.
        :return: Nothing
        """
        self.data = {
            "depth": [
                SeismicData(Point(-118, 34, 0)),
                SeismicData(Point(-118, 34, 16)),
                SeismicData(Point(-118, 34, 20)),
                SeismicData(Point(-118, 34, 100000))
            ]
        }

    def test_onedimensional_bbp_depth(self) -> None:
        """
        Tests the 1D model for basic depth query.
        :return: Nothing
        """
        UCVM.query(self.data["depth"], "1d[Whittier Narrows]", ["velocity"])

        assert_velocity_properties(
            self,
            self.data["depth"][0],
            VelocityProperties(1700, 450, 2000, 45.0, 22.5,
                               "whittier narrows 1d bbp", "whittier narrows 1d bbp",
                               "whittier narrows 1d bbp", "whittier narrows 1d bbp",
                               "whittier narrows 1d bbp")
        )
        assert_velocity_properties(
            self,
            self.data["depth"][1],
            VelocityProperties(1900, 950, 2100, 95.0, 47.5,
                               "whittier narrows 1d bbp", "whittier narrows 1d bbp",
                               "whittier narrows 1d bbp", "whittier narrows 1d bbp",
                               "whittier narrows 1d bbp")
        )
        assert_velocity_properties(
            self,
            self.data["depth"][2],
            VelocityProperties(2000, 1150, 2200, 115.0, 57.5,
                               "whittier narrows 1d bbp", "whittier narrows 1d bbp",
                               "whittier narrows 1d bbp", "whittier narrows 1d bbp",
                               "whittier narrows 1d bbp")
        )
        assert_velocity_properties(
            self,
            self.data["depth"][3],
            VelocityProperties(7800, 4500, 3200, 450.0, 225.0,
                               "whittier narrows 1d bbp", "whittier narrows 1d bbp",
                               "whittier narrows 1d bbp", "whittier narrows 1d bbp",
                               "whittier narrows 1d bbp")
        )

    def test_onedimensional_bbp_linear(self) -> None:
        """
        Tests the 1D model for basic depth, linear query.
        :return: Nothing
        """
        """UCVM.query(self.data["depth"], "1d[Whittier Narrows,linear]", ["velocity"])

        assert_velocity_properties(
            self,
            self.data["depth"][0],
            VelocityProperties(1700, 450, 2000, 45.0, 22.5,
                               "whittier narrows 1d bbp (interpolated)",
                               "whittier narrows 1d bbp (interpolated)",
                               "whittier narrows 1d bbp (interpolated)",
                               "whittier narrows 1d bbp (interpolated)",
                               "whittier narrows 1d bbp (interpolated)")
        )
        assert_velocity_properties(
            self,
            self.data["depth"][1],
            VelocityProperties(1950, 1050, 2150, 105.0, 52.5,
                               "whittier narrows 1d bbp (interpolated)",
                               "whittier narrows 1d bbp (interpolated)",
                               "whittier narrows 1d bbp (interpolated)",
                               "whittier narrows 1d bbp (interpolated)",
                               "whittier narrows 1d bbp (interpolated)")
        )
        assert_velocity_properties(
            self,
            self.data["depth"][2],
            VelocityProperties(2000, 1150, 2200, 115.0, 57.5,
                               "whittier narrows 1d bbp (interpolated)",
                               "whittier narrows 1d bbp (interpolated)",
                               "whittier narrows 1d bbp (interpolated)",
                               "whittier narrows 1d bbp (interpolated)",
                               "whittier narrows 1d bbp (interpolated)")
        )
        assert_velocity_properties(
            self,
            self.data["depth"][3],
            VelocityProperties(7800, 4500, 3200, 450.0, 225.0,
                               "whittier narrows 1d bbp (interpolated)",
                               "whittier narrows 1d bbp (interpolated)",
                               "whittier narrows 1d bbp (interpolated)",
                               "whittier narrows 1d bbp (interpolated)",
                               "whittier narrows 1d bbp (interpolated)")
        )"""
