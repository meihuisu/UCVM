#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <getopt.h>
#include "unittest_defs.h"
#include "test_helper.h"
#include "test_grid.h"


int test_vx_grid()
{
  char infile[128];
  char outfile[128];
  char reffile[128];
  char currentdir[128];

  printf("Test: vx executable w/ large grid\n");

  /* Save current directory */
  getcwd(currentdir, 128);

  sprintf(infile, "%s/%s", currentdir, "./inputs/test-grid.in");
  sprintf(outfile, "%s/%s", currentdir, "test-vx-grid-extract.out");
  sprintf(reffile, "%s/%s", currentdir, "./ref/test-extract.ref");

  if (test_assert_int(runVX(BIN_DIR, MODEL_DIR,infile, outfile), 0) != 0) {
    printf("vx failure\n"); 
    return(1);
  }

  /* Perform diff btw outfile and ref */
  if (test_assert_file(outfile, reffile) != 0) {
    return(1);
  }

  unlink(outfile);

  printf("PASS\n");
  return(0);
}



int test_vx_lite_grid_emul()
{
  char infile[128];
  char outfile[128];
  char reffile[128];
  char currentdir[128];

  printf("Test: vx_lite executable w/ large grid in emulation mode\n");

  /* Save current directory */
  getcwd(currentdir, 128);

  sprintf(infile, "%s/%s", currentdir, "./inputs/test-grid.in");
  sprintf(outfile, "%s/%s", currentdir, "test-vx-lite-grid-extract-emul.out");
  sprintf(reffile, "%s/%s", currentdir, "./ref/test-extract-vxlite.ref");

  if (test_assert_int(runVXLite(BIN_DIR, MODEL_DIR, infile, outfile, 
				MODE_EMUL), 0) != 0) {
    printf("vx_lite failure\n");
    return(1);
  }  

  /* Perform diff btw outfile and ref */
  if (test_assert_file(outfile, reffile) != 0) {
    return(1);
  }

  unlink(outfile);

  printf("PASS\n");
  return(0);
}


int test_vx_lite_grid_depth()
{
  char infile[128];
  char outfile[128];
  char reffile[128];
  char currentdir[128];

  printf("Test: vx_lite executable w/ large grid in depth mode\n");

  /* Save current directory */
  getcwd(currentdir, 128);

  sprintf(infile, "%s/%s", currentdir, "./inputs/test-grid-depth.in");
  sprintf(outfile, "%s/%s", currentdir, "test-vx-lite-grid-extract-depth.out");
  sprintf(reffile, "%s/%s", currentdir, "./ref/test-extract-vxlite-depth.ref");

  if (test_assert_int(runVXLite(BIN_DIR, MODEL_DIR, infile, outfile, 
				MODE_DEPTH), 0) != 0) {
    printf("vx_lite failure\n");
    return(1);
  }  

  /* Perform diff btw outfile and ref */
  if (test_assert_file(outfile, reffile) != 0) {
    return(1);
  }

  unlink(outfile);

  printf("PASS\n");
  return(0);
}


int test_vx_lite_grid_offset()
{
  char infile[128];
  char outfile[128];
  char reffile[128];
  char currentdir[128];

  printf("Test: vx_lite executable w/ large grid in elev offset mode\n");

  /* Save current directory */
  getcwd(currentdir, 128);

  sprintf(infile, "%s/%s", currentdir, "./inputs/test-grid-offset.in");
  sprintf(outfile, "%s/%s", currentdir, 
	  "test-vx-lite-grid-extract-offset.out");
  /* Use same reference file as depth */
  sprintf(reffile, "%s/%s", currentdir, 
	  "./ref/test-extract-vxlite-offset.ref");

  if (test_assert_int(runVXLite(BIN_DIR, MODEL_DIR, infile, outfile, 
				MODE_NONE), 0) != 0) {
    printf("vx_lite failure\n");
    return(1);
  }  

  /* Perform diff btw outfile and ref */
  if (test_assert_file(outfile, reffile) != 0) {
    return(1);
  }

  unlink(outfile);

  printf("PASS\n");
  return(0);
}


int suite_grid(const char *xmldir)
{
  suite_t suite;
  char logfile[256];
  FILE *lf = NULL;

  /* Setup test suite */
  strcpy(suite.suite_name, "suite_grid");
  suite.num_tests = 4;
  suite.tests = malloc(suite.num_tests * sizeof(test_t));
  if (suite.tests == NULL) {
    fprintf(stderr, "Failed to alloc test structure\n");
    return(1);
  }
  test_get_time(&suite.exec_time);

  /* Setup test cases */
  strcpy(suite.tests[0].test_name, "test_vx_grid");
  suite.tests[0].test_func = &test_vx_grid;
  suite.tests[0].elapsed_time = 0.0;

  strcpy(suite.tests[1].test_name, "test_vx_lite_grid_emul");
  suite.tests[1].test_func = &test_vx_lite_grid_emul;
  suite.tests[1].elapsed_time = 0.0;

  strcpy(suite.tests[2].test_name, "test_vx_lite_grid_depth");
  suite.tests[2].test_func = &test_vx_lite_grid_depth;
  suite.tests[2].elapsed_time = 0.0;

  strcpy(suite.tests[3].test_name, "test_vx_lite_grid_offset");
  suite.tests[3].test_func = &test_vx_lite_grid_offset;
  suite.tests[3].elapsed_time = 0.0;

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
