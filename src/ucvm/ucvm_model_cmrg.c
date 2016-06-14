#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ucvm_utils.h"
#include "ucvm_config.h"
#include "ucvm_proj_bilinear.h"
#include "ucvm_model_cmrg.h"
#include "cmrg.h"

/* Init flag */
int ucvm_cmrg_init_flag = 0;

/* Model ID */
int ucvm_cmrg_id = UCVM_SOURCE_NONE;

/* Model conf */
ucvm_modelconf_t ucvm_cmrg_conf;


/* Init CMRG */
int ucvm_cmrg_model_init(int id, ucvm_modelconf_t *conf)
{
  if (ucvm_cmrg_init_flag) {
    fprintf(stderr, "Model %s is already initialized\n", conf->label);
    return(UCVM_CODE_ERROR);
  }

  if ((conf->config == NULL) || (strlen(conf->config) == 0)) {
    fprintf(stderr, "No config path defined for model %s\n", conf->label);
    return(UCVM_CODE_ERROR);
  }

  /* Init model */
  if (cmrg_init(conf->config) != 0) {
    fprintf(stderr, "Failed to initialize model\n");
    return(UCVM_CODE_ERROR);
  }

  ucvm_cmrg_id = id;

  /* Save model conf */
  memcpy(&ucvm_cmrg_conf, conf, sizeof(ucvm_modelconf_t));

  ucvm_cmrg_init_flag = 1;
  return(UCVM_CODE_SUCCESS);
}


/* Finalize CMRG */
int ucvm_cmrg_model_finalize()
{
  if (ucvm_cmrg_init_flag != 0) {
    /* Finalize model */
    cmrg_finalize();
  }

  ucvm_cmrg_init_flag = 0;
  return(UCVM_CODE_SUCCESS);
}


/* Version CMRG */
int ucvm_cmrg_model_version(int id, char *ver, int len)
{
  if (id != ucvm_cmrg_id) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  if (cmrg_version(ver, len) != 0) {
    return(UCVM_CODE_ERROR);
  }

  return(UCVM_CODE_SUCCESS);
}


/* Label CMRG */
int ucvm_cmrg_model_label(int id, char *lab, int len)
{
  if (id != ucvm_cmrg_id) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  ucvm_strcpy(lab, ucvm_cmrg_conf.label, len);
  return(UCVM_CODE_SUCCESS);
}


/* Setparam CMRG */
int ucvm_cmrg_model_setparam(int id, int param, ...)
{
  va_list ap;

  if (id != ucvm_cmrg_id) {
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


/* Query CMRG */
int ucvm_cmrg_model_query(int id, ucvm_ctype_t cmode,
			  int n, ucvm_point_t *pnt, 
			  ucvm_data_t *data)
{
  int i;
  double depth;
  int datagap = 0;
  cmrg_point_t mpnt;
  cmrg_data_t mdata;

  if (id != ucvm_cmrg_id) {
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

  for (i = 0; i < n; i++) {
    if ((data[i].crust.source == UCVM_SOURCE_NONE) && 
	((data[i].domain == UCVM_DOMAIN_INTERP) || 
	 (data[i].domain == UCVM_DOMAIN_CRUST)) &&
      	(region_contains_null(&(ucvm_cmrg_conf.region), 
			      cmode, &(pnt[i])))) {

      /* Modify pre-computed depth to account for GTL interp range */
      depth = data[i].depth + data[i].shift_cr;

      /* CMRG extends from free surface on down */
      if (depth >= 0.0) {
	/* Query point */
	mpnt.coord[0] = pnt[i].coord[0];
	mpnt.coord[1] = pnt[i].coord[1];
	mpnt.coord[2] = depth;

	if ((cmrg_query(&mpnt, &mdata) == 0) &&
	    (mdata.vp > 0.0) && 
	    (mdata.vs > 0.0) && 
	    (mdata.rho > 0.0)) {
	  data[i].crust.source = ucvm_cmrg_id;
	  data[i].crust.vp = mdata.vp;
	  data[i].crust.vs = mdata.vs;
	  data[i].crust.rho = mdata.rho;
	} else {
	  datagap = 1;
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

  if (datagap) {
    return(UCVM_CODE_DATAGAP);
  }

  return(UCVM_CODE_SUCCESS);
}


/* Fill model structure with CMRG */
int ucvm_cmrg_get_model(ucvm_model_t *m)
{
  m->mtype = UCVM_MODEL_CRUSTAL;
  m->init = ucvm_cmrg_model_init;
  m->finalize = ucvm_cmrg_model_finalize;
  m->getversion = ucvm_cmrg_model_version;
  m->getlabel = ucvm_cmrg_model_label;
  m->setparam = ucvm_cmrg_model_setparam;
  m->query = ucvm_cmrg_model_query;

  return(UCVM_CODE_SUCCESS);
}


