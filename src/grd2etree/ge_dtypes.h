#ifndef GE_DTYPES_H
#define GE_DTYPES_H

#include <stdio.h>
#include "etree.h"
#include "grd.h"
#include "ucvm_config.h"
#include "ucvm_proj_ucvm.h"
#include "ucvm_meta_etree.h"


/* Projection parameters datatype, supports UCVM */
typedef struct ge_projinfo_t {
  char projstr[UCVM_MAX_PROJ_LEN];
  ucvm_point_t corner;
  ucvm_point_t dims;
  double rot;
  ucvm_proj_t proj;
} ge_projinfo_t;


/* Etree parameters datatype */
typedef struct ge_etree_t {
  double max_length;
  int level;
  double octsize;
  ucvm_dim_t oct_dims;
  etree_tick_t max_ticks[3];
  double ticksize;
  char title[UCVM_META_MAX_STRING_LEN];
  char author[UCVM_META_MAX_STRING_LEN];
  char date[UCVM_META_MAX_STRING_LEN];
  char outputfile[UCVM_MAX_PATH_LEN];
  etree_t *ep;
  ucvm_mpayload_t *bufp;
  int num_octants;
  int max_octants;
} ge_etree_t;


/* grd2etree configuration */
typedef struct ge_cfg_t {
  ge_projinfo_t projinfo;
  double spacing;
  char elev_hr_dir[UCVM_MAX_PATH_LEN];
  char elev_lr_dir[UCVM_MAX_PATH_LEN];
  char vs30_hr_dir[UCVM_MAX_PATH_LEN];
  char vs30_lr_dir[UCVM_MAX_PATH_LEN];
  ge_etree_t ecfg;
} ge_cfg_t;


#endif
