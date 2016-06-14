#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include "ue_extract.h"
#include "ue_utils.h"
#include "ucvm_proj_bilinear.h"
#include "ucvm_proj_ucvm.h"

/* Insert 2D grid at required resolution in a buffer */
int insert_grid_buf(ue_cfg_t *cfg,
		     etree_addr_t *pnts, ucvm_data_t *props, 
		     int num_points)
{
  int i;
  ue_octant_t *oct;

  if ((cfg->ecfg.bufp[0] == NULL) || (cfg->ecfg.efp[0] == NULL)) {
    fprintf(stderr, "[%d] Open file desc / buffer not found\n", cfg->rank);
    return(UCVM_CODE_ERROR);
  }

  //if (cfg->ecfg.num_octants[0] + num_points > cfg->ecfg.max_octants) {
  //  if (fwrite(cfg->ecfg.bufp[0], sizeof(ue_octant_t),
  //		       cfg->ecfg.num_octants[0], cfg->ecfg.efp[0]) != 
  //	cfg->ecfg.num_octants[0]) {
  //    fprintf(stderr, "[%d] Failed to write buffer to disk\n", cfg->rank);
  //    return(UCVM_CODE_ERROR);
  //  }
  //  cfg->ecfg.num_octants[0] = 0;
  //}

  /* Store these points in buffer */
  for (i = 0; i < num_points; i++) {
    if (cfg->ecfg.num_octants[0] == cfg->ecfg.max_octants) {
      /* Flush full buffer */
      if (fwrite(cfg->ecfg.bufp[0], sizeof(ue_octant_t),
		 cfg->ecfg.num_octants[0], cfg->ecfg.efp[0]) != 
	  cfg->ecfg.num_octants[0]) {
	fprintf(stderr, "[%d] Failed to write buffer to disk\n", cfg->rank);
	return(UCVM_CODE_ERROR);
      }
      cfg->ecfg.num_octants[0] = 0;
    }

    /* Save this point */
    oct = ((cfg->ecfg.bufp[0]) + cfg->ecfg.num_octants[0]);
    (cfg->ecfg.num_octants[0])++;
    memcpy(&(oct->addr), &(pnts[i]), sizeof(etree_addr_t));
    ue_addr2key(pnts[i], oct->key);
    oct->payload.Vp = (float)props[i].cmb.vp;
    oct->payload.Vs = (float)props[i].cmb.vs;
    oct->payload.density = (float)props[i].cmb.rho;
  }

  return(UCVM_CODE_SUCCESS);
}


/* Insert 2D grid at required resolution in etree */
int insert_grid_etree(ue_cfg_t *cfg,
		      etree_addr_t *pnts, ucvm_data_t *props, 
		      int num_points)
{
  int i;
  ucvm_epayload_t prop;
  etree_error_t err;

  if (cfg->ecfg.ep[0] == NULL) {
    fprintf(stderr, "[%d] Open etree not found\n", cfg->rank);
    return(UCVM_CODE_ERROR);
  }

  for (i = 0; i < num_points; i++) {
    prop.Vp = props[i].cmb.vp;
    prop.Vs = props[i].cmb.vs;
    prop.density = props[i].cmb.rho;

    /* Inserts data into the etree */
    if (etree_insert(cfg->ecfg.ep[0], pnts[i], &prop) != 0)  {
	err = etree_errno(cfg->ecfg.ep[0]);
	fprintf(stderr, "[%d] %s\n", cfg->rank, etree_strerror(err));
	return(UCVM_CODE_ERROR);
    }
  }

  return(UCVM_CODE_SUCCESS);
}


/* Get 2D grid at required resolution */
int get_grid(ue_cfg_t *cfg, int x, int y, etree_tick_t z, int level, 
	     ucvm_point_t *cvm_pnts, etree_addr_t *etree_pnts,
	     int *num_points)
{
  int i, j;
  double edgesize;
  etree_tick_t edgetics;
  double grid_z;
  etree_tick_t imin, imax, jmin, jmax;
  ucvm_point_t xy;

  *num_points = 0;
  edgesize = cfg->ecfg.max_length/(double)((etree_tick_t)1<<level);
  edgetics = (etree_tick_t)1 << (ETREE_MAXLEVEL - level);
  grid_z = (z * cfg->ecfg.ticksize) + edgesize/2.0;

  /* Convert column x,y and level to tick i,j */
  imin = x*cfg->ecfg.col_ticks[0];
  jmin = y*cfg->ecfg.col_ticks[1];
  imax = (x+1)*cfg->ecfg.col_ticks[0];
  jmax = (y+1)*cfg->ecfg.col_ticks[1];

  if (strcmp(cfg->projinfo.projstr, PROJ_GEO_BILINEAR) == 0) {
    for (j = jmin; j < jmax; j = j + edgetics) {
      for (i = imin; i < imax; i = i + edgetics) {
	xy.coord[0] = (i+(edgetics/2.0)) * cfg->ecfg.ticksize;
	xy.coord[1] = (j+(edgetics/2.0)) * cfg->ecfg.ticksize;
	if (ucvm_bilinear_xy2geo(&(cfg->projinfo.projb), &xy,
				 &(cvm_pnts[*num_points]))!= 
	    UCVM_CODE_SUCCESS) {
	  fprintf(stderr, "[%d] Bilinear interp failed for %d, %d\n", 
		  cfg->rank, x, y);
	  return(UCVM_CODE_ERROR);
	}
	cvm_pnts[*num_points].coord[2] = grid_z;

	etree_pnts[*num_points].x = i;
	etree_pnts[*num_points].y = j;
	etree_pnts[*num_points].z = z;
	etree_pnts[*num_points].level = level;
	etree_pnts[*num_points].type = ETREE_LEAF;
	*num_points = *num_points + 1;
      }
    }
  } else {
    for (j = jmin; j < jmax; j = j + edgetics) {
      for (i = imin; i < imax; i = i + edgetics) {
	xy.coord[0] = (i+(edgetics/2.0)) * cfg->ecfg.ticksize;
	xy.coord[1] = (j+(edgetics/2.0)) * cfg->ecfg.ticksize;
	if (ucvm_proj_ucvm_xy2geo(&(cfg->projinfo.proj), &xy,
				  &(cvm_pnts[*num_points]))
	    != UCVM_CODE_SUCCESS) {
	  fprintf(stderr, "[%d] UCVM projection failed for %d, %d\n", 
		  cfg->rank, x, y);
	  return(UCVM_CODE_ERROR);
	}
	cvm_pnts[*num_points].coord[2] = grid_z;

	etree_pnts[*num_points].x = i;
	etree_pnts[*num_points].y = j;
	etree_pnts[*num_points].z = z;
	etree_pnts[*num_points].level = level;
	etree_pnts[*num_points].type = ETREE_LEAF;
	*num_points = *num_points + 1;
      }
    }
  }

  return(UCVM_CODE_SUCCESS);
}


/* Scan properties for lowest Vs and calculated desired etree level */
int get_level(ue_cfg_t *cfg, int n, ucvm_point_t *pnts, ucvm_data_t *props, 
	      int *level, int *min_index, double *vs_min)
{
  int i;
  //double vs_min;
  double min_edgesize;

  *level = 0;
  if (n == 0) {
    return(UCVM_CODE_ERROR);
  }

  *vs_min = props[0].cmb.vs;
  *min_index = 0;
  for (i = 0; i < n; i++) {
    if ((props[i].cmb.vs <= 0.0) || 
	(props[i].cmb.vp <= 0.0) || 
	(props[i].cmb.rho <= 0.0)) {
      fprintf(stderr, 
	      "[%d] Invalid props at %lf,%lf,%lf: vs=%lf, vp=%lf, rho=%lf\n", 
	      cfg->rank, pnts[i].coord[0], 
	      pnts[i].coord[1], pnts[i].coord[2],
	      props[i].cmb.vs, props[i].cmb.vp, props[i].cmb.rho);
      return(UCVM_CODE_ERROR);
    }
    if (props[i].cmb.vs < *vs_min) {
      *vs_min = props[i].cmb.vs;
      *min_index = i;
    }
  }

  /* Compute minimum level needed to support with vs */
  min_edgesize = *vs_min / (cfg->ppwl * cfg->max_freq);
  *level = log(cfg->ecfg.max_length/min_edgesize)/log(2.0) + 1;
  return(UCVM_CODE_SUCCESS);
}


/* Perform extraction */
int extract(ue_cfg_t *cfg, 
	    int (*etree_writer)(ue_cfg_t *cfg, 
				etree_addr_t *pnts, 
				ucvm_data_t *props, 
				int num_points),
	    int col,
	    ucvm_point_t *cvm_pnts,
	    etree_addr_t *etree_pnts,
	    ucvm_data_t *props,
	    unsigned long *num_extracted)
{
  int i, j, l, n;
  int level, scanlevel;
  double edgesize = 0.0;
  etree_tick_t edgetics, ztics;
  etree_tick_t maxrez;
  int min_index;
  double vs_min;

  /* Points and properties */
  etree_tick_t max_points;
  int num_points;

  /* Performance measurements */
  struct timeval start, end;
  double elapsed;

  *num_extracted = 0;

  /* Allocate buffers */
  maxrez = (etree_tick_t)1 << (ETREE_MAXLEVEL - cfg->ecfg.max_level);
  max_points = (cfg->ecfg.col_ticks[0]/maxrez) * 
     (cfg->ecfg.col_ticks[1]/maxrez);

  j = col / cfg->col_dims.dim[0];
  i = col % cfg->col_dims.dim[0];

  level = cfg->ecfg.max_level;
  edgesize = cfg->ecfg.max_length/(double)((etree_tick_t)1<<level);
  edgetics = (etree_tick_t)1 << (ETREE_MAXLEVEL - level);
  ztics = 0;
  num_points = 0;
	
  gettimeofday(&start,NULL);
  
  /* Generate new grid at max level */
  if ((get_grid(cfg, i, j, ztics, level, cvm_pnts, etree_pnts,
		&num_points) != 0) || (num_points == 0)) {
    fprintf(stderr, 
	    "[%d] Failed to generate grid for %d,%d,ztics=%u\n",
	    cfg->rank, i, j, ztics);
    return(UCVM_CODE_ERROR);
  }
  if (num_points > max_points) {
    fprintf(stderr, 
	    "[%d] Num points exceeds max points at %d,%d,ztics=%u\n",
	    cfg->rank, i, j, ztics);
    return(UCVM_CODE_ERROR);
  }
  
  while (ztics < cfg->ecfg.max_ticks[2]) {
    
    /* Ensure ztics is appropriate for current level */
    if (ztics % ((etree_tick_t)(1 << (ETREE_MAXLEVEL - level))) != 0) {
      fprintf(stderr, 
	      "[%d] Failed ztics level check at %d,%d,ztics=%u\n",
	      cfg->rank, i, j, ztics);
      return(UCVM_CODE_ERROR);
    }
    
    /* Sample x-y slice @ z depth at 'level' resolution */
    if (ucvm_query(num_points, cvm_pnts, props) != UCVM_CODE_SUCCESS) {
      fprintf(stderr, 
	      "[%d] Failed to query grid for %d,%d,ztics=%u\n",
	      cfg->rank, i, j, ztics);
      return(UCVM_CODE_ERROR);
    }
    
    /* Determine real level needed for this x-y slice */
    if (get_level(cfg, num_points, cvm_pnts, props, &scanlevel, 
		  &min_index, &vs_min) != 0) {
      fprintf(stderr, 
	      "[%d] Failed to scan grid for %d,%d,ztics=%u\n",
	      cfg->rank, i, j, ztics);
      return(UCVM_CODE_ERROR);
    }
  
    /* Print point 0 */
    //if (ztics == 0) {
    //  printf("\t%lf,%lf,%lf: level=%d, scanlevel=%d, vs_min=%lf\n", 
    //	     cvm_pnts[min_index].coord[0], cvm_pnts[min_index].coord[1], 
    //	     cvm_pnts[min_index].coord[2], 
    //	     level, scanlevel, vs_min);
    //}

    /* Clamp scanlevel to min/max level */
    if (scanlevel < cfg->ecfg.min_level) {
      scanlevel = cfg->ecfg.min_level;
    } else if (scanlevel > cfg->ecfg.max_level) {
      scanlevel = cfg->ecfg.max_level;
    }
    
    if (level < scanlevel) {
      /* Increase resolution */
      level = scanlevel;
      edgesize = cfg->ecfg.max_length/(double)((etree_tick_t)1<<level);
      edgetics = (etree_tick_t)1 << (ETREE_MAXLEVEL - level);
      if ((get_grid(cfg, i, j, ztics, level, cvm_pnts, 
		    etree_pnts, &num_points) != 0) || 
	  (num_points == 0)) {
	fprintf(stderr, 
		"[%d] Failed to re-generate grid for %d,%d,ztics=%u\n",
		cfg->rank, i, j, ztics);
	return(UCVM_CODE_ERROR);
      }

      if (num_points > max_points) {
	fprintf(stderr, 
		"[%d] Num points exceeds max points at %d,%d,ztics=%u\n",
		cfg->rank, i, j, ztics);
	return(UCVM_CODE_ERROR);
      }
      
      /* Sample x-y slice @ z depth at 'level' resolution */
      if (ucvm_query(num_points, cvm_pnts, props) != UCVM_CODE_SUCCESS) {
	fprintf(stderr, 
		"[%d] Failed to query grid for %d,%d,ztics=%u\n",
		cfg->rank, i, j, ztics);
	return(UCVM_CODE_ERROR);
      }

    } else if (level > scanlevel) {
      for (l = scanlevel; l < level; l++) {
	if ((ztics % ((etree_tick_t)(1 << (ETREE_MAXLEVEL - l))) == 0) &&
	    (ztics + ((etree_tick_t)(1 << (ETREE_MAXLEVEL - l))) <= cfg->ecfg.max_ticks[2])) {

	  /* Level is supported at this depth. 
	     Decrease resolution and requery */
	  level = l;
	  edgesize = cfg->ecfg.max_length/(double)((etree_tick_t)1<<level);
	  edgetics = (etree_tick_t)1 << (ETREE_MAXLEVEL - level);
	  if ((get_grid(cfg, i, j, ztics, level, cvm_pnts, 
			etree_pnts, &num_points) != 0) || 
	      (num_points == 0)) {
	    fprintf(stderr, 
		    "[%d] Failed to re-generate grid for %d,%d,ztics=%u\n",
		    cfg->rank, i, j, ztics);
	    return(UCVM_CODE_ERROR);
	  }
	  
	  /* Sample x-y slice @ z depth at 'level' resolution */
	  if (ucvm_query(num_points, cvm_pnts, props) != UCVM_CODE_SUCCESS) {
	    fprintf(stderr, 
		    "[%d] Failed to query grid for %d,%d,ztics=%u\n",
		    cfg->rank, i, j, ztics);
	    return(UCVM_CODE_ERROR);
	  }

	  /* Break out of loop */
	  break;
	}
      }
    }

    for (n = 0; n < num_points; n++) {
      if ((props[n].cmb.vs <= 0.0) || 
	  (props[n].cmb.vp <= 0.0) || 
	  (props[n].cmb.rho <= 0.0)) {
	fprintf(stderr, 
		"[%d] Negative property detected at %d,%d,ztics=%u\n",
		cfg->rank, i, j, ztics);
	fprintf(stderr, "[%d] vp=%lf, vs=%lf, rho=%lf\n",
		cfg->rank, props[n].cmb.vp, props[n].cmb.vs, props[n].cmb.rho);
	return(UCVM_CODE_ERROR);
      }
    }
     
    /* Print point 0 */
    //if (ztics == 0) {
    //  printf("\tvp, vs, rho: %lf,%lf,%lf -> %lf, %lf, %lf\n", 
    //	     cvm_pnts[0].coord[0], cvm_pnts[0].coord[1],
    //	     cvm_pnts[0].coord[2], props[0].cmb.vp,
    //	     props[0].cmb.vs, props[0].cmb.rho);
    //}
 
    /* Insert x-y slice into etree/file */
    if (etree_writer(cfg, etree_pnts, props, num_points) != 0) {
      fprintf(stderr, 
	      "[%d] Failed to insert points for %d,%d,ztics=%u\n",
	      cfg->rank, i, j, ztics);
      return(UCVM_CODE_ERROR);
    }
    *num_extracted = *num_extracted + (size_t)num_points;
    
    ztics = ztics + edgetics;
    
    /* Increment Z value in existing grid */
    for (n = 0; n < num_points; n++) {
      cvm_pnts[n].coord[2] = (ztics * cfg->ecfg.ticksize) + 
	edgesize/2.0;
      etree_pnts[n].z = ztics;
    }
  }
  
  gettimeofday(&end,NULL);
  elapsed = (end.tv_sec - start.tv_sec) * 1000.0 +
    (end.tv_usec - start.tv_usec) / 1000.0;
  //printf("[%d] Finished col %d,%d in %.2f s\n", cfg->rank,
  //	 i, j, elapsed / 1000.0);
  //fflush(stdout);
  
  if (ztics != cfg->ecfg.max_ticks[2]) {
    fprintf(stderr, "ztics mismatch, was %u, expected %u\n", ztics,
	    cfg->ecfg.max_ticks[2]);
    return(UCVM_CODE_ERROR);
  }

  return(UCVM_CODE_SUCCESS);
}
