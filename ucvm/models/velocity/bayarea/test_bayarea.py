"""
Defines the tests for the Bay Area model within UCVM.

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
from ucvm.src.shared.properties import SeismicData, Point, VelocityProperties
from ucvm.src.shared.constants import UCVM_ELEVATION
from ucvm.src.shared.test import assert_velocity_properties, run_acceptance_test, UCVMTestCase


class BayAreaVelocityModelTest(UCVMTestCase):
    """
    Defines the test cases for the Bay Area velocity model. Two tests are done: an acceptance
    test and a test query by elevation.
    """
    description = "Bay Area"

    def test_bayarea_basic_elevation(self):
        """
        Tests the Bay Area query capabilities using elevation. This uses the model's DEM, not the
        UCVM DEM.

        Returns:
            None
        """
        self._test_start("test for Bay Area query by elevation")

        sd_test = [SeismicData(Point(-122.0322, 37.3230, 0, UCVM_ELEVATION))]

        self.assertTrue(UCVM.query(sd_test, "bayarea.elevation"))

        assert_velocity_properties(
            self,
            sd_test[0],
            VelocityProperties(1700.0, 390.0, 1990.0, 44.0, 22.0,
                               "bayarea", "bayarea", "bayarea", "bayarea", "bayarea")
        )

        sd_test = [SeismicData(Point(-130, 40, 0, UCVM_ELEVATION))]

        self.assertTrue(UCVM.query(sd_test, "bayarea.elevation", ["velocity", "elevation"]))

        # This is above sea level so it should be None for everything.
        assert_velocity_properties(
            self,
            sd_test[0],
            VelocityProperties(None, None, None, None, None,
                               None, None, None, None, None)
        )

        self._test_end()

    def test_bayarea_acceptance(self):
        """
        Runs the built-in acceptance test for the Bay Area velocity model. This compares a known
        grid of lat, lon material properties - queried at depth - to what this installation of the
        Bay Area velocity model returns on the user's computer.

        Returns:
            None
        """
        self._test_start("Bay Area acceptance test")
        self.assertTrue(run_acceptance_test(self, "bayarea"))
        self._test_end()
