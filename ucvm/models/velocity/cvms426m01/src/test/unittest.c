#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "cvmsi.h"
#include "cvmsi_utils.h"
#include "cvmsi_geo2xy.h"

#define SUCCESS 1
#define FAIL 0

#define ILONGLAT2UTM 0
#define IUTM2LONGLAT 1

void utm_geo_(double *rlon, double *rlat, double *rx, double *ry, int *utmzone, int *iway); 

/*
 *	First test: Is our interpolator interpolating correctly?
 *	Inputs: errStr - pointer to error string.
 *	Outputs: 0 or 1 (FAIL or SUCCESS)
 *	Example: firstTest(); Returns 1 if trilinear interpolation gives expected result. 0 if not.
 */
int firstTest(char *errStr) {
	double p[2][3], q[2][2][2];
	
	/* Determine p */
    p[0][0] = 0.0;    
    p[0][1] = 0.0;
    p[0][2] = 0.0;
    p[1][0] = 1.0;    
    p[1][1] = 1.0;
    p[1][2] = 1.0;
	
	/* Make the furthest vertices of q 1.0 and the closest 0.0.
	   That way interpolating (1,1,1) will be 1 and (0,0,0) will be 0. */
	q[0][0][0] = 0.0;
	q[0][0][1] = 1.0;
	q[0][1][0] = 0.0;
	q[0][1][1] = 1.0;
	q[1][0][0] = 1.0;
	q[1][1][0] = 0.0;
	q[1][1][1] = 1.0;
	q[1][0][1] = 0.0;
	
	// This should be 0.5 - half way between 0 and 1.
	double retVal = cvmsi_interp_trilinear(0.5, 0.5, 0.5, p, q);
	
	if (retVal != 0.5) {
		sprintf(errStr, "%sError! 1st interpolation returned value was %lf, not 0.50.\n", errStr, retVal);
		return FAIL;
	}
	
	// This is the first vertex so it is set to zero.
	retVal = cvmsi_interp_trilinear(0.0, 0.0, 0.0, p, q);
	
	if (retVal != 0.0) {
		sprintf(errStr, "%sError! 2nd interpolation returned value was %lf, not 0.00.\n", errStr, retVal);
		return FAIL;
	}
	
	// This is the last vertex so it should be one.
	retVal = cvmsi_interp_trilinear(1.0, 1.0, 1.0, p, q);
	
	if (retVal != 1.0) {
		sprintf(errStr, "%sError! 3rd interpolation returned value was %lf, not 1.00.\n", errStr, retVal);
		return FAIL;
	}
	
	return SUCCESS;
}

/*
 *	Second test: Are we converting to UTM proj correctly?
 *	Inputs: errStr - pointer to error string.
 *	Outputs: 0 or 1 (FAIL or SUCCESS)
 *	Example: secondTest(); Returns 1 if UTM conversion gives expected result. 0 if not.
 */
int secondTest(char *errStr) {
	
	double rlon = -118.0;
	double rlat = 34.0;
	double rx, ry;
	
	int utmzone = 11;
	int iway = ILONGLAT2UTM;
	
	utm_geo_(&rlon, &rlat, &rx, &ry, &utmzone, &iway);
	//printf("\nlon: %f lat: %f\n", rlon, rlat);
	//printf("X: %f Y: %f\n", rx, ry);

	if (rx < 407648.316882 || rx > 407648.316884 || ry < 3762400.269688 || ry > 3762400.26969) {
		sprintf(errStr, "%sError! UTM proj %lf, %lf does not matched expected 407648.32, 3762400.27.\n", errStr, rx, ry);
		return FAIL;
	}

	rlon = -117.0;
	rlat = 35.0;
	utm_geo_(&rlon, &rlat, &rx, &ry, &utmzone, &iway);
	//printf("lon: %f lat: %f\n", rlon, rlat);
	//printf("X: %f Y: %f", rx, ry);

	if (rx < 499999 || rx > 500001 || ry < 3872834.40 || ry > 3872834.50) {
		sprintf(errStr, "%sError! UTM proj %lf, %lf does not matched expected 500000.00, 3872834.44.\n", errStr, rx, ry);
		return FAIL;
	}
	
	return SUCCESS;
}

/* 
 *	Main method 
 */
int main(int argc, char **argv) {
	char version[128];
	char errStr[4096];
	printf("\nStarting Unit Tests\n");
	cvmsi_init("../model/i26");
	cvmsi_version(version, sizeof(version));
	printf("Version ID: %s\nNumber of tests: 2\n\n", version);

	printf("%-40s", "Starting Test 1 (interpolation check): ");
	
	int result = firstTest(errStr);
	int didFail = 0;
	
	if (result == SUCCESS) 
		printf("[PASSED]\n");
	else
		printf("[FAILED]\n%s", errStr);
	
	if (result == FAIL) didFail = 1;
	
	printf("%-40s", "Starting Test 2 (UTM convert check): ");
	result = secondTest(errStr);
	
	if (result == SUCCESS) 
		printf("[PASSED]\n");
	else
		printf("[FAILED]\n%s", errStr);
	
	if (result == FAIL) didFail = 1;
	
	printf("\n");
	
	if (didFail == 1)
		printf("Some tests were not successful. Please re-install\n%s. If that doesn't work, "
			   "please e-mail\ndavidgil@usc.edu for assistance.\n", version);
	else
		printf("Unit tests were successful!\n");
	
	cvmsi_finalize();
	
	printf("\n");
	
	return didFail;
}
