#!/bin/sh

PREFIX_DIR=$1

# GTS Library installation path
GTS_DIR=/home/rcf-104/patrices/opt/gts-0.7.6

# Recommended compilers/flags
CC=gcc

# Run configuration script
cd ../..
./configure --prefix=$1 CC="${CC}" --with-gts --with-gts-include-path="${GTS_DIR}/include" --with-gts-lib-path="${GTS_DIR}/lib"
