#ifndef UCVM_INTERP_H
#define UCVM_INTERP_H

#include "ucvm_dtypes.h"

/* Ely interpolation method */
int ucvm_interp_ely(double zmin, double zmax, ucvm_ctype_t cmode,
		    ucvm_point_t *pnt, ucvm_data_t *data);


/* Linear interpolation method */
int ucvm_interp_linear(double zmin, double zmax, ucvm_ctype_t cmode,
		       ucvm_point_t *pnt, ucvm_data_t *data);

/* Crustal pass-through method */
int ucvm_interp_crustal(double zmin, double zmax, ucvm_ctype_t cmode,
			ucvm_point_t *pnt, ucvm_data_t *data);



#endif
