"""
Defines the tests for the CVM-S4.26 model within UCVM.

Copyright:
    Southern California Earthquake Center

Developer:
    David Gill <davidgil@usc.edu>
"""

# UCVM Imports
from ucvm.src.framework.ucvm import UCVM
from ucvm.src.shared.properties import SeismicData, Point
from ucvm.src.shared.test import UCVMTestCase


class CVMS426VelocityModelTest(UCVMTestCase):
    """
    Defines the CVM-S4.26 test cases.
    """
    description = "CVM-S4.26"

    def test_cvms426_random_points(self):
        """
        Tests the CVM-S4.26 velocity model at 10 random points selected from the model. These points were picked with
        a random number generator.

        Returns:
            None
        """
        self._test_start("CVM-S4.26 points versus text model test")

        test_data = \
            """
            90      754     10      -118.737022   32.944866   5571.414  3057.374  2792.542
            724     627     15      -116.004835   34.763130   6383.228  3702.753  2863.826
            449     125     19      -114.943019   32.340362   6094.292  3597.746  2817.387
            924     1011    21      -116.896018   36.574456   5872.374  3248.397  2874.105
            664     1416    22      -119.570398   36.833491   6357.490  3675.896  2882.698
            385     1027    39      -118.869259   34.753449   6344.310  3578.079  2856.254
            100     1496    43      -121.846471   35.058333   7403.315  4269.781  3034.174
            689     567     48      -115.882460   34.466751   6493.230  3719.698  2909.571
            361     850     49      -118.208090   34.163152   6771.910  3571.393  2953.548
            444     723     53      -117.391091   34.083565   6937.791  4099.394  2912.123
            736     901     56      -117.105324   35.607358   6727.089  3885.899  2943.158
            949     1062    56      -117.023118   36.809121   6649.673  3770.664  2931.769
            396     1061    69      -118.975273   34.888717   7781.172  4487.968  3095.297
            558     1513    84      -120.364910   36.732035   7862.731  4538.450  3107.515
            961     534     87      -114.765700   35.290399   7873.649  4544.644  3108.936
            722     73      90      -113.767296   33.102653   7892.802  4558.516  3110.357
            142     309     96      -116.744822   31.837443   7945.724  4599.341  3113.200
            847     4       99      -113.043306   33.310262   7914.255  4569.352  3114.621
            670     240     99      -114.620776   33.430007   7919.039  4574.102  3114.621
            177     1240    100     -120.485602   34.623764   7869.447  4529.999  3115.094
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
        self.assertTrue(UCVM.query(sd_test, "cvms426", ["velocity"]))

        epsilon = 0.00001
        for i in range(len(sd_test)):
            self.assertGreater(sd_test[i].velocity_properties.vp / points[i][3], 1 - epsilon)
            self.assertLess(sd_test[i].velocity_properties.vp / points[i][3], 1 + epsilon)
            self.assertGreater(sd_test[i].velocity_properties.vs / points[i][4], 1 - epsilon)
            self.assertLess(sd_test[i].velocity_properties.vs / points[i][4], 1 + epsilon)
            self.assertGreater(sd_test[i].velocity_properties.density / points[i][5], 1 - epsilon)
            self.assertLess(sd_test[i].velocity_properties.density / points[i][5], 1 + epsilon)

        self._test_end()
