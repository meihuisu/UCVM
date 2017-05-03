#ifndef CVMSI_GEO2XY_H
#define CVMSI_GEO2XY_H


/* Convert lon,lat,depth to x,y,z */
void cvmsi_geo2xy_(int *dims, double *box, double *zgrid,
		   double *slat, double *slon, double *sdep, 
		   double *coords, int *izone, int *errcode);


#endif
