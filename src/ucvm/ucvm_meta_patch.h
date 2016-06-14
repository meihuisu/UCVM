#ifndef UCVM_META_PATCH_H
#define UCVM_META_PATCH_H

#include "ucvm_dtypes.h"


/* Patch payload datatype */
typedef struct ucvm_ppayload_t {
  float vp;
  float vs;
  float rho;
} ucvm_ppayload_t;


/* Surface information */
typedef struct ucvm_psurf_t {
  ucvm_point_t corners[2];
  ucvm_dim_t dims;
  int num_points;
  ucvm_ppayload_t *props;
} ucvm_psurf_t;


/* Compute surfaces based off dimensions and spacing */
int ucvm_meta_patch_getsurf(ucvm_point_t *dims, double spacing, 
			    ucvm_psurf_t surfs[2][2]);

#endif
