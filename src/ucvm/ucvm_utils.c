#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ucvm_utils.h"

/* Region coord delimiter */
#define REGION_DELIM ","

/* List delimiter */
#define LIST_DELIM ","


/* Returns true if path is a file */
int ucvm_is_file(const char *path)
{
  struct stat st;

  if (stat(path, &st) == 0) {
    if ((S_ISREG(st.st_mode)) && (!S_ISDIR(st.st_mode))) {
      return(1);
    } else {
      return(0);
    }
  }

  return(0);
}


/* Determine system endian */
int system_endian()
{
  int num = 1;
  if(*(char *)&num == 1) {
    return UCVM_BYTEORDER_LSB;
  } else {
    return UCVM_BYTEORDER_MSB;
  }
}


/* Swap float endian-ness */
float swap_endian_float(float f)
{
  ucvm_fdata_t dat1, dat2;

  dat1.f = f;
  dat2.b[0] = dat1.b[3];
  dat2.b[1] = dat1.b[2];
  dat2.b[2] = dat1.b[1];
  dat2.b[3] = dat1.b[0];
  return(dat2.f);
}


/* Safe string copy */
int ucvm_strcpy(char *str1, const char *str2, int str1len)
{
  if (str1 == NULL) {
    return(UCVM_CODE_ERROR);
  }

  if (str2 == NULL) {
    strcpy(str1, "");
    return(UCVM_CODE_ERROR);
  }

  if (snprintf(str1, str1len, "%s", str2) > str1len) {
    fprintf(stderr, "Warning (ucvm_strcpy): String %s truncated to %s\n",
	    str2, str1);
  }
  return(UCVM_CODE_SUCCESS);
}


/* Parses string list into double array */
int list_parse(const char *lstr, int llen, double *arr, int an)
{
  char *token;
  char *strbuf;
  int i = 0;

  if ((lstr == NULL) || (llen <= 0) || 
      (arr == NULL) || (an <= 0)) {
    return(UCVM_CODE_ERROR);
  }

  strbuf = malloc(llen);
  if (strbuf == NULL) {
    return(UCVM_CODE_ERROR);
  }

  ucvm_strcpy(strbuf, lstr, llen);
  token = strtok(lstr, LIST_DELIM);
  while ((token != NULL) && (i < an)) {
    arr[i++] = atof(token);
    token = strtok(NULL, LIST_DELIM);
  }

  free(strbuf);
  return(UCVM_CODE_SUCCESS);
}


/* Parses string list into string array */
int list_parse_s(const char *lstr, int llen, 
		 char **arr, int an, int alen)
{
  char *token;
  char *strbuf;
  int i = 0;

  if ((lstr == NULL) || (llen <= 0) || 
      (arr == NULL) || (an <= 0) || (alen <= 0)) {
    return(UCVM_CODE_ERROR);
  }

  strbuf = malloc(llen);
  if (strbuf == NULL) {
    return(UCVM_CODE_ERROR);
  }

  ucvm_strcpy(strbuf, lstr, llen);
  token = strtok(strbuf, LIST_DELIM);
  while ((token != NULL) && (i < an)) {
    ucvm_strcpy(arr[i++], token, alen);
    token = strtok(NULL, LIST_DELIM);
  }

  free(strbuf);
  return(UCVM_CODE_SUCCESS);
}


/* Returns true if region contains point p of coord type c */
int region_contains(ucvm_region_t *r, ucvm_ctype_t c, ucvm_point_t *p)
{
  if (c != r->cmode) {
    fprintf(stderr, "Coord type conversion not supported in region_contains().\n");
    return(0);
  }

  if ((p->coord[0] < r->p1[0]) || (p->coord[0] > r->p2[0])) {
    return(0);
  }
  if ((p->coord[1] < r->p1[1]) || (p->coord[1] > r->p2[1])) {
    return(0);
  }
  return(1);
}


/* Returns true */
int region_contains_null(ucvm_region_t *r, ucvm_ctype_t c, ucvm_point_t *p)
{
  return(1);
}


/* Parses string region specification into structure struct */
int region_parse(char *rstr, ucvm_region_t *r)
{
  char *token;
  int i = 0;

  r->p1[2] = 0.0;
  r->p2[2] = 0.0;

  token = strtok(rstr, REGION_DELIM);
  while ((token != NULL) && (i < 4)) {
    switch(i) {
    case 0:
      r->p1[0] = atof(token);
      break;
    case 1:
      r->p1[1] = atof(token);
      break;
    case 2:
      r->p2[0] = atof(token);
      break;
    case 3:
      r->p2[1] = atof(token);
      return(UCVM_CODE_SUCCESS);
      break;
    }
    i++;
    token = strtok(NULL, REGION_DELIM);
  }

  return(UCVM_CODE_ERROR);
}


/* Parses region structure into string*/
int region_string(ucvm_region_t *r, char *rstr, int len)
{
  snprintf(rstr, len, "lon=%lf, lat=%lf to lon=%lf, lat=%lf",
	   r->p1[0], r->p1[1], r->p2[0], r->p2[1]);
  return(UCVM_CODE_SUCCESS);
}


/* Rotate point in 2d about origin by theta radians */
int rot_point_2d(ucvm_point_t *p, double theta)
{
  double x, y;

  x = p->coord[0];
  y = p->coord[1];

  /* Rotate this offset */
  //printf("x_offset=%lf, y_offset=%lf\n", x_offset, y_offset);
  p->coord[0] = (x) * cos(theta) - (y) * sin(theta);
  p->coord[1] = (x) * sin(theta) + (y) * cos(theta);

  return(UCVM_CODE_SUCCESS);
}


/* Interpolate point linearly between two 1d values */
double interpolate_linear(double v1, double v2, double ratio) 
{
  return(ratio*v2 + v1*(1-ratio));
}



/* Interpolate point bilinearly between four corners */
double interpolate_bilinear(double x, double y, 
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
double interpolate_trilinear(double x, double y, double z,
			     double p[2][3], double q[2][2][2]) 
{
  double c0, c1;
  double ratio;

  /* Top plane */
  c0 = interpolate_bilinear(x, y,
			       p[0][0], p[0][1],
			       p[1][0], p[1][1],
			       q[0][0][0], q[0][0][1], 
			       q[0][1][0], q[0][1][1]);

  /* Bottom plane */
  c1 = interpolate_bilinear(x, y,
			       p[0][0], p[0][1],
			       p[1][0], p[1][1],
			       q[1][0][0], q[1][0][1], 
			       q[1][1][0], q[1][1][1]);

  /* Z axis */
  ratio = (z - p[0][2])/(p[1][2] - p[0][2]); 
  return(interpolate_linear(c0, c1, ratio));
}


/* Density derived from Vp via Nafe-Drake curve, Brocher (2005) eqn 1. */
double ucvm_nafe_drake_rho(double vp) 
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
double ucvm_brocher_vp(double vs) 
{
  double vp;

  vs = vs * 0.001;
  vp = 0.9409 + vs * (2.0947 - vs * (0.8206 - vs * (0.2683 - vs * 0.0251)));
  vp = vp * 1000.0;
  return(vp);
}
