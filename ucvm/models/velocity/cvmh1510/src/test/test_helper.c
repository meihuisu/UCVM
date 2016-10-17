#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "unittest_defs.h"
#include "test_helper.h"


/* Retrieve eight test points */
int get_test_points(double *x, double *y, double *z, 
                    vx_coord_t *coord_types)
{
  x[0] = -125.0;
  y[0] = 35.0;
  z[0] = -7777.0;
  coord_types[0] = VX_COORD_GEO;

  x[1] = -118.56;
  y[1] = 32.55;
  z[1] = -2450.0;
  coord_types[1] = VX_COORD_GEO;

  x[2] = 360061.0;
  y[2] = 3750229.0;
  z[2] = -1400.0;
  coord_types[2] = VX_COORD_UTM;

  x[3] = -118.52;
  y[3] = 34.12;
  z[3] = -1400.0;
  coord_types[3] = VX_COORD_GEO;

  x[4] = -116.40;
  y[4] = 32.34;
  z[4] = -1000.0;
  coord_types[4] = VX_COORD_GEO;

  x[5] = 376592.0;
  y[5] = 3773379.0;
  z[5] = -1700.0;
  coord_types[5] = VX_COORD_UTM;

  x[6] = 376592.0;
  y[6] = 3773379.0;
  z[6] = -17700.0;
  coord_types[6] = VX_COORD_UTM;

  x[7] = 408669.0;
  y[7] = 3766189.0;
  z[7] = -3000.0;
  coord_types[7] = VX_COORD_UTM;

  return(0);
}


int save_test_points(const char* filename)
{
  double x[MAX_TEST_POINTS], y[MAX_TEST_POINTS], z[MAX_TEST_POINTS];
  vx_coord_t coord_types[MAX_TEST_POINTS];
  FILE *fp;
  int i;
  char line[128];
  size_t retval;

  get_test_points(x, y, z, coord_types);

  fp = fopen(filename, "w");
  if (fp == NULL) {
    printf("FAIL: cannot open %s\n", filename);
    return(1);
  }
  for (i = 0; i < MAX_TEST_POINTS; i++) {
    sprintf(line, "%f %f %f\n", x[i], y[i], z[i]);
    retval = fwrite(line, strlen(line), 1, fp);
    if (retval != 1) {
      printf("FAIL: write failed\n");
      return(1);
    }
  }
  fclose(fp);
  return(0);
}


/* Retrieve expected surface elev at the test points */
int get_surf_values(float *surf_values)
{
  surf_values[0] = PLACEHOLDER;
  surf_values[1] = -1114.907715;
  //surf_values[1] = -1150.010010;
  surf_values[2] = -56.939297;
  surf_values[3] = 491.459229;
  //surf_values[3] = 449.989990;
  surf_values[4] = 780.433289;
  //surf_values[4] = 749.989990;
  surf_values[5] = 99.369789;
  surf_values[6] = 99.369789;
  surf_values[7] = 93.892632;
  //surf_values[7] = 49.990002;

  return(0);
}

/* Retrieve expected mat props at the test points */
int get_mat_props(float *vp, float *vs, double *rho, vx_test_dataset_t ds)
{
  switch (ds) {
  case VX_TEST_DATASET_NOBKG:
  case VX_TEST_DATASET_NOGTL:
    vp[0] = PLACEHOLDER;
    vp[1] = 5575.147461;
    vp[2] = 4554.516113;
    vp[3] = 5066.605469;
    vp[4] = 5372.791992;
    vp[5] = 4184.089355;
    vp[6] = 6533.309082;
    vp[7] = 4997.064453;

    vs[0] = PLACEHOLDER;
    vs[1] = 3132.099854;
    vs[2] = 2313.560547;
    vs[3] = 2916.298340;
    vs[4] = 3024.300049;
    vs[5] = 2434.559814;
    vs[6] = 3776.399902;
    vs[7] = 2889.031982;
    
    rho[0] = PLACEHOLDER;
    rho[1] = 2631.810447;
    rho[2] = 2469.775216;
    rho[3] = 2545.103296;
    rho[4] = 2595.547455;
    rho[5] = 2418.822011;
    rho[6] = 2841.465796;
    rho[7] = 2534.298282;
    break;
  case VX_TEST_DATASET_BKG:
    vp[0] = 6300.0;
    vp[1] = 5575.147461;
    vp[2] = 4554.516113;
    vp[3] = 5066.605469;
    vp[4] = 5372.791992;
    vp[5] = 4184.089355;
    vp[6] = 6533.309082;
    vp[7] = 4997.064453;

    vs[0] = 3637.306641;
    vs[1] = 3132.099854;
    vs[2] = 2313.560547;
    vs[3] = 2916.298340;
    vs[4] = 3024.300049;
    vs[5] = 2434.559814;
    vs[6] = 3776.399902;
    vs[7] = 2889.031982;
    
    rho[0] = 2859.770000;
    rho[1] = 2631.810447;
    rho[2] = 2469.775216;
    rho[3] = 2545.103296;
    rho[4] = 2595.547455;
    rho[5] = 2418.822011;
    rho[6] = 2841.465796;
    rho[7] = 2534.298282;
    break;
  default:
    return(1);
    break;
  }

  return(0);
}

/* Test bkg/topo handler */
int vx_test_bkg(vx_entry_t *entry, vx_request_t req_type)
{
  if ((req_type == VX_REQUEST_ALL) || (req_type == VX_REQUEST_TOPO)) {
    entry->topo = 0.0;
    entry->mtop = 0.0;
    entry->base = PLACEHOLDER;
    entry->moho = PLACEHOLDER;
  }

  if ((req_type == VX_REQUEST_ALL) || (req_type == VX_REQUEST_VSVPRHO)) {
    entry->data_src = VX_SRC_BK;
    entry->provenance = (float)VX_PROV_BACKGND;
    entry->vp = 1234.5;
    entry->vs = 2345.6;
    entry->rho = 3456.7;
  }
  return(0);
}


int runVX(const char *bindir, const char *cvmdir, 
	  const char *infile, const char *outfile)
{
  char currentdir[128];
  char runpath[128];

  sprintf(runpath, "%s/run_vx.sh", bindir);

  printf("Running cmd: vx %s %s\n", infile, outfile);

  /* Save current directory */
  getcwd(currentdir, 128);
  
  /* Fork process */
  pid_t pid;
  pid = fork();
  if (pid == -1) {
    perror("fork");
    return(1);
  } else if (pid == 0) {
    /* Change dir to cvmdir */
    if (chdir(cvmdir) != 0) {
      printf("FAIL: Error changing dir in runfortran\n");
      return(1);
    }

    execl(runpath, runpath, infile, outfile, (char *)0);
    perror("execl"); /* shall never get to here */
    printf("FAIL: CVM exited abnormally\n");
    return(1);
  } else {
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
      return(0);
    } else {
      printf("FAIL: CVM exited abnormally\n");
      return(1);
    }
  }

  return(0);
}


int runVXLite(const char *bindir, const char *cvmdir, 
	      const char *infile, const char *outfile,
	      int mode)
{
  char currentdir[128];
  char flags[128];

  char runpath[128];

  sprintf(runpath, "./run_vx_lite.sh");

  sprintf(flags, "-m %s ", cvmdir);
  if ((mode & 0xFFFF) == MODE_EMUL) {
    strcat(flags, "-z elev");
  }
  if ((mode & 0xFFFF) == MODE_DEPTH) {
      strcat(flags, "-z dep");
  }
  if ((mode & 0xFFFF) == MODE_SCEC) {
    if (strlen(flags) > 0) {
      strcat(flags, " -s");
    } else {
      strcat(flags, "-s");
    }
  }
  if ((mode & 0xFFFF) == MODE_NOGTL) {
    if (strlen(flags) > 0) {
      strcat(flags, " -g");
    } else {
      strcat(flags, "-g");
    }
  }

  printf("Running cmd: vx_lite %s %s %s\n", flags, infile, outfile);

  /* Save current directory */
  getcwd(currentdir, 128);
  
  /* Fork process */
  pid_t pid;
  pid = fork();
  if (pid == -1) {
    perror("fork");
    printf("FAIL: unable to fork\n");
    return(1);
  } else if (pid == 0) {

    /* Change dir to bindir */
    if (chdir(bindir) != 0) {
      printf("FAIL: Error changing dir in runfortran\n");
      return(1);
    }

    if (strlen(flags) == 0) {
      execl(runpath, runpath, infile, outfile, (char *)0);
    } else {
      execl(runpath, runpath, flags, infile, outfile, (char *)0);
    }
    perror("execl"); /* shall never get to here */
    printf("FAIL: CVM exited abnormally\n");
    return(1);
  } else {
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
      return(0);
    } else {
      printf("FAIL: CVM exited abnormally\n");
      return(1);
    }
  }

  return(0);
}
