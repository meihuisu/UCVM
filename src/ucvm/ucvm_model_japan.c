#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ucvm_utils.h"
#include "ucvm_model_japan.h"
#include "japan.h"

/* Init flag */
int ucvm_japan_init_flag = 0;

/* Maximum number of points to query */
#define JAPAN_MAX_POINTS 1000000

/* Buffers initialized flag */
int japan_buf_init = 0;

/* Query buffers */
int *japan_index = NULL;
double *japan_lon = NULL;
double *japan_lat = NULL;
double *japan_dep = NULL;
double *japan_vp = NULL;
double *japan_vs = NULL;
double *japan_rho = NULL;


/* Model ID */
int ucvm_japan_id = UCVM_SOURCE_NONE;

/* Model conf */
ucvm_modelconf_t ucvm_japan_conf;

/* Init Japan */
int ucvm_japan_model_init(int id, ucvm_modelconf_t *conf)
{
  int errcode;
  char modeldir[1024];

  if (ucvm_cvms_init_flag) {
    fprintf(stderr, "Model %s is already initialized\n", conf->label);
    return(UCVM_CODE_ERROR);
  }

  if ((conf->config == NULL) || (strlen(conf->config) == 0)) {
    fprintf(stderr, "No config path defined for model %s\n", conf->label);
    return(UCVM_CODE_ERROR);
  }

  ucvm_strcpy(modeldir, conf->config, 1024);

  if (japan_init(modeldir) != 0) {
    fprintf(stderr, "Failed to init Japan\n");
    return(UCVM_CODE_ERROR);
  }

  /* Allocate buffers */
  if (cvms_buf_init == 0) {
    japan_index = malloc(JAPAN_MAX_POINTS*sizeof(int));
    japan_lon = malloc(JAPAN_MAX_POINTS*sizeof(float));
    japan_lat = malloc(JAPAN_MAX_POINTS*sizeof(float));
    japan_dep = malloc(JAPAN_MAX_POINTS*sizeof(float));
    japan_vp = malloc(JAPAN_MAX_POINTS*sizeof(float));
    japan_vs = malloc(JAPAN_MAX_POINTS*sizeof(float));
    japan_rho = malloc(JAPAN_MAX_POINTS*sizeof(float));
    japan_buf_init = 1;
  }

  ucvm_japan_id = id;

  /* Save model conf */
  memcpy(&ucvm_japan_conf, conf, sizeof(ucvm_modelconf_t));

  ucvm_japan_init_flag = 1;

  return(UCVM_CODE_SUCCESS);
}


/* Finalize Japan */
int ucvm_cvms_model_finalize()
{
  if (japan_buf_init == 1) {
    free(japan_index);
    free(japan_lon);
    free(japan_lat);
    free(japan_dep);
    free(japan_vp);
    free(japan_vs);
    free(japan_rho);
  }
  ucvm_japan_init_flag = 0;
  return(UCVM_CODE_SUCCESS);
}


/* Version Japan */
int ucvm_japan_model_version(int id, char *ver, int len)
{
  int errcode;
  /* Fortran fixed string length */
  char verstr[1024];

  if (id != ucvm_cvms_id) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  verstr = "Japan";

  ucvm_strcpy(ver, verstr, len);
  return(UCVM_CODE_SUCCESS);
}


/* Label Japan */
int ucvm_japan_model_label(int id, char *lab, int len)
{
  if (id != ucvm_japan_id) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  ucvm_strcpy(lab, ucvm_japan_conf.label, len);
  return(UCVM_CODE_SUCCESS);
}


/* Setparam Japan */
int ucvm_japan_model_setparam(int id, int param, ...)
{
  va_list ap;

  if (id != ucvm_japan_id) {
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


/* Query Japan */
int ucvm_japan_model_query(int id, ucvm_ctype_t cmode,
			  int n, ucvm_point_t *pnt,
			  ucvm_data_t *data)
{
  int i, j;
  int nn = 0;
  double depth;
  int datagap = 0;
  int errcode;

  if (japan_buf_init == 0) {
    return(UCVM_CODE_ERROR);
  }

  if (id != ucvm_japan_id) {
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
      	(region_contains_null(&(ucvm_japan_conf.region),
			      cmode, &(pnt[i])))) {

      /* Modify pre-computed depth to account for GTL interp range */
      depth = data[i].depth + data[i].shift_cr;

      	  /* CVM-S extends from free surface on down */
      	  if (depth >= 0.0) {

			  struct japan_input *ji = malloc(sizeof(struct japan_input));

			  ji->longitude = pnt[i].coord[0];
			  ji->latitude = pnt[i].coord[1];
			  ji->depth = depth;

			  struct japan_properties *jo = malloc(sizeof(struct japan_properties));

			  errorcode = japan_query(ji, jo);

			  if (errorcode != 0) datagap = 1;

			  /* Query point */
			  japan_index[nn] = i;
			  japan_lon[nn] = pnt[i].coord[0];
			  japan_lat[nn] = pnt[i].coord[1];
			  japan_dep[nn] = depth;
			  japan_vp[nn] = jo->Vp;
			  japan_vs[nn] = jo->Vs;
			  japan_rho[nn] = jo->Rho;
			  nn++;

			  if (nn == JAPAN_MAX_POINTS) {
				  for (j = 0; j < nn; j++) {
					  data[japan_index[j]].crust.vp = japan_vp[j];
					  data[japan_index[j]].crust.vs = japan_vs[j];
					  data[japan_index[j]].crust.rho = japan_rho[j];
					  data[japan_index[j]].crust.source = ucvm_japan_id;
				  }
			  }

			  nn = 0;
		}
    } else {
    	if (data[i].crust.source == UCVM_SOURCE_NONE) {
    		datagap = 1;
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
  m->init = ucvm_japan_model_init;
  m->finalize = ucvm_japan_model_finalize;
  m->getversion = ucvm_japan_model_version;
  m->getlabel = ucvm_japan_model_label;
  m->setparam = ucvm_japan_model_setparam;
  m->query = ucvm_japan_model_query;

  return(UCVM_CODE_SUCCESS);
}
