#!/bin/bash

for x in `seq -120.5 0.1 -113.5`; do \
 for y in `seq 31 0.1 36.5`; do \
  for z in `seq 0 100 10000`; do \
   echo $x $y $z >> out.grd; \
  done; \
 done; \
done
