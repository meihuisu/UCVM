#ifndef UNITTEST_DEFS_H
#define UNITTEST_DEFS_H

#include <time.h>
#include <stdio.h>

#define boolean int
#define True 1
#define False 0

#define MAX_TEST_NAME 32

/* Location of binaries */
#define BIN_DIR "../src"


/* Location of model files */
#define MODEL_DIR "../model"


/* Test  datatype */
typedef struct test_t {
  char class_name[MAX_TEST_NAME];
  char test_name[MAX_TEST_NAME];
  int (*test_func)();
  int result;
  double elapsed_time;
} test_t;


/* Suite datatype */
typedef struct suite_t {
  char suite_name[MAX_TEST_NAME];
  int num_tests;
  test_t *tests;
  time_t exec_time;
} suite_t;


/* Assertions of equality */
int test_assert_int(int val1, int val2);
int test_assert_float(float val1, float val2);
int test_assert_double(double val1, double val2);
int test_assert_file(const char *file1, const char *file2);

/* Get time */
int test_get_time(time_t *ts);

/* Test execution */
int test_run_suite(suite_t *suite);

/* XML formatted logfiles */
FILE *init_log(const char *logfile);
int close_log(FILE *lf);
int write_log(FILE *lf, suite_t *suite);

#endif
