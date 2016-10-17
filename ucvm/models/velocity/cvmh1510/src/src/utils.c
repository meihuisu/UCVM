/** 
    utils.c - Commonly used math and interpolation routines.

    01/2011: PES: Initial implementation
**/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "utils.h"


/* Determine system endian */
int vx_system_endian()
{
  int num = 1;
  if(*(char *)&num == 1) {
    return VX_BYTEORDER_LSB;
  } else {
    return VX_BYTEORDER_MSB;
  }
}


/*
 * vx_minf
 *
 * Return min(v1, v2)
 *
 * Get minimum of two float values
 *
 */
float vx_minf(float v1, float v2)
{
  if (v1 < v2)
    return v1;
  else
    return v2;
}


/* Interpolate value linearly between two end points */
double vx_interpolate(double v1, double v2, double ratio) {
  return(ratio*v2 + v1*(1-ratio));
}


/* 2D Distance */
double vx_dist_2d(double x1, double y1, double x2, double y2) {
  return(sqrt(pow(x2-x1, 2.0) + pow(y2-y1, 2.0)));
}
