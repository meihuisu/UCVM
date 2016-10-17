#ifndef TEST_HELPER_H
#define TEST_HELPER_H

#include "vx_sub.h"

/* Constants */
#define MAX_TEST_POINTS 8
#define PLACEHOLDER -99999.0

/* vx_lite modes of operation */
#define MODE_NONE 0
#define MODE_EMUL 2
#define MODE_DEPTH 4
#define MODE_SCEC 8
#define MODE_NOGTL 16

/* Test data sets */
typedef enum { VX_TEST_DATASET_NOBKG = 0, 
	       VX_TEST_DATASET_BKG, 
	       VX_TEST_DATASET_NOGTL } vx_test_dataset_t;


/* Retrieve eight test points */
int get_test_points(double *x, double *y, double *z, 
                    vx_coord_t *coord_types);

/* Retrieve expected surface elev at the test points */
int get_surf_values(float *surf_values);

/* Retrieve expected mat props at the test points */
int get_mat_props(float *vp, float *vs, double *rho, vx_test_dataset_t ds);

/* Save eight test points to file */
int save_test_points(const char* filename);

/* Test bkg/topo handler */
int vx_test_bkg(vx_entry_t *entry, vx_request_t req_type);

/* Execute vx as a child process */
int runVX(const char *bindir, const char *cvmdir, 
	  const char *infile, const char *outfile);

/* Execute vx_lite as a child process */
int runVXLite(const char *bindir, const char *cvmdir, 
	      const char *infile, const char *outfile,
	      int mode);

#endif
