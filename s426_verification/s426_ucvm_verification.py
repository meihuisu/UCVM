#!/bin/env python
"""
Checks the CVM-S4.26 velocity model (sans GTL) against the original data. The idea is that we
want to see

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   October 31, 2016
:modified:  October 31, 2016
:target:    UCVM 15.10.0. Python 3.5 and above.
"""
import os
import sys
import math
from subprocess import Popen, PIPE, STDOUT

import numpy as np
import pickle
import pyproj

import matplotlib.pyplot as plt
from mpl_toolkits import basemap
import matplotlib.colors as mcolors
import matplotlib.cm as cm

global_data = np.zeros((992, 1536, 9))


def frange(start, stop, step):
    r = start
    while r < stop:
        yield r
        r += step


def parse_ucvm_line(line: str) -> dict:
    """

    :param line:
    :return:
    """
    parts = line.split()

    return {
        "x": float(parts[0]),
        "y": float(parts[1]),
        "z": float(parts[2]),
        "surface": float(parts[3]),
        "vs30": float(parts[4]),
        "vp": float(parts[6]),
        "vs": float(parts[7]),
        "density": float(parts[8])
    }


def _extract_and_parse_data() -> int:
    global global_data

    if os.path.exists("extracted_data.npy"):
        global_data = np.load("extracted_data.npy")
    else:
        p = Popen(["/Users/davidgil/ucvm-15.10.0-3/bin/ucvm_query", "-m", "cvms5", "-f",
                   "/Users/davidgil/ucvm-15.10.0-3/conf/ucvm.conf"], stdout=PIPE, stdin=PIPE,
                  stderr=STDOUT)

        with open(os.path.join(".", "mira_data", "CVM4SI26_slice_%03d" % 100), "r") as fd:
            input_lines = fd.read().split("\n")

        counter = 0

        # Build the data matrix.
        for y in range(0, 1536):
            for x in range(0, 992):
                if input_lines[counter].strip() == "":
                    continue
                else:
                    parts = input_lines[counter].strip().split()
                    global_data[int(parts[0]) - 1][int(parts[1]) - 1][0] = float(parts[2])
                    global_data[int(parts[0]) - 1][int(parts[1]) - 1][1] = float(parts[3])
                    global_data[int(parts[0]) - 1][int(parts[1]) - 1][2] = float(parts[4])
                    global_data[int(parts[0]) - 1][int(parts[1]) - 1][3] = float(parts[5])
                    global_data[int(parts[0]) - 1][int(parts[1]) - 1][4] = float(parts[6])
                    global_data[int(parts[0]) - 1][int(parts[1]) - 1][5] = float(parts[7])
                counter += 1

        # Now query UCVM based on this data matrix.
        query_points = ""
        for y in range(0, 1536):
            for x in range(0, 992):
                query_points += "%.5f %.5f %.2f\n" % (global_data[x][y][0], global_data[x][y][1],
                                                      global_data[x][y][2])

        output = [parse_ucvm_line(x) for x in p.communicate(query_points.encode("ASCII"))[0].
                                                  decode("ASCII").split("\n")[1:-1]]

        counter = 0
        for y in range(0, 1536):
            for x in range(0, 992):
                if not math.isclose(output[counter]["x"], global_data[x][y][0], abs_tol=10 ** -4) \
                   or not math.isclose(output[counter]["y"], global_data[x][y][1],
                                       abs_tol=10 ** -4):
                    print("Lat, lon do not match!")
                    print(output[counter]["x"], global_data[x][y][0], output[counter]["y"],
                          global_data[x][y][1])
                    return 1
                global_data[x][y][6] = output[counter]["vp"]
                global_data[x][y][7] = output[counter]["vs"]
                global_data[x][y][8] = output[counter]["density"]
                counter += 1

        np.save("extracted_data", global_data)


def plot_model_properties() -> int:
    """

    :return:
    """
    global global_data

    fig = plt.figure(figsize=(10, 10), dpi=100)
    if os.path.exists("test_one_basemapdump.dat"):
        with open("test_one_basemapdump.dat", "rb") as bm_pickle:
            map_base = pickle.load(bm_pickle)
    else:
            map_base = basemap.Basemap(projection='cyl',
                                       llcrnrlat=30.0,
                                       urcrnrlat=38.5,
                                       llcrnrlon=-122.5,
                                       urcrnrlon=-112.5,
                                       resolution='f', anchor='C')
            with open("test_one_basemapdump.dat", "wb") as bm_pickle:
                pickle.dump(map_base, bm_pickle)

    map_base.drawparallels([30, 34.25, 38.5], linewidth=1.0, labels=[1, 0, 0, 0])
    map_base.drawmeridians([-122.5, -117.75, -112.5], linewidth=1.0, labels=[0, 0, 0, 1])

    map_base.drawstates()
    map_base.drawcountries()
    map_base.drawcoastlines()

    lon_array = []
    lat_array = []

    for y in range(0, 1536):
        for x in range(0, 992):
            lon_array.append(global_data[x][y][0])
            lat_array.append(global_data[x][y][1])

    map_base.plot(lon_array, lat_array, "bo", markersize=0.1)

    plt.title("Grid Points Direct From S4.26 ASCII Model")

    plt.savefig("original_model_points.png")

    fig.clear()

    # Now go through UCVM at a fine resolution, 0.001 grid spacing and see where material properties
    # exist.
    p = Popen(["/Users/davidgil/ucvm-15.10.0-3/bin/ucvm_query", "-m", "cvms5", "-f",
               "/Users/davidgil/ucvm-15.10.0-3/conf/ucvm.conf"], stdout=PIPE, stdin=PIPE,
              stderr=STDOUT)

    query_points = ""
    for x in frange(-122.5, -112.501, 0.005):
        for y in frange(30.0, 38.501, 0.005):
            query_points += "%.5f %.5f %.2f\n" % (x, y, 0)

    output = [parse_ucvm_line(x) for x in p.communicate(query_points.encode("ASCII"))[0].
              decode("ASCII").split("\n")[1:-1]]

    lon_array = []
    lat_array = []

    with open("test_one_basemapdump.dat", "rb") as bm_pickle:
        map_base = pickle.load(bm_pickle)

    map_base.drawparallels([30, 34.25, 38.5], linewidth=1.0, labels=[1, 0, 0, 0])
    map_base.drawmeridians([-122.5, -117.75, -112.5], linewidth=1.0, labels=[0, 0, 0, 1])

    map_base.drawstates()
    map_base.drawcountries()
    map_base.drawcoastlines()

    for data in output:
        if data["vp"] != 0.0 and data["vs"] != 0.0:
            lon_array.append(data["x"])
            lat_array.append(data["y"])

    map_base.plot(lon_array, lat_array, "bo", markersize=0.1)

    plt.title("CVM-S4.26 Grid Points as Represented in UCVM")

    plt.savefig("s426_grid_points_with_fix.png")

    return 0


def plot_percentages() -> int:
    fig = plt.figure(figsize=(10, 10), dpi=100)

    cmap = cm.RdBu
    norm = mcolors.Normalize(vmin=0.1, vmax=5)

    data_percent = np.zeros((1534, 990))

    for y in range(1, 1534):
        for x in range(1, 990):
            data_percent[1534 - y][x - 1] = \
                math.fabs(global_data[x - 1][y - 1][7] - global_data[x - 1][y - 1][4]) / \
                global_data[x - 1][y - 1][4] * 100
            if data_percent[1534 - y][x - 1] > 5:
                print("HERE")

    data_percent = np.ma.masked_less(data_percent, 0.1, True)

    plt.title("UCVM - ASCII As a Percentage")
    plt.imshow(data_percent, cmap=cmap, norm=norm)

    cax = plt.axes([0.125, 0.05, 0.775, 0.02])
    cbar = plt.colorbar(cax=cax, orientation='horizontal')

    plt.savefig("ucvm_over_ascii_difference_percentage_with_fix.png")

    return 0


def plot_and_save_models() -> int:
    global global_data

    fig = plt.figure(figsize=(10, 10), dpi=100)

    cmap = cm.RdBu
    norm = mcolors.Normalize(vmin=500, vmax=4500)

    data_ucvm = np.zeros((1534, 990))
    data_ascii = np.zeros((1534, 990))

    for y in range(1, 1534):
        for x in range(1, 990):
            data_ucvm[1534 - y][x - 1] = global_data[x - 1][y - 1][7]
            data_ascii[1534 - y][x - 1] = global_data[x - 1][y - 1][4]

    plt.subplot(121)
    plt.imshow(data_ucvm, cmap=cmap, norm=norm)

    plt.subplot(122)
    plt.imshow(data_ascii, cmap=cmap, norm=norm)

    plt.savefig("ascii_and_ucvm_with_fix.png")

    fig.clear()

    data_zoom_ucvm = np.zeros((20, 20))
    data_zoom_ascii = np.zeros((20, 20))

    for y in range(900, 920):
        for x in range(400, 420):
            data_zoom_ucvm[919 - y][x - 400] = global_data[x - 1][y - 1][7]
            data_zoom_ascii[919 - y][x - 400] = global_data[x - 1][y - 1][4]

    plt.subplot(121)
    plt.imshow(data_zoom_ucvm, cmap=cmap, norm=norm)

    plt.subplot(122)
    plt.imshow(data_zoom_ascii, cmap=cmap, norm=norm)

    plt.savefig("ascii_and_ucvm_with_fix_zoom.png")

    return 0


def plot_and_save_model_differences() -> int:
    global global_data

    fig = plt.figure(figsize=(10, 10), dpi=100)

    #ax1 = plt.subplot(121)
    #ax2 = plt.subplot(222)
    #ax3 = plt.subplot(223)

    cmap = cm.RdBu
    norm = mcolors.Normalize(vmin=-50, vmax=50)

    data = np.zeros((1536, 992))

    for y in range(0, 1536):
        for x in range(0, 992):
            data[1535 - y][x] = global_data[x][y][4] - global_data[x][y][7]

    plt.imshow(data, cmap=cmap, norm=norm)

    plt.title("CVM-S4.26 ASCII Vs Minus UCVM Vs")

    cax = plt.axes([0.125, 0.05, 0.775, 0.02])
    cbar = plt.colorbar(cax=cax, orientation='horizontal')

    #plt.tight_layout()

    plt.savefig("ascii_minus_ucvm_with_fix.png")

    average = 0
    average_non_zero = 0
    for y in range(1, 1535):
        for x in range(1, 991):
            average += math.fabs(global_data[x][y][4] - global_data[x][y][7])
            if global_data[x][y][7] != 0:
                average_non_zero += math.fabs(global_data[x][y][4] - global_data[x][y][7])
    average /= 1535 * 991
    average_non_zero /= 1535 * 991

    print(average, average_non_zero)

    return 0


def main() -> int:
    """
    Defines the main function.
    :return: 0 if success, 1 if not.
    """
    _extract_and_parse_data()
    #if plot_model_properties() == 1:
    #    return 1
    #if plot_and_save_model_differences() == 1:
    #    return 1
    #if plot_and_save_models() == 1:
    #    return 1
    #plot_percentages()
    rotation = math.degrees(math.atan2(3368878.93935 - 3865231.5627414100,
                                       596013.92402 - 10616.34694))
    rotation2 = math.degrees(math.atan2(3368878.93935 - 3861649.01877,
                                        596013.92402 - 14841.59420))
    print(rotation, rotation2)

    total_width_m = math.sqrt(math.pow(3368878.93935 - 3865231.5627414100, 2.0) +
                              math.pow(596013.92402 - 10616.34694, 2.0))
    print(total_width_m, total_width_m / 1535)
    total_width_m = math.sqrt(math.pow(3368878.93935 - 3861649.01877, 2.0) +
                              math.pow(596013.92402 - 14841.59420, 2.0))
    print(total_width_m, total_width_m / 1535)

    return 0

if __name__ == "__main__":
    sys.exit(main())
