#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "grd.h"


/* Getopt flags */
extern char *optarg;
extern int optind, opterr, optopt;


/* Usage function */
void usage() {
  printf("Usage: grd_query [-e] [-v] -d datadir [-b bkgdir] < file.in\n\n");
  printf("Flags:\n");
  printf("\t-e Employ GRD bathymetry heuristic.\n");
  printf("\t-v Employ GRD vs30 heuristic.\n");
  printf("\t-d Main ArcGIS Gridfloat data directory. Default is ./ned.\n");
  printf("\t-b Background/Bathymetry data directory. Default is ./bath.\n");
  printf("\t-h This help message.\n\n");
  printf("Input format is:\n");
  printf("\tlon lat\n\n");
  printf("Output format is:\n");
  printf("\tlon lat val valid\n\n");
  printf("Notes:\n");
  printf("\t- If running interactively, type Cntl-D to end input coord list.\n\n");
  printf("Version: %s\n\n", VERSION);

  exit (0);
}


int main(int argc, char **argv)
{
  int opt;
  //char strbuf[512];
  char datadir[512], bkgdir[512];
  grd_heur_t heur = GRD_HEUR_NONE;
  grd_point_t pnt;
  grd_data_t data;

  strcpy(datadir, "./ned");
  strcpy(bkgdir, "");

  /* Parse options */
  while ((opt = getopt(argc, argv, "evd:b:h")) != -1) {
    switch (opt) {
    case 'b':
      strcpy(bkgdir, optarg);
      break;
    case 'd':
      strcpy(datadir, optarg);
      break;
    case 'e':
      heur = GRD_HEUR_DEM;
      break;
    case 'v':
      heur = GRD_HEUR_VS30;
      break;
    case 'h':
      usage();
      exit(0);
      break;
    default: /* '?' */
      usage();
      exit(1);
    }
  }

  /* Init */
  if (grd_init(datadir, bkgdir, heur) != 0) {
    fprintf(stderr, "Failed to init GRD interface\n");
    return(1);
  }

  /* Read in coords */
  while (!feof(stdin)) {
    if (fscanf(stdin,"%lf %lf", &(pnt.coord[0]), &(pnt.coord[1])) == 2) {
      if (grd_query(1, &pnt, &data) != 0) {
	fprintf(stderr, "Failed to query point\n");
	return(1);
      }

      printf("%9.5lf %9.5lf %s %9.5lf %d\n", 
	     pnt.coord[0], pnt.coord[1], data.source, data.data, data.valid);
    } else {
      //fgets(strbuf, 512, stdin);
    }
  }

  /* Finalize */
  grd_finalize();

  return(0);
}
