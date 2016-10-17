#ifndef VX_UTILS_H
#define VX_UTILS_H

/* Byte order */
typedef enum { VX_BYTEORDER_LSB = 0, 
               VX_BYTEORDER_MSB } vx_byteorder_t;


/* Determine system endian */
int vx_system_endian();

/* Minimum of two values */
float vx_minf(float v1, float v2);

/* Interpolate between two values */
double vx_interpolate(double v1, double v2, double ratio);

/* 2D distance */
double vx_dist_2d(double x1, double y1, double x2, double y2);

#endif
