#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ucvm_utils.h"
#include "ucvm_model_cvms.h"
#include "cvms.h"

/* Fortran constants */
#define CVMS_FORTRAN_MODELDIR_LEN 128
#define CVMS_FORTRAN_VERSION_LEN 64

/* Init flag */
int ucvm_cvms_init_flag = 0;

/* Maximum number of points to query */
#define CVMS_MAX_POINTS 1000000

/* Buffers initialized flag */
int cvms_buf_init = 0;

/* Query buffers */
int *cvms_index = NULL;
float *cvms_lon = NULL;
float *cvms_lat = NULL;
float *cvms_dep = NULL;
float *cvms_vp = NULL;
float *cvms_vs = NULL;
float *cvms_rho = NULL;


/* Model ID */
int ucvm_cvms_id = UCVM_SOURCE_NONE;

/* Model conf */
ucvm_modelconf_t ucvm_cvms_conf;


/* Init CVM-S */
int ucvm_cvms_model_init(int id, ucvm_modelconf_t *conf)
{
  int errcode;
  /* Fortran fixed string length */
  char modeldir[CVMS_FORTRAN_MODELDIR_LEN];

  if (ucvm_cvms_init_flag) {
    fprintf(stderr, "Model %s is already initialized\n", conf->label);
    return(UCVM_CODE_ERROR);
  }

  if ((conf->config == NULL) || (strlen(conf->config) == 0)) {
    fprintf(stderr, "No config path defined for model %s\n", conf->label);
    return(UCVM_CODE_ERROR);
  }

  if (strlen(conf->config) >= CVMS_FORTRAN_MODELDIR_LEN) {
    fprintf(stderr, "Config path too long for model %s\n", conf->label);
    return(UCVM_CODE_ERROR);
  }
  ucvm_strcpy(modeldir, conf->config, CVMS_FORTRAN_MODELDIR_LEN);

  cvms_init_(modeldir, &errcode);
  if (errcode != 0) {
    fprintf(stderr, "Failed to init CVM-S\n");
    return(UCVM_CODE_ERROR);
  }

  /* Allocate buffers */
  if (cvms_buf_init == 0) {
    cvms_index = malloc(CVMS_MAX_POINTS*sizeof(int));
    cvms_lon = malloc(CVMS_MAX_POINTS*sizeof(float));
    cvms_lat = malloc(CVMS_MAX_POINTS*sizeof(float));
    cvms_dep = malloc(CVMS_MAX_POINTS*sizeof(float));
    cvms_vp = malloc(CVMS_MAX_POINTS*sizeof(float));
    cvms_vs = malloc(CVMS_MAX_POINTS*sizeof(float));
    cvms_rho = malloc(CVMS_MAX_POINTS*sizeof(float));
    cvms_buf_init = 1;
  }

  ucvm_cvms_id = id;

  /* Save model conf */
  memcpy(&ucvm_cvms_conf, conf, sizeof(ucvm_modelconf_t));

  ucvm_cvms_init_flag = 1;

  return(UCVM_CODE_SUCCESS);
}


/* Finalize CVM-S */
int ucvm_cvms_model_finalize()
{
  if (cvms_buf_init == 1) {
    free(cvms_index);
    free(cvms_lon);
    free(cvms_lat);
    free(cvms_dep);
    free(cvms_vp);
    free(cvms_vs);
    free(cvms_rho);
  }
  ucvm_cvms_init_flag = 0;
  return(UCVM_CODE_SUCCESS);
}


/* Version CVM-S */
int ucvm_cvms_model_version(int id, char *ver, int len)
{
  int errcode;
  /* Fortran fixed string length */
  char verstr[CVMS_FORTRAN_VERSION_LEN];

  if (id != ucvm_cvms_id) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  cvms_version_(verstr, &errcode);
  if (errcode != 0) {
    fprintf(stderr, "Failed to retrieve version from CVM-S\n");
    return(UCVM_CODE_ERROR);
  }

  ucvm_strcpy(ver, verstr, len);
  return(UCVM_CODE_SUCCESS);
}


/* Label CVM-S */
int ucvm_cvms_model_label(int id, char *lab, int len)
{
  if (id != ucvm_cvms_id) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  ucvm_strcpy(lab, ucvm_cvms_conf.label, len);
  return(UCVM_CODE_SUCCESS);
}


/* Setparam CVM-S */
int ucvm_cvms_model_setparam(int id, int param, ...)
{
  va_list ap;

  if (id != ucvm_cvms_id) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  va_start(ap, param);
  switch (param) {
  default:
    break;
  }

  va_end(ap);

  return(UCVM_CODE_SUCCESS);
}


/* Query CVM-S */
int ucvm_cvms_model_query(int id, ucvm_ctype_t cmode,
			  int n, ucvm_point_t *pnt, 
			  ucvm_data_t *data)
{
  int i, j;
  int nn = 0;
  double depth;
  int datagap = 0;
  int errcode;

  if (cvms_buf_init == 0) {
    return(UCVM_CODE_ERROR);
  }

  if (id != ucvm_cvms_id) {
    fprintf(stderr, "Invalid model id\n");
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

  nn = 0;
  for (i = 0; i < n; i++) {
    if ((data[i].crust.source == UCVM_SOURCE_NONE) && 
	((data[i].domain == UCVM_DOMAIN_INTERP) || 
	 (data[i].domain == UCVM_DOMAIN_CRUST)) &&
      	(region_contains_null(&(ucvm_cvms_conf.region), 
			      cmode, &(pnt[i])))) {

      /* Modify pre-computed depth to account for GTL interp range */
      depth = data[i].depth + data[i].shift_cr;

      /* CVM-S extends from free surface on down */
      if (depth >= 0.0) {
	/* Query point */
	cvms_index[nn] = i;
	cvms_lon[nn] = (float)(pnt[i].coord[0]);
	cvms_lat[nn] = (float)(pnt[i].coord[1]);
	cvms_dep[nn] = (float)(depth);
	cvms_vp[nn] = 0.0;
	cvms_vs[nn] = 0.0;
	cvms_rho[nn] = 0.0;
	nn++;
	if (nn == CVMS_MAX_POINTS) {
	  cvms_query_(&nn, cvms_lon, cvms_lat, cvms_dep, 
		      cvms_vp, cvms_vs, cvms_rho, &errcode);
	  
	  if (errcode == 0) {
	    for (j = 0; j < nn; j++) {
	      data[cvms_index[j]].crust.vp = (double)cvms_vp[j];
	      data[cvms_index[j]].crust.vs = (double)cvms_vs[j];
	      data[cvms_index[j]].crust.rho = (double)cvms_rho[j];
	      data[cvms_index[j]].crust.source = ucvm_cvms_id;
	    }
	  }
	  
	  nn = 0;
	}
      } else {
	datagap = 1;
      }
    } else {
      if (data[i].crust.source == UCVM_SOURCE_NONE) {
	datagap = 1;
      }
    }
  }

  if (nn > 0) {
    cvms_query_(&nn, cvms_lon, cvms_lat, cvms_dep, 
		cvms_vp, cvms_vs, cvms_rho, &errcode);
    if (errcode == 0) {
      for (j = 0; j < nn; j++) {
	data[cvms_index[j]].crust.vp = (double)cvms_vp[j];
	data[cvms_index[j]].crust.vs = (double)cvms_vs[j];
	data[cvms_index[j]].crust.rho = (double)cvms_rho[j];
	data[cvms_index[j]].crust.source = ucvm_cvms_id;
      }
    }
  }

  if (datagap) {
    return(UCVM_CODE_DATAGAP);
  }

  return(UCVM_CODE_SUCCESS);
}


/* Fill model structure with CVM-S */
int ucvm_cvms_get_model(ucvm_model_t *m)
{
  m->mtype = UCVM_MODEL_CRUSTAL;
  m->init = ucvm_cvms_model_init;
  m->finalize = ucvm_cvms_model_finalize;
  m->getversion = ucvm_cvms_model_version;
  m->getlabel = ucvm_cvms_model_label;
  m->setparam = ucvm_cvms_model_setparam;
  m->query = ucvm_cvms_model_query;

  return(UCVM_CODE_SUCCESS);
}
