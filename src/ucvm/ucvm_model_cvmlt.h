#ifndef UCVM_MODEL_CVMLT_H
#define UCVM_MODEL_CVMLT_H

#include <stdarg.h>
#include "ucvm_dtypes.h"


/* Init CVMLT */
int ucvm_cvmlt_model_init(int id, ucvm_modelconf_t *conf);


/* Finalize CVMLT */
int ucvm_cvmlt_model_finalize();


/* Version CVMLT */
int ucvm_cvmlt_model_version(int id, char *ver, int len);


/* Label CVMLT */
int ucvm_cvmlt_model_label(int id, char *lab, int len);


/* Setparam CVMLT */
int ucvm_cvmlt_model_setparam(int id, int param, ...);


/* Query CVMLT */
int ucvm_cvmlt_model_query(int id, ucvm_ctype_t cmode,
			   int n, ucvm_point_t *pnt, 
			   ucvm_data_t *data);


/* Fill model structure with CVMLT */
int ucvm_cvmlt_get_model(ucvm_model_t *m);


#endif
