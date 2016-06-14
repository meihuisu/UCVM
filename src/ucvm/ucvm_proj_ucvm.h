#ifndef UCVM_PROJ_UCVM_H
#define UCVM_PROJ_UCVM_H

#include "ucvm_dtypes.h"
#include "proj_api.h"


/* Projection parameters */
typedef struct ucvm_proj_t 
{
  projPJ ipj;
  projPJ opj;
  ucvm_point_t p1;
  double rot;
  ucvm_point_t size;
  ucvm_point_t offset;
} ucvm_proj_t;


/* Initialize projection */
int ucvm_proj_ucvm_init(const char* pstr, const ucvm_point_t *o, double r,
			ucvm_point_t *s, ucvm_proj_t *p);

/* Finalize projection */
int ucvm_proj_ucvm_finalize(ucvm_proj_t *p);

/* Convert lon,lat to x,y */
int ucvm_proj_ucvm_geo2xy(ucvm_proj_t *p, ucvm_point_t *geo, ucvm_point_t *xy);


/* Convert x,y to lon,lat */
int ucvm_proj_ucvm_xy2geo(ucvm_proj_t *p, ucvm_point_t *xy, ucvm_point_t *geo);


#endif
