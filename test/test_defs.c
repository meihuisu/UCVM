#define _BSD_SOURCE /* Needed got gethostname with std=c99 */
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <stdio.h>
#include "test_defs.h"

/* Test model ID */
int test_id;

/* Assert two integers are equal */
int test_assert_int(int val1, int val2)
{
  if (val1 != val2) {
    fprintf(stderr, "FAIL: assertion %d != %d\n", val1, val2);
    return(1);
  }
  return(0);
}


/* Assert two floats are approximately equal */
int test_assert_float(float val1, float val2)
{
  if (abs(val1 - val2) > 0.01) {
    fprintf(stderr, "FAIL: assertion %f != %f\n", val1, val2);
    return(1);
  }
  return(0);
}


/* Assert two doubles are approximately equal */
int test_assert_double(double val1, double val2)
{
  if (abs(val1 - val2) > 0.01) {
    fprintf(stderr, "FAIL: assertion %lf != %lf\n", val1, val2);
    return(1);
  }
  return(0);
}


/* Assert two strings are identical */
int test_assert_string(const char* val1, const char * val2)
{
  if (strcmp(val1, val2) != 0) {
    fprintf(stderr, "FAIL: assertion %s != %s\n", val1, val2);
    return(1);
  }
  return(0);
}


/* Assert two text files are identical */
int test_assert_file(const char *file1, const char *file2)
{
  FILE *fp1, *fp2;
  char line1[MAX_STRING_LEN], line2[MAX_STRING_LEN];

  fp1 = fopen(file1, "r");
  fp2 = fopen(file2, "r");
  if ((fp1 == NULL) || (fp2 == NULL)) {
    printf("FAIL: unable to open %s and/or %s\n", file1, file2);
    return(1);
  }
  while ((!feof(fp1)) && (!feof(fp2))) {
    memset(line1, 0, MAX_STRING_LEN);
    memset(line2, 0, MAX_STRING_LEN);
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
int test_run_suite(test_suite_t *suite)
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


/* Initialize XML formatted logfiles */
FILE *init_log(const char *logfile)
{
  char line[MAX_STRING_LEN];
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


/* Close the log */
int close_log(FILE *lf)
{
  if (lf != NULL) {
    fclose(lf);
  }
  return(0);
}


/* Write suite test results to the log */
int write_log(FILE *lf, test_suite_t *suite)
{
  char hostname[MAX_STRING_LEN];
  char datestr[MAX_STRING_LEN];
  char line[MAX_STRING_LEN];
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
  if (gethostname(hostname, (size_t)MAX_STRING_LEN) != 0) {
    return(1);
  }

  /* Get timestamp */
  tmp = localtime(&(suite->exec_time));
  if (tmp == NULL) {
    fprintf(stderr, "Failed to retrieve time\n");
    return(1);
  }
  if (strftime(datestr, MAX_STRING_LEN, "%Y-%m-%dT%H:%M:%S", tmp) == 0) {
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


/* Read points */
int read_points(const char *file, int max, 
		ucvm_point_t *pnts, int *nn)
{
  FILE *fp;
  int i;

  *nn = 0;
  fp = fopen(file, "r");
  if (fp == NULL) {
    return(1);
  }

  i = 0;
  while ((i < max) && (!feof(fp))) {
    if (fscanf(fp, "%lf %lf %lf", &(pnts[i].coord[0]), 
	       &(pnts[i].coord[1]), &(pnts[i].coord[2])) != 3) {
      if (!feof(fp)) {
	fprintf(stderr, "Failed to read line %d from %s\n", i, file);
	fclose(fp);
	return(1);
      } else {
	break;
      }
    }
    i++;
  }
  fclose(fp);
  *nn = i;
  return(0);
}


/* Read data values */
int read_data(const char *file, int max, 
	      ucvm_data_t *data, int *nn)
{
  FILE *fp;
  int i;

  *nn = 0;
  fp = fopen(file, "r");
  if (fp == NULL) {
    return(1);
  }

  i = 0;
  while ((i < max) && (!feof(fp))) {
    if (fscanf(fp, "%*f %*f %*f %lf %lf %*s %*f %*f %*f %*s %*f %*f %*f %*s %lf %lf %lf", 
	       &(data[i].surf), &(data[i].vs30),
	       &(data[i].cmb.vp), &(data[i].cmb.vs), 
	       &(data[i].cmb.rho)) != 5) {
      if (!feof(fp)) {
	fprintf(stderr, "Failed to read line %d from %s\n", i, file);
	fclose(fp);
	return(1);
      } else {
	break;
      }
    }
    i++;
  }
  fclose(fp);
  *nn = i;
  return(0);
}


/* Run UCVM query tool */
int run_ucvm_query(const char *bindir, const char *conf, 
		   const char *model, const char *map,
		   const char *infile, const char *outfile)
{
  char currentdir[MAX_STRING_LEN];
  char flags[MAX_STRING_LEN];

  /* Save current directory */
  getcwd(currentdir, MAX_STRING_LEN);

  /* Setup flags */
  sprintf(flags, "-m %s -p %s -c %s -f %s", model, map, "gd", conf);

  /* Fork process */
  pid_t pid;
  pid = fork();
  if (pid == -1) {
    perror("fork");
    printf("FAIL: unable to fork\n");
    return(1);
  } else if (pid == 0) {

    /* Change dir to bindir */
    if (strcmp(bindir, ".") != 0) {
      if (chdir(bindir) != 0) {
	printf("FAIL: Error changing dir\n");
	return(1);
      }
    }

    /* Execute command */
    execl( "../src/ucvm/run_ucvm.sh", "../src/ucvm/run_ucvm.sh", 
	   flags, infile, outfile, (char *)0);

    perror("execl"); /* shall never get to here */
    printf("FAIL: UCVM exited abnormally\n");
    return(1);
  } else {
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
      return(0);
    } else {
      printf("FAIL: UCVM exited abnormally\n");
      return(1);
    }
  }

  return(0);
}


/* Init test model */
int test_model_init(int m, ucvm_modelconf_t *conf)
{
  test_id = m;
  return(UCVM_CODE_SUCCESS);
}


/* Finalize test model */
int test_model_finalize()
{
  return(UCVM_CODE_SUCCESS);
}


/* Version test model */
int test_model_version(int id, char *ver, int len)
{
  if (id != test_id) {
    return(UCVM_CODE_ERROR);
  }
  strcpy(ver, "test 1.0");
  return(UCVM_CODE_SUCCESS);
}


/* Label test model */
int test_model_label(int id, char *lab, int len)
{
  if (id != test_id) {
    return(UCVM_CODE_ERROR);
  }
  strcpy(lab, "test");
  return(UCVM_CODE_SUCCESS);
}


/* Setparam test model */
int test_model_setparam(int id, int param, ...)
{
  if (id != test_id) {
    return(UCVM_CODE_ERROR);
  }
  return(UCVM_CODE_SUCCESS);
}


/* Query 1D */
int test_model_query(int id, ucvm_ctype_t cmode,
		     int n, ucvm_point_t *pnt, ucvm_data_t *data)
{
  int i;

  if (id != test_id) {
    return(UCVM_CODE_ERROR);
  }

  for (i = 0; i < n; i++) {
    data[i].crust.source = test_id;
    data[i].crust.vp = 200.0;
    data[i].crust.vs = 100.0;
    data[i].crust.rho = 50.0;
  }

  return(UCVM_CODE_SUCCESS);
}


/* Fill model structure with test model */
int get_test_model(ucvm_model_t *m)
{
  m->mtype = UCVM_MODEL_CRUSTAL;
  m->init = test_model_init;
  m->finalize = test_model_finalize;
  m->setparam = test_model_setparam;
  m->getversion = test_model_version;
  m->getlabel = test_model_label;
  m->query = test_model_query;

  return(UCVM_CODE_SUCCESS);
}
