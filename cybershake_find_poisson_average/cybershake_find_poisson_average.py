"""
Uses PyCVM to find the poisson average (Vp/Vs) throughout a CyberShake region (as defined by
the bounding box). If the average lies outside of the acceptable boundaries (1.5/1.8) it is flagged
and reported in the CSV.

NOTE: This script must be run with the UCVM utilities folder as the working directory.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   August 29, 2016
:modified:  August 29, 2016
:target:    UCVM 14.3.0 - 15.10.0. Python 2.7 and lower.
"""
import math
import sys
import csv

import mpl_toolkits.basemap.pyproj as pyproj
from subprocess import Popen, PIPE, STDOUT

import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from mpl_toolkits import basemap

from typing import Iterator

BOUNDS = [1.5, 1.8]  #: list: Defines the acceptable boundaries for the ratios.

PROJ_UTM = pyproj.Proj("+proj=utm +datum=WGS84 +zone=11")
PROJ_LAT = pyproj.Proj("+proj=latlong +datum=WGS84")
PROJ_CCA = pyproj.Proj("+proj=utm +zone=10 +ellps=clrk66 +datum=NAD27 +units=m +no_defs")

CYBERSHAKE_BOX = {
    "bl": {"e": -119.93, "n": 33.80},
    "tl": {"e": -121.51, "n": 35.52},
    "tr": {"e": -119.92, "n": 36.50},
    "br": {"e": -118.35, "n": 34.76}
}  #: dict: Defines the CyberShake box in long, lat format.

CCA_CORNERS = {
    "tl": {"e": 504472.106530, "n": 4050290.088321},
    "bl": {"e": 779032.901477, "n": 3699450.983449},
    "tr": {"e": 905367.666914, "n": 4366503.431060},
    "br": {"e": 1181157.928029, "n": 4014713.414724}
}  #: dict: Defines the model corners in UTM format.

CCA_DEFINITION = {
    "origin": {
        "e": pyproj.transform(PROJ_CCA, PROJ_UTM,
                              CCA_CORNERS["bl"]["e"], CCA_CORNERS["bl"]["n"])[0],
        "n": pyproj.transform(PROJ_CCA, PROJ_UTM,
                              CCA_CORNERS["bl"]["e"], CCA_CORNERS["bl"]["n"])[1]
    },
    "rotation": 38.10,
    "dimensions": {
        "x": 1024,
        "y": 896,
        "z": 100
    },
    "spacing": 500,
    "sin_angle": math.sin(math.radians(38.10)),
    "cos_angle": math.cos(math.radians(38.10))
}

UCVM_LOCATION = "/Users/davidgil/ucvm-15.10.0-2"  #: str: Defines where UCVM has been installed.


class InternalMeshIterator:

    def __init__(self, num_at_a_time: int):
        self.current_point = 0
        self.start_point = 0
        self.end_point = \
            CCA_DEFINITION["dimensions"]["x"] * CCA_DEFINITION["dimensions"]["y"] * \
            CCA_DEFINITION["dimensions"]["z"]
        self.num_at_a_time = num_at_a_time

    def __iter__(self) -> Iterator:
        return self

    def __next__(self) -> list:
        internal_counter = 0

        if self.current_point >= self.end_point:
            raise StopIteration()

        ret_list = []

        while internal_counter < self.num_at_a_time and self.current_point < self.end_point:
            # Get our X, Y, and Z coordinates.
            slice_size = CCA_DEFINITION["dimensions"]["x"] * CCA_DEFINITION["dimensions"]["y"]

            z_val = math.floor(self.current_point / slice_size)
            y_val = math.floor((self.current_point - z_val * slice_size) /
                               CCA_DEFINITION["dimensions"]["x"])
            x_val = \
                self.current_point - z_val * slice_size - y_val * \
                                                          CCA_DEFINITION["dimensions"]["x"]

            x_point = \
                CCA_DEFINITION["origin"]["e"] + \
                (CCA_DEFINITION["cos_angle"] * x_val - CCA_DEFINITION["sin_angle"] *
                 y_val) * CCA_DEFINITION["spacing"]
            y_point = \
                CCA_DEFINITION["origin"]["n"] + \
                (CCA_DEFINITION["sin_angle"] * x_val + CCA_DEFINITION["cos_angle"] *
                 y_val) * CCA_DEFINITION["spacing"]
            z_point = z_val * CCA_DEFINITION["spacing"]

            converted_x, converted_y = pyproj.transform(PROJ_UTM, PROJ_LAT, x_point, y_point)

            ret_list.append((converted_x, converted_y, z_point))

            internal_counter += 1
            self.current_point += 1

        return ret_list


def query_ucvm_out_of_bounds(point_list: list, position: int) -> tuple:
    """
    Query the given points and return the points (as a list) that are out of the poisson bounds.
    :param list point_list: The list of all points to check poisson ratio on.
    :return: A list of all points out of bounds.
    """
    proc = Popen([UCVM_LOCATION + "/bin/ucvm_query", "-f", UCVM_LOCATION + "/conf/ucvm.conf",
                  "-m", "cca"], stdout=PIPE, stdin=PIPE, stderr=STDOUT)

    text_points = ""

    for point in point_list:
        text_points += "%.5f %.5f %.5f\n" % (point[0], point[1], point[2])
        if float(point[2]) != float(position * CCA_DEFINITION["spacing"]):
            print("ERROR IN ITERATOR")

    text_points = text_points.encode("ASCII")

    output = str(proc.communicate(input=bytes(text_points))[0], "ASCII")
    output = output.split("\n")[1:-1]

    below_bounds = []
    above_bounds = []

    for i in range(0, len(output)):
        split_line = output[i].split()
        vp = float(split_line[6])
        vs = float(split_line[7])

        if vs <= 0 or vp <= 0:
            continue

        if vp / vs > BOUNDS[1]:
            above_bounds.append((point_list[i][0], point_list[i][1], vp / vs))
        elif vp / vs < BOUNDS[0]:
            below_bounds.append((point_list[i][0], point_list[i][1], vp / vs))

    return above_bounds, below_bounds


def generate_and_save_data(point_list: tuple, filename: str, box: dict, layer: int) -> tuple:
    """
    Generates and saves the plot and CSV of points in point list. This also puts a red box around
    the point region.
    :param point_list: The list of points.
    :param filename: The file name to which this plot should be saved.
    :param box: The bounding corners of the box (CS region) to draw.
    :param layer: The layer number we are on.
    :return: Nothing.
    """
    plt.figure(figsize=(10, 10), dpi=100)
    map_base = basemap.Basemap(projection='cyl',
                               llcrnrlat=33.35,
                               urcrnrlat=39.35,
                               llcrnrlon=-123,
                               urcrnrlon=-115.25,
                               resolution='f', anchor='C')

    map_base.drawparallels([33.35, 36.35, 39.35], linewidth=1.0, labels=[1, 0, 0, 0])
    map_base.drawmeridians([-123, -119.125, -115.25], linewidth=1.0, labels=[0, 0, 0, 1])

    map_base.drawstates()
    map_base.drawcountries()
    map_base.drawcoastlines()

    csv_array = []

    lon_array = []
    lat_array = []

    for point in point_list[0]:
        lon_array.append(point[0])
        lat_array.append(point[1])
        csv_array.append([point[0], point[1], point[2]])

    map_base.plot(lon_array, lat_array, "r.", markersize=0.5)

    lon_array = []
    lat_array = []

    for point in point_list[1]:
        lon_array.append(point[0])
        lat_array.append(point[1])
        csv_array.append([point[0], point[1], point[2]])

    map_base.plot(lon_array, lat_array, "b.", markersize=0.5)

    # Plot the bounding box.
    lon_array = [box["tl"]["e"], box["tr"]["e"], box["br"]["e"],
                 box["bl"]["e"], box["tl"]["e"]]
    lat_array = [box["tl"]["n"], box["tr"]["n"], box["br"]["n"],
                 box["bl"]["n"], box["tl"]["n"]]
    map_base.plot(lon_array, lat_array, "k-")

    # Add the title.
    plt.title("Vp/Vs Ratio for CCA at Depth (%dm)" % (layer * CCA_DEFINITION["spacing"]))

    red_patch = mpatches.Patch(color='red', label="Vp/Vs Above %.1f" % BOUNDS[1])
    blue_patch = mpatches.Patch(color='blue', label="Vp/Vs Below %.1f" % BOUNDS[0])
    plt.legend(handles=[red_patch, blue_patch])

    plt.savefig(filename)

    csv_array.sort(key=lambda x: x[2])

    with open("ratio_csv_%dm.csv" % (layer * CCA_DEFINITION["spacing"]), "w") as fd:
        csv_file = csv.writer(fd)
        for item in csv_array:
            csv_file.writerow(item)

    print("Plot and CSV saved.")

    return csv_array[-1], csv_array[0]


def main() -> int:
    mesh_iterator = InternalMeshIterator(
        CCA_DEFINITION["dimensions"]["x"] * CCA_DEFINITION["dimensions"]["y"]
    )
    highest_lowest = []
    counter = 1

    while True:
        try:
            pts = next(mesh_iterator)
        except StopIteration:
            break

        above, below = query_ucvm_out_of_bounds(pts, counter - 1)
        highest, lowest = generate_and_save_data(
            (above, below), "ratio_map_%dm.png" % (counter * CCA_DEFINITION["spacing"]),
            CYBERSHAKE_BOX, counter)
        print("Layer %d done: highest %f, lowest %f" % (counter, highest[2], lowest[2]))
        highest_lowest.append((highest, lowest))
        counter += 1

    with open("metadata.txt", "w") as fd:
        fd.write("METADATA FOR CCA MODEL VP/VS RATIO:\n\n")

        highest_high = (0, 0, 0)
        lowest_low = (0, 0, 99999)
        counter = 0

        for item in highest_lowest:
            fd.write("%2.3f min at (%4.2f, %4.2f), %2.3f max at (%4.2f, %4.2f) for depth %dm\n" %
                     (item[1][2], item[1][0], item[1][1], item[0][2], item[0][0], item[0][1],
                      counter * CCA_DEFINITION["spacing"]))
            if item[0][2] > highest_high[2]:
                highest_high = item[0]
            if item[1][2] < lowest_low[2]:
                lowest_low = item[1]
            counter += 1

        fd.write("\nMin in model: %2.3f\nMax in model %2.3f" % (lowest_low[2], highest_high[2]))

    return 0

if __name__ == "__main__":
    sys.exit(main())
