#define _BSD_SOURCE  /* Required for gethostname */ 
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "unittest_defs.h"


int test_assert_int(int val1, int val2)
{
  if (val1 != val2) {
    fprintf(stderr, "FAIL: assertion %d != %d\n", val1, val2);
    return(1);
  }
  return(0);
}

int test_assert_float(float val1, float val2)
{
  if (abs(val1 - val2) > 0.01) {
    fprintf(stderr, "FAIL: assertion %f != %f\n", val1, val2);
    return(1);
  }
  return(0);
}

int test_assert_double(double val1, double val2)
{
  if (abs(val1 - val2) > 0.01) {
    fprintf(stderr, "FAIL: assertion %lf != %lf\n", val1, val2);
    return(1);
  }
  return(0);
}


int test_assert_file(const char *file1, const char *file2)
{
  FILE *fp1, *fp2;
  char line1[128], line2[128];

  fp1 = fopen(file1, "r");
  fp2 = fopen(file2, "r");
  if ((fp1 == NULL) || (fp2 == NULL)) {
    printf("FAIL: unable to open %s and/or %s\n", file1, file2);
    return(1);
  }
  while ((!feof(fp1)) && (!feof(fp2))) {
    memset(line1, 0, 128);
    memset(line2, 0, 128);
    fread(line1, 1, 127, fp1);
    fread(line2, 1, 127, fp2);
    if (test_assert_int(strcmp(line1, line2), 0) != 0) {
      return(1);
    }
  }
  if ((!feof(fp1)) || (!feof(fp2))) {
    printf("FAIL: %s and %s are of unequal length\n", file1, file2);
    return(1);
  }

  return(0);
}


/* Get time */
int test_get_time(time_t *ts)
{
  time(ts);
  return(0);
}



/* Test execution */
int test_run_suite(suite_t *suite)
{
  struct timeval start, end;

  int i;

  for (i = 0; i < suite->num_tests; i++) {
    gettimeofday(&start,NULL);
    if ((suite->tests[i].test_func)() != 0) {
      suite->tests[i].result = 1;
    } else {
      suite->tests[i].result = 0;
    }
    gettimeofday(&end,NULL);
    suite->tests[i].elapsed_time = (end.tv_sec - start.tv_sec) * 1.0 +
      (end.tv_usec - start.tv_usec) / 1000000.0;

  }

  return(0);
}


/* XML formatted logfiles */
FILE *init_log(const char *logfile)
{
  char line[256];
  FILE *lf;

  lf = fopen(logfile, "w");
  if (lf == NULL) {
    fprintf(stderr, "Failed to initialize logfile %s\n", logfile);
    return(NULL);
  }

  strcpy(line, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
  fwrite(line, 1, strlen(line), lf);

  return(lf);
}


int close_log(FILE *lf)
{
  if (lf != NULL) {
    fclose(lf);
  }
  return(0);
}


int write_log(FILE *lf, suite_t *suite)
{
  char hostname[256];
  char datestr[256];
  char line[256];
  int i;
  int num_fail = 0;
  double suite_elapsed = 0.0;
  struct tm *tmp;
  
  for (i = 0; i < suite->num_tests; i++) {
    if (suite->tests[i].result != 0) {
      num_fail++;
    }
    suite_elapsed = suite_elapsed + suite->tests[i].elapsed_time;
  }

  /* Get host name */
  if (gethostname(hostname, (size_t)256) != 0) {
    return(1);
  }

  /* Get timestamp */
  tmp = localtime(&(suite->exec_time));
  if (tmp == NULL) {
    fprintf(stderr, "Failed to retrieve time\n");
    return(1);
  }
  if (strftime(datestr, 256, "%Y-%m-%dT%H:%M:%S", tmp) == 0) {
    return(1);
  }

  if (lf != NULL) {
    sprintf(line, "<testsuite errors=\"0\" failures=\"%d\" hostname=\"%s\" name=\"%s\" tests=\"%d\" time=\"%lf\" timestamp=\"%s\">\n", num_fail, hostname, suite->suite_name, suite->num_tests, suite_elapsed, datestr);
    fwrite(line, 1, strlen(line), lf);

    for (i = 0; i < suite->num_tests; i++) {
      sprintf(line, "  <testcase classname=\"C func\" name=\"%s\" time=\"%lf\">\n",
	      suite->tests[i].test_name, suite->tests[i].elapsed_time);
      fwrite(line, 1, strlen(line), lf);

      if (suite->tests[i].result != 0) {
	sprintf(line, " <failure message=\"fail\" type=\"test failed\">test case FAIL</failure>\n");
	fwrite(line, 1, strlen(line), lf);
      }

      sprintf(line, "  </testcase>\n");
      fwrite(line, 1, strlen(line), lf);
    }

    sprintf(line, "</testsuite>\n");
    fwrite(line, 1, strlen(line), lf);

  }

  return(0);
}

