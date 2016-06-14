#ifndef UCVM_MODEL_PATCH_H
#define UCVM_MODEL_PATCH_H

#include <stdarg.h>
#include "ucvm_dtypes.h"


/* Init Patch */
int ucvm_patch_model_init(int id, ucvm_modelconf_t *conf);


/* Finalize Patch */
int ucvm_patch_model_finalize();


/* Version Patch */
int ucvm_patch_model_version(int id, char *ver, int len);


/* Setparam Patch */
int ucvm_patch_model_setparam(int id, int param, ...);


/* Query Patch */
int ucvm_patch_model_query(int id, ucvm_ctype_t cmode,
			   int n, ucvm_point_t *pnt, ucvm_data_t *data);


/* Fill model structure with Patch */
int ucvm_patch_get_model(ucvm_model_t *m);


#endif
