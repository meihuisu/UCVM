#!/bin/env python
"""
Checks PC and EL's original data set to verify that the ratios roughly match what we get from UCVM.
If they do, then we can be reasonably certain that UCVM is delivering the model correctly. If not,
then we have to check UCVM. There is no dependency on UCVM in this code.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   August 30, 2016
:modified:  August 30, 2016
"""
import sys
import os
import numpy as np
import pickle

DATA_FILE = "CCA06.ascii"

DIMENSIONS = {
    "x": 1024,
    "y": 896,
    "z": 100
}

VP = 0
VS = 1

THRESHOLD = 1.6

PICKLE_LOCATION = "loadedccadata.dat"


def main() -> int:
    print("Reading in data...")

    if os.path.exists(PICKLE_LOCATION):
        with open(PICKLE_LOCATION, "rb") as data_file:
            data_memory = pickle.load(data_file)
    else:
        data_memory = np.empty((DIMENSIONS["x"], DIMENSIONS["y"], DIMENSIONS["z"], 2))
        with open(DATA_FILE, "r") as original_file:
            for line in original_file:
                line_split = line.split()
                data_memory[int(line_split[0]) - 1, int(line_split[1]) - 1,
                            int(line_split[2]) - 1, VP] = float(line_split[3])
                data_memory[int(line_split[0]) - 1, int(line_split[1]) - 1,
                            int(line_split[2]) - 1, VS] = float(line_split[4])
        with open(PICKLE_LOCATION, "wb") as data_file:
            pickle.dump(data_memory, data_file)

    print("Finished reading in data...")

    # Now that we have the dataset loaded in memory, we need to compute the ratio over each slice
    # and output it.
    total_percent_below = 0
    max_ratio_model = 0
    min_ratio_model = 9999

    for k in range(DIMENSIONS["z"] - 1, 0, -1):
        highest_for_slice = 0
        lowest_for_slice = 9999
        percent_below = 0
        for j in range(DIMENSIONS["y"]):
            for i in range(DIMENSIONS["x"]):
                ratio = data_memory[i, j, k, VP] / data_memory[i, j, k, VS]
                if ratio < THRESHOLD:
                    percent_below += 1
                if ratio > highest_for_slice:
                    highest_for_slice = ratio
                if ratio < lowest_for_slice:
                    lowest_for_slice = ratio
        percent_below /= DIMENSIONS["x"] * DIMENSIONS["y"]
        percent_below *= 100
        if highest_for_slice > max_ratio_model:
            max_ratio_model = highest_for_slice
        if lowest_for_slice < min_ratio_model:
            min_ratio_model = lowest_for_slice
        total_percent_below += percent_below
        print("%2.3f min, %2.3f max, %% below ratio %2.2f for depth %dm" %
              (lowest_for_slice, highest_for_slice, percent_below, (DIMENSIONS["z"] - k - 1) * 500))

    total_percent_below /= DIMENSIONS["z"]

    print("\n----------")
    print("MODEL DATA")
    print("----------")
    print("%2.3f min, %2.3f max, %% below ratio %2.2f" %
          (min_ratio_model, max_ratio_model, total_percent_below))

if __name__ == "__main__":
    sys.exit(main())
