#ifndef UCVM_MODEL_WFCVM_H
#define UCVM_MODEL_WFCVM_H

#include "ucvm_dtypes.h"


/* Init WFCVM */
int ucvm_wfcvm_model_init(int id, ucvm_modelconf_t *conf);


/* Finalize WFCVM */
int ucvm_wfcvm_model_finalize();


/* Version WFCVM */
int ucvm_wfcvm_model_version(int id, char *ver, int len);


/* Label WFCVM */
int ucvm_wfcvm_model_label(int id, char *lab, int len);


/* Setparam WFCVM */
int ucvm_wfcvm_model_setparam(int id, int param, ...);


/* Query WFCVM */
int ucvm_wfcvm_model_query(int id, ucvm_ctype_t cmode,
			   int n, ucvm_point_t *pnt, 
			   ucvm_data_t *data);


/* Fill model structure with WFCVM */
int ucvm_wfcvm_get_model(ucvm_model_t *m);

#endif
