#!/bin/env python

import os
import sys

# CVMH LR/CM SW=-120.8620,30.9564, NE= -113.3329,36.6129
#   lon0 : -120.85 (-113.35)
#   lat0 : 30.96 (36.6)
#   nx = 1501
#   ny = 1129
# $ ./gen_grid.py -120.85 30.96 1501 1129 0.005
lon0 = float(sys.argv[1])
lat0 = float(sys.argv[2])
nx = int(sys.argv[3])
ny = int(sys.argv[4])
spacing = float(sys.argv[5])

points = []
for y in xrange(0, ny):
    for x in xrange(0, nx):
        points.append([lon0 + x*spacing, lat0 + y*spacing])


op = open('cvmh_grid_locs.txt', 'w')
for point in points:
    op.write('%.3lf %.3lf\n' % (point[0], point[1]))
op.close()

sys.exit(0)
