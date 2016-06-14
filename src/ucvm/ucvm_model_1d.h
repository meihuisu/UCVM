#ifndef UCVM_MODEL_1D_H
#define UCVM_MODEL_1D_H

#include <stdarg.h>
#include "ucvm_dtypes.h"


/* Init 1D */
int ucvm_1d_model_init(int id, ucvm_modelconf_t *conf);


/* Finalize 1D */
int ucvm_1d_model_finalize();


/* Version 1D */
int ucvm_1d_model_version(int id, char *ver, int len);


/* Label 1D */
int ucvm_1d_model_label(int id, char *lab, int len);


/* Setparam 1D */
int ucvm_1d_model_setparam(int id, int param, ...);


/* Query 1D */
int ucvm_1d_model_query(int id, ucvm_ctype_t cmode,
			int n, ucvm_point_t *pnt, ucvm_data_t *data);


/* Fill model structure with 1D */
int ucvm_1d_get_model(ucvm_model_t *m);


#endif
