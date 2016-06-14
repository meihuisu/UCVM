#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netcdf.h>
#include "ucvm.h"
#include "um_mesh.h"

/* Valid ranges for properties */
#define MIN_VP_VALUE -9000
#define MAX_VP_VALUE 9000
#define MIN_VS_VALUE -7500
#define MAX_VS_VALUE 7500
#define MIN_RHO_VALUE -6000
#define MAX_RHO_VALUE 6000

/*
#define MIN_VP_VALUE 0
#define MAX_VP_VALUE 9000
#define MIN_VS_VALUE 0
#define MAX_VS_VALUE 7500
#define MIN_RHO_VALUE 0
#define MAX_RHO_VALUE 6000
*/

/* Rank (number of dimensions) for each variable */
#define RANK_depth 1
#define RANK_longitude 2
#define RANK_latitude 2
#define RANK_Vp 3
#define RANK_Vs 3
#define RANK_density 3


/* Usage */
void usage(char *arg)
{
  printf("Usage: %s inmesh ingrid format spacing nx ny nz outfile\n\n", arg);
  printf("where:\n");
  printf("\tinmesh: input mesh file\n");
  printf("\tingrid: input grid file\n");
  printf("\tformat: format of file, IJK-12, IJK-20, IJK-32, SORD\n");
  printf("\tspacing: mesh spacing\n");
  printf("\tnx,ny,nz: mesh dimensions\n");
  printf("\toutfile: output mesh file\n\n");

  printf("Version: %s\n\n", VERSION);
}


/* Read in grid points from grid file */
int get_grid(char *file, size_t nx, size_t ny, ucvm_point_t *grid)
{
  size_t num_grid;
  FILE *ifp;
  size_t retval;

  num_grid = nx * ny;
  ifp = fopen(file, "rb");
  if (ifp == NULL) {
    fprintf(stderr, "Failed to open grid file\n");
    return(1);
  }
  retval = fread(grid, sizeof(ucvm_point_t), num_grid, ifp);
  fclose(ifp);

  if (retval != num_grid) {
    fprintf(stderr, "Failed to read grid file\n");
    return(1);
  }

  return(0);
}


/* Read in material properties from mesh, supports a variety
   of formats. Somewhat inefficient for non IJK-12 formats.  */
int get_props(char *file, mesh_format_t format, 
	      size_t nx, size_t ny, size_t nz, 
	      mesh_ijk12_t *props)
{
  size_t i;
  size_t num_grid;
  FILE *ifp1, *ifp2, *ifp3;
  char tmpfile[256];
  size_t retval;
  mesh_sord_t prop_sord_vp, prop_sord_vs, prop_sord_rho;
  mesh_ijk20_t prop_ijk20;
  mesh_ijk32_t prop_ijk32;

  num_grid = nx * ny * nz;

  /* Read mesh according to format */
  switch (format) {
  case MESH_FORMAT_IJK12:
    ifp1 = fopen(file, "rb");
    if (ifp1 == NULL) {
      fprintf(stderr, "Failed to open mesh file\n");
      return(1);
    }
    retval = fread(props, sizeof(mesh_ijk12_t), num_grid, ifp1);
    fclose(ifp1);
    if (retval != num_grid) {
      fprintf(stderr, "Failed to read mesh file\n");
      return(1);
    }
    break;
  case MESH_FORMAT_IJK20:
    ifp1 = fopen(file, "rb");
    if (ifp1 == NULL) {
      fprintf(stderr, "Failed to open mesh file\n");
      return(1);
    }
    for (i = 0; i < num_grid; i++) {
      if (fread(&prop_ijk20, sizeof(mesh_ijk20_t), 1, ifp1) != 1) {
	fprintf(stderr, "Failed to read mesh file\n");
	fclose(ifp1);
	return(1);
      }
      props[i].vp = prop_ijk20.vp;
      props[i].vs = prop_ijk20.vs;
      props[i].rho = prop_ijk20.rho;
    }
    fclose(ifp1);
    break;
  case MESH_FORMAT_IJK32:
    ifp1 = fopen(file, "rb");
    if (ifp1 == NULL) {
      fprintf(stderr, "Failed to open mesh file\n");
      return(1);
    }
    for (i = 0; i < num_grid; i++) {
      if (fread(&prop_ijk32, sizeof(mesh_ijk32_t), 1, ifp1) != 1) {
	fprintf(stderr, "Failed to read mesh file\n");
	fclose(ifp1);
	return(1);
      }
      props[i].vp = prop_ijk32.vp;
      props[i].vs = prop_ijk32.vs;
      props[i].rho = prop_ijk32.rho;
    }
    fclose(ifp1);
    break;
  case MESH_FORMAT_SORD:
    sprintf(tmpfile, "%s_vp", file);
    ifp1 = fopen(tmpfile, "rb");
    sprintf(tmpfile, "%s_vs", file);
    ifp2 = fopen(tmpfile, "rb");
    sprintf(tmpfile, "%s_rho", file);
    ifp3 = fopen(tmpfile, "rb");
    if ((ifp1 == NULL) || (ifp2 == NULL) || (ifp3 == NULL)) {
      fprintf(stderr, "Failed to open mesh file\n");
      return(1);
    }
    for (i = 0; i < num_grid; i++) {
      if (fread(&prop_sord_vp, sizeof(mesh_sord_t), 1, ifp1) != 1) {
	fprintf(stderr, "Failed to read vp mesh file\n");
	fclose(ifp1);
	fclose(ifp2);
	fclose(ifp3);
	return(1);
      }
      props[i].vp = prop_sord_vp.val;
      if (fread(&prop_sord_vs, sizeof(mesh_sord_t), 1, ifp2) != 1) {
	fprintf(stderr, "Failed to read vs mesh file\n");
	fclose(ifp1);
	fclose(ifp2);
	fclose(ifp3);
	return(1);
      }
      props[i].vs = prop_sord_vs.val;
      if (fread(&prop_sord_rho, sizeof(mesh_sord_t), 1, ifp3) != 1) {
	fprintf(stderr, "Failed to read rho mesh file\n");
	fclose(ifp1);
	fclose(ifp2);
	fclose(ifp3);
	return(1);
      }
      props[i].rho = prop_sord_rho.val;
    }
    fclose(ifp1);
    fclose(ifp2);
    fclose(ifp3);
    break;
  default:
    fprintf(stderr, "Invalid mesh format\n");
    return(1);
  }

  return(0);
}


/* Net2CDF error handler */
void check_err(const int stat, const int line, const char *file) {
  if (stat != NC_NOERR) {
    (void)fprintf(stderr,"line %d of %s: %s\n", line, file, 
		  nc_strerror(stat));
    fflush(stderr);
    exit(1);
  }
}


int main(int argc, char **argv) {
  int i, j, k;
  int  stat;  /* return status */
  int  ncid;  /* netCDF id */
  
  /* dimension ids */
  int nlon_dim;
  int nlat_dim;
  int depth_dim;
  
  /* variable ids */
  int depth_id;
  int longitude_id;
  int latitude_id;
  int Vp_id;
  int Vs_id;
  int density_id;
  
  /* variable shapes */
  int depth_dims[RANK_depth];
  int longitude_dims[RANK_longitude];
  int latitude_dims[RANK_latitude];
  int Vp_dims[RANK_Vp];
  int Vs_dims[RANK_Vs];
  int density_dims[RANK_density];

  /* mesh, grid variables */
  ucvm_point_t *grid = NULL;
  mesh_ijk12_t *props = NULL;

  /* variable data */
  float *depth_data, *longitude_data, *latitude_data;
  float *Vp_data, *Vs_data, *density_data;

  /* Arguments */
  char inmesh[256], ingrid[256], outfile[256], formatstr[256];
  int spacing;
  size_t nx, ny, nz;
  mesh_format_t format;

  /* Parse args */
  if(argc != 9) {
    usage(argv[0]);
    exit(1);
  }

  format = MESH_FORMAT_UNKNOWN;
  strcpy(inmesh, argv[1]);
  strcpy(ingrid, argv[2]);
  strcpy(formatstr, argv[3]);
  spacing = atoi(argv[4]);
  nx = atoi(argv[5]);
  ny = atoi(argv[6]);
  nz = atoi(argv[7]);
  strcpy(outfile, argv[8]);

  /* Check arguments */
  for (i = 0; i < MAX_MESH_FORMATS; i++) {
    if (strcmp(formatstr, MESH_FORMAT_NAMES[i]) == 0) {
      format = i;
      break;
    }
  }
  if (format == MESH_FORMAT_UNKNOWN) {
    fprintf(stderr, "Unsupported mesh format %s\n", formatstr);
    exit(1);
  }
  if ((nx <= 0) || (ny <= 0) || (nz <= 0)) {
    fprintf(stderr, "Invalid mesh dims %lud,%lud,%lud\n", nx, ny, nz);
    exit(1);
  }
  if (spacing <= 0) {
    fprintf(stderr, "Invalid spacing %d\n", spacing);
    exit(1);
  }

  /* enter define mode */
  stat = nc_create(outfile, NC_CLOBBER, &ncid);
  check_err(stat,__LINE__,__FILE__);

  /* define dimensions */
  stat = nc_def_dim(ncid, "nlon", nx, &nlon_dim);
  check_err(stat,__LINE__,__FILE__);
  stat = nc_def_dim(ncid, "nlat", ny, &nlat_dim);
  check_err(stat,__LINE__,__FILE__);
  stat = nc_def_dim(ncid, "depth", nz, &depth_dim);
  check_err(stat,__LINE__,__FILE__);

  /* define variables */
  depth_dims[0] = depth_dim;
  stat = nc_def_var(ncid, "depth", NC_FLOAT, RANK_depth, 
		    depth_dims, &depth_id);
  check_err(stat,__LINE__,__FILE__);

  longitude_dims[0] = nlat_dim;
  longitude_dims[1] = nlon_dim;
  stat = nc_def_var(ncid, "longitude", NC_FLOAT, RANK_longitude, 
		    longitude_dims, &longitude_id);
  check_err(stat,__LINE__,__FILE__);
  
  latitude_dims[0] = nlat_dim;
  latitude_dims[1] = nlon_dim;
  stat = nc_def_var(ncid, "latitude", NC_FLOAT, RANK_latitude, 
		    latitude_dims, &latitude_id);
  check_err(stat,__LINE__,__FILE__);
  
  Vp_dims[0] = depth_dim;
  Vp_dims[1] = nlat_dim;
  Vp_dims[2] = nlon_dim;
  stat = nc_def_var(ncid, "Vp", NC_FLOAT, RANK_Vp, Vp_dims, &Vp_id);
  check_err(stat,__LINE__,__FILE__);

  Vs_dims[0] = depth_dim;
  Vs_dims[1] = nlat_dim;
  Vs_dims[2] = nlon_dim;
  stat = nc_def_var(ncid, "Vs", NC_FLOAT, RANK_Vs, Vs_dims, &Vs_id);
  check_err(stat,__LINE__,__FILE__);

  density_dims[0] = depth_dim;
  density_dims[1] = nlat_dim;
  density_dims[2] = nlon_dim;
  stat = nc_def_var(ncid, "density", NC_FLOAT, RANK_density, 
		    density_dims, &density_id);
  check_err(stat,__LINE__,__FILE__);

  /* assign per-variable attributes */

  /* units */
  stat = nc_put_att_text(ncid, depth_id, "units", 5, "meter");
  check_err(stat,__LINE__,__FILE__);

  /* positive */
  stat = nc_put_att_text(ncid, depth_id, "positive", 4, "down");
  check_err(stat,__LINE__,__FILE__);

  /* long_name */
  stat = nc_put_att_text(ncid, longitude_id, "long_name", 24, 
			 "Longitude, positive East");
  check_err(stat,__LINE__,__FILE__);

  /* units */
  stat = nc_put_att_text(ncid, longitude_id, "units", 12, 
			 "degrees_east");
  check_err(stat,__LINE__,__FILE__);

  /* standard_name */
  stat = nc_put_att_text(ncid, longitude_id, "standard_name", 9, 
			 "longitude");
  check_err(stat,__LINE__,__FILE__);

  /* long_name */
  stat = nc_put_att_text(ncid, latitude_id, "long_name", 24, 
			 "Latitude, positive north");
  check_err(stat,__LINE__,__FILE__);

  /* units */
  stat = nc_put_att_text(ncid, latitude_id, "units", 13, 
			 "degrees_north");
  check_err(stat,__LINE__,__FILE__);

  /* standard_name */
  stat = nc_put_att_text(ncid, latitude_id, "standard_name", 8, 
			 "latitude");
  check_err(stat,__LINE__,__FILE__);

  /* long_name */
  stat = nc_put_att_text(ncid, Vp_id, "long_name", 15, 
			 "P wave velocity");
  check_err(stat,__LINE__,__FILE__);

  /* valid_range */
  static const float Vp_valid_range_att[2] = {MIN_VP_VALUE, MAX_VP_VALUE} ;
  stat = nc_put_att_float(ncid, Vp_id, "valid_range", NC_FLOAT, 2, 
			  Vp_valid_range_att);
  check_err(stat,__LINE__,__FILE__);

  /* units */
  stat = nc_put_att_text(ncid, Vp_id, "units", 12, "meter sec^-1");
  check_err(stat,__LINE__,__FILE__);

  /* coordinates */
  stat = nc_put_att_text(ncid, Vp_id, "coordinates", 24, 
			 "longitude latitude depth");
  check_err(stat,__LINE__,__FILE__);

  /* long_name */
  stat = nc_put_att_text(ncid, Vs_id, "long_name", 15, 
			 "S wave velocity");
  check_err(stat,__LINE__,__FILE__);

  /* valid_range */
  static const float Vs_valid_range_att[2] = {MIN_VS_VALUE, MAX_VS_VALUE} ;
  stat = nc_put_att_float(ncid, Vs_id, "valid_range", NC_FLOAT, 2, 
			  Vs_valid_range_att);
  check_err(stat,__LINE__,__FILE__);

  /* units */
  stat = nc_put_att_text(ncid, Vs_id, "units", 12, "meter sec^-1");
  check_err(stat,__LINE__,__FILE__);

  /* coordinates */
  stat = nc_put_att_text(ncid, Vs_id, "coordinates", 24, 
			 "longitude latitude depth");
  check_err(stat,__LINE__,__FILE__);

  /* long_name */
  stat = nc_put_att_text(ncid, density_id, "long_name", 7, 
			 "density");
  check_err(stat,__LINE__,__FILE__);

  /* valid_range */
  static const float density_valid_range_att[2] = 
    {MIN_RHO_VALUE, MAX_RHO_VALUE} ;
  stat = nc_put_att_float(ncid, density_id, "valid_range", NC_FLOAT, 2, 
			  density_valid_range_att);
  check_err(stat,__LINE__,__FILE__);

  /* units */
  stat = nc_put_att_text(ncid, density_id, "units", 17, 
			 "kilogram meter^-3");
  check_err(stat,__LINE__,__FILE__);

  /* coordinates */
  stat = nc_put_att_text(ncid, density_id, "coordinates", 24, 
			 "longitude latitude depth");
  check_err(stat,__LINE__,__FILE__);
  
  /* leave define mode */
  stat = nc_enddef (ncid);
  check_err(stat,__LINE__,__FILE__);
    
  /* Allocate buffers */
  grid = malloc(nx * ny * sizeof(ucvm_point_t));
  props = malloc(nx * ny * nz * sizeof(mesh_ijk12_t));
  if ((grid == NULL) || (props == NULL)) {
    fprintf(stderr, "Failed to allocate mesh buffers\n");
    return(1);
  }

  latitude_data = malloc(nx * ny * sizeof(float));
  longitude_data = malloc(nx * ny * sizeof(float));
  depth_data = malloc(nz * sizeof(float));
  Vp_data = malloc(nx * ny * nz * sizeof(float));
  Vs_data = malloc(nx * ny * nz * sizeof(float));  
  density_data = malloc(nx * ny * nz * sizeof(float));
  
  /* Get grid points */
  if (get_grid(ingrid, nx, ny, grid) != 0) {
    fprintf(stderr, "Failed to get grid\n");
    return(1);
  }
  
  /* Get mesh values */
  if (get_props(inmesh, format, nx, ny, nz, props) != 0) {
    fprintf(stderr, "Failed to get mesh\n");
      return(1);
  }

  /* assign variable data */

  /* Store depth */
  for (i = 0; i < nz; i++) {
    depth_data[i] = i * spacing;
  }
  size_t depth_startset[1] = {0} ;
  size_t depth_countset[1] = {nz} ;
  stat = nc_put_vara(ncid, depth_id, depth_startset, depth_countset, 
		     depth_data);
  stat = nc_put_vara(ncid, depth_id, depth_startset, depth_countset, 
		     depth_data);
  check_err(stat,__LINE__,__FILE__);
  
  /* Store longitude */
  for (j = 0; j < ny; j++) {
    for (i = 0; i < nx; i++) {
      longitude_data[j*nx+i] = grid[j*nx+i].coord[0];
    }
  }
  size_t longitude_startset[2] = {0, 0} ;
  size_t longitude_countset[2] = {ny, nx} ;
  stat = nc_put_vara(ncid, longitude_id, longitude_startset, 
		     longitude_countset, longitude_data);
  stat = nc_put_vara(ncid, longitude_id, longitude_startset, 
		     longitude_countset, longitude_data);
  check_err(stat,__LINE__,__FILE__);
  
  /* Store latitudes */
  for (j = 0; j < ny; j++) {
    for (i = 0; i < nx; i++) {
      latitude_data[j*nx+i] = grid[j*nx+i].coord[1];
    }
  }
  size_t latitude_startset[2] = {0, 0} ;
  size_t latitude_countset[2] = {ny, nx} ;
  stat = nc_put_vara(ncid, latitude_id, latitude_startset, 
		     latitude_countset, latitude_data);
  stat = nc_put_vara(ncid, latitude_id, latitude_startset, 
		     latitude_countset, latitude_data);
  check_err(stat,__LINE__,__FILE__);
  
  /* Store Vp */
  for (k = 0; k < nz; k++) {
    for (j = 0; j < ny; j++) {
      for (i = 0; i < nx; i++) {
	Vp_data[k*(nx*ny)+j*(nx)+i] = 
	  props[k*(nx*ny)+j*(nx)+i].vp;
      }
    }
  }
  size_t Vp_startset[3] = {0, 0, 0} ;
  size_t Vp_countset[3] = {nz, ny, nx} ;
  stat = nc_put_vara(ncid, Vp_id, Vp_startset, Vp_countset, Vp_data);
  stat = nc_put_vara(ncid, Vp_id, Vp_startset, Vp_countset, Vp_data);
  check_err(stat,__LINE__,__FILE__);
  
  /* Store Vs */
  for (k = 0; k < nz; k++) {
    for (j = 0; j < ny; j++) {
      for (i = 0; i < nx; i++) {
	Vs_data[k*(nx*ny)+j*(nx)+i] = 
	  props[k*(nx*ny)+j*(nx)+i].vs;
      }
    }
  }
  size_t Vs_startset[3] = {0, 0, 0} ;
  size_t Vs_countset[3] = {nz, ny, nx} ;
  stat = nc_put_vara(ncid, Vs_id, Vs_startset, Vs_countset, Vs_data);
  stat = nc_put_vara(ncid, Vs_id, Vs_startset, Vs_countset, Vs_data);
  check_err(stat,__LINE__,__FILE__);

  /* Store density */
  for (k = 0; k < nz; k++) {
    for (j = 0; j < ny; j++) {
      for (i = 0; i < nx; i++) {
	density_data[k*(nx*ny)+j*(nx)+i] = 
	  props[k*(nx*ny)+j*(nx)+i].rho;
      }
    }
  }
  size_t density_startset[3] = {0, 0, 0} ;
  size_t density_countset[3] = {nz, ny, nx} ;
  stat = nc_put_vara(ncid, density_id, density_startset, 
		     density_countset, density_data);
  stat = nc_put_vara(ncid, density_id, density_startset, 
		     density_countset, density_data);
  check_err(stat,__LINE__,__FILE__);
  
  /* Free memory */
  free(depth_data);
  free(longitude_data);
  free(latitude_data);
  free(Vp_data);
  free(Vs_data);
  free(density_data);
  free(grid);
  free(props);
  
  stat = nc_close(ncid);
  check_err(stat,__LINE__,__FILE__);
  return 0;
}

