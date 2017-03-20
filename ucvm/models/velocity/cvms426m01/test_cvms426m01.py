"""
Defines the tests for the CVM-S4.26.M01 model within UCVM.

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
