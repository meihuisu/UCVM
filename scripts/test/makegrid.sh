#!/bin/sh

HOR_SPACING=0.1
VERT_SPACING=250
MIN_DEPTH=0
MAX_DEPTH=20000
MIN_LON=-130.0
MAX_LON=-110.0
MIN_LAT=26.0
MAX_LAT=46.0

rm -f out.grd

echo "Generating grid"

for z in `seq ${MIN_DEPTH} ${VERT_SPACING} ${MAX_DEPTH}`; do \
 for y in `seq ${MIN_LAT} ${HOR_SPACING} ${MAX_LAT}`; do \
  for x in `seq ${MIN_LON} ${HOR_SPACING} ${MAX_LON}`; do \
   echo $x $y $z >> out.grd; \
  done; \
 done; \
done;

echo "Done"
