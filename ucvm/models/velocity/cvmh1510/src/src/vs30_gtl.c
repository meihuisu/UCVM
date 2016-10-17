/** 
    vs30_gtl.c - Vs30 derived GTL based on Ely (2010)

    01/2011: PES: Initial implementation
**/

#include <string.h>
#include "stdlib.h"
#include <stdio.h>
#include <math.h>
#include "vs30_gtl.h"
#include "voxet.h"
#include "utils.h"

#define boolean int
#define True 1
#define False 0

/* Global variables */
static int gtl_is_setup = False;
static char *gtlbuffer = NULL;

/* Extents of Vs30 GTL in UTM coords */
struct gtl_info {
  double extent[4];
  int x;
  int y;
  int dsize;
  double spacing;
  double nodata_flag;
} gtl_info;

struct gtl_info gtl;


/* Interpolation parameters */
double zt = 350.0;
double a, b, c;

/* Smooth the material properties contained in entry with the GTL
   material properties. The flag updated is set to true if the
   interpolation was performed, false otherwise. Smoothing with the
   GTL is not possible if one or more material property is missing,
   or if the data point falls outside the GTL region. */
int gtl_interp(gtl_entry_t *entry, int *updated) {
  int i;
  gtl_grid_t gtlgrid;
  double vs30;
  double new_vp, new_vs, new_rho;
  double z, f, g;
  double cur_zt;

  /* GTL interpolation may fail for a variety of reasons:
     1) GTL is not initialized
     2) Negative vs30 returned from GTL
     3) Depth not in 0-zt range
     4) vp,vs,rho from core model is negative
     5) Interpolated vp,vs,rho is negative
     6) Point does not lay within GTL
  */

  if (gtl_is_setup != True) {
    return(1);
  }
  
  for (i = 0; i < 3; i++) {
    gtlgrid.coor_utm[i] = entry->coor_utm[i];
    entry->cell[i] = -9999.0;
  }
  entry->provenance = GTL_PROV_NONE;
  *updated = False;

  cur_zt = gtl_get_adj_transition(entry->topo_gap);

  if ((entry->depth < 0.0) || (entry->depth >= cur_zt) ||
      (entry->vp <= 0.0) || (entry->vs <= 0.0) || (entry->rho <= 0.0)) {
    return(0);
  }
  
  /* Query GTL */
  gtl_getcoord(&gtlgrid);
  if (gtlgrid.vs30 <= 0.0) {
    return(0);
  }

  /* Retrieve vs30 at this location */
  vs30 = gtlgrid.vs30;

  /* Determine interpolated vp, vs, rho */
  z = entry->depth / cur_zt;
  f = z - pow(z, 2.0);
  g = pow(z, 2.0) + 2*pow(z, 0.5) - 3*z;
  new_vs = (z + b*f)*(entry->vs) + (a - a*z + c*g)*vs30;
  new_vp = (z + b*f)*(entry->vp) + (a - a*z + c*g)*brocher_vp(vs30);
  new_rho = nafe_drake_rho(new_vp);
  if ((new_vs <= 0.0) || (new_vp <= 0.0) || (new_rho <= 0.0)) {
    return(1);
  }
  entry->vp = new_vp;
  entry->vs = new_vs;
  entry->rho = new_rho;
  entry->provenance = gtlgrid.provenance;
  for (i = 0; i < 3; i++) {
    entry->cell[i] = gtlgrid.vs30_cell[i];
  }

  *updated = True;

  return(0);
}


/* Retrieve GTL data point in UTM */
void gtl_getcoord(gtl_grid_t *entry) {
  int j;
  int gcoor[3];
  float vs30;

  /* Proceed only if setup has been performed */
  if ((entry == NULL) || (gtl_is_setup != True)) {
    return;
  }

  /* Initialize entry structure */
  entry->vs30_cell[0] = -1;
  entry->vs30_cell[1] = -1;
  entry->provenance = GTL_PROV_NONE;
  entry->vs30 = gtl.nodata_flag;

  gcoor[0]=round((entry->coor_utm[0]-gtl.extent[0])/gtl.spacing);
  gcoor[1]=round((entry->coor_utm[1]-gtl.extent[2])/gtl.spacing);
  gcoor[2]=0;

  // Check if inside GTL
  if(gcoor[0]>=0 && gcoor[1]>=0 && gcoor[0]<gtl.x && gcoor[1]<gtl.y) {
    j = ((gcoor[1] * gtl.x) + gcoor[0]) * gtl.dsize;
    memcpy(&vs30, &gtlbuffer[j], gtl.dsize);

    /* Save data */
    entry->vs30_cell[0] = gcoor[0];
    entry->vs30_cell[1] = gcoor[1];
    entry->vs30 = vs30;
    entry->provenance = GTL_PROV_VS30;
  }

  return;
}


/* Return true if the UTM coords fall inside the GTL, false otherwise */
int gtl_point_is_inside(double *coor_utm) {
  int gcoor[3];

  /* Proceed only if setup has been performed */
  if ((coor_utm == NULL) || (gtl_is_setup != True)) {
    return(False);
  }

  gcoor[0]=round((coor_utm[0]-gtl.extent[0])/gtl.spacing);
  gcoor[1]=round((coor_utm[1]-gtl.extent[2])/gtl.spacing);
  gcoor[2]=0;

  // Check if inside GTL
  if(gcoor[0]>=0 && gcoor[1]>=0 && gcoor[0]<gtl.x && gcoor[1]<gtl.y) {
    return True;
  } else {
    return False;
  }

}

/* Get closest GTL point (closest_coor_utm) and its distance to the 
   user specified point (coor_utm). Useful in smoothing GTL with
   background. */
int gtl_closest_point(double *coor_utm, double *closest_coor_utm, 
		      double *dist_2d)
{
  int gcoor[3];
  int i;
  double dxyz[2];

  /* Proceed only if setup has been performed */
  if ((coor_utm == NULL) || (gtl_is_setup != True)) {
    return(False);
  }

  gcoor[0]=round((coor_utm[0]-gtl.extent[0])/gtl.spacing);
  gcoor[1]=round((coor_utm[1]-gtl.extent[2])/gtl.spacing);
  gcoor[2]=0;

  /* Check if inside GTL */
  if(gcoor[0]>=0 && gcoor[1]>=0 && gcoor[0]<gtl.x && gcoor[1]<gtl.y) {
    for (i = 0; i < 3; i++) {
      closest_coor_utm[i] = coor_utm[i];
    }
    *dist_2d = 0.0;
    return (0);
  } else {
    /* Find closest and compute distance */
    for (i = 0; i < 2; i++) {
      if (gcoor[i] < 0) {
	gcoor[i] = 0;
      }
    }
    if (gcoor[0] >= gtl.x) {
      gcoor[0] = gtl.x - 1;
    }
    if (gcoor[1] >= gtl.y) {
      gcoor[1] = gtl.y - 1;
    }

    closest_coor_utm[0] = gtl.extent[0] + (gcoor[0] * gtl.spacing);
    closest_coor_utm[1] = gtl.extent[2] + (gcoor[1] * gtl.spacing);
    closest_coor_utm[2] = 0.0;

    for (i = 0; i < 2; i++) {
      dxyz[i] = coor_utm[i] - closest_coor_utm[i];
    }
    *dist_2d = sqrt(pow(dxyz[0], 2.0) + pow(dxyz[1], 2.0));
  }

  return(0);
}


/* Get current default GTL transition depth */
double gtl_get_transition() {
  return(zt);
}


/* Get current GTL transition depth taking into account topo-mtop 
   gap, which may be greater than the predefined default. */
double gtl_get_adj_transition(double topo_gap) {
  if (zt >= topo_gap) {
    return(zt);
  } else {
    return(topo_gap);
  }
}


/* Initialize GTL */
int gtl_setup(char *file_path) {
  FILE *ifi;
  int j;
  int bufsize, ncells;
  union zahl l, *h;
  char mdlfile[256], hdrfile[256];
  char cfgbuf[512];
  char *key, *value;

  /* Setup interpolation parameters */
  a = 1.0/2.0;
  b = 2.0/3.0;
  c = 3.0/2.0;

  /*
  c = 4.0/3.0;
  b3 = 1/40000000.0;
  b2 = -1/40000.0;
  b1 = (b3*(pow(zs, 3.0) - pow(zt, 3.0)) + 
	b2 * (pow(zs, 2.0) - pow(zt, 2.0)) + 1.0) / (zt - 30.0);
  b0 = -b3 * pow(zs, 3.0) - b2 * pow(zs, 2.0) - b1 * zs;
  a1 = (2.0*c - 2.0) / 30.0;
  a0 = 2.0 - c;
  */

  sprintf(mdlfile, "%s.mdl", file_path);
  sprintf(hdrfile, "%s.hdr", file_path);

  /* Load GTL header from file */
  ifi=fopen(hdrfile, "r");
  if (ifi == NULL) {
    return(1);
  }
  while (fgets(cfgbuf, 512, ifi) != NULL) {
    key = strtok(cfgbuf, "=");
    if (key[0] != '#') {
      value = strtok(NULL, "=");
      if (value != NULL) {
	if (value[strlen(value)-1] == '\n') {
	  value[strlen(value)-1] = '\0';
	}
	if (strcmp(key, "x0") == 0) {
	  gtl.extent[0] = atof(value);
	} else if (strcmp(key, "x1") == 0) {
	  gtl.extent[1] = atof(value);
	} else if (strcmp(key, "y0") == 0) {
	  gtl.extent[2] = atof(value);
	} else if (strcmp(key, "y1") == 0) {
	  gtl.extent[3] = atof(value);
	} else if (strcmp(key, "dsize") == 0) {
	  gtl.dsize = atoi(value);
	} else if (strcmp(key, "spacing") == 0) {
	  gtl.spacing = atof(value);
	} else if (strcmp(key, "nodata") == 0) {
	  gtl.nodata_flag = atof(value);
	}
      }
    }
  }

  fclose(ifi);

  gtl.x = round((gtl.extent[1] - gtl.extent[0]) / gtl.spacing) + 1;
  gtl.y = round((gtl.extent[3] - gtl.extent[2]) / gtl.spacing) + 1;

  /* Allocate memory buffer for GTL */
  ncells = gtl.x * gtl.y;
  bufsize = ncells * gtl.dsize;
  gtlbuffer=(char *)malloc(bufsize);
  if (gtlbuffer == NULL) {
    return(1);
  }

  /* Load GTL model from file */
  ifi=fopen(mdlfile, "r");
  if (ifi == NULL) {
    return(1);
  }

  if (fread(gtlbuffer, gtl.dsize, ncells, ifi) != ncells) {
    fprintf(stderr, "Failed to read %d cells of size %d from %s\n", 
            ncells, ncells, mdlfile);
    fclose(ifi);
    return(1);
  }

  fclose(ifi);

  /* GTL file is little endian */
  if (vx_system_endian() == VX_BYTEORDER_MSB) {
    /* Swap endian */
    for (j = 0; j < ncells; j++) {
      h = (union zahl *)&(gtlbuffer[j*gtl.dsize]);
      l.c[3]=h->c[0];
      l.c[2]=h->c[1];
      l.c[1]=h->c[2];
      l.c[0]=h->c[3];
      memcpy(&(gtlbuffer[j*gtl.dsize]), &l, sizeof(union zahl));
    }
  }

  gtl_is_setup = True;

  return(0);
}


/* Density derived from Vp via Nafe-Drake curve, Brocher (2005) eqn 1. */
double nafe_drake_rho(double f) {
  double rho;

  /* Convert m to km */
  f = f * 0.001;
  rho = f * (1.6612 - f * (0.4721 - f * (0.0671 - f * (0.0043 - f * 0.000106))));
  if (rho < 1.0) {
    rho = 1.0;
  }
  rho = rho * 1000.0;
  return(rho);
}


/* Vp derived from Vs via Brocher (2005) eqn 9. */
double brocher_vp(double f) {
  double vp;

  f = f * 0.001;
  vp = 0.9409 + f * (2.0947 - f * (0.8206 - f * (0.2683 - f * 0.0251)));
  vp = vp * 1000.0;
  return(vp);
}
