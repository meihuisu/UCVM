#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/time.h>
#include <unistd.h>
#include "ue_dtypes.h"
#include "ue_mpi.h"
#include "ue_queue.h"
#include "ue_merge.h"
#include "ue_config.h"
#include "code.h"


/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;


/* Usage function */
void usage() {
  printf("Usage: ucvm2etree-merge-MPI [-h] -f config\n\n");
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
  int ph;
  int havemsg;

  /* Options and config */
  int opt;
  char cfgfile[UCVM_MAX_PATH_LEN];
  ue_cfg_t cfg;

  /* Etree parameters */
  char efile1[UCVM_MAX_PATH_LEN], efile2[UCVM_MAX_PATH_LEN];
  char appschema[UCVM_META_MIN_SCHEMA_LEN]; 
  char appmeta[UCVM_META_MIN_META_LEN]; 
  FILE *fp;
  ucvm_meta_cmu_t appmeta_cmu;
  ucvm_meta_ucvm_t appmeta_ucvm;

  /* Merge parameters */
  int rank1, rank2;
  ue_octant_t *octbuf;
  ue_octant_t *oct;
  ue_flatfile_t *ffbuf;
  ue_queue_t octq1, octq2;
  int octcount = 0;
  int ffcount = 0;
  int octsum = 0;
  int num_msg = 0;
  int eof1 = 0;
  int eof2 = 0;
  char lastkey[UE_MAX_KEYSIZE] = "";
  int (*source_reader)(ue_cfg_t *cfg, int id,
		       ue_queue_t *octq, int *eof);

  /* Performance measurements */
  struct timeval start, end, total_start, total_end;
  double elapsed, total_elapsed;
  unsigned long stage_octants, total_octants;

  /* Init MPI */
  mpi_init(&argc, &argv, &nproc, &myid, procname, &pnlen);

  /* Register new data types */
  mpi_register_octant(&(cfg.MPI_OCTANT));

  if (myid == 0) {
    printf("[%d] %s Version: %s (svn %s)\n", myid, argv[0],
           "Unknown", "Unknown");
    printf("[%d] Running on %d cores\n", myid, nproc);
    fflush(stdout);
  }

  /* Nproc must be 2^N cores */
  if ((nproc & (nproc - 1)) != 0) {
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

  if (myid == 0) {
    /* This is the etree writer */
    ffbuf = malloc(cfg.buf_merge_io_buf_oct * sizeof(ue_flatfile_t));
    if (ffbuf == NULL) {
      fprintf(stderr, "[%d] Failed to allocate octant buffers\n", myid);
      return(1);
    }
    q_init(cfg.buf_merge_sendrecv_buf_oct, sizeof(ue_octant_t), &octq1);

    /* Pack schema and meta data */
    if (strcmp(cfg.projinfo.projstr, PROJ_GEO_BILINEAR) == 0) {

      if (ucvm_schema_etree_cmu_pack(appschema, 
				     UCVM_META_MIN_SCHEMA_LEN) != 
	  UCVM_CODE_SUCCESS) {
	fprintf(stderr, "Failed to pack CMU etree schema\n");
	return(UCVM_CODE_ERROR);
      }

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
      if (ucvm_schema_etree_ucvm_pack(appschema, 
				      UCVM_META_MIN_SCHEMA_LEN) != 
	  UCVM_CODE_SUCCESS) {
	fprintf(stderr, "Failed to pack UCVM etree schema\n");
	return(UCVM_CODE_ERROR);
      }

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
    
    if (strcmp(cfg.ecfg.format, "etree") == 0) {
      /* Create (open) the unpacked etree */
      printf("[%d] Opening etree %s\n", myid, cfg.ecfg.outputfile);
      cfg.ecfg.ep[0] = etree_open(cfg.ecfg.outputfile, 
    				  O_CREAT|O_TRUNC|O_RDWR, 
    				  cfg.buf_etree_cache, 0, 3);
      if (cfg.ecfg.ep[0] == NULL) {
	fprintf(stderr, "[%d] Failed to create the %s etree (unpacked)\n", 
		myid, cfg.ecfg.outputfile);
	return(1);
      }
      
      /* Register etree schema */
      printf("[%d] Registering schema\n", myid);
      if (etree_registerschema(cfg.ecfg.ep[0], appschema) != 0) {
	fprintf(stderr, "[%d] %s\n", myid,
		etree_strerror(etree_errno(cfg.ecfg.ep[0])));
	return(1);
      }

      /* Apply the metadata to the etree */
      printf("[%d] Setting application metadata\n", myid);
      if (etree_setappmeta(cfg.ecfg.ep[0], appmeta) != 0) {
	fprintf(stderr, "[%d] %s\n", myid,
		etree_strerror(etree_errno(cfg.ecfg.ep[0])));
	return(1);
      }
      
      /* Begin append transaction */
      printf("[%d] Begin transaction\n", myid);
      if (etree_beginappend (cfg.ecfg.ep[0], 1.0) != 0) {
	fprintf(stderr, "[%d] %s\n", myid,
		etree_strerror(etree_errno(cfg.ecfg.ep[0])));
	return(1);
      }
    } else if (strcmp(cfg.ecfg.format, "flatfile") == 0) {
      cfg.ecfg.efp[0] = fopen(cfg.ecfg.outputfile, "wb");
      if (cfg.ecfg.efp[0] == NULL) {
	fprintf(stderr, "[%d] Failed to open flatfile %s\n", 
		myid, cfg.ecfg.outputfile);
	return(1);
      }
    } else {
      fprintf(stderr, "[%d] Unsupported file format %s\n", 
	      myid, cfg.ecfg.format);
      return(1);
    }

    mpi_barrier();
    rank1 = 1;
    ffcount = 0;
    eof1 = 0;
    stage_octants = 0;
    total_octants = 0;
    printf("[%d] Writing octants\n", myid);

    gettimeofday(&total_start, NULL);
    gettimeofday(&start, NULL);
    while (!eof1) {
      /* Recv from rank 1 */
      if (get_child(&cfg, rank1, &octq1, &eof1) != 0) {
	fprintf(stderr, "[%d] Failed to get octs from rank %d.\n", 
		myid, rank1);
	return(1);
      }

      stage_octants = stage_octants + q_get_len(&octq1);
      total_octants = total_octants + q_get_len(&octq1);

      if (strcmp(cfg.ecfg.format, "etree") == 0) {
	while (q_get_len(&octq1) > 0) {
	  q_pop_head(&octq1, (void *)&oct);
	  /* Append to etree */
	  if (etree_append(cfg.ecfg.ep[0], oct->addr, 
			   &(oct->payload)) != 0) {
	    fprintf(stderr, "[%d] Error appending octant: %s\n", myid,
		    etree_strerror(etree_errno(cfg.ecfg.ep[0])));
	    return(1);
	  }
	}
      } else if (strcmp(cfg.ecfg.format, "flatfile") == 0) {
	if (ffcount + q_get_len(&octq1) > cfg.buf_merge_io_buf_oct) {
	  /* Flush buffer to flat file */
	  if (fwrite(ffbuf, sizeof(ue_flatfile_t), ffcount, 
		     cfg.ecfg.efp[0]) != ffcount) {
	    fprintf(stderr, "[%d] Error appending octants to flatfile\n", 
		    myid);
	    return(1);
	  }
	  ffcount = 0;
	}
	/* Copy contents of message to flatfile buffer */
	while (q_get_len(&octq1) > 0) {
	  q_pop_head(&octq1, (void *)&oct);
	  memcpy(ffbuf[ffcount].key, oct->key, UE_MAX_KEYSIZE);
	  memcpy(&(ffbuf[ffcount].payload), &(oct->payload), 
		 sizeof(ucvm_epayload_t));
	  ffcount++;
	}
      } else {
	fprintf(stderr, "[%d] Unsupported file format %s\n", 
		myid, cfg.ecfg.format);
	return(1);
      }

      /* Print progress report */
      if (stage_octants > cfg.buf_merge_report_min_oct) {
	gettimeofday(&end,NULL);
	elapsed = (end.tv_sec - start.tv_sec) * 1000.0 +
	  (end.tv_usec - start.tv_usec) / 1000.0;
	total_elapsed = (end.tv_sec - total_start.tv_sec) * 1000.0 +
	  (end.tv_usec - total_start.tv_usec) / 1000.0;
	printf("[%d] Appended %lu octants in %.2f s (run avg %.2f oct/s)\n", 
	       myid, stage_octants, elapsed / 1000.0, 
	       total_octants / (total_elapsed / 1000.0));
	stage_octants = 0;
	gettimeofday(&start, NULL);
      }
    }

    if (strcmp(cfg.ecfg.format, "flatfile") == 0) {
      /* Flush remaining octants to flatfile */
      if (ffcount > 0) {
	if (fwrite(ffbuf, sizeof(ue_flatfile_t), ffcount, 
		   cfg.ecfg.efp[0]) != ffcount) {
	  fprintf(stderr, "[%d] Error appending octants to flatfile\n", 
		  myid);
	  return(1);
	}
      }
    }

    gettimeofday(&total_end,NULL);
    elapsed = (total_end.tv_sec - total_start.tv_sec) * 1000.0 +
      (total_end.tv_usec - total_start.tv_usec) / 1000.0;
    
    printf("[%d] Total: %lu octants in %.2f s\n", myid, total_octants,
	   elapsed/1000.0);
    fflush(stdout);
    
    if (strcmp(cfg.ecfg.format, "etree") == 0) {
      /* End append transaction */
      printf("[%d] End transaction\n", myid);
      if (etree_endappend(cfg.ecfg.ep[0]) != 0) {
	fprintf(stderr, "[%d] %s\n", myid,
		etree_strerror(etree_errno(cfg.ecfg.ep[0])));
	return(1);
      }
      
      /* Close the etree */
      printf("[%d] Closing etree\n", myid);
      if (etree_close(cfg.ecfg.ep[0]) != 0) {
	fprintf(stderr, "[%d] Error closing etree\n", myid);
	return(1);
      }
    } else if (strcmp(cfg.ecfg.format, "flatfile") == 0) {
      /* Close flat file */
      fclose(cfg.ecfg.efp[0]);

      /* Save schema and meta data */
      sprintf(efile1, "%s.schema", cfg.ecfg.outputfile);
      sprintf(efile2, "%s.metadata", cfg.ecfg.outputfile);
      fp = fopen(efile1, "wb");
      fwrite(appschema, UCVM_META_MIN_META_LEN, 1, fp);
      fclose(fp);
      fp = fopen(efile2, "wb");
      fwrite(appmeta, UCVM_META_MIN_META_LEN, 1, fp);
      fclose(fp);
    } else {
      fprintf(stderr, "[%d] Unsupported file format %s\n", 
	      myid, cfg.ecfg.format);
      return(1);
    }

    free(ffbuf);

  } else if ((myid > 0) && (myid < nproc/2)) {

    /* Merge from 2 cores */
    octbuf = malloc(cfg.buf_merge_sendrecv_buf_oct * sizeof(ue_octant_t));
    if (octbuf == NULL) {
      fprintf(stderr, "[%d] Failed to allocate octant buffers\n", myid);
      return(1);
    }
    q_init(cfg.buf_merge_sendrecv_buf_oct, sizeof(ue_octant_t), &octq1);
    q_init(cfg.buf_merge_sendrecv_buf_oct, sizeof(ue_octant_t), &octq2);

    mpi_barrier();
    rank1 = myid * 2;
    rank2 = rank1 + 1;
    printf("[%d] Merging cores %d and %d\n", myid, rank1, rank2);

    octcount = 0;
    num_msg = 0;
    eof1 = 0;
    eof2 = 0;

    /* Get some data */
    if (get_child(&cfg, rank1, &octq1, &eof1) != 0) {
      fprintf(stderr, "[%d] Failed to get octs from rank %d.\n", 
	      myid, rank1);
      return(1);
    }
    if (get_child(&cfg, rank2, &octq2, &eof2) != 0) {
      fprintf(stderr, "[%d] Failed to get octs from rank %d.\n", 
	      myid, rank2);
      return(1);
    }

    while ((octcount > 0) || 
	   (q_get_len(&octq1) > 0) || 
	   (q_get_len(&octq2) > 0)) {

      /* Sort octants */
      octsum = octcount + q_get_len(&octq1) + q_get_len(&octq2);
      if (merge_queues(&cfg, octbuf, cfg.buf_merge_sendrecv_buf_oct, &octcount,
		       &octq1, eof1, &octq2, eof2) != 0) {
	fprintf(stderr, 
		"[%d] Failed to merge streams with counts %d, %d, %d.\n", 
		myid, octcount, q_get_len(&octq1), q_get_len(&octq2));
	return(1);
      }

      if (octsum != octcount + q_get_len(&octq1) + q_get_len(&octq2)) {
	fprintf(stderr, 
		"[%d] Octant mismatch after merge: %d before, %d after\n", 
		myid, octsum, 
		octcount + q_get_len(&octq1) + q_get_len(&octq2));
	return(1);
      }
      if (octcount > 0) {
	if (strcmp(lastkey, "") != 0) {
	  if (code_comparekey((void *)octbuf[0].key, 
			      (void *)lastkey, 
			      3 * sizeof(etree_tick_t) + 1) < 0) {
	    fprintf(stderr, 
		    "[%d] Merged out-of-order child octants.\n", 
		  myid);
	      return(1);
	  }
	}
	memcpy(lastkey, octbuf[octcount-1].key, UE_MAX_KEYSIZE);
      }

      /* Clear incoming queue */
      MPI_Iprobe((myid>>1), UE_MSG_REQ, MPI_COMM_WORLD, &havemsg, 
		 MPI_STATUS_IGNORE);
      while (havemsg) {
	MPI_Recv(&ph, 1, MPI_INT, (myid>>1), UE_MSG_REQ, MPI_COMM_WORLD, 
		 MPI_STATUS_IGNORE);
	num_msg--;
	MPI_Iprobe((myid>>1), UE_MSG_REQ, MPI_COMM_WORLD, &havemsg, 
		   MPI_STATUS_IGNORE);
      }

      if (octcount > 0) {
	if (num_msg == MAX_QUEUED_MSG) {
	  MPI_Recv(&ph, 1, MPI_INT, (myid>>1), UE_MSG_REQ, MPI_COMM_WORLD, 
		   &status);
	  num_msg--;
	}

	//printf("[%d] Sent %d octants to parent\n", myid, octcount);

	MPI_Send(&(octbuf[0]), octcount, cfg.MPI_OCTANT, (myid>>1), 
		 UE_MSG_DATA, MPI_COMM_WORLD);
	octcount = 0;
	num_msg++;
      }

      /* Get more data */
      if ((!eof1) && (q_get_len(&octq1) == 0)) {
	if (get_child(&cfg, rank1, &octq1, &eof1) != 0) {
	  fprintf(stderr, "[%d] Failed to get octs from rank %d.\n", 
		  myid, rank1);
	  return(1);
	}
      }
      if ((!eof2) && (q_get_len(&octq2) == 0)) {
	if (get_child(&cfg, rank2, &octq2, &eof2) != 0) {
	  fprintf(stderr, "[%d] Failed to get octs from rank %d.\n", 
		  myid, rank2);
	  return(1);
	}
      }
    }

    /* Receive straggler messages */
    for (i = 0; i < num_msg; i++) {
      MPI_Recv(&ph, 1, MPI_INT, (myid>>1), UE_MSG_REQ, MPI_COMM_WORLD, 
	       MPI_STATUS_IGNORE);
    }

    /* Send eof message */
    printf("[%d] Sending EOF.\n", myid);
    MPI_Send(&(octbuf[0]), 1, cfg.MPI_OCTANT, (myid>>1), 
	     UE_MSG_END, MPI_COMM_WORLD);
    MPI_Recv(&ph, 1, MPI_INT, (myid>>1), UE_MSG_REQ, MPI_COMM_WORLD, 
	     MPI_STATUS_IGNORE);
    if ((octcount > 0) || 
	(q_get_len(&octq1) > 0) || 
	(q_get_len(&octq2) > 0)) {
      fprintf(stderr, "[%d] Unsorted octants detected\n", myid);
      return(1);
    }

    printf("[%d] Worker done.\n", myid);

    free(octbuf);
    q_free(&octq1);
    q_free(&octq2);

  } else {
    /* Merge from 2 etrees */
    octbuf = malloc(cfg.buf_merge_sendrecv_buf_oct * sizeof(ue_octant_t));
    if (octbuf == NULL) {
      fprintf(stderr, "[%d] Failed to allocate octant buffers\n", myid);
      return(1);
    }
    q_init(cfg.buf_merge_io_buf_oct, sizeof(ue_octant_t), &octq1);
    q_init(cfg.buf_merge_io_buf_oct, sizeof(ue_octant_t), &octq2);

    rank1 = (myid - nproc/2)*2;
    rank2 = rank1 + 1;
    printf("[%d] Merging etrees %d and %d\n", myid, rank1, rank2);

    sprintf(efile1, "%s/cvmbycols_%07d.fs", cfg.scratch, rank1);
    sprintf(efile2, "%s/cvmbycols_%07d.fs", cfg.scratch, rank2);
    printf("[%d] %s, %s\n", myid, efile1, efile2);
    cfg.ecfg.efp[0] = fopen(efile1, "rb");
    if (cfg.ecfg.efp[0] == NULL) {
      fprintf(stderr, "[%d] Failed to open flatfile %s\n", 
	      myid, efile1);
      return(1);
    }
    cfg.ecfg.efp[1] = fopen(efile2, "rb");
    if (cfg.ecfg.efp[1] == NULL) {
      fprintf(stderr, "[%d] Failed to open flatfile %s\n", 
	      myid, efile2);
      return(1);
    }
    
    mpi_barrier();
    octcount = 0;
    num_msg = 0;
    eof1 = 0;
    eof2 = 0;
    source_reader = get_flatfile;
    
    printf("[%d] Extracting octants from etrees\n", myid);

    /* Get some data */
    if (source_reader(&cfg, 0, &octq1, &eof1) != 0) {
      fprintf(stderr, "[%d] Failed to get octs from etree %s\n", 
	      myid, efile1);
      return(1);
    }
    if (source_reader(&cfg, 1, &octq2, &eof2) != 0) {
      fprintf(stderr, "[%d] Failed to get octs from etree %s\n", 
	      myid, efile2);
      return(1);
    }

    while ((octcount > 0) || 
	   (q_get_len(&octq1) > 0) || 
	   (q_get_len(&octq2) > 0)) {

      /* Sort octants */
      octsum = octcount + q_get_len(&octq1) + q_get_len(&octq2);
      if (merge_queues(&cfg, octbuf, cfg.buf_merge_sendrecv_buf_oct, &octcount,
		       &octq1, eof1, &octq2, eof2) != 0) {
	fprintf(stderr, 
		"[%d] Failed to merge streams with counts %d, %d, %d.\n", 
		myid, octcount, q_get_len(&octq1), q_get_len(&octq2));
	return(1);
      }
      if (octsum != octcount + q_get_len(&octq1) + q_get_len(&octq2)) {
	fprintf(stderr, 
		"[%d] Octant mismatch after merge: %d before, %d after\n", 
		myid, octsum, 
		octcount + q_get_len(&octq1) + q_get_len(&octq2));
	return(1);
      }
      if (octcount > 0) {
	if (strcmp(lastkey, "") != 0) {
	  if (code_comparekey((void *)octbuf[0].key, 
			      (void *)lastkey, 
			      3 * sizeof(etree_tick_t) + 1) < 0) {
	    fprintf(stderr, 
		    "[%d] Merged out-of-order etree octants.\n", 
		    myid);
	    return(1);
	  }
	}
	memcpy(lastkey, octbuf[octcount-1].key, UE_MAX_KEYSIZE);
      }

      /* Clear incoming queue */
      MPI_Iprobe((myid>>1), UE_MSG_REQ, MPI_COMM_WORLD, &havemsg, 
		 MPI_STATUS_IGNORE);
      while (havemsg) {
	MPI_Recv(&ph, 1, MPI_INT, (myid>>1), UE_MSG_REQ, MPI_COMM_WORLD, 
		 MPI_STATUS_IGNORE);
	num_msg--;
	if (num_msg < 0) {
	  fprintf(stderr, "[%d] Received too many msgs\n", myid);
	  return(1);
	}
	MPI_Iprobe((myid>>1), UE_MSG_REQ, MPI_COMM_WORLD, &havemsg, 
		   MPI_STATUS_IGNORE);
      }

      if (octcount > 0) {
	if (num_msg == MAX_QUEUED_MSG) {
	  MPI_Recv(&ph, 1, MPI_INT, (myid>>1), UE_MSG_REQ, MPI_COMM_WORLD, 
		   MPI_STATUS_IGNORE);
	  num_msg--;
	}

	//printf("[%d] Sent %d octants to parent\n", myid, octcount);

	MPI_Send(&(octbuf[0]), octcount, cfg.MPI_OCTANT, (myid>>1), 
		 UE_MSG_DATA, MPI_COMM_WORLD);
	octcount = 0;
	num_msg++;
      }

      /* Get more data */
      if ((!eof1) && (q_get_len(&octq1) == 0)) {
	if (source_reader(&cfg, 0, &octq1, &eof1) != 0) {
	  fprintf(stderr, "[%d] Failed to get octs from etree %s\n", 
		  myid, efile1);
	  return(1);
	}
      }
      if ((!eof2) && (q_get_len(&octq2) == 0)) {
	if (source_reader(&cfg, 1, &octq2, &eof2) != 0) {
	  fprintf(stderr, "[%d] Failed to get octs from etree %s\n", 
		  myid, efile2);
	  return(1);
	}
      }
    }

    /* Receive straggler messages */
    for (i = 0; i < num_msg; i++) {
      MPI_Recv(&ph, 1, MPI_INT, (myid>>1), UE_MSG_REQ, MPI_COMM_WORLD, 
	       MPI_STATUS_IGNORE);
    }

    /* Send eof message */
    printf("[%d] Sending EOF.\n", myid);
    MPI_Send(&(octbuf[0]), 1, cfg.MPI_OCTANT, (myid>>1), 
	     UE_MSG_END, MPI_COMM_WORLD);
    MPI_Recv(&ph, 1, MPI_INT, (myid>>1), UE_MSG_REQ, MPI_COMM_WORLD, 
	     MPI_STATUS_IGNORE);
    if ((octcount > 0) || 
	(q_get_len(&octq1) > 0) || 
	(q_get_len(&octq2) > 0)) {
      fprintf(stderr, "[%d] Unsorted octants detected\n", myid);
      return(1);
    }

    fclose(cfg.ecfg.efp[0]);
    fclose(cfg.ecfg.efp[1]);

    printf("[%d] Worker done.\n", myid);

    free(octbuf);
    q_free(&octq1);
    q_free(&octq2);
  }

  fflush(stdout);

  /* Wait for etree merge to finish */
  mpi_barrier();
  mpi_final("MPI Done");

  return(0);
}
