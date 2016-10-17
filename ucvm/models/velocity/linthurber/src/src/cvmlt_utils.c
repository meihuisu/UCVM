#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "cvmlt_utils.h"


/* List delimiter */
#define CVMLT_LIST_DELIM ","


/* Parses string list into double array */
int cvmlt_list_parse(char *lstr, int n, double *arr)
{
  char *token;
  int i = 0;

  if ((lstr == NULL) || (n <= 0) || (arr == NULL)) {
    return(1);
  }

  token = strtok(lstr, CVMLT_LIST_DELIM);
  while ((token != NULL) && (i < n)) {
    arr[i] = atof(token);
    i++;
    token = strtok(NULL, CVMLT_LIST_DELIM);
  }

  return(0);
}


/* Parses string list into string array */
int cvmlt_list_parse_s(char *lstr, int n, char **arr)
{
  char *token;
  int i = 0;

  if ((lstr == NULL) || (n <= 0) || (arr == NULL)) {
    return(1);
  }

  token = strtok(lstr, CVMLT_LIST_DELIM);
  while ((token != NULL) && (i < n)) {
    strcpy(arr[i], token);
    i++;
    token = strtok(NULL, CVMLT_LIST_DELIM);
  }

  return(0);
}


/* Rotate point in 2d about origin by theta radians */
int cvmlt_rot_point_2d(cvmlt_point_t *p, double theta)
{
  double x, y;

  x = p->coord[0];
  y = p->coord[1];

  /* Rotate this offset */
  p->coord[0] = (x) * cos(theta) - (y) * sin(theta);
  p->coord[1] = (x) * sin(theta) + (y) * cos(theta);

  return(0);
}


/* Interpolate point linearly between two 1d values */
double cvmlt_interpolate_linear(double v1, double v2, double ratio) 
{
  return(ratio*v2 + v1*(1-ratio));
}



/* Interpolate point bilinearly between four corners */
double cvmlt_interpolate_bilinear(double x, double y, 
			       double x1, double y1, double x2, double y2, 
			       double q11, double q21, double q12, double q22)
{
  double p = (x2 - x1) * (y2 - y1);
  double f1 = (q11 / p) * (x2 - x) * (y2 - y);
  double f2 = (q21 / p) * (x - x1) * (y2 - y);
  double f3 = (q12 / p) * (x2 - x) * (y - y1);
  double f4 = (q22 / p) * (x - x1) * (y - y1);
  return f1 + f2 + f3 + f4;
}


/* Interpolate point tri-linearly between 8 cube corners.
   Points are indexed [ll,ur][x,y,z], q is indexed[z][y][x] */
double cvmlt_interpolate_trilinear(double x, double y, double z,
				double p[2][3], double q[2][2][2]) 
{
  double c0, c1;
  double ratio;

  /* Top plane */
  c0 = cvmlt_interpolate_bilinear(x, y,
				  p[0][0], p[0][1],
				  p[1][0], p[1][1],
				  q[0][0][0], q[0][0][1], 
				  q[0][1][0], q[0][1][1]);
  
  /* Bottom plane */
  c1 = cvmlt_interpolate_bilinear(x, y,
				  p[0][0], p[0][1],
				  p[1][0], p[1][1],
				  q[1][0][0], q[1][0][1], 
				  q[1][1][0], q[1][1][1]);

  /* Z axis */
  ratio = (z - p[0][2])/(p[1][2] - p[0][2]); 
  return(cvmlt_interpolate_linear(c0, c1, ratio));
}


/* Density derived from Vp via Nafe-Drake curve, Brocher (2005) eqn 1. */
double cvmlt_nafe_drake_rho(double vp) 
{
  double rho;

  /* Convert m to km */
  vp = vp * 0.001;
  rho = vp * (1.6612 - vp * (0.4721 - vp * (0.0671 - vp * (0.0043 - vp * 0.000106))));
  if (rho < 1.0) {
    rho = 1.0;
  }
  rho = rho * 1000.0;
  return(rho);
}


/* Vp derived from Vs via Brocher (2005) eqn 9. */
double cvmlt_brocher_vp(double vs) 
{
  double vp;

  vs = vs * 0.001;
  vp = 0.9409 + vs * (2.0947 - vs * (0.8206 - vs * (0.2683 - vs * 0.0251)));
  vp = vp * 1000.0;
  return(vp);
}
