#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ucvm_grid.h"

#define MAX_POINTS 10000


int main(int argc, char **argv)
{
  int i, numpoints;
  ucvm_projdef_t iproj, oproj, fproj;
  ucvm_trans_t trans;
  ucvm_point_t points[MAX_POINTS];
  ucvm_dim_t dims;

  /* Setup grid projection */
  strcpy(iproj.proj, "+proj=latlong +datum=WGS84");
  strcpy(oproj.proj, "+proj=utm +datum=WGS84 +zone=11");
  strcpy(fproj.proj, "+proj=latlong +datum=WGS84");
  trans.rotate = -45.0;
  trans.origin[0] = -118.0;
  trans.origin[1] = 34.0;
  trans.origin[2] = 0.0;
  trans.translate[0] = 0.0;
  trans.translate[1] = 0.0;
  trans.translate[2] = 0.0;
  trans.gtype = UCVM_GRID_CELL_CENTER;

  /* Setup desired grid dimensions */
  dims.dim[0] = 10;
  dims.dim[1] = 10;
  dims.dim[2] = 1;
  numpoints = dims.dim[0] * dims.dim[1] * dims.dim[2];

  /* Generate grid */
  printf("Generating grid\n");
  ucvm_grid_gen(&iproj, &trans, &oproj, &dims, 100.0, points);

  printf("Projection Grid:\n");
  for (i = 0; i < numpoints; i++) {
    printf("%lf, %lf\n", points[i].coord[0], points[i].coord[1]);
  }

  /* Convert grid */
  ucvm_grid_convert(&oproj, &fproj, numpoints, points);
  
  printf("GEO Grid:\n");
  for (i = 0; i < numpoints; i++) {
    printf("%lf, %lf\n", points[i].coord[0], points[i].coord[1]);
  }

  /* Generate grid file */
  printf("Generating grid\n");
  ucvm_grid_gen_file(&iproj, &trans, &oproj, &dims, 100.0, "grid.out");

  /* Convert grid file */
  ucvm_grid_convert_file(&oproj, &fproj, numpoints, "grid.out");
  

  return(0);
}
