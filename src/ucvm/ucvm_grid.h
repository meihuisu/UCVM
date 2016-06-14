#ifndef UCVM_GRID_H
#define UCVM_GRID_H

#include "ucvm_dtypes.h"

/* Supported grid types */
typedef enum { UCVM_GRID_CELL_CENTER = 0, 
	       UCVM_GRID_CELL_VERTEX } ucvm_gtype_t;


/* Projection definition */
typedef struct ucvm_projdef_t 
{
  char proj[UCVM_MAX_PROJ_LEN];
} ucvm_projdef_t;


/* Projection transformation */
typedef struct ucvm_trans_t 
{
  double origin[3];
  double rotate;
  double translate[3];
  ucvm_gtype_t gtype;
} ucvm_trans_t;


/* Generate grid from projection and dimensions */
int ucvm_grid_gen(ucvm_projdef_t *iproj, ucvm_trans_t *trans,
		  ucvm_projdef_t *oproj,
		  ucvm_dim_t *dims, double spacing, 
		  ucvm_point_t *pnts);


/* Generate grid from projection and dimensions */
int ucvm_grid_gen_file(ucvm_projdef_t *iproj, ucvm_trans_t *trans,
		       ucvm_projdef_t *oproj,
		       ucvm_dim_t *dims, double spacing, 
		       const char *filename);


/* Convert point list from one projection to another */
int ucvm_grid_convert(ucvm_projdef_t *iproj, 
		      ucvm_projdef_t *oproj, 
		      size_t n, ucvm_point_t *pnts);


/* Convert point list from one projection to another */
int ucvm_grid_convert_file(ucvm_projdef_t *iproj, 
			   ucvm_projdef_t *oproj, 
			   size_t n, const char *filename);


#endif
