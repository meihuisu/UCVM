#!/usr/bin/env python
"""
Gets the source data files from intensity.usc.edu and generates the floating point data files from
the source ASCII data.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   August 1, 2015
:modified:  August 31, 2016
"""

import getopt
import sys
import os
import subprocess

import numpy as np

model = "CVM-S4.26"
dimension_x = 992
dimension_y = 1536
dimension_z = 100


def usage() -> None:
    """
    Lets users know how to run this script.
    :return: Nothing - this code just prints the help information.
    """
    print("\n./make_data_files.py -i [iteration number]\n\n")
    print("-i - The iteration number to retrieve from intensity.\n\n")


def main() -> int:
    """
    Runs the main method for our program. Returns 0 on success.
    :return: 0 if successful.
    """

    # Set our variable defaults.
    iteration = -1
    username = ""
    path = "/home/scec-01/enjuilee/work/CVM4SI26_ascii"

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
        ["scp", username + "intensity.usc.edu:" + path + "/CVM4SI" + iteration.zfill(2) +
         "_model", "."]
    )

    # Now we need to go through the data files and put them in the correct
    # format for CCA. More specifically, we need a Vp.dat, Vs.dat, and Density.dat

    subprocess.check_call(["mkdir", "-p", "./i" + iteration.zfill(2)])

    print("\nWriting out CVM-S4.26 data files\n")

    f = open("./CVM4SI" + iteration.zfill(2) + "_model")

    f_vp = open("./i" + iteration.zfill(2) + "/vp.dat", "wb")
    f_vs = open("./i" + iteration.zfill(2) + "/vs.dat", "wb")
    f_density = open("./i" + iteration.zfill(2) + "/density.dat", "wb")

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

        vp_arr[x_pos, y_pos, z_pos] = vp
        vs_arr[x_pos, y_pos, z_pos] = vs
        density_arr[x_pos, y_pos, z_pos] = density

    np.save(f_vp, vp_arr)
    np.save(f_vs, vs_arr)
    np.save(f_density, density_arr)

    f.close()
    f_vp.close()
    f_vs.close()
    f_density.close()

    os.remove("./CVM4SI" + iteration.zfill(2) + "_model")

    print("\nDone!")

    return 0

if __name__ == "__main__":
    sys.exit(main())
