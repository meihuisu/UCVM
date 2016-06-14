#ifndef UM_DTYPES_H
#define UM_DTYPES_H

#include "ucvm.h"
#include "ucvm_grid.h"
#include "um_mesh.h"


/* Mesh Config */
typedef struct mesh_config_t 
{
  int rank;
  char ucvmstr[UCVM_MAX_LABEL_LEN];
  double ucvm_zrange[2];
  char ucvmconf[UCVM_MAX_PATH_LEN];
  ucvm_gtype_t gridtype;
  double spacing;
  char proj[UCVM_MAX_PROJ_LEN];
  double rot;
  ucvm_point_t origin;
  ucvm_dim_t dims;
  ucvm_dim_t proc_dims;
  double vs_min, vp_min;
  char meshfile[UCVM_MAX_PATH_LEN];
  char gridfile[UCVM_MAX_PATH_LEN];
  mesh_format_t meshtype;
  char scratch[UCVM_MAX_PATH_LEN];
} mesh_config_t;


#endif
