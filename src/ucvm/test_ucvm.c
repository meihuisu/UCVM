#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ucvm.h"
#include "ucvm_model_1d.h"
#include "ucvm_model_linthurber.h"
#ifdef _UCVM_ENABLE_CENCAL
#include "ucvm_model_cencal.h"
#endif
#ifdef _UCVM_ENABLE_CVMH
#include "ucvm_model_cvmh.h"
#endif
#ifdef _UCVM_ENABLE_CVMS
#include "ucvm_model_cvms.h"
#endif

#define NUM_POINTS 20

int main(int argc, char **argv)
{
  int i;
  int nn = NUM_POINTS;
  ucvm_point_t *pnt1;
  ucvm_prop_t *prop1;
    
  pnt1 = malloc(NUM_POINTS * sizeof(ucvm_point_t));
  prop1 = malloc(NUM_POINTS * sizeof(ucvm_prop_t));
  
  printf("Init\n");
  if (ucvm_init("../conf/kraken/ucvm.conf") != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Init failed\n");
    return(1);
  }
  
  printf("Query Mode\n");
  if (ucvm_query_mode(UCVM_COORD_GEO_DEPTH) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Set query mode failed\n");
    return(1);
  }
  
  printf("Get Models\n");
#ifdef _UCVM_ENABLE_CVMS
  if (ucvm_add_model("cvms") != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Retrieval of CVM-S failed\n");
    return(1);
  }
#endif
#ifdef _UCVM_ENABLE_CVMH
  if (ucvm_add_model("cvmh") != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Retrieval of CVM-H failed\n");
    return(1);
  }
#endif
#ifdef _UCVM_ENABLE_CENCAL
  if (ucvm_add_model("cencal") != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Retrieval of CenCal failed\n");
    return(1);
  }
#endif
  if (ucvm_add_model("linthurber") != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Retrieval of LinThurber failed\n");
    return(1);
  }
  if (ucvm_add_model("1d") != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Retrieval of 1D failed\n");
    return(1);
  }
  
  printf("Create points\n");
  for (i = 0; i < NUM_POINTS; i++) {
    pnt1[i].coord[0] = -118.0;
    pnt1[i].coord[1] = 34.0;
    pnt1[i].coord[2] = 0.0;
    //pnt1[i].coord[0] = -122.0;
    //pnt1[i].coord[1] = 38.0;
    //pnt1[i].coord[2] = 2000.0;
  }

  printf("Query Models\n");
  if (ucvm_query(nn, pnt1, prop1) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Query CVM failed\n");
    return(1);
  }
 
  printf("Results\n");
  for (i = 0; i < NUM_POINTS; i++) {
    printf("%d: model=%d, vp=%lf, vs=%lf, rho=%lf\n", i, 
	   prop1[i].model, prop1[i].vp, prop1[i].vs, prop1[i].rho);
  }

  ucvm_finalize();
  free(pnt1);
  free(prop1);

  return(0);
}
