#ifndef UCVM_META_ETREE_H
#define UCVM_META_ETREE_H

#include "ucvm_dtypes.h"


/* Constants */
#define UCVM_META_MAX_STRING_LEN 64
#define UCVM_META_MIN_SCHEMA_LEN 128
#define UCVM_META_MIN_META_LEN 1024


/* CMU metadata */
typedef struct ucvm_meta_cmu_t {
  char title[UCVM_META_MAX_STRING_LEN];
  char author[UCVM_META_MAX_STRING_LEN];
  char date[UCVM_META_MAX_STRING_LEN];
  ucvm_point_t origin;
  ucvm_point_t dims_xyz;
  ucvm_dim_t ticks_xyz;
} ucvm_meta_cmu_t;


/* UCVM model metadata */
typedef struct ucvm_meta_ucvm_t {
  char title[UCVM_META_MAX_STRING_LEN];
  char author[UCVM_META_MAX_STRING_LEN];
  char date[UCVM_META_MAX_STRING_LEN];
  double vs_min;
  double max_freq;
  double ppwl;
  char projstr[UCVM_MAX_PROJ_LEN];
  ucvm_point_t origin;
  double rot;
  ucvm_point_t dims_xyz;
  ucvm_dim_t ticks_xyz;
} ucvm_meta_ucvm_t;


/* UCVM map metadata */
typedef struct ucvm_meta_map_t {
  char title[UCVM_META_MAX_STRING_LEN];
  char author[UCVM_META_MAX_STRING_LEN];
  char date[UCVM_META_MAX_STRING_LEN];
  double spacing;
  char projstr[UCVM_MAX_PROJ_LEN];
  ucvm_point_t origin;
  double rot;
  ucvm_point_t dims_xyz;
  ucvm_dim_t ticks_xyz;
} ucvm_meta_map_t;


/* UCVM Etree payload datatype */
typedef struct ucvm_epayload_t {
  float Vp;
  float Vs;
  float density;
} ucvm_epayload_t;


/* Map Etree payload datatype */
typedef struct ucvm_mpayload_t {
  float surf;
  float vs30;
} ucvm_mpayload_t;


/* Pack CMU Etree schema */
int ucvm_schema_etree_cmu_pack(char *schemastr, int len);

/* Pack UCVM Etree schema */
int ucvm_schema_etree_ucvm_pack(char *schemastr, int len);

/* Pack Map Etree schema */
int ucvm_schema_etree_map_pack(char *schemastr, int len);


/* Pack CMU Etree metadata */
int ucvm_meta_etree_cmu_pack(ucvm_meta_cmu_t *meta, char *metastr, int len);

/* Unpack CMU Etree metadata */
int ucvm_meta_etree_cmu_unpack(char *metastr, ucvm_meta_cmu_t *meta);


/* Pack UCVM Etree metadata */
int ucvm_meta_etree_ucvm_pack(ucvm_meta_ucvm_t *meta,
			      char *metastr, int len);

/* Unpack UCVM Etree metadata */
int ucvm_meta_etree_ucvm_unpack(char *metastr, ucvm_meta_ucvm_t *meta);


/* Pack Map Etree metadata */
int ucvm_meta_etree_map_pack(ucvm_meta_map_t *meta,
			     char *metastr, int len);

/* Unpack Map Etree metadata */
int ucvm_meta_etree_map_unpack(char *metastr, ucvm_meta_map_t *meta);

#endif
