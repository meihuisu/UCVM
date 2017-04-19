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
import multiprocessing
import psutil
import sqlite3
import sys

from subprocess import Popen, PIPE

from ucvm.src.framework.ucvm import UCVM


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
    command_parts = command.split()

    process = Popen(command_parts, stdout=PIPE, stdin=PIPE, stderr=PIPE)
    process.stdin.write(stdinput.encode("UTF-8"))
    streams = process.communicate()
    str_out = streams[0].decode("utf-8")
    str_err = streams[1].decode("utf-8")

    if "ucvm_query" in command or "ucvm_mesh" in command or "ucvm_etree" in command:
        return (command + "\n" + "\n".join(str_out.strip().split("\n")[3:])) if str_out.strip() != "" else "", \
               (command + "\n" + str_err.strip()) if str_err.strip() != "" else ""
    elif "diff" in command:
        if str_out == "":
            return "No difference between " + command_parts[1] + " and " + command_parts[2] + " found.", ""
        else:
            return "", "Difference found between " + command_parts[1] + " and " + command_parts[2] + "."
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
    elif stdout_compare_method == "Contains":
        if stdout.strip() not in command_output[0].strip():
            print(command_output[0])
            return False

    # Compare standard error second. Get comparison method, then compare.
    stderr_compare_method = known_output[1].split("\n")[0].strip()
    stderr = "\n".join(known_output[1].split("\n")[1:])
    if stderr_compare_method == "Equal":
        if stderr.strip() != command_output[1].strip():
            print(command_output[1])
            return False
    elif stderr_compare_method == "Contains":
        if stderr.strip() not in command_output[1].strip():
            print(command_output[1])
            return False

    return True


def write_rst_doc(test_results: list) -> bool:
    """
    Writes out the RST test result documentation that is available either on hypocenter.usc.edu or on GitHub.

    Parameters:
        test_results (list): A sorted list containing the test results. The format of the list's entries can be found by
            looking at the code.

    Returns:
        bool: True if successful. Raises an error if not.
    """
    # Open the RST file and begin writing.
    with open("tests_release_verification_include.rst", "w") as output_file:
        # Loop through all the categories.
        current_group = ""
        for test in test_results:

            if test["category"]["group"] != current_group:
                output_file.write("%s\n" % test["category"]["group"])
                output_file.write(("{:-<" + str(len(test["category"]["group"])) + "}\n\n").format("-"))
                output_file.write("%s\n\n" % test["category"]["description"])
                current_group = test["category"]["group"]

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
                if "ucvm_plot" not in test["command"]:
                    output_file.write("Actual Output:\n")
                    output_file.write("::\n\n")
                    for line in test["result_out"].strip().split("\n"):
                        output_file.write("    %s\n" % line)
                    output_file.write("\n")
                else:
                    output_file.write("Plots:\n\n")
                    for plot in str(test["command"]).split("\n"):
                        if "ucvm_plot" not in plot:
                            continue
                        if len(plot.split("/")) > 1:
                            output_file.write(".. image:: _static/" + plot.split("/")[1].replace(".xml", ".png") + "\n")
                            output_file.write("    :width: 500px\n\n")
                        else:
                            output_file.write(".. image:: _static/" + plot + "_cmd_check.png\n")
                            output_file.write("    :width: 500px\n\n")

            # Print out the standard error that the commands produced, if there was one.
            if test["result_err"] != "" and "ucvm_plot" not in test["command"]:
                output_file.write("Actual Error:\n")
                output_file.write("::\n\n")
                for line in test["result_err"].strip().split("\n"):
                    output_file.write("    %s\n" % line)
                output_file.write("\n")

    # All done, return true.
    return True


def mp_run(*args):
    """
    This function is called by multiprocessing.Pool to run the commands in parallel.

    Parameters:
        args (list): Only one argument is ever passed to this function and that is the array of test entries.

    Returns:
        dict: The revised test entry with the result keys filled in.
    """
    test_entry = dict(args[0])

    # Check to see if we need to run this command for multiple models. If we do, then we run it for each
    # model and concatenate the output. Otherwise, we just run it for the one.
    if len(str(test_entry["required_models"]).split(",")) > 1 and "[models]" in test_entry["command"]:
        models = [x.strip() for x in str(test_entry["required_models"]).split(",")]
        current_command = 1
        for model in models:
            print("%-30s%-70s%s" % (
                test_entry["category"]["group"], test_entry["name"], test_entry["command"].replace("[models]", model)
            ))
            sys.stdout.flush()
            data = execute_command(test_entry["command"].replace("[models]", model), test_entry["input"])
            test_entry["result_out"] += str(data[0]) + "\n\n"
            test_entry["result_err"] += str(data[1]) + "\n\n"
            current_command += 1
    else:
        if len(str(test_entry["command"]).split("\n")) > 1:
            commands = str(test_entry["command"]).split("\n")
            current_command = 1
            for command in commands:
                print("%-30s%-70s%s" % (
                    test_entry["category"]["group"], test_entry["name"], command
                ))
                sys.stdout.flush()
                data = execute_command(command, test_entry["input"])
                if "ucvm_plot_" in command:
                    test_entry["result_out"] += command.split()[2] + "\n"
                    test_entry["result_err"] += str(data[1]) + "\n\n"
                else:
                    test_entry["result_out"] += str(data[0]) + "\n\n"
                    test_entry["result_err"] += str(data[1]) + "\n\n"
                current_command += 1
        else:
            print("%-30s%-70s%s" % (
                test_entry["category"]["group"], test_entry["name"], test_entry["command"]
            ))
            sys.stdout.flush()
            (test_entry["result_out"], test_entry["result_err"]) = \
                execute_command(test_entry["command"], test_entry["input"])

    # Strip extra characters.
    test_entry["result_out"] = test_entry["result_out"].strip()
    test_entry["result_err"] = test_entry["result_err"].strip()

    # Compare against known data.
    test_entry["result"] = compare_against_known_data(
        (test_entry["result_out"], test_entry["result_err"]),
        (test_entry["known_out"], test_entry["known_err"])
    )

    return test_entry


def main() -> int:
    """
    Executes all the commands within the testsuite.db file and generates the corresponding RST document detailing
    what the results of the commands were, if they passed, and so on.

    Returns:
        int: 0 if successful. Raises an error if not.
    """
    # Remove everything in the scratch directory.
    for item in os.listdir("./scratch"):
        if ".awp" in item or ".rwg" in item or ".e" in item:
            execute_command("rm ./scratch/" + item, "")

    # Remove everything in the static directory.
    for item in os.listdir("./_static"):
        if ".png" in item:
            execute_command("rm ./_static/" + item, "")

    # Open the SQLite connection.
    conn = sqlite3.connect(os.path.join(os.path.dirname(__file__), "testsuite.db"))

    # Read through the categories. These are sorted by ID, so we don't need to re-sort.
    categories = conn.execute("SELECT * FROM Category ORDER BY ID ASC")

    # Create the RST dictionary which will hold the results of all the tests in memory before writing them to
    # a RST file to be included within the documentation.
    test_entries = []
    command_count = 0

    for category in categories:
        tests = conn.execute("SELECT * FROM TestCase WHERE `Category ID`= ? ORDER BY ID ASC", (category[0],))
        for test in tests:
            # if test[0] != 66:
            #     continue
            # Create entry for this particular test and append it to the test entries array.
            test_entries.append({
                "category_id": category[0],
                "id": test[0],
                "name": str(test[2]).title(),
                "description": str(test[3]),
                "command": str(test[4]),
                "required_models": str(test[8]),
                "input": str(test[5]),
                "result_out": "",
                "result_err": "",
                "known_out": str(test[6]),
                "known_err": str(test[7]),
                "result": None,
                "category": {
                    "group": category[1],
                    "description": category[2]
                }
            })

            if "[models]" in str(test[4]):
                command_count += len(str(test[8]).split(","))
            elif len(str(test[4]).split("\n")) > 1:
                command_count += len(str(test[4]).strip().split("\n"))
            else:
                command_count += 1

    # Number of cores to run on. This is set automatically.
    cores = int(round(min(
        multiprocessing.cpu_count(),
        psutil.virtual_memory().total / 3221225472
    ), 0))

    # Print our header statistics first.
    print("UCVM Release Tests")
    print("")
    print("Running %d tests consisting of %d commands on %d cores. Estimated time to complete is %d minutes.\n" % (
        len(test_entries), command_count, cores, len(test_entries) / cores * 10
    ))
    print("%-30s%-70s%s" % ("Test Category", "Test Title", "Command"))
    print("%-30s%-70s%s" % ("-------------", "----------", "-------"))

    # Append total number of commands
    for entry in test_entries:
        entry["total"] = command_count

    # Now we need to run all the tests using the multiprocessing pool. This helps speed up the test results as we can
    # run all the tests in parallel instead of requiring sequential tests.
    pool = multiprocessing.Pool(cores)
    results = pool.map(mp_run, test_entries)
    pool.close()
    pool.join()

    # Sort the array by id.
    results.sort(key=lambda x: (x["category_id"], x["id"]))

    # Generate the RST content for the relevant tests page either with the GitHub documentation or the more extensive
    # documentation on hypocenter.usc.edu.
    write_rst_doc(results)

    all_good = True
    for result in results:
        if not result["result"]:
            all_good = False

    print("")
    print("All tests passed!" if all_good else "One or more tests failed!")

    # Remove everything in the scratch directory.
    for item in os.listdir("./scratch"):
        if ".awp" in item or ".rwg" in item:
            execute_command("rm ./scratch/" + item, "")

    return 0

if __name__ == "__main__":
    sys.exit(main())
