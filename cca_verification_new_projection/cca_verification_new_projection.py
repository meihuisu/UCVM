"""
Uses the PyCVM library included within UCVM 14.3.0 and above to generate a grid to diff the old
CCA06 projection vs. the new CCA06 projection.

NOTE: This script must be run with the UCVM utilities folder as the working directory.

Copyright:
    Southern California Earthquake Center

Author:
    David Gill <davidgil@usc.edu>
"""
# Python Imports
import sys
import struct
import os
import pickle
from subprocess import Popen, PIPE, STDOUT

import numpy as np

#  Matplotlib is required.
try:
    import matplotlib.pyplot as plt
    import matplotlib.colors as mcolors
    import matplotlib.cm as cm
except StandardError, e:
    print "ERROR: NumPy and Matplotlib must be installed on your system in order to generate these plots."
    exit(1)

#  Basemap is required.
try:
    from mpl_toolkits import basemap
except StandardError, e:
    print "ERROR: Matplotlib Toolkit must be installed on your system in order to generate these plots."
    exit(1)

UCVM_LOCATION = "/Users/davidgil/ucvm-15.10.0"  #: str: Defines where UCVM has been installed.
DEPTHS = [0, 1000, 2000, 5000, 10000]


def frange(start, end, step):
    cur = start
    while cur < end:
        yield cur
        cur += step


def show_plots():
    lon_array = []
    lat_array = []

    for x in frange(-123, -115.4, 0.005):
        lon_array.append(x)
    for y in frange(33.3, 39.4, 0.005):
        lat_array.append(y)

    for z in DEPTHS:
        plt.figure(figsize=(10, 10), dpi=100)
        if os.path.exists("basemapdump.dat"):
            with open("basemapdump.dat", "rb") as bm_pickle:
                map_base = pickle.load(bm_pickle)
        else:
            map_base = basemap.Basemap(projection='cyl',
                                       llcrnrlat=33.3,
                                       urcrnrlat=39.4,
                                       llcrnrlon=-123,
                                       urcrnrlon=-115.4,
                                       resolution='f', anchor='C')

            with open("basemapdump.dat", "wb") as bm_pickle:
                pickle.dump(map_base, bm_pickle)

        map_base.drawparallels([33.3, 36.35, 39.4], linewidth=1.0, labels=[1, 0, 0, 0])
        map_base.drawmeridians([-123, -119.2, -115.4], linewidth=1.0, labels=[0, 0, 0, 1])

        map_base.drawstates()
        map_base.drawcountries()
        map_base.drawcoastlines()

        colormap = cm.RdBu
        norm = mcolors.Normalize(vmin=-10, vmax=10)

        old_values = np.arange(len(lon_array) * len(lat_array),dtype=float).reshape(len(lat_array), len(lon_array))
        new_values = np.arange(len(lon_array) * len(lat_array),dtype=float).reshape(len(lat_array), len(lon_array))
        percent_values = np.arange(len(lon_array) * len(lat_array),dtype=float).reshape(len(lat_array), len(lon_array))

        with open("/Users/davidgil/PycharmProjects/UCVM Scripts/cca_verification_new_projection/cca06_new_" + str(z) + ".dat", "rb") as fd_new, \
             open("/Users/davidgil/PycharmProjects/UCVM Scripts/cca_verification_new_projection/cca06_old_" + str(z) + ".dat", "rb") as fd_old:
            for x in range(len(lon_array)):
                for y in range(len(lat_array)):
                    print(x, y)
                    old_values[y][x] = struct.unpack(">f4", fd_old.read(4))[0]
                    new_values[y][x] = struct.unpack(">f4", fd_new.read(4))[0]
                    if old_values[y][x] != 0:
                        percent_values[y][x] = (new_values[y][x] / old_values[y][x] - 1) * 100
                    else:
                        percent_values[y][x] = 0

        lon_array = np.array(lon_array)
        lat_array = np.array(lat_array)

        # Add the title.
        plt.title("CCA Ratio At %dm" % z)
        t = map_base.transform_scalar(percent_values, lon_array, lat_array, len(lon_array), len(lat_array))
        img = map_base.imshow(t, norm=norm, cmap=colormap)

        cax = plt.axes([0.125, 0.05, 0.775, 0.02])
        cbar = plt.colorbar(img, cax=cax, orientation='horizontal', ticks=[-10, -5, 0, 5, 10])
        cbar.set_label("Difference (%)")

        plt.savefig("cca_ratio_" + str(z) + ".png")

        plt.close('all')

        print("Plot for depth " + str(z) + "saved.")


def gen_grid():
    for z in DEPTHS:

        proc = Popen([UCVM_LOCATION + "/bin/ucvm_query", "-f", UCVM_LOCATION + "/conf/ucvm.conf",
                      "-m", "cca"], stdout=PIPE, stdin=PIPE, stderr=STDOUT)

        text_points = ""

        for x in frange(-123, -115.4, 0.005):
            for y in frange(33.3, 39.4, 0.005):
                    text_points += "%.5f %.5f %.5f\n" % (x, y, z)

        text_points = text_points.encode("ASCII")

        output = bytes(proc.communicate(input=bytes(text_points))[0]).decode("ASCII")
        output = output.split("\n")[1:-1]

        with open("cca06_old_" + str(z) + ".dat", "wb") as fd:
            for i in range(0, len(output)):
                fd.write(struct.pack(">f4", float(output[i].split()[7])))

        print("Wrote depth " + str(z))


def main():
    """
    The main function.

    Returns:
        Integer (0 if successful)
    """
    #gen_grid()
    show_plots()

    return 0

if __name__ == "__main__":
    sys.exit(main())
