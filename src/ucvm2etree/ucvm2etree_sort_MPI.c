#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/time.h>
#include "ue_mpi.h"
#include "ue_config.h"
#include "code.h"


/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;


/* Usage function */
void usage() {
  printf("Usage: ucvm2etree-sort-MPI [-h] -f config\n\n");
  printf("Flags:\n");
  printf("\t-f: Configuration file\n");
  printf("\t-h: Help message\n\n");
  printf("Version: %s\n\n", VERSION);

  return;
}


int init_app(int myid, int nproc, const char *cfgfile, ue_cfg_t *cfg)
{
  int i;

  /* Read in config */
  if (read_config(myid, nproc, cfgfile, cfg) != 0) {
    fprintf(stderr, "[%d] Failed to parse config file %s\n", myid, cfgfile);
  }

  if (myid == 0) {
    disp_config(cfg);
  }

  /* Max edgesize must be equal or greater than min edgesize */
  if (cfg->ecfg.max_edgesize < cfg->ecfg.min_edgesize) {
    fprintf(stderr, "[%d] Min edge size larger than max\n", myid);
    return(1);
  }

  /* Columns must be square */
  if (cfg->ecfg.col_ticks[0] != cfg->ecfg.col_ticks[1]) {
    fprintf(stderr, "[%d] Column length and width must be equal\n", myid);
    return(1);
  }

  /* Make sure each rank can get at least one column */
  if (cfg->nproc > cfg->col_dims.dim[0]*cfg->col_dims.dim[1]) {
    fprintf(stderr, 
	    "[%d] Nproc must be <= number of columns\n", myid);
    return(1);
  }

  for (i = 0; i < 2; i++) {
    cfg->ecfg.ep[i] = NULL;
    cfg->ecfg.efp[i] = NULL;
    cfg->ecfg.bufp[i] = NULL;
    cfg->ecfg.num_octants[i] = 0;
  }
  cfg->ecfg.max_octants = 0;

  return(0);
}


int compare_keys(const void *p1, const void *p2)
{    
  ue_octant_t *oct1;
  ue_octant_t *oct2;

  oct1 = (ue_octant_t *)p1;
  oct2 = (ue_octant_t *)p2;
  return(code_comparekey((void *)(oct1->key),
			 (void *)(oct2->key), 
			 (3*sizeof(etree_tick_t))+1));
}


int main(int argc, char **argv)
{
  int i;

  /* MPI stuff and distributed computation variables */
  int myid, nproc, pnlen;
  char procname[128];

  /* Options and config */
  int opt;
  char cfgfile[UCVM_MAX_PATH_LEN];
  ue_cfg_t cfg;

  /* Sort parameters */
  char inputfile[UCVM_MAX_PATH_LEN];
  int num_read;


  /* Init MPI */
  mpi_init(&argc, &argv, &nproc, &myid, procname, &pnlen);

  if (myid == 0) {
    printf("[%d] %s Version: %s (svn %s)\n", myid, argv[0],
           "Unknown", "Unknown");
    printf("[%d] Running on %d cores\n", myid, nproc);
    fflush(stdout);
  }

  /* Nproc must be 2^N cores */
  if (((nproc) & (nproc - 1)) != 0) {
    fprintf(stderr, "[%d] nproc must be 2^N cores\n", myid);
    return(1);
  }

  /* Parse options */
  strcpy(cfgfile, "");
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

  if (strcmp(cfgfile, "") == 0) {
    fprintf(stderr, "[%d] No config file specified\n", myid);
    return(1);
  }

  /* Parse configuration */
  if (init_app(myid, nproc, cfgfile, &cfg) != 0) {
    fprintf(stderr, "[%d] Failed to read configuration file %s.\n", 
	    myid, cfgfile);
    return(1);
  }

  cfg.ecfg.bufp[0] = malloc(cfg.buf_sort_ffile_max_oct * sizeof(ue_octant_t));
  if (cfg.ecfg.bufp[0] == NULL) {
    fprintf(stderr, "[%d] Failed to alloc flatfile buffer\n", myid);
    return(1);
  }
  cfg.ecfg.num_octants[0] = 0;
  cfg.ecfg.max_octants = cfg.buf_sort_ffile_max_oct;

  /* Open flat files */
  sprintf(inputfile, "%s/cvmbycols_%07d.f", cfg.scratch, cfg.rank);
  sprintf(cfg.ecfg.outputfile, "%s/cvmbycols_%07d.fs", cfg.scratch, 
	  cfg.rank);
  cfg.ecfg.efp[0] = fopen(inputfile, "rb");
  cfg.ecfg.efp[1] = fopen(cfg.ecfg.outputfile, "wb");
  if ((cfg.ecfg.efp[0] == NULL) || (cfg.ecfg.efp[1] == NULL)) {
    fprintf(stderr, "[%d] Failed to open flat files\n", myid);
    return(1);
  }

  printf("[%d] Reading flat file %s, max octants=%d\n", myid, 
	 inputfile, cfg.ecfg.max_octants);
  while (!feof(cfg.ecfg.efp[0])) {
    if (cfg.ecfg.num_octants[0] == cfg.ecfg.max_octants) {
      fprintf(stderr, "[%d] Flat file buffer is full\n", myid);
      return(1);
    }
    num_read = fread((cfg.ecfg.bufp[0] + cfg.ecfg.num_octants[0]), 
		     sizeof(ue_octant_t), 
		     cfg.ecfg.max_octants - cfg.ecfg.num_octants[0], 
		     cfg.ecfg.efp[0]);
    cfg.ecfg.num_octants[0] = cfg.ecfg.num_octants[0] + num_read;
  }
  
  /* Sort the octants by key */
  printf("[%d] Sorting %d octants\n", myid, cfg.ecfg.num_octants[0]);
  qsort(cfg.ecfg.bufp[0], cfg.ecfg.num_octants[0], sizeof(ue_octant_t), 
	compare_keys);
  
  for (i = 1; i < cfg.ecfg.num_octants[0]; i++) {
    if (code_comparekey((void *)cfg.ecfg.bufp[0][i-1].key, 
			(void *)cfg.ecfg.bufp[0][i].key, 
			3 * sizeof(etree_tick_t) + 1) > 0) {
      fprintf(stderr, 
	      "[%d] Qsort produced out-of-order octants.\n", 
	      myid);
      return(1);
    }
  }
  
  /* Write sorted octant list */
  printf("[%d] Writing flat file %s\n", myid, cfg.ecfg.outputfile);
  if (fwrite(cfg.ecfg.bufp[0], sizeof(ue_octant_t), 
	     cfg.ecfg.num_octants[0], cfg.ecfg.efp[1]) != 
      cfg.ecfg.num_octants[0]) {
    fprintf(stderr, "[%d] Failed to write %d octants to flat file\n", 
	    myid, cfg.ecfg.num_octants[0]);
    return(1);
  }
  
  /* Close flat files */
  fclose(cfg.ecfg.efp[0]);
  fclose(cfg.ecfg.efp[1]);
  
  /* Free buffer */
  free(cfg.ecfg.bufp[0]);

  /* Wait for final rendezvous */
  mpi_barrier();
  mpi_final("MPI Done");

  return(0);
}
