#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ucvm_utils.h"
#include "ucvm_model_cvmh.h"
#include "vx_sub.h"

/* VX no data value */
#define VX_NO_DATA -99999.0

/* Init flag */
int ucvm_cvmh_init_flag = 0;

/* Model ID */
int ucvm_cvmh_id = UCVM_SOURCE_NONE;

/* Model conf */
ucvm_modelconf_t ucvm_cvmh_conf;

/* Model flags */
int ucvm_cvmh_force_depth = 0;


/* Init CVM-H */
int ucvm_cvmh_model_init(int id, ucvm_modelconf_t *conf)
{
  if (ucvm_cvmh_init_flag) {
    fprintf(stderr, "Model %s is already initialized\n", conf->label);
    return(UCVM_CODE_ERROR);
  }

  if ((conf->config == NULL) || (strlen(conf->config) == 0)) {
    fprintf(stderr, "No config path defined for model %s\n", conf->label);
    return(UCVM_CODE_ERROR);
  }

  /* Init vx */
  if (vx_setup(conf->config) != 0) {
    return(UCVM_CODE_ERROR);
  }

  ucvm_cvmh_id = id;

  /* Save model conf */
  memcpy(&ucvm_cvmh_conf, conf, sizeof(ucvm_modelconf_t));

  ucvm_cvmh_init_flag = 1;

  return(UCVM_CODE_SUCCESS);
}


/* Finalize CVM-H */
int ucvm_cvmh_model_finalize()
{
#ifdef _UCVM_MODEL_CVMH_11_2_0
  /* Cleanup not available */
#else
  vx_cleanup();
#endif
  ucvm_cvmh_init_flag = 0;
  return(UCVM_CODE_SUCCESS);
}


/* Version CVM-H */
int ucvm_cvmh_model_version(int id, char *ver, int len)
{
  char vxver[512];

  if (id != ucvm_cvmh_id) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  /* Get version */
  vx_version(vxver);

  ucvm_strcpy(ver, vxver, len);
  return(UCVM_CODE_SUCCESS);
}


/* Label CVM-H */
int ucvm_cvmh_model_label(int id, char *lab, int len)
{
  if (id != ucvm_cvmh_id) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  ucvm_strcpy(lab, ucvm_cvmh_conf.label, len);
  return(UCVM_CODE_SUCCESS);
}


/* Setparam CVM-H */
int ucvm_cvmh_model_setparam(int id, int param, ...)
{
  char *pstr, *pval;
  va_list ap;

  if (id != ucvm_cvmh_id) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  va_start(ap, param);
  switch (param) {
  case UCVM_MODEL_PARAM_FORCE_DEPTH_ABOVE_SURF:
    ucvm_cvmh_force_depth = va_arg(ap, int);
    break;
  case UCVM_PARAM_MODEL_CONF:
    pstr = va_arg(ap, char *);
    pval = va_arg(ap, char *);
    if (strcmp(pstr, "USE_1D_BKG") == 0) {
      if (strcmp(pval, "True") == 0) {
	/* Register SCEC 1D background model */
	vx_register_scec();
      } else {
	/* Disable background model */
	vx_register_bkg(NULL);
      }
    } else if (strcmp(pstr, "USE_GTL") == 0) {

#ifdef _UCVM_MODEL_CVMH_11_2_0
  /* GTL Toggle not available */
      fprintf(stderr, "CVM-H flag %s not supported in 11.2.0\n", 
	      pstr);
      return(UCVM_CODE_ERROR);
#else
      if (strcmp(pval, "True") == 0) {
	vx_setgtl(1);
      } else {
	vx_setgtl(0);
      }
#endif

    }
    break;
  default:
    break;
  }

  va_end(ap);

  return(UCVM_CODE_SUCCESS);
}


/* Query CVM-H */
int ucvm_cvmh_model_query(int id, ucvm_ctype_t cmode,
			  int n, ucvm_point_t *pnt, 
			  ucvm_data_t *data)
{
  int i;
  vx_entry_t entry;
  float vx_surf;
  vx_zmode_t zmode;
  int datagap = 0;

  if (id != ucvm_cvmh_id) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  switch (cmode) {
  case UCVM_COORD_GEO_DEPTH:
    zmode = VX_ZMODE_DEPTH;
    break;
  case UCVM_COORD_GEO_ELEV:
    zmode = VX_ZMODE_ELEV;
    break;
  default:
    fprintf(stderr, "Unsupported coord type\n");
    return(UCVM_CODE_ERROR);
    break;
  }

  /* Set vx query mode */
  vx_setzmode(zmode);

  /* Set query by geo coordinates */
  entry.coor_type = VX_COORD_GEO;

  for (i = 0; i < n; i++) {
    /* 
       Conditions:
       1) Point data has not been filled in by previous model
       2) Point falls in crust or interpolation zone
       3) Point falls within the configured model region
     */
    if ((data[i].crust.source == UCVM_SOURCE_NONE) && 
	((data[i].domain == UCVM_DOMAIN_INTERP) || 
	 (data[i].domain == UCVM_DOMAIN_CRUST)) &&
	(region_contains_null(&(ucvm_cvmh_conf.region), 
			      cmode, &(pnt[i])))) {

      //printf("depth, shift_cr, shift_gtl = %lf, %lf, %lf\n", 
      //	     data[i].depth, data[i].shift_cr, data[i].shift_gtl);

      /* Force depth mode if directed and point is above surface */
      if ((ucvm_cvmh_force_depth) && (cmode == UCVM_COORD_GEO_ELEV) && 
	  (data[i].depth < 0.0)) {
	/* Setup point to query */
	entry.coor[0] = pnt[i].coord[0];
	entry.coor[1] = pnt[i].coord[1];
	vx_getsurface(&(entry.coor[0]), entry.coor_type, &vx_surf);
	if (vx_surf - VX_NO_DATA < 0.01) {
	  /* Fallback to using UCVM topo */
	  entry.coor[2] = data[i].depth;
	} else {
	  entry.coor[2] = vx_surf - data[i].depth;
	}
      } else {
	/* Setup point to query */
	entry.coor[0] = pnt[i].coord[0];
	entry.coor[1] = pnt[i].coord[1];
	switch (cmode) {
	case UCVM_COORD_GEO_DEPTH:
	  entry.coor[2] = pnt[i].coord[2] + data[i].shift_cr;
	  break;
	case UCVM_COORD_GEO_ELEV:
	  entry.coor[2] = pnt[i].coord[2] - data[i].shift_cr;
	  break;
	default:
	  fprintf(stderr, "Unsupported coord type\n");
	  return(UCVM_CODE_ERROR);
	  break;
	}
      }

      /* Query CVM-H for the point */
      vx_getcoord(&entry);

      if (entry.data_src != VX_SRC_NR) {
	if (entry.vp > 0.0) {
	  data[i].crust.vp = entry.vp;
	}
	if (entry.vs > 0.0) {
	  data[i].crust.vs = entry.vs;
	}
	if (entry.rho > 0.0) {
	  data[i].crust.rho = entry.rho;
	}
	data[i].crust.source = ucvm_cvmh_id;
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


/* Fill model structure with CVM-H */
int ucvm_cvmh_get_model(ucvm_model_t *m)
{
  m->mtype = UCVM_MODEL_CRUSTAL;
  m->init = ucvm_cvmh_model_init;
  m->finalize = ucvm_cvmh_model_finalize;
  m->getversion = ucvm_cvmh_model_version;
  m->getlabel = ucvm_cvmh_model_label;
  m->setparam = ucvm_cvmh_model_setparam;
  m->query = ucvm_cvmh_model_query;

  return(UCVM_CODE_SUCCESS);
}
