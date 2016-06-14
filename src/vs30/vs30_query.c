/**
 * vs30_map.c - Returns the Vs30 value for a given lat-long and set of paramters.
 *
 * Created by David Gill <davidgil@usc.edu>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include "ucvm.h"
#include "ucvm_utils.h"

#define NUM_POINTS 20000
#define DEFAULT_ZRANGE_MIN 0.0
#define DEFAULT_ZRANGE_MAX 350.0
#define DEFAULT_Z_INTERVAL 0.1
#define OUTPUT_FMT "%10.4lf %10.4lf %10.3lf\n"

/* Display usage information */
void usage() {
	printf("Usage: vs30_query [-h] [-m models<:ifunc>] [-f config] [-i inter]\n\n");
	printf("where:\n");
	printf("\t-h This help message\n");
	printf("\t-f Configuration file. Default is ./ucvm.conf.\n");
	printf("\t-i Interval between query points along z-axis (m, default is 1)\n");
	printf("\t-m Comma delimited list of crustal/GTL models to query in order\n");
	printf("Notes:\n");
	printf("\t- If running interactively, type Cntl-D to end input coord list.\n\n");
	printf("Version: %s\n\n", VERSION);
	return;
}

int vs30_query(int points, ucvm_point_t *pnts, double z_inter) {

	int p = 0;
	int i = 0;
	ucvm_point_t query_pt;
	ucvm_data_t query_data;

	double vs_sum = 0.0;
	double vs = 0.0;

	for (p = 0; p < points; p++) {
		query_pt.coord[0] = pnts[p].coord[0];
		query_pt.coord[1] = pnts[p].coord[1];

		for (i = 0; i < 30; i++) {
			query_pt.coord[2] = i;
			if (ucvm_query(1, &query_pt, &query_data) != UCVM_CODE_SUCCESS) {
				fprintf(stderr, "UCVM query failed.\n");
				exit(-3);
			}
			vs_sum += 1.0/query_data.cmb.vs;
		}

		vs = 30/vs_sum;

		printf(OUTPUT_FMT, pnts[p].coord[0], pnts[p].coord[1], vs);
	}

	return UCVM_CODE_SUCCESS;
}

int main(int argc, char **argv) {
	int opt;
	char modellist[UCVM_MAX_MODELLIST_LEN];
	char configfile[UCVM_MAX_PATH_LEN];
	double zrange[2];
	double z_inter;

	ucvm_ctype_t cmode;
	int have_model = 0;
	int have_zrange = 0;
	int have_map = 0;

	ucvm_point_t *pnts;
	int numread = 0;
	char map_label[UCVM_MAX_LABEL_LEN];

	cmode = UCVM_COORD_GEO_DEPTH;
	snprintf(configfile, UCVM_MAX_PATH_LEN, "%s", "./ucvm.conf");
	snprintf(modellist, UCVM_MAX_MODELLIST_LEN, "%s", "1d");
	snprintf(map_label, UCVM_MAX_LABEL_LEN, "%s", UCVM_MAP_UCVM);
	zrange[0] = DEFAULT_ZRANGE_MIN;
	zrange[1] = DEFAULT_ZRANGE_MAX;
	z_inter = DEFAULT_Z_INTERVAL;

	/* Parse options */
	while ((opt = getopt(argc, argv, "i:v:f:hm:p:z:")) != -1) {
		switch (opt) {
			case 'i':
				z_inter = (double)atof(optarg);
				if (z_inter <= 0.0) {
					fprintf(stderr, "Invalid z interval %s.\n", optarg);
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

	/* Read in coords */
	while (!feof(stdin)) {
		memset(&(pnts[numread]), 0, sizeof(ucvm_point_t));
		if (fscanf(stdin,"%lf %lf", &(pnts[numread].coord[0]), &(pnts[numread].coord[1])) == 2) {
			/* Check for scan failure */
			if ((pnts[numread].coord[0] == 0.0) || (pnts[numread].coord[1] == 0.0)) {
				continue;
			}

			numread++;

			if (numread == NUM_POINTS) {
				if (vs30_query(numread, pnts, z_inter) != UCVM_CODE_SUCCESS) {
					fprintf(stderr, "Vs30 query failed\n");
					return(1);
				}
				numread = 0;
			}
		}
	}

	if (numread > 0) {
		if (vs30_query(numread, pnts, z_inter) != UCVM_CODE_SUCCESS) {
			fprintf(stderr, "Vs30 query failed\n");
			return(1);
		}
		numread = 0;
	}

	ucvm_finalize();
	free(pnts);

	return(0);
}
