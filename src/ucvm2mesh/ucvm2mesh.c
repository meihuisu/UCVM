/**
 * ucvm2mesh.c - Query UCVM to produce a 3D mesh on a regular grid
 *
 * Created by Patrick Small <patrices@usc.edu>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <getopt.h>
#include <unistd.h>
#include "ucvm_config.h"
#include "um_dtypes.h"
#include "um_config.h"
#include "um_utils.h"
#include "um_stat.h"


/* Get opt args */
extern char *optarg;
extern int optind, opterr, optopt;


/* Display usage information */
void usage(char *arg)
{
  printf("Usage: %s [-h] -f configfile\n\n",arg);
  printf("where:\n");
  printf("\t-h: help message\n");
  printf("\t-f: config file containing mesh params\n\n");
  printf("Config file format:\n");
  printf("\tucvmlist: comma-delimited list of CVMs to query (as supported by UCVM)\n");
  printf("\tucvmconf: UCVM API config file\n");
  printf("\tgridtype: location of x-y gridded points: VERTEX, or CENTER\n");
  printf("\tspacing: grid spacing (units appropriate for proj)\n");
  printf("\tproj: Proj.4 projection specification, or 'cmu' for TeraShake\n");
  printf("\trot: proj rotation angle in degrees, (+ is counter-clockwise)\n");
  printf("\tx0: longitude of origin (deg), or x offset in cmu proj (m)\n");
  printf("\ty0: latitude of origin (deg), or y offset in cmu proj (m)\n");
  printf("\tz0: depth of origin (m, typically 0.0)\n");
  printf("\tnx: number of points along x-axis\n");
  printf("\tny: number of points along y-axis\n");
  printf("\tnz: number of points along z-axis (depth positive)\n");
  printf("\tpx: number of procs along x-axis\n");
  printf("\tpy: number of procs along y-axis\n");
  printf("\tpz: number of procs along z-axis\n");
  printf("\tvp_min: vp minimum (m/s), enforced on vs_min conditions\n");
  printf("\tvs_min: vs minimum (m/s)\n");
  printf("\tmeshfile: path and basename to output mesh files\n");
  printf("\tgridfile: path and filename to output grid filesfiles\n");
  printf("\tmeshtype: mesh format: IJK-12, IJK-20, IJK-32, or SORD\n");
  printf("\tscratch: path to scratch space\n\n");

  printf("Version: %s\n\n", VERSION);
  return;
}


/* Initializer */
int init_app(const char *cfgfile, mesh_config_t *cfg)
{
  /* Read in config */
  if (read_config(0, -1, cfgfile, cfg) != 0) {
    return(1);
  }
  
  disp_config(cfg);

  /* Setup UCVM */
  printf("Setting up UCVM\n");
  if (ucvm_init(cfg->ucvmconf) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to initialize UCVM\n");
    return(1);
  }

  /* Set depth query mode */
  if (ucvm_setparam(UCVM_PARAM_QUERY_MODE,
		    UCVM_COORD_GEO_DEPTH) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Set query mode failed\n");
    return(1);
  }

  /* Add model */
  if (ucvm_add_model_list(cfg->ucvmstr) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to add model list %s\n", cfg->ucvmstr);
    return(1);
  }

  /* Set interpolation z range */
  if (ucvm_setparam(UCVM_PARAM_IFUNC_ZRANGE, 
		    cfg->ucvm_zrange[0], 
		    cfg->ucvm_zrange[1]) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to set interpolation z range\n");
    return(1);
  }

  return(0);
}


/* Perform extraction from UCVM */
int extract(mesh_config_t *cfg) 
{
  /* Statistics */
  stat_t stats[STAT_MAX_STATS];

  /* Performance measurements */
  struct timeval start, end;
  double elapsed;

  /* Buffers */
  int num_grid, num_points;
  ucvm_point_t *pntbuf;
  ucvm_data_t *propbuf;
  mesh_ijk32_t *node_buf;
  int j, k, n;
  double z;
  FILE *ifp;

  /* Initialize statistics */
  memset(&stats[0], 0, STAT_MAX_STATS*sizeof(stat_t));
  stats[STAT_MIN_VP].val = 100000.0;
  stats[STAT_MIN_VS].val = 100000.0;
  stats[STAT_MIN_RHO].val = 100000.0;
  stats[STAT_MIN_RATIO].val = 100000.0;

  /* Compute number of nodes in x-y grid */
  num_grid = cfg->dims.dim[0] * cfg->dims.dim[1];

  /* Allocate buffers */
  fprintf(stdout, "Allocating %d grid points\n", num_grid);
  pntbuf = malloc(num_grid * sizeof(ucvm_point_t));
  propbuf = malloc(num_grid * sizeof(ucvm_data_t));
  node_buf = malloc(num_grid * sizeof(mesh_ijk32_t));
  if ((pntbuf == NULL) || (propbuf == NULL) || (node_buf == NULL)) {
    fprintf(stderr, "Failed to allocate buffers\n");
    return(1);
  }

  printf("Mesh dimensions: %d x %d x %d\n", 
 	 cfg->dims.dim[0], cfg->dims.dim[1], cfg->dims.dim[2]);

  /* Open the grid file */
  ifp = fopen(cfg->gridfile, "rb");
  if (ifp == NULL) {
    fprintf(stderr, "Failed to open gridfile %s for reading\n", 
	    cfg->gridfile);
    return(1);
  }

  /* Read grid points */
  printf("Reading grid points\n");
  if (fread(&(pntbuf[0]), sizeof(ucvm_point_t), 
	    num_grid, ifp) != num_grid) {
    fprintf(stderr, "Failed to read points from grid file (expected %d)\n", 
	    num_grid);
    return(1);
  }

  /* Close grid file */
  fclose(ifp);

  /* Open the mesh file */
  if (mesh_open_serial(&(cfg->dims), cfg->meshfile, cfg->meshtype, num_grid) != 0) {
    fprintf(stderr, "Error: mesh_open_serial reported failure\n");
    return(1);
  }

  num_points = 0;
  for (k = 0; k < cfg->dims.dim[2]; k++) {
    gettimeofday(&start,NULL);

    /* Set z coordinate */
    z = cfg->origin.coord[2] + (k * cfg->spacing);
    for (n = 0; n < num_grid; n++) {
      pntbuf[n].coord[2] = z;
    }

    /* Query UCVM at this k */
    if (ucvm_query(num_grid, pntbuf, propbuf) != UCVM_CODE_SUCCESS) {
      fprintf(stderr, "Query UCVM failed\n");
      return(1);
    }

    /* Convert the data points to a mesh node list */
    if (mesh_data_to_node(0, 0, cfg->dims.dim[0], 0, cfg->dims.dim[1],
			  k, pntbuf, propbuf, node_buf, cfg->vp_min,
			  cfg->vs_min) != 0) {
      return(1);
    }

    /* Calculate statistics */
    calc_stats_list(0, cfg->dims.dim[0], 0, cfg->dims.dim[1], k, 
		    node_buf, &stats[0]);
    
    if (n != num_grid) {
      fprintf(stderr, "Number of nodes mismatch\n");
      return(1);
    }

    /* Calculate extraction elapsed time */
    gettimeofday(&end,NULL);
    elapsed = (end.tv_sec - start.tv_sec) * 1000.0 +
      (end.tv_usec - start.tv_usec) / 1000.0;
    fprintf(stdout,
	    "Extracted slice %d (%d pnts) in %.2f ms, %f pps\n",
	    k, num_grid, elapsed, (float)(num_grid/(elapsed/1000.0)));
    gettimeofday(&start,NULL);

    /* Write this buffer */
    if (mesh_write_serial(&(node_buf[0]), num_grid) != 0) {
      fprintf(stderr, "Failed to write nodes to mesh file\n");
      return(1);
    }
    num_points = num_points + num_grid;

    /* Calculate write elapsed time */
    gettimeofday(&end,NULL);
    elapsed = (end.tv_sec - start.tv_sec) * 1000.0 +
      (end.tv_usec - start.tv_usec) / 1000.0;
    if (elapsed > 0.001) {
      fprintf(stdout,
	      "Wrote slice %d (%d pnts) in %.2f ms, %f pps\n",
	      k, num_grid, elapsed, (float)(num_grid/(elapsed/1000.0)));
    } else {
      fprintf(stdout,
	      "Wrote slice %d (%d pnts) in %.2f ms\n",
	      k, num_grid, elapsed);
    }
  }
  
  fprintf(stdout, "Extracted %d points\n", num_points);

  /* Close the mesh writer */
  mesh_close_serial();

  /* Free buffers */
  free(pntbuf);
  free(propbuf);
  free(node_buf);

  for (j = 0; j < STAT_MAX_STATS; j++) {
    printf("%s: %f at\n", stat_get_label(j), stats[j].val);
    printf("\ti,j,k : %d, %d, %d\n", 
	   stats[j].i, stats[j].j, stats[j].k);
  }

  return(0);
}


int main(int argc, char **argv)
{
  /* Config params */
  mesh_config_t cfg;
  size_t slice_size;

  /* Grid params */
  ucvm_projdef_t iproj, oproj;
  ucvm_trans_t trans;

  /* Filesytem IO */
  int i;

  /* Options */
  int opt;
  char configfile[UCVM_MAX_PATH_LEN];

  /* Parse options */
  strcpy(configfile, "");
  while ((opt = getopt(argc, argv, "hf:")) != -1) {
    switch (opt) {
    case 'f':
      strcpy(configfile, optarg);
      break;
    case 'h':
      usage(argv[0]);
      exit(0);
      break;
    default: /* '?' */
      fprintf(stderr, "Unrecognized option %c\n", opt);
      usage(argv[0]);
      return(1);
    }
  }

  if (strcmp(configfile, "") == 0) {
    fprintf(stderr, "No config file specified\n");
    return(1);
  }

  /* Application init */
  if (init_app(configfile, &cfg) != 0) {
    fprintf(stderr, "Initialization failed\n");
    return(1);
  }

  /* Delete output mesh file if present */
  deleteFile(cfg.meshfile);
  deleteFile(cfg.gridfile);

  /* Generate the 2D grid */
  printf("Generating 2D grid\n");
  sprintf(iproj.proj, "%s", UCVM_PROJ_GEO);
  sprintf(oproj.proj, "%s", cfg.proj);
  trans.rotate = cfg.rot;
  for (i = 0; i < 3; i++) {
    trans.origin[i] = cfg.origin.coord[i];
    trans.translate[i] = 0.0;
  }
  trans.gtype = cfg.gridtype;
    
  if (ucvm_grid_gen_file(&iproj, &trans, &oproj, &(cfg.dims), 
			 cfg.spacing, cfg.gridfile) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to create gridfile %s\n", cfg.gridfile);
    return(1);
  }

  /* Convert grid from Proj.4 projection to latlong */
  printf("Converting grid to latlong\n");
  fflush(stdout);
  slice_size = (size_t)cfg.dims.dim[0] * (size_t)cfg.dims.dim[1];
  if (ucvm_grid_convert_file(&oproj, &iproj, slice_size, 
			     cfg.gridfile) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to convert gridfile %s\n", cfg.gridfile);
    return(1);
  }

  printf("Grid generation complete\n");
  
  /* Perform extractions */
  if (extract(&cfg) != 0) {
    return(1);
  }

  fprintf(stdout, "Done.\n");
  return(0);
}
