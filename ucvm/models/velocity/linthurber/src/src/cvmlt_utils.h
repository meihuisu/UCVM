#ifndef CVMLT_UTILS_H
#define CVMLT_UTILS_H

#include "cvmlt.h"

/* Parses string list into double array */
int cvmlt_list_parse(char *lstr, int n, double *arr);

/* Parses string list into string array */
int cvmlt_list_parse_s(char *lstr, int n, char **arr);

/* Rotate point in 2d about origin by theta radians */
int cvmlt_rot_point_2d(cvmlt_point_t *p, double theta);

/* Interpolate point linearly between two 1d values */
double cvmlt_interpolate_linear(double v1, double v2, double ratio);

/* Interpolate point bilinearly between four corners */
double cvmlt_interpolate_bilinear(double x, double y, 
				  double x1, double y1,
				  double x2, double y2, 
				  double q11, double q21, 
				  double q12, double q22);

/* Interpolate point tri-linearly between 8 cube corners.
   Points are indexed [ll,ur][x,y,z], q is indexed[z][y][x] */
double cvmlt_interpolate_trilinear(double x, double y, double z,
				   double p[2][3], double q[2][2][2]);


/* Density derived from Vp via Nafe-Drake curve, Brocher (2005) eqn 1. */
double cvmlt_nafe_drake_rho(double vp); 


/* Vp derived from Vs via Brocher (2005) eqn 9. */
double cvmlt_brocher_vp(double vs);


#endif
