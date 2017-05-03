#ifndef CVMSI_UTILS_H
#define CVMSI_UTILS_H

/* For swapping endian */
typedef union cvmsi_fdata_t {
    float f;
    unsigned char b[4];
} cvmsi_fdata_t;


/* Determine system endian */
int cvmsi_is_little_endian();

/* Swap float endian-ness */
float cvmsi_swap_endian_float(float f);

/* Strip trailing whitespace from string */
void cvmsi_strip_trailing_whitespace(char *str);

/* Interpolate point linearly between two 1d values */
double cvmsi_interp_linear(double v1, double v2, double ratio);

/* Interpolate point bilinearly between four corners */
double cvmsi_interp_bilinear(double x, double y, 
			     double x1, double y1, double x2, double y2, 
			     double q11, double q21, double q12, double q22);

/* Interpolate point tri-linearly between 8 cube corners.
   Points are indexed [ll,ur][x,y,z], q is indexed[z][y][x] */
double cvmsi_interp_trilinear(double x, double y, double z,
			      double p[2][3], double q[2][2][2]);

#endif

