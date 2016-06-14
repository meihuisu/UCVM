#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef UE_ENABLE_MPI
#include "mpi.h"
#endif
#include "ue_config.h"
#include "ucvm_config.h"
#include "ucvm_utils.h"


/* Read in configuration file and populate config structure */
int read_config(int myid, int nproc, const char *cfgfile, ue_cfg_t *cfg)
{
  int i;
  ucvm_config_t *chead;
  ucvm_config_t *cptr;
  char tmpstr[UCVM_CONFIG_MAX_STR];

  /* Set rank,nproc to -1 to disable MPI */
  cfg->rank = myid;
  cfg->nproc = nproc;

  if (myid == 0) {
    printf("[%d] Using config file %s\n", myid, cfgfile);


    chead = ucvm_parse_config(cfgfile);
    if (chead == NULL) {
      fprintf(stderr, "[%d] Failed to parse config file %s\n", 
	      myid, cfgfile);
      return(UCVM_CODE_ERROR);
    }
    
    cptr = ucvm_find_name(chead, "proj");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find proj in config\n", myid);
      return(UCVM_CODE_ERROR);
    }
    if (strlen(cptr->value) > UCVM_MAX_PROJ_LEN - 1) {
      memset(cfg->projinfo.projstr, 0, UCVM_MAX_PROJ_LEN);
      strncpy(cfg->projinfo.projstr, cptr->value, UCVM_MAX_PROJ_LEN - 1);
    } else {
      sprintf(cfg->projinfo.projstr, "%s", cptr->value);
    }
    
    /* Geo-bilinear projection is special case */
    if (strcmp(cfg->projinfo.projstr, PROJ_GEO_BILINEAR) == 0) {
      for (i = 0; i < 4; i++) {
	/* Parse longitude */
	sprintf(tmpstr, "lon_%d", i);
	cptr = ucvm_find_name(chead, tmpstr);
	if (cptr == NULL) {
	  fprintf(stderr, "[%d] Failed to find %s in config\n", 
		  myid, tmpstr);
	  return(UCVM_CODE_ERROR);
	}
	if(sscanf(cptr->value, "%lf", &cfg->projinfo.corner[i].coord[0]) != 1) {
	  fprintf(stderr, "[%d] Failed to parse %s in config\n", 
		  myid, tmpstr);
	  return(UCVM_CODE_ERROR);
	}
	
	/* Parse latitude */
	sprintf(tmpstr, "lat_%d", i);
	cptr = ucvm_find_name(chead, tmpstr);
	if (cptr == NULL) {
	  fprintf(stderr, "[%d] Failed to find %s in config\n", myid, tmpstr);
	  return(UCVM_CODE_ERROR);
	}
	if(sscanf(cptr->value, "%lf", &cfg->projinfo.corner[i].coord[1]) != 1) {
	  fprintf(stderr, "[%d] Failed to parse %s in config\n", 
		  myid, tmpstr);
	  return(UCVM_CODE_ERROR);
	}
      }
    } else {
      /* Parse origin longitude */
      cptr = ucvm_find_name(chead, "lon_0");
      if (cptr == NULL) {
	fprintf(stderr, "[%d] Failed to find lon_0 in config\n", myid);
	return(UCVM_CODE_ERROR);
      }
      if(sscanf(cptr->value, "%lf", &cfg->projinfo.corner[0].coord[0]) != 1) {
	fprintf(stderr, "[%d] Failed to parse lon_0 in config\n", myid);
	return(UCVM_CODE_ERROR);
      }

      /* Parse origin latitude */
      cptr = ucvm_find_name(chead, "lat_0");
      if (cptr == NULL) {
	fprintf(stderr, "[%d] Failed to find lat_0 in config\n", myid);
	return(UCVM_CODE_ERROR);
      }
      if(sscanf(cptr->value, "%lf", &cfg->projinfo.corner[0].coord[1]) != 1) {
	fprintf(stderr, "[%d] Failed to parse lat_0 in config\n", myid);
	return(UCVM_CODE_ERROR);
      }

      /* Parse rotation angle */
      cptr = ucvm_find_name(chead, "rot");
      if (cptr == NULL) {
	fprintf(stderr, "[%d] Failed to find rot in config\n", myid);
	return(UCVM_CODE_ERROR);
      }
      if(sscanf(cptr->value, "%lf", &cfg->projinfo.rot) != 1) {
	fprintf(stderr, "[%d] Failed to parse rot in config\n", myid);
	return(UCVM_CODE_ERROR);
      }
    }
    
    cptr = ucvm_find_name(chead, "x-size");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find x-size in config\n", myid);
      return(UCVM_CODE_ERROR);
    }
    if(sscanf(cptr->value, "%lf", &(cfg->projinfo.dims.coord[0])) != 1) {
      fprintf(stderr, "[%d] Failed to find nx in config\n", myid);
      return(UCVM_CODE_ERROR);
    }
    
    cptr = ucvm_find_name(chead, "y-size");
    if (cptr == NULL) {
    fprintf(stderr, "[%d] Failed to find y-size in config\n", myid);
    return(UCVM_CODE_ERROR);
    }
    if(sscanf(cptr->value, "%lf", &(cfg->projinfo.dims.coord[1])) != 1){
    fprintf(stderr, "[%d] Failed to find y-size in config\n", myid);
    return(UCVM_CODE_ERROR);
    }
    
    cptr = ucvm_find_name(chead, "z-size");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find z-size in config\n", myid);
      return(UCVM_CODE_ERROR);
    }
    if(sscanf(cptr->value, "%lf", &(cfg->projinfo.dims.coord[2])) != 1){
      fprintf(stderr, "[%d] Failed to find z-size in config\n", myid);
      return(UCVM_CODE_ERROR);
    }
    
    cptr = ucvm_find_name(chead, "nx");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find nx in config\n", myid);
      return(UCVM_CODE_ERROR);
    }
    if(sscanf(cptr->value, "%d", &(cfg->col_dims.dim[0])) != 1) {
      fprintf(stderr, "[%d] Failed to find nx in config\n", myid);
      return(UCVM_CODE_ERROR);
    }
    
    cptr = ucvm_find_name(chead, "ny");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find ny in config\n", myid);
      return(UCVM_CODE_ERROR);
    }
    if(sscanf(cptr->value, "%d", &(cfg->col_dims.dim[1])) != 1) {
      fprintf(stderr, "[%d] Failed to find ny in config\n", myid);
      return(UCVM_CODE_ERROR);
    }
    
    cptr = ucvm_find_name(chead, "max_freq");
    if (cptr == NULL) {
    fprintf(stderr, "[%d] Failed to find max_freq in config\n", myid);
    return(UCVM_CODE_ERROR);
    }
    if(sscanf(cptr->value, "%lf", &(cfg->max_freq)) != 1){
      fprintf(stderr, "[%d] Failed to find max_freq in config\n", myid);
      return(UCVM_CODE_ERROR);
    }
    
    cptr = ucvm_find_name(chead, "ppwl");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find ppwl in config\n", myid);
      return(UCVM_CODE_ERROR);
    }
    if(sscanf(cptr->value, "%lf", &(cfg->ppwl)) != 1){
      fprintf(stderr, "[%d] Failed to find ppwl in config\n", myid);
      return(UCVM_CODE_ERROR);
    }
    
    cptr = ucvm_find_name(chead, "max_octsize");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find max_octsize in config\n", myid);
      return(UCVM_CODE_ERROR);
    }
    if(sscanf(cptr->value, "%lf", &cfg->max_octsize) != 1){
      fprintf(stderr, "[%d] Failed to find max_octsize in config\n", myid);
      return(UCVM_CODE_ERROR);
    }
    
    cptr = ucvm_find_name(chead, "title");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find title in config\n", myid);
      return(UCVM_CODE_ERROR);
    }
    if (strlen(cptr->value) > UCVM_META_MAX_STRING_LEN - 1) {
      memset(cfg->ecfg.title, 0, UCVM_META_MAX_STRING_LEN);
      strncpy(cfg->ecfg.title, cptr->value, UCVM_META_MAX_STRING_LEN - 1);
    } else {
      sprintf(cfg->ecfg.title, "%s", cptr->value);
    }

    cptr = ucvm_find_name(chead, "author");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find author in config\n", myid);
      return(UCVM_CODE_ERROR);
    }
    if (strlen(cptr->value) > UCVM_META_MAX_STRING_LEN - 1) {
      memset(cfg->ecfg.author, 0, UCVM_META_MAX_STRING_LEN);
      strncpy(cfg->ecfg.author, cptr->value, UCVM_META_MAX_STRING_LEN - 1);
    } else {
      sprintf(cfg->ecfg.author, "%s", cptr->value);
    }
    
    cptr = ucvm_find_name(chead, "date");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find date in config\n", myid);
      return(UCVM_CODE_ERROR);
    }
    if (strlen(cptr->value) > UCVM_META_MAX_STRING_LEN - 1) {
      memset(cfg->ecfg.date, 0, UCVM_META_MAX_STRING_LEN);
      strncpy(cfg->ecfg.date, cptr->value, UCVM_META_MAX_STRING_LEN - 1);
    } else {
      sprintf(cfg->ecfg.date, "%s", cptr->value);
    }
    
    cptr = ucvm_find_name(chead, "outputfile");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find outputfile in config\n", myid);
    return(UCVM_CODE_ERROR);
    }
    sprintf(cfg->ecfg.outputfile, "%s", cptr->value);
    
    cptr = ucvm_find_name(chead, "format");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find format in config\n", myid);
    return(UCVM_CODE_ERROR);
    }
    sprintf(cfg->ecfg.format, "%s", cptr->value);
    
    if ((strcmp(cfg->ecfg.format, "flatfile") != 0) && 
      (strcmp(cfg->ecfg.format, "etree") != 0)) {
      fprintf(stderr, "[%d] Unsupported format type %s.\n", myid, 
	      cfg->ecfg.format);
      return(UCVM_CODE_ERROR);
    }
    
    cptr = ucvm_find_name(chead, "ucvmstr");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find ucvmstr in config\n", myid);
      return(UCVM_CODE_ERROR);
    }
    sprintf(cfg->ucvmstr, "%s", cptr->value);

    cptr = ucvm_find_name(chead, "ucvm_interp_zrange");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find ucvm_interp_zrange in config\n", 
	      myid);
      return(UCVM_CODE_ERROR);
    }
    if (list_parse(cptr->value, UCVM_CONFIG_MAX_STR, 
		   &(cfg->ucvm_zrange[0]), 2) != UCVM_CODE_SUCCESS) {
      fprintf(stderr, "[%d] Failed to parse ucvm_interp_zrange in config\n", 
	      myid);
      return(UCVM_CODE_ERROR);
    }
    
    cptr = ucvm_find_name(chead, "ucvmconf");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find ucvmconf in config\n", myid);
      return(UCVM_CODE_ERROR);
    }
    sprintf(cfg->ucvmconf, "%s", cptr->value);
    
    cptr = ucvm_find_name(chead, "vs_min");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find vs_min in config\n", myid);
    return(UCVM_CODE_ERROR);
    }
    if(sscanf(cptr->value, "%lf", &cfg->vs_min) != 1){
      fprintf(stderr, "[%d] Failed to find vs_min in config\n", myid);
      return(UCVM_CODE_ERROR);
    }
    
    cptr = ucvm_find_name(chead, "scratch");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find scratch in config\n", myid);
      return(UCVM_CODE_ERROR);
    }
    sprintf(cfg->scratch, "%s", cptr->value);
    
    cptr = ucvm_find_name(chead, "buf_etree_cache");
    if (cptr == NULL) {
      fprintf(stderr, "[%d] Failed to find buf_etree_cache in config\n", myid);
    return(UCVM_CODE_ERROR);
    }
    if(sscanf(cptr->value, "%d", &cfg->buf_etree_cache) != 1){
      fprintf(stderr, "[%d] Failed to find buf_etree_cache in config\n", myid);
      return(UCVM_CODE_ERROR);
    }

    cptr = ucvm_find_name(chead, "buf_extract_mem_max_oct");
    if (cptr == NULL) {
      fprintf(stderr, 
	      "[%d] Failed to find buf_extract_mem_max_oct in config\n", myid);
    return(UCVM_CODE_ERROR);
    }
    if(sscanf(cptr->value, "%d", &cfg->buf_extract_mem_max_oct) != 1){
      fprintf(stderr, 
	      "[%d] Failed to find buf_extract_mem_max_oct in config\n", myid);
      return(UCVM_CODE_ERROR);
    }

    cptr = ucvm_find_name(chead, "buf_extract_ffile_max_oct");
    if (cptr == NULL) {
      fprintf(stderr, 
	      "[%d] Failed to find buf_extract_ffile_max_oct in config\n", 
	      myid);
    return(UCVM_CODE_ERROR);
    }
    if(sscanf(cptr->value, "%d", &cfg->buf_extract_ffile_max_oct) != 1){
      fprintf(stderr, 
	      "[%d] Failed to find buf_extract_ffile_max_oct in config\n", myid);
      return(UCVM_CODE_ERROR);
    }

    cptr = ucvm_find_name(chead, "buf_sort_ffile_max_oct");
    if (cptr == NULL) {
      fprintf(stderr, 
	      "[%d] Failed to find buf_sort_ffile_max_oct in config\n", 
	      myid);
    return(UCVM_CODE_ERROR);
    }
    if(sscanf(cptr->value, "%d", &cfg->buf_sort_ffile_max_oct) != 1){
      fprintf(stderr, 
	      "[%d] Failed to find buf_sort_ffile_max_oct in config\n", myid);
      return(UCVM_CODE_ERROR);
    }

    cptr = ucvm_find_name(chead, "buf_merge_report_min_oct");
    if (cptr == NULL) {
      fprintf(stderr, 
	      "[%d] Failed to find buf_merge_report_min_oct in config\n", 
	      myid);
    return(UCVM_CODE_ERROR);
    }
    if(sscanf(cptr->value, "%d", &cfg->buf_merge_report_min_oct) != 1){
      fprintf(stderr, 
	      "[%d] Failed to find buf_merge_report_min_oct in config\n", 
	      myid);
      return(UCVM_CODE_ERROR);
    }

    cptr = ucvm_find_name(chead, "buf_merge_sendrecv_buf_oct");
    if (cptr == NULL) {
      fprintf(stderr, 
	      "[%d] Failed to find buf_merge_sendrecv_buf_oct in config\n", 
	      myid);
    return(UCVM_CODE_ERROR);
    }
    if(sscanf(cptr->value, "%d", &cfg->buf_merge_sendrecv_buf_oct) != 1){
      fprintf(stderr, 
	      "[%d] Failed to find buf_merge_sendrecv_buf_oct in config\n", 
	      myid);
      return(UCVM_CODE_ERROR);
    }

    cptr = ucvm_find_name(chead, "buf_merge_io_buf_oct");
    if (cptr == NULL) {
      fprintf(stderr, 
	      "[%d] Failed to find buf_merge_io_buf_oct in config\n", 
	      myid);
    return(UCVM_CODE_ERROR);
    }
    if(sscanf(cptr->value, "%d", &cfg->buf_merge_io_buf_oct) != 1){
      fprintf(stderr, 
	      "[%d] Failed to find buf_merge_io_buf_oct in config\n", 
	      myid);
      return(UCVM_CODE_ERROR);
    }

    ucvm_free_config(chead); 
  }

#ifdef UE_ENABLE_MPI
  if (nproc > 0) {
    /* Broadcast config to all ranks */
    if (MPI_Bcast(cfg->projinfo.projstr, UCVM_MAX_PROJ_LEN, MPI_CHAR, 
		  0, MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast proj\n", myid);
      return(UCVM_CODE_ERROR);
    }
  
    for(i = 0; i < 4; i++) {
      if (MPI_Bcast(&(cfg->projinfo.corner[i].coord[0]), 1, MPI_DOUBLE, 0, 
		    MPI_COMM_WORLD) != MPI_SUCCESS) {
	fprintf(stderr, "[%d] Failed to broadcast lon_%d\n", myid, i);
	return(UCVM_CODE_ERROR);
      }
      if (MPI_Bcast(&(cfg->projinfo.corner[i].coord[1]), 1, MPI_DOUBLE, 0, 
		    MPI_COMM_WORLD) != MPI_SUCCESS) {
	fprintf(stderr, "[%d] Failed to broadcast lat_%d\n", myid, i);
	return(UCVM_CODE_ERROR);
      }
    }

    for(i = 0; i < 3; i++) {
      if (MPI_Bcast(&(cfg->projinfo.dims.coord[i]), 1, MPI_DOUBLE, 0, 
		    MPI_COMM_WORLD) != MPI_SUCCESS) {
	fprintf(stderr, "[%d] Failed to broadcast size dim %d\n", myid, i);
	return(UCVM_CODE_ERROR);
      }
    }

    if (MPI_Bcast(&(cfg->projinfo.rot), 1, MPI_DOUBLE, 0, 
		  MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast rot\n", myid);
      return(UCVM_CODE_ERROR);
    }
    
    for(i = 0; i < 2; i++) {
      if (MPI_Bcast(&(cfg->col_dims.dim[i]), 1, MPI_INT, 0, 
		    MPI_COMM_WORLD) != MPI_SUCCESS) {
	fprintf(stderr, "[%d] Failed to broadcast col dims %d\n", myid, i);
	return(UCVM_CODE_ERROR);
      }
    }
    
    if (MPI_Bcast(&(cfg->max_freq), 1, MPI_DOUBLE, 0, 
		  MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast max_freq\n", myid);
      return(UCVM_CODE_ERROR);
    }
    
    if (MPI_Bcast(&(cfg->ppwl), 1, MPI_DOUBLE, 0, 
		  MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast ppwl\n", myid);
      return(UCVM_CODE_ERROR);
    }
    
    if (MPI_Bcast(&(cfg->max_octsize), 1, MPI_DOUBLE, 0, 
		  MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast max_octsize\n", myid);
      return(UCVM_CODE_ERROR);
    }
    
    if (MPI_Bcast(&(cfg->vs_min), 1, MPI_DOUBLE, 0, 
		  MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast vs_min\n", myid);
      return(UCVM_CODE_ERROR);
    }
    
    if (MPI_Bcast(cfg->ucvmstr, UCVM_CONFIG_MAX_STR, MPI_CHAR, 
		  0, MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast ucvmstr\n", myid);
      return(UCVM_CODE_ERROR);
    }
    
    for(i = 0; i < 2; i++) {
      if (MPI_Bcast(&(cfg->ucvm_zrange[i]), 1, MPI_DOUBLE, 0, 
		    MPI_COMM_WORLD) != MPI_SUCCESS) {
	fprintf(stderr, "[%d] Failed to broadcast size ucvm zrange %d\n", 
		myid, i);
	return(UCVM_CODE_ERROR);
      }
    }
    
    if (MPI_Bcast(cfg->ucvmconf, UCVM_CONFIG_MAX_STR, MPI_CHAR, 
		  0, MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast ucvmconf\n", myid);
      return(UCVM_CODE_ERROR);
    }
    
    if (MPI_Bcast(cfg->scratch, UCVM_CONFIG_MAX_STR, MPI_CHAR, 
		  0, MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast scratch\n", myid);
      return(UCVM_CODE_ERROR);
    }
    
    if (MPI_Bcast(&cfg->buf_etree_cache, 1, MPI_INT, 
		  0, MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast buf_etree_cache\n", myid);
      return(UCVM_CODE_ERROR);
    }
    
    if (MPI_Bcast(&cfg->buf_extract_mem_max_oct, 1, MPI_INT, 
		  0, MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast buf_extract_mem_max_oct\n", 
	      myid);
      return(UCVM_CODE_ERROR);
    }
    
    if (MPI_Bcast(&cfg->buf_extract_ffile_max_oct, 1, MPI_INT, 
		  0, MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast buf_extract_ffile_max_oct\n", 
	      myid);
      return(UCVM_CODE_ERROR);
    }
    
    if (MPI_Bcast(&cfg->buf_sort_ffile_max_oct, 1, MPI_INT, 
		  0, MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast buf_sort_ffile_max_oct\n", 
	      myid);
      return(UCVM_CODE_ERROR);
    }
    
    if (MPI_Bcast(&cfg->buf_merge_report_min_oct, 1, MPI_INT, 
		  0, MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast buf_merge_report_min_oct\n", 
	      myid);
      return(UCVM_CODE_ERROR);
    }
    
    if (MPI_Bcast(&cfg->buf_merge_sendrecv_buf_oct, 1, MPI_INT, 
		  0, MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast buf_merge_sendrecv_buf_oct\n", 
	      myid);
      return(UCVM_CODE_ERROR);
    }
    
    if (MPI_Bcast(&cfg->buf_merge_io_buf_oct, 1, MPI_INT, 
		  0, MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(stderr, "[%d] Failed to broadcast buf_merge_io_buf_oct\n", 
	      myid);
      return(UCVM_CODE_ERROR);
    }
  }
#endif

  /* Setup projection */
  if (strcmp(cfg->projinfo.projstr, PROJ_GEO_BILINEAR) == 0) {
    for (i = 0; i < 4; i++) {
      cfg->projinfo.projb.xi[i] = cfg->projinfo.corner[i].coord[0];      
      cfg->projinfo.projb.yi[i] = cfg->projinfo.corner[i].coord[1];
    }
    for (i = 0; i < 2; i++) {
      cfg->projinfo.projb.dims[i] = cfg->projinfo.dims.coord[i];
    }
  } else {
    if (ucvm_proj_ucvm_init(cfg->projinfo.projstr, 
			    &(cfg->projinfo.corner[0]), 
			    cfg->projinfo.rot,
			    &(cfg->projinfo.dims),
			    &(cfg->projinfo.proj)) != UCVM_CODE_SUCCESS) {
      fprintf(stderr, "[%d] Failed to setup proj %s.\n", myid, 
	      cfg->projinfo.projstr);
      return(UCVM_CODE_ERROR);
    }
  }

  /* Setup etree parameters */

  /* Determine length of longest side */
  if (cfg->projinfo.dims.coord[0] > cfg->projinfo.dims.coord[1]) {
    cfg->ecfg.max_length = cfg->projinfo.dims.coord[0];
  } else {
    cfg->ecfg.max_length = cfg->projinfo.dims.coord[1];
  }
  /* Setup tick size */
  cfg->ecfg.ticksize = cfg->ecfg.max_length / 
    (double)((etree_tick_t)1 << (ETREE_MAXLEVEL));
  for (i = 0; i < 2; i++) {
    /* Setup column/row sizes in tics */
    cfg->ecfg.col_ticks[i] = (cfg->projinfo.dims.coord[i]/
			      cfg->col_dims.dim[i]) / cfg->ecfg.ticksize;
  }
  /* Setup min and max edgesize/level for this etree */
  cfg->ecfg.min_edgesize = cfg->vs_min / (cfg->ppwl * cfg->max_freq);
  if (cfg->max_octsize < (cfg->ecfg.col_ticks[0]*cfg->ecfg.ticksize)) {
    cfg->ecfg.max_edgesize = cfg->max_octsize;
  } else {
    /* Maximum edge size cannot be larger than a column */
    cfg->ecfg.max_edgesize = cfg->ecfg.col_ticks[0]*cfg->ecfg.ticksize;
  }
  cfg->ecfg.max_level = ceil(log(cfg->ecfg.max_length/
				 cfg->ecfg.min_edgesize) /log(2.0));
  cfg->ecfg.min_level = ceil(log(cfg->ecfg.max_length/
				 cfg->ecfg.max_edgesize) /log(2.0));
  for (i = 0; i < 3; i++) {
    /* Determine max ticks in each dimension */
    cfg->ecfg.max_ticks[i] = (cfg->projinfo.dims.coord[i] / 
			      cfg->ecfg.max_length) * 
      (double)((etree_tick_t)1 << (ETREE_MAXLEVEL));
  }
  
  return(UCVM_CODE_SUCCESS);
}


/* Dump config to stdout */
int disp_config(ue_cfg_t *cfg) 
{
  int i;

  printf("[%d] Configuration:\n", cfg->rank);
  printf("\t[%d] Column Dims: %d, %d\n", cfg->rank, 
	 cfg->col_dims.dim[0], cfg->col_dims.dim[1]);
  printf("\t[%d] Projection: %s\n", cfg->rank, cfg->projinfo.projstr);
  if (strcmp(cfg->projinfo.projstr, PROJ_GEO_BILINEAR) == 0) {
    for (i = 0; i < 4; i++) {
      printf("\t\t[%d] Bilinear xi/yi %d: %lf deg, %lf deg\n", cfg->rank, i, 
	     cfg->projinfo.projb.xi[i],
	     cfg->projinfo.projb.yi[i]);
    }
    printf("\t\t[%d] Bilinear Dims: %lf m, %lf m\n", cfg->rank, 
	   cfg->projinfo.projb.dims[0],
	   cfg->projinfo.projb.dims[1]);
  } else {
    printf("\t\t[%d] Origin: %lf deg, %lf deg\n", cfg->rank, 
	   cfg->projinfo.corner[0].coord[0],
	   cfg->projinfo.corner[0].coord[1]);
    printf("\t\t[%d] Rot Angle: %lf deg\n", cfg->rank, cfg->projinfo.rot);
  }

  printf("\t[%d] Region Dims: %lf m, %lf m, %lf m\n", cfg->rank, 
	 cfg->projinfo.dims.coord[0], 
	 cfg->projinfo.dims.coord[1],
	 cfg->projinfo.dims.coord[2]); 

  printf("\t[%d] Max Freq: %lf Hz\n", cfg->rank, cfg->max_freq);
  printf("\t[%d] Points/Wavelength: %lf\n", cfg->rank, cfg->ppwl);
  printf("\t[%d] Max Cell Size: %lf m\n", cfg->rank, cfg->max_octsize);
  
  printf("\t[%d] Author: %s\n", cfg->rank, cfg->ecfg.author);
  printf("\t[%d] Title: %s\n", cfg->rank, cfg->ecfg.title);
  printf("\t[%d] Date: %s\n", cfg->rank, cfg->ecfg.date);
  printf("\t[%d] Output File: %s\n", cfg->rank, cfg->ecfg.outputfile);
  
  printf("\t[%d] UCVM String: %s\n", cfg->rank, cfg->ucvmstr);
  printf("\t[%d] UCVM Interp: %lf m - %lf m\n", cfg->rank, cfg->ucvm_zrange[0],
	 cfg->ucvm_zrange[1]);
  printf("\t[%d] UCVM Conf: %s\n", cfg->rank, cfg->ucvmconf);
  printf("\t[%d] Vs Min: %lf m/s\n", cfg->rank, cfg->vs_min);
  printf("\t[%d] Scratch Dir: %s\n\n", cfg->rank, cfg->scratch);

  printf("[%d] Buffers:\n", cfg->rank);
  printf("\t[%d] Etree Cache: %d MB\n", cfg->rank, cfg->buf_etree_cache);
  printf("\t[%d] Extract Mem Max Oct: %d\n", cfg->rank, 
	 cfg->buf_extract_mem_max_oct);
  printf("\t[%d] Extract FF Max Oct: %d\n", cfg->rank, 
	 cfg->buf_extract_ffile_max_oct);
  printf("\t[%d] Sort FF Max Oct: %d\n", cfg->rank, 
	 cfg->buf_sort_ffile_max_oct);
  printf("\t[%d] Merge Report Min Oct: %d\n", cfg->rank, 
	 cfg->buf_merge_report_min_oct);
  printf("\t[%d] Merge SendRecv Buf Oct: %d\n", cfg->rank, 
	 cfg->buf_merge_sendrecv_buf_oct);
  printf("\t[%d] Merge IO Buf Oct: %d\n\n", cfg->rank, 
	 cfg->buf_merge_io_buf_oct);

  printf("[%d] Calculated for %lf Hz, %lf m/s, %lf ppwl:\n",
	   cfg->rank, cfg->max_freq, cfg->vs_min, cfg->ppwl);
  printf("\t[%d] Min Edge Size To Support Vs Min: %lf m\n", 
	 cfg->rank, cfg->ecfg.min_edgesize);
  printf("\t[%d] Max Edge Size To Allow: %lf m\n", 
	 cfg->rank, cfg->ecfg.max_edgesize);
  printf("\t[%d] Max Level: %d (%lf m)\n", cfg->rank, cfg->ecfg.max_level,
	 cfg->ecfg.max_length/pow(2.0, cfg->ecfg.max_level));
  printf("\t[%d] Min Level: %d (%lf m)\n", cfg->rank, cfg->ecfg.min_level, 
	 cfg->ecfg.max_length/pow(2.0, cfg->ecfg.min_level));
  printf("\t[%d] Tick Size: %e m\n", cfg->rank, cfg->ecfg.ticksize);
  printf("\t[%d] Col X,Y,Z Tics: %u, %u, %u\n\n", cfg->rank, 
	 cfg->ecfg.col_ticks[0], cfg->ecfg.col_ticks[1],
	 cfg->ecfg.max_ticks[2]);
  
  return(UCVM_CODE_SUCCESS);
}
