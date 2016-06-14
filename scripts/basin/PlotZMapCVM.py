#!/usr/bin/env python

# Basic modules
import os
import sys
import array
import getopt
import numpy as np

# Patrick's modules
from Params import *
from PlotUtils import *
from ParseMeta import *

# Matplotlib modules
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors 
import matplotlib.cm as cm


class PlotZMapCVM:
    def __init__(self, outfile, infile, title, \
                     color=None, discretize=None, scale=None):
        self.valid = False
        self.outfile = outfile
        self.source = ''
        self.point1 = [0.0, 0.0]
        self.point2 = [0.0, 0.0]
        self.infile = infile
        self.vs = 0.0
        self.title= title
        self.spacing = [0.0, 0.0]
        self.dims = [0, 0]
        self.value = 'dep'
        self.format = []
        self.color = color
        if (discretize != None):
            self.discretize = discretize
        else:
            self.discretize = [False, 10]
        if (scale != None):
            self.scale = scale
        else:
            self.scale = PLOT_PROP_RANGES[self.value]
        self.valid = True

    def isValid(self):
        return self.valid

    def cleanup(self):
        return

    def _parseHeader(self, hdr):
        self.vs = 1000.0
        #self.vs = 2500.0
        self.point1 = [-121.0, 31.0]
        self.point2 = [-112.505, 36.505]
        self.dims = [1701, 1101]
        #self.point1 = [-120.85, 30.96]
        #self.point2 = [-113.35, 36.6]
        #self.dims = [1501, 1129]

        return(0)
        p = ParseMeta(hdr)
        p.showDict()
        config = p.getDict()
 
        if (config['datatype'] != 'z_map_cvm'):
            print "Invalid file datatype: %s" % (config['datatype'])
            return(1)

        self.source = config['source']

        point_tokens = config['point_1'].split(',')
        self.point1 = [float(point_tokens[0]), \
                           float(point_tokens[1])]

        point_tokens = config['point_2'].split(',')
        self.point2 = [float(point_tokens[0]), \
                           float(point_tokens[1])]

        self.vs = float(config['vs'])
        self.spacing[0] = float(config['hspacing'])

        dim_tokens = config['dims'].split(',')
        self.dims = [int(dim_tokens[0]), int(dim_tokens[1])]

        self.format = config['format'].split(',')
        if (self.value not in self.format):
            print "Specified value %s not in data file" % (self.value)
            return(1)

        return(0)
        

    def _getPlotPointsArray(self):
        if (not os.path.exists(self.infile)):
            print "File %s does not exist" % (self.infile)
            return(None, None, None)

        fp = open(self.infile, 'r')
        data = fp.readlines()
        fp.close()

        # Remove header/comment lines
        i = 0
        while (data[i][0] == '#'):
            i = i + 1
        hdr = data[0:i]
        data = data[i:]

        if (self._parseHeader(hdr) != 0):
            return(None, None, None)

        num_points = self.dims[0]*self.dims[1]
        #if (num_points != len(data)):
        #    print "Data length mismatch, expected %d points" %(num_points)
        #    return(None, None, None)

        # Find index position of column
        index = 2
        #index = 3

        # Define point array
        points = np.arange(self.dims[0]*self.dims[1], \
                           dtype=float).reshape(self.dims[1],\
                                                self.dims[0])

        lons = np.arange(self.dims[0], dtype=float)
        lats = np.arange(self.dims[1], dtype=float)

        # Data is x-fast
        for y in xrange(0, self.dims[1]):
            for x in xrange(0, self.dims[0]):
                d = data[y*self.dims[0] + x]
                tokens = d.split()
                points[y][x] = float(tokens[index]) / 1000.0
                lons[x] = float(tokens[0])
                lats[y] = float(tokens[1])

        for x in xrange(1, self.dims[0]):
            if (lons[x] <= lons[x-1]):
                print "Out of order (x=%d): %lf %lf" % (x, lons[x], lons[x-1])
        for y in xrange(1, self.dims[1]):
            if (lats[y] <= lats[y-1]):
                print "Out of order (y=%d): %lf %lf" % (x, lats[y], lats[y-1])

        # Mask array to hide negative depth values
        masked = np.ma.masked_less(points, 0.0)

        return(masked, lons, lats)


    def main(self):

        plot_x_size = 8
        plot_y_size = 6
        fig = plt.figure(figsize=(plot_x_size,plot_y_size))
        
        # Get plot points
        points,lons,lats = self._getPlotPointsArray()
        try:
            if (points == None):
                print "Failed to get plot points"
                return(1)       
        except:
            print "WARNING: Update numpy to fix masked array bug"

        value_min = self.scale[0]
        value_max = self.scale[1]
        print "Colorbar range: %f to %f" % (value_min, value_max)

        if (self.color == None):
            cmap = cm.Spectral
        else:
            cmap = eval("cm.%s" % (self.color))

        # Discrete intervals
        if (self.discretize[0]):
            print "Converting continuous color scale to discrete"
            cmap = PlotUtils().plotCmapDiscretize(cmap, self.discretize[1])

        norm = mcolors.Normalize(vmin=value_min,vmax=value_max)

        # Plot the depth map
        PlotUtils().plotMapArray(fig, PLOT_MAP_LOC, \
                                     [self.point1, self.point2], \
                                     points, lons, lats, \
                                     self.title, cmap, norm)
        
        # Plot colorbar
        PlotUtils().plotColorbar(self.value, \
                                     PLOT_PROP_UNITS[self.value], \
                                     cmap, norm, \
                                     value_min, value_max, \
                                     self.discretize[1])

        print "Saving plot file %s" % (self.outfile)
        plt.savefig(self.outfile)
        plt.show()

        return 0


class PlotZMapCVMScript:
    def __init__(self, argv):
        self.argc = len(argv)
        self.argv = argv

    def usage(self):
        print "Usage: " + sys.argv[0] + " [-c color] [-d bool,#] [-s] <outfile> infile> <title>"
        print "Example: " + sys.argv[0] + " test.png data.in Test\n"
        print "\t[-h]: This help message"
        print "\t[-c]: Set custom color bar (default: Spectral)"
        print "\t[-d]: Set color bar discretization (default: false,10)"
        print "\t[-s]: Set custom scale"
        print "\t<outfile>: Filename for plot"
        print "\t<infile>: Data source"
        print "\t<title>: Title for plot\n"
        sys.exit(1)


    def main(self):

        # Parse options
        try:
            opts, args = getopt.getopt(self.argv[1:], "hc:d:s:", \
                                           ["help", "colorbar", \
                                                "discretize", "scale"])
        except getopt.GetoptError, err:
            print str(err)
            self.usage()
            return(1)

        color = None
        discretize = None
        scale = None

        for o, a in opts:
            if o in ('-s', '--scale'):
                stokens = a.split(',')
                scale = [float(stokens[0]), float(stokens[1])]
            elif o in ('-d', '--discretize'):
                stokens = a.split(',')
                discretize = [stokens[0], int(stokens[1])]
                if ((discretize[0] == 'true') or (discretize[0] == 'True')):
                    discretize[0] = True
                else:
                    discretize[0] = False
            elif o in ('-c', '--colorbar'):
                color = a
            elif o in ("-h", "--help"):
                self.usage()
                return(0)
            else:
                print "Invalid option %s" % (o)
                return(1)

        # Check command line arguments
        if (len(args) < 3):
            self.usage()

        outfile = args[0]
        infile = args[1]
        title = args[2]

        prog = PlotZMapCVM(outfile, infile, title, color, discretize, scale)
        if (prog.isValid() == False):
            return(1)
            
        retval = prog.main()
        prog.cleanup()
        return(retval)
        

if __name__ == '__main__':

    prog = PlotZMapCVMScript(sys.argv)
    sys.exit(prog.main())
