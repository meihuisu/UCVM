#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "cvmsi.h"

/* Getopt flags */
extern char *optarg;
extern int optind, opterr, optopt;

#define MAX_READ_POINTS 1000000

/* Usage function */
void usage() {
  printf("\n     cvmsi_query - (c) SCEC\n");
  printf("Extract velocities from SCEC CVM-SI. Accepts\n");
  printf("geographic coordinates coordinates in lon,lat,dep columns.\n\n");
  printf("\tusage: cvmsi_query < file.in\n\n");
  printf("Flags:\n");
  printf("\t-h This help message.\n");
  printf("\t-m Path to model files.\n\n");
  printf("Output format is:\n");
  printf("\tlon lat dep(m) x y z vp(m/s) vs(m/s) rho\n\n");
  printf("Notes:\n");
  printf("\t- If running interactively, type Cntl-D to end input coord list.\n");
  exit (0);
}


int main(int argc, char **argv)
{
  int opt, i;

  /* Config variables */
  char modelpath[CVMSI_MAX_STR_LEN];

  strcpy(modelpath, "../model/i26");

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
  if (cvmsi_init(modelpath) != 0) {
    fprintf(stderr, "Failed to initialize model\n");
    return(1);
  }
	
	cvmsi_point_t *pnts = malloc(sizeof(cvmsi_point_t) * MAX_READ_POINTS);
	int counter = 0;
	
	/* Read in coords */
	while (!feof(stdin)) {
		if (fscanf(stdin,"%lf %lf %lf", &(pnts[counter].coord[0]), &(pnts[counter].coord[1]), &(pnts[counter].coord[2])) == 3) {
			/* Check for scan failure */
			if ((pnts[counter].coord[0] == 0.0) || (pnts[counter].coord[1] == 0.0)) {
				continue;
			}
			counter++;
		}
	}
	
	cvmsi_data_t *data = malloc(sizeof(cvmsi_data_t) * MAX_READ_POINTS);
	
	/* Query the model */
    cvmsi_query(pnts, data, counter);

	/* Display results */
	for (i = 0; i < counter; i++) {
		printf("%12.5lf %12.5lf %12.5lf %6d %6d %6d %10.4lf %10.4lf %10.4lf %10.4lf %10.4lf %10.4lf\n", 
			   pnts[i].coord[0], pnts[i].coord[1], pnts[i].coord[2], data[i].xyz.coord[0]+1, data[i].xyz.coord[1]+1, 
			   data[i].xyz.coord[2]+1, data[i].prop.vp, data[i].prop.vs, data[i].prop.rho, data[i].prop.diff_vp, 
			   data[i].prop.diff_vs, data[i].prop.diff_rho);
    }

  /* Finalize */
  cvmsi_finalize();

  return(0);
}
