#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ucvm_utils.h"
#include "ucvm_model_cvmsi.h"
#include "cvmsi.h"

/* Init flag */
int ucvm_cvmsi_init_flag = 0;

/* Model ID */
int ucvm_cvmsi_id = UCVM_SOURCE_NONE;

/* Model conf */
ucvm_modelconf_t ucvm_cvmsi_conf;

/* Maximum number of points to query */
#define CVMSI_MAX_POINTS 1000000

/* Query buffers */
cvmsi_point_t *pntBuffer = NULL;
cvmsi_data_t *dataBuffer = NULL;

/* Init CVM-SI */
int ucvm_cvmsi_model_init(int id, ucvm_modelconf_t *conf)
{

  if (ucvm_cvmsi_init_flag) {
    fprintf(stderr, "Model %s is already initialized\n", conf->label);
    return(UCVM_CODE_ERROR);
  }

  if ((conf->config == NULL) || (strlen(conf->config) == 0)) {
    fprintf(stderr, "No config path defined for model %s\n", conf->label);
    return(UCVM_CODE_ERROR);
  }

  /* Init model */
  if (cvmsi_init(conf->config) != 0) {
    fprintf(stderr, "Failed to initialize model\n");
    return(UCVM_CODE_ERROR);
  }

  ucvm_cvmsi_id = id;

  /* Save model conf */
  memcpy(&ucvm_cvmsi_conf, conf, sizeof(ucvm_modelconf_t));

	dataBuffer = malloc(CVMSI_MAX_POINTS * sizeof(cvmsi_data_t));
	pntBuffer = malloc(CVMSI_MAX_POINTS * sizeof(cvmsi_point_t));
	
  ucvm_cvmsi_init_flag = 1;

  return(UCVM_CODE_SUCCESS);
}


/* Finalize CVM-SI */
int ucvm_cvmsi_model_finalize()
{
  if (ucvm_cvmsi_init_flag != 0) {
    /* Finalize model */
    cvmsi_finalize();
  }

  free(dataBuffer);
  free(pntBuffer);

  ucvm_cvmsi_init_flag = 0;
  return(UCVM_CODE_SUCCESS);
}


/* Version CVM-SI */
int ucvm_cvmsi_model_version(int id, char *ver, int len)
{
  if (id != ucvm_cvmsi_id) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  if (cvmsi_version(ver, len) != 0) {
    return(UCVM_CODE_ERROR);
  } 

  return(UCVM_CODE_SUCCESS);
}


/* Label CVM-SI */
int ucvm_cvmsi_model_label(int id, char *lab, int len)
{
  if (id != ucvm_cvmsi_id) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  ucvm_strcpy(lab, ucvm_cvmsi_conf.label, len);
  return(UCVM_CODE_SUCCESS);
}


/* Setparam CVM-SI */
int ucvm_cvmsi_model_setparam(int id, int param, ...)
{
  va_list ap;

  if (id != ucvm_cvmsi_id) {
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


/* Query CVM-SI */
int ucvm_cvmsi_model_query(int id, ucvm_ctype_t cmode,
			  int n, ucvm_point_t *pnt, 
			  ucvm_data_t *data)
{
  int i, j, j2;
  double depth;
  int datagap = 0;
  int nn = 0;
  int maxPtsTimes = 0;

  if (id != ucvm_cvmsi_id) {
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
      	(region_contains_null(&(ucvm_cvmsi_conf.region), 
			      cmode, &(pnt[i])))) {

      /* Modify pre-computed depth to account for GTL interp range */
      depth = data[i].depth + data[i].shift_cr;
        
      /* CVM-SI extends from free surface on down */
      if (depth >= 0.0) {
		  
		  pntBuffer[nn].coord[0] = pnt[i].coord[0];
		  pntBuffer[nn].coord[1] = pnt[i].coord[1];
		  pntBuffer[nn].coord[2] = depth;

          nn++;
          
		  
		  if (nn == CVMSI_MAX_POINTS) {
			  cvmsi_query(pntBuffer, dataBuffer, nn);
			  
			  for (j = 0; j < nn; j++) {
				  data[(maxPtsTimes * CVMSI_MAX_POINTS) + j].crust.source = ucvm_cvmsi_id;
				  data[(maxPtsTimes * CVMSI_MAX_POINTS) + j].crust.vp = dataBuffer[j].prop.vp;
				  data[(maxPtsTimes * CVMSI_MAX_POINTS) + j].crust.vs = dataBuffer[j].prop.vs;
				  data[(maxPtsTimes * CVMSI_MAX_POINTS) + j].crust.rho = dataBuffer[j].prop.rho;
			  }
              
              nn = 0;
              maxPtsTimes++;

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
        cvmsi_query(pntBuffer, dataBuffer, nn);
        
        for (j2 = 0; j2 < nn; j2++) {
            data[(maxPtsTimes * CVMSI_MAX_POINTS) + j2].crust.source = ucvm_cvmsi_id;
            data[(maxPtsTimes * CVMSI_MAX_POINTS) + j2].crust.vp = dataBuffer[j2].prop.vp;
            data[(maxPtsTimes * CVMSI_MAX_POINTS) + j2].crust.vs = dataBuffer[j2].prop.vs;
            data[(maxPtsTimes * CVMSI_MAX_POINTS) + j2].crust.rho = dataBuffer[j2].prop.rho;
        }
    }

  if (datagap) {
    return(UCVM_CODE_DATAGAP);
  }

  return(UCVM_CODE_SUCCESS);
}


/* Fill model structure with CVM-SI */
int ucvm_cvmsi_get_model(ucvm_model_t *m)
{
  m->mtype = UCVM_MODEL_CRUSTAL;
  m->init = ucvm_cvmsi_model_init;
  m->finalize = ucvm_cvmsi_model_finalize;
  m->getversion = ucvm_cvmsi_model_version;
  m->getlabel = ucvm_cvmsi_model_label;
  m->setparam = ucvm_cvmsi_model_setparam;
  m->query = ucvm_cvmsi_model_query;

  return(UCVM_CODE_SUCCESS);
}
