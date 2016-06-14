#ifndef UCVM_MODEL_CMRG_H
#define UCVM_MODEL_CMRG_H

#include <stdarg.h>
#include "ucvm_dtypes.h"


/* Init Cape Mendocino RG */
int ucvm_cmrg_model_init(int id, ucvm_modelconf_t *conf);


/* Finalize Cape Mendocino RG */
int ucvm_cmrg_model_finalize();


/* Version Cape Mendocino RG */
int ucvm_cmrg_model_version(int id, char *ver, int len);


/* Version Cape Mendocino RG */
int ucvm_cmrg_model_label(int id, char *lab, int len);


/* Setparam Cape Mendocino RG */
int ucvm_cmrg_model_setparam(int id, int param, ...);


/* Query Cape Mendocino RG */
int ucvm_cmrg_model_query(int id, ucvm_ctype_t cmode,
			  int n, ucvm_point_t *pnt, 
			  ucvm_data_t *data);


/* Fill model structure with Cape Mendocino RG */
int ucvm_cmrg_get_model(ucvm_model_t *m);


#endif
