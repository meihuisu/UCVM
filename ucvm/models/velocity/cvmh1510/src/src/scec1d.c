/** 
    scec1d.c - Routines that implement SCEC 1D background model
    based on Hadley-Kanamori 1D for southern Calfornia.

    01/2011: PES: Initial implementation
**/

#include <string.h>
#include "stdlib.h"
#include <stdio.h>
#include <math.h>

/* SCEC 1D depth (km) -> vs (km/s) array */
#define MAX_SCEC_1D 9
double scec_layer_depths[MAX_SCEC_1D] = 
  {1.0, 5.0, 6.0, 10.0, 15.5, 16.5, 22.0, 31.0, 33.0};
double scec_layer_vp[MAX_SCEC_1D] = 
  {5.0, 5.5, 6.3, 6.3, 6.4, 6.7, 6.75, 6.8, 7.8};


/* Determine vp by depth */
double scec_vp(double depth) {
  int i;
  double vp;
  double depth_ratio;
  double vp_range;

  /* Convert from m to km */
  depth = depth / 1000.0;

  /* Scale vp by depth with linear interpolation */
  for (i = 0; i < MAX_SCEC_1D; i++) {
    if (scec_layer_depths[i] > depth) {
      if (i == 0) {
	vp = scec_layer_vp[i];
      } else {
	depth_ratio = ((depth - scec_layer_depths[i-1]) / 
		       (scec_layer_depths[i] - scec_layer_depths[i - 1]));
	vp_range = scec_layer_vp[i] - scec_layer_vp[i - 1];
	vp = ((vp_range * depth_ratio) + scec_layer_vp[i - 1]);
      }
      break;
    } 
  }
  if (i == MAX_SCEC_1D) {
    vp = scec_layer_vp[MAX_SCEC_1D - 1];
  }

  /* Convert from km/s back to m/s */
  vp = vp * 1000.0;

  return(vp);
}


/* Determine density by vp */
double scec_rho(double vp) {
  double rho;

  /* Calculate rho */
  rho = 1865.0 + 0.1579 * vp;
  return(rho);
}


/* Determine vs by vp and density */
double scec_vs(double vp, double rho) {
  double vs;
  double nu;

  if (rho < 2060.0) {
    nu = 0.40;
  } else if (rho > 2500.0) {
    nu = 0.25;
  } else {
    nu = 0.40 - ((rho - 2060.0) * 0.15 / 440.0);
  }

  vs = vp * sqrt( (0.5 - nu) / (1.0 - nu) );

  return(vs);
}
