"""
Identifies the minimum Vp, Vs, and density (Dn) in one of EL's standard model text files.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   July 13, 2016
:modified:  July 13, 2016
"""
import getopt
import sys


def main():
    """
    The main function for our program.
    :return: Zero when successful.
    """
    minimums = {
        "vp": {"value": 99999, "x": -1, "y": -1, "z": -1, "position": 3},
        "vs": {"value": 99999, "x": -1, "y": -1, "z": -1, "position": 4},
        "dn": {"value": 99999, "x": -1, "y": -1, "z": -1, "position": 5}
    }

    try:
        opts, _ = getopt.getopt(sys.argv[1:], "f:", ["file="])
    except getopt.GetoptError as _:
        raise ValueError("Invalid options selected.")

    file_location = None

    for option, argument in opts:
        if option in ("-f", "--file"):
            file_location = argument

    if file_location is None:
        print("ERROR: The location of the text file to search must be provided with the -f " +
              "parameter.")
        return -1

    with open(file_location, "r") as iteration_file:
        for line in iteration_file:
            components = line.split()
            for _, value in minimums.items():
                if float(components[value["position"]]) < value["value"]:
                    value["value"] = float(components[value["position"]])
                    value["x"] = components[0]
                    value["y"] = components[1]
                    value["z"] = components[2]

    for key, value in minimums.items():
        print("Minimum %s occurs at (%d, %d, %d) and is %.02f." %
              (key.capitalize(), int(value["x"]), int(value["y"]), int(value["z"]),
               float(value["value"])))

    return 0

if __name__ == "__main__":
    main()
