#ifndef UCVM_PROJ_BILINEAR_H
#define UCVM_PROJ_BILINEAR_H


/* Bilinear parameters */
typedef struct ucvm_bilinear_t 
{
  double xi[4];
  double yi[4];
  double dims[2];
} ucvm_bilinear_t;


/* Convert lon,lat to x,y */
int ucvm_bilinear_geo2xy(ucvm_bilinear_t *par,
			 ucvm_point_t *geo, ucvm_point_t *xy);


/* Convert x,y to lon,lat */
int ucvm_bilinear_xy2geo(ucvm_bilinear_t *p,
			 ucvm_point_t *xy, ucvm_point_t *geo);



#endif
