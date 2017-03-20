"""
Defines the tests for the USGS/NOAA model within UCVM.

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
from ucvm.src.shared.properties import SeismicData, Point
from ucvm.src.shared.test import UCVMTestCase


class USGSNOAAElevationModelTest(UCVMTestCase):
    """
    Defines the test cases for the USGS/NOAA elevation model. Three cases are tested in total.
    """
    description = "USGS/NOAA"

    def test_etopo1_basic(self):
        """
        Tests that the USGS/NOAA map delivers the expected ETOPO1 data at certain latitudes and
        longitudes around the world.

        Returns:
            None
        """
        self._test_start("test for correct ETOPO1 data")

        sd_test = [
            SeismicData(Point(-122.65, 49.21667, 0)),
            SeismicData(Point(-114.0833, 51.05, 0))
        ]

        self.assertTrue(UCVM.query(sd_test, "usgs-noaa", ["elevation"]))

        self.assertAlmostEqual(sd_test[0].elevation_properties.elevation, 19.994, 3)
        self.assertEqual(sd_test[0].elevation_properties.elevation_source, "usgs-noaa")
        self.assertAlmostEqual(sd_test[1].elevation_properties.elevation, 1051.015, 3)
        self.assertEqual(sd_test[1].elevation_properties.elevation_source, "usgs-noaa")

        self._test_end()

    def test_nationalmap_basic(self):
        """
        Tests that the USGS/NOAA map delivers the expected National Map data at certain latitudes
        and longitudes around the world.

        Returns:
             None
        """
        self._test_start("test for correct National Map data")

        sd_test = [
            SeismicData(Point(-118, 34, 0)),
            SeismicData(Point(-119, 35, 0))
        ]

        self.assertTrue(UCVM.query(sd_test, "usgs-noaa", ["elevation"]))

        self.assertAlmostEqual(sd_test[0].elevation_properties.elevation, 287.997, 3)
        self.assertEqual(sd_test[0].elevation_properties.elevation_source, "usgs-noaa")
        self.assertAlmostEqual(sd_test[1].elevation_properties.elevation, 466.993, 3)
        self.assertEqual(sd_test[1].elevation_properties.elevation_source, "usgs-noaa")

        self._test_end()

    def test_fails_incorrect_lat_lon_bounds(self):
        """
        Tests that the USGS/NOAA map does not return heights for latitudes and longitudes that
        fall outside of the possible ranges.

        Returns:
            None
        """
        self._test_start("test for out-of-bounds lat, lon")

        sd_test = [SeismicData(Point(200, 34, 0)), SeismicData(Point(-118, 340, 0))]

        self.assertTrue(UCVM.query(sd_test, "usgs-noaa", ["elevation"]))

        self.assertEqual(sd_test[0].elevation_properties.elevation, None)
        self.assertEqual(sd_test[0].elevation_properties.elevation_source, None)
        self.assertEqual(sd_test[1].elevation_properties.elevation, None)
        self.assertEqual(sd_test[1].elevation_properties.elevation_source, None)

        self._test_end()

    def test_falls_to_etopo_in_ocean(self):
        """
        Tests that in the ocean - where the USGS map has no data - we fall back to the ETOPO1
        map which has the bathymetry data in it.

        Returns:
            None
        """
        self._test_start("test for ocean using ETOPO1 data")

        sd_test = [SeismicData(Point(-118.2, 33.5, 0))]

        self.assertTrue(UCVM.query(sd_test, "usgs-noaa", ["elevation"]))

        self.assertLess(sd_test[0].elevation_properties.elevation, 0)
        self.assertEqual(sd_test[0].elevation_properties.elevation_source, "usgs-noaa")

        self._test_end()
