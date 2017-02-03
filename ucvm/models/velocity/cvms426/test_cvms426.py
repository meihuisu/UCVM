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
from ucvm.src.shared.test import run_acceptance_test, UCVMTestCase


class CVMS426VelocityModelTest(UCVMTestCase):
    """
    Defines the CVM-S4.26 test cases.
    """
    description = "CVM-S4.26"

    def test_cvms426_acceptance(self):
        """
        Runs the built-in acceptance test for CVM-S4.26. This compares a known grid of lat, lon
        material properties - queried at depth - to what this installation of CVM-S4.26 returns
        on the user's computer.

        Returns:
            None
        """
        self._test_start("CVM-S4.26 acceptance test")
        self.assertTrue(run_acceptance_test(self, "cvms426", ["vp", "vs"]))
        self._test_end()

    def test_cvms426_random_points(self):
        """
        Tests the CVM-S4.26 velocity model at 10 random points selected from the model. These points were picked with
        a random number generator.

        Returns:
            None
        """
        self._test_start("CVM-S4.26 points versus text model test")

        points = [
            (-120.36917, 33.94132, 28500, 7635.163, 4404.275, 3072.936),
            (-117.34040, 33.45887, 31000, 6860.176, 3925.341, 2940.449),
            (-119.57411, 34.95741, 13500, 5774.342, 3246.397, 2826.065),
            (-119.08653, 35.39738, 30500, 6714.774, 3936.081, 2935.889),
            (-119.55662, 34.59422, 0, 5261.258, 2622.981, 2691.475),
            (-116.61521, 33.09164, 41000, 7817.117, 4579.332, 3097.921),
            (-116.69498, 36.58593, 13000, 5823.612, 3226.479, 2868.256),
            (-116.65387, 36.21513, 39000, 7857.211, 4601.706, 3103.789),
            (-119.54603, 34.31039, 10500, 5758.586, 3166.990, 2786.374),
            (-116.07668, 34.49004, 500, 4333.793, 2590.457, 2657.449)
        ]

        sd_test = [SeismicData(Point(p[0], p[1], p[2])) for p in points]
        self.assertTrue(UCVM.query(sd_test, "cvms426", ["velocity"]))

        epsilon = 0.015
        for i in range(len(sd_test)):
            self.assertGreater(sd_test[i].velocity_properties.vp / points[i][3], 1 - epsilon)
            self.assertLess(sd_test[i].velocity_properties.vp / points[i][3], 1 + epsilon)
            self.assertGreater(sd_test[i].velocity_properties.vs / points[i][4], 1 - epsilon)
            self.assertLess(sd_test[i].velocity_properties.vs / points[i][4], 1 + epsilon)
            self.assertGreater(sd_test[i].velocity_properties.density / points[i][5], 1 - epsilon)
            self.assertLess(sd_test[i].velocity_properties.density / points[i][5], 1 + epsilon)

        self._test_end()
