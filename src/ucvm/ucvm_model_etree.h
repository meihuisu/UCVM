#ifndef UCVM_MODEL_ETREE_H
#define UCVM_MODEL_ETREE_H

#include <stdarg.h>
#include "ucvm_dtypes.h"


/* Init Etree */
int ucvm_etree_model_init(int id, ucvm_modelconf_t *conf);


/* Finalize Etree */
int ucvm_etree_model_finalize();


/* Version Etree */
int ucvm_etree_model_version(int id, char *ver, int len);


/* Setparam Etree */
int ucvm_etree_model_setparam(int id, int param, ...);


/* Query Etree */
int ucvm_etree_model_query(int id, ucvm_ctype_t cmode,
			   int n, ucvm_point_t *pnt, ucvm_data_t *data);


/* Fill model structure with Etree */
int ucvm_etree_get_model(ucvm_model_t *m);


#endif
