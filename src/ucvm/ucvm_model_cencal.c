#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ucvm_utils.h"
#include "ucvm_model_cencal.h"
#include "cvmquery.h"
#include "cvmerror.h"

/* CenCal no data value */
#define CENCAL_NO_DATA -99999.0

/* Constants used in surface calculations */
/* Probably version dependent */
#define CENCAL_MODEL_BOTTOM -45000.0
#define CENCAL_HR_OCTANT_HEIGHT 100.0
#define CENCAL_DEM_ACCURACY 1.0

/* Init flag */
int ucvm_cc_init_flag = 0;

/* Query buffers */
void *query = NULL;
void *errHandler = NULL;

/* Default cache size */
#define CC_CACHE_SIZE 64

/* Query by depth squash limit */
#define CC_SQUASH_LIMIT -200000.0

/* CenCal number of values */
#define CC_NUM_VALS 9

/* CenCal return value buffer */
double *cc_pvals = NULL;

/* Version string */
const char *ucvm_cencal_version_id = "";

/* Model ID */
int ucvm_cc_id = UCVM_SOURCE_NONE;

/* Model conf */
ucvm_modelconf_t ucvm_cc_conf;

/* Model flags */
int ucvm_cc_force_depth = 0;
int ucvm_cc_smode = 0;
double ucvm_cc_slimit = 0.0;


/* Init CenCal */
int ucvm_cencal_model_init(int id, ucvm_modelconf_t *conf)
{
  if (ucvm_cc_init_flag) {
    fprintf(stderr, "Model %s is already initialized\n", conf->label);
    return(UCVM_CODE_ERROR);
  }

  if ((conf->config == NULL) || (strlen(conf->config) == 0)) {
    fprintf(stderr, "No config path defined for model %s\n", conf->label);
    return(UCVM_CODE_ERROR);
  }

  /* Create query */
  query = cencalvm_createQuery();
  if (query == NULL) {
    fprintf(stderr, "Could not create query.\n");
    return(UCVM_CODE_ERROR);
  }

  /* Get handle to error handler */
  errHandler = cencalvm_errorHandler(query);
  if (errHandler == NULL) {
    fprintf(stderr, "Could not get handle to error handler.\n");
    return(UCVM_CODE_ERROR);
  }

  /* Set database filename */
  if (cencalvm_filename(query, conf->config) != 0) {
    fprintf(stderr, "%s\n", cencalvm_error_message(errHandler));
    return(UCVM_CODE_ERROR);
  }

  /* Set extended DB here */
  if (strlen(conf->extconfig) > 0) {
    if (cencalvm_filenameExt(query, conf->extconfig) != 0) {
      fprintf(stderr, "%s\n", cencalvm_error_message(errHandler));
      return(UCVM_CODE_ERROR);
    }
  }

  /* Set cache size */
  if (cencalvm_cacheSize(query, CC_CACHE_SIZE) != 0) {
    fprintf(stderr, "%s\n", cencalvm_error_message(errHandler));
    return(UCVM_CODE_ERROR);
  }
  if (cencalvm_cacheSizeExt(query, CC_CACHE_SIZE) != 0) {
    fprintf(stderr, "%s\n", cencalvm_error_message(errHandler));
    return(UCVM_CODE_ERROR);
  }

  /* Open database for querying */
  if (cencalvm_open(query) != 0) {
    fprintf(stderr, "%s\n", cencalvm_error_message(errHandler));
    return(UCVM_CODE_ERROR);
  }

 /* Set query type and resolution */
  cencalvm_queryType(query, 0);

  /* Create array to hold values returned in queries */
  cc_pvals = (double*) malloc(sizeof(double)*CC_NUM_VALS);
  if (cc_pvals == NULL) {
    return(UCVM_CODE_ERROR);
  }

  ucvm_cc_id = id;

  /* Save model conf */
  memcpy(&ucvm_cc_conf, conf, sizeof(ucvm_modelconf_t));

  ucvm_cc_init_flag = 1;
  return(UCVM_CODE_SUCCESS);
}


/* Finalize CenCal */
int ucvm_cencal_model_finalize()
{

  if (query != NULL) {
    /* Close database */
    cencalvm_close(query);

    /* Destroy query handle */
    cencalvm_destroyQuery(query);
  }

  query = NULL;

  /* Free data buffer */
  if (cc_pvals != NULL) {
    free(cc_pvals);
  }
  cc_pvals = NULL;

  ucvm_cc_init_flag = 0;
  return(UCVM_CODE_SUCCESS);
}


/* Version Cencal */
int ucvm_cencal_model_version(int id, char *ver, int len)
{
  if (id != ucvm_cc_id) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  ucvm_strcpy(ver, ucvm_cencal_version_id, len);
  return(UCVM_CODE_SUCCESS);
}


/* Label Cencal */
int ucvm_cencal_model_label(int id, char *lab, int len)
{
  if (id != ucvm_cc_id) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  ucvm_strcpy(lab, ucvm_cc_conf.label, len);
  return(UCVM_CODE_SUCCESS);
}


/* Setparam CenCal */
int ucvm_cencal_model_setparam(int id, int param, ...)
{
  va_list ap;

  if (id != ucvm_cc_id) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  va_start(ap, param);
  switch (param) {
  case UCVM_MODEL_PARAM_FORCE_DEPTH_ABOVE_SURF:
    ucvm_cc_force_depth = va_arg(ap, int);
    break;
  default:
    break;
  }

  va_end(ap);

  return(UCVM_CODE_SUCCESS);
}


/* Get elevation of free surface (ground/water interface) at point */
int ucvm_cencal_getsurface(ucvm_point_t *pnt, double *surf, 
			   double accuracy)
{
  int i;
  double lon, lat, elev, prevelev, startelev, endelev, resid;

  *surf = CENCAL_NO_DATA;

  for (i = 0; i < CC_NUM_VALS; i++) {
    cc_pvals[i] = 0.0;
  }
  
  /* Calculate starting z for surface search */
  if (cencalvm_squash(query, 0, 0.0) != 0) {
    fprintf(stderr, "%s\n", cencalvm_error_message(errHandler));
    return(UCVM_CODE_ERROR);
  }
  
  lon = pnt->coord[0];
  lat = pnt->coord[1];
  elev = CENCAL_MODEL_BOTTOM;
  
  if (cencalvm_query(query, &cc_pvals, CC_NUM_VALS, 
		     lon, lat, elev) != 0) {
    cencalvm_error_resetStatus(errHandler);
  } else {
    /* Set up one octant higher than reported depth wrt free surface */
    elev = elev + cc_pvals[5] + CENCAL_HR_OCTANT_HEIGHT * 2;
    prevelev = elev;

    /* Find surface estimate by stepping down in octant increments */
    while (elev >= CENCAL_MODEL_BOTTOM) {
      if (cencalvm_query(query, &cc_pvals, CC_NUM_VALS, 
			 lon, lat, elev) != 0) {
	cencalvm_error_resetStatus(errHandler);
      } else {
	if ((cc_pvals[0] > 0.0) && (cc_pvals[1] > 0.0) && 
	    (cc_pvals[2] > 0.0)) {
	  /* Refine surface estimate with logarithmic search */
	  startelev = elev;
	  endelev = prevelev;
	  resid = endelev - startelev;
	  while (resid > accuracy) {
	    elev = (startelev + endelev) / 2.0;
	    if (cencalvm_query(query, &cc_pvals, CC_NUM_VALS, 
			       lon, lat, elev) != 0) {
	      cencalvm_error_resetStatus(errHandler);
	      endelev = elev;
	    } else {
	      if ((cc_pvals[0] <= 0.0) || (cc_pvals[1] <= 0.0) || 
		  (cc_pvals[2] <= 0.0)) {
		endelev = elev;
	      } else {
		startelev = elev;
	      }
	    }
	    resid = endelev - startelev;
	  }
	  elev = startelev;
	  break;
	}
      }
      prevelev = elev;
      elev = elev - CENCAL_HR_OCTANT_HEIGHT;
    }
  }

  /* Restore original squash mode */
  if (cencalvm_squash(query, ucvm_cc_smode, ucvm_cc_slimit) != 0) {
    fprintf(stderr, "%s\n", cencalvm_error_message(errHandler));
    return(UCVM_CODE_ERROR);
  }

  if (elev - CENCAL_MODEL_BOTTOM <= 0.01) {
    return(UCVM_CODE_ERROR);
  }

  *surf = elev;
  return(UCVM_CODE_SUCCESS);
}


/* Query CenCal */
int ucvm_cencal_model_query(int id, ucvm_ctype_t cmode,
			    int n, ucvm_point_t *pnt, 
			    ucvm_data_t *data)
{
  int i;
  double lon, lat, elev, surf;
  int datagap = 0;

  if (ucvm_cc_init_flag == 0) {
    fprintf(stderr, "Model is not initialized\n");
    return(UCVM_CODE_ERROR);
  }

  if (id != ucvm_cc_id) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  /* Check query mode */
  switch (cmode) {
  case UCVM_COORD_GEO_DEPTH:
    /* Turn off squashing */
    ucvm_cc_smode = 0;
    ucvm_cc_slimit = -CC_SQUASH_LIMIT;
    break;
  case UCVM_COORD_GEO_ELEV:
    /* Turn off squashing */
    ucvm_cc_smode = 0;
    ucvm_cc_slimit = -CC_SQUASH_LIMIT;
    break;
  default:
    fprintf(stderr, "Unsupported coord type\n");
    return(UCVM_CODE_ERROR);
    break;
  }

  if (cencalvm_squash(query, ucvm_cc_smode, ucvm_cc_slimit) != 0) {
    fprintf(stderr, "%s\n", cencalvm_error_message(errHandler));
    return(UCVM_CODE_ERROR);
  }

  for (i = 0; i < n; i++) {
    if ((data[i].crust.source == UCVM_SOURCE_NONE) && 
	((data[i].domain == UCVM_DOMAIN_INTERP) || 
	 (data[i].domain == UCVM_DOMAIN_CRUST)) &&
	(region_contains_null(&(ucvm_cc_conf.region), cmode, &(pnt[i])))) {

      /* Force depth mode if directed and geo_elev point is 
	 above surface */
      if ((ucvm_cc_force_depth) && (cmode == UCVM_COORD_GEO_ELEV) && 
	  (data[i].depth < 0.0)) {
	/* Setup point to query */
	lon = pnt[i].coord[0];
	lat = pnt[i].coord[1];
	if (ucvm_cencal_getsurface(&(pnt[i]), 
				   &surf, 
				   CENCAL_DEM_ACCURACY) != UCVM_CODE_SUCCESS) {
	  /* Fallback to using UCVM topo */
	  elev = data[i].surf - data[i].depth;
	} else {
	  if (data[i].depth > -CENCAL_DEM_ACCURACY) {
	    /* Add small buffer to overcome surface uncertainty */
	    elev = surf + CENCAL_DEM_ACCURACY;
	  } else {
	    elev = surf - data[i].depth;
	  }
	}
      } else {
	/* Setup point to query */
	lon = pnt[i].coord[0];
	lat = pnt[i].coord[1];
	switch (cmode) {
	case UCVM_COORD_GEO_DEPTH:
	  if (ucvm_cencal_getsurface(&(pnt[i]), 
				     &surf,
				     CENCAL_DEM_ACCURACY) 
	      != UCVM_CODE_SUCCESS) {
	    /* Fallback to using UCVM topo */
	    elev = data[i].surf - pnt[i].coord[2] - data[i].shift_cr;
	  } else {
	    elev = surf - pnt[i].coord[2] - data[i].shift_cr;
	  }
	  break;
	case UCVM_COORD_GEO_ELEV:
	  elev = pnt[i].coord[2] - data[i].shift_cr;
	  break;
	default:
	  fprintf(stderr, "Unsupported coord type\n");
	  return(UCVM_CODE_ERROR);
	  break;
	}
      }

      /* Query CenCal for the point */
      if (cencalvm_query(query, &cc_pvals, CC_NUM_VALS, 
			 lon, lat, elev) != 0) {
	datagap = 1;
	cencalvm_error_resetStatus(errHandler);
      } else {
	/* Check for model error where water found below surface */
	if ((cmode == UCVM_COORD_GEO_DEPTH) && ((cc_pvals[0] <= 0.0) || 
					       (cc_pvals[1] <= 0.0) || 
					       (cc_pvals[2] <= 0.0))) {
	  datagap = 1;
	} else {
	  if (cc_pvals[0] > 0.0) {
	    data[i].crust.vp = cc_pvals[0];
	  }
	  if (cc_pvals[1] > 0.0) {
	    data[i].crust.vs = cc_pvals[1];
	  }
	  if (cc_pvals[2] > 0.0) {
	    data[i].crust.rho = cc_pvals[2];
	  }
	  data[i].crust.source = ucvm_cc_id;
	}
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


/* Fill model structure with CenCal */
int ucvm_cencal_get_model(ucvm_model_t *m)
{
  m->mtype = UCVM_MODEL_CRUSTAL;
  m->init = ucvm_cencal_model_init;
  m->finalize = ucvm_cencal_model_finalize;
  m->getversion = ucvm_cencal_model_version;
  m->getlabel = ucvm_cencal_model_label;
  m->setparam = ucvm_cencal_model_setparam;
  m->query = ucvm_cencal_model_query;

  return(UCVM_CODE_SUCCESS);
}
