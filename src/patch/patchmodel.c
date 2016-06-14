#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/time.h>
#include "patch_config.h"
#include "ucvm_proj_ucvm.h"
#include "ucvm_meta_patch.h"


/* Default config file */
#define PATCH_DEFAULT_CFG "patch.conf"


/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;


/* Usage function */
void usage() 
{
  printf("Usage: patchmodel [-h] -f config\n\n");
  printf("Flags:\n");
  printf("\t-f: Configuration file\n");
  printf("\t-h: Help message\n\n");

  printf("Version: %s\n\n", VERSION);
  return;
}


int init_app(const char *cfgfile, patch_cfg_t *cfg)
{
  int i, j;

  /* Read in config */
  if (read_config(cfgfile, cfg) != UCVM_CODE_SUCCESS) {
    return(UCVM_CODE_ERROR);
  }
  
  disp_config(cfg);

  /* Setup surface dimensions */
  ucvm_meta_patch_getsurf(&(cfg->projinfo.dims), cfg->spacing, 
			  cfg->surfs);

  printf("Surface Information:\n");
  for (j = 0; j < 2; j++) {
    for (i = 0; i < 2; i++) {
      printf("\tSurf(%d,%d) num_points=%d\n", i,j, 
	     cfg->surfs[j][i].num_points);
      printf("\tSurf(%d,%d) p1->p2: %lf,%lf,%lf -> %lf,%lf,%lf\n",
	     i,j, 
	     cfg->surfs[j][i].corners[0].coord[0],
	     cfg->surfs[j][i].corners[0].coord[1],
	     cfg->surfs[j][i].corners[0].coord[2],
	     cfg->surfs[j][i].corners[1].coord[0],
	     cfg->surfs[j][i].corners[1].coord[1],
	     cfg->surfs[j][i].corners[1].coord[2]);
    }
  }
  printf("\n");
  
  /* Setup UCVM */
  printf("Setting up UCVM\n");
  if (ucvm_init(cfg->ucvmconf) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to initialize UCVM\n");
    return(UCVM_CODE_ERROR);
  }

  /* Set depth query mode */
  if (ucvm_setparam(UCVM_PARAM_QUERY_MODE,
		    UCVM_COORD_GEO_DEPTH) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Set query mode failed\n");
    return(UCVM_CODE_ERROR);
  }

  /* Add model */
  if (ucvm_add_model_list(cfg->ucvmstr) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to add model list %s\n", cfg->ucvmstr);
    return(UCVM_CODE_ERROR);
  }

  /* Set interpolation z range */
  if (ucvm_setparam(UCVM_PARAM_IFUNC_ZRANGE, 
		    cfg->ucvm_zrange[0], 
		    cfg->ucvm_zrange[1]) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to set interpolation z range\n");
    return(UCVM_CODE_ERROR);
  }
  
  return(UCVM_CODE_SUCCESS);
}


/* Extract surface info */
int extract_surfs(patch_cfg_t *cfg) 
{
  int i, j, n;
  double x, y, z;

  /* Extraction */
  ucvm_point_t *cvm_pnts[2][2];
  ucvm_data_t *data[2][2];
  int num_points;
  ucvm_point_t proj_pnt;

  /* Allocate buffers and generate surface grids*/
  printf("Allocating buffers and generating surface grids\n");
  for (j = 0; j < 2; j++) {
    for (i = 0; i < 2; i++) {
      cvm_pnts[j][i] = malloc(cfg->surfs[j][i].num_points * 
			      sizeof(ucvm_point_t));
      data[j][i] = malloc(cfg->surfs[j][i].num_points * 
			  sizeof(ucvm_data_t));
      if ((cvm_pnts[j][i] == NULL) || (data[j][i] == NULL)) {
	fprintf(stderr, 
		"Failed to allocate buffers for surface %d, %d\n", 
		i,j);
	return(UCVM_CODE_ERROR);
      }
      
      /* Compute proj/cvm points */
      num_points = 0;
      for (z = cfg->surfs[j][i].corners[0].coord[2]; 
	   z < cfg->surfs[j][i].corners[1].coord[2]; 
	   z = z + cfg->spacing) {
	for (y = cfg->surfs[j][i].corners[0].coord[1]; 
	     y < cfg->surfs[j][i].corners[1].coord[1]; 
	     y = y + cfg->spacing) {
	  for (x = cfg->surfs[j][i].corners[0].coord[0]; 
	       x < cfg->surfs[j][i].corners[1].coord[0]; 
	       x = x + cfg->spacing) {
	    /* Cell center registration along x-y plane since no
	     smoothing is performed in the x-y by UCVM */
	    proj_pnt.coord[0] = x + cfg->spacing/2.0;
	    proj_pnt.coord[1] = y + cfg->spacing/2.0;
	    proj_pnt.coord[2] = z;

	    /* Project point into geo coords */
	    if (ucvm_proj_ucvm_xy2geo(&(cfg->projinfo.proj),
				      &(proj_pnt),
				      &(cvm_pnts[j][i][num_points]))
		!= UCVM_CODE_SUCCESS) {
	      fprintf(stderr, "UCVM projection failed for %lf, %lf, %lf\n", 
		      proj_pnt.coord[0], 
		      proj_pnt.coord[1], 
		      proj_pnt.coord[2]);
	      return(UCVM_CODE_ERROR);
	    }
	    num_points++;
	  }
	}
      }
      if (num_points != cfg->surfs[j][i].num_points) {
	fprintf(stderr, 
		"Number of generated points don't match computed number");
	return(UCVM_CODE_ERROR);
      }
    }
  }

  for (j = 0; j < 2; j++) {
    for (i = 0; i < 2; i++) {
      printf("\tSurf(%d,%d): %d points\n", i, j, 
	     cfg->surfs[j][i].num_points);
      if (ucvm_query(cfg->surfs[j][i].num_points, 
		     cvm_pnts[j][i], data[j][i]) != UCVM_CODE_SUCCESS) {
	fprintf(stderr, "Failed to query surface\n");
	return(UCVM_CODE_ERROR);
      }

      for (n = 0; n < cfg->surfs[j][i].num_points; n++) {
	/* Check data for bad values */
	if ((data[j][i][n].cmb.vp <= 0.0) || 
	    (data[j][i][n].cmb.vs <= 0.0) || 
	    (data[j][i][n].cmb.rho <= 0.0)) {
	  fprintf(stderr, 
		  "Invalid props at %lf,%lf,%lf: vp=%lf, vs=%lf, rho=%lf\n",
		  cvm_pnts[j][i][n].coord[0],
		  cvm_pnts[j][i][n].coord[1],
		  cvm_pnts[j][i][n].coord[2],
		  data[j][i][n].cmb.vp, 
		  data[j][i][n].cmb.vs, 
		  data[j][i][n].cmb.rho);
	  return(UCVM_CODE_ERROR);
	}

	/* Copy to payload array */
	cfg->surfs[j][i].props[n].vp = data[j][i][n].cmb.vp;
	cfg->surfs[j][i].props[n].vs = data[j][i][n].cmb.vs;
	cfg->surfs[j][i].props[n].rho = data[j][i][n].cmb.rho;
      }
    }
  }

  /* Free buffers */
  for (j = 0; j < 2; j++) {
    for (i = 0; i < 2; i++) {
      free(cvm_pnts[j][i]);
      free(data[j][i]);
    }
  }

  return(UCVM_CODE_SUCCESS);
}


/* Write surface files */
int write_surfs(patch_cfg_t *cfg) 
{
  int i, j;

  /* File I/O */
  char filename[UCVM_MAX_PATH_LEN];
  FILE *fp;

  printf("Saving surfaces\n");
  for (j = 0; j < 2; j++) {
    for (i = 0; i < 2; i++) {
      sprintf(filename, "%s/%s_%s_%d_%d.bin", 
	      cfg->modelpath, cfg->modelname, "surf", i, j);
      printf("\tSurf(%d,%d): Saving to %s\n", i, j, filename);
      fp = fopen(filename, "wb");
      if (fp == NULL) {
	fprintf(stderr, "Failed to open surf file %s\n", filename);
	return(UCVM_CODE_ERROR);
      }

      /* Write to disk */
      if (fwrite(cfg->surfs[j][i].props, sizeof(ucvm_ppayload_t), 
		 cfg->surfs[j][i].num_points, fp) != 
	  cfg->surfs[j][i].num_points) {
	fprintf(stderr, "Failed to write surf (%d,%d) values\n",i, j);
	return(UCVM_CODE_ERROR);
      }

      fclose(fp);
    }
  }

  return(UCVM_CODE_SUCCESS);
}


/* Write patch conf file */
int write_conf(patch_cfg_t *cfg) {
  int i, j;

  /* File I/O */
  char filename[UCVM_MAX_PATH_LEN];
  char tmpstr[UCVM_CONFIG_MAX_STR];
  FILE *fp;

  sprintf(filename, "%s/%s.conf", cfg->modelpath, cfg->modelname);
  printf("Writing patch conf file %s\n", filename);
  fp = fopen(filename, "wb");
  if (fp == NULL) {
    fprintf(stderr, "Failed to open conf file %s\n", filename);
    return(UCVM_CODE_ERROR);
  }
  sprintf(tmpstr, "# %s patch conf file\n\n", cfg->modelname);
  fwrite(tmpstr, 1, strlen(tmpstr), fp);

  sprintf(tmpstr, "# Version\n");
  fwrite(tmpstr, 1, strlen(tmpstr), fp);
  sprintf(tmpstr, "%s=%s\n", "version", cfg->version);
  fwrite(tmpstr, 1, strlen(tmpstr), fp);

  sprintf(tmpstr, "# Projection\n");
  fwrite(tmpstr, 1, strlen(tmpstr), fp);
  sprintf(tmpstr, "%s=%s\n", "proj", cfg->projinfo.projstr);
  fwrite(tmpstr, 1, strlen(tmpstr), fp);
  sprintf(tmpstr, "%s=%lf\n", "lon_0", cfg->projinfo.corner.coord[0]);
  fwrite(tmpstr, 1, strlen(tmpstr), fp);
  sprintf(tmpstr, "%s=%lf\n", "lat_0", cfg->projinfo.corner.coord[1]);
  fwrite(tmpstr, 1, strlen(tmpstr), fp);
  sprintf(tmpstr, "%s=%lf\n", "rot", cfg->projinfo.rot);
  fwrite(tmpstr, 1, strlen(tmpstr), fp);

  sprintf(tmpstr, "# Spacing\n");
  fwrite(tmpstr, 1, strlen(tmpstr), fp);
  sprintf(tmpstr, "%s=%lf\n", "spacing", cfg->spacing);
  fwrite(tmpstr, 1, strlen(tmpstr), fp);

  sprintf(tmpstr, "# Dimensions\n");
  fwrite(tmpstr, 1, strlen(tmpstr), fp);
  sprintf(tmpstr, "%s=%lf\n", "x-size", cfg->projinfo.dims.coord[0]);
  fwrite(tmpstr, 1, strlen(tmpstr), fp);
  sprintf(tmpstr, "%s=%lf\n", "y-size", cfg->projinfo.dims.coord[1]);
  fwrite(tmpstr, 1, strlen(tmpstr), fp);
  sprintf(tmpstr, "%s=%lf\n", "z-size", cfg->projinfo.dims.coord[2]);
  fwrite(tmpstr, 1, strlen(tmpstr), fp);

  sprintf(tmpstr, "# Paths to surface files\n");
  fwrite(tmpstr, 1, strlen(tmpstr), fp);
  for (j = 0; j < 2; j++) {
    for (i = 0; i < 2; i++) {
      sprintf(filename, "%s/%s_%s_%d_%d.bin", 
	      cfg->modelpath, cfg->modelname, "surf", i, j);
      sprintf(tmpstr, "surf_%d_%d_path=%s\n", i, j, filename);
      fwrite(tmpstr, 1, strlen(tmpstr), fp);
    }
  }

  fclose(fp);

  return(UCVM_CODE_SUCCESS);
}


int main(int argc, char **argv)
{
  int i, j;

  /* Config and options */
  int opt;
  char cfgfile[UCVM_MAX_PATH_LEN];
  patch_cfg_t cfg;

  strcpy(cfgfile, PATCH_DEFAULT_CFG);

  /* Parse options */
  while ((opt = getopt(argc, argv, "f:h")) != -1) {
    switch (opt) {
    case 'f':
      strcpy(cfgfile, optarg);
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

  /* Parse configuration */
  if (init_app(cfgfile, &cfg) != 0) {
    fprintf(stderr, "Failed to read configuration file %s.\n", cfgfile);
    return(UCVM_CODE_ERROR);
  }

  /* Allocate patch payload buffers */
  for (j = 0; j < 2; j++) {
    for (i = 0; i < 2; i++) {
      cfg.surfs[j][i].props = malloc(cfg.surfs[j][i].num_points *
				     sizeof(ucvm_ppayload_t)); 
      if ((cfg.surfs[j][i].props == NULL)) {
	fprintf(stderr, 
		"Failed to allocate buffers for surface %d, %d\n", 
		i,j);
	return(UCVM_CODE_ERROR);
      }
    }
  }

  /* Extract each surface */
  printf("Extracting surfaces\n");
  if (extract_surfs(&cfg) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to extract surface info\n");
    return(UCVM_CODE_ERROR);
  }

  /* Write surfaces to model path */
  if (write_surfs(&cfg) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to write surface files\n");
    return(UCVM_CODE_ERROR);
  }

  /* Write patch conf file */
  if (write_conf(&cfg) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to write patch conf file\n");
    return(UCVM_CODE_ERROR);
  }

  /* Finalize UCVM */
  ucvm_finalize();

  /* Finalize projection */
  ucvm_proj_ucvm_finalize(&(cfg.projinfo.proj));

  /* Free buffers */
  for (j = 0; j < 2; j++) {
    for (i = 0; i < 2; i++) {
      free(cfg.surfs[j][i].props);
    }
  }

  return(UCVM_CODE_SUCCESS);
}
