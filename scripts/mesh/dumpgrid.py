#!/usr/bin/env python
##########################################################
#
# Script: dumpgrid.py
#
# Description: Dumps mesh grid in readable form. Needs
#              input gridfile nad dimensions nx,ny. Display
#              format is:
#
# x, y, lon, lat
#
##########################################################


# Basic modules
import os
import sys
import struct

GRD_FORMAT='ddd'

class DumpGrid:
    def __init__(self, gridfile, dims):
        self.gridfile = gridfile
        self.dims = dims
        return

    def main(self):
        ip = open(self.gridfile)
        size = struct.calcsize(GRD_FORMAT)

        for y in xrange(0, self.dims[1]):
            for x in xrange(0, self.dims[0]):
                buf = ip.read(size)
                point = struct.unpack(GRD_FORMAT, buf)
                print '%d, %d : %f, %f' % (x, y, point[0], point[1])
 
        ip.close()

        return(0)


def usage():
    print "usage: %s <gridfile> <nx> <ny>" % (sys.argv[0])
    return


if __name__ == '__main__':
    if (len(sys.argv) != 4):
        usage()
        sys.exit(1)

    gridfile = sys.argv[1]
    dims = [int(sys.argv[2]), int(sys.argv[3])]

    prog = DumpGrid(gridfile, dims)
    sys.exit(prog.main())
