"""
Defines the tests for the CCA model within UCVM.

Copyright:
    Southern California Earthquake Center

Developer:
    David Gill <davidgil@usc.edu>
"""
# UCVM Imports
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
