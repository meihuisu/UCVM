#!/bin/bash

#
# Convert GMT formatted Wald vs30 data to ArcGIS gridfloat
#

GMTFILE="$1.grd"
TXTFILE="$1.txt"
GRDFILE="$1.flt"
HDRFILE="$1.hdr"

if [ ! -e ${GMTFILE} ]; then
    echo "GMT grid file ${GMTFILE} does not exist"
    exit 0
fi

if [ ! -e ${HDRFILE} ]; then
    echo "Header file ${HDRFILE} does not exist, dumping info for you"
    grdinfo ${GMTFILE} > ${GMTFILE}.info
    exit 0
fi


grd2xyz ${GMTFILE} > ${TXTFILE}
./txt2float.py ${HDRFILE} ${TXTFILE} ${GRDFILE}

rm ${TXTFILE}

