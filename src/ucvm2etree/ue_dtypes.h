#ifndef UE_DTYPES_H
#define UE_DTYPES_H

#include <stdio.h>
#ifdef UE_ENABLE_MPI
#include "mpi.h"
#endif
#include "etree.h"
#include "ucvm.h"
#include "ucvm_config.h"
#include "ucvm_proj_bilinear.h"
#include "ucvm_proj_ucvm.h"
#include "ucvm_meta_etree.h"


/* Geo-bilinear projection */
#define PROJ_GEO_BILINEAR "geo-bilinear"

/* Maximum etree key size */
#define UE_MAX_KEYSIZE 16


/* Flat-file etree schema */
typedef struct ue_flatfile_t {
  char key[UE_MAX_KEYSIZE];
  ucvm_epayload_t payload;
} ue_flatfile_t;


/* Octant information */
typedef struct ue_octant_t {
  etree_addr_t addr;
  char key[UE_MAX_KEYSIZE];
  ucvm_epayload_t payload;
} ue_octant_t;


/* Extraction dispatch information */
typedef struct ue_dispatch_t {
  int col;
  int status;
  unsigned long octcount;
} ue_dispatch_t;


/* Projection parameters datatype, supports both bilinear and UCVM */
typedef struct ue_projinfo_t {
  char projstr[UCVM_MAX_PROJ_LEN];
  ucvm_point_t corner[4];
  ucvm_point_t dims;
  double rot;
  ucvm_bilinear_t projb;
  ucvm_proj_t proj;
} ue_projinfo_t;


/* Etree parameters datatype */
typedef struct ue_etree_t {
  double max_length;
  double min_edgesize;
  double max_edgesize;
  int min_level;
  int max_level;
  etree_tick_t max_ticks[3];
  etree_tick_t col_ticks[3];
  double ticksize;
  char title[UCVM_META_MAX_STRING_LEN];
  char author[UCVM_META_MAX_STRING_LEN];
  char date[UCVM_META_MAX_STRING_LEN];
  char outputfile[UCVM_MAX_PATH_LEN];
  char format[UCVM_CONFIG_MAX_STR];
  etree_t *ep[2];
  FILE *efp[2];
  ue_octant_t *bufp[2];
  int num_octants[2];
  int max_octants;
} ue_etree_t;


/* ucvm2etree configuration */
typedef struct ue_cfg_t {
  ue_projinfo_t projinfo;
  ucvm_dim_t col_dims;
  double max_freq;
  double ppwl;
  double max_octsize;
  double vs_min;
  char ucvmstr[UCVM_CONFIG_MAX_STR];
  double ucvm_zrange[2];
  char ucvmconf[UCVM_CONFIG_MAX_STR];
  ue_etree_t ecfg;
  int rank;
  int nproc;
  char scratch[UCVM_CONFIG_MAX_STR];
  int buf_etree_cache;
  int buf_extract_mem_max_oct;
  int buf_extract_ffile_max_oct;
  int buf_sort_ffile_max_oct;
  int buf_merge_report_min_oct;
  int buf_merge_sendrecv_buf_oct;
  int buf_merge_io_buf_oct;
  #ifdef UE_ENABLE_MPI
  MPI_Datatype MPI_OCTANT;
  #endif
} ue_cfg_t;


/* Dispatch tracking types */
typedef int ue_rank_t;
typedef unsigned long ue_oct_t;


#endif
