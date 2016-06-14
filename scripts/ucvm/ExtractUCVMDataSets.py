#!/usr/bin/env python
##########################################################
#
# Script: ExtractUCVMDataSets.py
#
# Description: Constructs UCVM DEM and Vs30 map in
# standard UCVM projection. UCVM Model is x-y grid with the 
# following structure:
#
#  float elev;
#  float vs30;
#
# Elevation data is extracted from USGS NED and ETOPO1.
# Vs30 data is extracted from Wills 2006, Wald 2007 
#
##########################################################


# Basic modules
import os
import sys
import struct
import numpy
from ParseConfig import *
from ProjUCVM import *
from DEM import *
from Vs30 import *

# GRD Constants
GRD_DIR = '../bin'
GRD_SCRIPT = 'run_grd.sh'


class ExtractUCVMDataSets:
    def __init__(self, conf, neddir, bathdir, willsdir, walddir, demfile, vs30file):
        self.valid = False
        self.conf = conf
        self.neddir = neddir
        self.bathdir = bathdir
        self.willsdir = willsdir
        self.walddir = walddir
        self.demfile = demfile
        self.vs30file = vs30file
        self.gridfile = "ucvm.grid"
        self.dims = [0, 0]
        self.valid = True


    def isValid(self):
        return self.valid


    def cleanup(self):
        return


    def _parseModel(self):
        fp = open(self.conf, 'r')
        data = fp.readlines()
        fp.close()

        p = ParseConfig(data)
        p.showDict()
        config = p.getDict()
 
        self.proj_str = config['proj_str']
        tokens = config['proj_origin'].split(',')
        self.origin = [float(tokens[0]), float(tokens[1])]
        self.rot = float(config['proj_rot'])
        tokens = config['proj_size'].split(',')
        self.size = [float(tokens[0]), float(tokens[1])]
        self.spacing = float(config['spacing'])
        return(0)


    def _getGrid(self):
        proj = ProjUCVM(self.proj_str, self.origin, self.rot, self.size)
        self.dims = [int(self.size[0]/self.spacing), \
                           int(self.size[1]/self.spacing)]
        print "Dims: ", self.dims

        fp = open(self.gridfile, 'w')

        extents = [int(self.dims[0] * self.spacing), \
                       int(self.dims[1] * self.spacing)]
        print "Extents: ", extents

        print "Four corners of projection:"
        print "\tNW: ", proj.xy2geo(0.0, 0.0)
        print "\tNE: ", proj.xy2geo(0.0, self.size[1])
        print "\tSW: ", proj.xy2geo(self.size[0], 0.0)
        print "\tSE: ", proj.xy2geo(self.size[0], self.size[1])

        print "Google Format:"
        lon,lat = proj.xy2geo(0.0, 0.0)
        print "\tNW: %lf,%lf" % (lat,lon)
        lon,lat = proj.xy2geo(0.0, self.size[1])
        print "\tNE: %lf,%lf" % (lat,lon)
        lon,lat = proj.xy2geo(self.size[0], 0.0)
        print "\tSW: %lf,%lf" % (lat,lon)
        lon,lat = proj.xy2geo(self.size[0], self.size[1])
        print "\tSE: %lf,%lf" % (lat,lon)

        # Generate cell vertex grid
        #start = int(self.spacing / 2.0)
        start = 0

        for y in xrange(start, extents[1], int(self.spacing)):
            for x in xrange(start, extents[0], int(self.spacing)):
                retval = proj.xy2geo(x, y)
                if (retval == None):
                    print "Failed to convert %lf, %lf" % (x * self.spacing, \
                                                              y * self.spacing)
                    return(None)
                #val = struct.pack('ff', lon, lat)
                #fp.write(val)
                fp.write("%lf %lf\n" % (retval[0], retval[1]))

        fp.close()
        return(self.gridfile);


    def _queryGRD(self, source):
        infile = self.gridfile
        outfile = '%s.out' % (source)

        if (source == 'DEM'):
            grd = DEM(GRD_DIR, GRD_SCRIPT, self.neddir, self.bathdir)
        else:
            grd = Vs30(GRD_DIR, GRD_SCRIPT, self.willsdir, self.walddir)

        grd.run(infile, outfile)

        fp = open(outfile, 'r')
        demdata = fp.readlines()
        fp.close()

        #os.remove(infile)
        #os.remove(outfile)

        data = numpy.arange(self.dims[0]*self.dims[1], \
                                dtype=float).reshape(self.dims[1], \
                                                         self.dims[0])
        for y in xrange(0, self.dims[1]):
            for x in xrange(0, self.dims[0]):
                tokens = demdata[y*self.dims[0]+x].split()
                lon = float(tokens[0])
                lat = float(tokens[1])
                elev = float(tokens[3])
                flag = int(tokens[4])
                if (flag):
                    data[y][x] = elev
                else:
                    print "No data for point %lf, %lf" % \
                        (lon, lat)
                    return(None)

        return(data)


    def main(self):

        # Parse LT model
        print "Parsing model config"
        self._parseModel()

        # Generate grid in desired projection
        print "Generating 2D grid"
        gridfile = self._getGrid()
        if (gridfile == None):
            print "No points to extract"
            return(1)

        # Query DEM
        print "Querying DEM"
        dem_data = self._queryGRD('DEM')
        if (dem_data == None):
            print "No DEM data found"
            return(1)

        # Query Vs30
        print "Querying DEM"
        vs30_data = self._queryGRD('Vs30')
        if (vs30_data == None):
            print "No Vs30 data found"
            return(1)

        # Write topo file, x-fast
        print "Writing DEM %s" % (self.demfile)
        fp = open(self.demfile, 'wb')
        for y in xrange(0, self.dims[1]):
            for x in xrange(0, self.dims[0]):
                val = struct.pack('f', dem_data[y][x])
                fp.write(val)
        fp.close()

        print "Writing Vs30 %s" % (self.vs30file)
        fp = open(self.vs30file, 'wb')
        for y in xrange(0, self.dims[1]):
            for x in xrange(0, self.dims[0]):
                val = struct.pack('f', vs30_data[y][x])
                fp.write(val)
        fp.close()
        return 0


def usage():
    print "usage: %s <ucvm conf> <neddir> <bathdir> <willsdir> <walddir> <demfile> <vs30file>" % (sys.argv[0])
    return


if __name__ == '__main__':
    if (len(sys.argv) != 8):
        usage()
        sys.exit(1)

    conf = sys.argv[1]
    neddir = sys.argv[2]
    bathdir = sys.argv[3]
    willsdir = sys.argv[4]
    walddir = sys.argv[5]
    demfile = sys.argv[6]
    vs30file = sys.argv[7]

    prog = ExtractUCVMDataSets(conf, neddir, bathdir, \
                                   willsdir, walddir, demfile, vs30file)
    sys.exit(prog.main())
