#!/usr/bin/env python
##########################################################
#
# Script: Text2Grid.py
#
# Description: Converts CVM-SI model files in text format
#              into a 3D binary grid file
#
##########################################################


# Basic modules
import os
import sys
import glob
import getopt
import struct


class Text2Grid:
    def __init__(self, indir, filebase, dims, outfile, iter):
        self.valid = False
        self.indir = indir
        self.filebase = filebase
        self.dims = dims
        self.outfile = outfile
        self.iter = iter
        self.valid = True


    def isValid(self):
        return self.valid


    def cleanup(self):
        return


    def _sortPoints(self, points):
        newpoints = sorted(points, key=lambda item: item[0], reverse=False)
        return (newpoints)
        

    def main(self):
        num_points = self.dims[0] * self.dims[1] * self.dims[2]
        num_read = 0
        recsize = struct.calcsize('fff')
        
        # Create empty 3D model
        print "Creating empty 3D model file %s" % (self.outfile)
        op = open(self.outfile, 'w')
        data = struct.pack('fff', 0.0, 0.0, 0.0)
        for i in xrange(0, num_points):
            op.write(data)
        op.close()

        # Get list of model files
        filelist = glob.glob('%s/%s*' % (self.indir, self.filebase))

        # Process each file
        op = open(self.outfile, 'r+')
        for file in filelist:
            print "Processing file %s" % (file)
            sys.stdout.flush()
            ip = open(file, 'r')
            lines = ip.readlines()
            ip.close()

            # Determine point offsets in final model file
            points = []
            for line in lines:
                tokens = line.split()
                x = int(tokens[0]) - 1
                y = int(tokens[1]) - 1
                z = int(tokens[2]) - 1
                if (self.iter == 0):
                    vp = float(tokens[6])
                    vs = float(tokens[7])
                    rho = float(tokens[8])
                else:
                    vp = float(tokens[9])
                    vs = float(tokens[10])
                    rho = float(tokens[11])
                #op.seek((z*(self.dims[0]*self.dims[1]) + \
                #         y*self.dims[0] + x) * recsize, os.SEEK_SET)
                #op.write(struct.pack('fff', vp, vs, rho))
                offset = (z*(self.dims[0]*self.dims[1]) + \
                          y*self.dims[0] + x) * recsize
                points.append([offset, struct.pack('fff', vp, vs, rho)])

            # Sort these points by offset and check that
            # they are contiguous
            points = self._sortPoints(points)
            prevpoint = points[0]
            for point in points[1:]:
                diff = point[0] - prevpoint[0]
                if (diff != recsize):
                    print "Found unexpected offset %d between points" % \
                          (diff)
                    return(1)
                prevpoint = point

            # Write the points to the model file as a group
            buf = ""
            for point in points:
                buf = buf + point[1]
            op.seek(points[0][0], os.SEEK_SET)
            op.write(buf)
                
            num_read = num_read + len(lines)
            
        op.close()

        if (num_read != num_points):
            print "Expected %d points, but read %d" % (num_points, num_read)
        else:
            print "Converted %d points" % (num_points)
                        
        return 0


def usage():
    print "usage: %s <indir> <filebase> <nx> <ny> <nz> <outfile>" % \
          (sys.argv[0])
    return


def usage():
    print "Usage: " + sys.argv[0] + " [-i #] <model dir> <filebase> <nx> <ny> <nz> <outfile>"
    return


if __name__ == '__main__':
    # Parse options
    try:
        opts, args = getopt.getopt(sys.argv[1:], "hi:", \
                                   ["help", "iteration"])
    except getopt.GetoptError, err:
        print str(err)
        self.usage()
        sys.exit(1)

    iter = 1
    for o, a in opts:
        if o in ('-i', '--iteration'):
            iter = int(a)
        elif o in ("-h", "--help"):
            self.usage()
            sys.exit(0)
        else:
            print "Invalid option '%s'" % (o)
            sys.exit(1)

    if (len(args) != 6):
        usage()
        sys.exit(1)

    indir = args[0]
    filebase = args[1]
    nx = int(args[2])
    ny = int(args[3])
    nz = int(args[4])
    outfile = args[5]
    
    prog = Text2Grid(indir, filebase, [nx,ny,nz], outfile, iter)
    
    sys.exit(prog.main())
