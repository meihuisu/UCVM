#!/usr/bin/env python
"""
Generate Data File for CCA Model

Gets the source data files for the CCA model from intensity.usc.edu and generates the HDF5 files
using the source ASCII data.

Copyright:
    Southern California Earthquake Center

Developer:
    David Gill <davidgil@usc.edu>
"""
# Python Imports
import getopt
import sys
import os
import subprocess

# Package Imports
import h5py
import numpy as np

# Globals
model = "CCA"
dimension_x = 1024
dimension_y = 896
dimension_z = 100


def usage() -> None:
    """
    Lets users know how to run this script.
    Returns:
         Nothing - this code just prints the help information.
    """
    print("\n./make_data_files.py -i [iteration number]\n\n")
    print("-i - The iteration number to retrieve from intensity.\n\n")


def main() -> int:
    """
    Runs the main method for our program. Returns 0 on success.

    Returns:
        0 if successful.
    """

    # Set our variable defaults.
    iteration = -1
    username = ""
    path = "/home/scec-01/enjuilee/work/" + model + "_ASCII"

    # Get the iteration number.
    try:
        opts, args = getopt.getopt(sys.argv[1:], "u:i:", ["user=", "iteration="])
    except getopt.GetoptError as err:
        print(str(err))
        usage()
        sys.exit(1)

    for o, a in opts:
        if o in ("-i", "--iteration"):
            iteration = str(a)
        if o in ("-u", "--user"):
            username = str(a) + "@"

    # If the iteration number was not provided, display the usage.
    if iteration == -1:
        usage()
        sys.exit(0)

    print("\nDownloading model file\n")

    subprocess.check_call(
        ["scp", username + "intensity.usc.edu:" + path + "/" + model + iteration.zfill(2) +
         ".ascii", "."]
    )

    # Now we need to go through the data files and put them in the correct
    # format for CCA (i.e. the tables format).
    print("\nWriting out CCA data file\n")
    f = open("./" + model + iteration.zfill(2) + ".ascii")

    vp_arr = np.zeros((dimension_x, dimension_y, dimension_z), dtype="<f8")
    vs_arr = np.zeros((dimension_x, dimension_y, dimension_z), dtype="<f8")
    density_arr = np.zeros((dimension_x, dimension_y, dimension_z), dtype="<f8")

    for line in f:
        arr = line.split()
        x_pos = int(arr[0]) - 1
        y_pos = int(arr[1]) - 1
        z_pos = int(arr[2]) - 1
        vp = float(arr[3])
        vs = float(arr[4])
        density = float(arr[5])

        vp_arr[x_pos][y_pos][z_pos] = vp
        vs_arr[x_pos][y_pos][z_pos] = vs
        density_arr[x_pos][y_pos][z_pos] = density

    f.close()

    # Save the data.
    out_file = h5py.File("cca06.dat", mode="w")
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
        "data", (density_arr.shape[0], density_arr.shape[1], density_arr.shape[2]), density_arr.dtype,
        chunks=True, data=density_arr, compression="lzf"
    )

    out_file.close()

    os.remove("./" + model + iteration.zfill(2) + ".ascii")

    print("\nDone!")

    return 0

if __name__ == "__main__":
    sys.exit(main())
