#ifndef UCVM_MODEL_JAPAN_H
#define UCVM_MODEL_JAPAN_H

#include "ucvm_dtypes.h"


/* Init Japan */
int ucvm_japan_model_init(int id, ucvm_modelconf_t *conf);


/* Finalize Japan */
int ucvm_japan_model_finalize();


/* Version Japan */
int ucvm_japan_model_version(int id, char *ver, int len);


/* Label Japan */
int ucvm_japan_model_label(int id, char *lab, int len);


/* Setparam Japan */
int ucvm_japan_model_setparam(int id, int param, ...);


/* Query Japan */
int ucvm_japan_model_query(int id, ucvm_ctype_t cmode,
			  int n, ucvm_point_t *pnt,
			  ucvm_data_t *data);


/* Fill model structure with Japan */
int ucvm_japan_get_model(ucvm_model_t *m);

#endif
