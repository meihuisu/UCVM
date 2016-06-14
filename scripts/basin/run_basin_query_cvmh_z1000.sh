#!/bin/bash

BIN_DIR=/home/scec-00/patrices/code/ucvm/src/basin

${BIN_DIR}/basin_query -m cvmh -f ~/opt/aftershock/ucvm-12.2.0/conf/ucvm.conf -v 1000.0 < ./cvm_grid_locs_rev.txt > cvmh_z1000.txt


