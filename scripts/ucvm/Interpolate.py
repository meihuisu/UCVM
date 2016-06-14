#!/usr/bin/env python

# Basic modules
import os
import sys
import math

# Constants
csii =  [-1.0, -1.0, 1.0,  1.0 ]
ethai = [-1.0,  1.0, 1.0, -1.0 ]

# Interpolate
class Interpolate:
    def __init__(self, xi, yi, dims):
        self.valid = False
        self.xi = xi
        self.yi = yi
        self.dims = dims
        self.valid = True

    def isValid(self):
        return self.valid

    def cleanup(self):
        return

    def bilinear(self, x, y, x1, y1, x2, y2, q11, q21, q12, q22):

        p = (x2 - x1) * (y2 - y1)
        f1 = (q11 / p) * (x2 - x) * (y2 - y)
        f2 = (q21 / p) * (x - x1) * (y2 - y)
        f3 = (q12 / p) * (x2 - x) * (y - y1)
        f4 = (q22 / p) * (x - x1) * (y - y1)
        return (f1 + f2 + f3 + f4)


    def ibilinear(lon, lat):
        x = 0.0
        y = 0.0
        k = 0
        xce = 0.0
        yce = 0.0
        j1 =   [0.0, 0.0, 0.0, 0.0,]
        j2 =   [0.0, 0.0, 0.0, 0.0,]
        j =    [0.0, 0.0, 0.0, 0.0,]
        jinv = [0.0, 0.0, 0.0, 0.0,]
        d = 0.0
        x0 = 0.0
        y0 = 0.0
        p = 0.0
        q = 0.0
        dx = 0.0
        dy = 0.0
        res = 1.0
    
        for i in xrange(0, 4):
            j1[0] = j1[0] + self.xi[i] * csii[i];
            j1[1] = j1[1] + self.xi[i] * ethai[i];
            j1[2] = j1[2] + self.yi[i] * csii[i];
            j1[3] = j1[3] + self.yi[i] * ethai[i];
            xce += self.xi[i] * csii[i] * ethai[i];
            yce += self.yi[i] * csii[i] * ethai[i];
     

        while (True):
            k = k + 1

            j2[0] = y * xce
            j2[1] = x * xce
            j2[2] = y * yce
            j2[3] = x * yce
        
            j[0] = .25 * (j1[0] + j2[0])
            j[1] = .25 * (j1[1] + j2[1])
            j[2] = .25 * (j1[2] + j2[2])
            j[3] = .25 * (j1[3] + j2[3])

            d = (j[0]*j[3]) - (j[2]*j[1])
            jinv[0] =  j[3] / d
            jinv[1] = -j[1] / d
            jinv[2] = -j[2] / d
            jinv[3] =  j[0] / d

            x0 = 0
            y0 = 0

            for i in xrange(0, 4):
                x0 = x0 + self.xi[i] * (.25 * (1 + (csii[i]  * x))
                                         * (1 + (ethai[i] * y)))
                y0 = y0 + self.yi[i] * (.25 * (1 + (csii[i]  * x))
                                         * (1 + (ethai[i] * y)))

            p = lon - x0
            q = lat - y0
            dx = (jinv[0]*p) + (jinv[1]*q)
            dy = (jinv[2]*p) + (jinv[3]*q)

            x = x + dx
            y = y + dy

            res = dx*dx + dy*dy

            if ((res > 1e-12) and (k < 10)):
                break
        

        if (k >= 10):
            print "Unable to convert %lf %lf\n" % (lat,lon)
            return (-1.0,-1.0);
    
        x = (x + 1) * self.dims[0]/2.0
        y = (y + 1) * self.dims[1]/2.0

        return (x,y)


    def binterp(self, x, y, inverse=False):
        newx = 0.0
        newy = 0.0

        if (inverse):
            newy = self.bilinear(x, y, \
                                     0.0, 0.0, self.dims[0], self.dims[1], \
                                     self.yi[0], self.yi[3], \
                                     self.yi[1], self.yi[2])

            newx = self.bilinear(x, y, \
                                     0.0, 0.0, self.dims[0], self.dims[1], \
                                     self.xi[0], self.xi[3], \
                                     self.xi[1], self.xi[2])
        
        else:
            newx, newy = self.ibininear(x, y)

        return(newx,newy)
