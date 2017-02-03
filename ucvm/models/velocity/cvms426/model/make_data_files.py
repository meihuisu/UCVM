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
model = "CVM-S4.26"
dimension_x = 1536
dimension_y = 992
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
    username = ""
    path = "/home/scec-01/enjuilee/work/CVM4SI26_ascii"

    # Get the iteration number.
    try:
        opts, args = getopt.getopt(sys.argv[1:], "u:", ["user="])
    except getopt.GetoptError as err:
        print(str(err))
        usage()
        sys.exit(1)

    for o, a in opts:
        if o in ("-u", "--user"):
            username = str(a) + "@"

    print("\nDownloading model file\n")

    subprocess.check_call(
        ["scp", username + "intensity.usc.edu:" + path + "/CVM4SI26_model", "."]
    )

    # Now we need to go through the data files and put them in the correct
    # format for CCA (i.e. the tables format).
    print("\nWriting out CVM-S4.26 data file\n")
    f = open("./CVM4SI26_model")

    vp_arr = np.zeros((dimension_z, dimension_x, dimension_y), dtype="<f4")
    vs_arr = np.zeros((dimension_z, dimension_x, dimension_y), dtype="<f4")
    density_arr = np.zeros((dimension_z, dimension_x, dimension_y), dtype="<f4")

    for line in f:
        arr = line.split()
        x_pos = dimension_x - int(arr[1])
        y_pos = int(arr[0]) - 1
        z_pos = int(arr[2]) - 1
        vp = float(arr[3])
        vs = float(arr[4])
        density = float(arr[5])

        vp_arr[z_pos][x_pos][y_pos] = vp
        vs_arr[z_pos][x_pos][y_pos] = vs
        density_arr[z_pos][x_pos][y_pos] = density

    f.close()

    # Save the data.
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
        "data", (density_arr.shape[0], density_arr.shape[1], density_arr.shape[2]), density_arr.dtype, chunks=True,
        data=density_arr, compression="lzf"
    )

    out_file.close()

    os.remove("./CVM4SI26_model")

    print("\nDone!")

    return 0

if __name__ == "__main__":
    sys.exit(main())
