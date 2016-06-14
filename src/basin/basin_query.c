/**
 * ucvm2mesh.c - Query UCVM for basin depths
 *
 * Created by Patrick Small <patrices@usc.edu>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "ucvm.h"
#include "ucvm_utils.h"

/* Constants */
#define MAX_RES_LEN 256
#define NUM_POINTS 20000
#define DEFAULT_ZRANGE_MIN 0.0
#define DEFAULT_ZRANGE_MAX 350.0
#define DEFAULT_MAX_DEPTH 15000.0
#define DEFAULT_Z_INTERVAL 20.0
#define DEFAULT_VS_THRESH 1000.0
#define OUTPUT_FMT "%10.4lf %10.4lf %10.3lf %10.3lf\n"


/* Get opt args */
extern char *optarg;
extern int optind, opterr, optopt;


/* Display usage information */
void usage()
{
  printf("Usage: basin_query [-h] [-m models<:ifunc>] [-p user_map] [-f config] [-z zmin,zmax] [-d max_depth] [-i inter] [-v vs_thresh] < file.in\n\n");
  printf("where:\n");
  printf("\t-h This help message\n");
  printf("\t-d Maximum depth (m, default is 15000.0)\n");
  printf("\t-f Configuration file. Default is ./ucvm.conf.\n");
  printf("\t-i Interval between query points along z-axis (m, default is 20.0)\n");
  printf("\t-m Comma delimited list of crustal/GTL models to query in order\n");
  printf("\t-p User-defined map to use for elevation and vs30 data.\n");
  printf("\t-v Vs threshold (m/s, default is 1000.0).\n");
  printf("\t-z Optional depth range for gtl/crust interpolation.\n\n");
  printf("Input format is:\n");
  printf("\tlon lat\n\n");
  printf("Output format is:\n");
  printf("\tlon lat min_depth max_depth\n\n");
  printf("Notes:\n");
  printf("\t- If running interactively, type Cntl-D to end input coord list.\n\n");
  printf("Version: %s\n\n", VERSION);
  return;
}


/* Extract basin values for the specified points */
int extract_basins(int n, ucvm_point_t *pnts, \
		   ucvm_point_t *qpnts, ucvm_data_t *qprops,
		   double max_depth, double z_inter, double vs_thresh)
{
  int i, p, dnum, numz;
  double vs_prev;
  double depths[2];
  
  numz = (int)(max_depth / z_inter);
  for (p = 0; p < n; p++) {
    /* Setup query points */
    for (i = 0; i < numz; i++) {
      qpnts[i].coord[0] = pnts[p].coord[0];
      qpnts[i].coord[1] = pnts[p].coord[1];
      qpnts[i].coord[2] = (double)i * z_inter;
    }

    /* Query the UCVM */
    if (ucvm_query(numz, qpnts, qprops) != UCVM_CODE_SUCCESS) {
      fprintf(stderr, "Query CVM failed\n");
      return(1);
    }

    /* Check for threshold crossing */
    vs_prev = 0.0;
    dnum = 0;
    depths[0] = 0.0;
    depths[1] = 0.0;
    for (i = 0; i < numz; i++) {
      /* Compare the Vs if it is valid */
      if (qprops[i].cmb.vs > 0.0) {
	if ((vs_prev < vs_thresh) && (qprops[i].cmb.vs >= vs_thresh)) {
	  depths[dnum] = (double)i * z_inter;
	  if (dnum == 0) {
	    depths[1] = depths[0];
	    dnum++;
	  }
	}

	/* Save current vs value */
	vs_prev = qprops[i].cmb.vs;
      }
    }

    /* Display output */
    printf(OUTPUT_FMT, pnts[p].coord[0], pnts[p].coord[1], 
	   depths[0], depths[1]);

  }
  
  return(0);
}


int main(int argc, char **argv)
{
  int opt;
  char modellist[UCVM_MAX_MODELLIST_LEN];
  char configfile[UCVM_MAX_PATH_LEN];
  double zrange[2];
  double max_depth;
  double z_inter;
  double vs_thresh;

  ucvm_ctype_t cmode;
  int have_model = 0;
  int have_zrange = 0;
  int have_map = 0;

  ucvm_point_t *pnts;
  ucvm_point_t *qpnts;
  ucvm_data_t *qprops;
  int numread = 0;
  char map_label[UCVM_MAX_LABEL_LEN];

  cmode = UCVM_COORD_GEO_DEPTH;
  snprintf(configfile, UCVM_MAX_PATH_LEN, "%s", "./ucvm.conf");
  snprintf(modellist, UCVM_MAX_MODELLIST_LEN, "%s", "1d");
  snprintf(map_label, UCVM_MAX_LABEL_LEN, "%s", UCVM_MAP_UCVM);
  zrange[0] = DEFAULT_ZRANGE_MIN;
  zrange[1] = DEFAULT_ZRANGE_MAX;
  max_depth = DEFAULT_MAX_DEPTH;
  z_inter = DEFAULT_Z_INTERVAL;
  vs_thresh = DEFAULT_VS_THRESH;

  /* Parse options */
  while ((opt = getopt(argc, argv, "d:i:v:f:hm:p:z:")) != -1) {
    switch (opt) {
    case 'd':
      max_depth = (double)atof(optarg);
      if (max_depth <= 0.0) {
	fprintf(stderr, "Invalid max depth %s.\n", optarg);
	usage();
	exit(1);
      }
      break;
    case 'i':
      z_inter = (double)atof(optarg);
      if (z_inter <= 0.0) {
	fprintf(stderr, "Invalid z interval %s.\n", optarg);
	usage();
	exit(1);
      }
      break;
    case 'v':
      vs_thresh = (double)atof(optarg);
      if (vs_thresh <= 0.0) {
	fprintf(stderr, "Invalid vs threshold %s.\n", optarg);
	usage();
	exit(1);
      }
      break;
    case 'f':
      if (strlen(optarg) < UCVM_MAX_PATH_LEN) {
	snprintf(configfile, UCVM_MAX_PATH_LEN, "%s", optarg);
      } else {
	fprintf(stderr, "Invalid config file specified: %s.\n", optarg);
	usage();
	exit(1);
      }
      break;
    case 'h':
      usage();
      exit(0);
      break;
    case 'm':
      if (strlen(optarg) >= UCVM_MAX_MODELLIST_LEN - 1) {
	fprintf(stderr, "Model list is too long.\n");
	usage();
	exit(1);
      } else {
	snprintf(modellist, UCVM_MAX_MODELLIST_LEN, "%s", optarg);
	have_model = 1;
      }
      break;
    case 'p':
      if (strlen(optarg) >= UCVM_MAX_LABEL_LEN) {
	fprintf(stderr, "Map name is too long.\n");
	usage();
	exit(1);
      } else {
	snprintf(map_label, UCVM_MAX_LABEL_LEN, "%s", optarg);
	have_map = 1;
      }
      break;
    case 'z':
      if (list_parse(optarg, UCVM_MAX_PATH_LEN, 
		     zrange, 2) != UCVM_CODE_SUCCESS) {
	fprintf(stderr, "Invalid zrange specified: %s.\n", optarg);
	usage();
	exit(1);
      }
      have_zrange = 1;
      break;
    default: /* '?' */
      usage();
      exit(1);
    }
  }

  /* Initialize interface */
  if (ucvm_init(configfile) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to initialize UCVM API\n");
    return(1);
  }

  /* Add models */
  if (have_model == 0) {
    /* Add default model if none specified */
    fprintf(stderr, "Using 1D as default model.\n");
  }
  if (ucvm_add_model_list(modellist) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to enable model list %s\n", modellist);
    return(1);
  }

  /* Set user map if necessary */
  if (have_map == 1) {
    if (ucvm_use_map(map_label) != UCVM_CODE_SUCCESS) {
      fprintf(stderr, "Failed to set user map %s\n", map_label);
      return(1);
    }
  }

  /* Set z mode for depth*/
  if (ucvm_setparam(UCVM_PARAM_QUERY_MODE, cmode) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to set z mode\n");
    return(1);
  }

  /* Set interpolation z range */
  if (have_zrange) {
    if (ucvm_setparam(UCVM_PARAM_IFUNC_ZRANGE, 
		      zrange[0], zrange[1]) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to set interpolation z range\n");
    return(1);
    }
  }

  /* Allocate buffers */
  pnts = malloc(NUM_POINTS * sizeof(ucvm_point_t));
  qpnts = malloc((int)(max_depth/z_inter) * sizeof(ucvm_data_t));
  qprops = malloc((int)(max_depth/z_inter) * sizeof(ucvm_data_t));

  /* Read in coords */
  while (!feof(stdin)) {
    memset(&(pnts[numread]), 0, sizeof(ucvm_point_t));
    if (fscanf(stdin,"%lf %lf",
               &(pnts[numread].coord[0]),
	       &(pnts[numread].coord[1])) == 2) {

      /* Check for scan failure */
      if ((pnts[numread].coord[0] == 0.0) || 
	  (pnts[numread].coord[1] == 0.0)) {
	continue;
      }

      numread++;
      if (numread == NUM_POINTS) {
	if (extract_basins(numread, pnts, qpnts, qprops, 
			   max_depth, z_inter, 
			   vs_thresh) != UCVM_CODE_SUCCESS) {
	  fprintf(stderr, "Query basins failed\n");
	  return(1);
	}
	numread = 0;
      }
    }
  }

  if (numread > 0) {
    if (extract_basins(numread, pnts, qpnts, qprops,
		       max_depth, z_inter, 
		       vs_thresh) != UCVM_CODE_SUCCESS) {
      fprintf(stderr, "Query basins failed\n");
      return(1);
    }
    numread = 0;
  }

  ucvm_finalize();
  free(pnts);
  free(qpnts);
  free(qprops);

  return(0);
}
