"""
Defines the tests for the CVM-H 15.1.0 model within UCVM.

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
from ucvm.src.shared.properties import SeismicData, Point, VelocityProperties
from ucvm.src.shared.constants import UCVM_ELEVATION
from ucvm.src.framework.ucvm import UCVM
from ucvm.src.shared.test import run_acceptance_test, UCVMTestCase, assert_velocity_properties


class CVMH1510VelocityModelTest(UCVMTestCase):
    """
    Defines the test cases for the CVM-H velocity model. Four tests are done: an acceptance test, a test query by
    elevation, a test query with and without GTL, and a test query with and without the 1D background model.
    """
    description = "CVM-H 15.1.0"

    def test_cvmh_query_by_elevation(self):
        """
        Tests the CVM-H query capabilities using the model's DEM.

        Returns:
            None
        """
        self._test_start("test for CVM-H query using elevation")

        sd_test = [SeismicData(Point(-118, 34, 0, UCVM_ELEVATION))]

        self.assertTrue(UCVM.query(sd_test, "cvmh1510[gt].elevation"))

        assert_velocity_properties(
            self,
            sd_test[0],
            VelocityProperties(2495.1269, 978.5577, 2091.678, None, None,
                               "cvmh1510", "cvmh1510", "cvmh1510", None, None)
        )

        sd_test = [SeismicData(Point(-118, 34, 0))]

        self.assertTrue(UCVM.query(sd_test, "cvmh1510[gtl]"))

        # Query by depth.
        assert_velocity_properties(
            self,
            sd_test[0],
            VelocityProperties(824.177, 195, 1084.062, None, None,
                               "cvmh1510", "cvmh1510", "cvmh1510", None, None)
        )

        # Checks to see that the material properties returned at 266.690277 elevation at -118, 34 equal the
        # material properties returned at 0m depth (i.e. that CVM-H is using its own DEM correctly).
        sd_test = [SeismicData(Point(-118, 34, 266.690277099609, UCVM_ELEVATION))]

        self.assertTrue(UCVM.query(sd_test, "cvmh1510[gtl].elevation"))

        assert_velocity_properties(
            self,
            sd_test[0],
            VelocityProperties(824.177, 195, 1084.062, None, None,
                               "cvmh1510", "cvmh1510", "cvmh1510", None, None)
        )

        # Check to see that "air" returns nothing (there's a bug in vx that will return 1000 for density in air).
        sd_test = [SeismicData(Point(-118, 34, 1000, UCVM_ELEVATION))]

        self.assertTrue(UCVM.query(sd_test, "cvmh1510.elevation"))
        assert_velocity_properties(
            self,
            sd_test[0],
            VelocityProperties(None, None, None, None, None, None, None, None, None, None)
        )

        self._test_end()

    def test_cvmh_query_gtl(self):
        """
        Tests the CVM-H query capabilities both with the GTL and without.

        Returns:
            None
        """
        self._test_start("test for CVM-H query using GTL")

        sd_test = [SeismicData(Point(-118, 34, 0))]

        self.assertTrue(UCVM.query(sd_test, "cvmh1510[gtl]"))

        assert_velocity_properties(
            self,
            sd_test[0],
            VelocityProperties(824.177, 195, 1084.062, None, None,
                               "cvmh1510", "cvmh1510", "cvmh1510", None, None)
        )

        sd_test = [SeismicData(Point(-118, 34, 0))]

        self.assertTrue(UCVM.query(sd_test, "cvmh1510"))

        # No 1D means None for everything.
        assert_velocity_properties(
            self,
            sd_test[0],
            VelocityProperties(2484.3879, 969.2999, 2088.316, None, None,
                               "cvmh1510", "cvmh1510", "cvmh1510", None, None)
        )

        self._test_end()

    def test_cvmh_query_1d_background(self):
        """
        Tests the CVM-H query capabilities both with the 1D background model and without.

        Returns:
            None
        """
        self._test_start("test for CVM-H query with 1D model")

        sd_test = [SeismicData(Point(-122.0322, 37.3230, 0))]

        self.assertTrue(UCVM.query(sd_test, "cvmh1510[1d]"))

        assert_velocity_properties(
            self,
            sd_test[0],
            VelocityProperties(5000.0, 2886.7514, 2654.5, None, None,
                               "cvmh1510", "cvmh1510", "cvmh1510", None, None)
        )

        sd_test = [SeismicData(Point(-122.0322, 37.3230, 0))]

        self.assertTrue(UCVM.query(sd_test, "cvmh1510"))

        # No 1D means None for everything.
        assert_velocity_properties(
            self,
            sd_test[0],
            VelocityProperties(None, None, None, None, None,
                               None, None, None, None, None)
        )

        self._test_end()

    def test_cvmh_acceptance(self):
        """
        Runs the built-in acceptance test for the CVM-H velocity model. This compares a known
        grid of lat, lon material properties - queried at depth - to what this installation of the
        CVM-H velocity model returns on the user's computer.

        Returns:
            None
        """
        self._test_start("CVM-H acceptance test")
        self.assertTrue(run_acceptance_test(self, "cvmh1510"))
        self._test_end()
