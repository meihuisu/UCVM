#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "etree.h"
#include "ucvm_utils.h"
#include "ucvm_meta_etree.h"
#include "ucvm_model_etree.h"
#include "ucvm_proj_ucvm.h"

/* Etree buffer size in MB */
#define UCVM_ETREE_BUF_SIZE 64


/* Etree record */
typedef struct ucvm_etree_t {
  int valid;
  ucvm_modelconf_t conf;
  etree_t *ep;
  ucvm_meta_ucvm_t meta;
  ucvm_proj_t proj;
} ucvm_etree_t;


/* Etree list */
int ucvm_num_etrees = 0;
ucvm_etree_t ucvm_etree_list[UCVM_MAX_MODELS];


/* Init Etree */
int ucvm_etree_model_init(int id, ucvm_modelconf_t *conf)
{
  char *appmeta;

  /* Startup initialization */
  if (ucvm_num_etrees == 0) {
    memset(ucvm_etree_list, 0, sizeof(ucvm_etree_t) * UCVM_MAX_MODELS);
  }

  if ((conf->config == NULL) || (strlen(conf->config) == 0)) {
    fprintf(stderr, "No config path defined for model %s\n", conf->label);
    return(UCVM_CODE_ERROR);
  }

  if ((id < 0) || (id >= UCVM_MAX_MODELS)) {
    fprintf(stderr, "Model ID is outside of max range\n");
    return(UCVM_CODE_ERROR);
  }

  if (!ucvm_is_file(conf->config)) {
    fprintf(stderr, "Etree %s is not a valid file\n", conf->config);
    return(UCVM_CODE_ERROR);
  }

  /* Save model conf */
  memcpy(&ucvm_etree_list[id].conf, conf, sizeof(ucvm_modelconf_t));

  /* Open Etree */
  ucvm_etree_list[id].ep = etree_open(ucvm_etree_list[id].conf.config, 
				      O_RDONLY, UCVM_ETREE_BUF_SIZE, 0, 3);
  if (ucvm_etree_list[id].ep == NULL) {
    fprintf(stderr, "Failed to open the etree %s\n", 
	    ucvm_etree_list[id].conf.config);
    return(UCVM_CODE_ERROR);
  }

  /* Read meta data and check it */
  appmeta = etree_getappmeta(ucvm_etree_list[id].ep);
  if (appmeta == NULL) {
    fprintf(stderr, "Failed to read metadata from etree %s\n", 
	    ucvm_etree_list[id].conf.config);
    return(UCVM_CODE_ERROR);
  }
  if (ucvm_meta_etree_ucvm_unpack(appmeta, 
				  &ucvm_etree_list[id].meta) != 
      UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to unpack metadata from etree %s\n", 
	    ucvm_etree_list[id].conf.config);
    return(UCVM_CODE_ERROR);
  }
  free(appmeta);

  /* Setup projection */
  if (ucvm_proj_ucvm_init(ucvm_etree_list[id].meta.projstr, 
			   &(ucvm_etree_list[id].meta.origin), 
			   ucvm_etree_list[id].meta.rot,
			   &(ucvm_etree_list[id].meta.dims_xyz),
			   &(ucvm_etree_list[id].proj)) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to setup proj %s.\n", 
	    ucvm_etree_list[id].meta.projstr);
    return(UCVM_CODE_ERROR);
  }

  ucvm_etree_list[id].valid = 1;
  ucvm_num_etrees++;
  return(UCVM_CODE_SUCCESS);
}


/* Finalize Etree */
int ucvm_etree_model_finalize()
{
  int i;

  /* Close Etrees and finalize projections */
  for (i = 0; i < UCVM_MAX_MODELS; i++) {
    if (ucvm_etree_list[i].valid) {
      etree_close(ucvm_etree_list[i].ep);
      ucvm_proj_ucvm_finalize(&(ucvm_etree_list[i].proj));
    }
  }

  ucvm_num_etrees = 0;
  memset(ucvm_etree_list, 0, sizeof(ucvm_etree_t) * UCVM_MAX_MODELS);

  return(UCVM_CODE_SUCCESS);
}


/* Version Etree */
int ucvm_etree_model_version(int id, char *ver, int len)
{
  if ((id < 0) || (id >= UCVM_MAX_MODELS) || 
      (ucvm_etree_list[id].valid == 0)) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  ucvm_strcpy(ver, ucvm_etree_list[id].meta.date, len);
  return(UCVM_CODE_SUCCESS);
}


/* Label Etree */
int ucvm_etree_model_label(int id, char *lab, int len)
{
  if ((id < 0) || (id >= UCVM_MAX_MODELS) || 
      (ucvm_etree_list[id].valid == 0)) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  ucvm_strcpy(lab, ucvm_etree_list[id].conf.label, len);
  return(UCVM_CODE_SUCCESS);
}


/* Setparam Etree */
int ucvm_etree_model_setparam(int id, int param, ...)
{
  va_list ap;

  if ((id < 0) || (id >= UCVM_MAX_MODELS) || 
      (ucvm_etree_list[id].valid == 0)) {
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


/* Query Etree */
int ucvm_etree_model_query(int id, ucvm_ctype_t cmode,
			   int n, ucvm_point_t *pnt, ucvm_data_t *data)
{
  int i;
  double depth;
  ucvm_point_t xy;
  etree_addr_t addr;
  ucvm_epayload_t payload;
  int datagap = 0;

  if ((id < 0) || (id >= UCVM_MAX_MODELS) || 
      (ucvm_etree_list[id].valid == 0)) {
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
	(region_contains_null(&(ucvm_etree_list[id].conf.region), 
			      cmode, &(pnt[i])))) {

      /* Modify pre-computed depth to account for GTL interp range */
      depth = data[i].depth + data[i].shift_cr;

      /* Etree extends from free surface on down */
      if (depth >= 0.0) {
	/* Convert point to address */
	if (ucvm_proj_ucvm_geo2xy(&(ucvm_etree_list[id].proj), 
				  &(pnt[i]),
				  &xy) == UCVM_CODE_SUCCESS) {

	  xy.coord[0] = xy.coord[0] / 
	    ucvm_etree_list[id].meta.dims_xyz.coord[0] *
	    ucvm_etree_list[id].meta.ticks_xyz.dim[0]; 
	  xy.coord[1] = xy.coord[1] / 
	    ucvm_etree_list[id].meta.dims_xyz.coord[1] *
	    ucvm_etree_list[id].meta.ticks_xyz.dim[1];
	  switch (cmode) {
	  case UCVM_COORD_GEO_DEPTH:
	    xy.coord[2] = xy.coord[2] / 
	      ucvm_etree_list[id].meta.dims_xyz.coord[2] *
	      ucvm_etree_list[id].meta.ticks_xyz.dim[2];
	    break;
	  case UCVM_COORD_GEO_ELEV:
	    xy.coord[2] = (data[i].surf - xy.coord[2]) / 
	      ucvm_etree_list[id].meta.dims_xyz.coord[2] *
	      ucvm_etree_list[id].meta.ticks_xyz.dim[2];
	    break;
	  }

	  addr.x = (int)xy.coord[0];
	  addr.y = (int)xy.coord[1];
	  addr.z = (int)xy.coord[2];
	  addr.level = ETREE_MAXLEVEL;

	  /* Query etree */
	  if (etree_search(ucvm_etree_list[id].ep, addr, 
	  		   NULL, "*", &payload) == 0) {
	    data[i].crust.source = id;
	    data[i].crust.vp = payload.Vp;
	    data[i].crust.vs = payload.Vs;
	    data[i].crust.rho = payload.density;
	  } else {
	    datagap = 1;
	  }
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


/* Fill model structure with Etree */
int ucvm_etree_get_model(ucvm_model_t *m)
{
  m->mtype = UCVM_MODEL_CRUSTAL;
  m->init = ucvm_etree_model_init;
  m->finalize = ucvm_etree_model_finalize;
  m->setparam = ucvm_etree_model_setparam;
  m->getversion = ucvm_etree_model_version;
  m->getlabel = ucvm_etree_model_label;
  m->query = ucvm_etree_model_query;

  return(UCVM_CODE_SUCCESS);
}

