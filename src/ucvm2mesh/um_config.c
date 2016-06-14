/**
 * um_config.c -- Read ucvm2mesh configuration
 *
 * Created by Patrick Small <patrices@usc.edu>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef UM_ENABLE_MPI
#include <mpi.h>
#endif
#include "ucvm_config.h"
#include "um_dtypes.h"
#include "um_mesh.h"


/* Read config file*/
int read_config(int myid, int nproc, const char *cfgfile, mesh_config_t *cfg)
{
  ucvm_config_t *chead;
  ucvm_config_t *cptr;

  /* Misc */
  int i;

  /* Initialize variables */
  cfg->meshtype = MESH_FORMAT_UNKNOWN;
  cfg->rank = myid;

  /* Parse config file */
  if (myid == 0) {
    
    printf("[%d] Using config file %s\n", myid, cfgfile);
    chead = ucvm_parse_config(cfgfile);
    if (chead == NULL) {
      fprintf(stderr, "[%d] Failed to parse config file %s\n", 
	      myid, cfgfile);
      return(1);
    }
    
    cptr = ucvm_find_name(chead, "ucvmlist");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find ucvmlist in config\n", myid);
	return(1);
    }
    sprintf(cfg->ucvmstr, "%s", cptr->value);
    
    cptr = ucvm_find_name(chead, "ucvmconf");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find ucvmconf in config\n", myid);
      return(1);
    }
    sprintf(cfg->ucvmconf, "%s", cptr->value);

    cptr = ucvm_find_name(chead, "gridtype");
    if (cptr == NULL) {
	fprintf(stderr, "[%d] Failed to find gridtype in config\n", myid);
	return(1);
    }
    if (strcmp(cptr->value, "CENTER") == 0) {
      cfg->gridtype = UCVM_GRID_CELL_CENTER;
    } else if (strcmp(cptr->value, "VERTEX") == 0) {
      cfg->gridtype = UCVM_GRID_CELL_VERTEX;
    } else {
      fprintf(stderr, "[%d] Failed to find gridtype in config\n", myid);
      return(1);
    }

    cptr = ucvm_find_name(chead, "spacing");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find spacing in config\n", myid);
      return(1);
    }
    if(sscanf(cptr->value, "%lf", &cfg->spacing) != 1) {
      fprintf(stderr, "[%d] Failed to find spacing in config\n", myid);
      return(1);
    }
    
    cptr = ucvm_find_name(chead, "proj");
    if (cptr == NULL) {
	fprintf(stderr, "[%d] Failed to find proj in config\n", myid);
	return(1);
    }
    sprintf(cfg->proj, "%s", cptr->value);

    cptr = ucvm_find_name(chead, "rot");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find rot in config\n", myid);
      return(1);
    }
    if(sscanf(cptr->value, "%lf", &cfg->rot) != 1){
      fprintf(stderr, "[%d] Failed to find rot in config\n", myid);
      return(1);
    }
    
    cptr = ucvm_find_name(chead, "x0");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find x0 in config\n", myid);
      return(1);
    }
    if(sscanf(cptr->value, "%lf", &(cfg->origin.coord[0])) != 1) {
      fprintf(stderr, "[%d] Failed to find lon in config\n", myid);
      return(1);
    }
    
    cptr = ucvm_find_name(chead, "y0");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find y0 in config\n", myid);
      return(1);
    }
    if(sscanf(cptr->value, "%lf", &(cfg->origin.coord[1])) != 1){
      fprintf(stderr, "[%d] Failed to find y0 in config\n", myid);
      return(1);
    }
    
    cptr = ucvm_find_name(chead, "z0");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find z0 in config\n", myid);
      return(1);
    }
    if(sscanf(cptr->value, "%lf", &(cfg->origin.coord[2])) != 1){
      fprintf(stderr, "[%d] Failed to find z0 in config\n", myid);
      return(1);
    }
    
    cptr = ucvm_find_name(chead, "nx");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find nx in config\n", myid);
      return(1);
    }
    if(sscanf(cptr->value, "%d", &(cfg->dims.dim[0])) != 1) {
      fprintf(stderr, "[%d] Failed to find nx in config\n", myid);
      return(1);
    }
    
    cptr = ucvm_find_name(chead, "ny");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find ny in config\n", myid);
      return(1);
    }
    if(sscanf(cptr->value, "%d", &(cfg->dims.dim[1])) != 1){
      fprintf(stderr, "[%d] Failed to find ny in config\n", myid);
      return(1);
    }
    
    cptr = ucvm_find_name(chead, "nz");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find nz in config\n", myid);
      return(1);
    }
    if(sscanf(cptr->value, "%d", &(cfg->dims.dim[2])) != 1){
      fprintf(stderr, "[%d] Failed to find nz in config\n", myid);
      return(1);
    }

    cptr = ucvm_find_name(chead, "px");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find px in config\n", myid);
      return(1);
    }
    if(sscanf(cptr->value, "%d", &(cfg->proc_dims.dim[0])) != 1) {
      fprintf(stderr, "[%d] Failed to find px in config\n", myid);
      return(1);
    }
    
    cptr = ucvm_find_name(chead, "py");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find py in config\n", myid);
      return(1);
    }
    if(sscanf(cptr->value, "%d", &(cfg->proc_dims.dim[1])) != 1){
      fprintf(stderr, "[%d] Failed to find py in config\n", myid);
      return(1);
    }
    
    cptr = ucvm_find_name(chead, "pz");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find pz in config\n", myid);
      return(1);
    }
    if(sscanf(cptr->value, "%d", &(cfg->proc_dims.dim[2])) != 1){
      fprintf(stderr, "[%d] Failed to find pz in config\n", myid);
      return(1);
    }
    
    cptr = ucvm_find_name(chead, "vp_min");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find vp_min in config\n", myid);
      return(1);
    }
    if(sscanf(cptr->value, "%lf", &cfg->vp_min) != 1){
      fprintf(stderr, "[%d] Failed to find vp_min in config\n", myid);
      return(1);
    }
    
    cptr = ucvm_find_name(chead, "vs_min");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find vs_min in config\n", myid);
      return(1);
    }
    if(sscanf(cptr->value, "%lf", &cfg->vs_min) != 1){
      fprintf(stderr, "[%d] Failed to find vs_min in config\n", myid);
      return(1);
    }
    
    cptr = ucvm_find_name(chead, "meshfile");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find meshfile in config\n", myid);
      return(1);
    }
    sprintf(cfg->meshfile, "%s", cptr->value);

    cptr = ucvm_find_name(chead, "gridfile");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find gridfile in config\n", myid);
      return(1);
    }
    sprintf(cfg->gridfile, "%s", cptr->value);
    
    cptr = ucvm_find_name(chead, "meshtype");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find meshtype in config\n", myid);
      return(1);
    }
    for (i = 1; i < MAX_MESH_FORMATS; i++) {
      if (strcmp(cptr->value, MESH_FORMAT_NAMES[i]) == 0) {
	cfg->meshtype = (mesh_format_t)i;
	break;
      }
    }
    if (cfg->meshtype == MESH_FORMAT_UNKNOWN) {
      fprintf(stderr, "[%d] Failed to find meshtype in config\n", myid);
      return(1);
    }
    
    cptr = ucvm_find_name(chead, "scratch");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find scratch in config\n", myid);
      return(1);
    }
    sprintf(cfg->scratch, "%s", cptr->value);

    ucvm_free_config(chead);

    /* Set default interp z-range */
    cfg->ucvm_zrange[0] = 0.0;
    cfg->ucvm_zrange[1] = 350.0;

    /* Check config */
    for (i = 0; i < 3; i++) {
      if (cfg->dims.dim[0] <= 0) {
	fprintf(stderr, "[%d] Mesh dim index %d must be positive\n", myid, i);
	return(1);
      }
      if (cfg->proc_dims.dim[0] <= 0) {
	fprintf(stderr, "[%d] Proc dim index %d must be positive\n", myid, i);
	return(1);
      }
    }

    if (cfg->spacing <= 0.0) {
      fprintf(stderr, "[%d] Spacing must be positive\n", myid);
      return(1);
    }

#ifdef UM_ENABLE_MPI
    /* Check MPI related config items */
    if (nproc > 0) {
      if ((cfg->dims.dim[0] % cfg->proc_dims.dim[0] != 0) || 
	  (cfg->dims.dim[1] % cfg->proc_dims.dim[1] != 0) ||
	  (cfg->dims.dim[2] % cfg->proc_dims.dim[2] != 0)) {
	fprintf(stderr, "[%d] Mesh dims must be divisible by proc dims\n", 
		myid);
	return(1);
      }
      
      if (nproc != cfg->proc_dims.dim[0]*cfg->proc_dims.dim[1]*cfg->proc_dims.dim[2]) {
	fprintf(stderr, "[%d] Proc space does not equal MPI core count\n", 
		myid);
	return(1);
      }
    }
#endif
  }

#ifdef UM_ENABLE_MPI
  if (nproc > 0) {
    int gridtype_i;
    
    /* Broadcast config to all ranks */
    if (MPI_Bcast(&(cfg->ucvmstr[0]), UCVM_MAX_LABEL_LEN, MPI_CHAR, 
		  0, MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast ucvmstr\n", myid);
      return(1);
    }
    
    if (MPI_Bcast(&(cfg->ucvmconf[0]), UCVM_MAX_PATH_LEN, MPI_CHAR, 
		  0, MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast ucvmconf\n", myid);
      return(1);
    }
    
    gridtype_i = (int)(cfg->gridtype);
    if (MPI_Bcast(&gridtype_i, 1, MPI_INT, 0, 
		  MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast gridtype\n", myid);
      return(1);
    }
    
    if (MPI_Bcast(&cfg->spacing, 1, MPI_DOUBLE, 0, 
		  MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast spacing\n", myid);
      return(1);
    }
    
    if (MPI_Bcast(&(cfg->proj[0]), 256, MPI_CHAR, 
		  0, MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast proj\n", myid);
      return(1);
    }
    
    if (MPI_Bcast(&cfg->rot, 1, MPI_DOUBLE, 0, 
		  MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast rot\n", myid);
      return(1);
    }
    
    if (MPI_Bcast(&cfg->origin, 3, MPI_DOUBLE, 0, 
		  MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast origin\n", myid);
      return(1);
    }
    
    if (MPI_Bcast(&cfg->dims, 3, MPI_INT, 0, 
		  MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast dims\n", myid);
      return(1);
    }
    
    if (MPI_Bcast(&cfg->proc_dims, 3, MPI_INT, 0, 
		  MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast proc_dims\n", myid);
      return(1);
    }
    
    if (MPI_Bcast(&cfg->vp_min, 1, MPI_DOUBLE, 0, 
		  MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast vp_min\n", myid);
      return(1);
    }
    
    if (MPI_Bcast(&cfg->vs_min, 1, MPI_DOUBLE, 0, 
		  MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast vs_min\n", myid);
      return(1);
    }
    
    if (MPI_Bcast(&(cfg->meshfile[0]), UCVM_MAX_PATH_LEN, MPI_CHAR, 
		  0, MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast meshfile\n", myid);
      return(1);
    }
    
    if (MPI_Bcast(&(cfg->gridfile[0]), UCVM_MAX_PATH_LEN, MPI_CHAR, 
		  0, MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast gridfile\n", myid);
      return(1);
    }
    
    if (MPI_Bcast(&cfg->meshtype, 1, MPI_INT, 0, 
		  MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast meshtype\n", myid);
      return(1);
    }
    
    if (MPI_Bcast(&(cfg->scratch[0]), UCVM_MAX_PATH_LEN, MPI_CHAR, 
		  0, MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast scratch\n", myid);
      return(1);
    }
    
    if (MPI_Bcast(&(cfg->ucvm_zrange[0]), 2, MPI_DOUBLE, 0, 
		  MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast vs_min\n", myid);
      return(1);
    }
  }
#endif

  return(0);
}


/* Dump config to stdout */
int disp_config(mesh_config_t *cfg) {

  printf("[%d] Configuration:\n", cfg->rank);
  printf("\t[%d] UCVM Model List: %s\n", cfg->rank, cfg->ucvmstr);
  printf("\t[%d] UCVM Conf file: %s\n", cfg->rank, cfg->ucvmconf);
  printf("\t[%d] Gridtype: %d\n", cfg->rank, (int)cfg->gridtype);
  printf("\t[%d] Spacing: %lf\n", cfg->rank, cfg->spacing);
  printf("\t[%d] Projection: %s\n", cfg->rank, cfg->proj);
  printf("\t\t[%d] Rotation Angle: %lf\n", cfg->rank, cfg->rot);
  printf("\t\t[%d] Origin x0,y0,z0: %lf, %lf, %lf\n", 
	 cfg->rank, cfg->origin.coord[0], cfg->origin.coord[1], 
	 cfg->origin.coord[2]);
  printf("\t\t[%d] Dimensions: %d, %d, %d\n", 
	 cfg->rank, cfg->dims.dim[0], cfg->dims.dim[1], cfg->dims.dim[2]);
  printf("\t[%d] Proc Dimensions: %d, %d, %d\n", 
	 cfg->rank, cfg->proc_dims.dim[0], cfg->proc_dims.dim[1], 
	 cfg->proc_dims.dim[2]);
  printf("\t[%d] Vp Min: %lf, Vs Min: %lf\n", 
	 cfg->rank, cfg->vp_min, cfg->vs_min);
  printf("\t[%d] Mesh File: %s\n", cfg->rank, cfg->meshfile);
  printf("\t[%d] Grid File: %s\n", cfg->rank, cfg->gridfile);
  printf("\t[%d] Mesh Type: %d\n", cfg->rank, cfg->meshtype);
  printf("\t[%d] Scratch Dir: %s\n", cfg->rank, cfg->scratch);

  return(0);
}



