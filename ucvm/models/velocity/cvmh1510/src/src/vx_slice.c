/** 
    vx_slice - A simple program to extract velocity values from
    a voxet for a regular grid within a rectangular region. 
    Accepts Geographic Coordinates or UTM Zone 11 coordinates.

    01/2011: PES: Initial implementation
**/


#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <getopt.h>
#include "params.h"
#include "vx_sub.h"


/* Global variables */
#define DEFAULT_GRIDSIZE 0.1

extern char *optarg;
extern int optind, opterr, optopt;


/* Usage function */
void usage() {
  printf("     vx_slice - (c) Harvard University, SCEC\n");
  printf("Extract a map of velocities from a simple GOCAD voxet for a regular\n");
  printf("grid within the specified region at the specified depth/elev. Accepts\n");
  printf("geographic/UTM coordinates for the geographic region. Outputs gridded\n");
  printf("data suitable for plotting in MATLAB or GMT.\n\n");
  printf("\tusage: vx_slice [-g] [-s] [-m dir] [-z dep/elev/off] [-r gridsize] [-f outfile] -- <x1> <y1> <x2> <y2> <z> <value>\n\n");
  printf("Flags:\n");
  printf("\t-g disable GTL (default is on).\n");
  printf("\t-s directs use of SCEC 1D background and topo.\n");
  printf("\t-m directory containing model files (default is '.').\n");
  printf("\t-z directs use of dep/elev/off for Z column.\n");
  printf("\t-r flag is gridsize in degrees/meters. Defaults to 0.1.\n");
  printf("\t-f flag specifies filename to save x,y,z values. Otherwise stdout is used.\n\n");

  printf("Arguments:\n");
  printf("\t<x1> <y1> is SW corner of region to extract in geo/utm coords\n");
  printf("\t<x2> <y2> is NE corner of region to extract in geo/utm coords\n");
  printf("\t<z> is elev_offset, depth, or elevation depending on mode\n");
  printf("\t<value> is one of: vp, vs, rho.\n\n");
  printf("Output format is:\n");
  printf("\tX Y value\n\n");
  printf("Version: %s\n\n", VERSION);
  exit (0);
}


int main (int argc, char *argv[])
{
  vx_entry_t entry;
  char modeldir[CMLEN];
  vx_zmode_t zmode;
  int use_gtl = True;
  int use_scec = False;
  int use_log = False;
  int opt;

  double gridsize_x = DEFAULT_GRIDSIZE;
  double gridsize_y = DEFAULT_GRIDSIZE;
  double sw_coord[2];
  double ne_coord[2];
  double elev;
  char value_type[128];
  char logfile[128];
  int num_x, num_y;
  int i, j;
  FILE *lf = stdout;

  zmode = VX_ZMODE_ELEVOFF;
  strcpy(modeldir, ".");

   /* Parse options */
  while ((opt = getopt(argc, argv, "gm:sz:hf:r:")) != -1) {
    switch (opt) {
    case 'g':
      use_gtl = False;
      break;
    case 'm':
      strcpy(modeldir, optarg);
      break;
    case 's':
      use_scec = True;
      break;
    case 'z':
      if (strcasecmp(optarg, "dep") == 0) {
        zmode = VX_ZMODE_DEPTH;
      } else if (strcasecmp(optarg, "elev") == 0) {
        zmode = VX_ZMODE_ELEV;
      } else if (strcasecmp(optarg, "off") == 0) {
        zmode = VX_ZMODE_ELEVOFF;
      } else {
        fprintf(stderr, "Invalid coord type %s", optarg);
        usage();
        exit(0);
      }
      break;
    case 'r':
      gridsize_x = gridsize_y = atof(optarg);
      break;
    case 'f':
      use_log = True;
      strcpy(logfile, optarg);
      break;
    case 'h':
      usage();
      exit(0);
      break;
    default: /* '?' */
      usage();
      exit(1);
    }
  }

  if (argc != optind + 6) {
    usage();
    exit(1);
  }

  /* Save arguments */
  sw_coord[0] = atof(argv[optind]);
  sw_coord[1] = atof(argv[optind + 1]);
  ne_coord[0] = atof(argv[optind + 2]);
  ne_coord[1] = atof(argv[optind + 3]);
  elev = atof(argv[optind + 4]);
  strcpy(value_type, argv[optind + 5]);

  printf("Orig Point SW: %lf, %lf\n", sw_coord[0], sw_coord[1]);
  printf("Orig Point NE: %lf, %lf\n", ne_coord[0], ne_coord[1]);

  if (zmode == VX_ZMODE_DEPTH) {
    printf("Depth: %lf\n", elev);
  } else if (zmode == VX_ZMODE_ELEV) {
    printf("Elevation: %lf\n", elev);
  } else {
    printf("Elevation Offset: %lf\n", elev);
  }

  printf("Value Type: %s\n", value_type);
  printf("Grid size: x=%lf,y=%lf\n", gridsize_x, gridsize_y);
  if (use_log) {
    printf("Logfile: %s\n", logfile);
  }

  /* Perform setup */
  if (vx_setup(modeldir) != 0) {
    fprintf(stderr, "Failed to init vx\n");
    exit(1);
  }

  /* Register SCEC 1D background model */
  if (use_scec) {
    vx_register_scec();
  }

  /* Set GTL */
  vx_setgtl(use_gtl);

  /* Set zmode */
  vx_setzmode(zmode);

  /* Set coordinate type */
  if ((sw_coord[0]<360.) && (fabs(sw_coord[1])<90)) {
    entry.coor_type = VX_COORD_GEO;
  } else {
    entry.coor_type = VX_COORD_UTM;
  }
  switch (entry.coor_type) {
  case VX_COORD_GEO:
    printf("Coord Type: GEO\n");
    break;
  case VX_COORD_UTM:
    printf("Coord Type: UTM\n");
    break;
  default:
    printf("Invalid coord type\n");
    exit(1);
    break;
  }

  /* Generate grid */
  num_x = round((ne_coord[0] - sw_coord[0]) / gridsize_x);
  num_y = round((ne_coord[1] - sw_coord[1]) / gridsize_y);
  ne_coord[0] = sw_coord[0] + (num_x * gridsize_x);
  ne_coord[1] = sw_coord[1] + (num_y * gridsize_y);
  printf("Adjusted Point SW: %lf, %lf\n", sw_coord[0], sw_coord[1]);
  printf("Adjusted Point NE: %lf, %lf\n", ne_coord[0], ne_coord[1]);
  if (num_x < 0) {
    num_x = abs(num_x);
    gridsize_x = -gridsize_x;
  }
  if (num_y < 0) {
    num_y = abs(num_y);
    gridsize_y = -gridsize_y;
  }
  /* Include fence-post */
  num_x += 1;
  num_y += 1;

  printf("Slice Dims: %d x %d\n", num_x, num_y);

  printf("Querying CVM-H\n");

  if (use_log) {
    printf("Writing to logfile %s\n", logfile);
    lf = fopen(logfile, "w");
  }

  for (j = 0; j < num_y; j++) {
    for (i = 0; i < num_x; i++) {
      entry.coor[0] = sw_coord[0] + (i * gridsize_x);
      entry.coor[1] = sw_coord[1] + (j * gridsize_y);
      entry.coor[2] = elev;

      /* Query the point */
      vx_getcoord(&entry);

      if (strcmp(value_type, "vp") == 0) {
	fprintf(lf, "%d %d %f\n", i, j, entry.vp);
      } else if (strcmp(value_type, "vs") == 0) {
	fprintf(lf, "%d %d %f\n", i, j, entry.vs);
      } else if (strcmp(value_type, "rho") == 0) {
	fprintf(lf, "%d %d %f\n", i, j, entry.rho);
      } else {
	fprintf(lf, "%d %d %f\n", i, j, -99999.0);
      }

    }
  }
  
  if (use_log) {
    fclose(lf);
  }

  /* Perform cleanup */
  vx_cleanup();

  return 0;
}
