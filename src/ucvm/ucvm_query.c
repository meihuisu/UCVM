#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "ucvm.h"
#include "ucvm_utils.h"

/* Constants */
#define MAX_RES_LEN 256
#define NUM_POINTS 20000
#define ZRANGE_MIN 0.0
#define ZRANGE_MAX 350.0
#define OUTPUT_FMT "%10.4lf %10.4lf %10.3lf %10.3lf %10.3lf %10s %10.3lf %10.3lf %10.3lf %10s %10.3lf %10.3lf %10.3lf %10s %10.3lf %10.3lf %10.3lf\n"


/* Getopt flags */
extern char *optarg;
extern int optind, opterr, optopt;


/* Display resource information */
int disp_resources(int active_only)
{
  int i, j, len;
  ucvm_resource_t resources[MAX_RES_LEN];
  char rtype[MAX_RES_LEN];

  len = MAX_RES_LEN;
  if (ucvm_get_resources(resources, &len) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to retrieve UCVM resources\n");
    return(1);
  }

  printf("Installed Resources:\n");
  for (i = 0; i < len; i++) {
    switch (resources[i].rtype) {
    case UCVM_RESOURCE_MODEL:
      switch(resources[i].mtype) {
      case UCVM_MODEL_CRUSTAL:
	snprintf(rtype, MAX_RES_LEN, "%s", "crustal model");
	break;
      case UCVM_MODEL_GTL:
	snprintf(rtype, MAX_RES_LEN, "%s", "gtl");
	break;
      };
      break;
    case UCVM_RESOURCE_IFUNC:
      snprintf(rtype, MAX_RES_LEN, "%s", "ifunc");
      break;
    case UCVM_RESOURCE_MAP:
      snprintf(rtype, MAX_RES_LEN, "%s", "map");
      break;
    case UCVM_RESOURCE_MODEL_IF:
      snprintf(rtype, MAX_RES_LEN, "%s", "model i/f");
      break;
    case UCVM_RESOURCE_MAP_IF:
      snprintf(rtype, MAX_RES_LEN, "%s", "map i/f");
      break;
    };

    if ((!active_only) || ((active_only) && (resources[i].active))) {
      printf("%12s : %s\n", resources[i].label, rtype);
      if (strlen(resources[i].version) > 0) {
	printf("\t\tVersion    : %s\n", resources[i].version);
      }
      if (strlen(resources[i].config) > 0) {
	printf("\t\tConfig     : %s\n", resources[i].config);
      }
      if (strlen(resources[i].extconfig) > 0) {
	printf("\t\tExt Config : %s\n", resources[i].extconfig);
      }
      for (j = 0; j < resources[i].numflags; j++) {
	printf("\t\tFlag       : %s=%s\n", 
	       resources[i].flags[j].key, resources[i].flags[j].value);
      }
    }
  }

  return(0);
}



/* Usage function */
void usage() {
  printf("Usage: ucvm_query [-m models<:ifunc>] [-p user_map] [-c coordtype] [-f config] [-z zmin,zmax] < file.in\n\n");
  printf("Flags:\n");
  printf("\t-h This help message.\n");
  printf("\t-m Comma delimited list of crustal/GTL models to query in order\n");
  printf("\t   of preference. GTL models may optionally be suffixed with ':ifunc'\n");
  printf("\t   to specify interpolation function.\n");
  printf("\t-c Z coordinate mode: geo-depth (gd, default), geo-elev (ge).\n");
  printf("\t-f Configuration file. Default is ./ucvm.conf.\n");
  printf("\t-p User-defined map to use for elevation and vs30 data.\n");
  printf("\t-v Display model version information only.\n");
  printf("\t-z Optional depth range for gtl/crust interpolation.\n\n");
  printf("Input format is:\n");
  printf("\tlon lat Z\n\n");
  printf("Output format is:\n");
  printf("\tlon lat Z surf vs30 crustal cr_vp cr_vs cr_rho gtl gtl_vp gtl_vs gtl_rho cmb_algo cmb_vp cmb_vs cmb_rho\n\n");
  printf("Notes:\n");
  printf("\t- If running interactively, type Cntl-D to end input coord list.\n\n");
  printf("Version: %s\n\n", VERSION);
  disp_resources(0);
  exit (0);
}



int main(int argc, char **argv)
{
  int i;
  int opt;
  char modellist[UCVM_MAX_MODELLIST_LEN];
  char configfile[UCVM_MAX_PATH_LEN];
  double zrange[2];
  int dispver = 0;

  ucvm_ctype_t cmode;
  int have_model = 0;
  int have_cmode = 0;
  int have_zrange = 0;
  int have_map = 0;

  ucvm_point_t *pnts;
  ucvm_data_t *props;
  int numread = 0;
  char map_label[UCVM_MAX_LABEL_LEN];
  char cr_label[UCVM_MAX_LABEL_LEN];
  char gtl_label[UCVM_MAX_LABEL_LEN];
  char if_label[UCVM_MAX_LABEL_LEN];

  cmode = UCVM_COORD_GEO_DEPTH;
  snprintf(configfile, UCVM_MAX_PATH_LEN, "%s", "./ucvm.conf");
  snprintf(modellist, UCVM_MAX_MODELLIST_LEN, "%s", "1d");
  snprintf(map_label, UCVM_MAX_LABEL_LEN, "%s", UCVM_MAP_UCVM);
  zrange[0] = ZRANGE_MIN;
  zrange[1] = ZRANGE_MAX;

  /* Parse options */
  while ((opt = getopt(argc, argv, "c:f:hm:p:vz:")) != -1) {
    switch (opt) {
    case 'c':
      if (strcmp(optarg, "gd") == 0) {
	cmode = UCVM_COORD_GEO_DEPTH;
      } else if (strcmp(optarg, "ge") == 0) {
	cmode = UCVM_COORD_GEO_ELEV;
      } else {
	fprintf(stderr, "Invalid z mode %s.\n", optarg);
	usage();
	exit(1);
      }
      have_cmode = 1;
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
    case 'v':
      dispver = 1;
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

  /* Set z mode */
  if (!have_cmode) {
    cmode = UCVM_COORD_GEO_DEPTH;
    fprintf(stderr, "Using Geo Depth coordinates as default mode.\n");
  }
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

  if (dispver) {
    disp_resources(1);
    return(0);
  }

  /* Allocate buffers */
  pnts = malloc(NUM_POINTS * sizeof(ucvm_point_t));
  props = malloc(NUM_POINTS * sizeof(ucvm_data_t));

  /* Read in coords */
  while (!feof(stdin)) {
    memset(&(pnts[numread]), 0, sizeof(ucvm_point_t));
    if (fscanf(stdin,"%lf %lf %lf",
               &(pnts[numread].coord[0]),
	       &(pnts[numread].coord[1]),
	       &(pnts[numread].coord[2])) == 3) {

      /* Check for scan failure */
      if ((pnts[numread].coord[0] == 0.0) || 
	  (pnts[numread].coord[1] == 0.0)) {
	continue;
      }

      numread++;
      if (numread == NUM_POINTS) {
	/* Query the UCVM */
	if (ucvm_query(numread, pnts, props) != UCVM_CODE_SUCCESS) {
	  fprintf(stderr, "Query CVM failed\n");
	  return(1);
	}

	/* Display results */
	for (i = 0; i < numread; i++) {
	  ucvm_model_label(props[i].crust.source, 
			   cr_label, UCVM_MAX_LABEL_LEN);
	  ucvm_model_label(props[i].gtl.source, 
	  		   gtl_label, UCVM_MAX_LABEL_LEN);
	  ucvm_ifunc_label(props[i].cmb.source, 
			   if_label, UCVM_MAX_LABEL_LEN);

	  printf(OUTPUT_FMT, 
		 pnts[i].coord[0], pnts[i].coord[1], pnts[i].coord[2],
		 props[i].surf, props[i].vs30,
		 cr_label, props[i].crust.vp, props[i].crust.vs, 
		 props[i].crust.rho, gtl_label, props[i].gtl.vp,
		 props[i].gtl.vs, props[i].gtl.rho,
		 if_label, props[i].cmb.vp, props[i].cmb.vs, 
		 props[i].cmb.rho);
	}

	numread = 0;
      }
    }
  }

  if (numread > 0) {
    /* Query the UCVM */
    if (ucvm_query(numread, pnts, props) != UCVM_CODE_SUCCESS) {
      fprintf(stderr, "Query CVM failed\n");
      return(1);
    }
    
    /* Display results */
    for (i = 0; i < numread; i++) {
      ucvm_model_label(props[i].crust.source, 
		       cr_label, UCVM_MAX_LABEL_LEN);
      ucvm_model_label(props[i].gtl.source, 
		       gtl_label, UCVM_MAX_LABEL_LEN);
      ucvm_ifunc_label(props[i].cmb.source, 
		       if_label, UCVM_MAX_LABEL_LEN);

      printf(OUTPUT_FMT, 
	     pnts[i].coord[0], pnts[i].coord[1], pnts[i].coord[2],
	     props[i].surf, props[i].vs30,
	     cr_label, props[i].crust.vp, props[i].crust.vs, 
	     props[i].crust.rho, gtl_label, props[i].gtl.vp,
	     props[i].gtl.vs, props[i].gtl.rho,
	     if_label, props[i].cmb.vp, props[i].cmb.vs, 
	     props[i].cmb.rho);
    }
    
    numread = 0;
  }

  ucvm_finalize();
  free(pnts);
  free(props);

  return(0);
}
