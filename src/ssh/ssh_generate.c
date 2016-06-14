/**
 * ssh_generate.c - Generates a mesh containing Small Scale Heterogeneities.
 *
 * Implemented by David Gill <davidgil@usc.edu>
 */

#include "ssh_generate.h"

#define ARRs3d(A,i,j,k) A[(i)*((long)n3iso_bigger)*((long)n2iso_bigger)+(j)*((long)n3iso_bigger)+(k)]
#define ARRrand(A,i,j,k) A[(k)*((long)n1iso_bigger)*((long)n2iso_bigger)+(j)*((long)n1iso_bigger)+(i)]
#define ABS(x)           (((x) < 0) ? -(x) : (x))

extern char *optarg;

float randn() {
	float x1, x2, w, y1;
    do {
    	x1 = 2.0 * (double)rand()/RAND_MAX - 1.0;
        x2 = 2.0 * (double)rand()/RAND_MAX - 1.0;
        w = x1 * x1 + x2 * x2;
    } while ( w >= 1.0 );

    w = sqrt( (-2.0 * log( w ) ) / w );
    y1 = x1 * w;
    return y1;
}

/**
 * Let's the user know how to use this utility.
 */
void usage() {
	  printf("Usage: ssh_generate [-h] [-m mesh_to_add] [--d1 distance] [--hurst hurst value] [--l1 correlation length] \n");
	  printf("       [--st23 stretching factor] [--seed random value] [--n1 distribs vert] [--n2 distribs EW] [--n3 distribs NS]\n\n");
	  printf("where:\n");
	  printf("\t-h This help message\n");
	  printf("\t-m Mesh to which these heterogeneities should be added (if not provided new mesh ssh.out will be created)\n");
	  printf("\t--d1 Sample distance (default is 16m)\n");
	  printf("\t--hurst Defines how rough heterogeneities are (between 0 and 1, default is 0.05)\n");
	  printf("\t--l1 Correlation length in vertical direction (default 50 meters)\n");
	  printf("\t--st23 Factor by which heterogeneities are stretched horizontally (default is 5)\n");
	  printf("\t--seed Random seed value set by user\n");
	  printf("\t--n1 Distributions along vertical axis.\n");
	  printf("\t--n2 Distributions along EW axis.\n");
	  printf("\t--n3 Distributions along NS axis.\n\n");
	  printf("Version: %s\n\n", VERSION);

	  return;
}

/**
 * Initializes a 2D array of rows and columns.
 */
double **mallocHelper(int rows, int cols) {
	int i = 0;
	double **array = malloc(rows * sizeof(double *));
	for (i = 0; i < rows; i++) {
		array[i] = malloc(cols * sizeof(double));
	}
	return array;
}

void freeHelperMalloc(double **array, int rows) {
	int i = 0;
	for (i = 0; i < rows; i++) {
		free(array[i]);	
	}
	free(array);
}

/**
 * Initializes a 2D array of rows and columns in FFTW format.
 */
fftw_complex **mallocHelperFFTW(int rows, int cols) {
	int i = 0;
	fftw_complex **array = malloc(rows * sizeof(fftw_complex *));
	for (i = 0; i < rows; i++) {
		array[i] = malloc(cols * sizeof(fftw_complex));
		memset(array[i], 0, cols * sizeof(fftw_complex));
	}
	return array;
}

void freeHelperFFTW(fftw_complex **array, int rows) {
	int i = 0;
	for (i = 0; i < rows; i++) {
		free(array[i]);
	}
	free(array);
}

/**
 * Initializes a 3D array of rows and columns.
 */
void mallocHelper3D(int z, int y, int x, double ***result) {
	result = malloc(z * sizeof(double **));
	int i = 0; int j = 0;
	for (i = 0; i < z; i++) {
		result[z] = malloc(y * sizeof(double *));
		for (j = 0; j < y; j++) {
			result[z][y] = malloc(x * sizeof(double));
		}
	}
}

/**
 * Extracts 2D plane from 3D array.
 */
fftw_complex **squeeze(int zIndex, int nz, int ny, int nx, int ty, int tx, fftw_complex *array) {
	int i = 0; int j = 0;
	fftw_complex **retArray = mallocHelperFFTW(ty, tx);
	for (i = 0; i < ty; i++) {
		for (j = 0; j < tx; j++) {
			retArray[i][j][0] = array[zIndex * (long)ny * (long)nx + i * (long)nx + j][0];
			retArray[i][j][1] = array[zIndex * (long)ny * (long)nx + i * (long)nx + j][1];
		}
	}
	return retArray;
}

/**
 * Nice FFT function.
 * Finds the next mixed power of 2 and 3 for a given input N
 * This number allows a fairly fast FFT without having til go to the optimal next power of 2.
 */
int nicefft(double num, double *nice_n_bigger, double *nice_n_smaller) {
	int n2 = 0, i = 0, j = 0;
	double n3 = 0;
	double *p2, *p3, **p22, **p33, **p2233;
	double retValBigger = INFINITY, retValSmaller = -1 * INFINITY;

	// Find power of 2 that exceeds num.
	while (pow(2, n2) < num) {
		n2++;
	}

	n3 = round(n2 / 1.5);

	p2 = malloc(n2 * sizeof(double));
	p3 = malloc(n3 * sizeof(double));
	p22 = mallocHelper(n3, n2);
	p33 = mallocHelper(n3, n2);
	p2233 = mallocHelper(n3, n2);

	for (i = 1; i <= n2; i++) {
		p2[i - 1] = pow(2, i);
	}

	for (i = 0; i < n3; i++) {
		p3[i] = pow(3, i);
	}

	for (i = 0; i < n3; i++) {
		for (j = 0; j < n2; j++) {
			p22[i][j] = p2[j];
		}
	}

	for (i = 0; i < n3; i++) {
		for (j = 0; j < n2; j++) {
			p33[i][j] = p3[i];
		}
	}

	// Multiply the elements together.
	for (i = 0; i < n3; i++) {
		for (j = 0; j < n2; j++) {
			p2233[i][j] = p22[i][j] * p33[i][j];
		}
	}

	// Now find the biggest element that is less than num and the smallest one that is greater than num.
	for (i = 0; i < n3; i++) {
		for (j = 0; j < n2; j++) {
			if (p2233[i][j] >= num && p2233[i][j] < retValBigger) retValBigger = p2233[i][j];
			if (p2233[i][j] <= num && p2233[i][j] > retValSmaller) retValSmaller = p2233[i][j];
		}
	}

	memcpy(nice_n_bigger, &retValBigger, sizeof(double));
	memcpy(nice_n_smaller, &retValSmaller, sizeof(double));

	free(p2);
	free(p3);
	freeHelperMalloc(p22, n3);
	freeHelperMalloc(p33, n3);
	freeHelperMalloc(p2233, n3);

	return 0;

}

/**
 * Permutes the array.
 */
int permute(fftw_complex **inarray, fftw_complex **outarray, int *permuteArray, int rows, int permuteLength) {
	int i = 0;
	int j = 0;

	if (permuteArray[0] == 1 && permuteArray[1] == 2) {
		for (j = 0; j < rows; j++) {
			for (i = 0; i < permuteLength; i++) {
				outarray[j][i][0] = inarray[j][i][0];
				outarray[j][i][1] = inarray[j][i][1];
			}
		}
	} else {
		for (i = 0; i < rows; i++) {
			for (j = 0; j < permuteLength; j++) {
				outarray[i][j][0] = inarray[j][i][0];
				outarray[i][j][1] = inarray[j][i][1];
			}
		}
	}

	return 0;
}

/**
 * This function interpolates using the FFT method.
 */
int interpft(fftw_complex **inarray, int m, int n, int ny, int dim, fftw_complex **outarray) {
	// All arguments are required.
	int maxToUse = -1;
	int i = 0; int j = 0; int incr = -1;
	int nyqst = 0;
	fftw_complex **outarray_permute;
	int permarray[2];
	int swap = 1;

	if (dim == 2) {
		swap = m;
		m = n;
		n = swap;
	}

	if (n > dim) maxToUse = n;
	else maxToUse = dim;

	if (dim == 2) {
		permarray[0] = 2;
		permarray[1] = 1;
	} else {
		permarray[0] = 1;
		permarray[1] = 2;
	}

	outarray_permute = mallocHelperFFTW(m, n);

	permute(inarray, outarray_permute, permarray, m, maxToUse);

	if (ny > m) {
		incr = 1;
	} else {
		if (ny == 0) return 0;
		incr = floor(m / ny) + 1;
		ny = incr * ny;
	}

	fftw_complex **results = mallocHelperFFTW(m, n);

	fftw_plan p;

	for (i = 0; i < n; i++) {
		fftw_complex *fft_out = malloc(m * sizeof(fftw_complex));
		fftw_complex *fft_in = malloc(m * sizeof(fftw_complex));
		for (j = 0; j < m; j++) {
			fft_in[j][0] = outarray_permute[j][i][0];
			fft_in[j][1] = outarray_permute[j][i][1];
		}
		p = fftw_plan_dft_1d(m, fft_in, fft_out, FFTW_FORWARD, FFTW_ESTIMATE);
		fftw_execute(p);
		fftw_destroy_plan(p);
		for (j = 0; j < m; j++) {
			results[j][i][0] = fft_out[j][0];
			results[j][i][1] = fft_out[j][1];
		}
		free(fft_out);
		free(fft_in);
	}

	freeHelperFFTW(outarray_permute, m);

	nyqst = ceil((m + 1.0f) / 2.0f);

	for (j = 0; j < n; j++) {
		fftw_complex *fft_in = malloc(ny * sizeof(fftw_complex));
		memset(fft_in, 0, ny * sizeof(fftw_complex));
		fftw_complex *fft_out = malloc(ny * sizeof(fftw_complex));
		memset(fft_out, 0, ny * sizeof(fftw_complex));
		for (i = 0; i < ny; i++) {
			if (i < nyqst) {
				fft_in[i][0] = results[i][j][0];
				fft_in[i][1] = results[i][j][1];
			} else if (i >= nyqst && i < nyqst + ny - m) {
				fft_in[i][0] = 0;
				fft_in[i][1] = 0;
			} else if (i >= nyqst + ny - m) {
				fft_in[i][0] = results[i - ny + m][j][0];
				fft_in[i][1] = results[i - ny + m][j][1];
			}
		}

		p = fftw_plan_dft_1d(ny, fft_in, fft_out, FFTW_BACKWARD, FFTW_ESTIMATE);
		fftw_execute(p);
		fftw_destroy_plan(p);

		for (i = 0; i < ny; i += incr) {
			if (permarray[0] == 1 && permarray[1] == 2) {
				outarray[i / incr][j][0] = fft_out[i][0] / m;
				outarray[i / incr][j][1] = fft_out[i][1] / m;
			} else {
				outarray[i / incr][j][0] = fft_out[i][0] / m;
				outarray[i / incr][j][1] = fft_out[i][1] / m;
			}
		}

		free(fft_in);
		free(fft_out);
	}

	freeHelperFFTW(results, m);

	return 0;

}

/**s
 * This function is primarily responsible for calculating the ssh mesh.
 */
int pow3iso(double dx, double hurst, double l1, double seed, double n1iso_bigger, double n2iso_bigger,
			double n3iso_bigger, fftw_complex *result) {
	// Seed the random number generator.
	srand(seed);

	double dk1, dk2, dk3;
	double *k1s, *k2s, *k3s;
	double **k12squared, **k123squared, **ampspec;
	fftw_complex *sim3d;
	double kc, powerexponent, amplitudeexponent;

	fftw_complex xbar;
	double distBeforeDiv = 0;

	long i = 0, j = 0, k = 0;

	size_t totalsize = 0;

	FILE *fp;

	dk1 = 1 / (n1iso_bigger * dx);
	dk2 = 1 / (n2iso_bigger * dx);
	dk3 = 1 / (n3iso_bigger * dx);

	k1s = malloc(n1iso_bigger * sizeof(double));
	for (i = 0; i <= n1iso_bigger / 2 - 1; i++) {
		k1s[i] = dk1 * i;
	}
	for (i = 1; i <= n1iso_bigger / 2; i++) {
		k1s[(int)(n1iso_bigger) - i] = dk1 * -1 * i;
	}

	k2s = malloc(n2iso_bigger * sizeof(double));
	for (i = 0; i <= n2iso_bigger / 2 - 1; i++) {
		k2s[i] = dk2 * i;
	}
	for (i = 1; i <= n2iso_bigger / 2; i++) {
		k2s[(int)(n2iso_bigger) - i] = dk2 * -1 * i;
	}

	k3s = malloc(n3iso_bigger * sizeof(double));
	for (i = 0; i <= n3iso_bigger / 2 - 1; i++) {
		k3s[i] = dk3 * i;
	}
	for (i = 1; i <= n3iso_bigger / 2; i++) {
		k3s[(int)(n3iso_bigger) - i] = dk3 * -1 * i;
	}

	k12squared = mallocHelper((int)n1iso_bigger, (int)n2iso_bigger);

	for (i = 0; i < n1iso_bigger; i++) {
		for (j = 0; j < n2iso_bigger; j++) {
			k12squared[i][j] = pow(k1s[i], 2) + pow(k2s[j], 2);
		}
	}

	kc = 1 / (2 * 3.14159265359 * l1);
	powerexponent = 2 * ((-1 * hurst) - 1.5);
	amplitudeexponent = 0.5 * powerexponent;

	printf("Allocating sim3d\n");
	
	// Calculate totalsize.
	totalsize = (long)n1iso_bigger * (long)n2iso_bigger * (long)n3iso_bigger;
	
	sim3d = malloc(totalsize * sizeof(fftw_complex)); // * sizeof(fftw_complex));

	if (sim3d) { } else { printf("SIM3D not allocated!\n"); exit(0); }

	srand(seed);

	k123squared = mallocHelper((int)n3iso_bigger, (int)n2iso_bigger);
	ampspec = mallocHelper((int)n1iso_bigger, (int)n2iso_bigger);

	float *randarray = malloc(totalsize * sizeof(float));
	float *randarray2 = malloc(totalsize * sizeof(float));

	fp = fopen("floats.in", "rb");

	if (fp != NULL) {
		printf("Reading in random array of floats\n");

		fp = fopen("floats.in", "rb");
		fread(randarray, sizeof(float), totalsize, fp);
		fclose(fp);

		fp = fopen("floats_complex.in", "rb");
		fread(randarray2, sizeof(float), totalsize, fp);
		fclose(fp);
	} else {
		printf("Generating random numbers.\n");

		for (i = 0; i < totalsize; i++) {
			randarray[i] = randn();
			randarray2[i] = randn();
		}
	}

	// Now loop through n3iso_bigger
	for (j = 0; j < n3iso_bigger; j++) {
		for (i = 0; i < n1iso_bigger; i++) {
			for (k = 0; k < n2iso_bigger; k++) {
				k123squared[j][k] = k12squared[i][k] + pow(k3s[j], 2);
				ampspec[i][k] = pow(1.0f + (k123squared[j][k] / pow(kc, 2)), amplitudeexponent / 2.0f);
				ARRs3d(sim3d, i, k, j)[0] = ampspec[i][k] * ARRrand(randarray, i, k, j);
				ARRs3d(sim3d, i, k, j)[1] = ampspec[i][k] * ARRrand(randarray2, i, k, j);
			}
		}
	}

	free(randarray);
	free(randarray2);
	freeHelperMalloc(ampspec, (int)n1iso_bigger);
	freeHelperMalloc(k123squared, (int)n3iso_bigger);

	printf("Beginning 3D DFT\n");

	fftw_complex *out_array = malloc(totalsize * sizeof(fftw_complex));
	fftw_plan p;

	p = fftw_plan_dft_3d((int)n1iso_bigger, (int)n2iso_bigger, (int)n3iso_bigger, sim3d, result, FFTW_BACKWARD, FFTW_ESTIMATE);

	fftw_execute(p);
	fftw_destroy_plan(p);

	for (j = 0; j < n3iso_bigger; j++) {
		for (i = 0; i < n1iso_bigger; i++) {
			for (k = 0; k < n2iso_bigger; k++) {
				ARRs3d(result, i, k, j)[0] = ARRs3d(result, i, k, j)[0] / totalsize;
				ARRs3d(result, i, k, j)[1] = ARRs3d(result, i, k, j)[1] / totalsize;
			}
		}
	}

	free(sim3d);
	free(out_array);

	xbar[0] = 0;
	xbar[1] = 0;

	printf("Calculating standard deviation\n");
	for (i = 0; i < n1iso_bigger; i++) {
		for (j = 0; j < n3iso_bigger; j++) {
			for (k = 0; k < n2iso_bigger; k++) {
				xbar[0] += ARRs3d(result, i, k, j)[0];
				xbar[1] += ARRs3d(result, i, k, j)[1];
			}
		}
	}

	xbar[0] = xbar[0] / totalsize;

	for (i = 0; i < n1iso_bigger; i++) {
		for (j = 0; j < n3iso_bigger; j++) {
			for (k = 0; k < n2iso_bigger; k++) {
				distBeforeDiv += pow(ABS(ARRs3d(result, i, k, j)[0] - xbar[0]), 2.0);
			}
		}
	}

	double stdDev = sqrt(distBeforeDiv / (totalsize - 1));

	printf("stdDev: %.10f\n", stdDev);

	printf("Dividing by standard deviation\n");
	for (i = 0; i < n1iso_bigger; i++) {
		for (j = 0; j < n3iso_bigger; j++) {
			for (k = 0; k < n2iso_bigger; k++) {
				ARRs3d(result, i, k, j)[0] = ARRs3d(result, i, k, j)[0] / stdDev;
				ARRs3d(result, i, k, j)[1] = ARRs3d(result, i, k, j)[1] / stdDev;
			}
		}
	}

	free(k1s);
	free(k2s);
	free(k3s);
	freeHelperMalloc(k12squared, (int)n1iso_bigger);

	return 0;

}

/**
 * Main function.
 */
int main(int argc, char **argv) {

	char mesh_file[1024];				// Mesh file to output these ssh's
	double d1 = 16;						// Sample distance (default 16m)
	double hurst = 0.05;				// Hurst value (default 0.05)
	double l1 = 50;						// Correlation length (default 50m)
	double st23 = 5;					// Stretch factor horizontally (default 5)
	double seed = 3;					// Random seed (default 3)
	double n1 = 100;					// Vert distribs (default 100)
	double n2 = 100;					// Horizontal distribs EW (default 100)
	double n3 = 100;					// Horizontal distribs NS (default 100)

	double d23, n1iso, n2iso, n3iso;	// Variables to be calculated later on
	double n1iso_bigger, n1iso_smaller, n2iso_bigger, n2iso_smaller, n3iso_bigger, n3iso_smaller;
	int retVal;							// Return value
	fftw_complex *sim3d;				// Resulting matrix (sim3d in the original code)
	fftw_complex **sqzArray;
	fftw_complex **powx;
	fftw_complex **powxy;
	int iz = 0;
	int i = 0; int j = 0; int k = 0;
	int display = 0;
	int info = 0;
	float holder = 0;

	size_t totalsize = 0;

	FILE *fp;

	// Define all the possible options.
	static struct option long_options[] =
	{
			{ "d1", required_argument, 0, 'd' },
			{ "hurst", required_argument, 0, 'u'},
			{ "l1", required_argument, 0, 'l'},
			{ "st23", required_argument, 0, 's'},
			{ "seed", required_argument, 0, 'e'},
			{ "n1", required_argument, 0, 'a'},
			{ "n2", required_argument, 0, 'b'},
			{ "n3", required_argument, 0, 'c'},
			{ "mesh", required_argument, 0, 'm'},
			{ "help", no_argument, 0, 'h'},
			{ 0, 0, 0, 0 }
	};

	// Define the option index and next option character.
	int option_index = 0;
	int nextopt = 0;

	// Loop through the available options and set variables.
	while (1) {
		nextopt = getopt_long(argc, argv, "m:u:d:hl:s:e:a:b:c:pi", long_options, &option_index);
		if (nextopt == -1) break;

		switch (nextopt) {
		case '0':
			break;
		case 'm':
			sprintf(mesh_file, "%s", optarg);
			break;
		case 'u':
			hurst = atof(optarg);
			break;
		case 'd':
			d1 = atof(optarg);
			break;
		case 'h':
			usage();
			exit(0);
		case 'l':
			l1 = atof(optarg);
			break;
		case 's':
			st23 = atof(optarg);
			break;
		case 'e':
			seed = atof(optarg);
			break;
		case 'a':
			n1 = atof(optarg);
			break;
		case 'b':
			n2 = atof(optarg);
			break;
		case 'c':
			n3 = atof(optarg);
			break;
		case 'p':
			display = 1;
			break;
		case 'i':
			info = 1;
			break;
		}
	}

	if (strcmp(mesh_file, "") == 0) {
		sprintf(mesh_file, "./ssh.out");
	}

	// Let the user know we're starting to build the mesh with these properties.
	printf("Beginning generation of small-scale heterogeneities mesh.\n\n");

	printf("Using properties:\n");
	printf("\tHurst: %f\n", hurst);
	printf("\tSample distance: %f\n", d1);
	printf("\tCorrelation length vertically: %f\n", l1);
	printf("\tStretching factor horizontally: %f\n", st23);
	printf("\tSeed used: %f\n", seed);
	printf("\tDistributions vertically: %f\n", n1);
	printf("\tDistributions East-West: %f\n", n2);
	printf("\tDistributions North-South: %f\n", n3);
	printf("\tOutputting to file: %s\n", mesh_file);

	d23 = d1 * st23;
	n1iso = n1;
	n2iso = ceil(n2/st23);
	n3iso = ceil(n3/st23);

	printf("\n\nBeginning nicefft for n1\n");
	nicefft(n1iso, &n1iso_bigger, &n1iso_smaller);
	printf("n1iso_bigger: %f, n1iso_smaller: %f\n\nBeginning nicefft for n2\n", n1iso_bigger, n1iso_smaller);
	nicefft(n2iso, &n2iso_bigger, &n2iso_smaller);
	printf("n2iso_bigger: %f, n2iso_smaller: %f\n\nBeginning nicefft for n3\n", n2iso_bigger, n2iso_smaller);
	nicefft(n3iso, &n3iso_bigger, &n3iso_smaller);
	printf("n3iso_bigger: %f, n3iso_smaller: %f\n\n", n3iso_bigger, n3iso_smaller);

	if (info == 1) return -1;

	totalsize = (long)n1iso_bigger * (long)n2iso_bigger * (long)n3iso_bigger;

	if (!display) {
		fp = fopen(mesh_file, "wb");
	}

	sim3d = malloc(totalsize * sizeof(fftw_complex));

	printf("Starting pow3iso\n");
	retVal = pow3iso(d1, hurst, l1, seed, n1iso_bigger, n2iso_bigger, n3iso_bigger, sim3d);
	printf("Finished pow3iso\n");

	for (iz = 0; iz < n1; iz++) {

		powxy = mallocHelperFFTW(n3, n2);
		powx = mallocHelperFFTW(n2, n3);

		sqzArray = squeeze(iz, n1iso_bigger, n2iso_bigger, n3iso_bigger, n2iso, n3iso, sim3d);

		interpft(sqzArray, (int)n2iso, (int)n3iso, n2, 1, powx);
		interpft(powx, n2, n3iso, n3, 2, powxy);

		printf("Calculated interpft for index %d\n", iz);

		fseek(fp, (long)iz * (long)n2 * (long)n3 * sizeof(float), SEEK_SET);

		for (j = 0; j < n3; j++) {
			for (k = 0; k < n2; k++) {
				holder = powxy[j][k][0];
				fwrite(&holder, 1, sizeof(float), fp);	
			}
		}

		printf("Wrote slice to file.\n");

		freeHelperFFTW(powxy, n3);
		freeHelperFFTW(powx, n2);
		freeHelperFFTW(sqzArray, n2iso);
	}

	// Now write everything out.
	if (display) {
		printf("Calculations complete. Displaying results.\n");
	}

	if (display) {
		for (i = 0; i < n3; i++) {
			printf("z-index: %d\n", i);
			for (j = 0; j < n1; j++) {
				for (k = 0; k < n2; k++) {
					//printf("%f ", pow3aniso[j][i][k][0]);
				}
				printf("\n");
			}
		}
	}

	if (!display) {
		fclose(fp);
	}

	free(sim3d);

	printf("\nAll finished!\n");

	return 0;

}
