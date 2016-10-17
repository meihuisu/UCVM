/** 
    vx_lite - A command line program to extract velocity values
    from CVM-H. Accepts Geographic Coordinates or UTM Zone 11 
    coordinates.

    01/2011: PES: Initial implementation derived from vx.c
    07/2011: PES: Added option for specifying model directory,
                  reorganized options
**/


#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <getopt.h>
#include "params.h"
#include "vx_sub.h"


/* Usage function */
void usage() {
  printf("     vx_lite - (c) Harvard University, SCEC\n");
  printf("Extract velocities from a simple GOCAD voxet. Accepts\n");
  printf("geographic coordinates and UTM Zone 11, NAD27 coordinates in\n");
  printf("X Y Z columns. Z is expressed as elevation offset by default.\n\n");
  printf("\tusage: vx_lite [-g] [-s] [-m dir] [-z dep/elev/off] < file.in\n\n");
  printf("Flags:\n");
  printf("\t-g disable GTL (default is on).\n");
  printf("\t-s directs use of SCEC 1D background and topo.\n");
  printf("\t-m directory containing model files (default is '.').\n");
  printf("\t-z directs use of dep/elev/off for Z column (default is offset).\n\n");
  printf("Output format is:\n");
  printf("\tX Y Z utmX utmY elevX elevY topo mtop base moho hr/lr/cm cellX cellY cellZ tg vp vs rho\n\n");
  printf("Version: %s\n\n", VERSION);
  exit (0);
}

extern char *optarg;
extern int optind, opterr, optopt;


int main (int argc, char *argv[])
{
  vx_entry_t entry;
  char modeldir[CMLEN];
  vx_zmode_t zmode;
  int use_gtl = True;
  int use_scec = False;
  int opt;
  
  zmode = VX_ZMODE_ELEVOFF;
  strcpy(modeldir, ".");

  /* Parse options */
  while ((opt = getopt(argc, argv, "gm:sz:h")) != -1) {
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
    case 'h':
      usage();
      exit(0);
      break;
    default: /* '?' */
      usage();
      exit(1);
    }
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

  /* now let's start with searching .... */
  while (!feof(stdin)) {
    if (fscanf(stdin,"%lf %lf %lf",
	       &entry.coor[0],&entry.coor[1],&entry.coor[2]) == 3) {

      if (entry.coor[1]<10000000) {
	printf("%14.6f %15.6f %9.2f ", 
	       entry.coor[0], entry.coor[1], entry.coor[2]);
      }

      /* In case we got anything like degrees */
      if ((entry.coor[0]<360.) && (fabs(entry.coor[1])<90)) {
	entry.coor_type = VX_COORD_GEO;
      } else {
	entry.coor_type = VX_COORD_UTM;
      }

      /* Query the point */
      vx_getcoord(&entry);

      /*** Prevent all to obvious bad coordinates from being displayed */
      if (entry.coor[1]<10000000) {
	//printf("%14.6f %15.6f %9.2f ", 
	//       entry.coor[0], entry.coor[1], entry.coor[2]);
	/* AP: Let's provide the computed UTM coordinates as well */
	printf("%10.2f %11.2f ", entry.coor_utm[0], entry.coor_utm[1]);
	
	printf("%10.2f %11.2f ", entry.elev_cell[0], entry.elev_cell[1]);
	printf("%9.2f ", entry.topo);
	printf("%9.2f ", entry.mtop);
	printf("%9.2f ", entry.base);
	printf("%9.2f ", entry.moho);
	printf("%s %10.2f %11.2f %9.2f ", VX_SRC_NAMES[entry.data_src], 
	       entry.vel_cell[0], entry.vel_cell[1], entry.vel_cell[2]);
	printf("%9.2f %9.2f %9.2f ", entry.provenance, entry.vp, entry.vs);
	printf("%9.2f\n", entry.rho);
      }
    }
  }

  /* Perform cleanup */
  vx_cleanup();

  return 0;
}
