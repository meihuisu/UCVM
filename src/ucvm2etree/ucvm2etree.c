#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/time.h>
#include "ue_extract.h"
#include "ue_config.h"
#include "ucvm_meta_etree.h"


/* Default config file */
#define UE_DEFAULT_CFG "ucvm2etree.conf"


/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;


/* Usage function */
void usage() {
  printf("Usage: ucvm2etree [-h] -f config\n\n");
  printf("Flags:\n");
  printf("\t-f: Configuration file\n");
  printf("\t-h: Help message\n\n");

  printf("Version: %s\n\n", VERSION);
  return;
}


int init_app(const char *cfgfile, ue_cfg_t *cfg)
{
  int i;

  /* Read in config */
  if (read_config(0, -1, cfgfile, cfg) != UCVM_CODE_SUCCESS) {
    return(UCVM_CODE_ERROR);
  }
  
  disp_config(cfg);

  /* Max edgesize must be equal or greater than min edgesize */
  if (cfg->ecfg.max_edgesize < cfg->ecfg.min_edgesize) {
    fprintf(stderr, "Min edge size larger than max\n");
    return(UCVM_CODE_ERROR);
  }

  /* Columns must be square */
  if (cfg->ecfg.col_ticks[0] != cfg->ecfg.col_ticks[1]) {
    fprintf(stderr, "Column length and width must be equal, adjust nx/ny.\n");
    return(UCVM_CODE_ERROR);
  }

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
  
  /* Set file pointers to NULL */
  for (i = 0; i < 2; i++) {
    cfg->ecfg.ep[i] = NULL;
    cfg->ecfg.efp[i] = NULL;
    cfg->ecfg.bufp[i] = NULL;
    cfg->ecfg.num_octants[i] = 0;
  }
  cfg->ecfg.max_octants = 0;

  return(UCVM_CODE_SUCCESS);
}


int main(int argc, char **argv)
{
  int i, j;
  int opt;
  char cfgfile[UCVM_MAX_PATH_LEN];
  ue_cfg_t cfg;
  size_t num_extracted, total_extracted;

  /* Etree parameters */
  char appmeta[UCVM_META_MIN_META_LEN]; 
  char appschema[UCVM_META_MIN_SCHEMA_LEN]; 
  ucvm_meta_cmu_t appmeta_cmu;
  ucvm_meta_ucvm_t appmeta_ucvm;

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
    printf("Using default config file %s", UE_DEFAULT_CFG);
    strcpy(cfgfile, UE_DEFAULT_CFG);
  }

  /* Parse configuration */
  if (init_app(cfgfile, &cfg) != 0) {
    fprintf(stderr, "Failed to initialize application.\n");
    return(UCVM_CODE_ERROR);
  }

  /* Create (open) the unpacked etree */
  printf("Opening etree %s\n", cfg.ecfg.outputfile);
  cfg.ecfg.ep[0] = etree_open(cfg.ecfg.outputfile, 
			   O_CREAT|O_TRUNC|O_RDWR, cfg.buf_etree_cache, 0, 3);
  if (cfg.ecfg.ep[0] == NULL) {
    fprintf(stderr, "Failed to create the %s etree (unpacked)\n", 
	    cfg.ecfg.outputfile);
    return(UCVM_CODE_ERROR);
  }

  /* Register etree schema */
  printf("Registering schema\n");
  if (strcmp(cfg.projinfo.projstr, PROJ_GEO_BILINEAR) == 0) {
    if (ucvm_schema_etree_cmu_pack(appschema, UCVM_META_MIN_SCHEMA_LEN) != 
	UCVM_CODE_SUCCESS) {
      fprintf(stderr, "Failed to pack CMU etree schema\n");
      return(UCVM_CODE_ERROR);
    }
  } else {
    if (ucvm_schema_etree_ucvm_pack(appschema, UCVM_META_MIN_SCHEMA_LEN) != 
	UCVM_CODE_SUCCESS) {
      fprintf(stderr, "Failed to pack UCVM etree schema\n");
      return(UCVM_CODE_ERROR);
    }
  }
  if (etree_registerschema(cfg.ecfg.ep[0], appschema) != 0) {
    fprintf(stderr, "%s\n", etree_strerror(etree_errno(cfg.ecfg.ep[0])));
    return(UCVM_CODE_ERROR);
  }

  /* Allocate buffers */
  ucvm_point_t *cvm_pnts = NULL;
  etree_addr_t *etree_pnts = NULL;
  ucvm_data_t *props = NULL;
  etree_tick_t maxrez, max_points;
  
  maxrez = (etree_tick_t)1 << (ETREE_MAXLEVEL - cfg.ecfg.max_level);
  max_points = (cfg.ecfg.col_ticks[0]/maxrez) * 
    (cfg.ecfg.col_ticks[1]/maxrez);
  printf("Allocating buffers to hold %u points\n", max_points);
  cvm_pnts = malloc(max_points * sizeof(ucvm_point_t));
  etree_pnts = malloc(max_points * sizeof(etree_addr_t));
  props = malloc(max_points * sizeof(ucvm_data_t));
  if ((cvm_pnts == NULL) || (etree_pnts == NULL) || (props == NULL)) {
    fprintf(stderr, "Failed to allocate buffers\n");
    return(UCVM_CODE_ERROR);
  }

  total_extracted = 0;
  for (j = 0; j < cfg.col_dims.dim[1]; j++) {
    for (i = 0; i < cfg.col_dims.dim[0]; i++) {
      printf("Extracting col %d,%d\n", i,j);
      /* Populate the etree */
      if (extract(&cfg, insert_grid_etree, 
		  j*cfg.col_dims.dim[0]+i,
		  cvm_pnts, etree_pnts, props,
		  &num_extracted) != 0) {
	fprintf(stderr, "Extraction failed\n");
	return(UCVM_CODE_ERROR);
      }
      total_extracted = total_extracted + num_extracted;
    }
  }

  printf("Total of %zu octants extracted\n", total_extracted);

  /* Free buffers */
  free(cvm_pnts);
  free(etree_pnts);
  free(props);

  /* Apply the metadata to the etree */
  printf("Setting application metadata\n");
  if (strcmp(cfg.projinfo.projstr, PROJ_GEO_BILINEAR) == 0) {
    strcpy(appmeta_cmu.title, cfg.ecfg.title);
    strcpy(appmeta_cmu.author, cfg.ecfg.author);
    strcpy(appmeta_cmu.date, cfg.ecfg.date);
    memcpy(&appmeta_cmu.origin, &(cfg.projinfo.corner[0]), 
	   sizeof(ucvm_point_t));
    memcpy(&appmeta_cmu.dims_xyz, &(cfg.projinfo.dims), 
	   sizeof(ucvm_point_t));
    memcpy(&appmeta_cmu.ticks_xyz, &cfg.ecfg.max_ticks, 
	   sizeof(unsigned int)*3);
    if (ucvm_meta_etree_cmu_pack(&appmeta_cmu, appmeta, 
				 UCVM_META_MIN_META_LEN) != 
	UCVM_CODE_SUCCESS) {
      fprintf(stderr, "Failed to form meta string\n");
      return(UCVM_CODE_ERROR);
    }
  } else {
    strcpy(appmeta_ucvm.title, cfg.ecfg.title);
    strcpy(appmeta_ucvm.author, cfg.ecfg.author);
    strcpy(appmeta_ucvm.date, cfg.ecfg.date);
    appmeta_ucvm.vs_min = cfg.vs_min;
    appmeta_ucvm.max_freq = cfg.max_freq;
    appmeta_ucvm.ppwl = cfg.ppwl;
    strcpy(appmeta_ucvm.projstr, cfg.projinfo.projstr);
    memcpy(&appmeta_ucvm.origin, &(cfg.projinfo.corner[0]), 
	   sizeof(ucvm_point_t));
    appmeta_ucvm.rot = cfg.projinfo.rot;
    memcpy(&appmeta_ucvm.dims_xyz, &(cfg.projinfo.dims), 
	   sizeof(ucvm_point_t));
    memcpy(&appmeta_ucvm.ticks_xyz, &cfg.ecfg.max_ticks, 
	   sizeof(unsigned int)*3);
    if (ucvm_meta_etree_ucvm_pack(&appmeta_ucvm, appmeta, 
				  UCVM_META_MIN_META_LEN) != 
	UCVM_CODE_SUCCESS) {
      fprintf(stderr, "Failed to form meta string\n");
      return(UCVM_CODE_ERROR);
    }
  }

  /* Set metadata */
  printf("Saving metadata: %s\n", appmeta);
  if (etree_setappmeta(cfg.ecfg.ep[0], appmeta) != 0) {
    fprintf(stderr, "%s\n", etree_strerror(etree_errno(cfg.ecfg.ep[0])));
    return(UCVM_CODE_ERROR);
  }

  /* Close the etree */
  printf("Closing etree\n");
  if (etree_close(cfg.ecfg.ep[0]) != 0) {
      fprintf(stderr, "Error closing etree\n");
      return(UCVM_CODE_ERROR);
  }

  /* Finalize UCVM */
  ucvm_finalize();

  /* Finalize projection */
  if (strcmp(cfg.projinfo.projstr, PROJ_GEO_BILINEAR) != 0) {
    ucvm_proj_ucvm_finalize(&(cfg.projinfo.proj));
  }

  return(UCVM_CODE_SUCCESS);
}
