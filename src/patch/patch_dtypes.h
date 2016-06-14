#ifndef PATCH_DTYPES_H
#define PATCH_DTYPES_H

#include "ucvm.h"
#include "ucvm_config.h"
#include "ucvm_proj_ucvm.h"
#include "ucvm_meta_patch.h"


/* Projection parameters datatype, supports UCVM */
typedef struct patch_projinfo_t {
  char projstr[UCVM_MAX_PROJ_LEN];
  ucvm_point_t corner;
  ucvm_point_t dims;
  double rot;
  ucvm_proj_t proj;
} patch_projinfo_t;


/* patchmodel configuration */
typedef struct patch_cfg_t {
  char version[UCVM_MAX_VERSION_LEN];
  patch_projinfo_t projinfo;
  char ucvmstr[UCVM_CONFIG_MAX_STR];
  double ucvm_zrange[2];
  char ucvmconf[UCVM_MAX_PATH_LEN];
  double spacing;
  char modelname[UCVM_CONFIG_MAX_STR];
  char modelpath[UCVM_MAX_PATH_LEN];
  ucvm_psurf_t surfs[2][2];
} patch_cfg_t;


#endif
