#ifndef UCVM_UTILS_H
#define UCVM_UTILS_H

#include "ucvm_dtypes.h"

/* Returns true if path is a file */
int ucvm_is_file(const char *path);

/* Determine system endian */
int system_endian();

/* Swap float endian */
float swap_endian_float(float f);

/* Safe string copy */
int ucvm_strcpy(char *str1, const char *str2, int str1len);

/* Parses string list into double array */
int list_parse(const char *lstr, int llen, double *arr, int an);

/* Parses string list into string array of an elements of alen chars */
int list_parse_s(const char *lstr, int llen, 
		 char **arr, int an, int alen);

/* Returns true if region contains point p of coord type c */
int region_contains(ucvm_region_t *r, ucvm_ctype_t c, ucvm_point_t *p);

/* Returns true */
int region_contains_null(ucvm_region_t *r, ucvm_ctype_t c, ucvm_point_t *p);

/* Parses string region into structure */
int region_parse(char *rstr, ucvm_region_t *r);

/* Parses region structure into string */
int region_string(ucvm_region_t *r, char *rstr, int len);

/* Rotate point in 2d about origin by theta radians */
int rot_point_2d(ucvm_point_t *p, double theta);

/* Interpolate point linearly between two 1d values */
double interpolate_linear(double v1, double v2, double ratio);

/* Interpolate point bilinearly between four corners */
double interpolate_bilinear(double x, double y, 
			       double x1, double y1, double x2, double y2, 
			       double q11, double q21, double q12, double q22);

/* Interpolate point tri-linearly between 8 cube corners.
   Points are indexed [ll,ur][x,y,z], q is indexed[z][y][x] */
double interpolate_trilinear(double x, double y, double z,
				double p[2][3], double q[2][2][2]);


/* Density derived from Vp via Nafe-Drake curve, Brocher (2005) eqn 1. */
double ucvm_nafe_drake_rho(double vp); 


/* Vp derived from Vs via Brocher (2005) eqn 9. */
double ucvm_brocher_vp(double vs);


#endif
