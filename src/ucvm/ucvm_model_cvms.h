#ifndef UCVM_MODEL_CVMS_H
#define UCVM_MODEL_CVMS_H

#include "ucvm_dtypes.h"


/* Init CVM-S */
int ucvm_cvms_model_init(int id, ucvm_modelconf_t *conf);


/* Finalize CVM-S */
int ucvm_cvms_model_finalize();


/* Version CVM-S */
int ucvm_cvms_model_version(int id, char *ver, int len);


/* Label CVM-S */
int ucvm_cvms_model_label(int id, char *lab, int len);


/* Setparam CVM-S */
int ucvm_cvms_model_setparam(int id, int param, ...);


/* Query CVM-S */
int ucvm_cvms_model_query(int id, ucvm_ctype_t cmode,
			  int n, ucvm_point_t *pnt, 
			  ucvm_data_t *data);


/* Fill model structure with CVM-S */
int ucvm_cvms_get_model(ucvm_model_t *m);

#endif
