#!/bin/sh
##########################################################
#
# Script : plot_hslice.sh
#
# Description: Produces a map plot of Vp/Vs/Rho at the
#              desired depth/elevation. Supports optional
#              vx_slice flags.
#
# Usage: ./plot_hslice.sh [vx_slice flags] <label> <depth>
#
##########################################################

# Process vx_slice options
VX_FLAGS=""

# Pass along any arguments to vx_slice
while getopts 'm:gsz:' OPTION
do
  if [ "$OPTARG" != "" ]; then
      VX_FLAGS="${VX_FLAGS} -$OPTION $OPTARG"
  else
      VX_FLAGS="${VX_FLAGS} -$OPTION"
  fi
done
shift $(($OPTIND - 1))


label=$1
z_offset=$2
filebase=./${label}_slice_data
spacing=1
# 1 : vp, 2 : vs, 3 : rho
#value_type=vs
#value_max=4500
value_type=vp
value_max=8500

#point1_x=-123
#point1_y=30
#point2_x=-112
#point2_y=39
# LA area
#point1_x=231000
#point1_y=3631000
#point2_x=628000
#point2_y=3858000
#gridsize=100
# SoCal area
point1_x=31000
point1_y=3331000
point2_x=928000
point2_y=4158000
gridsize=500

xmax=`echo "(((${point2_x} - ${point1_x}) / ${gridsize}) + 1)" | bc`
ymax=`echo "(((${point2_y} - ${point1_y}) / ${gridsize}) + 1)" | bc`
echo "Determined slice size to be ${xmax} ${ymax}"

asciifile=${filebase}_${z_offset}.txt
outputfile=${filebase}_${value_type}_xy.ps
cptfile=${filebase}_xy.cpt
projfile=${filebase}_${z_offset}_xy.txt
gridfile=${filebase}_${z_offset}_xy.grd

cd ../../src
echo "./vx_slice ${VX_FLAGS} -r ${gridsize} -f ../viz/${asciifile} -- ${point1_x} ${point1_y} ${point2_x} ${point2_y} ${z_offset} ${value_type}"
./vx_slice ${VX_FLAGS} -r ${gridsize} -f ../scripts/viz/${asciifile} -- ${point1_x} ${point1_y} ${point2_x} ${point2_y} ${z_offset} ${value_type}
cd ../scripts/viz

# General gmt params
#gmtset PAPER_MEDIA letter
gmtset PAPER_MEDIA A5
gmtset LABEL_FONT_SIZE 12
#gmtset BASEMAP_FRAME_RGB 0/0/0
gmtset ANOT_FONT Helvetica
gmtset ANOT_FONT_SIZE 12
gmtset HEADER_FONT Helvetica
gmtset HEADER_FONT_SIZE 12

subimg_xoffset=3
subimg_yoffset=7
overlay=" "

# Create color scale
makecpt -Chot -T0/${value_max}/500 -Z -I > ${cptfile}

cp ${asciifile} ${projfile}

# Convert to GMT grid
xyz2grd ${projfile} -G${gridfile} -I${spacing}/${spacing} -R0/$((xmax-1))/0/$((ymax-1)) -V -Dunits/units/msec/1/0/=/=
	
# Construct basemap
echo "Plotting grid lines and labels for z_offset=${z_offset}"
psbasemap -Jx0.005/0.005 -Bg100a500:"X units":/g100a500:"Y units":WeSn:."CVM-H Horiz Slice ${value_type} (depth=${z_offset})": -R0/$((xmax-1))/0/$((ymax-1)) -X${subimg_xoffset} -Y${subimg_yoffset} -P ${overlay} -K  > ${outputfile}
overlay="-O"

# Plot points
echo "Plotting points for z_offset=${z_offset}"
grdimage ${gridfile} -Jx -C${cptfile} -P ${overlay} -K >> ${outputfile}

# Color scale
echo "Adding color scale"
psscale -C${cptfile} -D0.0/0.0/12/1h -Bg250a1000:"":/g250a1000:"":weSn:."${value_type} Scale": -P -O -X4.0 -Y-2.5 >> ${outputfile}

# Convert to PNG
echo "Converting to PNG, saving as ${outputfile}"
ps2raster ${outputfile} -Tg

exit 0
