"""
Uses the PyCVM library from UCVM 15.10.0 (although this will also work with 14.3.0 as well) to
generate a 1D depth profile. This is a very basic example just to show how these scripts work.

NOTE: This script must be run with the UCVM utilities folder as the working directory.

:copyright: Southern California Earthquake Center
:author:    David Gill <davidgil@usc.edu>
:created:   July 12, 2016
:modified:  July 12, 2016
:target:    UCVM 14.3.0 - 15.10.0. Python 2.7 and lower.
"""

from pycvm import DepthProfile, Point

DEPTH = 1000
INTERVAL = 1
MODEL = "cvmh"
FILENAME = "example_cvmh.png"
PROP_TO_PLOT = "vs"


def main():
    """
    Generates the 1D profile given the constants above.
    :return: Zero on success.
    """

    # Get the material properties.
    depth_profile = DepthProfile(Point(-118, 34, 0), DEPTH, INTERVAL, MODEL)
    depth_profile.plot(PROP_TO_PLOT, FILENAME)

    return 0

if __name__ == "__main__":
    main()
