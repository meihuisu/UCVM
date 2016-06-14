#ifndef UCVM_H
#define UCVM_H

#include <stdarg.h>
#include "ucvm_dtypes.h"


/* Initializer */
int ucvm_init(const char *config);

/* Finalizer */
int ucvm_finalize();

/* Enable specific model(s), by string list, by label, or 
   by ucvm_model_t */
int ucvm_add_model_list(const char *list);
int ucvm_add_model(const char *label);
int ucvm_add_user_model(ucvm_model_t *m, ucvm_modelconf_t *mconf);

/* Associate specific interp func with GTL model, by label 
   or by ucvm_ifunc_t */
int ucvm_assoc_ifunc(const char *mlabel, const char *ilabel);
int ucvm_assoc_user_ifunc(const char *mlabel, ucvm_ifunc_t *ifunc);

/* Use specific map (elev, vs30) by label */
int ucvm_use_map(const char *label);

/* Get label for a model */
int ucvm_model_label(int m, char *label, int len);

/* Get label for an interpolation function */
int ucvm_ifunc_label(int f, char *label, int len);

/* Get version for a model */
int ucvm_model_version(int m, char *ver, int len);

/* Set parameters (see ucvm_dtypes.h for valid param flags) */
int ucvm_setparam(ucvm_param_t param, ...);

/* Query underlying models */
int ucvm_query(int n, ucvm_point_t *pnt, ucvm_data_t *data);

/* Get installed feature information */
int ucvm_get_resources(ucvm_resource_t *res, int *len);

#endif
