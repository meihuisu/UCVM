/**
 * vs30_query_mpi.c - Generates Vs30 maps given a starting lat, lon and a number of
 * x and y co-ordinate points.
 *
 * Created by David Gill <davidgil@usc.edu>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <mpi.h>
#include "ucvm.h"
#include "ucvm_utils.h"

/* Constants */
#define MAX_RES_LEN 256
#define OUTPUT_FMT "%10.4lf %10.4lf %10.3lf\n"

#define WORKERREADY 99
#define DIEOFF -1

/* Get opt args */
extern char *optarg;
extern int optind, opterr, optopt;

/* Display usage information */
void usage() {
	printf("Usage: vs30_query_mpi [-h] [-b outfile] [-m models<:ifunc>] [-f config] [-i inter] ");
	printf("[-l lon,lat] [-s spacing] [-x num lon pts] [-y num lat pts]\n\n");
	printf("where:\n");
	printf("\t-b Binary output to file.\n");
	printf("\t-h This help message\n");
	printf("\t-m Comma delimited list of crustal/GTL models to query in order\n");
	printf("\t-f Configuration file. Default is ./ucvm.conf.\n");
	printf("\t-l Bottom-left lat,lon separated by comma.\n");
	printf("\t-s Grid spacing.\n");
	printf("\t-x Number of longitude points.\n");
	printf("\t-y Number of latitude points.\n");
	printf("Notes:\n");
	printf("\t- If running interactively, type Cntl-D to end input coord list.\n\n");
	printf("Version: %s\n\n", VERSION);
	return;
}

int vs30_query(int points, ucvm_point_t *pnts, float *outvs30) {

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

		outvs30[p] = vs;
	}

	return UCVM_CODE_SUCCESS;

}

int main(int argc, char **argv) {
	int opt;
	char modellist[UCVM_MAX_MODELLIST_LEN];
	char configfile[UCVM_MAX_PATH_LEN];
	double latlon[2];
	int nx = 0;
	int ny = 0;
	int currentline = 0;	// Current line we're working on.
	double spacing = 0;

	ucvm_ctype_t cmode;
	int have_model = 0;
	int have_map = 0;

	int numread = 0;
	char map_label[UCVM_MAX_LABEL_LEN];
	char *binary_outfile = malloc(512 * sizeof(char));

	int numprocs, rank, i;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	cmode = UCVM_COORD_GEO_DEPTH;
	snprintf(configfile, UCVM_MAX_PATH_LEN, "%s", "./ucvm.conf");
	snprintf(modellist, UCVM_MAX_MODELLIST_LEN, "%s", "1d");
	snprintf(map_label, UCVM_MAX_LABEL_LEN, "%s", UCVM_MAP_UCVM);

	/* Parse options */
	while ((opt = getopt(argc, argv, "hb:m:f:i:l:s:x:y:")) != -1) {
		switch (opt) {
		case 'h':
			usage();
			exit(0);
			break;
		case 'b':
			binary_outfile = optarg;
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
		case 'f':
			if (strlen(optarg) < UCVM_MAX_PATH_LEN) {
				snprintf(configfile, UCVM_MAX_PATH_LEN, "%s", optarg);
			} else {
				fprintf(stderr, "Invalid config file specified: %s.\n", optarg);
				usage();
				exit(1);
			}
			break;
		case 'l':
			if (list_parse(optarg, UCVM_MAX_PATH_LEN,
					       latlon, 2) != UCVM_CODE_SUCCESS) {
				fprintf(stderr, "Invalid lat, lon range specified: %s.\n", optarg);
				usage();
				exit(1);
			}
			break;
		case 's':
			spacing = (double) atof(optarg);
			if (spacing <= 0.0) {
				fprintf(stderr, "Invalid spacing %s", optarg);
				usage();
				exit(1);
			}
			break;
		case 'x':
			nx = (int) atoi(optarg);
			if (nx <= 0) {
				fprintf(stderr, "Invalid number of x points %s", optarg);
				usage();
				exit(1);
			}
			break;
		case 'y':
			ny = (int) atoi(optarg);
			if (ny <= 0) {
				fprintf(stderr, "Invalid number of x points %s", optarg);
				usage();
				exit(1);
			}
			break;
		default: /* '?' */
			usage();
			exit(1);
		}
	}

	/* Initialize interface */
	if (ucvm_init(configfile) != UCVM_CODE_SUCCESS) {
		fprintf(stderr, "Failed to initialize UCVM API\n");
		return (1);
	}

	/* Add models */
	if (have_model == 0) {
		/* Add default model if none specified */
		fprintf(stderr, "Using 1D as default model.\n");
	}
	if (ucvm_add_model_list(modellist) != UCVM_CODE_SUCCESS) {
		fprintf(stderr, "Failed to enable model list %s\n", modellist);
		return (1);
	}

	/* Set user map if necessary */
	if (have_map == 1) {
		if (ucvm_use_map(map_label) != UCVM_CODE_SUCCESS) {
			fprintf(stderr, "Failed to set user map %s\n", map_label);
			return (1);
		}
	}

	/* Set z mode for depth*/
	if (ucvm_setparam(UCVM_PARAM_QUERY_MODE, cmode) != UCVM_CODE_SUCCESS) {
		fprintf(stderr, "Failed to set z mode\n");
		return (1);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	MPI_File fh;
	MPI_File_open(MPI_COMM_SELF, binary_outfile, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);

	// We divide the job up in terms of lines (i.e. ny).
	currentline = rank;

	while (currentline < ny) {
		ucvm_point_t *pnts = malloc(nx * sizeof(ucvm_point_t));
		float *retVals = malloc(nx * sizeof(float));

		printf("Current line: %d. Progress: %.2f%%\n", currentline, (float)currentline / (float)ny * 100.0f);

		for (i = 0; i < nx; i++) {
			pnts[i].coord[1] = (currentline * spacing) + latlon[0];
			pnts[i].coord[0] = (i * spacing) + latlon[1];
		}

		vs30_query(nx, pnts, retVals);

		MPI_File_set_view(fh, currentline * nx * sizeof(float), MPI_FLOAT, MPI_FLOAT, "native", MPI_INFO_NULL);
		MPI_File_write(fh, retVals, nx, MPI_FLOAT, MPI_STATUS_IGNORE);

		free(pnts);
		free(retVals);

		currentline += numprocs;

	}

	ucvm_finalize();

	MPI_Barrier(MPI_COMM_WORLD);

	MPI_File_close(&fh);

	MPI_Finalize();

	return (0);
}
