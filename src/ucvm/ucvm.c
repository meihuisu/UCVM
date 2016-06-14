#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ucvm.h"
#include "ucvm_config.h"
#include "ucvm_utils.h"
#include "ucvm_proj_ucvm.h"
#include "ucvm_map.h"
/* Interpolation functions */
#include "ucvm_interp.h"
/* Crustal models */
#ifdef _UCVM_ENABLE_CVMS
#include "ucvm_model_cvms.h"
#endif
#ifdef _UCVM_ENABLE_CVMH
#include "ucvm_model_cvmh.h"
#endif
#ifdef _UCVM_ENABLE_CENCAL
#include "ucvm_model_cencal.h"
#endif
#ifdef _UCVM_ENABLE_CVMSI
#include "ucvm_model_cvmsi.h"
#endif
#ifdef _UCVM_ENABLE_CVMNCI
#include "ucvm_model_cvmnci.h"
#endif
#ifdef _UCVM_ENABLE_WFCVM
#include "ucvm_model_wfcvm.h"
#endif
#ifdef _UCVM_ENABLE_CVMLT
#include "ucvm_model_cvmlt.h"
#endif
#ifdef _UCVM_ENABLE_CMRG
#include "ucvm_model_cmrg.h"
#endif
#ifdef _UCVM_ENABLE_TAPE
#include "ucvm_model_tape.h"
#endif
#include "ucvm_model_1d.h"
#include "ucvm_model_bbp1d.h"

#include "ucvm_model_plugin.h"

/* GTL models */
#include "ucvm_model_elygtl.h"
#include "ucvm_model_1dgtl.h"
/* Etree model */
#include "ucvm_model_etree.h"
#include "ucvm_model_cmuetree.h"
#include "ucvm_model_patch.h"

/* Constants */
#define UCVM_MODELLIST_DELIM ","
#define UCVM_GTL_DELIM ":"


/* Init flag */
int ucvm_init_flag = 0;


/* Current query mode */
ucvm_ctype_t ucvm_cur_qmode = UCVM_COORD_GEO_DEPTH;

/* Current model mode */
ucvm_opmode_t ucvm_cur_mmode = UCVM_OPMODE_CRUSTAL;


/* Crustal/GTL model lists */
int ucvm_num_models = 0;
ucvm_model_t ucvm_model_list[UCVM_MAX_MODELS];
ucvm_ifunc_t ucvm_ifunc_list[UCVM_MAX_MODELS];


/* UCVM config */
ucvm_config_t *ucvm_cfg = NULL;

/* GTL smoothing parameters */
double ucvm_interp_zmin = UCVM_DEFAULT_INTERP_ZMIN;
double ucvm_interp_zmax = UCVM_DEFAULT_INTERP_ZMAX;


/* Get topo and vs30 values from UCVM models */
int ucvm_get_model_vals(ucvm_point_t *pnt, ucvm_data_t *data)
{

  /* Re-compute point depth based on query mode */
  switch (ucvm_cur_qmode) {
  case UCVM_COORD_GEO_DEPTH:
    data->depth = pnt->coord[2];
    break;
  case UCVM_COORD_GEO_ELEV:
    data->depth = data->surf - pnt->coord[2];
    break;
  default:
    fprintf(stderr, "Unsupported coord type\n");
    return(UCVM_CODE_ERROR);
    break;
  }

  /* Recompute domain and depth shift values */
  switch (ucvm_cur_mmode) {
  case UCVM_OPMODE_GTL:
    if ((data->depth < ucvm_interp_zmax) && 
	(data->depth >= ucvm_interp_zmin)) {
      data->shift_cr = ucvm_interp_zmax - data->depth;
      data->shift_gtl = ucvm_interp_zmin - data->depth;
      data->domain = UCVM_DOMAIN_INTERP;
    } else if ((data->depth >= 0.0) && (data->depth < ucvm_interp_zmin)) {
      data->domain = UCVM_DOMAIN_GTL;
    }
    break;
  default:
    break;
  }

  /* Disallow negative depths in depth mode */
  switch (ucvm_cur_qmode) {
  case UCVM_COORD_GEO_DEPTH:
    if (data->depth < 0.0) {
      data->domain = UCVM_DOMAIN_NONE;
    }
    break;
  default:
    break;
  }

  return(UCVM_CODE_SUCCESS);
}


/* Initializer */
int ucvm_init(const char *config)
{
  ucvm_init_flag = 0;

  /* Config file parameters */
  ucvm_config_t *cfgentry = NULL;
  //ucvm_config_t *cfg = NULL;
  //char modelconf[UCVM_CONFIG_MAX_STR];

  ucvm_num_models = 0;
  memset(ucvm_model_list, 0, sizeof(ucvm_model_t)*UCVM_MAX_MODELS);
  memset(ucvm_ifunc_list, 0, sizeof(ucvm_ifunc_t)*UCVM_MAX_MODELS);

  /* Read in general config file */
  ucvm_cfg = ucvm_parse_config(config);
  if (ucvm_cfg == NULL) {
    fprintf(stderr, "Failed to read UCVM conf file\n");
    return(UCVM_CODE_ERROR);
  }

  /* Uncomment to dump config to screen */
  //ucvm_dump_config(ucvm_cfg);

  /* Check that UCVM interface and map path are defined */
  cfgentry = ucvm_find_name(ucvm_cfg, "ucvm_interface");
  if (cfgentry == NULL) {
    fprintf(stderr, "UCVM map interface not found in %s\n", config);
    return(UCVM_CODE_ERROR);
  }
  if (strcmp(cfgentry->value, UCVM_MAP_ETREE) != 0) {
    fprintf(stderr, "Invalid UCVM map interface %s\n", cfgentry->value);
    return(UCVM_CODE_ERROR);
  }
  if (ucvm_find_name(ucvm_cfg, "ucvm_mappath") == NULL) {
    fprintf(stderr, "UCVM map path not found in %s\n", config);
    return(UCVM_CODE_ERROR);
  }

  /* Read in UCVM conf file */
  /* There is currently no UCVM-specific config that is needed */
  //snprintf(modelconf, UCVM_CONFIG_MAX_STR, "%s/ucvm.conf", 
  //	  ucvm_find_name(ucvm_cfg, "ucvm_modelpath")->value);
  //cfg = ucvm_parse_config(modelconf);
  //if (cfg == NULL) {
  //  fprintf(stderr, "Failed to read UCVM model conf file\n");
  //  return(UCVM_CODE_ERROR);
  //}

  /* Free UCVM model config */
  //ucvm_free_config(cfg);

  /* Initialize default map */
  if (ucvm_map_init(UCVM_MAP_UCVM, 
		    ucvm_find_name(ucvm_cfg, "ucvm_mappath")->value) 
      != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to initialize UCVM map\n");
    return(UCVM_CODE_ERROR);
  }

  ucvm_init_flag = 1;
  return(UCVM_CODE_SUCCESS);
}


/* Finalizer */
int ucvm_finalize()
{
  int i;

  /* Call all model finalizers */
  for (i = 0; i < ucvm_num_models; i++) {
    (ucvm_model_list[i].finalize)();
  }

  /* Free config file parser resources */
  if (ucvm_cfg != NULL) {
    ucvm_free_config(ucvm_cfg);
    ucvm_cfg = NULL;
  }

  /* Finalize map */
  ucvm_map_finalize();

  /* Zero out structures */
  ucvm_num_models = 0;
  memset(ucvm_model_list, 0, sizeof(ucvm_model_t)*UCVM_MAX_MODELS);
  memset(ucvm_ifunc_list, 0, sizeof(ucvm_ifunc_t)*UCVM_MAX_MODELS);

  ucvm_cur_qmode = UCVM_COORD_GEO_DEPTH;
  ucvm_cur_mmode = UCVM_OPMODE_CRUSTAL;

  ucvm_init_flag = 0;
  return(UCVM_CODE_SUCCESS);
}


/* Enable list of models */
int ucvm_add_model_list(const char *list) {
  char modelstr[UCVM_MAX_MODELLIST_LEN];
  char *token, *strptr;
  int i;
  int num_models = 0;
  char models[UCVM_MAX_MODELS][UCVM_MAX_LABEL_LEN];
  char ifuncs[UCVM_MAX_MODELS][UCVM_MAX_LABEL_LEN];

  if (strlen(list) >= UCVM_MAX_MODELLIST_LEN) {
    return(UCVM_CODE_ERROR);
  }

  memset(models, 0, UCVM_MAX_MODELS * UCVM_MAX_LABEL_LEN);
  memset(ifuncs, 0, UCVM_MAX_MODELS * UCVM_MAX_LABEL_LEN);

  /* Parse model list */
  ucvm_strcpy(modelstr, list, UCVM_MAX_MODELLIST_LEN);
  token = strtok(modelstr, UCVM_MODELLIST_DELIM);
  while (token != NULL) {
    if (num_models == UCVM_MAX_MODELS) {
      fprintf(stderr, "Max number of models reached\n");
      return(UCVM_CODE_ERROR);
    }

    /* Parse ifunc */
    strptr = strstr(token, UCVM_GTL_DELIM);
    if (strptr != NULL) {
      strptr[0] = '\0';
      ucvm_strcpy(ifuncs[num_models], &(strptr[1]), UCVM_MAX_LABEL_LEN);
    } else {
      ucvm_strcpy(ifuncs[num_models], "", UCVM_MAX_LABEL_LEN);
    }

    /* Parse model */
    ucvm_strcpy(models[num_models], token, UCVM_MAX_LABEL_LEN);

    num_models++;
    token = strtok(NULL, UCVM_MODELLIST_DELIM);
  }
  
  /* Activate each model and ifunc */
  for (i = 0; i < num_models; i++) {
    /* Add model */
    if (strlen(models[i]) > 0) {
      //fprintf(stderr, "Adding model %s\n", models[i]);
      if (ucvm_add_model(models[i]) != UCVM_CODE_SUCCESS) {
	fprintf(stderr, "Failed to add parsed model %s\n", models[i]);
	return(UCVM_CODE_ERROR);
      }
      
      /* Associate optional interp function */
      if (strlen(ifuncs[i]) > 0) {
	//fprintf(stderr, "Adding ifunc %s\n", ifuncs[i]);
	if (ucvm_assoc_ifunc(models[i], ifuncs[i]) != UCVM_CODE_SUCCESS) {
	  fprintf(stderr, "Failed to add interp func %s for model %s\n", 
		  ifuncs[i], models[i]);
	  return(UCVM_CODE_ERROR);
	}
      }
    }  
  }
  
  return(UCVM_CODE_SUCCESS);
}


/* Enable specific model, by label */
int ucvm_add_model(const char *label) {
  int is_predef = 0, is_plugin = 0;
  ucvm_model_t m;
  ucvm_modelconf_t mconf;
  int retval = UCVM_CODE_ERROR;
  char key[UCVM_CONFIG_MAX_STR];
  ucvm_config_t *cfgentry = NULL;

  if (ucvm_init_flag == 0) {
    fprintf(stderr, "UCVM not initialized\n");
    return(UCVM_CODE_ERROR);
  }

  if (ucvm_num_models >= UCVM_MAX_MODELS) {
    fprintf(stderr, "Maximum number of models reached\n");
    return(UCVM_CODE_ERROR);
  }

  /* Setup model conf */
  memset(&mconf, 0, sizeof(ucvm_modelconf_t));
  ucvm_strcpy(mconf.label, label, UCVM_MAX_LABEL_LEN);

  /* Lookup predefined models */
  /* Crustal models */
  if (strcmp(label, UCVM_MODEL_CVMS) == 0) {
#ifdef _UCVM_ENABLE_CVMS
    retval = ucvm_cvms_get_model(&m);
#endif
    is_predef = 1;
  }

  if (strcmp(label, UCVM_MODEL_CVMH) == 0) {
#ifdef _UCVM_ENABLE_CVMH
    retval = ucvm_cvmh_get_model(&m);
#endif
    is_predef = 1;
  }

  if (strcmp(label, UCVM_MODEL_CENCAL) == 0) {
#ifdef _UCVM_ENABLE_CENCAL
    retval = ucvm_cencal_get_model(&m);
#endif
    is_predef = 1;
  }

  if (strcmp(label, UCVM_MODEL_CVMSI) == 0) {
#ifdef _UCVM_ENABLE_CVMSI
    retval = ucvm_cvmsi_get_model(&m);
#endif
    is_predef = 1;
  }

  if (strcmp(label, UCVM_MODEL_CVMNCI) == 0) {
#ifdef _UCVM_ENABLE_CVMNCI
    retval = ucvm_cvmnci_get_model(&m);
#endif
    is_predef = 1;
  }

  if (strcmp(label, UCVM_MODEL_WFCVM) == 0) {
#ifdef _UCVM_ENABLE_WFCVM
    retval = ucvm_wfcvm_get_model(&m);
#endif
    is_predef = 1;
  }

  if (strcmp(label, UCVM_MODEL_CVMLT) == 0) {
#ifdef _UCVM_ENABLE_CVMLT
    retval = ucvm_cvmlt_get_model(&m);
#endif
    is_predef = 1;
  }

  if (strcmp(label, UCVM_MODEL_CMRG) == 0) {
#ifdef _UCVM_ENABLE_CMRG
    retval = ucvm_cmrg_get_model(&m);
#endif
    is_predef = 1;
  }

  if (strcmp(label, UCVM_MODEL_TAPE) == 0) {
#ifdef _UCVM_ENABLE_TAPE
    retval = ucvm_tape_get_model(&m);
#endif
    is_predef = 1;
  }

  if (strcmp(label, UCVM_MODEL_1D) == 0) {
    retval = ucvm_1d_get_model(&m);
    is_predef = 1;
  }

  if (strcmp(label, UCVM_MODEL_BBP1D) == 0) {
    retval = ucvm_bbp1d_get_model(&m);
    is_predef = 1;
  }

  /* GTL models */
  if (strcmp(label, UCVM_MODEL_ELYGTL) == 0) {
    retval = ucvm_elygtl_get_model(&m);
    is_predef = 1;
  }
  
  if (strcmp(label, UCVM_MODEL_1DGTL) == 0) {
    retval = ucvm_1dgtl_get_model(&m);
    is_predef = 1;
  }

  /* CMU Etree */
  if (strcmp(label, UCVM_MODEL_CMUETREE) == 0) {
    retval = ucvm_cmuetree_get_model(&m);
    is_predef = 1;
  }

  /* Lookup any plugin-based model. */
  if (retval != UCVM_CODE_SUCCESS && is_predef == 0) {
	  snprintf(key, UCVM_CONFIG_MAX_STR, "ucvm_install_path");
	  cfgentry = ucvm_find_name(ucvm_cfg, key);
	  retval = ucvm_plugin_get_model(cfgentry->value, label, &m);

	  //PluginModel *pm = new PluginModel();

	  is_predef = 1;
	  is_plugin = 1;
  }

  /* Lookup user-defined model */
  if ((retval != UCVM_CODE_SUCCESS) && 
      (label != NULL) && 
      (strlen(label) > 0)) {
    /* Lookup interface type */
    snprintf(key, UCVM_CONFIG_MAX_STR, "%s_interface", label);
    cfgentry = ucvm_find_name(ucvm_cfg, key);
    if (cfgentry != NULL) {
      if (strcmp(cfgentry->value, UCVM_MODEL_ETREE) == 0) {
	/* Get the etree model */
	retval = ucvm_etree_get_model(&m);
      } else if (strcmp(cfgentry->value, UCVM_MODEL_PATCH) == 0) {
	/* Get the patch model */
	retval = ucvm_patch_get_model(&m);
      }
    }
  }

  if (retval != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Model %s is not a valid model. ", label);
    if (is_predef) {
      fprintf(stderr, "It was not enabled when UCVM was compiled or ");
      fprintf(stderr, "the config key %s_modelpath is not defined.\n", 
	      label);
    } else {
      fprintf(stderr, "The config keys %s_interface and/or %s_modelpath ",
	      label, label);
      fprintf(stderr, "are not defined.\n");
    }
    return(UCVM_CODE_ERROR);
  }

  /* Lookup model config */
  memset(&mconf, 0, sizeof(ucvm_modelconf_t));
  ucvm_strcpy(mconf.label, label, UCVM_MAX_LABEL_LEN);

  if (is_plugin) {
	  snprintf(key, UCVM_CONFIG_MAX_STR, "ucvm_install_path");
	  cfgentry = ucvm_find_name(ucvm_cfg, key);
	  if (cfgentry != NULL) {
	    ucvm_strcpy(mconf.config, cfgentry->value, UCVM_MAX_PATH_LEN);
	  }
  } else {
	  snprintf(key, UCVM_CONFIG_MAX_STR, "%s_region", label);
	  cfgentry = ucvm_find_name(ucvm_cfg, key);
	  if (cfgentry != NULL) {
	    region_parse(cfgentry->value, &(mconf.region));
	  }
	  snprintf(key, UCVM_CONFIG_MAX_STR, "%s_modelpath", label);
	  cfgentry = ucvm_find_name(ucvm_cfg, key);
	  if (cfgentry != NULL) {
	    ucvm_strcpy(mconf.config, cfgentry->value, UCVM_MAX_PATH_LEN);
	  }
	  snprintf(key, UCVM_CONFIG_MAX_STR, "%s_extmodelpath", label);
	  cfgentry = ucvm_find_name(ucvm_cfg, key);
	  if (cfgentry != NULL) {
	    ucvm_strcpy(mconf.extconfig, cfgentry->value, UCVM_MAX_PATH_LEN);
	  }
  }

  /* Register the model */
  return(ucvm_add_user_model(&m, &mconf));
}


/* Enable specific model, by type and ucvm_model_t */
int ucvm_add_user_model(ucvm_model_t *m, ucvm_modelconf_t *mconf)
{
  int i, mmax;
  ucvm_model_t *mptr;
  ucvm_model_t *mlist;
  char mlabel[UCVM_MAX_LABEL_LEN];
  char key[UCVM_CONFIG_MAX_STR];
  char param[UCVM_CONFIG_MAX_STR];
  char setting[UCVM_CONFIG_MAX_STR];
  char *flag[2];
  ucvm_config_t *cfgentry = NULL;

  if (ucvm_init_flag == 0) {
    fprintf(stderr, "UCVM not initialized\n");
    return(UCVM_CODE_ERROR);
  }

  mmax = ucvm_num_models;
  mlist = ucvm_model_list;

  if (mmax >= UCVM_MAX_MODELS) {
    fprintf(stderr, "Maximum number of models reached\n");
    return(UCVM_CODE_ERROR);
  }

  /* Check if model already enabled */
  for (i = 0; i < mmax; i++) {
    mptr = &(mlist[i]);
    mptr->getlabel(i, mlabel, UCVM_MAX_LABEL_LEN);
    if (strcmp(mlabel, mconf->label) == 0) {
      fprintf(stderr, "Model %s already enabled\n", mconf->label);
      return(UCVM_CODE_ERROR);
    }
  }

  /* Place model on active list */
  mptr = &(mlist[mmax]);
  memcpy(mptr, m, sizeof(ucvm_model_t));

  /* Perform init */
  if ((mptr->init)(mmax, mconf) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to init model %s with config '%s' and extconfig '%s'. ", 
	    mconf->label, mconf->config, mconf->extconfig);
    fprintf(stderr, 
	    "Config keys %s_modelpath and/or %s_extmodelpath are likely undefined.\n", 
	    mconf->label, mconf->label);

    return(UCVM_CODE_ERROR);
  }

  /* Activate model so that other API functions will work */
  ucvm_num_models++;

  /* Pass model-specific flags from config file */
  flag[0] = param;
  flag[1] = setting;
  snprintf(key, UCVM_CONFIG_MAX_STR, "%s_param", mconf->label);
  cfgentry = ucvm_find_name(ucvm_cfg, key);
  while (cfgentry != NULL) {
    list_parse_s(cfgentry->value, UCVM_CONFIG_MAX_STR, 
		 flag, 2, UCVM_CONFIG_MAX_STR);
    cfgentry = ucvm_find_name(cfgentry->next, key);
    if (ucvm_setparam(UCVM_PARAM_MODEL_CONF, mconf->label, 
		      flag[0], flag[1]) != UCVM_CODE_SUCCESS) {
      fprintf(stderr, "Warning: Failed to set conf %s=%s for model %s\n", 
	      flag[0], flag[1], mconf->label);
    }
  }

  /* Associate default interpolation function */
  switch (mptr->mtype) {
  case UCVM_MODEL_CRUSTAL:
    /* Use crustal pass-through interpolation */
    if (ucvm_assoc_ifunc(mconf->label, UCVM_IFUNC_CRUST) != 
	UCVM_CODE_SUCCESS) {
      fprintf(stderr, "Failed to associate crustal interp func for %s\n",
	      mconf->label);
      ucvm_num_models--;
      return(UCVM_CODE_ERROR);
    }
    break;
  case UCVM_MODEL_GTL:
    /* Enable GTL mode */
    ucvm_cur_mmode = UCVM_OPMODE_GTL;
    /* Default to linear interpolation for this GTL */
    if (ucvm_assoc_ifunc(mconf->label, UCVM_IFUNC_LINEAR) != 
	UCVM_CODE_SUCCESS) {
      fprintf(stderr, "Failed to associate linear interp func for %s\n",
	      mconf->label);
      ucvm_num_models--;
      return(UCVM_CODE_ERROR);
    }
    break;
  default:
    break;
  }

  if ((ucvm_cur_mmode == UCVM_OPMODE_GTL) && 
      (ucvm_cur_qmode == UCVM_COORD_GEO_ELEV)) {
    /* Set force depth flag for all active models */
    for (i = 0; i < ucvm_num_models; i++) {
      mptr = &(ucvm_model_list[i]);
      if (mptr->setparam(i, UCVM_MODEL_PARAM_FORCE_DEPTH_ABOVE_SURF, 
			 1) != 0) {
	fprintf(stderr, "Failed to set force depth flag for model %s\n", 
		mconf->label);
	return(UCVM_CODE_ERROR);
      }
    }
  }

  return(UCVM_CODE_SUCCESS);
}


/* Associate specific interp func with GTL model */
int ucvm_assoc_ifunc(const char *mlabel, const char *ilabel)
{
  ucvm_ifunc_t ifunc;

  if (ucvm_init_flag == 0) {
    fprintf(stderr, "UCVM not initialized\n");
    return(UCVM_CODE_ERROR);
  }

  if (strcmp(ilabel, UCVM_IFUNC_LINEAR) == 0) {
    ucvm_strcpy(ifunc.label, UCVM_IFUNC_LINEAR, UCVM_MAX_LABEL_LEN);
    ifunc.interp = ucvm_interp_linear;
  } else if (strcmp(ilabel, UCVM_IFUNC_ELY) == 0) {
    ucvm_strcpy(ifunc.label, UCVM_IFUNC_ELY, UCVM_MAX_LABEL_LEN);
    ifunc.interp = ucvm_interp_ely;
  } else if (strcmp(ilabel, UCVM_IFUNC_CRUST) == 0) {
    ucvm_strcpy(ifunc.label, UCVM_IFUNC_CRUST, UCVM_MAX_LABEL_LEN);
    ifunc.interp = ucvm_interp_crustal;
  } else {
    fprintf(stderr, "Invalid interp func %s\n", ilabel);
    return(UCVM_CODE_ERROR);
  }

  /* Register the model */
  return(ucvm_assoc_user_ifunc(mlabel, &ifunc));
}


/* Associate user-defined interp func with GTL model */
int ucvm_assoc_user_ifunc(const char *mlabel, ucvm_ifunc_t *ifunc)
{
  int i, mmax;
  ucvm_model_t *mptr;
  ucvm_model_t *mlist;
  char label[UCVM_MAX_LABEL_LEN];

  mmax = ucvm_num_models;
  mlist = ucvm_model_list;

  for (i = 0; i < mmax; i++) {
    mptr = &(mlist[i]);
    mptr->getlabel(i, label, UCVM_MAX_LABEL_LEN);
    if ((strcmp(mlabel, label) == 0) && 
	((mptr->mtype == UCVM_MODEL_GTL) || 
	 ((mptr->mtype == UCVM_MODEL_CRUSTAL) && 
	  (strcmp(ifunc->label, UCVM_IFUNC_CRUST) == 0)))) {
      ucvm_strcpy(ucvm_ifunc_list[i].label, ifunc->label, 
		  UCVM_MAX_LABEL_LEN);
      ucvm_ifunc_list[i].interp = ifunc->interp;
      return(UCVM_CODE_SUCCESS);
    }
  }

  fprintf(stderr, "Model %s is not currently active and/or a GTL\n",
	  mlabel);
  return(UCVM_CODE_ERROR);
}


/* Use specific map (elev, vs30), by label */
int ucvm_use_map(const char *label) {
  char key[UCVM_CONFIG_MAX_STR];
  ucvm_config_t *cfgentry = NULL;

  if (ucvm_init_flag == 0) {
    fprintf(stderr, "UCVM not initialized\n");
    return(UCVM_CODE_ERROR);
  }

  /* Lookup map */
  if ((label != NULL) && (strlen(label) > 0)) {
    /* Lookup user map interface type */
    snprintf(key, UCVM_CONFIG_MAX_STR, "%s_interface", label);
    cfgentry = ucvm_find_name(ucvm_cfg, key);
    if (cfgentry != NULL) {
      if (strcmp(cfgentry->value, UCVM_MAP_ETREE) == 0) {
	/* Lookup mappath */
	snprintf(key, UCVM_CONFIG_MAX_STR, "%s_mappath", label);
	cfgentry = ucvm_find_name(ucvm_cfg, key);
	if (cfgentry != NULL) {
	  /* Finalize old map */
	  ucvm_map_finalize();
	  /* Initialize new map */
	  if (ucvm_map_init(label, cfgentry->value) 
	      != UCVM_CODE_SUCCESS) {
	    fprintf(stderr, "Failed to initialize map %s\n", label);
	    return(UCVM_CODE_ERROR);
	  }
	} else {
	  fprintf(stderr, "Map %s is not a valid map. ", label);
	  fprintf(stderr, "Config key %s_mappath not defined.\n", 
		  label);
	  return(UCVM_CODE_ERROR);
	}
      } else {
	fprintf(stderr, "Map %s is not a valid map. ", label);
	fprintf(stderr, "Unsupported map interface %s specified.\n", 
		cfgentry->value);
	return(UCVM_CODE_ERROR);
      }
    } else {
      fprintf(stderr, "Map %s is not a valid map. ", label);
      fprintf(stderr, "Config key %s_interface not defined.\n", 
	      label);
      return(UCVM_CODE_ERROR);
    }
  }

  return(UCVM_CODE_SUCCESS);
}


/* Get label for a model */
int ucvm_model_label(int m, char *label, int len)
{
  if (ucvm_init_flag == 0) {
    fprintf(stderr, "UCVM not initialized\n");
    return(UCVM_CODE_ERROR);
  }

  if (m >= ucvm_num_models) {
    fprintf(stderr, "Invalid model ID %d\n", m);
    return(UCVM_CODE_ERROR);
  }

  switch (m) {
  case UCVM_SOURCE_NONE:
    ucvm_strcpy(label, UCVM_MODEL_NONE, UCVM_MAX_LABEL_LEN);
    break;
  default:
    if (ucvm_model_list[m].getlabel(m, label, len) != UCVM_CODE_ERROR) {
      return(UCVM_CODE_ERROR);
    }
    break;
  }
 
  return(UCVM_CODE_SUCCESS);
}


/* Get label for an interpolation function */
int ucvm_ifunc_label(int f, char *label, int len)
{
  if (ucvm_init_flag == 0) {
    fprintf(stderr, "UCVM not initialized\n");
    return(UCVM_CODE_ERROR);
  }

  if (f >= ucvm_num_models) {
    fprintf(stderr, "Invalid interp func ID %d\n", f);
    return(UCVM_CODE_ERROR);
  }

  switch (f) {
  case UCVM_SOURCE_NONE:
    ucvm_strcpy(label, UCVM_IFUNC_NONE, UCVM_MAX_LABEL_LEN);
    break;
  case UCVM_SOURCE_CRUST:
    ucvm_strcpy(label, UCVM_IFUNC_CRUST, UCVM_MAX_LABEL_LEN);
    break;
  case UCVM_SOURCE_GTL:
    ucvm_strcpy(label, UCVM_IFUNC_GTL, UCVM_MAX_LABEL_LEN);
    break;
  default:
    ucvm_strcpy(label, ucvm_ifunc_list[f].label, UCVM_MAX_LABEL_LEN);
    break;
  }

  return(UCVM_CODE_SUCCESS);
}


/* Get version for a model */
int ucvm_model_version(int m, char *ver, int len)
{
  if (ucvm_init_flag == 0) {
    fprintf(stderr, "UCVM not initialized\n");
    return(UCVM_CODE_ERROR);
  }

  if (m >= ucvm_num_models) {
    fprintf(stderr, "Invalid model ID %d\n", m);
    return(UCVM_CODE_ERROR);
  }

  switch (m) {
  case UCVM_SOURCE_NONE:
    ucvm_strcpy(ver, "unknown", UCVM_MAX_VERSION_LEN);
    break;
  default:
    if (ucvm_model_list[m].getversion(m, ver, len) != UCVM_CODE_SUCCESS) {
      return(UCVM_CODE_ERROR);
    }
    break;
  }

  return(UCVM_CODE_SUCCESS);
}


/* Set parameters */
int ucvm_setparam(ucvm_param_t param, ...)
{
  int i;
  ucvm_model_t *mptr;
  char mlabel[UCVM_MAX_LABEL_LEN];
  va_list ap;
  double dval, dval2;
  char *str, *str2, *str3;
  int new_flag;

  if (ucvm_init_flag == 0) {
    fprintf(stderr, "UCVM not initialized\n");
    return(UCVM_CODE_ERROR);
  }

  va_start(ap, param);
  switch (param) {
  case UCVM_PARAM_QUERY_MODE:
    ucvm_cur_qmode = va_arg(ap, int);
    if (ucvm_cur_mmode == UCVM_OPMODE_GTL) {
      if (ucvm_cur_qmode == UCVM_COORD_GEO_ELEV) {
	/* Set force depth flag for all active models */
	new_flag = 1;
      } else {
	/* Clear force depth flag for all active models */
	new_flag = 0;
      }
      for (i = 0; i < ucvm_num_models; i++) {
	mptr = &(ucvm_model_list[i]);
	mptr->getlabel(i, mlabel, UCVM_MAX_LABEL_LEN);
	if (mptr->setparam(i, UCVM_MODEL_PARAM_FORCE_DEPTH_ABOVE_SURF, 
			   new_flag) != 0) {
	  fprintf(stderr, "Failed to set force depth flag for model %s\n", 
		  mlabel);
	  return(UCVM_CODE_ERROR);
	}
      }
    }
    break;
  case UCVM_PARAM_IFUNC_ZRANGE:
    dval = va_arg(ap, double);
    dval2 = va_arg(ap, double);
    if (dval > dval2) {
      fprintf(stderr, "Interp min depth greater than max depth\n");
      return(UCVM_CODE_ERROR);
    }
    if ((dval < 0.0) || (dval2 < 0.0)) {
      fprintf(stderr, "Interp depth range must be positive\n");
      return(UCVM_CODE_ERROR);
    }
    ucvm_interp_zmin = dval;
    ucvm_interp_zmax = dval2;
    break;
  case UCVM_PARAM_MODEL_CONF:
    str = va_arg(ap, char *);
    str2 = va_arg(ap, char *);
    str3 = va_arg(ap, char *);
    for (i = 0; i < ucvm_num_models; i++) {
      mptr = &(ucvm_model_list[i]);
      mptr->getlabel(i, mlabel, UCVM_MAX_LABEL_LEN);
      if (strcmp(mlabel, str) == 0) {
	if (mptr->setparam(i, UCVM_PARAM_MODEL_CONF, str2, str3) != 0) {
	  fprintf(stderr, "Failed to set param %s for model %s\n", 
		  str2, mlabel);
	  return(UCVM_CODE_ERROR);
	}
      }
    }
    break;
  default:
    break;
  }
  
  va_end(ap);

  return(UCVM_CODE_SUCCESS);
}


/* Query underlying models */
int ucvm_query(int n, ucvm_point_t *pnt, ucvm_data_t *data)
{
  int i;
  ucvm_model_t *mptr;

  if (ucvm_init_flag == 0) {
    fprintf(stderr, "UCVM not initialized\n");
    return(UCVM_CODE_ERROR);
  }

  if (ucvm_num_models == 0) {
    fprintf(stderr, "No models enabled\n");
    return(UCVM_CODE_ERROR);
  }

  /* Initialize properties array */
  for (i = 0; i < n; i++) {
    data[i].surf = 0.0;
    data[i].vs30 = 0.0;
    switch (ucvm_cur_qmode) {
    case UCVM_COORD_GEO_DEPTH:
      data[i].depth = pnt[i].coord[2];
      break;
    case UCVM_COORD_GEO_ELEV:
      data[i].depth = data[i].surf - pnt[i].coord[2];
      break;
    default:
      fprintf(stderr, "Unsupported coord type\n");
      return(UCVM_CODE_ERROR);
      break;
    }
    data[i].domain = UCVM_DOMAIN_CRUST;
    data[i].shift_cr = 0.0;
    data[i].shift_gtl = 0.0;
    data[i].crust.source = UCVM_SOURCE_NONE;
    data[i].crust.vp = 0.0;
    data[i].crust.vs = 0.0;
    data[i].crust.rho = 0.0;
    data[i].gtl.source = UCVM_SOURCE_NONE;
    data[i].gtl.vp = 0.0;
    data[i].gtl.vs = 0.0;
    data[i].gtl.rho = 0.0;
    data[i].cmb.source = UCVM_SOURCE_NONE;
    data[i].cmb.vp = 0.0;
    data[i].cmb.vs = 0.0;
    data[i].cmb.rho = 0.0;
  }

  /* Query map model */
  if (ucvm_map_query(ucvm_cur_qmode, n, pnt, data) != 
      UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to query UCVM map\n");
    return(UCVM_CODE_ERROR);
  }

  /* Compute derived values */
  for (i = 0; i < n; i++) {
    ucvm_get_model_vals(&(pnt[i]), &(data[i]));
  }

  /* Query crustal models */
  for (i = 0; i < ucvm_num_models; i++) {
    mptr = &(ucvm_model_list[i]);
    if (mptr->mtype == UCVM_MODEL_CRUSTAL) {
      if ((mptr->query)(i, ucvm_cur_qmode, n, pnt, data) == 
	  UCVM_CODE_SUCCESS) {
	break;
      }
    }
  }

  /* Query GTLs */
  for (i = 0; i < ucvm_num_models; i++) {
    mptr = &(ucvm_model_list[i]);
    if (mptr->mtype == UCVM_MODEL_GTL) {
      if ((mptr->query)(i, ucvm_cur_qmode, n, pnt, data) == 
	  UCVM_CODE_SUCCESS) {
	break;
      }
    }
  }

  /* Attempt interpolation depending on operating mode */
  switch (ucvm_cur_mmode) {
  case UCVM_OPMODE_CRUSTAL:
    for (i = 0; i < n; i++) {
      if ((data[i].domain == UCVM_DOMAIN_CRUST) &&
	  (data[i].crust.source != UCVM_SOURCE_NONE)) {
	ucvm_ifunc_list[data[i].crust.source].interp(ucvm_interp_zmin, 
						   ucvm_interp_zmax, 
						   ucvm_cur_qmode,
						   &(pnt[i]), 
						   &(data[i]));
      }
    }
    break;
  case UCVM_OPMODE_GTL:
    for (i = 0; i < n; i++) {
      if (data[i].gtl.source != UCVM_SOURCE_NONE) {
	ucvm_ifunc_list[data[i].gtl.source].interp(ucvm_interp_zmin, 
						   ucvm_interp_zmax, 
						   ucvm_cur_qmode,
						   &(pnt[i]), 
						   &(data[i]));
      } else if ((data[i].domain == UCVM_DOMAIN_CRUST) &&
		 (data[i].crust.source != UCVM_SOURCE_NONE)) {
	ucvm_ifunc_list[data[i].crust.source].interp(ucvm_interp_zmin, 
						     ucvm_interp_zmax, 
						     ucvm_cur_qmode,
						     &(pnt[i]), 
						     &(data[i]));
      }
    } 
    break;
  default:
    break;
  }
  
  return(UCVM_CODE_SUCCESS);
}


/* Save resource information in structure */
int ucvm_save_resource(ucvm_rtype_t rtype, ucvm_mtype_t mtype,
		       const char *label, const char *version,
		       ucvm_resource_t *res, int num, int maxlen)
{

  if (num >= maxlen) {
    return(UCVM_CODE_ERROR);
  }

  res[num].rtype = rtype;
  res[num].mtype = mtype;
  ucvm_strcpy(res[num].label, label, UCVM_MAX_LABEL_LEN);
  ucvm_strcpy(res[num].version, version, UCVM_MAX_VERSION_LEN);
  return(UCVM_CODE_SUCCESS);
}


/* Get installed feature information */
int ucvm_get_resources(ucvm_resource_t *res, int *len)
{
  int i, j, numinst, startj;
  ucvm_model_t *mptr;
  char tmplabel[UCVM_MAX_LABEL_LEN];
  char ver[UCVM_MAX_VERSION_LEN];
  char key[UCVM_CONFIG_MAX_STR];
  ucvm_config_t *cfgentry = NULL;

  char param[UCVM_CONFIG_MAX_STR];
  char setting[UCVM_CONFIG_MAX_STR];
  char *flag[2];

  if ((*len <= 0) || (res == NULL)) {
    return(UCVM_CODE_ERROR);
  }

  flag[0] = param;
  flag[1] = setting;

  /* Get installed models */
  memset(res, 0, sizeof(ucvm_resource_t) * *len);
  numinst = 0;

  /* 1D */
  if (ucvm_save_resource(UCVM_RESOURCE_MODEL, UCVM_MODEL_CRUSTAL,
			 UCVM_MODEL_1D, "", res, numinst++, *len) 
      != UCVM_CODE_SUCCESS) {
    return(UCVM_CODE_ERROR);
  }

  /* CMU CVM-Etree */
  if (ucvm_save_resource(UCVM_RESOURCE_MODEL, UCVM_MODEL_CRUSTAL,
		     UCVM_MODEL_CMUETREE, "", res, numinst++, *len) 
      != UCVM_CODE_SUCCESS) {
    return(UCVM_CODE_ERROR);
  }

  /* 1D GTL */
  if (ucvm_save_resource(UCVM_RESOURCE_MODEL, UCVM_MODEL_GTL,
		     UCVM_MODEL_1DGTL, "", res, numinst++, *len) 
      != UCVM_CODE_SUCCESS) {
    return(UCVM_CODE_ERROR);
  }

  /* Ely GTL */
  if (ucvm_save_resource(UCVM_RESOURCE_MODEL, UCVM_MODEL_GTL,
		     UCVM_MODEL_ELYGTL, "", res, numinst++, *len) 
      != UCVM_CODE_SUCCESS) {
    return(UCVM_CODE_ERROR);
  }


#ifdef _UCVM_ENABLE_CVMS
  if (ucvm_save_resource(UCVM_RESOURCE_MODEL, UCVM_MODEL_CRUSTAL,
		     UCVM_MODEL_CVMS, "", res, numinst++, *len) 
      != UCVM_CODE_SUCCESS) {
    return(UCVM_CODE_ERROR);
  }
#endif

#ifdef _UCVM_ENABLE_CVMH
  if (ucvm_save_resource(UCVM_RESOURCE_MODEL, UCVM_MODEL_CRUSTAL,
		     UCVM_MODEL_CVMH, "", res, numinst++, *len) 
      != UCVM_CODE_SUCCESS) {
    return(UCVM_CODE_ERROR);
  }
#endif

#ifdef _UCVM_ENABLE_CENCAL
  if (ucvm_save_resource(UCVM_RESOURCE_MODEL, UCVM_MODEL_CRUSTAL,
		     UCVM_MODEL_CENCAL, "", res, numinst++, *len) 
      != UCVM_CODE_SUCCESS) {
    return(UCVM_CODE_ERROR);
  }
#endif

#ifdef _UCVM_ENABLE_CVMSI
  if (ucvm_save_resource(UCVM_RESOURCE_MODEL, UCVM_MODEL_CRUSTAL,
		     UCVM_MODEL_CVMSI, "", res, numinst++, *len) 
      != UCVM_CODE_SUCCESS) {
    return(UCVM_CODE_ERROR);
  }
#endif

#ifdef _UCVM_ENABLE_CVMNCI
  if (ucvm_save_resource(UCVM_RESOURCE_MODEL, UCVM_MODEL_CRUSTAL,
		     UCVM_MODEL_CVMNCI, "", res, numinst++, *len) 
      != UCVM_CODE_SUCCESS) {
    return(UCVM_CODE_ERROR);
  }
#endif

#ifdef _UCVM_ENABLE_WFCVM
  if (ucvm_save_resource(UCVM_RESOURCE_MODEL, UCVM_MODEL_CRUSTAL,
		     UCVM_MODEL_WFCVM, "", res, numinst++, *len) 
      != UCVM_CODE_SUCCESS) {
    return(UCVM_CODE_ERROR);
  }
#endif

#ifdef _UCVM_ENABLE_CVMLT
  if (ucvm_save_resource(UCVM_RESOURCE_MODEL, UCVM_MODEL_CRUSTAL,
		     UCVM_MODEL_CVMLT, "", res, numinst++, *len) 
      != UCVM_CODE_SUCCESS) {
    return(UCVM_CODE_ERROR);
  }
#endif

#ifdef _UCVM_ENABLE_CMRG
  if (ucvm_save_resource(UCVM_RESOURCE_MODEL, UCVM_MODEL_CRUSTAL,
		     UCVM_MODEL_CMRG, "", res, numinst++, *len) 
      != UCVM_CODE_SUCCESS) {
    return(UCVM_CODE_ERROR);
  }
#endif

#ifdef _UCVM_ENABLE_TAPE
  if (ucvm_save_resource(UCVM_RESOURCE_MODEL, UCVM_MODEL_CRUSTAL,
		     UCVM_MODEL_TAPE, "", res, numinst++, *len) 
      != UCVM_CODE_SUCCESS) {
    return(UCVM_CODE_ERROR);
  }
#endif

  if (ucvm_init_flag) {
    for (i = 0; i < ucvm_num_models; i++) {
      mptr = &(ucvm_model_list[i]);
      mptr->getlabel(i, tmplabel, UCVM_MAX_LABEL_LEN);
      if (ucvm_model_version(i, ver, UCVM_MAX_VERSION_LEN) 
	  != UCVM_CODE_SUCCESS) {
	return(UCVM_CODE_ERROR);
      }
      /* Populate version info for active models */
      for (j = 0; j < numinst; j++) {
	if (strcmp(res[j].label, tmplabel) == 0) {
	  ucvm_strcpy(res[j].version, ver, UCVM_MAX_VERSION_LEN);
	  res[j].active = 1;
	}
      }
    }

    /* Populate modelpath, extmodelpath, flags  */
    for (j = 0; j < numinst; j++) {
      snprintf(key, UCVM_CONFIG_MAX_STR, "%s_modelpath", res[j].label);
      cfgentry = ucvm_find_name(ucvm_cfg, key);
      if (cfgentry != NULL) {
	ucvm_strcpy(res[j].config, cfgentry->value, UCVM_MAX_PATH_LEN);
      }
      snprintf(key, UCVM_CONFIG_MAX_STR, "%s_extmodelpath", res[j].label);
      cfgentry = ucvm_find_name(ucvm_cfg, key);
      if (cfgentry != NULL) {
	ucvm_strcpy(res[j].extconfig, cfgentry->value, UCVM_MAX_PATH_LEN);
      }
      snprintf(key, UCVM_CONFIG_MAX_STR, "%s_param", res[j].label);
      cfgentry = ucvm_find_name(ucvm_cfg, key);
      res[j].numflags = 0;
      while ((cfgentry != NULL) && (res[j].numflags < UCVM_MAX_FLAGS)) {
	list_parse_s(cfgentry->value, UCVM_CONFIG_MAX_STR, 
		     flag, 2, UCVM_CONFIG_MAX_STR);
	cfgentry = ucvm_find_name(cfgentry->next, key);
	ucvm_strcpy(res[j].flags[res[j].numflags].key, flag[0], 
		    UCVM_MAX_FLAG_LEN);
	ucvm_strcpy(res[j].flags[(res[j].numflags)++].value, flag[1], 
		    UCVM_MAX_FLAG_LEN);
      }
    }
  }

  /* Get installed ifuncs */
  if (ucvm_save_resource(UCVM_RESOURCE_IFUNC, UCVM_MODEL_CRUSTAL,
		     UCVM_IFUNC_LINEAR, "", res, numinst++, *len) 
      != UCVM_CODE_SUCCESS) {
    return(UCVM_CODE_ERROR);
  }
  if (ucvm_save_resource(UCVM_RESOURCE_IFUNC, UCVM_MODEL_CRUSTAL,
		     UCVM_IFUNC_ELY, "", res, numinst++, *len) 
      != UCVM_CODE_SUCCESS) {
    return(UCVM_CODE_ERROR);
  }

  /* Get installed maps */
  startj = numinst;
  if (ucvm_save_resource(UCVM_RESOURCE_MAP, UCVM_MODEL_CRUSTAL,
		     UCVM_MAP_UCVM, "", res, numinst++, *len) 
      != UCVM_CODE_SUCCESS) {
    return(UCVM_CODE_ERROR);
  }
  if (ucvm_save_resource(UCVM_RESOURCE_MAP, UCVM_MODEL_CRUSTAL,
		     UCVM_MAP_YONG, "", res, numinst++, *len) 
      != UCVM_CODE_SUCCESS) {
    return(UCVM_CODE_ERROR);
  }

  if (ucvm_init_flag) {
    /* Populate version info for active map */
    ucvm_map_label(tmplabel, UCVM_MAX_LABEL_LEN);
    ucvm_map_version(ver, UCVM_MAX_VERSION_LEN);
    for (j = startj; j < numinst; j++) {
      if (strcmp(res[j].label, tmplabel) == 0) {
	ucvm_strcpy(res[j].version, ver, UCVM_MAX_VERSION_LEN);
	res[j].active = 1;
      }
    }

    /* Populate mappath */
    for (j = startj; j < numinst; j++) {
      snprintf(key, UCVM_CONFIG_MAX_STR, "%s_mappath", res[j].label);
      cfgentry = ucvm_find_name(ucvm_cfg, key);
      if (cfgentry != NULL) {
	ucvm_strcpy(res[j].config, cfgentry->value, UCVM_MAX_PATH_LEN);
      }
    }
  }

  /* Get installed model interfaces */
  if (ucvm_save_resource(UCVM_RESOURCE_MODEL_IF, UCVM_MODEL_CRUSTAL,
		     UCVM_MODEL_ETREE, "", res, numinst++, *len) 
      != UCVM_CODE_SUCCESS) {
    return(UCVM_CODE_ERROR);
  }
  if (ucvm_save_resource(UCVM_RESOURCE_MODEL_IF, UCVM_MODEL_CRUSTAL,
		     UCVM_MODEL_PATCH, "", res, numinst++, *len) 
      != UCVM_CODE_SUCCESS) {
    return(UCVM_CODE_ERROR);
  }

  /* Get installed map interfaces */
  if (ucvm_save_resource(UCVM_RESOURCE_MAP_IF, UCVM_MODEL_CRUSTAL,
		     UCVM_MAP_ETREE, "", res, numinst++, *len) 
      != UCVM_CODE_SUCCESS) {
    return(UCVM_CODE_ERROR);
  }

  *len = numinst;  
  return(UCVM_CODE_SUCCESS);
}
