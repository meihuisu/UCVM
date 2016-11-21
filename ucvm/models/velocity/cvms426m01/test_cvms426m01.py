"""
Defines the tests for the CVM-S4.26.M01 model within UCVM.

Copyright:
    Southern California Earthquake Center

Developer:
    David Gill <davidgil@usc.edu>
"""
# UCVM Imports
from ucvm.src.shared.test import run_acceptance_test, UCVMTestCase


class CVMS426M01VelocityModelTest(UCVMTestCase):
    """
    Defines the CVM-S4.26.M01 test cases.
    """
    description = "CVM-S4.26.M01"

    def test_cvms426_acceptance(self):
        """
        Runs the built-in acceptance test for CVM-S4.26.M01. This compares a known grid of lat, lon
        material properties - queried at depth - to what this installation of CVM-S4.26.M01 returns
        on the user's computer.

        Returns:
            None
        """
        self._test_start("CVM-S4.26.M01 acceptance test")
        self.assertTrue(run_acceptance_test(self, "cvms426m01"))
        self._test_end()
