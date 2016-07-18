"""
Uses the PyCVM library from UCVM 15.10.0 (although this will also work with 14.3.0 as well) to
generate a series of 1D depth profiles. This is a very basic example just to show how these
scripts work.

NOTE: This script must be run with the UCVM utilities folder as the working directory.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   July 18, 2016
:modified:  July 18, 2016
:target:    UCVM 14.3.0 - 15.10.0. Python 2.7 and lower.
"""

import numpy as np
import matplotlib.pyplot as plt

from pycvm import DepthProfile, Point, Plot, UCVM

DEPTH = 40000                                   #: int: Depth in meters.
INTERVAL = 200                                  #: int: Spacing between points in meters.
MODEL = "cvms5"                                 #: str: Model to query (UCVM name).
FILENAME = "cvms426_[lon]_[lat]_[prop].png"     #: str: The file name of the image file.
PROPS_TO_PLOT = ["vs", "vp", "density"]         #: list: The properties to plot (separate + whole).
POINTS_TO_PLOT = [
    (-116.335, 34.829),
    (-117.680, 33.998),
    (-117.316, 34.093),
    (-118.488, 35.305)
]                                               #: list: The points (lat, lon WGS84) to plot.
OUTPUT_FORMAT = "%-10.3f%-10.3f%-10.3f" + \
                "%-10.3f%-10.3f%-10.3f\n"       #: str: Defines the output format.


def main():
    """
    Generates the 1D profiles given the constants above.
    :return: Zero on success.
    """
    for point in POINTS_TO_PLOT:

        for mat_prop in PROPS_TO_PLOT:
            file_name = FILENAME.replace("[lon]", str(point[0])).replace("[lat]", str(point[1]))\
                                .replace("[prop]", mat_prop)
            plot = Plot("CVM-S4.26 (" + str(point[0]) + ", " + str(point[1]) + ") From 0m to " +
                        str(DEPTH) + "m", "Units (see legend)", "Depth (m)", None, 7, 10)

            depth_profile = DepthProfile(Point(point[0], point[1], 0), DEPTH, INTERVAL, MODEL)
            depth_profile.addtoplot(plot, mat_prop)
            plt.savefig(file_name)
            plt.gcf().clear()

        plot_all = Plot("CVM-S4.26 (" + str(point[0]) + ", " + str(point[1]) + ") From 0m to " +
                        str(DEPTH) + "m", "Units", "Depth (m)", None, 7, 10)

        plt.xticks(np.arange(0.0, 8.51, 1.0))
        plt.xlim(xmin=0.0, xmax=8.5)

        for mat_prop in PROPS_TO_PLOT:
            depth_profile = DepthProfile(Point(point[0], point[1], 0), DEPTH, INTERVAL, MODEL)
            depth_profile.addtoplot(plot_all, mat_prop, customlabels={
                "vp": "Vp", "vs": "Vs", "density": "Density"
            })

        plt.savefig(FILENAME.replace("[lon]", str(point[0]))
                    .replace("[lat]", str(point[1]))
                    .replace("[prop]", "all"))
        plt.gcf().clear()

        # Finally, get the text value list of properties.
        ucvm_obj = UCVM()
        point_list = []

        for i in range(0, DEPTH + 1, INTERVAL):
            point_list.append(Point(point[0], point[1], i))

        list_of_properties = ucvm_obj.query(point_list, MODEL)

        file_to_write = open(FILENAME.replace("[lon]", str(point[0]))
                             .replace("[lat]", str(point[1]))
                             .replace("[prop]", "all")
                             .replace("png", "txt"), "w")

        file_to_write.write("Lon       Lat       Depth     Vp        Vs        Density\n")
        counter = 0

        for line in list_of_properties:
            file_to_write.write(OUTPUT_FORMAT % (point[0], point[1], counter * INTERVAL,
                                                 line.vp, line.vs, line.density))
            counter += 1

        file_to_write.close()

    return 0

if __name__ == "__main__":
    main()
