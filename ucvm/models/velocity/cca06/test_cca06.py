"""
Defines the tests for the CCA model within UCVM.

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


class CCA06VelocityModelTest(UCVMTestCase):
    """
    Defines the test cases for the CCA06 velocity model. It just runs an acceptance test.
    """
    description = "CCA06"

    def test_cca06_random_points(self):
        """
        Tests the CCA06 velocity model at 10 random points selected from the model. These points were picked with
        a random number generator.

        Returns:
            None
        """
        self._test_start("CCA06 points versus text model test")

        test_data = \
            """
            438     254     5       -118.887875   35.478906   4895.436  2667.120  2435.278
            46      796     5       -122.422779   36.382854   2429.266  1417.205  2241.661
            335     108     8       -118.879781   34.674750   5172.307  2793.584  2480.476
            566     730     13      -119.865852   37.552742   6236.374  3768.482  2791.083
            951     7       19      -115.790940   35.880222   5989.860  3398.884  2668.398
            963     7       27      -115.735974   35.910148   6137.117  3460.438  2692.684
            954     887     31      -118.588744   39.145429   6179.948  3394.584  2665.174
            234     495     49      -120.580990   35.813587   6534.183  3684.867  2791.827
            447     679     49      -120.236755   37.050627   6946.152  4085.203  3005.396
            339     616     55      -120.513406   36.533256   6727.385  3888.453  2922.629
            987     167     58      -116.119090   36.566131   6582.264  3690.730  2803.978
            139     137     66      -119.834750   34.261905   8074.908  4733.310  3348.805
            532     82      68      -117.920905   35.092949   7987.021  4711.226  3343.804
            968     586     68      -117.534766   38.072441   7602.601  4214.103  3079.460
            574     40      72      -117.600991   35.046711   7739.257  4522.150  3249.446
            369     337     75      -119.463563   35.600098   7765.333  4526.807  3250.931
            156     149     90      -119.798850   34.350777   7970.437  4622.248  3294.685
            226     496     92      -120.619796   35.795691   7765.998  4442.617  3200.752
            5       522     96      -121.679680   35.290345   7905.824  4564.610  3270.179
            778     154     97      -117.036585   35.990024   7867.409  4607.975  3291.877
            """

        points = []
        for point in test_data.split("\n"):
            if point.strip() == "":
                continue
            point = point.split()
            points.append(
                (float(point[3]), float(point[4]), (int(point[2]) - 1) * 500, float(point[5]), float(point[6]),
                 float(point[7]))
            )

        sd_test = [SeismicData(Point(p[0], p[1], p[2])) for p in points]
        self.assertTrue(UCVM.query(sd_test, "cca06", ["velocity"]))

        epsilon = 0.00001
        for i in range(len(sd_test)):
            self.assertGreater(sd_test[i].velocity_properties.vp / points[i][3], 1 - epsilon)
            self.assertLess(sd_test[i].velocity_properties.vp / points[i][3], 1 + epsilon)
            self.assertGreater(sd_test[i].velocity_properties.vs / points[i][4], 1 - epsilon)
            self.assertLess(sd_test[i].velocity_properties.vs / points[i][4], 1 + epsilon)
            self.assertGreater(sd_test[i].velocity_properties.density / points[i][5], 1 - epsilon)
            self.assertLess(sd_test[i].velocity_properties.density / points[i][5], 1 + epsilon)

        self._test_end()
