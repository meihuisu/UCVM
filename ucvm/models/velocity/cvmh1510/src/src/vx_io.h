#ifndef VX_IO_H
#define VX_IO_H

#include <stdlib.h>
#include <stdio.h>

/* Initialize voxel prop reader */
int vx_io_init(char *);


/* Finalize voxel prop reader */
int vx_io_finalize();


/* Get vector from voxel property file */
int vx_io_getvec(char *,float *);


/* Get model dimensions from voxel property file */
int vx_io_getdim(char *,int *);


/* Get property name from voxel property file */
int vx_io_getpropname(char *, int, char *);


/* Get property size from voxel property file */
int vx_io_getpropsize(char *, int, int *);


/* Get property value from voxel property file */
int vx_io_getpropval(char *, int, float *);


/* Load voxel volume from disk to memory. Translate 
   endian if necessary */
int vx_io_loadvolume(const char *, const char *, int, int, char *);


#endif
