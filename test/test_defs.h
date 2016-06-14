#ifndef TEST_DEFS_H
#define TEST_DEFS_H

#include <time.h>
#include <stdio.h>
#include "ucvm.h"

#define boolean int
#define True 1
#define False 0

#define MAX_POINTS 4000000
#define MAX_TEST_NAME 64
#define MAX_STRING_LEN 256

/* Test datatype */
typedef struct test_info_t {
  char class_name[MAX_TEST_NAME];
  char test_name[MAX_TEST_NAME];
  int (*test_func)();
  int result;
  double elapsed_time;
} test_info_t;


/* Suite datatype */
typedef struct test_suite_t {
  char suite_name[MAX_TEST_NAME];
  int num_tests;
  test_info_t *tests;
  time_t exec_time;
} test_suite_t;


/* Assertions of equality */
int test_assert_int(int val1, int val2);
int test_assert_float(float val1, float val2);
int test_assert_double(double val1, double val2);
int test_assert_string(const char* val1, const char* val2);
int test_assert_file(const char *file1, const char *file2);

/* Get time */
int test_get_time(time_t *ts);

/* Test execution */
int test_run_suite(test_suite_t *suite);

/* XML formatted logfiles */
FILE *init_log(const char *logfile);
int close_log(FILE *lf);
int write_log(FILE *lf, test_suite_t *suite);

/* Read points from text file */
int read_points(const char *file, int max, ucvm_point_t *pnts, int *nn);

/* Read data values from text file */
int read_data(const char *file, int max, ucvm_data_t *data, int *nn);

/* UCVM query tool */
int run_ucvm_query(const char *bindir, const char *conf,
		   const char *model, const char *map,
		   const char *infile, const char *outfile);

/* Fill model structure with test model */
int get_test_model(ucvm_model_t *m);

#endif
