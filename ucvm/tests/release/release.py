#!/usr/bin/env python3
"""
Executes all the commands in the SQLite testsuite.db file located within the ucvm/tests folder. This also creates
the RST file which we then embed and include within the documentation to show that we have, indeed, tested for all
of these cases.

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
import sqlite3
import sys

from collections import OrderedDict
from subprocess import Popen, PIPE


def execute_command(command: str, stdinput: str) -> (str, str):
    """
    Given a command and input, run the command and return back the header row and the material properties rows. Do not
    return back all the extra "press enter" text shown before the input.

    Parameters:
        command (str): The full command, including parameters, to run (e.g. ucvm_query -m cvms4).
        stdinput (str): The input, as a string, to pass to the command.

    Returns:
        tuple: A tuple of two strings containing the relevant output from the command on both standard out and standard
        error.
    """
    process = Popen(command.split(), stdout=PIPE, stdin=PIPE, stderr=PIPE)
    process.stdin.write(stdinput.encode("UTF-8"))
    streams = process.communicate()
    str_out = streams[0].decode("utf-8")
    str_err = streams[1].decode("utf-8")

    if "ucvm_query" in command:
        return (command + "\n" + "\n".join(str_out.strip().split("\n")[3:])) if str_out.strip() != "" else "", \
               (command + "\n" + str_err.strip()) if str_err.strip() != "" else ""
    else:
        return str_out, str_err


def compare_against_known_data(command_output: tuple, known_output: tuple) -> bool:
    """
    Returns true if the output of the execute command function (i.e. the standard output and error tuple) is the same
    as the known output tuple. This parses the first line of the output and error columns to see if we're checking for
    equality, contains, or some other comparison.

    Parameters:
        command_output (tuple): The output of the execute_command function.
        known_output (tuple): The known output as retrieved from the database.

    Returns:
        bool: True if the comparison against known data is successful. False if not.
    """
    # Compare standard out first. Get the comparison method, then compare.
    stdout_compare_method = known_output[0].split("\n")[0].strip()
    stdout = "\n".join(known_output[0].split("\n")[1:])
    if stdout_compare_method == "Equal":
        if stdout.strip() != command_output[0].strip():
            print(command_output[0])
            return False

    # Compare standard error second. Get comparison method, then compare.
    stderr_compare_method = known_output[1].split("\n")[0].strip()
    stderr = "\n".join(known_output[1].split("\n")[1:])
    if stderr_compare_method == "Equal":
        if stderr.strip() != command_output[1].strip():
            return False
    elif stderr_compare_method == "Contains":
        if stderr.strip() not in command_output[1].strip():
            print(command_output[1])
            return False

    return True


def write_rst_doc(test_results: dict) -> bool:
    """
    Writes out the RST test result documentation that is available either on hypocenter.usc.edu or on GitHub.

    Parameters:
        test_results (dict): A dictionary containing the test results. The format of this dictionary can be found by
            looking at the code.

    Returns:
        bool: True if successful. Raises an error if not.
    """
    # Open the RST file and begin writing.
    with open("tests_release_verification_include.rst", "w") as output_file:
        # Loop through all the categories.
        for category, values in test_results.items():
            output_file.write("%s\n" % category)
            output_file.write(("{:-<" + str(len(category)) + "}\n\n").format("-"))
            output_file.write("%s\n\n" % values["description"])

            # Now loop through the tests.
            for test in values["tests"]:
                output_file.write("%s\n" % test["name"])
                output_file.write(("{:~<" + str(len(test["name"])) + "}\n\n").format("~"))
                output_file.write("%s\n\n" % test["description"])
                output_file.write("Result: %s\n\n" % ("PASSED" if test["result"] is True else "**FAILED**"))

                # Print out the models that we checked this test against.
                output_file.write("Models Checked:\n")
                output_file.write("::\n\n")

                models = [x.strip() for x in str(test["required_models"]).split(",")]
                for model in models:
                    output_file.write("    %s\n" % model)
                output_file.write("\n")

                if "[models]" in test["command"]:
                    # Print out the precise commands (plural) that were tested.
                    output_file.write("Commands Tested:\n")
                    output_file.write("::\n\n")

                    for model in models:
                        output_file.write("    %s\n" % test["command"].replace("[models]", model))
                    output_file.write("\n")
                elif len(test["command"].split("\n")) > 1:
                    # Print out the precise commands (plural) that were tested.
                    output_file.write("Commands Tested:\n")
                    output_file.write("::\n\n")

                    for command in test["command"].split("\n"):
                        output_file.write("    %s\n" % command.strip())
                    output_file.write("\n")
                else:
                    # Print out the precise command that were tested.
                    output_file.write("Command Tested:\n")
                    output_file.write("::\n\n")
                    output_file.write("    %s\n" % test["command"])
                    output_file.write("\n")

                if test["input"].strip() != "":
                    # Print out the uniform input that was provided to each command.
                    output_file.write("Input:\n")
                    output_file.write("::\n\n")
                    for line in test["input"].strip().split("\n"):
                        output_file.write("    %s\n" % line)
                    output_file.write("\n")

                # Print out the standard out that the commands produced, if there was one.
                if test["result_out"] != "":
                    if "ucvm_query" in test["command"]:
                        output_file.write("Actual Output:\n")
                        output_file.write("::\n\n")
                        for line in test["result_out"].strip().split("\n"):
                            output_file.write("    %s\n" % line)
                        output_file.write("\n")
                    else:
                        output_file.write("Plots:\n\n")
                        for plot in str(test["command"]).split("\n"):
                            output_file.write(".. image:: _static/" + plot.split("/")[1].replace(".xml", ".png") + "\n")
                            output_file.write("    :width: 500px\n\n")

                # Print out the standard error that the commands produced, if there was one.
                if test["result_err"] != "" and "ucvm_plot_horizontal_slice" not in test["command"]:
                    output_file.write("Actual Error:\n")
                    output_file.write("::\n\n")
                    for line in test["result_err"].strip().split("\n"):
                        output_file.write("    %s\n" % line)
                    output_file.write("\n")

    # Close the file.
    output_file.close()

    # All done, return true.
    return True


def main() -> int:
    """
    Executes all the commands within the testsuite.db file and generates the corresponding RST document detailing
    what the results of the commands were, if they passed, and so on.

    Returns:
        int: 0 if successful. Raises an error if not.
    """
    # Open the SQLite connection.
    conn = sqlite3.connect(os.path.join(os.path.dirname(__file__), "testsuite.db"))

    # Read through the categories. These are sorted by ID, so we don't need to re-sort.
    categories = conn.execute("SELECT * FROM Category ORDER BY ID ASC")

    # Create the RST dictionary which will hold the results of all the tests in memory before writing them to
    # a RST file to be included within the documentation.
    rst_test_dict = OrderedDict()

    for category in categories:
        print("Testing " + category[1])
        # Add category to RST dictionary.
        rst_test_dict[category[1]] = {
            "description": category[2],
            "tests": []
        }
        tests = conn.execute("SELECT * FROM TestCase WHERE `Category ID`= ? ORDER BY ID ASC", (category[0],))
        for test in tests:
            # Create entry for this particular test. This is then inserted into the tests array.
            rst_test_entry = {
                "name": str(test[2]).title(),
                "description": str(test[3]),
                "command": str(test[4]),
                "required_models": str(test[8]),
                "input": str(test[5]),
                "result_out": "",
                "result_err": "",
                "result": None
            }

            # Check to see if we need to run this command for multiple models. If we do, then we run it for each
            # model and concatenate the output. Otherwise, we just run it for the one.
            if len(str(test[8]).split(",")) > 1 and "[models]" in test[4]:
                models = [x.strip() for x in str(test[8]).split(",")]
                current_command = 1
                for model in models:
                    print("\r\t%-70stest %d of %d" % (str(test[2]).title(), current_command, len(models)), end="")
                    data = execute_command(test[4].replace("[models]", model), test[5])
                    rst_test_entry["result_out"] += str(data[0]) + "\n\n"
                    rst_test_entry["result_err"] += str(data[1]) + "\n\n"
                    current_command += 1
            else:
                if len(str(test[4]).split("\n")) > 1:
                    commands = str(test[4]).split("\n")
                    current_command = 1
                    for command in commands:
                        print("\r\t%-70stest %d of %d" % (str(test[2]).title(), current_command, len(commands)), end="")
                        data = execute_command(command, test[5])
                        if "ucvm_plot_" in command:
                            rst_test_entry["result_out"] += command.split()[2] + "\n"
                            rst_test_entry["result_err"] += str(data[1]) + "\n\n"
                        else:
                            rst_test_entry["result_out"] += str(data[0]) + "\n\n"
                            rst_test_entry["result_err"] += str(data[1]) + "\n\n"
                        current_command += 1
                else:
                    print("\r\t%-70stest %d of %d" % (str(test[2]).title(), 1, 1), end="")
                    (rst_test_entry["result_out"], rst_test_entry["result_err"]) = execute_command(test[4], test[5])

            # Strip extra characters.
            rst_test_entry["result_out"] = rst_test_entry["result_out"].strip()
            rst_test_entry["result_err"] = rst_test_entry["result_err"].strip()

            # Compare against known data.
            rst_test_entry["result"] = compare_against_known_data(
                (rst_test_entry["result_out"], rst_test_entry["result_err"]),
                (test[6], test[7])
            )

            # Print success if result is true.
            print("\r\t%-70s" % str(test[2]).title(), end="")
            print("[SUCCESS]" if rst_test_entry["result"] is True else "[FAIL]")

            # Add test to relevant tests key in dictionary.
            rst_test_dict[category[1]]["tests"].append(rst_test_entry)

    # Generate the RST content for the relevant tests page either with the GitHub documentation or the more extensive
    # documentation on hypocenter.usc.edu.
    write_rst_doc(rst_test_dict)

    return 0

if __name__ == "__main__":
    sys.exit(main())
