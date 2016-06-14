#ifndef UCVM_MODEL_TAPE_H
#define UCVM_MODEL_TAPE_H

#include <stdarg.h>
#include "ucvm_dtypes.h"


/* Init Cape Mendocino RG */
int ucvm_tape_model_init(int id, ucvm_modelconf_t *conf);


/* Finalize Cape Mendocino RG */
int ucvm_tape_model_finalize();


/* Version Cape Mendocino RG */
int ucvm_tape_model_version(int id, char *ver, int len);


/* Version Cape Mendocino RG */
int ucvm_tape_model_label(int id, char *lab, int len);


/* Setparam Cape Mendocino RG */
int ucvm_tape_model_setparam(int id, int param, ...);


/* Query Cape Mendocino RG */
int ucvm_tape_model_query(int id, ucvm_ctype_t cmode,
			  int n, ucvm_point_t *pnt, 
			  ucvm_data_t *data);


/* Fill model structure with Cape Mendocino RG */
int ucvm_tape_get_model(ucvm_model_t *m);


#endif
