#ifndef UCVM_MODEL_CENCAL_H
#define UCVM_MODEL_CENCAL_H

#include <stdarg.h>
#include "ucvm_dtypes.h"


/* Init CenCal */
int ucvm_cencal_model_init(int id, ucvm_modelconf_t *conf);


/* Finalize CenCal */
int ucvm_cencal_model_finalize();


/* Version CenCal */
int ucvm_cencal_model_version(int id, char *ver, int len);


/* Label CenCal */
int ucvm_cencal_model_label(int id, char *lab, int len);


/* Setparam CenCal */
int ucvm_1d_model_setparam(int id, int param, ...);


/* Query CenCal */
int ucvm_cencal_model_query(int id, ucvm_ctype_t cmode,
			    int n, ucvm_point_t *pnt, 
			    ucvm_data_t *data);


/* Fill model structure with CenCal */
int ucvm_cencal_get_model(ucvm_model_t *m);


#endif
