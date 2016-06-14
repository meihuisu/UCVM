#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/time.h>
#include "grd.h"
#include "ge_dtypes.h"
#include "ge_config.h"
#include "ucvm_config.h"
#include "ucvm_meta_etree.h"
#include "ucvm_proj_ucvm.h"


/* Default config file */
#define GE_DEFAULT_CFG "grd2etree.conf"

/* Temp files */
#define TMP_ELEV_FILE "./elev.bin"
#define TMP_VS30_FILE "./vs30.bin"

/* Etree cache size */
#define ETREE_CACHE_SIZE 64

/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;


/* Usage function */
void usage() {
  printf("Usage: grd2etree [-h] -f config\n\n");
  printf("Flags:\n");
  printf("\t-f: Configuration file\n");
  printf("\t-h: Help message\n\n");
  printf("Version: %s\n\n", VERSION);

  return;
}


int init_app(const char *cfgfile, ge_cfg_t *cfg)
{

  /* Read in config */
  if (read_config(0, -1, cfgfile, cfg) != UCVM_CODE_SUCCESS) {
    return(UCVM_CODE_ERROR);
  }
  
  disp_config(cfg);

  /* Set file pointers to NULL */
  cfg->ecfg.ep = NULL;
  cfg->ecfg.bufp = NULL;
  cfg->ecfg.num_octants = 0;
  cfg->ecfg.max_octants = 0;

  return(UCVM_CODE_SUCCESS);
}


int main(int argc, char **argv)
{
  int opt;
  char cfgfile[UCVM_MAX_PATH_LEN];
  ge_cfg_t cfg;

  int i, j;
  ucvm_point_t xy, geo;
  int num_points;
  grd_point_t *geo_pnts;
  grd_data_t *griddata;
  ucvm_mpayload_t mapdata;
  float dataf;

  /* Etree parameters */
  char appmeta[UCVM_META_MIN_META_LEN]; 
  char appschema[UCVM_META_MIN_SCHEMA_LEN]; 
  ucvm_meta_map_t appmeta_map;
  etree_addr_t addr;
  etree_tick_t edgetics;

  /* File IO */
  FILE *fp1, *fp2;

  strcpy(cfgfile, "");

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

  /* Use default config if none specified */
  if (strcmp(cfgfile, "") == 0) {
    printf("Using default config file %s\n", GE_DEFAULT_CFG);
    strcpy(cfgfile, GE_DEFAULT_CFG);
  }

  /* Parse configuration */
  if (init_app(cfgfile, &cfg) != 0) {
    fprintf(stderr, "Failed to read configuration file %s.\n", cfgfile);
    return(UCVM_CODE_ERROR);
  }

  /* Create (open) the unpacked etree */
  printf("Opening etree %s\n", cfg.ecfg.outputfile);
  cfg.ecfg.ep = etree_open(cfg.ecfg.outputfile, 
			   O_CREAT|O_TRUNC|O_RDWR, ETREE_CACHE_SIZE, 0, 3);
  if (cfg.ecfg.ep == NULL) {
    fprintf(stderr, "Failed to create the %s etree (unpacked)\n", 
	    cfg.ecfg.outputfile);
    return(UCVM_CODE_ERROR);
  }

  /* Register etree schema */
  printf("Registering schema\n");
  if (ucvm_schema_etree_map_pack(appschema, UCVM_META_MIN_SCHEMA_LEN) != 
      UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to pack UCVM map etree schema\n");
    return(UCVM_CODE_ERROR);
  }
  if (etree_registerschema(cfg.ecfg.ep, appschema) != 0) {
    fprintf(stderr, "%s\n", etree_strerror(etree_errno(cfg.ecfg.ep)));
    return(UCVM_CODE_ERROR);
  }

  /* Allocate buffers */
  num_points = cfg.ecfg.oct_dims.dim[0] * cfg.ecfg.oct_dims.dim[1];
  geo_pnts = malloc(num_points * sizeof(grd_point_t));
  griddata = malloc(num_points * sizeof(grd_data_t));
  if ((geo_pnts == NULL) || (griddata == NULL)) {
    fprintf(stderr, "Failed to allocate point and grid data buffers\n");
    return(UCVM_CODE_ERROR);
  }

  /* Generate 2D grid file in projection and geo coords */
  printf("Creating 2D grid for %d points\n", num_points);
  for (j = 0; j < cfg.ecfg.oct_dims.dim[1]; j++) {
    for (i = 0; i < cfg.ecfg.oct_dims.dim[0]; i++) {
      /* Grid registration since smoothing is performed by UCVM */
      xy.coord[0] = i * cfg.ecfg.octsize;
      xy.coord[1] = j * cfg.ecfg.octsize;
      //xy.coord[0] = (i + 0.5) * cfg.ecfg.octsize;
      //xy.coord[1] = (j + 0.5) * cfg.ecfg.octsize;
      xy.coord[2] = 0.0;

      if (ucvm_proj_ucvm_xy2geo(&(cfg.projinfo.proj), &xy, &geo) != 
      	  UCVM_CODE_SUCCESS) {
      	fprintf(stderr, "Failed to projection point to geo\n");
      	return(UCVM_CODE_ERROR);
      }

      geo_pnts[(j*cfg.ecfg.oct_dims.dim[0])+i].coord[0] = geo.coord[0];
      geo_pnts[(j*cfg.ecfg.oct_dims.dim[0])+i].coord[1] = geo.coord[1];
    }
  }

  /* Extract DEM map */
    printf("Querying DEMs for %d points\n", num_points);
  if (grd_init(cfg.elev_hr_dir, cfg.elev_lr_dir, GRD_HEUR_DEM) != 0) {
      	fprintf(stderr, "Failed to init DEM grid query tool\n");
      	return(UCVM_CODE_ERROR);
  }
  if (grd_query(num_points, geo_pnts, griddata) != 0) {
    fprintf(stderr, "Failed to query DEMs\n");
    return(UCVM_CODE_ERROR);
  }
  grd_finalize();

  /* Write elevations to disk */
  fp1 = fopen(TMP_ELEV_FILE, "wb");
  for (i = 0; i < num_points; i++) {
    if (griddata[i].valid == 0) {
      fprintf(stderr, "No elevation data at %lf, %lf\n",
	      geo_pnts[i].coord[0], geo_pnts[i].coord[1]);
      return(UCVM_CODE_ERROR);
    }
    dataf = griddata[i].data;
    if (fwrite(&dataf, sizeof(float), 1, fp1) != 1) {
      fprintf(stderr, "Failed to write point to disk\n");
      return(UCVM_CODE_ERROR);
    }
  }
  fclose(fp1);

  /* Extract Vs30 map */
  printf("Querying Vs30 Maps for %d points\n", num_points);
  if (grd_init(cfg.vs30_hr_dir, cfg.vs30_lr_dir, GRD_HEUR_VS30) != 0) {
      	fprintf(stderr, "Failed to init Vs30 grid query tool\n");
      	return(UCVM_CODE_ERROR);
  }
  if (grd_query(num_points, geo_pnts, griddata) != 0) {
      	fprintf(stderr, "Failed to query Vs30 maps\n");
      	return(UCVM_CODE_ERROR);
  }
  grd_finalize();

  /* Write vs30s to disk */
  fp1 = fopen(TMP_VS30_FILE, "wb");
  for (i = 0; i < num_points; i++) {
    if ((griddata[i].valid == 0) || (griddata[i].data <= 0.0)) {
      fprintf(stderr, "No vs30 data at (%d) %lf, %lf\n",
	      i, geo_pnts[i].coord[0], geo_pnts[i].coord[1]);
      return(UCVM_CODE_ERROR);
    }
    dataf = griddata[i].data;
    if (fwrite(&dataf, sizeof(float), 1, fp1) != 1) {
      fprintf(stderr, "Failed to write point to disk\n");
      return(UCVM_CODE_ERROR);
    }
  }
  fclose(fp1);

  /* Free data buffers */
  free(geo_pnts);
  free(griddata);

  /* Combine data into Etree map */
  printf("Inserting data into etree\n");
  addr.level = cfg.ecfg.level;
  addr.type = ETREE_LEAF;
  edgetics = (etree_tick_t)1 << (ETREE_MAXLEVEL - cfg.ecfg.level);
  fp1 = fopen(TMP_ELEV_FILE, "rb");
  fp2 = fopen(TMP_VS30_FILE, "rb");
  for (j = 0; j < cfg.ecfg.oct_dims.dim[1]; j++) {
    for (i = 0; i < cfg.ecfg.oct_dims.dim[0]; i++) {
      /* Read data points */
      fread(&(mapdata.surf), sizeof(float), 1, fp1);
      fread(&(mapdata.vs30), sizeof(float), 1, fp2);
      /* Insert into etree */
      addr.x = i * edgetics;
      addr.y = j * edgetics;
      addr.z = 0;
      if (etree_insert(cfg.ecfg.ep, addr, &mapdata) != 0)  {
	fprintf(stderr, "Failed to insert point %d,%d into etree - %s\n",
		i, j, etree_strerror(etree_errno(cfg.ecfg.ep)));
	return(UCVM_CODE_ERROR);
      }
    }
  }
  fclose(fp1);
  fclose(fp2);
  //unlink(TMP_ELEV_FILE);
  //unlink(TMP_VS30_FILE);

  /* Apply the metadata to the etree */
  printf("Setting application metadata\n");
  strcpy(appmeta_map.title, cfg.ecfg.title);
  strcpy(appmeta_map.author, cfg.ecfg.author);
  strcpy(appmeta_map.date, cfg.ecfg.date);
  appmeta_map.spacing = cfg.spacing;
  strcpy(appmeta_map.projstr, cfg.projinfo.projstr);
  memcpy(&appmeta_map.origin, &(cfg.projinfo.corner), 
  	   sizeof(ucvm_point_t));
  appmeta_map.rot = cfg.projinfo.rot;
  memcpy(&appmeta_map.dims_xyz, &(cfg.projinfo.dims), 
  	 sizeof(ucvm_point_t));
  memcpy(&appmeta_map.ticks_xyz, &cfg.ecfg.max_ticks, 
  	 sizeof(unsigned int)*3);
  if (ucvm_meta_etree_map_pack(&appmeta_map, appmeta, 
  				UCVM_META_MIN_META_LEN) != 
      UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to form meta string\n");
    return(UCVM_CODE_ERROR);
  }
  
  /* Set metadata */
  printf("Saving metadata: %s\n", appmeta);
  if (etree_setappmeta(cfg.ecfg.ep, appmeta) != 0) {
    fprintf(stderr, "%s\n", etree_strerror(etree_errno(cfg.ecfg.ep)));
    return(UCVM_CODE_ERROR);
  }

  /* Close the etree */
  printf("Closing etree\n");
  if (etree_close(cfg.ecfg.ep) != 0) {
      fprintf(stderr, "Error closing etree\n");
      return(UCVM_CODE_ERROR);
  }

  /* Finalize projection */
  ucvm_proj_ucvm_finalize(&(cfg.projinfo.proj));

  return(UCVM_CODE_SUCCESS);
}
