#ifndef UCVM_MODEL_CVMH_H
#define UCVM_MODEL_CVMH_H

#include "ucvm_dtypes.h"


/* Init CVM-H */
int ucvm_cvmh_model_init(int id, ucvm_modelconf_t *conf);


/* Finalize CVM-H */
int ucvm_cvmh_model_finalize();


/* Version CVM-H */
int ucvm_cvmh_model_version(int id, char *ver, int len);


/* Label CVM-H */
int ucvm_cvmh_model_label(int id, char *lab, int len);


/* Setparam CVM-H */
int ucvm_cvmh_model_setparam(int id, int param, ...);


/* Query CVM-H */
int ucvm_cvmh_model_query(int id, ucvm_ctype_t cmode,
			  int n, ucvm_point_t *pnt, 
			  ucvm_data_t *data);


/* Fill model structure with 1D */
int ucvm_cvmh_get_model(ucvm_model_t *m);


#endif
