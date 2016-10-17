#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <getopt.h>
#include "vx_sub.h"
#include "unittest_defs.h"
#include "test_helper.h"
#include "test_vx_sub.h"


int test_setup()
{
  printf("Test: vx_setup() and vx_cleanup()\n");

  if (test_assert_int(vx_setup(MODEL_DIR), 0) != 0) {
    return(1);
  }

  if (test_assert_int(vx_cleanup(), 0) != 0) {
    return(1);
  }

  printf("PASS\n");
  return(0);
}


int test_init_entry()
{
  vx_entry_t entry;
  int j;

  printf("Test: vx_init_entry()\n");

  memset(&entry, 1, sizeof(vx_entry_t));

  vx_init_entry(&entry);

  for(j = 0; j < 2; j++) {
    if (test_assert_double(entry.coor_utm[j], PLACEHOLDER) != 0) {
      return(1);
    }
    if (test_assert_float(entry.elev_cell[j], PLACEHOLDER) != 0) {
      return(1);
    }
    if (test_assert_float(entry.vel_cell[j], PLACEHOLDER) != 0) {
      return(1);
    }
  }

  if (test_assert_double(entry.coor_utm[2], PLACEHOLDER) != 0) {
    return(1);
  }
  if (test_assert_float(entry.elev_cell[2], PLACEHOLDER) != 0) {
    return(1);
  }

  if (test_assert_float(entry.topo, PLACEHOLDER) != 0) {
    return(1);
  }
  if (test_assert_float(entry.mtop, PLACEHOLDER) != 0) {
    return(1);
  }
  if (test_assert_float(entry.base, PLACEHOLDER) != 0) {
    return(1);
  }
  if (test_assert_float(entry.moho, PLACEHOLDER) != 0) {
    return(1);
  }
  if (test_assert_float(entry.provenance, PLACEHOLDER) != 0) {
    return(1);
  }
  if (test_assert_float(entry.vp, PLACEHOLDER) != 0) {
    return(1);
  }
  if (test_assert_float(entry.vs, PLACEHOLDER) != 0) {
    return(1);
  }
  if (test_assert_double(entry.rho, PLACEHOLDER) != 0) {
    return(1);
  }
  if (test_assert_int(entry.data_src, VX_SRC_NR) != 0) {
    return(1);
  }

  printf("PASS\n");
  return(0);
}


int test_init_voxel()
{
  vx_voxel_t voxel;
  int j;

  printf("Test: vx_init_voxel()\n");

  memset(&voxel, 1, sizeof(vx_voxel_t));

  vx_init_voxel(&voxel);

  for(j = 0; j < 2; j++) {
    if (test_assert_float(voxel.elev_cell[j], PLACEHOLDER) != 0) {
      return(1);
    }
    if (test_assert_float(voxel.vel_cell[j], PLACEHOLDER) != 0) {
      return(1);
    }
  }

  if (test_assert_float(voxel.elev_cell[2], PLACEHOLDER) != 0) {
    return(1);
  }

  if (test_assert_float(voxel.topo, PLACEHOLDER) != 0) {
    return(1);
  }
  if (test_assert_float(voxel.mtop, PLACEHOLDER) != 0) {
    return(1);
  }
  if (test_assert_float(voxel.base, PLACEHOLDER) != 0) {
    return(1);
  }
  if (test_assert_float(voxel.moho, PLACEHOLDER) != 0) {
    return(1);
  }
  if (test_assert_float(voxel.provenance, PLACEHOLDER) != 0) {
    return(1);
  }
  if (test_assert_float(voxel.vp, PLACEHOLDER) != 0) {
    return(1);
  }
  if (test_assert_float(voxel.vs, PLACEHOLDER) != 0) {
    return(1);
  }
  if (test_assert_double(voxel.rho, PLACEHOLDER) != 0) {
    return(1);
  }

  printf("PASS\n");
  return(0);
}


int test_getsurface()
{
  int i;
  vx_entry_t entry;
  float surf;

  double x[MAX_TEST_POINTS], y[MAX_TEST_POINTS], z[MAX_TEST_POINTS];
  vx_coord_t coord_types[MAX_TEST_POINTS];
  float surf_values[MAX_TEST_POINTS];

  printf("Test: vx_getsurface()\n");

  get_test_points(x, y, z, coord_types);
  get_surf_values(surf_values);

  if (test_assert_int(vx_setup(MODEL_DIR), 0) != 0) {
    return(1);
  }

  vx_register_bkg(NULL);

  for (i = 0; i < MAX_TEST_POINTS; i++) {
    entry.coor[0] = x[i];
    entry.coor[1] = y[i];
    entry.coor[2] = z[i];
    vx_getsurface(entry.coor, coord_types[i], &surf);
    if (test_assert_float(surf, surf_values[i]) != 0) {
      return(1);
    }
  }

  if (test_assert_int(vx_cleanup(), 0) != 0) {
    return(1);
  }

  printf("PASS\n");
  return(0);
}


int test_query_no_1d()
{
  int i;
  vx_entry_t entry;

  double x[MAX_TEST_POINTS], y[MAX_TEST_POINTS], z[MAX_TEST_POINTS];
  vx_coord_t coord_types[MAX_TEST_POINTS];
  float vp_values[MAX_TEST_POINTS], vs_values[MAX_TEST_POINTS];
  double rho_values[MAX_TEST_POINTS];

  printf("Test: vx_getcoord() w/o SCEC 1D\n");

  get_test_points(x, y, z, coord_types);
  get_mat_props(vp_values, vs_values, rho_values, VX_TEST_DATASET_NOBKG);

  if (test_assert_int(vx_setup(MODEL_DIR), 0) != 0) {
    return(1);
  }

  vx_register_bkg(NULL);

  for (i = 0; i < MAX_TEST_POINTS; i++) {
    entry.coor[0] = x[i];
    entry.coor[1] = y[i];
    entry.coor[2] = z[i];
    entry.coor_type = coord_types[i];
    vx_getcoord(&entry);
    if (test_assert_float(entry.vp, vp_values[i]) != 0) {
      return(1);
    }
    if (test_assert_float(entry.vs, vs_values[i]) != 0) {
      return(1);
    }
    if (test_assert_float(entry.rho, rho_values[i]) != 0) {
      return(1);
    }
  }

  if (test_assert_int(vx_cleanup(), 0) != 0) {
    return(1);
  }

  printf("PASS\n");
  return(0);
}


int test_query_1d()
{
  int i;
  vx_entry_t entry;

  double x[MAX_TEST_POINTS], y[MAX_TEST_POINTS], z[MAX_TEST_POINTS];
  vx_coord_t coord_types[MAX_TEST_POINTS];
  float vp_values[MAX_TEST_POINTS], vs_values[MAX_TEST_POINTS];
  double rho_values[MAX_TEST_POINTS];

  printf("Test: vx_getcoord() w/ SCEC 1D\n");

  get_test_points(x, y, z, coord_types);
  get_mat_props(vp_values, vs_values, rho_values, VX_TEST_DATASET_BKG);

  if (test_assert_int(vx_setup(MODEL_DIR), 0) != 0) {
    return(1);
  }

  vx_register_scec();

  for (i = 0; i < MAX_TEST_POINTS; i++) {
    entry.coor[0] = x[i];
    entry.coor[1] = y[i];
    entry.coor[2] = z[i];
    entry.coor_type = coord_types[i];
    vx_getcoord(&entry);
    if (test_assert_float(entry.vp, vp_values[i]) != 0) {
      return(1);
    }
    if (test_assert_float(entry.vs, vs_values[i]) != 0) {
      return(1);
    }
    if (test_assert_double(entry.rho, rho_values[i]) != 0) {
      return(1);
    }
  }

  if (test_assert_int(vx_cleanup(), 0) != 0) {
    return(1);
  }

  printf("PASS\n");
  return(0);
}


int test_register_bkg()
{
  vx_entry_t entry;

  printf("Test: vx_register_bkg()\n");

  if (test_assert_int(vx_setup(MODEL_DIR), 0) != 0) {
    return(1);
  }

  vx_register_bkg(vx_test_bkg);

  entry.coor[0] = -118.0;
  entry.coor[1] = 37.5;
  entry.coor[2] = -500.0;
  entry.coor_type = VX_COORD_GEO;
  vx_getcoord(&entry);
  if (test_assert_float(entry.vp, 1234.5) != 0) {
    return(1);
  }
  if (test_assert_float(entry.vs, 2345.6) != 0) {
    return(1);
  }
  if (test_assert_double(entry.rho, 3456.7) != 0) {
    return(1);
  }
  
  if (test_assert_int(vx_cleanup(), 0) != 0) {
    return(1);
  }

  printf("PASS\n");
  return(0);
}


int test_query_nogtl()
{
  int i;
  vx_entry_t entry;

  double x[MAX_TEST_POINTS], y[MAX_TEST_POINTS], z[MAX_TEST_POINTS];
  vx_coord_t coord_types[MAX_TEST_POINTS];
  float vp_values[MAX_TEST_POINTS], vs_values[MAX_TEST_POINTS];
  double rho_values[MAX_TEST_POINTS];

  printf("Test: vx_getcoord() w/ no GTL\n");

  get_test_points(x, y, z, coord_types);
  get_mat_props(vp_values, vs_values, rho_values, VX_TEST_DATASET_NOGTL);

  if (test_assert_int(vx_setup(MODEL_DIR), 0) != 0) {
    return(1);
  }

  vx_setgtl(False);

  /* Test 8 standard points */
  for (i = 0; i < MAX_TEST_POINTS; i++) {
    entry.coor[0] = x[i];
    entry.coor[1] = y[i];
    entry.coor[2] = z[i];
    entry.coor_type = coord_types[i];
    vx_getcoord(&entry);
    if (test_assert_float(entry.vp, vp_values[i]) != 0) {
      return(1);
    }
    if (test_assert_float(entry.vs, vs_values[i]) != 0) {
      return(1);
    }
    if (test_assert_double(entry.rho, rho_values[i]) != 0) {
      return(1);
    }
  }

  /* Test near surface point */
  entry.coor[0] = -118.0;
  entry.coor[1] = 34.0;
  entry.coor[2] = 200.0;
  entry.coor_type = VX_COORD_GEO;
  vx_getcoord(&entry);
  if (test_assert_float(entry.vp, 2484.39) != 0) {
    return(1);
  }
  if (test_assert_float(entry.vs, 969.30) != 0) {
    return(1);
  }
  if (test_assert_double(entry.rho, 2088.32) != 0) {
    return(1);
  }

  if (test_assert_int(vx_cleanup(), 0) != 0) {
    return(1);
  }

  printf("PASS\n");
  return(0);
}


int suite_vx_sub(const char *xmldir)
{
  suite_t suite;
  char logfile[256];
  FILE *lf = NULL;

  /* Setup test suite */
  strcpy(suite.suite_name, "suite_vx_sub");
  suite.num_tests = 8;
  suite.tests = malloc(suite.num_tests * sizeof(test_t));
  if (suite.tests == NULL) {
    fprintf(stderr, "Failed to alloc test structure\n");
    return(1);
  }
  test_get_time(&suite.exec_time);

  /* Setup test cases */
  strcpy(suite.tests[0].test_name, "test_setup()");
  suite.tests[0].test_func = &test_setup;
  suite.tests[0].elapsed_time = 0.0;

  strcpy(suite.tests[1].test_name, "test_init_entry()");
  suite.tests[1].test_func = &test_init_entry;
  suite.tests[1].elapsed_time = 0.0;

  strcpy(suite.tests[2].test_name, "test_init_voxel()");
  suite.tests[2].test_func = &test_init_voxel;
  suite.tests[2].elapsed_time = 0.0;

  strcpy(suite.tests[3].test_name, "test_getsurface()");
  suite.tests[3].test_func = &test_getsurface;
  suite.tests[3].elapsed_time = 0.0;

  strcpy(suite.tests[4].test_name, "test_query_no_1d()");
  suite.tests[4].test_func = &test_query_no_1d;
  suite.tests[4].elapsed_time = 0.0;

  strcpy(suite.tests[5].test_name, "test_query_1d()");
  suite.tests[5].test_func = &test_query_1d;
  suite.tests[5].elapsed_time = 0.0;

  strcpy(suite.tests[6].test_name, "test_register_bkg()");
  suite.tests[6].test_func = &test_register_bkg;
  suite.tests[6].elapsed_time = 0.0;

  strcpy(suite.tests[7].test_name, "test_setgtl()");
  suite.tests[7].test_func = &test_query_nogtl;
  suite.tests[7].elapsed_time = 0.0;

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
