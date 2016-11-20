#ifndef CVMS_H
#define CVMS_H


/* Initializer. Modeldir buffer must be 128 bytes in size */
void cvms_init_(char *modeldir, int *errcode);

/* Get version ID. Version string buffer must be 64 bytes in size */
void cvms_version_(char *ver, int *errcode);

/* Query CVM-S */
void cvms_query_(int *nn, 
		 float *rlon, float *rlat,float *rdep,
		 float *alpha, float *beta, float *rho, int *errcode);


#endif
