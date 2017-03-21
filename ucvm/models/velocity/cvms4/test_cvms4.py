"""
Defines the tests for the CVM-S4 model within UCVM.

Copyright 2017 Southern California Earthquake Center

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""
# UCVM Imports
from ucvm.src.framework.ucvm import UCVM
from ucvm.src.shared.constants import UCVM_ELEVATION
from ucvm.src.shared.properties import SeismicData, Point, VelocityProperties
from ucvm.src.shared.test import assert_velocity_properties, run_acceptance_test, UCVMTestCase


class CVMS4VelocityModelTest(UCVMTestCase):
    """
    Defines the test cases for the CVM-S4 velocity model. Three cases are tested in total.
    """
    description = "CVM-S4"

    def test_cvms4_basic_depth(self):
        """
        Tests UCVM's basic query capabilities with CVM-S4 using depth. This tests that a known
        point within the model returns the correct material properties and also that a known point
        outside of the model returns the SCEC 1D background material properties.

        Returns:
            None
        """
        self._test_start("test for UCVM query by depth")

        sd_test = [SeismicData(Point(-118, 34, 0))]

        self.assertTrue(UCVM.query(sd_test, "cvms4", ["velocity"]))

        assert_velocity_properties(
            self,
            sd_test[0],
            VelocityProperties(696.491, 213, 1974.976, None, None,
                               "cvms4", "cvms4", "cvms4", None, None)
        )

        sd_test = [SeismicData(Point(-130, 40, 0))]

        self.assertTrue(UCVM.query(sd_test, "cvms4", ["velocity"]))

        assert_velocity_properties(
            self,
            sd_test[0],
            VelocityProperties(5000, 2886.751, 2654.5, None, None,
                               "cvms4", "cvms4", "cvms4", None, None)
        )

        self._test_end()

    def test_cvms4_basic_elevation(self):
        """
        Tests UCVM's query capabilities with CVM-S4 but using elevation. The tests are the same
        as with test_cvms4_basic_depth.

        Returns:
            None
        """
        self._test_start("test for UCVM query by elevation")

        sd_test = [SeismicData(Point(-118, 34, 0, UCVM_ELEVATION))]

        self.assertTrue(UCVM.query(sd_test, "cvms4.usgs-noaa.elevation", ["velocity", "elevation"]))

        assert_velocity_properties(
            self,
            sd_test[0],
            VelocityProperties(2677.1596, 725.390, 2287.7236, None, None,
                               "cvms4", "cvms4", "cvms4", None, None)
        )

        sd_test = [SeismicData(Point(-130, 40, 0, UCVM_ELEVATION))]

        self.assertTrue(UCVM.query(sd_test, "cvms4.usgs-noaa.elevation", ["velocity", "elevation"]))

        # This is above sea level so it should be None for everything.
        assert_velocity_properties(
            self,
            sd_test[0],
            VelocityProperties(None, None, None, None, None,
                               None, None, None, None, None)
        )

        self._test_end()

    def test_cvms4_acceptance(self):
        """
        Runs the built-in acceptance test for CVM-S4. This compares a known grid of lat, lon
        material properties - queried at depth - to what this installation of CVM-S4 returns
        on the users computer.

        Returns:
            None
        """
        self._test_start("CVM-S4 acceptance test")
        self.assertTrue(run_acceptance_test(self, "cvms4"))
        self._test_end()
