#ifndef VS30_GTL_H
#define VS30_GTL_H

#define DEFAULT_GTL_FILE "cvm_vs30_wills"


typedef enum { GTL_PROV_NONE = 0,
               GTL_PROV_VS30 } gtl_prov_t;


typedef struct gtl_grid_t 
{
  double coor_utm[3];
  gtl_prov_t provenance;
  double vs30_cell[2];
  double vs30;
} gtl_grid_t;


typedef struct gtl_entry_t 
{
  double coor_utm[3];
  double depth;
  double topo_gap; // topo-mtop
  double cell[3];
  gtl_prov_t provenance;
  double vp;
  double vs;
  double rho;
} gtl_entry_t;


/* Retrieve GTL data point in UTM and interpolate with existing properties */
int gtl_interp(gtl_entry_t *entry, int *updated);

/* Retrieve GTL data point in UTM */
void gtl_getcoord(gtl_grid_t *entry);

/* Checks if point inside GTL */
int gtl_point_is_inside(double *coor_utm);

/* Return closest point within GTL to target point */
int gtl_closest_point(double *coor_utm, double *closest_coor_utm, 
		      double *dist_2d);

/* Get transition depth */
double gtl_get_transition();

/* Get transition depth adjusted for topo gap */
double gtl_get_adj_transition(double topo_gap);

/* Read GTL from flat file */
int gtl_setup(char *file_path);

/* Density derived from Vp via Nafe-Drake curve, Brocher (2005) eqn 1. */
double nafe_drake_rho(double f);

/* Vp derived from Vs via Brocher (2005) eqn 9. */
double brocher_vp(double f);

#endif
