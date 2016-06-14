#!/bin/env python

import os
import sys

ip = open('socal_station_list_bdeps.txt')
lines = ip.readlines()
ip.close()
values = []
for line in lines:
    if (line[0] == '#'):
        continue
    tokens = line.split()
    l = len(tokens)
    val = [float(tokens[l-10]), float(tokens[l-9]), \
               float(tokens[l-8]), float(tokens[l-7]), \
               float(tokens[l-6]), float(tokens[l-5]), \
               float(tokens[l-4]), float(tokens[l-3]), \
               float(tokens[l-2]), float(tokens[l-1]), tokens[:l-12]]
    values.append(val)

op = open('basins.in', 'w')
for val in values:
    op.write('%lf %lf\n' % (val[1], val[0]))
op.close()

for model in ['cvmh', 'cvms']:
    for vel in [1000, 2500]:
        os.system('../../src/basin/basin_query -m %s -f ~/opt/aftershock/ucvm-12.2.0/conf/ucvm.conf -v %d < ./basins.in > socal_%s_z%d.txt' % (model, vel, model, vel))

        ip = open('socal_%s_z%d.txt' % (model, vel), 'r')
        lines = ip.readlines()
        ip.close()

        if (len(lines) != len(values)):
            print "Output length mismatch"
            sys.exit(1)

        i = 0
        for line in lines:
            tokens = line.split()
            umin = float(tokens[2])
            umax = float(tokens[3])
            if (umin <= 0.0):
                umin = -99999.0
            if (umax <= 0.0):
                umax = -99999.0
            if (model == 'cvms'):
                if (vel == 1000):
                    modelmin = values[i][2]
                    modelmax = values[i][3]
                else:
                    modelmin = values[i][4]
                    modelmax = values[i][5]
            else:
                if (vel == 1000):
                    modelmin = values[i][6]
                    modelmax = values[i][7]
                else:
                    modelmin = values[i][8]
                    modelmax = values[i][9]

            if (abs(modelmin - umin) > 1.0) or (abs(modelmax - umax) > 1.0):
                print "Mismatch line %d, %s, z=%d: rob=%.1lf/%.1lf ucvm=%.1lf/%.1lf" % (i+1, model, vel, modelmin, modelmax, umin, umax)
                print "\tName: %s, %lf %lf" % (str(' '.join(values[i][10])), \
                                                   values[i][1], values[i][0])
            i = i + 1

sys.exit(0)
