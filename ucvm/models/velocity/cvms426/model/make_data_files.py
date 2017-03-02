#!/usr/bin/env python
"""
Generate Data File for CVM-S4.26 Model

Given one of the new format CVM-S4.26 files (i.e. the one generated from _ucvm_calculate_box_for_inversion), read in
this file and generate the data files for the gridded velocity model. This also generates the acceptance test data
which consists of calculating the center of each gridded cell and outputting the interpolated material properties.

Copyright:
    Southern California Earthquake Center

Developer:
    David Gill <davidgil@usc.edu>
"""
# Python Imports
import sys
from random import randint

# Package Imports
import h5py
import numpy as np

# Globals
dimension_x = 992
dimension_y = 1536
dimension_z = 100


def main() -> int:
    """
    Runs the main method for our program. Returns 0 on success.

    Returns:
        0 if successful.
    """

    # Set our variable defaults.
    data_file = "./cvms426_final_model.txt"

    print("Creating arrays.")
    vp_arr = np.zeros((dimension_z, dimension_y, dimension_x), dtype="<f4")
    vs_arr = np.zeros((dimension_z, dimension_y, dimension_x), dtype="<f4")
    dn_arr = np.zeros((dimension_z, dimension_y, dimension_x), dtype="<f4")

    # For point data test.
    points = []
    point_data = []
    for i in range(0, 20):
        x = randint(0, dimension_x - 1)
        y = randint(0, dimension_y - 1)
        z = randint(0, dimension_z - 1)

        points.append((x, y, z))

    # Read in the velocity model information.
    print("Reading velocity model data.")
    with open(data_file, "r") as f:
        f.readline()
        for line in f:
            arr = line.split()
            x_pos = int(arr[0]) - 1
            y_pos = int(arr[1]) - 1
            z_pos = int(arr[2]) - 1
            vp = float(arr[8])
            vs = float(arr[9])
            dn = float(arr[10])

            vp_arr[z_pos][y_pos][x_pos] = vp
            vs_arr[z_pos][y_pos][x_pos] = vs
            dn_arr[z_pos][y_pos][x_pos] = dn

            for point in points:
                if point[0] == x_pos and point[1] == y_pos and point[2] == z_pos:
                    point_data.append((x_pos + 1, y_pos + 1, z_pos + 1, float(arr[5]), float(arr[6]), vp, vs, dn))

    f.close()

    # Save the data.
    print("Saving to HDF5 format.")
    out_file = h5py.File("cvms426.dat", mode="w")
    grp_vp = out_file.create_group("vp")
    grp_vs = out_file.create_group("vs")
    grp_dn = out_file.create_group("dn")

    grp_vp.create_dataset(
        "data", (vp_arr.shape[0], vp_arr.shape[1], vp_arr.shape[2]), vp_arr.dtype, chunks=True, data=vp_arr,
        compression="lzf"
    )
    grp_vs.create_dataset(
        "data", (vs_arr.shape[0], vs_arr.shape[1], vs_arr.shape[2]), vs_arr.dtype, chunks=True, data=vs_arr,
        compression="lzf"
    )
    grp_dn.create_dataset(
        "data", (dn_arr.shape[0], dn_arr.shape[1], dn_arr.shape[2]), dn_arr.dtype, chunks=True, data=dn_arr,
        compression="lzf"
    )

    out_file.close()

    # Create acceptance tests.
    print("Creating acceptance test and point data test.")

    division_factor = 4
    test_array = np.zeros(
        (int(dimension_z / division_factor) - 1, int(dimension_y / division_factor) - 1,
         int(dimension_x / division_factor) - 1, 3)
    )
    for z in range(int(dimension_z / division_factor) - 1):
        for y in range(int(dimension_y / division_factor) - 1):
            for x in range(int(dimension_x / division_factor) - 1):
                zt = z * division_factor
                yt = y * division_factor
                xt = x * division_factor
                test_array[z][y][x][0] = \
                    (vp_arr[zt][yt][xt] + vp_arr[zt][yt][xt + 1] + vp_arr[zt][yt + 1][xt] + vp_arr[zt][yt + 1][xt + 1] +
                     vp_arr[zt + 1][yt][xt] + vp_arr[zt + 1][yt][xt + 1] + vp_arr[zt + 1][yt + 1][xt] +
                     vp_arr[zt + 1][y + 1][x + 1]) / 8
                test_array[z][y][x][1] = \
                    (vs_arr[zt][yt][xt] + vs_arr[zt][yt][xt + 1] + vs_arr[zt][yt + 1][xt] + vs_arr[zt][yt + 1][xt + 1] +
                     vs_arr[zt + 1][yt][xt] + vs_arr[zt + 1][yt][xt + 1] + vs_arr[zt + 1][yt + 1][xt] +
                     vs_arr[zt + 1][yt + 1][xt + 1]) / 8
                test_array[z][y][x][2] = \
                    (dn_arr[zt][yt][xt] + dn_arr[zt][yt][xt + 1] + dn_arr[zt][yt + 1][xt] + dn_arr[zt][yt + 1][xt + 1] +
                     dn_arr[zt + 1][yt][xt] + dn_arr[zt + 1][yt][xt + 1] + dn_arr[zt + 1][yt + 1][xt] +
                     dn_arr[zt + 1][yt + 1][xt + 1]) / 8
    np.save("test_cvms426", test_array)

    print("\nPoints for testing:")
    for point in point_data:
        print("%-8d%-8d%-8d%-14.6f%-12.6f%-10.3f%-10.3f%-10.3f" % point)

    print("\nDone!")

    return 0

if __name__ == "__main__":
    sys.exit(main())
