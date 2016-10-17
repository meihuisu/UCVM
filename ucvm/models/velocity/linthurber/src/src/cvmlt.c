#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "cvmlt.h"
#include "cvmlt_proj_bilinear.h"
#include "cvmlt_config.h"
#include "cvmlt_utils.h"


/* Model config */
int cvmlt_init_flag = 0;
char cvmlt_version_id[CVMLT_CONFIG_MAX_STR];
cvmlt_bilinear_t cvmlt_proj;


/* Model dimensions */
#define CVMLT_MAX_Z_DIM 100
int cvmlt_z_dim = 0;
double cvmlt_depths_msl[CVMLT_MAX_Z_DIM];
double cvmlt_vp_spacing = 0.0;
double cvmlt_vs_spacing = 0.0;
int cvmlt_vp_dims[3];
int cvmlt_vs_dims[3];
double cvmlt_vp_origin[2];
double cvmlt_vs_origin[2];

/* Model volumes */
float *cvmlt_vp_buf = NULL;
float *cvmlt_vs_buf = NULL;

/* Property constants */
#define CVMLT_VP 0
#define CVMLT_VS 1


/* Initialize */
int cvmlt_init(const char *dir)
{
  int i, j, k;
  FILE *fp;
  char filename[CVMLT_MAX_STR_LEN];
  float dep, x, y, val;
  int num_read, retval;

  cvmlt_config_t *cfg = NULL;
  cvmlt_config_t *cfgentry = NULL;

  if (cvmlt_init_flag) {
    fprintf(stderr, "LT Model is already initialized\n");
    return(1);
  }

  if ((dir == NULL) || (strlen(dir) == 0)) {
    fprintf(stderr, "No config path defined for model\n");
    return(1);
  }

  /* Read CONF file */
  sprintf(filename, "%s/lt.conf", dir);
  cfg = cvmlt_parse_config(filename);
  if (cfg == NULL) {
    fprintf(stderr, "Failed to read LT config file\n");
    return(1);
  }

  cfgentry = cvmlt_find_name(cfg, "version");
  if (cfgentry == NULL) {
    fprintf(stderr, "Failed to find version key\n");
    return(1);
  }
  strcpy(cvmlt_version_id, cfgentry->value);

  cfgentry = cvmlt_find_name(cfg, "proj_xi");
  if (cfgentry == NULL) {
    fprintf(stderr, "Failed to find proj_xi key\n");
    return(1);
  }
  if (cvmlt_list_parse(cfgentry->value, 4, 
		    cvmlt_proj.xi) != 0) {
    fprintf(stderr, "Failed to parse %s value\n", cfgentry->name);
    return(1);
  }

  cfgentry = cvmlt_find_name(cfg, "proj_yi");
  if (cfgentry == NULL) {
    fprintf(stderr, "Failed to find proj_yi key\n");
    return(1);
  }
  if (cvmlt_list_parse(cfgentry->value, 4, 
		    cvmlt_proj.yi) != 0) {
    fprintf(stderr, "Failed to parse %s value\n", cfgentry->name);
    return(1);
  }

  cfgentry = cvmlt_find_name(cfg, "proj_size");
  if (cfgentry == NULL) {
    fprintf(stderr, "Failed to find proj_size key\n");
    return(1);
  }
  if (cvmlt_list_parse(cfgentry->value, 2, 
		    cvmlt_proj.dims) != 0) {
    fprintf(stderr, "Failed to parse %s value\n", cfgentry->name);
    return(1);
  }

  cfgentry = cvmlt_find_name(cfg, "spacing_vp");
  if (cfgentry == NULL) {
    fprintf(stderr, "Failed to find spacing_vp key\n");
    return(1);
  }
  cvmlt_vp_spacing = atof(cfgentry->value);

  cfgentry = cvmlt_find_name(cfg, "spacing_vs");
  if (cfgentry == NULL) {
    fprintf(stderr, "Failed to find spacing_vs key\n");
    return(1);
  }
  cvmlt_vs_spacing = atof(cfgentry->value);

  cfgentry = cvmlt_find_name(cfg, "num_z");
  if (cfgentry == NULL) {
    fprintf(stderr, "Failed to find num_z key\n");
    return(1);
  }
  cvmlt_z_dim = atoi(cfgentry->value);
  if (cvmlt_z_dim > CVMLT_MAX_Z_DIM) {
    fprintf(stderr, "Value for key num_z exceeds %d\n", CVMLT_MAX_Z_DIM);
    return(1);
  }

  cfgentry = cvmlt_find_name(cfg, "z_vals");
  if (cfgentry == NULL) {
    fprintf(stderr, "Failed to find z_vals key\n");
    return(1);
  }
  if (cvmlt_list_parse(cfgentry->value, cvmlt_z_dim, 
		    cvmlt_depths_msl) != 0) {
    fprintf(stderr, "Failed to parse %s value\n", cfgentry->name);
    return(1);
  }

  cfgentry = cvmlt_find_name(cfg, "grid_origin");
  if (cfgentry == NULL) {
    fprintf(stderr, "Failed to find grid_origin key\n");
    return(1);
  }
  if (cvmlt_list_parse(cfgentry->value, 2, 
		    cvmlt_vp_origin) != 0) {
    fprintf(stderr, "Failed to parse %s value\n", cfgentry->name);
    return(1);
  }
  cvmlt_vs_origin[0] = cvmlt_vp_origin[0];
  cvmlt_vs_origin[1] = cvmlt_vp_origin[1];

  cvmlt_free_config(cfg);

  /* Calculate model dims */
  cvmlt_vp_dims[0] = cvmlt_proj.dims[0] / cvmlt_vp_spacing + 1;
  cvmlt_vp_dims[1] = cvmlt_proj.dims[1] / cvmlt_vp_spacing + 1;
  cvmlt_vp_dims[2] = cvmlt_z_dim;

  cvmlt_vs_dims[0] = cvmlt_proj.dims[0] / cvmlt_vs_spacing + 1;
  cvmlt_vs_dims[1] = cvmlt_proj.dims[1] / cvmlt_vs_spacing + 1;
  cvmlt_vs_dims[2] = cvmlt_z_dim;

  //printf("Calculated:\n");
  //printf("\tVp Dims: %d, %d, %d\n", cvmlt_vp_dims[0],
  //	 cvmlt_vp_dims[1],
  //	 cvmlt_vp_dims[2]);
  //printf("\tVs Dims: %d, %d, %d\n", cvmlt_vs_dims[0],
  //	 cvmlt_vs_dims[1],
  //	 cvmlt_vs_dims[2]);

  /* Allocate buffers */
  cvmlt_vp_buf = malloc(cvmlt_vp_dims[0] * cvmlt_vp_dims[1] *
			cvmlt_vp_dims[2] * sizeof(float));
  cvmlt_vs_buf = malloc(cvmlt_vs_dims[0] * cvmlt_vs_dims[1] *
			cvmlt_vs_dims[2] * sizeof(float));
  if ((cvmlt_vp_buf == NULL) || (cvmlt_vs_buf == NULL)) {
    fprintf(stderr, "Failed to allocate buffers LT model\n");
    return(1);
  }

  /* Initialize buffers */
  for (i = 0; i < cvmlt_vp_dims[0] * cvmlt_vp_dims[1] *
	 cvmlt_vp_dims[2]; i++) {
    cvmlt_vp_buf[i] = -1.0;
  }
  for (i = 0; i < cvmlt_vs_dims[0] * cvmlt_vs_dims[1] *
	 cvmlt_vs_dims[2]; i++) {
    cvmlt_vs_buf[i] = -1.0;
  }

  /* Load Vp velocity file*/
  sprintf(filename, "%s/lt.vp", dir);
  num_read = 0;
  fp = fopen(filename, "r");
  while (!feof(fp)) {
    retval = fscanf(fp, "%*f %*f %f %f %f %f %*f", &dep, &y, &x, &val);
    if (retval != EOF) {
      if (retval != 4) {
	fprintf(stderr, 
		"Failed to read LT Vp file, line %d (parsed %d)\n",
		num_read, retval);
	return(1);
      }
      for (k = 0; k < cvmlt_z_dim; k++) {
	if (cvmlt_depths_msl[k] >= dep) {
	  break;
	}
      }
      /* Flip x and y axis */
      j = round((y + (-cvmlt_vp_origin[0])) 
		* 1000.0 / cvmlt_vp_spacing);
      i = round((cvmlt_proj.dims[0]/1000.0 - 
		 (x + (-cvmlt_vp_origin[1]))) 
		* 1000.0 / cvmlt_vp_spacing);
      if ((i < 0) || (j < 0) || (k < 0) || 
	  (i >= cvmlt_vp_dims[0]) || 
	  (j >= cvmlt_vp_dims[1]) || 
	  (k >= cvmlt_vp_dims[2])) {
	fprintf(stderr, "Invalid index %d,%d,%d calculated\n", i, j, k);
	return(1);
      }
      cvmlt_vp_buf[k*cvmlt_vp_dims[0]*cvmlt_vp_dims[1] + 
		   j*cvmlt_vp_dims[0] + i] = val * 1000.0;
      num_read++;
    }
  }
  fclose(fp);

  /* Load Vs velocity file */
  sprintf(filename, "%s/lt.vs", dir);
  num_read = 0;
  fp = fopen(filename, "r");
  while (!feof(fp)) {
    retval = fscanf(fp, "%*f %*f %f %f %f %f %*f", &dep, &y, &x, &val);
    if (retval != EOF) {
      if (retval != 4) {
	fprintf(stderr, 
		"Failed to read LT Vs file, line %d (parsed %d)\n",
		num_read, retval);
	return(1);
      }
      for (k = 0; k < cvmlt_z_dim; k++) {
	if (cvmlt_depths_msl[k] >= dep) {
	  break;
	}
      }
      /* Flip x and y axis */
      j = round((y + (-cvmlt_vs_origin[0])) 
		* 1000.0 / cvmlt_vs_spacing);
      i = round((cvmlt_proj.dims[0]/1000.0 - 
		 (x + (-cvmlt_vs_origin[1]))) 
		* 1000.0 / cvmlt_vs_spacing);
      if ((i < 0) || (j < 0) || (k < 0) || 
	  (i >= cvmlt_vs_dims[0]) || 
	  (j >= cvmlt_vs_dims[1]) || 
	  (k >= cvmlt_vs_dims[2])) {
	fprintf(stderr, "Invalid index %d,%d,%d calculated\n", i, j, k);
	return(1);
      }
      cvmlt_vs_buf[k*cvmlt_vs_dims[0]*cvmlt_vs_dims[1] + 
		   j*cvmlt_vs_dims[0] + i] = val * 1000.0;
      num_read++;
    }
  }
  fclose(fp);

  cvmlt_init_flag = 1;

  return(0);
}


/* Finalize */
int cvmlt_finalize()
{
  if (cvmlt_vp_buf != NULL) {
    free(cvmlt_vp_buf);
  }
  cvmlt_vp_buf = NULL;
  if (cvmlt_vs_buf != NULL) {
    free(cvmlt_vs_buf);
  }
  cvmlt_vs_buf = NULL;

  cvmlt_init_flag = 0;
  return(0);
}


/* Version ID */
int cvmlt_version(char *ver, int len)
{
  int verlen;

  if (!cvmlt_init_flag) {
    fprintf(stderr, "LT Model not initialized\n");
    return(1);
  }

  verlen = strlen(cvmlt_version_id);
  if (verlen > len - 1) {
    verlen = len - 1;
  }
  memset(ver, 0, len);
  strncpy(ver, cvmlt_version_id, verlen);
  return(0);
}


/* Get specified material property at grid location i,j,k */
int cvmlt_getval(double i, double j, double k, int prop, float *val)
{
  int i0, j0, k0;
  int a, b, c, x, y, z;
  int *dims = NULL;
  float *buf = NULL;
  double p[2][3];
  double q[2][2][2];

  *val = -1.0;

  i0 = (int)i;
  j0 = (int)j;
  k0 = (int)k;

  switch (prop) {
  case CVMLT_VP:
    dims = cvmlt_vp_dims;
    buf = cvmlt_vp_buf;
    break;
  case CVMLT_VS:
    dims = cvmlt_vs_dims;
    buf = cvmlt_vs_buf;
   break;
  };

  /* Check if point falls outside of model region */
  if ((i0 < 0) || (j0 < 0) || (k0 < 0) || 
      (i0 >= dims[0]) || (j0 >= dims[1]) || (k0 >= dims[2])) {
    return(1);
  }

  /* Values at corners of interpolation cube */
  for (z = 0; z < 2; z++) {
    for (y = 0; y < 2; y++) {
      for (x = 0; x < 2; x++) {

	a = i0 + x;
	b = j0 + y;
	c = k0 + z;

	if (a < 0) {
	  a = 0;
	}
	if (b < 0) {
	  b = 0;
	}
	if (c < 0) {
	  c = 0;
	}
	if (a >= dims[0]) {
	  a = dims[0] - 1;
	}
	if (b >= dims[1]) {
	  b = dims[1] - 1;
	}
	if (c >= dims[2]) {
	  c = dims[2] - 1;
	}

	q[z][y][x] = buf[c*dims[0]*dims[1] + b*dims[0] + a];
      }
    }
  }

  /* Corners of interpolation cube */
  for (b = 0; b < 2; b++) {
    for (a = 0; a < 3; a++) { 
      p[b][a] = (double)b;
    }
  }

  /* Trilinear interpolation */
  *val = cvmlt_interpolate_trilinear(i-i0, j-j0, k-k0, p, q);

  return(0);
}


/* Query */
int cvmlt_query(cvmlt_point_t *pnt, cvmlt_data_t *data)
{
  int k;
  double depth, depth_msl;
  cvmlt_point_t xy;

  if (!cvmlt_init_flag) {
    fprintf(stderr, "LT Model not initialized\n");
    return(1);
  }

  data->vp = 0.0;
  data->vs = 0.0;
  data->rho = 0.0;

  /* Query model by elevation */
  if (cvmlt_bilinear_geo2xy(&cvmlt_proj, pnt, &xy) == 0) {
    depth = pnt->coord[2];
    depth_msl = depth/1000.0;
    for (k = 0; k < cvmlt_z_dim; k++) {
      if (cvmlt_depths_msl[k] >= depth_msl) {
	break;
      }
    }
    if (k == cvmlt_z_dim) {
      k = cvmlt_z_dim - 1;
    }
    xy.coord[2] = k;
    
    /* Vp value */
    cvmlt_getval(xy.coord[0] / cvmlt_vp_spacing,
		 xy.coord[1] / cvmlt_vp_spacing,
		 xy.coord[2], 
		 CVMLT_VP, &(data->vp));
    
    /* Vs value */
    cvmlt_getval(xy.coord[0] / cvmlt_vs_spacing,
		 xy.coord[1] / cvmlt_vs_spacing,
		 xy.coord[2], 
		 CVMLT_VS, &(data->vs));
    
  }
  
  /* Calculate density */
  if (data->vp > 0.0) {
    data->rho = cvmlt_nafe_drake_rho(data->vp);
  }

  return(0);
}




