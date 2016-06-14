#!/usr/bin/env python
##########################################################
#
# Script: ProjUCVM.py
#
# Description: Defines UCVM projection
#
##########################################################


# Basic modules
import os
import sys
import math
import numpy
import pyproj

# Constants
DEG_TO_RAD = 0.0174532925


class ProjUCVM:
    def __init__(self, pstr, p1, rot, dims):
        self.valid = False
        self.p1 = p1
        self.rot = rot*DEG_TO_RAD
        self.dims = dims

        # Create projection
        self.proj = pyproj.Proj(pstr)

        # Offsets to apply from origin
        self.offsets = self.proj(self.p1[0], self.p1[1])

        # Determine p2
        self.p2 = self.xy2geo(self.dims[0], self.dims[1])

        self.valid = True


    def isValid(self):
        return self.valid


    def cleanup(self):
        return


    def _rotate(self, x, y, theta):
        newx = (x) * math.cos(theta) - (y) * math.sin(theta);
        newy = (x) * math.sin(theta) + (y) * math.cos(theta);
        return(newx,newy)


    def geo2xy(self, lon, lat):
        x, y = self.proj(lon, lat)
        x = x - self.offsets[0]
        y = y - self.offsets[1]
        x,y = self._rotate(x,y, self.rot)
        if ((x < 0) or (y < 0) or (x > self.dims[0]) or \
                (y > self.dims[1])):
            return(None)
        else:
            return(x,y)


    def xy2geo(self, x, y):
        if ((x < 0) or (y < 0) or (x > self.dims[0]) or \
                (y > self.dims[1])):
            return(None)

        x,y = self._rotate(x,y, -self.rot)
        x = x + self.offsets[0]
        y = y + self.offsets[1]
        lon, lat = self.proj(x, y, inverse=True)
        return(lon,lat)


    def getDims(self):
        return(self.dims)


def usage():
    print "usage: %s" % (sys.argv[0])
    return


if __name__ == '__main__':
    # AEQD projection
    pstr = "+proj=aeqd +lat_0=36.0 +lon_0=-120.0 +x_0=0.0 +y_0=0.0"
    origin = [-129.75, 40.75]
    rot=55.0
    dims = [1800000.0,900000.0]
    prog = ProjUCVM(pstr, origin, rot, dims)
    dims = prog.getDims()
    print "Four corners of AEQD projection:"
    print "NW: ", prog.xy2geo(0.0, 0.0)
    print "NE: ", prog.xy2geo(0.0, dims[1])
    print "SW: ", prog.xy2geo(dims[0], 0.0)
    print "SE: ", prog.xy2geo(dims[0], dims[1])
    print ""

    # UTM-11 projection
    pstr = "+proj=utm +datum=WGS84 +zone=11"
    origin = [-118.5, 28.0]
    rot=-30.0
    dims = [900000.0, 1800000.0]
    prog = ProjUCVM(pstr, origin, rot, dims)
    dims = prog.getDims()
    print "Four corners of UTM-11 projection:"
    print "SW: ", prog.xy2geo(0.0, 0.0)
    print "NW: ", prog.xy2geo(0.0, dims[1])
    print "SE: ", prog.xy2geo(dims[0], 0.0)
    print "NE: ", prog.xy2geo(dims[0], dims[1])


    sys.exit(0)
