#ifndef CVMLT_PROJ_BILINEAR_H
#define CVMLT_PROJ_BILINEAR_H

#include "cvmlt.h"

/* Bilinear parameters */
typedef struct cvmlt_bilinear_t 
{
  double xi[4];
  double yi[4];
  double dims[2];
} cvmlt_bilinear_t;


/* Convert lon,lat to x,y */
int cvmlt_bilinear_geo2xy(cvmlt_bilinear_t *par,
			  cvmlt_point_t *geo, cvmlt_point_t *xy);


/* Convert x,y to lon,lat */
int cvmlt_bilinear_xy2geo(cvmlt_bilinear_t *p,
			  cvmlt_point_t *xy, cvmlt_point_t *geo);



#endif
