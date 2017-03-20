"""
Defines the tests for the 1D model within UCVM.

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
from ucvm.src.shared.test import UCVMTestCase
from ucvm.src.framework.ucvm import UCVM
from ucvm.src.shared.properties import SeismicData, Point, VelocityProperties
from ucvm.src.shared.test import assert_velocity_properties


class OneDimensionalVelocityModelTest(UCVMTestCase):
    """
    Defines the 1D test cases.
    """
    description = "1D"

    def setUp(self) -> None:
        """
        Set up all the initial SeismicData points.

        Returns:
            Nothing
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

        Returns:
            Nothing
        """
        self._test_start("1D BBP format test")

        UCVM.query(self.data["depth"], "1d[CyberShake_BBP_LA_Basin]", ["velocity"])

        assert_velocity_properties(
            self,
            self.data["depth"][0],
            VelocityProperties(1700, 450, 2000, 45.0, 22.5,
                               "cybershake la basin linearly interpolated with 1km moho (interpolated)",
                               "cybershake la basin linearly interpolated with 1km moho (interpolated)",
                               "cybershake la basin linearly interpolated with 1km moho (interpolated)",
                               "cybershake la basin linearly interpolated with 1km moho (interpolated)",
                               "cybershake la basin linearly interpolated with 1km moho (interpolated)")
        )
        assert_velocity_properties(
            self,
            self.data["depth"][1],
            VelocityProperties(1850, 900, 2100, 90.0, 45,
                               "cybershake la basin linearly interpolated with 1km moho (interpolated)",
                               "cybershake la basin linearly interpolated with 1km moho (interpolated)",
                               "cybershake la basin linearly interpolated with 1km moho (interpolated)",
                               "cybershake la basin linearly interpolated with 1km moho (interpolated)",
                               "cybershake la basin linearly interpolated with 1km moho (interpolated)")
        )
        assert_velocity_properties(
            self,
            self.data["depth"][2],
            VelocityProperties(1900, 950, 2100, 95.0, 47.5,
                               "cybershake la basin linearly interpolated with 1km moho (interpolated)",
                               "cybershake la basin linearly interpolated with 1km moho (interpolated)",
                               "cybershake la basin linearly interpolated with 1km moho (interpolated)",
                               "cybershake la basin linearly interpolated with 1km moho (interpolated)",
                               "cybershake la basin linearly interpolated with 1km moho (interpolated)")
        )
        assert_velocity_properties(
            self,
            self.data["depth"][3],
            VelocityProperties(7800, 4500, 3200, 450.0, 225.0,
                               "cybershake la basin linearly interpolated with 1km moho (interpolated)",
                               "cybershake la basin linearly interpolated with 1km moho (interpolated)",
                               "cybershake la basin linearly interpolated with 1km moho (interpolated)",
                               "cybershake la basin linearly interpolated with 1km moho (interpolated)",
                               "cybershake la basin linearly interpolated with 1km moho (interpolated)")
        )

        self._test_end()
