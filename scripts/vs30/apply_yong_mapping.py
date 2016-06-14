#!/usr/bin/env python
##########################################################
#
# Script: apply_yong_mapping.py
#
# Description: Convert Yong terrain types to est. Vs30
#
##########################################################


# Basic modules
import os
import sys
import struct
from ParseHeader import *

# Mapping of terrain type to estimated vs30
#
# 17 No Data (DEM related)
# 18 water bodies
#
MAPPING={1:519.0, \
         2:393.0, \
         3:547.0, \
         4:459.0, \
         5:402.0, \
         6:345.0, \
         7:388.0, \
         8:374.0, \
         9:497.0, \
         10:349.0, \
         11:328.0, \
         12:297.0, \
         13:253.0, \
         14:209.0, \
         15:363.0, \
         16:246.0, \
         17:0.0, \
         18:0.0}


class apply_yong_mapping:
    def __init__(self, hdr, infile, outfile):
        self.valid = False
        self.hdr = hdr
        self.infile = infile
        self.outfile = outfile
        self.valid = True


    def isValid(self):
        return self.valid


    def cleanup(self):
        return

    def _parseHdr(self):
        fp = open(self.hdr, 'r')
        data = fp.readlines()
        fp.close()

        p = ParseConfig(data)
        p.showDict()
        config = p.getDict()
 
        self.ncols = int(config['ncols'])
        self.nrows = int(config['nrows'])

        return(0)

    def main(self):

        # Parse header
        print "Parsing data header"
        self._parseHdr()

        ifp = open(self.infile, 'rb')
        ofp = open(self.outfile, 'wb')

        for j in xrange(0, self.nrows):
            for i in xrange(0, self.ncols):
                buf = ifp.read(4)
                val = int(struct.unpack('f', buf)[0])
                if (not MAPPING.has_key(val)):
                    print "Terrain type %d not found in mapping" % (val)
                    return 1
                buf = struct.pack('f', MAPPING[val])
                ofp.write(buf)

        ifp.close()
        ofp.close()
        return 0


def usage():
    print "usage: %s <hdr> <infile> <outfile>" % (sys.argv[0])
    return


if __name__ == '__main__':
    if (len(sys.argv) != 4):
        usage()
        sys.exit(1)

    hdr = sys.argv[1]
    infile = sys.argv[2]
    outfile = sys.argv[3]

    prog = apply_yong_mapping(hdr, infile, outfile)
    sys.exit(prog.main())
