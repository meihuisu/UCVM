#!/usr/bin/env python3
"""
Tests that the actual UCVM command line utilities work as they should.

Even though the functionality behind the command-line utilities may work, there can sometimes be bugs in the programs
themselves. This ensures that the command-line utilities behave as they should under real-world usages.

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
# Python Imports
import os
import sys
import inspect
import unittest
import sqlite3

from subprocess import Popen, PIPE, CalledProcessError

# UCVM Imports
from ucvm.src.framework.ucvm import UCVM
import ucvm.tests


class UCVMCommandsTest(unittest.TestCase):

    dir = os.path.dirname(inspect.getfile(ucvm.tests))

    def setUp(self):
        self.conn = sqlite3.connect(os.path.join(os.path.dirname(__file__), "data", "commands.db"))

    def _find_tests_for(self, command):
        test_cases = {}

        for row in self.conn.execute("SELECT * FROM Command WHERE Command=?", (command,)):
            for row_tc in self.conn.execute("SELECT * FROM TestCase WHERE CommandID=?", (row[0],)):
                test_cases[row_tc[0]] = {
                    "name": row_tc[2],
                    "parameters": row_tc[3],
                    "input": row_tc[4],
                    "stdout": row_tc[5],
                    "stderr": row_tc[6],
                    "requiresmodel": row_tc[7]
                }

        return test_cases

    def _run_and_verify_commands(self, command, test_cases):
        for _, case_data in test_cases.items():
            if case_data["requiresmodel"] != "":
                if not UCVM.is_model_installed(case_data["requiresmodel"]):
                    print("\tModel " + case_data["requiresmodel"] + " is not installed. Skipping test.",
                          file=sys.stderr)
                    continue
            print("\tRunning test for " + case_data["name"] + ".", file=sys.stderr)
            p = Popen(["python3.5", "./ucvm/bin/" + command] + case_data["parameters"].split(),
                      stdout=PIPE, stdin=PIPE, stderr=PIPE)
            p.stdin.write(str(case_data["input"]).encode("UTF-8"))
            streams = p.communicate()
            str_out = streams[0].decode("utf-8")
            str_err = streams[1].decode("utf-8")
            self.assertTrue(str(case_data["stdout"]) == str(str_out))
            if case_data["stderr"] != "":
                self.assertIn(case_data["stderr"], str_err)
            else:
                self.assertEqual(case_data["stderr"], str_err)

    def test_ucvm_query(self):
        print("", file=sys.stderr)
        self._run_and_verify_commands("ucvm_query", self._find_tests_for("ucvm_query"))

    def test_ucvm_plot_horizontal_slice(self):
        print("", file=sys.stderr)
        self._run_and_verify_commands("ucvm_plot_horizontal_slice", self._find_tests_for("ucvm_plot_horizontal_slice"))

    def test_ucvm_plot_cross_section(self):
        print("", file=sys.stderr)
        self._run_and_verify_commands("ucvm_plot_cross_section", self._find_tests_for("ucvm_plot_cross_section"))

    def test_ucvm_plot_depth_profile(self):
        print("", file=sys.stderr)
        self._run_and_verify_commands("ucvm_plot_depth_profile", self._find_tests_for("ucvm_plot_depth_profile"))

    def test_ucvm_plot_comparison(self):
        print("", file=sys.stderr)
        self._run_and_verify_commands("ucvm_plot_comparison", self._find_tests_for("ucvm_plot_comparison"))


def make_suite() -> unittest.TestSuite:
    suite = unittest.TestSuite()
    suite.addTest(unittest.makeSuite(UCVMCommandsTest, "test"))
    return suite

if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(make_suite())