#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "etree.h"
#include "ucvm_utils.h"
#include "ucvm_meta_etree.h"
#include "ucvm_map.h"
#include "ucvm_proj_ucvm.h"


/* Etree buffer size in MB */
#define UCVM_MAP_BUF_SIZE 64


/* Map information */
int ucvm_map_init_flag = 0;
char ucvm_map_label_str[UCVM_MAX_LABEL_LEN];
etree_t *ucvm_map_ep = NULL;
ucvm_meta_map_t ucvm_map_meta;
ucvm_proj_t ucvm_map_proj;
int ucvm_map_level;
double ucvm_map_max_len;
etree_tick_t ucvm_map_edgetics;
double ucvm_map_edgesize;


/* Init Map */
int ucvm_map_init(const char *label, const char *conf)
{
  char *appmeta;

  if (ucvm_map_init_flag) {
    fprintf(stderr, "UCVM map interface is already initialized\n");
    return(UCVM_CODE_ERROR);
  }

  if ((conf == NULL) || (strlen(conf) == 0)) {
    fprintf(stderr, "No map path defined for map\n");
    return(UCVM_CODE_ERROR);
  }

  if ((label == NULL) || (strlen(label) == 0)) {
    fprintf(stderr, "Invalid map label\n");
    return(UCVM_CODE_ERROR);
  }

  if (!ucvm_is_file(conf)) {
    fprintf(stderr, "Etree map %s is not a valid file\n", conf);
    return(UCVM_CODE_ERROR);
  }

  /* Save label */
  ucvm_strcpy(ucvm_map_label_str, label, UCVM_MAX_LABEL_LEN);

  /* Open Etree map */
  ucvm_map_ep = etree_open(conf, O_RDONLY, UCVM_MAP_BUF_SIZE, 0, 3);
  if (ucvm_map_ep == NULL) {
    fprintf(stderr, "Failed to open the etree %s\n", conf);
    return(UCVM_CODE_ERROR);
  }

  /* Read meta data and check it */
  appmeta = etree_getappmeta(ucvm_map_ep);
  if (appmeta == NULL) {
    fprintf(stderr, "Failed to read metadata from etree %s\n", conf);
    return(UCVM_CODE_ERROR);
  }
  if (ucvm_meta_etree_map_unpack(appmeta, &ucvm_map_meta) != 
      UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to unpack metadata from etree %s\n", conf);
    return(UCVM_CODE_ERROR);
  }
  free(appmeta);

  /* Setup projection */
  if (ucvm_proj_ucvm_init(ucvm_map_meta.projstr, 
			  &(ucvm_map_meta.origin), 
			  ucvm_map_meta.rot,
			  &(ucvm_map_meta.dims_xyz),
			  &(ucvm_map_proj)) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to setup proj %s.\n", 
	    ucvm_map_meta.projstr);
    return(UCVM_CODE_ERROR);
  }

  /* Determine length of longest side */
  if (ucvm_map_meta.dims_xyz.coord[0] > ucvm_map_meta.dims_xyz.coord[1]) {
    ucvm_map_max_len = ucvm_map_meta.dims_xyz.coord[0];
  } else {
    ucvm_map_max_len = ucvm_map_meta.dims_xyz.coord[1];
  }

  /* Compute level based on grid spacing */
  ucvm_map_level = ceil(log(ucvm_map_max_len/
			    ucvm_map_meta.spacing) /log(2.0));

  /* Compute edge size in tics */
  ucvm_map_edgetics = (etree_tick_t)1 << (ETREE_MAXLEVEL - ucvm_map_level);

  /* Compute edge size in meters */
  ucvm_map_edgesize = ucvm_map_max_len/
    (double)((etree_tick_t)1<<ucvm_map_level);

  ucvm_map_init_flag = 1;

  return(UCVM_CODE_SUCCESS);
}


/* Finalize Map */
int ucvm_map_finalize()
{
  if (ucvm_map_init_flag) {
    etree_close(ucvm_map_ep);
    ucvm_proj_ucvm_finalize(&ucvm_map_proj);
  }

  ucvm_map_init_flag = 0;

  return(UCVM_CODE_SUCCESS);
}


/* Get Version Map */
int ucvm_map_version(char *ver, int len)
{
  int verlen;

  if (ucvm_map_init_flag == 0) {
    fprintf(stderr, "UCVM map interface not initialized\n");
    return(UCVM_CODE_ERROR);
  }

  verlen = strlen(ucvm_map_meta.date);
  if (verlen > len - 1) {
    verlen = len - 1;
  }
  memset(ver, 0, len);
  strncpy(ver, ucvm_map_meta.date, verlen);

  return(UCVM_CODE_SUCCESS);
}


/* Get Label Map */
int ucvm_map_label(char *label, int len)
{
  int labellen;

  if (ucvm_map_init_flag == 0) {
    fprintf(stderr, "UCVM map interface not initialized\n");
    return(UCVM_CODE_ERROR);
  }

  labellen = strlen(ucvm_map_label_str);
  if (labellen > len - 1) {
    labellen = len - 1;
  }
  memset(label, 0, len);
  strncpy(label, ucvm_map_label_str, labellen);

  return(UCVM_CODE_SUCCESS);
}


/* Query Map */
int ucvm_map_query(ucvm_ctype_t cmode,
		   int n, ucvm_point_t *pnt, ucvm_data_t *data)
{
  int i, x, y;
  int x0, y0;
  ucvm_point_t xy;
  etree_addr_t addr;
  double p[2][2];
  ucvm_mpayload_t q[2][2];
  
  if (ucvm_map_init_flag == 0) {
    fprintf(stderr, "UCVM map interface is not initialized");
    return(UCVM_CODE_ERROR);
  }

  /* Check query mode */
  switch (cmode) {
  case UCVM_COORD_GEO_DEPTH:
  case UCVM_COORD_GEO_ELEV:
    break;
  default:
    fprintf(stderr, "Unsupported coord type\n");
    return(UCVM_CODE_ERROR);
    break;
  }

  /* Corners of interpolation plane */
  p[0][0] = 0.0;
  p[0][1] = 0.0;
  p[1][0] = 1.0;
  p[1][1] = 1.0;

  for (i = 0; i < n; i++) {
    /* Convert point from geo to xy offset in meters */
    if (ucvm_proj_ucvm_geo2xy(&(ucvm_map_proj), 
			      &(pnt[i]),
			      &xy) == UCVM_CODE_SUCCESS) {
      if ((xy.coord[0] >= 0.0) && 
	  (xy.coord[0] < ucvm_map_meta.dims_xyz.coord[0]) &&
	  (xy.coord[1] >= 0.0) &&
	  (xy.coord[1] < ucvm_map_meta.dims_xyz.coord[1])) {

	/* Calculate grid location in fp and integer octants */
	xy.coord[0] = xy.coord[0]/ucvm_map_edgesize;
	xy.coord[1] = xy.coord[1]/ucvm_map_edgesize;

	x0 = (int)(xy.coord[0]);
	y0 = (int)(xy.coord[1]);

	/* Determine q values for interpolation */
	for (y = 0; y < 2; y++) {
	  for (x = 0; x < 2; x++) {
	    addr.x = (x0 + x)*ucvm_map_edgetics;
	    addr.y = (y0 + y)*ucvm_map_edgetics;
	    addr.z = 0;
	    addr.level = ETREE_MAXLEVEL;
	    
	    /* Adjust addresses for edges of grid */
	    if (addr.x >= ucvm_map_meta.ticks_xyz.dim[0]) {
	      addr.x = ucvm_map_meta.ticks_xyz.dim[0] - ucvm_map_edgetics; 
	    }
	    if (addr.y >= ucvm_map_meta.ticks_xyz.dim[1]) {
	      addr.y = ucvm_map_meta.ticks_xyz.dim[1] - ucvm_map_edgetics; 
	    }
	    
	    /* Query etree */
	    if (etree_search(ucvm_map_ep, addr, NULL, "*", &(q[y][x])) == 0) {
	      //printf("vals: %lf, %lf\n", q[y][x].surf, q[y][x].vs30);
	    } else {
	      fprintf(stderr, "%s (%d %d %d)\n", 
		      etree_strerror(etree_errno(ucvm_map_ep)),
		      addr.x, addr.y, addr.z);
	      return(UCVM_CODE_ERROR);
	    }
	  }
	}

	/* Bilinear interpolation of values */
	data[i].surf = interpolate_bilinear(xy.coord[0]-x0, 
					    xy.coord[1]-y0, 
					    p[0][0], p[0][1],
					    p[1][0], p[1][1], 
					    q[0][0].surf, 
					    q[0][1].surf, 
					    q[1][0].surf, 
					    q[1][1].surf);
	
	data[i].vs30 = interpolate_bilinear(xy.coord[0]-x0, 
					    xy.coord[1]-y0, 
					    p[0][0], p[0][1],
					    p[1][0], p[1][1], 
					    q[0][0].vs30, 
					    q[0][1].vs30, 
					    q[1][0].vs30, 
					    q[1][1].vs30);

      }
    }
  }

  return(UCVM_CODE_SUCCESS);
}


