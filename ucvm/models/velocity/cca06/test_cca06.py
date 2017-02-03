"""
Defines the tests for the CCA model within UCVM.

Copyright:
    Southern California Earthquake Center

Developer:
    David Gill <davidgil@usc.edu>
"""
# UCVM Imports
from ucvm.src.framework.ucvm import UCVM
from ucvm.src.shared.properties import SeismicData, Point
from ucvm.src.shared.test import run_acceptance_test, UCVMTestCase


class CCA06VelocityModelTest(UCVMTestCase):
    """
    Defines the test cases for the CCA06 velocity model. It just runs an acceptance test.
    """
    description = "CCA06"

    def test_cca06_acceptance(self):
        """
        Runs the built-in acceptance test for the CCA06 velocity model. This compares a known
        grid of lat, lon material properties - queried at depth - to what this installation of the
        CCA06 velocity model returns on the user's computer.

        Returns:
            None
        """
        self._test_start("CCA06 acceptance test")
        self.assertTrue(run_acceptance_test(self, "cca06"))
        self._test_end()

    def test_cca06_random_points(self):
        """
        Tests the CCA06 velocity model at 10 random points selected from the model. These points were picked with
        a random number generator.

        Returns:
            None
        """
        self._test_start("CCA06 points versus text model test")

        points = [
            (-119.49640, 38.32592, 15500, 5959.643, 3353.884, 2650.539),
            (-120.09714, 37.10320, 35500, 7854.391, 4519.072, 3246.978),
            (-120.55020, 35.44320, 47000, 8120.136, 4751.165, 3356.421),
            (-118.93577, 38.21206, 23500, 6277.583, 3554.756, 2734.685),
            (-118.00735, 37.96094, 0, 4007.691, 2712.415, 2442.030),
            (-119.44246, 35.94601, 15000, 6852.613, 4022.224, 2988.574),
            (-120.47650, 35.00007, 21500, 6893.294, 3984.993, 2944.352),
            (-119.10807, 35.01673, 23000, 7134.716, 4122.945, 3017.147),
            (-119.48761, 35.14042, 19000, 6471.395, 3805.464, 2867.885),
            (-120.11491, 35.41599, 500, 4076.398, 2076.811, 2359.239)
        ]

        sd_test = [SeismicData(Point(p[0], p[1], p[2])) for p in points]
        self.assertTrue(UCVM.query(sd_test, "cca06", ["velocity"]))

        epsilon = 0.01
        for i in range(len(sd_test)):
            self.assertGreater(sd_test[i].velocity_properties.vp / points[i][3], 1 - epsilon)
            self.assertLess(sd_test[i].velocity_properties.vp / points[i][3], 1 + epsilon)
            self.assertGreater(sd_test[i].velocity_properties.vs / points[i][4], 1 - epsilon)
            self.assertLess(sd_test[i].velocity_properties.vs / points[i][4], 1 + epsilon)
            self.assertGreater(sd_test[i].velocity_properties.density / points[i][5], 1 - epsilon)
            self.assertLess(sd_test[i].velocity_properties.density / points[i][5], 1 + epsilon)

        self._test_end()
