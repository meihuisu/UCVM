#ifndef GRD_H
#define GRD_H

#include "grd_dtypes.h"


/* Initializer */
int grd_init(const char *datadir, const char *bkgdir, grd_heur_t heur);


/* Finalizer */
int grd_finalize();


/* Query underlying models */
int grd_query(int n, grd_point_t *pnt, grd_data_t *data);


#endif
