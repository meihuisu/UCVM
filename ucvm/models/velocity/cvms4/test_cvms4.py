"""
Defines the tests for the CVM-S4 model within UCVM.

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


class CVMS4VelocityModelTest(unittest.TestCase):

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

    def test_cvms4_basic_depth(self):
        """
        Tests UCVM's basic query capabilities with CVM-S4.
        :return: None
        """
        self.assertTrue(UCVM.query(self.data["none"], "cvms4"))

        UCVM.query(self.data["one"], "cvms4")

        assert_velocity_properties(
            self,
            self.data["one"][0],
            VelocityProperties(696.491, 213, 1974.976, None, None,
                               "cvms4", "cvms4", "cvms4", None, None)
        )

    def test_cvms4_basic_elevation(self):
        """
        Tests UCVM's basic query but with elevation instead of depth.
        :return: None
        """
        UCVM.query(self.data["elevation"], "cvms4.elevation")

        assert_velocity_properties(
            self,
            self.data["elevation"][0],
            VelocityProperties(696.491, 213, 1974.976, None, None,
                               "cvms4", "cvms4", "cvms4", None, None)
        )

    def test_cvms4_parse_ucvm_string(self):
        """
        Tests that UCVM correctly assigns the default models to CVM-S4 with various string
        configurations.
        :return: None
        """
        self.assertEqual(UCVM.parse_model_string("cvms4"),
                         {0: {0: "cvms4", 1: "usgs_noaa", 2: "vs30_calc", "query_by": None}})
        self.assertEqual(UCVM.parse_model_string("cvms4.depth"),
                         {0: {0: "cvms4", 1: "usgs_noaa", 2: "vs30_calc", "query_by": None}})
        self.assertEqual(UCVM.parse_model_string("cvms4.elevation"),
                         {0: {0: "usgs_noaa", 1: "cvms4", 2: "vs30_calc", "query_by": "depth"}})
        self.assertEqual(UCVM.parse_model_string("cvms4(Extra Stuff).vs30_calc"),
                         {0: {0: "cvms4;Extra Stuff", 1: "usgs_noaa", 2: "vs30_calc",
                              "query_by": None}})
        self.assertEqual(UCVM.parse_model_string("cvms4(ES),cvms4"),
                         {0: {0: "cvms4;ES", 1: "usgs_noaa", 2: "vs30_calc", "query_by": None},
                          1: {0: "cvms4", 1: "usgs_noaa", 2: "vs30_calc", "query_by": None}})
