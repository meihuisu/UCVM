/** vx_io.c - Voxel IO routines

07/2011: PES: Extracted io into separate module from vx_sub.c
**/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "params.h"
#include "voxet.h"
#include "vx_io.h"
#include "utils.h"

/* Max number of properties */
#define VX_MAX_PROP 512

 /* Property state */
char vx_props[VX_MAX_PROP][CMLEN];
int vx_num_prop = 0;


/* Gets a line without knowing where it writes the info, so
be careful assinging enough space */
int vx_io_getline(FILE *input, char *p, int len) {
  int slen;

  if (fgets(p, len, input) == NULL) {
    *p = '\0';
    return(1);
  }

  /* Strip terminating newline */
  slen = strlen(p);
  if ((slen > 0) && (p[slen-1] == '\n')) {
    p[slen-1] = '\0';
  }
  return(0);
}


/* Initialize voxel prop reader */
int vx_io_init(char *fn)
{
  FILE *ip;
  char buf[CMLEN];

  if (vx_num_prop > 0) {
    return(1);
  }

  ip = fopen(fn, "r");
  if (ip == NULL) {
    return(1);
  }
  
  while (vx_io_getline(ip, &buf[0], CMLEN) == 0) {
    strcpy(vx_props[vx_num_prop++], buf); 
    if (vx_num_prop >= VX_MAX_PROP) {
      fclose(ip);
      return(1);
    }
  }

  fclose(ip);
  return(0);
}


/* Finalize vozel prop reader */
int vx_io_finalize()
{
  vx_num_prop = 0;
  return(0);
}


/* Get vector from voxel property file */
int vx_io_getvec(char *search, float *vec)
{
  int i;

  i = 0;
  while (i < vx_num_prop) {
    if (strstr(vx_props[i], search)) {
      sscanf(&vx_props[i][0]+strlen(search)+1,"%f %f %f",
	     &vec[0], &vec[1], &vec[2]);
      return(0);
    }
    i++;
  }

  return(1);
}



/* Get model dimensions from voxel property file */
int vx_io_getdim(char *search, int *vec)
{
  int i;

  i = 0;
  while (i < vx_num_prop) {
    if (strstr(vx_props[i], search)) {
      sscanf(&vx_props[i][0]+strlen(search),"%d %d %d",
	     &vec[0], &vec[1], &vec[2]);
      return(0);
    }
    i++;
  }

  return(1);
}


/* Get property name from voxel property file */
int vx_io_getpropname(char *search, int PNumber, char *name)
{
  int i, erg;

  i = 0;
  while (i < vx_num_prop) {
    if (strstr(vx_props[i], search)) {
      sscanf(&vx_props[i][0]+strlen(search)+1,"%d" ,&erg);
      if (erg == PNumber) {
	sscanf(&vx_props[i][0]+strlen(search)+1, "%d %s",
	       &erg, name);
	return(0);
      }
    }
    i++;
  }

  return(1);
}


/* Get property size from voxel property file */
int vx_io_getpropsize(char *search, int PNumber, int *size)
{
  int i, erg;

  i = 0;
  while (i < vx_num_prop) {
    if (strstr(vx_props[i], search)) {
      sscanf(&vx_props[i][0]+strlen(search)+1,"%d" ,&erg);
      if (erg == PNumber) {
	sscanf(&vx_props[i][0]+strlen(search)+1,"%d %d",
	       &erg,size);
	return(0);
      }
    }
    i++;
  }

  return(1);
}


/* Get property value from voxel property file */
int vx_io_getpropval(char *search, int PNumber, float *val)
{
  int i, erg;

  i = 0;
  while (i < vx_num_prop) {
    if (strstr(vx_props[i], search)) {
      sscanf(&vx_props[i][0]+strlen(search)+1,"%d" ,&erg);
      if (erg == PNumber) {
	sscanf(&vx_props[i][0]+strlen(search)+1,"%d %f" ,&erg,val);
	return(0);
      }
    }
    i++;
  }

  return(1);
}


/* Load voxel volume from disk to memory. Translate endian if necessary */
int vx_io_loadvolume(const char *data_dir, const char *FN, 
		     int ESIZE, int ncells, char *buffer)
{ 
  FILE *ifi;
  int j, retval;
  union zahl l,*h;
  char file_path[CMLEN];

  /* Read in the file */
  sprintf(file_path, "%s/%s", data_dir, FN);
  ifi = fopen(file_path, "r");
  if (ifi == NULL) {
    return(1);
  }
  retval = fread(buffer, ESIZE, ncells, ifi);
  if (retval != ncells) {
    fprintf(stderr, "Failed to read %d cells of size %d from %s (read %d)\n", 
	    ncells, ESIZE, file_path, retval);
    fclose(ifi);
    return(1);
  }
  fclose(ifi);

  /* Voxet files are big endian */
  if (vx_system_endian() == VX_BYTEORDER_LSB) {
    /* Swap endian */
    for (j = 0; j < ncells; j++) {
      h = (union zahl *)&(buffer[j*ESIZE]);
      l.c[3]=h->c[0];
      l.c[2]=h->c[1];
      l.c[1]=h->c[2];
      l.c[0]=h->c[3];
      memcpy(&(buffer[j*ESIZE]), &l, sizeof(union zahl));
    }
  }


  return 0;
}

