#ifndef UCVM_MODEL_BBP1D_H
#define UCVM_MODEL_BBP1D_H

#include <stdarg.h>
#include "ucvm_dtypes.h"


/* Init BBP 1D */
int ucvm_bbp1d_model_init(int id, ucvm_modelconf_t *conf);


/* Finalize BBP 1D */
int ucvm_bbp1d_model_finalize();


/* Version BBP 1D */
int ucvm_bbp1d_model_version(int id, char *ver, int len);


/* Label BBP 1D */
int ucvm_bbp1d_model_label(int id, char *lab, int len);


/* Setparam BBP 1D */
int ucvm_bbp1d_model_setparam(int id, int param, ...);


/* Query BBP 1D */
int ucvm_bbp1d_model_query(int id, ucvm_ctype_t cmode,
			int n, ucvm_point_t *pnt, ucvm_data_t *data);


/* Fill model structure with BBP 1D */
int ucvm_bbp1d_get_model(ucvm_model_t *m);

typedef enum { NONE = 0, LINEAR = 1 } ucvm_bbp1d_interpolation_t;

#endif
