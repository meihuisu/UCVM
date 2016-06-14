#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ucvm_utils.h"
#include "ucvm_model_wfcvm.h"
#include "wfcvm.h"

/* Fortran constants */
#define WFCVM_FORTRAN_MODELDIR_LEN 128
#define WFCVM_FORTRAN_VERSION_LEN 64

/* Init flag */
int ucvm_wfcvm_init_flag = 0;

/* Maximum number of points to query */
#define WFCVM_MAX_POINTS 1000000

/* Buffers initialized flag */
int wfcvm_buf_init = 0;

/* Query buffers */
int *wfcvm_index = NULL;
float *wfcvm_lon = NULL;
float *wfcvm_lat = NULL;
float *wfcvm_dep = NULL;
float *wfcvm_vp = NULL;
float *wfcvm_vs = NULL;
float *wfcvm_rho = NULL;

/* Model ID */
int ucvm_wfcvm_id = UCVM_SOURCE_NONE;

/* Model conf */
ucvm_modelconf_t ucvm_wfcvm_conf;


/* Init WFCVM */
int ucvm_wfcvm_model_init(int id, ucvm_modelconf_t *conf)
{
  int errcode;
  /* Fortran fixed string length */
  char modeldir[WFCVM_FORTRAN_MODELDIR_LEN];

  if (ucvm_wfcvm_init_flag) {
    fprintf(stderr, "Model %s is already initialized\n", conf->label);
    return(UCVM_CODE_ERROR);
  }

  if ((conf->config == NULL) || (strlen(conf->config) == 0)) {
    fprintf(stderr, "No config path defined for model %s\n", conf->label);
    return(UCVM_CODE_ERROR);
  }

  if (strlen(conf->config) >= WFCVM_FORTRAN_MODELDIR_LEN) {
    fprintf(stderr, "Config path too long for model %s\n", conf->label);
    return(UCVM_CODE_ERROR);
  }
  ucvm_strcpy(modeldir, conf->config, WFCVM_FORTRAN_MODELDIR_LEN);

  wfcvm_init_(modeldir, &errcode);
  if (errcode != 0) {
    fprintf(stderr, "Failed to init CVM-S\n");
    return(UCVM_CODE_ERROR);
  }

  /* Allocate buffers */
  if (wfcvm_buf_init == 0) {
    wfcvm_index = malloc(WFCVM_MAX_POINTS*sizeof(int));
    wfcvm_lon = malloc(WFCVM_MAX_POINTS*sizeof(float));
    wfcvm_lat = malloc(WFCVM_MAX_POINTS*sizeof(float));
    wfcvm_dep = malloc(WFCVM_MAX_POINTS*sizeof(float));
    wfcvm_vp = malloc(WFCVM_MAX_POINTS*sizeof(float));
    wfcvm_vs = malloc(WFCVM_MAX_POINTS*sizeof(float));
    wfcvm_rho = malloc(WFCVM_MAX_POINTS*sizeof(float));
    wfcvm_buf_init = 1;
  }

  ucvm_wfcvm_id = id;

  /* Save model conf */
  memcpy(&ucvm_wfcvm_conf, conf, sizeof(ucvm_modelconf_t));

  ucvm_wfcvm_init_flag = 1;

  return(UCVM_CODE_SUCCESS);
}


/* Finalize WFCVM */
int ucvm_wfcvm_model_finalize()
{
  if (wfcvm_buf_init == 1) {
    free(wfcvm_index);
    free(wfcvm_lon);
    free(wfcvm_lat);
    free(wfcvm_dep);
    free(wfcvm_vp);
    free(wfcvm_vs);
    free(wfcvm_rho);
  }
  ucvm_wfcvm_init_flag = 0;
  return(UCVM_CODE_SUCCESS);
}


/* Version WFCVM */
int ucvm_wfcvm_model_version(int id, char *ver, int len)
{
  int errcode;
  /* Fortran fixed string length */
  char verstr[WFCVM_FORTRAN_VERSION_LEN];

  if (id != ucvm_wfcvm_id) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  wfcvm_version_(verstr, &errcode);
  if (errcode != 0) {
    fprintf(stderr, "Failed to retrieve version from WFCVM\n");
    return(UCVM_CODE_ERROR);
  }

  ucvm_strcpy(ver, verstr, len);
  return(UCVM_CODE_SUCCESS);
}


/* Label WFCVM */
int ucvm_wfcvm_model_label(int id, char *lab, int len)
{
  if (id != ucvm_wfcvm_id) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  ucvm_strcpy(lab, ucvm_wfcvm_conf.label, len);
  return(UCVM_CODE_SUCCESS);
}


/* Setparam WFCVM */
int ucvm_wfcvm_model_setparam(int id, int param, ...)
{
  va_list ap;

  if (id != ucvm_wfcvm_id) {
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


/* Query WFCVM */
int ucvm_wfcvm_model_query(int id, ucvm_ctype_t cmode,
			  int n, ucvm_point_t *pnt, 
			  ucvm_data_t *data)
{
  int i, j;
  int nn = 0;
  double depth;
  int datagap = 0;
  int errcode = 0;

  if (wfcvm_buf_init == 0) {
    return(UCVM_CODE_ERROR);
  }

  if (id != ucvm_wfcvm_id) {
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
      	(region_contains_null(&(ucvm_wfcvm_conf.region), 
			      cmode, &(pnt[i])))) {

      /* Modify pre-computed depth to account for GTL interp range */
      depth = data[i].depth + data[i].shift_cr;

      /* CVM-S extends from free surface on down */
      if (depth >= 0.0) {
	/* Query point */
	wfcvm_index[nn] = i;
	wfcvm_lon[nn] = (float)(pnt[i].coord[0]);
	wfcvm_lat[nn] = (float)(pnt[i].coord[1]);
	wfcvm_dep[nn] = (float)(depth);
	wfcvm_vp[nn] = 0.0;
	wfcvm_vs[nn] = 0.0;
	wfcvm_rho[nn] = 0.0;
	nn++;
	if (nn == WFCVM_MAX_POINTS) {
	  wfcvm_query_(&nn, wfcvm_lon, wfcvm_lat, wfcvm_dep, 
		       wfcvm_vp, wfcvm_vs, wfcvm_rho, &errcode);
	  
	  if (errcode == 0) {
	  for (j = 0; j < nn; j++) {
	    data[wfcvm_index[j]].crust.vp = (double)wfcvm_vp[j];
	    data[wfcvm_index[j]].crust.vs = (double)wfcvm_vs[j];
	    data[wfcvm_index[j]].crust.rho = (double)wfcvm_rho[j];
	    data[wfcvm_index[j]].crust.source = ucvm_wfcvm_id;
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
    wfcvm_query_(&nn, wfcvm_lon, wfcvm_lat, wfcvm_dep, 
		 wfcvm_vp, wfcvm_vs, wfcvm_rho, &errcode);
    if (errcode == 0) {
      for (j = 0; j < nn; j++) {
	data[wfcvm_index[j]].crust.vp = (double)wfcvm_vp[j];
	data[wfcvm_index[j]].crust.vs = (double)wfcvm_vs[j];
	data[wfcvm_index[j]].crust.rho = (double)wfcvm_rho[j];
	data[wfcvm_index[j]].crust.source = ucvm_wfcvm_id;
      }
    }
  }

  if (datagap) {
    return(UCVM_CODE_DATAGAP);
  }

  return(UCVM_CODE_SUCCESS);
}


/* Fill model structure with CVM-S */
int ucvm_wfcvm_get_model(ucvm_model_t *m)
{
  m->mtype = UCVM_MODEL_CRUSTAL;
  m->init = ucvm_wfcvm_model_init;
  m->finalize = ucvm_wfcvm_model_finalize;
  m->getversion = ucvm_wfcvm_model_version;
  m->getlabel = ucvm_wfcvm_model_label;
  m->setparam = ucvm_wfcvm_model_setparam;
  m->query = ucvm_wfcvm_model_query;

  return(UCVM_CODE_SUCCESS);
}
