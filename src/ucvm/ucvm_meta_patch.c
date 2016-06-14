#include <stdio.h>
#include <string.h>
#include "ucvm_meta_patch.h"


/* Compute surfaces based off dimensions and spacing */
int ucvm_meta_patch_getsurf(ucvm_point_t *dims, double spacing, 
			    ucvm_psurf_t surfs[2][2]) 
{
  int i, j;

  /* Surf 0,0 */
  surfs[0][0].corners[0].coord[0] = 0.0;
  surfs[0][0].corners[0].coord[1] = 0.0;
  surfs[0][0].corners[0].coord[2] = 0.0;
  surfs[0][0].corners[1].coord[0] = dims->coord[0];
  surfs[0][0].corners[1].coord[1] = spacing;
  surfs[0][0].corners[1].coord[2] = dims->coord[2];

  /* Surf 0,1 */
  surfs[0][1].corners[0].coord[0] = 0.0;
  surfs[0][1].corners[0].coord[1] = 0.0;
  surfs[0][1].corners[0].coord[2] = 0.0;
  surfs[0][1].corners[1].coord[0] = spacing;
  surfs[0][1].corners[1].coord[1] = dims->coord[1];
  surfs[0][1].corners[1].coord[2] = dims->coord[2];

  /* Surf 1,0 */
  surfs[1][0].corners[0].coord[0] = 0.0;
  surfs[1][0].corners[0].coord[1] = dims->coord[1] - spacing;
  surfs[1][0].corners[0].coord[2] = 0.0;
  surfs[1][0].corners[1].coord[0] = dims->coord[0];
  surfs[1][0].corners[1].coord[1] = dims->coord[1];
  surfs[1][0].corners[1].coord[2] = dims->coord[2];

  /* Surf 1,1 */
  surfs[1][1].corners[0].coord[0] = dims->coord[0] - spacing;
  surfs[1][1].corners[0].coord[1] = 0.0;
  surfs[1][1].corners[0].coord[2] = 0.0;
  surfs[1][1].corners[1].coord[0] = dims->coord[0];
  surfs[1][1].corners[1].coord[1] = dims->coord[1];
  surfs[1][1].corners[1].coord[2] = dims->coord[2];

  for (j = 0; j < 2; j++) {
    for (i = 0; i < 2; i++) {
      surfs[j][i].dims.dim[0] = (int)(surfs[j][i].corners[1].coord[0] - 
				      surfs[j][i].corners[0].coord[0])/spacing;
      surfs[j][i].dims.dim[1] = (int)(surfs[j][i].corners[1].coord[1] - 
				      surfs[j][i].corners[0].coord[1])/spacing;
      surfs[j][i].dims.dim[2] = (int)(surfs[j][i].corners[1].coord[2] - 
				      surfs[j][i].corners[0].coord[2])/spacing;
      //fprintf(stderr, "Surf %d,%d dims: %d,%d,%d\n", j, i, 
      //	      surfs[j][i].dims.dim[0],
      //	      surfs[j][i].dims.dim[1],
      //	      surfs[j][i].dims.dim[2]);
      surfs[j][i].num_points = surfs[j][i].dims.dim[0] *
	surfs[j][i].dims.dim[1] *
	surfs[j][i].dims.dim[2];
      surfs[j][i].props = NULL;
    }
  }

  return(UCVM_CODE_SUCCESS);
}


