#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "patch_config.h"
#include "ucvm_config.h"
#include "ucvm_utils.h"


/* Read in configuration file and populate config structure */
int read_config(const char *cfgfile, patch_cfg_t *cfg)
{
  int i;

  ucvm_config_t *chead;
  ucvm_config_t *cptr;

  printf("Using config file %s\n", cfgfile);

  chead = ucvm_parse_config(cfgfile);
  if (chead == NULL) {
    fprintf(stderr, "Failed to parse config file %s\n", 
	      cfgfile);
    return(UCVM_CODE_ERROR);
  }
    
  cptr = ucvm_find_name(chead, "version");
  if (cptr == NULL) {
    fprintf(stderr, "Failed to find version in config\n");
    return(UCVM_CODE_ERROR);
  }
  if (strlen(cptr->value) > UCVM_MAX_VERSION_LEN - 1) {
    memset(cfg->version, 0, UCVM_MAX_VERSION_LEN);
    strncpy(cfg->version, cptr->value, UCVM_MAX_VERSION_LEN - 1);
  } else {
    sprintf(cfg->version, "%s", cptr->value);
  }

  cptr = ucvm_find_name(chead, "proj");
  if (cptr == NULL) {
    fprintf(stderr, "Failed to find proj in config\n");
    return(UCVM_CODE_ERROR);
  }
  if (strlen(cptr->value) > UCVM_MAX_PROJ_LEN - 1) {
    memset(cfg->projinfo.projstr, 0, UCVM_MAX_PROJ_LEN);
    strncpy(cfg->projinfo.projstr, cptr->value, UCVM_MAX_PROJ_LEN - 1);
  } else {
    sprintf(cfg->projinfo.projstr, "%s", cptr->value);
  }    

  /* Parse origin longitude */
  cptr = ucvm_find_name(chead, "lon_0");
  if (cptr == NULL) {
    fprintf(stderr, "Failed to find lon_0 in config\n");
    return(UCVM_CODE_ERROR);
  }
  if(sscanf(cptr->value, "%lf", &cfg->projinfo.corner.coord[0]) != 1) {
    fprintf(stderr, "Failed to parse lon_0 in config\n");
    return(UCVM_CODE_ERROR);
  }

  /* Parse origin latitude */
  cptr = ucvm_find_name(chead, "lat_0");
  if (cptr == NULL) {
    fprintf(stderr, "Failed to find lat_0 in config\n");
    return(UCVM_CODE_ERROR);
  }
  if(sscanf(cptr->value, "%lf", &cfg->projinfo.corner.coord[1]) != 1) {
    fprintf(stderr, "Failed to parse lat_0 in config\n");
    return(UCVM_CODE_ERROR);
  }

  /* Parse rotation angle */
  cptr = ucvm_find_name(chead, "rot");
  if (cptr == NULL) {
    fprintf(stderr, "Failed to find rot in config\n");
    return(UCVM_CODE_ERROR);
  }
  if(sscanf(cptr->value, "%lf", &cfg->projinfo.rot) != 1) {
    fprintf(stderr, "Failed to parse rot in config\n");
    return(UCVM_CODE_ERROR);
  }
    
  cptr = ucvm_find_name(chead, "x-size");
  if (cptr == NULL) {
    fprintf(stderr, "Failed to find x-size in config\n");
    return(UCVM_CODE_ERROR);
  }
  if(sscanf(cptr->value, "%lf", &(cfg->projinfo.dims.coord[0])) != 1) {
    fprintf(stderr, "Failed to find nx in config\n");
    return(UCVM_CODE_ERROR);
  }
    
  cptr = ucvm_find_name(chead, "y-size");
  if (cptr == NULL) {
    fprintf(stderr, "Failed to find y-size in config\n");
    return(UCVM_CODE_ERROR);
  }
  if(sscanf(cptr->value, "%lf", &(cfg->projinfo.dims.coord[1])) != 1){
    fprintf(stderr, "Failed to find y-size in config\n");
    return(UCVM_CODE_ERROR);
  }
    
  cptr = ucvm_find_name(chead, "z-size");
  if (cptr == NULL) {
    fprintf(stderr, "Failed to find z-size in config\n");
    return(UCVM_CODE_ERROR);
  }
  if(sscanf(cptr->value, "%lf", &(cfg->projinfo.dims.coord[2])) != 1){
    fprintf(stderr, "Failed to find z-size in config\n");
    return(UCVM_CODE_ERROR);
  }
    
  cptr = ucvm_find_name(chead, "spacing");
  if (cptr == NULL) {
    fprintf(stderr, "Failed to find spacing in config\n");
    return(UCVM_CODE_ERROR);
  }
  if(sscanf(cptr->value, "%lf", &(cfg->spacing)) != 1) {
    fprintf(stderr, "Failed to find spacing in config\n");
    return(UCVM_CODE_ERROR);
  }
    
  cptr = ucvm_find_name(chead, "modelname");
  if (cptr == NULL) {
    fprintf(stderr, "Failed to find modelname in config\n");
    return(UCVM_CODE_ERROR);
  }
  sprintf(cfg->modelname, "%s", cptr->value);

  cptr = ucvm_find_name(chead, "modelpath");
  if (cptr == NULL) {
    fprintf(stderr, "Failed to find modelpath in config\n");
    return(UCVM_CODE_ERROR);
  }
  sprintf(cfg->modelpath, "%s", cptr->value);

  cptr = ucvm_find_name(chead, "ucvmstr");
  if (cptr == NULL) {
    fprintf(stderr, "Failed to find ucvmstr in config\n");
    return(UCVM_CODE_ERROR);
  }
  sprintf(cfg->ucvmstr, "%s", cptr->value);

  cptr = ucvm_find_name(chead, "ucvm_interp_zrange");
  if (cptr == NULL) {
    fprintf(stderr, "Failed to find ucvm_interp_zrange in config\n");
    return(UCVM_CODE_ERROR);
  }
  if (list_parse(cptr->value, UCVM_CONFIG_MAX_STR, 
		 &(cfg->ucvm_zrange[0]), 2) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to parse ucvm_interp_zrange in config\n");
    return(UCVM_CODE_ERROR);
  }
    
  cptr = ucvm_find_name(chead, "ucvmconf");
  if (cptr == NULL) {
    fprintf(stderr, "Failed to find ucvmconf in config\n");
    return(UCVM_CODE_ERROR);
  }
  sprintf(cfg->ucvmconf, "%s", cptr->value);
    
  ucvm_free_config(chead); 


  /* Setup projection */
  if (ucvm_proj_ucvm_init(cfg->projinfo.projstr, 
			  &(cfg->projinfo.corner), 
			  cfg->projinfo.rot,
			  &(cfg->projinfo.dims),
			  &(cfg->projinfo.proj)) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to setup proj %s.\n", 
	    cfg->projinfo.projstr);
    return(UCVM_CODE_ERROR);
  }

  /* Check spacing */
  if (cfg->spacing <= 0.0) {
      fprintf(stderr, "Spacing must be greater than zero\n");
      return(UCVM_CODE_ERROR);
  }

  /* Check dimensions */
  for (i = 0; i < 3; i++) {
    if (cfg->projinfo.dims.coord[i] < cfg->spacing) {
      fprintf(stderr, "Dim %d must be greater than or equal to spacing\n", i);
      return(UCVM_CODE_ERROR);
    }

    if ((int)cfg->projinfo.dims.coord[i] % (int)cfg->spacing) {
      fprintf(stderr, "Dim %d must be divisible by spacing\n", i);
      return(UCVM_CODE_ERROR);
    }
  }
  
  return(UCVM_CODE_SUCCESS);
}


/* Dump config to stdout */
int disp_config(patch_cfg_t *cfg) 
{
  printf("Configuration:\n");
  printf("\tProjection: %s\n", cfg->projinfo.projstr);
  printf("\t\tOrigin: %lf deg, %lf deg\n", 
	 cfg->projinfo.corner.coord[0],
	 cfg->projinfo.corner.coord[1]);
  printf("\t\tRot Angle: %lf deg\n", cfg->projinfo.rot);

  printf("\tRegion Dims: %lf m, %lf m, %lf m\n", 
	 cfg->projinfo.dims.coord[0], 
	 cfg->projinfo.dims.coord[1],
	 cfg->projinfo.dims.coord[2]); 

  printf("\tSpacing: %lf m\n", cfg->spacing);
  printf("\tModel Name: %s\n", cfg->modelname);
  printf("\tModelpath: %s\n", cfg->modelpath);
  printf("\tUCVM String: %s\n", cfg->ucvmstr);
  printf("\tUCVM Interp: %lf m - %lf m\n", cfg->ucvm_zrange[0],
	 cfg->ucvm_zrange[1]);
  printf("\tUCVM Conf: %s\n\n", cfg->ucvmconf);
  
  return(UCVM_CODE_SUCCESS);
}
