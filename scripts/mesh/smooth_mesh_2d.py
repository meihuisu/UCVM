#!/usr/bin/env python
##########################################################
#
# Script: smooth_mesh_2d.py
#
# Description: Smooths a 2D region of an AWP-12 mesh for
#              each k value along the z axis.
#
#              The interpolation boundaries are points on
#              the x-axis.
#
# Eg: M8 80m CVMS smoothing
# $ smooth_mesh_2d.py mesh.media 10125 5000 1060 800 1425
##########################################################

# Basic modules
import sys
import os
import numpy
import struct

AWP12_FORMAT = 'fff'

class SmoothMesh2D:
    def __init__(self, meshfile, dims, bounds, zrange):
        self.meshfile = meshfile
        self.dims = dims
        self.bounds = bounds
        self.zrange = zrange
        return

    def main(self):
        print "Mesh    : %s" % (self.meshfile)
        print "Dims    : %s" % (self.dims)
        print "X-bounds: %s" % (self.bounds)
        print "Z-bounds: %s" % (self.zrange)
        sys.stdout.flush()
        recsize = struct.calcsize(AWP12_FORMAT)
        ip = open(self.meshfile, 'r+')

        # Step through each z value
        for z in xrange(self.zrange[0], self.zrange[1]):
            print "Processing k=%s" % (z)
            sys.stdout.flush()

            # Step through each y value
            for y in xrange(0, self.dims[1]):
                #print "Processing j=%s" % (y)
                #sys.stdout.flush()

                # Find value at max x-bound
                offset = (z * (self.dims[0] * self.dims[1]) + \
                            y * (self.dims[0]) + self.bounds[1]) * recsize
                #print "Seeking to max offset %d" % (offset)
                ip.seek(offset)
                maxdata = struct.unpack(AWP12_FORMAT, ip.read(recsize))
                #print self.bounds[1], y, z, maxdata

                # Find value at min x-bound
                offset = (z * (self.dims[0] * self.dims[1]) + \
                              y * (self.dims[0]) + self.bounds[0]) * recsize
                #print "Seeking to min offset %d" % (offset)
                ip.seek(offset)
                mindata = struct.unpack(AWP12_FORMAT, ip.read(recsize))
                #print self.bounds[0], y, z, mindata

                # Interpolate interior range for vp, vs, and rho
                ip_vp = numpy.interp(xrange(self.bounds[0]+1, \
                                                self.bounds[1]), \
                                         self.bounds, \
                                         [mindata[0], maxdata[0]])
                ip_vs = numpy.interp(xrange(self.bounds[0]+1, \
                                                self.bounds[1]), \
                                         self.bounds, \
                                         [mindata[1], maxdata[1]])
                ip_rho = numpy.interp(xrange(self.bounds[0]+1, \
                                                 self.bounds[1]), \
                                          self.bounds, \
                                          [mindata[2], maxdata[2]])

                # Write interpolated values back to mesh
                # Note that file pointer is now positioned immediately
                # following min bounds.
                idata = ''
                for x in xrange(self.bounds[0]+1, self.bounds[1]):
                    idata = idata + \
                        struct.pack(AWP12_FORMAT, \
                                        ip_vp[x-(self.bounds[0]+1)], \
                                        ip_vs[x-(self.bounds[0]+1)], \
                                        ip_rho[x-(self.bounds[0]+1)])
                #print "Writing data"
                ip.write(idata)
 
        print "Closing mesh file"
        sys.stdout.flush()
        ip.close()
        return(0)


def usage():
    print "usage: %s <meshfile> <nx> <ny> <nz> <xmin> <xmax> <zmin> <zmax>" % (sys.argv[0])
    return


if __name__ == '__main__':
    if (len(sys.argv) != 9):
        usage()
        sys.exit(1)

    meshfile = sys.argv[1]
    dims = [int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4])]
    bounds = [int(sys.argv[5]), int(sys.argv[6])]
    zrange = [int(sys.argv[7]), int(sys.argv[8])]
    #zrange = [0, dims[2]]
    prog = SmoothMesh2D(meshfile, dims, bounds, zrange)
    sys.exit(prog.main())
