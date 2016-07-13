"""
Uses the PyCVM library included within UCVM 14.3.0 and above to generate three 1D depth profiles
for the BBP 1D model. This is how the three depth profiles showing the new CyberShake 1D model
were generated.

NOTE: This script must be run with the UCVM utilities folder as the working directory.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   July 11, 2016
:modified:  July 11, 2016
:target:    UCVM 14.3.0 - 15.10.0. Python 2.7 and lower.
"""

import numpy as np
import matplotlib.pyplot as plt

from pycvm import DepthProfile, Point, Plot

DEPTH = 50000
Y_TICKS = 5000
INTERVAL = 20
MODEL = "bbp1d"
FILENAME = "cs_cca_ucvm_1d_[prop].png"

LABELS = {
    "vp": {
        "title": "CyberShake CCA 1D Depth Profile Vp",
        "x_axis": "Velocity (m/s)",
        "y_axis": "Depth (m)",
        "x_ticks": {"start": 3.5, "end": 8.0, "spacing": 0.5, "min": 3.0}
    },
    "vs": {
        "title": "CyberShake CCA 1D Depth Profile Vs",
        "x_axis": "Velocity (m/s)",
        "y_axis": "Depth (m)",
        "x_ticks": {"start": 2.0, "end": 4.5, "spacing": 0.5, "min": 1.5, "max": 5.0}
    },
    "density": {
        "title": "CyberShake CCA 1D Depth Profile Density",
        "x_axis": "Density (kg/m^3)",
        "y_axis": "Depth (m)",
        "x_ticks": {"start": 2.2, "end": 3.4, "spacing": 0.2}
    }
}


def generate_and_save_plot(prop):
    """
    Generates and saves the 1D profile for each property listed in LABELS.
    :param prop: The property that we are making the 1D profile for.
    :return: Nothing.
    """

    # Get the material properties.
    depth_profile = DepthProfile(Point(-118, 34, 0), DEPTH, INTERVAL, MODEL)

    # Create the plot object.
    plot = Plot(LABELS[prop]["title"], LABELS[prop]["x_axis"], LABELS[prop]["y_axis"],
                None, 7, 10)

    plt.xticks(np.arange(float(LABELS[prop]["x_ticks"]["start"]),
                         float(LABELS[prop]["x_ticks"]["end"]) + 0.01,
                         float(LABELS[prop]["x_ticks"]["spacing"])))
    plt.yticks(np.arange(0, DEPTH + 0.5, Y_TICKS))
    plt.grid(True)

    x_min = float(LABELS[prop]["x_ticks"]["start"])
    if "min" in LABELS[prop]["x_ticks"].keys():
        x_min = float(LABELS[prop]["x_ticks"]["min"])

    x_max = float(LABELS[prop]["x_ticks"]["end"])
    if "max" in LABELS[prop]["x_ticks"].keys():
        x_max = float(LABELS[prop]["x_ticks"]["max"])

    plt.xlim(xmin=x_min, xmax=x_max)

    # Add to plot.
    depth_profile.addtoplot(plot, prop)

    plt.savefig(FILENAME.replace("[prop]", prop))


def generate_and_save_tri_plot():
    """
    Generates and saves the 1D profile that shows all the properties listed in LABELS on one
    profile, like we did with the Southern California CyberShake 1D profile.
    :return: Nothing.
    """

    # Create the plot object.
    plot = Plot("CyberShake CCA 1D Depth Profile All", "Units", "Depth (m)",
                None, 7, 10)

    # Find the min_x, max_x values.
    min_x = 9999
    max_x = -1

    for prop in LABELS.keys():
        if "start" in LABELS[prop]["x_ticks"] and LABELS[prop]["x_ticks"]["start"] < min_x:
            min_x = LABELS[prop]["x_ticks"]["start"]
        if "min" in LABELS[prop]["x_ticks"] and LABELS[prop]["x_ticks"]["min"] < min_x:
            min_x = LABELS[prop]["x_ticks"]["min"]
        if "end" in LABELS[prop]["x_ticks"] and LABELS[prop]["x_ticks"]["end"] > max_x:
            max_x = LABELS[prop]["x_ticks"]["end"]
        if "max" in LABELS[prop]["x_ticks"] and LABELS[prop]["x_ticks"]["max"] > max_x:
            max_x = LABELS[prop]["x_ticks"]["max"]

    plt.xticks(np.arange(float(min_x), float(max_x) + 0.01, 0.5))

    plt.yticks(np.arange(0, DEPTH + 0.5, Y_TICKS))
    plt.grid(True)
    plt.xlim(xmin=min_x, xmax=max_x)

    for prop in LABELS.keys():
        # Get the material properties.
        depth_profile = DepthProfile(Point(-118, 34, 0), DEPTH, INTERVAL, MODEL)
        depth_profile.addtoplot(plot, prop, customlabels={
            "vp": "Vp", "vs": "Vs", "density": "Density"
        })

    plt.savefig(FILENAME.replace("[prop]", "all"))


def main():
    """
    The main function for our program.
    :return: Zero when successful.
    """

    for key in LABELS.keys():
        generate_and_save_plot(key)

    generate_and_save_tri_plot()

    return 0

if __name__ == "__main__":
    main()
