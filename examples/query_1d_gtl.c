#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/time.h>
#include "ucvm.h"


int main(int argc, char **argv)
{
  int nn = 1;
  ucvm_point_t pnts;
  ucvm_data_t data;
  char cmb_label[UCVM_MAX_LABEL_LEN];

  printf("Init\n");
  if (ucvm_init("../conf/test/ucvm.conf") != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Init failed\n");
    return(1);
  }
  
  printf("Query Mode\n");
  if (ucvm_setparam(UCVM_PARAM_QUERY_MODE, 
		    UCVM_COORD_GEO_DEPTH) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to set z mode\n");
    return(1);
  }
  
  printf("Add Crustal Model 1D\n");
  if (ucvm_add_model(UCVM_MODEL_1D) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Retrieval of 1D failed\n");
    return(1);
  }

  printf("Add GTL Model Ely\n");
  if (ucvm_add_model(UCVM_MODEL_ELYGTL) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Retrieval of Ely GTL failed\n");
    return(1);
  }

  /* Change GTL interpolation function from default (linear) 
     to Ely interpolation */
  if (ucvm_assoc_ifunc(UCVM_MODEL_ELYGTL, 
		       UCVM_IFUNC_ELY) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, 
	    "Failed to associate interpolation function with Ely GTL\n");
    return(1);
  }

  /* Change interpolation z range from 0,0 to 0,350 */
  if (ucvm_setparam(UCVM_PARAM_IFUNC_ZRANGE, 0.0, 
		    350.0) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to set interpolation range\n");
    return(1);
  }

  printf("Create point\n");
  pnts.coord[0] = -118.0;
  pnts.coord[1] = 34.0;
  pnts.coord[2] = 2000.0;

  printf("Query Model\n");
  if (ucvm_query(nn, &pnts, &data) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Query UCVM failed\n");
    return(1);
  }

  /* Get cmb data label */
  ucvm_ifunc_label(data.cmb.source, 
		   cmb_label, UCVM_MAX_LABEL_LEN);

  printf("Results:\n");
  printf("\tsource=%s, vp=%lf, vs=%lf, rho=%lf\n",
         cmb_label, data.cmb.vp, data.cmb.vs, data.cmb.rho);

  return(0);
}
