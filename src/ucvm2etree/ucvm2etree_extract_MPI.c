#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include "ue_extract.h"
#include "ue_mpi.h"
#include "ue_config.h"
#include "code.h"

/* Extraction status */
#define UE_EXTRACT_OK 0
#define UE_EXTRACT_FULL 1
#define UE_EXTRACT_DONE 2


/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;


/* Usage function */
void usage() {
  printf("Usage: ucvm2etree-extract-MPI [-h] -f config\n\n");
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
    return(1);
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


int main(int argc, char **argv)
{
  int i;

  /* MPI stuff and distributed computation variables */
  int myid, nproc, pnlen;
  char procname[128];
  MPI_Status status;
  int src;
  unsigned long total_count;
  MPI_Datatype MPI_DISPATCH;
  ue_dispatch_t dispatch;
  
  /* Options and config */
  int opt;
  char cfgfile[UCVM_MAX_PATH_LEN];
  ue_cfg_t cfg;
  int (*write_func)(ue_cfg_t *cfg,
		    etree_addr_t *pnts, ucvm_data_t *props, 
		    int num_points);
  
  /* Dispatch buffers */
  ue_rank_t *active = NULL;
  ue_rank_t *ranks = NULL;
  ue_oct_t *octs = NULL;
  int col_left, next_col;
  int done;
  int num_full;
  
  /* Query buffers */
  ucvm_point_t *cvm_pnts = NULL;
  etree_addr_t *etree_pnts = NULL;
  ucvm_data_t *props = NULL;
  etree_tick_t maxrez, max_points;
  
  
  /* Init MPI */
  mpi_init(&argc, &argv, &nproc, &myid, procname, &pnlen);
  
  /* Register new data types */
  mpi_register_dispatch(&MPI_DISPATCH);
  
  if (myid == 0) {
    printf("[%d] %s Version: %s (svn %s)\n", myid, argv[0],
           "Unknown", "Unknown");
    printf("[%d] Running on %d cores\n", myid, nproc);
    fflush(stdout);
  }

  /* Nproc must be 2^N + 1 cores */
  if (((nproc - 1) & (nproc - 2)) != 0) {
    fprintf(stderr, "[%d] nproc must be 2^N+1 cores\n", myid);
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

  memset(&dispatch, 0, sizeof(ue_dispatch_t));

  if (myid == 0) {
    /* Allocate dispatch buffers */
    /* Notes those ranks who have checked in at least once */
    active = (ue_rank_t *)malloc(nproc*sizeof(ue_rank_t));
    if (active == NULL) {
      fprintf(stderr, "[%d] Failed to allocated active buffer\n", myid);
      return(1);
    }
    /* Notes column assignment for each rank */
    ranks = (ue_rank_t *)malloc(nproc*sizeof(ue_rank_t));
    if (ranks == NULL) {
      fprintf(stderr, "[%d] Failed to allocated rank buffer\n", myid);
      return(1);
    }
    /* Notes octant count for each column */
    octs = (ue_oct_t *)malloc(cfg.col_dims.dim[0]*cfg.col_dims.dim[1]*
			       sizeof(ue_oct_t));
    if (octs == NULL) {
      fprintf(stderr, "[%d] Failed to allocated oct buffer\n", myid);
      return(1);
    }

    /* Initialize buffers with start state */
    for (i = 0; i < nproc; i++) {
      ranks[i] = -1;
      active[i] = 0;
    }
    for (i = 0; i < cfg.col_dims.dim[0]*cfg.col_dims.dim[1]; i++) {
      octs[i] = 0;
    }

    /* Wait for worker ready state */
    printf("[%d] Barrier for worker ready state\n", myid);
    mpi_barrier();

    /* Wait for worker request */
    printf("[%d] Waiting for worker requests\n", myid);
    col_left = cfg.col_dims.dim[0]*cfg.col_dims.dim[1];
    next_col = 0;
    num_full = 0;
    while (col_left > 0) {

      if ((col_left % 100 == 0) || (col_left < 20)) {
	printf("[%d] Columns Rem: %d, Num Full: %d, Next Col: %d\n", 
	       myid, col_left, num_full, next_col);

	for (i = 1; i < nproc; i++) {
	  if (!active[i]) {
	    printf("[%d] Inactive: rank %d\n", myid, i);
	  }
	}
      }
				 
      /* Recv message count from src */
      MPI_Recv(&dispatch, 1, MPI_DISPATCH,
	       MPI_ANY_SOURCE , MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      src = status.MPI_SOURCE;
      active[src] = 1;

      //printf("[%d] Recv from %d: %d %lu %d\n", myid, src,
      //	     dispatch.col, dispatch.octcount, dispatch.status);

      if (ranks[src] >= 0) {
	/* Response to previous assignment */
	if (dispatch.octcount > 0) {
	  octs[ranks[src]] = dispatch.octcount;
	  col_left--;
	} else {
	  fprintf(stderr, "[%d] Worker reported zero counts.\n", myid);
	  return(1);
	}
      }
      
      if (dispatch.status == UE_EXTRACT_OK) {
	/* Send message next_col to src */
	if (next_col == cfg.col_dims.dim[0]*cfg.col_dims.dim[1]) {
	  ranks[src] = -1;
	} else {
	  ranks[src] = next_col++;
	}
	dispatch.col = ranks[src];
	dispatch.octcount = 0;
	if (dispatch.col < 0) {
	  dispatch.status = UE_EXTRACT_DONE;
	  //active[src] = 0;
	} else {
	  dispatch.status = UE_EXTRACT_OK;
	}
      } else {
	printf("[%d] Received full msg from worker\n", myid);
	/* Mark this rank as done */
	//active[src] = 0;
	ranks[src] = -1;
	num_full++;
	dispatch.col = ranks[src];
	dispatch.octcount = 0;
	dispatch.status = UE_EXTRACT_DONE;
      }
      MPI_Send(&dispatch, 1, MPI_DISPATCH, src, 0, MPI_COMM_WORLD);

      //printf("[%d] Num full = %d\n", myid, num_full);
      if ((num_full == nproc - 1) && (col_left > 0)) {
	fprintf(stderr, 
		"[%d] All workers report full yet columns remain.\n", 
		myid);
	return(1);
      }

    }
    
    /* Check for stragglers, tell them they are done */
    printf("[%d] Checking for inactive workers\n", myid);
    for (i = 1; i < nproc; i++) {
      if (!active[i]) {
	printf("[%d] Ordering worker %d to stop\n", myid, i);
	MPI_Recv(&dispatch, 1, MPI_DISPATCH,
		 MPI_ANY_SOURCE , MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	src = status.MPI_SOURCE;
	ranks[src] = -1;
	dispatch.col = ranks[src];
	dispatch.octcount = 0;
	dispatch.status = UE_EXTRACT_DONE;
	MPI_Send(&dispatch, 1, MPI_DISPATCH, src, 0, MPI_COMM_WORLD);
      }
    }
    
    printf("[%d] Server is done.\n", myid);
    total_count = 0;
    for (i = 0; i < cfg.col_dims.dim[0]*cfg.col_dims.dim[1]; i++) {
      total_count = total_count + octs[i];
    }
    printf("[%d] Total octants: %lu.\n", myid, total_count);
    fflush(stdout);

    free(active);
    free(ranks);
    free(octs);

  } else {

    /* Stagger setup */
    if (nproc > 5) {    
      sleep(myid % (nproc/5));
    }

    /* Setup UCVM */
    fflush(stdout);
    if (ucvm_init(cfg.ucvmconf) != UCVM_CODE_SUCCESS) {
      fprintf(stderr, "[%d] Failed to initialize UCVM\n", myid);
      return(1);
    }
    
    /* Set depth query mode */
    if (ucvm_setparam(UCVM_PARAM_QUERY_MODE,
		      UCVM_COORD_GEO_DEPTH) != UCVM_CODE_SUCCESS) {
      fprintf(stderr, "[%d] Set query mode failed\n", myid);
      return(1);
    }
    
    /* Add model */
    if (ucvm_add_model_list(cfg.ucvmstr) != UCVM_CODE_SUCCESS) {
      fprintf(stderr, "[%d] Failed to add model list %s\n", myid, cfg.ucvmstr);
      return(1);
    }

    /* Set interpolation z range */
    if (ucvm_setparam(UCVM_PARAM_IFUNC_ZRANGE, 
		      cfg.ucvm_zrange[0], 
		      cfg.ucvm_zrange[1]) != UCVM_CODE_SUCCESS) {
      fprintf(stderr, "[%d] Failed to set interpolation z range\n", myid);
      return(1);
    }
    
    /* Wait for worker ready state */
    mpi_barrier();

    /* Allocate octant buffer */
    printf("[%d] Allocating buffer for %d flatfile octants\n", 
	   myid, cfg.buf_extract_mem_max_oct);
    cfg.ecfg.bufp[0] = malloc(cfg.buf_extract_mem_max_oct * sizeof(ue_octant_t));
    if (cfg.ecfg.bufp[0] == NULL) {
      fprintf(stderr, "[%d] Failed to alloc flatfile buffer\n", myid);
      return(1);
    }
    cfg.ecfg.num_octants[0] = 0;
    cfg.ecfg.max_octants = cfg.buf_extract_mem_max_oct;
    write_func = insert_grid_buf;

    /* Open flat file */
    sprintf(cfg.ecfg.outputfile, "%s/cvmbycols_%07d.f", cfg.scratch, 
	    (cfg.rank)-1);
    cfg.ecfg.efp[0] = fopen(cfg.ecfg.outputfile, "wb");
    if (cfg.ecfg.efp[0] == NULL) {
      fprintf(stderr, "[%d] Failed to open flat file\n", myid);
      return(1);
    }

    /* Allocate query buffers */
    maxrez = (etree_tick_t)1 << (ETREE_MAXLEVEL - cfg.ecfg.max_level);
    max_points = (cfg.ecfg.col_ticks[0]/maxrez) * 
      (cfg.ecfg.col_ticks[1]/maxrez);
    printf("[%d] Allocating buffers for %d points, data values\n", 
	   myid, max_points);
    cvm_pnts = malloc(max_points * sizeof(ucvm_point_t));
    etree_pnts = malloc(max_points * sizeof(etree_addr_t));
    props = malloc(max_points * sizeof(ucvm_data_t));
    if ((cvm_pnts == NULL) || (etree_pnts == NULL) || (props == NULL)) {
      fprintf(stderr, "[%d] Failed to allocate buffers\n", myid);
      return(1);
    }

    done = 0;
    total_count = 0;
    while (!done) {

      //printf("[%d] Send to %d: %d %lu %d\n", myid, 0,
      //	     dispatch.col, dispatch.octcount, dispatch.status);

      /* Send message count to rank 0 */
      MPI_Send(&dispatch, 1, MPI_DISPATCH, 0, 0, MPI_COMM_WORLD);
	       
      /* Recv message next_col from rank 0 */
      MPI_Recv(&dispatch, 1, MPI_DISPATCH, 0, MPI_ANY_TAG, 
	       MPI_COMM_WORLD, &status);

      //src = status.MPI_SOURCE;
      //printf("[%d] Recv from %d: %d %lu %d\n", myid, src,
      //	     dispatch.col, dispatch.octcount, dispatch.status);

      if (dispatch.status == UE_EXTRACT_OK) {
	//printf("[%d] Extracting col %lu.\n", myid, dispatch.col);
	if (extract(&cfg, write_func, dispatch.col, 
		    cvm_pnts, etree_pnts, props, &(dispatch.octcount)) != 0) {
	  fprintf(stderr, "[%d] Extraction failed\n", myid);
	  return(1);
	}
	total_count = total_count + dispatch.octcount;
	/* Check for full output file */
	if (total_count > cfg.buf_extract_ffile_max_oct) {
	  printf("[%d] Worker is full\n", myid);
	  dispatch.status = UE_EXTRACT_FULL;
	} else {
	  dispatch.status = UE_EXTRACT_OK;
	}
      } else {
	done = 1;
      }
    }
    printf("[%d] Worker is done with extractions.\n", myid);
    printf("[%d] Worker extracted octants: %lu.\n", myid, total_count);
    fflush(stdout);

    /* Free buffers */
    free(cvm_pnts);
    free(etree_pnts);
    free(props);

    /* Finalize UCVM */
    ucvm_finalize();

    /* Finalize projection */
    if (strcmp(cfg.projinfo.projstr, PROJ_GEO_BILINEAR) != 0) {
      ucvm_proj_ucvm_finalize(&(cfg.projinfo.proj));
    }
    
    /* Flush remaining octants */
    if (cfg.ecfg.num_octants[0] > 0) {
      if (fwrite(cfg.ecfg.bufp[0], sizeof(ue_octant_t), 
		 cfg.ecfg.num_octants[0], cfg.ecfg.efp[0]) != 
	  cfg.ecfg.num_octants[0]) {
	fprintf(stderr, "[%d] Failed to flush buffer to disk\n", cfg.rank);
	return(1);
      }
      cfg.ecfg.num_octants[0] = 0;
    }

    /* Close file */
    fclose(cfg.ecfg.efp[0]);
    
    /* Free buffer */
    free(cfg.ecfg.bufp[0]);
  }

  /* Wait for final rendezvous */
  mpi_barrier();
  mpi_final("MPI Done");

  return(0);
}
