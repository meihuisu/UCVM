/**
 * ssh_merge.c - Merges already generated heterogeneities to a given
 * 				 AWP mesh or generates an e-tree for use in Hercules.
 *
 * Implemented by David Gill <davidgil@usc.edu>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include "etree.h"

/* UCVM Etree payload datatype */
typedef struct ssh_epayload_t {
  float ssh;
} ssh_epayload_t;

void usage() {
	 printf("Usage: ssh_add [-h] -i [heterogeneity mesh] -m [AWP mesh] -o [output file] \n");
	 printf("where:\n");
	 printf("\t-h This help message.\n");
	 printf("\t-i The input heterogeneity medium generated with ssh_generate.\n");
	 printf("\t-m If adding to AWP mesh, the input AWP mesh.\n");
	 printf("\t-o The output AWP mesh or e-tree.\n");

	 return;
}

int main(int argc, char **argv) {

	char input_file[1024];
	char output_file[1024];
	char mesh_file[1024];
	char appmeta[1024];
	int option_index = 0;
	int nextopt = 0;
	int counter = 0;

	long input_size = 0;
	long mesh_size = 0;

	int x = 0, y = 0, z = 0;
	int i = 0, j = 0, k = 0;

	double std = 0.1;

	FILE *fp1 = NULL;
	FILE *fp2 = NULL;
	FILE *fp3 = NULL;

	float ssh = 0;
	float vp = 0;
	float vs = 0;
	float rho = 0;

	etree_t *eth = NULL;
	etree_addr_t addr;
	etree_addr_t hitloc;
	ssh_epayload_t sshpayload;

	//addr.level = 1;
	addr.type = ETREE_LEAF;

	double progress = 0;
	double nextLevel = 5;

	// Define all the possible options.
	static struct option long_options[] =
	{
			{ "input", required_argument, 0, 'i' },
			{ "output", required_argument, 0, 'o'},
			{ "std", required_argument, 0, 's'},
			{ "mesh", optional_argument, 0, 'm'},
			{ "x", optional_argument, 0, 'x'},
			{ "y", optional_argument, 0, 'y'},
			{ "z", optional_argument, 0, 'z'},
			{ "help", no_argument, 0, 'h'},
			{ 0, 0, 0, 0 }
	};

	while (1) {
		nextopt = getopt_long(argc, argv, "i:o:m:h:s:x:y:z:", long_options, &option_index);
		if (nextopt == -1) break;

		switch (nextopt) {
		case '0':
			break;
		case 'i':
			sprintf(input_file, "%s", optarg);
			break;
		case 'o':
			sprintf(output_file, "%s", optarg);
			break;
		case 'm':
			sprintf(mesh_file, "%s", optarg);
			break;
		case 's':
			std = atof(optarg);
			break;
		case 'x':
			x = atoi(optarg);
			break;
		case 'y':
			y = atoi(optarg);
			break;
		case 'z':
			z = atoi(optarg);
			break;
		case 'h':
			usage();
			exit(0);
		}
	}

	if (input_file[0] == '\0' || output_file[0] == '\0') {
		fprintf(stderr, "Input and output files required.\n");
		return -1;
	}

	if (mesh_file[0] == '\0') {

		if (x == 0 || y == 0 || z == 0) {
			fprintf(stderr, "X, Y, or Z dimension not included.\n");
			return -1;
		}

		// We want to use the e-tree version of this code.
		eth = etree_open(output_file, O_CREAT|O_TRUNC|O_RDWR, 0, 4, 3);

		if (eth == NULL) {
			fprintf(stderr, "Could not open e-tree database.");
			return -1;
		}

		// We have a mesh of heterogeneities so open that now.
		fp1 = fopen(input_file, "rb");

		if (fp1 == NULL) {
			fprintf(stderr, "Could not open input file.\n");
			return -1;
		}

	    sprintf(appmeta, "ssh_generate automated 1/1/2014 1 ssh 34 -118 %u %u %u %u %u %u %u",
	            x, y, 0, z, 1, 1, 1);
	    printf("appmeta: %s\n", appmeta);

	    if (etree_setappmeta(eth, appmeta) != 0) {
	        fprintf(stderr, "%s\n", etree_strerror(etree_errno(eth)));
	        return -1;
	    }

		for (k = 0; k < z; k++) {
			for (j = 0; j < y; j++) {
				for (i = 0; i < x; i++) {
					addr.x = i;
					addr.y = j;
					addr.z = k;

					counter = fread((void*)(&ssh), sizeof(float), 1, fp1);

					sshpayload.ssh = ssh;

					if (counter > 0) {
						if (etree_insert(eth, addr, &sshpayload) != 0) {
							fprintf(stderr, "Error inserting SSH point into e-tree.\n");
							return -1;
						}

						sshpayload.ssh = -1;

						if (etree_search(eth, addr, &hitloc, NULL, &sshpayload) != 0) {
							fprintf(stderr, "Could not find the query address.");
							return -1;
						}

						if (sshpayload.ssh != ssh) {
							fprintf(stderr, "Error on insertion. Read value not equal to inserted value.\n");
							return -1;
						}

					} else {
						fprintf(stderr, "Error reading next SSH point.\n");
						return -1;
					}

					progress = (double)((k * y * x) + (j * x) + i) / (double)(z * y * x);
					progress = progress * 100.0;

					if (progress >= nextLevel) {
						printf("%.1f%% completed.\n", progress);
						nextLevel += 5;
					}
				}
			}
		}

		etree_close(eth);

	} else {
		// We have a mesh of heterogeneities so open that now.
		fp1 = fopen(input_file, "rb");

		if (fp1 == NULL) {
			fprintf(stderr, "Could not open input file.\n");
			return -1;
		}

		fp2 = fopen(mesh_file, "rb");

		if (fp2 == NULL) {
			fprintf(stderr, "Could not open mesh file.\n");
			return -1;
		}

		fseek(fp1, 0, SEEK_END);
		input_size = ftell(fp1);
		input_size = input_size / 4.0;
		fseek(fp1, 0, SEEK_SET);

		fseek(fp2, 0, SEEK_END);
		mesh_size = ftell(fp2) / 3.0 / 4.0;
		fseek(fp2, 0, SEEK_SET);

		if (input_size != mesh_size) {
			fprintf(stderr, "Mesh size and input file size are different.\n");
			return -1;
		}

		fp3 = fopen(output_file, "wb");

		if (fp3 == NULL) {
			fprintf(stderr, "Could not open output file for writing.\n");
			return -1;
		}

		printf("Adding heterogeneities to mesh...\n");

		for (counter = 0; counter < input_size; counter++) {
			fread((void*)(&ssh), sizeof(float), 1, fp1);
			fread((void*)(&vp), sizeof(float), 1, fp2);
			fread((void*)(&vs), sizeof(float), 1, fp2);
			fread((void*)(&rho), sizeof(float), 1, fp2);

			// Add to slowness.
			vp = 1.0 / vp;
			vs = 1.0 / vs;

			vp = vp * (1 + (std * ssh));
			vs = vs * (1 + (std * ssh));
			rho = rho * (1 + (std * ssh));

			vp = 1.0 / vp;
			vs = 1.0 / vs;

			fwrite((void*)(&vp), sizeof(float), 1, fp3);
			fwrite((void*)(&vs), sizeof(float), 1, fp3);
			fwrite((void*)(&rho), sizeof(float), 1, fp3);

			progress = (double)counter / (double)input_size;
			progress = progress * 100.0;

			if (progress >= nextLevel) {
				printf("%.1f%% completed.\n", progress);
				nextLevel += 5;
			}
		}

		fclose(fp1);
		fclose(fp2);
		fclose(fp3);
	}

	printf("Completed.\n");

	return 0;
}
