#!/bin/bash

BIN_DIR=/home/scec-00/patrices/code/ucvm/src/basin

${BIN_DIR}/basin_query -m cvmh -f ~/opt/aftershock/ucvm-12.2.0/conf/ucvm.conf -v 2500.0 < ./cvmh_grid_locs.txt > cvmh_z2500_cvmhgrid.txt


