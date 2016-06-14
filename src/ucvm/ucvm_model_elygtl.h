#ifndef UCVM_MODEL_ELYGTL_H
#define UCVM_MODEL_ELYGTL_H

#include <stdarg.h>
#include "ucvm_dtypes.h"


/* Init ELY */
int ucvm_elygtl_model_init(int id, ucvm_modelconf_t *conf);


/* Finalize ELY */
int ucvm_elygtl_model_finalize();


/* Version ELY */
int ucvm_elygtl_model_version(int id, char *ver, int len);


/* Label ELY */
int ucvm_elygtl_model_version(int id, char *lab, int len);


/* Setparam ELY */
int ucvm_elygtl_model_setparam(int id, int param, ...);


/* Query ELY */
int ucvm_elygtl_model_query(int id, ucvm_ctype_t cmode,
			 int n, ucvm_point_t *pnt, 
			 ucvm_data_t *data);


/* Fill model structure with ELY */
int ucvm_elygtl_get_model(ucvm_model_t *m);


#endif
