#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "etree.h"
#include "ucvm_config.h"
#include "ucvm_utils.h"
#include "ucvm_meta_etree.h"
#include "ucvm_model_cmuetree.h"
#include "ucvm_proj_bilinear.h"


/* Init flag */
int ucvm_cmuetree_init_flag = 0;


/* Cmuetree buffer size in MB */
#define CMUETREE_BUF_SIZE 64


/* CMU Etree record */
typedef struct ucvm_cmuetree_t {
  int valid;
  ucvm_modelconf_t conf;
  etree_t *ep;
  char epath[UCVM_MAX_PATH_LEN];
  ucvm_point_t corners[4];
  ucvm_meta_cmu_t meta;
  ucvm_bilinear_t proj;
} ucvm_cmuetree_t;


/* Model ID */
int ucvm_cmuetree_id = UCVM_SOURCE_NONE;


/* CMU Etree info */
ucvm_cmuetree_t ucvm_cmuetree;


/* Init Cmuetree */
int ucvm_cmuetree_model_init(int id, ucvm_modelconf_t *conf)
{
  int i;
  char filename[UCVM_MAX_PATH_LEN];
  char *appmeta;

  ucvm_config_t *cfg = NULL;
  ucvm_config_t *cfgentry = NULL;

  /* Startup initialization */
  if (ucvm_cmuetree_init_flag) {
    fprintf(stderr, "Model %s is already initialized\n", conf->label);
    return(UCVM_CODE_ERROR);
  }

  if ((conf->config == NULL) || (strlen(conf->config) == 0)) {
    fprintf(stderr, "No config path defined for model %s\n", conf->label);
    return(UCVM_CODE_ERROR);
  }

  if (!ucvm_is_file(conf->config)) {
    fprintf(stderr, "CMU etree %s is not a valid file\n", conf->config);
    return(UCVM_CODE_ERROR);
  }

  /* Save model conf */
  memcpy(&ucvm_cmuetree.conf, conf, sizeof(ucvm_modelconf_t));

  /* Read conf file */
  snprintf(filename, UCVM_MAX_PATH_LEN, "%s/cmuetree.conf", conf->config);
  cfg = ucvm_parse_config(filename);
  if (cfg == NULL) {
    fprintf(stderr, "Failed to read CMU etree config file\n");
    return(UCVM_CODE_ERROR);
  }

  cfgentry = ucvm_find_name(cfg, "proj_xi");
  if (cfgentry == NULL) {
    fprintf(stderr, "Failed to find proj_xi key\n");
    return(UCVM_CODE_ERROR);
  }
  if (list_parse(cfgentry->value, UCVM_CONFIG_MAX_STR, 
		 ucvm_cmuetree.proj.xi, 4) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to parse %s value\n", cfgentry->name);
    return(UCVM_CODE_ERROR);
  }

  cfgentry = ucvm_find_name(cfg, "proj_yi");
  if (cfgentry == NULL) {
    fprintf(stderr, "Failed to find proj_yi key\n");
    return(UCVM_CODE_ERROR);
  }
  if (list_parse(cfgentry->value, UCVM_CONFIG_MAX_STR, 
		 ucvm_cmuetree.proj.yi, 4) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to parse %s value\n", cfgentry->name);
    return(UCVM_CODE_ERROR);
  }

  /* Take projection size from etree metadata */
  //cfgentry = ucvm_find_name(cfg, "proj_size");
  //if (cfgentry == NULL) {
  //  fprintf(stderr, "Failed to find proj_size key\n");
  //  return(UCVM_CODE_ERROR);
  //}
  //if (list_parse(cfgentry->value, UCVM_CONFIG_MAX_STR, 
  //		 ucvm_cmuetree.proj.dims, 2) != UCVM_CODE_SUCCESS) {
  //  fprintf(stderr, "Failed to parse %s value\n", cfgentry->name);
  //  return(UCVM_CODE_ERROR);
  //}

  cfgentry = ucvm_find_name(cfg, "modelpath");
  if (cfgentry == NULL) {
    fprintf(stderr, "Failed to find modelpath key\n");
    return(UCVM_CODE_ERROR);
  }
  ucvm_strcpy(ucvm_cmuetree.epath, cfgentry->value, UCVM_MAX_PATH_LEN);

  /* Open Etree */
  ucvm_cmuetree.ep = etree_open(ucvm_cmuetree.epath, 
				O_RDONLY, CMUETREE_BUF_SIZE, 0, 3);
  if (ucvm_cmuetree.ep == NULL) {
    fprintf(stderr, "Failed to open the CMU etree %s\n", 
	    ucvm_cmuetree.conf.config);
    return(UCVM_CODE_ERROR);
  }

  /* Read meta data and check it */
  appmeta = etree_getappmeta(ucvm_cmuetree.ep);
  if (appmeta == NULL) {
    fprintf(stderr, "Failed to read metadata from CMU etree %s\n", 
	    ucvm_cmuetree.conf.config);
    return(UCVM_CODE_ERROR);
  }
  if (ucvm_meta_etree_cmu_unpack(appmeta, 
				 &ucvm_cmuetree.meta) != 
      UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to unpack metadata from etree %s\n", 
  	    ucvm_cmuetree.conf.config);
    return(UCVM_CODE_ERROR);
  }
  free(appmeta);

  /* Finish setting up projection */
  for (i = 0; i < 2; i++) {
    ucvm_cmuetree.proj.dims[i] = ucvm_cmuetree.meta.dims_xyz.coord[i];
  }

  ucvm_cmuetree_id = id;

  ucvm_cmuetree_init_flag = 1;
  return(UCVM_CODE_SUCCESS);
}


/* Finalize CMU etree */
int ucvm_cmuetree_model_finalize()
{
  if (ucvm_cmuetree_init_flag == 0) {
    fprintf(stderr, "Model not initialized\n");
    return(UCVM_CODE_ERROR);
  }

  /* Close Etree */
  etree_close(ucvm_cmuetree.ep);

  ucvm_cmuetree_id = UCVM_SOURCE_NONE;
  memset(&ucvm_cmuetree, 0, sizeof(ucvm_cmuetree_t));

  ucvm_cmuetree_init_flag = 0;
  return(UCVM_CODE_SUCCESS);
}


/* Version CMU etree */
int ucvm_cmuetree_model_version(int id, char *ver, int len)
{
  if (id != ucvm_cmuetree_id) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  ucvm_strcpy(ver, ucvm_cmuetree.meta.date, len);
  return(UCVM_CODE_SUCCESS);
}


/* Label Cmuetree */
int ucvm_cmuetree_model_label(int id, char *lab, int len)
{
  if (id != ucvm_cmuetree_id) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  ucvm_strcpy(lab, ucvm_cmuetree.conf.label, len);
  return(UCVM_CODE_SUCCESS);
}


/* Setparam Cmuetree */
int ucvm_cmuetree_model_setparam(int id, int param, ...)
{
  va_list ap;

  if (id != ucvm_cmuetree_id) {
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


/* Query Cmuetree */
int ucvm_cmuetree_model_query(int id, ucvm_ctype_t cmode,
			   int n, ucvm_point_t *pnt, ucvm_data_t *data)
{
  int i;
  double depth;
  ucvm_point_t xy;
  etree_addr_t addr;
  ucvm_epayload_t payload;
  int datagap = 0;

  if (id != ucvm_cmuetree_id) {
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
	(region_contains_null(&(ucvm_cmuetree.conf.region), 
			      cmode, &(pnt[i])))) {

      /* Modify pre-computed depth to account for GTL interp range */
      depth = data[i].depth + data[i].shift_cr;

      /* Etree extends from free surface on down */
      if (depth >= 0.0) {
	/* Convert point to address */
	if (ucvm_bilinear_geo2xy(&(ucvm_cmuetree.proj), 
				 &(pnt[i]),
				 &xy) == UCVM_CODE_SUCCESS) {
	  xy.coord[2] = pnt[i].coord[2];

	  xy.coord[0] = xy.coord[0] / 
	    ucvm_cmuetree.meta.dims_xyz.coord[0] *
	    ucvm_cmuetree.meta.ticks_xyz.dim[0]; 
	  xy.coord[1] = xy.coord[1] / 
	    ucvm_cmuetree.meta.dims_xyz.coord[1] *
	    ucvm_cmuetree.meta.ticks_xyz.dim[1];
	  xy.coord[2] = xy.coord[2] / 
	    ucvm_cmuetree.meta.dims_xyz.coord[2] *
	   ucvm_cmuetree.meta.ticks_xyz.dim[2];
  
	  addr.x = (int)xy.coord[0];
	  addr.y = (int)xy.coord[1];
	  addr.z = (int)xy.coord[2];
	  addr.level = ETREE_MAXLEVEL;

	  /* Query etree */
	  if (etree_search(ucvm_cmuetree.ep, addr, 
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
int ucvm_cmuetree_get_model(ucvm_model_t *m)
{
  m->mtype = UCVM_MODEL_CRUSTAL;
  m->init = ucvm_cmuetree_model_init;
  m->finalize = ucvm_cmuetree_model_finalize;
  m->setparam = ucvm_cmuetree_model_setparam;
  m->getversion = ucvm_cmuetree_model_version;
  m->getlabel = ucvm_cmuetree_model_label;
  m->query = ucvm_cmuetree_model_query;

  return(UCVM_CODE_SUCCESS);
}

