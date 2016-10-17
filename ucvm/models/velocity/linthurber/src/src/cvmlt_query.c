#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "cvmlt.h"


/* Getopt flags */
extern char *optarg;
extern int optind, opterr, optopt;


/* Usage function */
void usage() {
  printf("\n     cvmlt_query - (c) SCEC\n");
  printf("Extract velocities from the Lin-Thurber Statewide CVM. Accepts\n");
  printf("geographic coordinates coordinates in lon,lat,elev_off(msl) columns.\n\n");
  printf("\tusage: cvmlt_query < file.in\n\n");
  printf("Flags:\n");
  printf("\t-h This help message.\n");
  printf("\t-m Path to model files.\n\n");
  printf("Output format is:\n");
  printf("\tlon lat elev_off(m) vp(m/s) vs(m/s) rho\n\n");
  printf("Notes:\n");
  printf("\t- If running interactively, type Cntl-D to end input coord list.\n");
  exit (0);
}


int main(int argc, char **argv)
{
  int opt;

  /* Config variables */
  char modelpath[CVMLT_MAX_STR_LEN];

  /* Model variables */
  cvmlt_point_t pnt;
  cvmlt_data_t data;

  strcpy(modelpath, "../model");

  /* Parse options */
  while ((opt = getopt(argc, argv, "hm:")) != -1) {
    switch (opt) {
    case 'h':
      usage();
      exit(0);
      break;
    case 'm':
      strcpy(modelpath, optarg);
      break;
    default: /* '?' */
      usage();
      exit(1);
    }
  }

  /* Init model */
  if (cvmlt_init(modelpath) != 0) {
    fprintf(stderr, "Failed to initialize model\n");
    return(1);
  }

  /* Read in coords */
  while (!feof(stdin)) {
    memset(&pnt, 0, sizeof(cvmlt_point_t));
    if (fscanf(stdin,"%lf %lf %lf", 
	       &(pnt.coord[0]), &(pnt.coord[1]), &(pnt.coord[2])) == 3) {

      /* Check for scan failure */
      if ((pnt.coord[0] == 0.0) || (pnt.coord[1] == 0.0)) {
	continue;
      }

      /* Query the model */
      cvmlt_query(&pnt, &data);

      /* Display results */
      printf("%12.5lf %12.5lf %12.5lf %10.4lf %10.4lf %10.4lf\n", 
	     pnt.coord[0], pnt.coord[1], pnt.coord[2],
	     data.vp, data.vs, data.rho);
    }
  }

  /* Finalize */
  cvmlt_finalize();

  return(0);
}
