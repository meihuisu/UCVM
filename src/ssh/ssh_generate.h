/**
 * ssh_generate.h - Generates a mesh containing Small Scale Heterogeneities.
 *
 * Implemented by David Gill <davidgil@usc.edu>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include "fftw3.h"

#ifndef INFINITY
#define INFINITY (1.0/0.0)
#endif

float randn();
void usage();
double **mallocHelper(int rows, int cols);
fftw_complex **mallocHelperFFTW(int rows, int cols);
fftw_complex **squeeze(int zIndex, int nz, int ny, int nx, int ty, int tx, fftw_complex *array);
int nicefft(double num, double *nice_n_bigger, double *nice_n_smaller);
int pow3iso(double dx, double hurst, double l1, double seed, double n1iso_bigger, double n2iso_bigger,
			double n3iso_bigger, fftw_complex *result);
int permute(fftw_complex **inarray, fftw_complex **outarray, int *permuteArray, int rows, int permuteLength);
int interpft(fftw_complex **inarray, int m, int n, int ny, int dim, fftw_complex **outarray);

