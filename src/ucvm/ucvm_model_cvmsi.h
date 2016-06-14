#ifndef UCVM_MODEL_CVMSI_H
#define UCVM_MODEL_CVMSI_H

#include "ucvm_dtypes.h"


/* Init CVM-SI */
int ucvm_cvmsi_model_init(int id, ucvm_modelconf_t *conf);


/* Finalize CVM-SI */
int ucvm_cvmsi_model_finalize();


/* Version CVM-SI */
int ucvm_cvmsi_model_version(int id, char *ver, int len);


/* Label CVM-SI */
int ucvm_cvmsi_model_label(int id, char *lab, int len);


/* Setparam CVM-SI */
int ucvm_cvmsi_model_setparam(int id, int param, ...);


/* Query CVM-SI */
int ucvm_cvmsi_model_query(int id, ucvm_ctype_t cmode,
			  int n, ucvm_point_t *pnt, 
			  ucvm_data_t *data);


/* Fill model structure with CVM-SI */
int ucvm_cvmsi_get_model(ucvm_model_t *m);

#endif
