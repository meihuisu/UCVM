#ifndef UCVM_MODEL_CMUETREE_H
#define UCVM_MODEL_CMUETREE_H

#include <stdarg.h>
#include "ucvm_dtypes.h"


/* Init CMU Etree */
int ucvm_cmuetree_model_init(int id, ucvm_modelconf_t *conf);


/* Finalize CMU Etree */
int ucvm_cmuetree_model_finalize();


/* Version CMU Etree */
int ucvm_cmuetree_model_version(int id, char *ver, int len);


/* Setparam CMU Etree */
int ucvm_cmuetree_model_setparam(int id, int param, ...);


/* Query CMU Etree */
int ucvm_cmuetree_model_query(int id, ucvm_ctype_t cmode,
			   int n, ucvm_point_t *pnt, ucvm_data_t *data);


/* Fill model structure with CMU Etree */
int ucvm_cmuetree_get_model(ucvm_model_t *m);


#endif
