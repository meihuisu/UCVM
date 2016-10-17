#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <getopt.h>
#include "vx_sub.h"
#include "unittest_defs.h"
#include "test_helper.h"
#include "test_vx_lite_exec.h"


int test_vx_lite_points_emulation()
{
  char infile[128];
  char outfile[128];
  char reffile[128];
  char currentdir[128];

  printf("Test: vx_lite executable w/ emulation option\n");

  /* Save current directory */
  getcwd(currentdir, 128);

  sprintf(infile, "%s/%s", currentdir, "./inputs/test.in");
  sprintf(outfile, "%s/%s", currentdir, 
	  "test-8-point-vx-lite-extract-elev.out");
  sprintf(reffile, "%s/%s", currentdir, 
	  "./ref/test-8-point-vx-extract-elev.ref");

  if (test_assert_int(save_test_points(infile), 0) != 0) {
    return(1);
  }

  if (test_assert_int(runVXLite(BIN_DIR, MODEL_DIR, infile, outfile, 
				MODE_EMUL), 0) != 0) {
    printf("vx_lite failure\n");
    return(1);
  }

  /* Perform diff btw outfile and ref */
  if (test_assert_file(outfile, reffile) != 0) {
    return(1);
  }

  unlink(infile);
  unlink(outfile);

  printf("PASS\n");
  return(0);
}



int test_vx_lite_points_depth()
{
  char infile[128];
  char outfile[128];
  char reffile[128];
  char currentdir[128];

  printf("Test: vx_lite executable w/ depth option\n");

  /* Save current directory */
  getcwd(currentdir, 128);

  sprintf(infile, "%s/%s", currentdir, "./inputs/test.in");
  sprintf(outfile, "%s/%s", currentdir, 
	  "test-8-point-vx-lite-extract-depth.out");
  sprintf(reffile, "%s/%s", currentdir, 
	  "./ref/test-8-point-vx-extract-depth.ref");

  if (test_assert_int(save_test_points(infile), 0) != 0) {
    printf("save test point failure\n");
    return(1);
  }

  if (test_assert_int(runVXLite(BIN_DIR, MODEL_DIR, infile, outfile, 
				MODE_DEPTH), 0) != 0) {
    printf("vx_lite failure\n");
    return(1);
  }  

  /* Perform diff btw outfile and ref */
  if (test_assert_file(outfile, reffile) != 0) {
    printf("diff failure\n");
    return(1);
  }

  unlink(infile);
  unlink(outfile);

  printf("PASS\n");
  return(0);
}


int test_vx_lite_points_scec()
{
  char infile[128];
  char outfile[128];
  char reffile[128];
  char currentdir[128];

  printf("Test: vx_lite executable w/ SCEC 1D bkg option\n");

  /* Save current directory */
  getcwd(currentdir, 128);

  sprintf(infile, "%s/%s", currentdir, "./inputs/test.in");
  sprintf(outfile, "%s/%s", currentdir, "test-8-point-vx-lite-extract-scec.out");
  sprintf(reffile, "%s/%s", currentdir, 
	  "./ref/test-8-point-vx-extract-scec.ref");

  if (test_assert_int(save_test_points(infile), 0) != 0) {
    printf("save test point failure\n");
    return(1);
  }

  if (test_assert_int(runVXLite(BIN_DIR, MODEL_DIR, infile, outfile, 
				MODE_SCEC), 0) != 0) {
    printf("vx_lite failure\n");
    return(1);
  }

  /* Perform diff btw outfile and ref */
  if (test_assert_file(outfile, reffile) != 0) {
    printf("diff failure\n");
    return(1);
  }

  unlink(infile);
  unlink(outfile);

  printf("PASS\n");
  return(0);
}



int suite_vx_lite_exec(const char *xmldir)
{
  suite_t suite;
  char logfile[256];
  FILE *lf = NULL;

  /* Setup test suite */
  strcpy(suite.suite_name, "suite_vx_lite_exec");
  suite.num_tests = 3;
  suite.tests = malloc(suite.num_tests * sizeof(test_t));
  if (suite.tests == NULL) {
    fprintf(stderr, "Failed to alloc test structure\n");
    return(1);
  }
  test_get_time(&suite.exec_time);

  /* Setup test cases */
  strcpy(suite.tests[0].test_name, "test_vx_lite_points_emulation");
  suite.tests[0].test_func = &test_vx_lite_points_emulation;
  suite.tests[0].elapsed_time = 0.0;

  strcpy(suite.tests[1].test_name, "test_vx_lite_points_depth");
  suite.tests[1].test_func = &test_vx_lite_points_depth;
  suite.tests[1].elapsed_time = 0.0;

  strcpy(suite.tests[2].test_name, "test_vx_lite_points_scec");
  suite.tests[2].test_func = &test_vx_lite_points_scec;
  suite.tests[2].elapsed_time = 0.0;

  if (test_run_suite(&suite) != 0) {
    fprintf(stderr, "Failed to execute tests\n");
    return(1);
  }

  if (xmldir != NULL) {
    sprintf(logfile, "%s/%s.xml", xmldir, suite.suite_name);
    lf = init_log(logfile);
    if (lf == NULL) {
      fprintf(stderr, "Failed to initialize logfile\n");
      return(1);
    }
    
    if (write_log(lf, &suite) != 0) {
      fprintf(stderr, "Failed to write test log\n");
      return(1);
    }
    
    close_log(lf);
  }

  free(suite.tests);

  return 0;
}
