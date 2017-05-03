#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cvmsi_utils.h"


/* Determine system endian */
int cvmsi_is_little_endian()
{
  int num = 1;
  if(*(char *)&num == 1) {
    return(1);
  } else {
    return(0);
  }
}


/* Swap float endian-ness */
float cvmsi_swap_endian_float(float f)
{
  cvmsi_fdata_t dat1, dat2;

  dat1.f = f;
  dat2.b[0] = dat1.b[3];
  dat2.b[1] = dat1.b[2];
  dat2.b[2] = dat1.b[1];
  dat2.b[3] = dat1.b[0];
  return(dat2.f);
}


/* Strip trailing whitespace from string */
void cvmsi_strip_trailing_whitespace(char *str)
{
  int i;

  i = strlen(str);
  while (strchr(" \t\n", str[i-1]) != NULL) {
    str[i-1] = '\0';
    i = i - 1;
  }
  return;
}


/* Interpolate point linearly between two 1d values */
double cvmsi_interp_linear(double v1, double v2, double ratio) 
{
  return(ratio*v2 + v1*(1-ratio));
}


/* Interpolate point bilinearly between four corners */
double cvmsi_interp_bilinear(double x, double y, 
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
double cvmsi_interp_trilinear(double x, double y, double z,
			      double p[2][3], double q[2][2][2]) 
{
  double c0, c1;
  double ratio;

  /* Top plane */
  c0 = cvmsi_interp_bilinear(x, y,
			     p[0][0], p[0][1],
			     p[1][0], p[1][1],
			     q[0][0][0], q[0][0][1], 
			     q[0][1][0], q[0][1][1]);

  /* Bottom plane */
  c1 = cvmsi_interp_bilinear(x, y,
			     p[0][0], p[0][1],
			     p[1][0], p[1][1],
			     q[1][0][0], q[1][0][1], 
			     q[1][1][0], q[1][1][1]);
  
  /* Z axis */
  ratio = (z - p[0][2])/(p[1][2] - p[0][2]); 
  return(cvmsi_interp_linear(c0, c1, ratio));
}
