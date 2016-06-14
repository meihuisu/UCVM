#ifndef UCVM_MAP_H
#define UCVM_MAP_H

#include <stdarg.h>
#include "ucvm_dtypes.h"


/* Init Map */
int ucvm_map_init(const char *label, const char *conf);


/* Finalize Map */
int ucvm_map_finalize();


/* Get Version Map */
int ucvm_map_version(char *ver, int len);


/* Get Label Map */
int ucvm_map_label(char *label, int len);


/* Query Map */
int ucvm_map_query(ucvm_ctype_t cmode,
		   int n, ucvm_point_t *pnt, ucvm_data_t *data);


#endif
