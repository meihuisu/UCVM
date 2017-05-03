#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "cvmsi.h"

#define SUCCESS 1
#define FAIL 0

/*
 *	Converts a number to %10.4'f format.
 *	Input: data - the double to convert to %10.4lf format.
 *	Returns: The formatted float.
 */
double convertTo10P4(double data) {
	char buf[32];
	sprintf(buf, "%10.4lf", data);
	return atof(buf);		
}

/*
 *	Runs a test.
 *	Input:	num - test number
 *			lon - starting longitude
 *			lat - starting latitude
 *			depth - starting depth
 *			moveOn - what should change, either lat, long, or depth
 *			moveTo - move from start (lat, lon, depth) to this (lat, lon, depth)
 *			increment - amount to change
 *			errStr - error string
 *	Returns: 1 or 0 (SUCCESS or FAIL) if the test passed. 
 *	Example: runTest(1, -118, 34, 0, "lat", 35, 0.1)
 *			 Runs the test starting at (-118, 34) at depth zero. Increments by 0.1 until we hit (-118, 35).
 *			 Compares results to atest1.out.
 */
int runTest(int num, float lon, float lat, float depth, char *moveOn, float moveTo, float increment, char *errStr) {
	cvmsi_data_t data;	// Initialize the data struct
	cvmsi_point_t pnt;	// Initialize the point struct
	memset(&pnt, 0, sizeof(cvmsi_point_t));	// Set the memory for the point to be nothing for now.
	memset(&data, 0, sizeof(cvmsi_data_t));	// Set the memory for the data to be nothing for now.
	
	int expectedX, expectedY, expectedZ;	// Expected X,Y,Z grid co-ordinates. 
	double expectedVp, expectedVs, expectedRho, expectedDiffVp, expectedDiffVs;	// Expected material properties.
	
	// Read in the correct values file.
	char readInFile[128];
	sprintf(readInFile, "./data/atest%u.out", num);
	FILE *theFile = fopen(readInFile, "r");
	
	if (theFile == NULL) {
		printf("Could not open %s.\n", readInFile);
		return FAIL;
	}
	
	int passed = SUCCESS;	// Assume we've passed the test unless otherwise told.
	float baseVal = -1;	// Initialize our base value.
	
	if (strcmp(moveOn, "lat") == 0)
		baseVal = lat;
	else if (strcmp(moveOn, "long") == 0)
		baseVal = lon;
	else if (strcmp(moveOn, "depth") == 0)
		baseVal = depth;
	
	for (; fabs(baseVal) <= fabs(moveTo); baseVal += increment) {
		
		if (strcmp(moveOn, "lat") == 0) {
			pnt.coord[0] = convertTo10P4(lon);
			pnt.coord[1] = convertTo10P4(baseVal);
			pnt.coord[2] = convertTo10P4(depth);
		} else if (strcmp(moveOn, "long") == 0) {
			pnt.coord[0] = convertTo10P4(baseVal);
			pnt.coord[1] = convertTo10P4(lat);
			pnt.coord[2] = convertTo10P4(depth);
		} else if (strcmp(moveOn, "depth") == 0) {
			pnt.coord[0] = convertTo10P4(lon);
			pnt.coord[1] = convertTo10P4(lat);
			pnt.coord[2] = convertTo10P4(baseVal);
		}

		cvmsi_query(&pnt, &data, 1);
		
		fscanf(theFile, "%u %u %u %lf %lf %lf %lf %lf", &expectedX, &expectedY, &expectedZ, &expectedVp, 
			   &expectedVs, &expectedRho, &expectedDiffVp, &expectedDiffVs);
		
		// Check that everything pairs up.
		if (expectedX != data.xyz.coord[0] + 1) {
			sprintf(errStr, "%sError! X for (%.2f, %.2f) at depth %.2f is %u, not %u as expected.\n", 
				    errStr, pnt.coord[0], pnt.coord[1], pnt.coord[2], data.xyz.coord[0] + 1, expectedX);
			passed = FAIL;
		}
		if (expectedY != data.xyz.coord[1] + 1) {
			sprintf(errStr, "%sError! Y for (%.2f, %.2f) at depth %.2f is %u, not %u as expected.\n",
				    errStr, pnt.coord[0], pnt.coord[1], pnt.coord[2], data.xyz.coord[1] + 1, expectedY);
			passed = FAIL;
		}
		if (expectedZ != data.xyz.coord[2] + 1) {
			sprintf(errStr, "%sError! Z for (%.2f, %.2f) at depth %.2f is %u, not %u as expected.\n", 
				    errStr, pnt.coord[0], pnt.coord[1], pnt.coord[2], data.xyz.coord[2] + 1, expectedZ);
			passed = FAIL;
		}
		if (expectedVp != convertTo10P4(data.prop.vp)) {
			sprintf(errStr, "%sError! Vp for (%.2f, %.2f) at depth %.2f is %f, not %f as expected.\n", 
				    errStr, pnt.coord[0], pnt.coord[1], pnt.coord[2], convertTo10P4(data.prop.vp), expectedVp);
			passed = FAIL;
		}
		if (expectedVs != convertTo10P4(data.prop.vs)) {
			sprintf(errStr, "%sError! Vs for (%.2f, %.2f) at depth %.2f is %f, not %f as expected.\n", 
				    errStr, pnt.coord[0], pnt.coord[1], pnt.coord[2], convertTo10P4(data.prop.vs), expectedVs);
			passed = FAIL;
		}
		if (expectedRho != convertTo10P4(data.prop.rho)) {
			sprintf(errStr, "%sError! Rho for (%.2f, %.2f) at depth %.2f is %f, not %f as expected.\n", 
				    errStr, pnt.coord[0], pnt.coord[1], pnt.coord[2], convertTo10P4(data.prop.rho), expectedRho);
			passed = FAIL;
		}
		if (expectedDiffVp != convertTo10P4(data.prop.diff_vp)) {
			sprintf(errStr, "%sError! Vp perturbation for (%.2f, %.2f) at depth %.2f is %f, not %f as expected.\n", 
				    errStr, pnt.coord[0], pnt.coord[1], pnt.coord[2], convertTo10P4(data.prop.diff_vp), expectedDiffVp);
			passed = FAIL;
		}
		if (expectedDiffVs != convertTo10P4(data.prop.diff_vs)) {
			sprintf(errStr, "%sError! Vs perturbation for (%.2f, %.2f) at depth %.2f is %f, not %f as expected.\n", 
				    errStr, pnt.coord[0], pnt.coord[1], pnt.coord[2], convertTo10P4(data.prop.diff_vs), expectedDiffVs);
			passed = FAIL;
		}
	}
	
	return passed;
}

/* 
 *	Main method 
 */
int main(int argc, char **argv) {
	char version[128];
	char errStr[4096];
	printf("\nStarting Acceptance Tests\n");
	cvmsi_init("../model/i26");
	cvmsi_version(version, sizeof(version));
	printf("Version ID: %s\nNumber of tests: 3\n\n", version);
	
	printf("%-40s", "Starting Test 1 (change lat): ");
	
	// Test one. Takes a row from (-118, 34) to (-118, 35)
	int result = runTest(1, -118, 34, 0, "lat", 35, 0.1, errStr);
	int didFail = 0;
	
	if (result == SUCCESS) 
		printf("[PASSED]\n");
	else
		printf("[FAILED]\n%s", errStr);

	if (result == FAIL) didFail = 1;
	
	printf("%-40s", "Starting Test 2 (change long): ");
	
	// Test two. Takes a row from (-118, 35) to (-117, 35).
	result = runTest(2, -117, 35, 20000, "long", -118, -0.5, errStr);
	
	if (result == SUCCESS) 
		printf("[PASSED]\n");
	else
		printf("[FAILED]\n%s", errStr);	
	
	if (result == FAIL) didFail = 1;

	printf("%-40s", "Starting Test 3 (boundary test): ");
	
	// Test three. Goes out of bounds. Ensures that we get CVM-S data only.
	result = runTest(3, -120, 34, 500, "long", -122, -1, errStr);
	
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
		printf("Acceptance tests were successful!\n");

	
	cvmsi_finalize();
	
	printf("\n");
	
	return didFail;
}
