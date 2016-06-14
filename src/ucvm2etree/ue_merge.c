#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include "ue_merge.h"
#include "ue_utils.h"
#include "code.h"


int get_child(ue_cfg_t *cfg, int rank, ue_queue_t *octq, int *eof)
{
  int ph;
  MPI_Status status;
  ue_octant_t *oct;
  int octcount = 0;

  if (*eof) {
    q_reset(octq);
    return(0);
  }

  if (q_get_len(octq) > 0) {
    fprintf(stderr, "[%d] Expected empty queue\n", cfg->rank);
    return(1);
  }
  q_reset(octq);

  if (q_alloc_contig_tail(octq, q_get_maxlen(octq), (void *)&oct) != 0) {
      fprintf(stderr, "[%d] Failed to alloc contig queue buf\n", cfg->rank);
      return(1);
  }

  //MPI_Recv(msgbuf, UE_OCTBUF_RECV_SIZE, cfg->MPI_OCTANT, rank, 
  //	   MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  
  MPI_Recv(oct, q_get_maxlen(octq), cfg->MPI_OCTANT, rank, 
  	   MPI_ANY_TAG, MPI_COMM_WORLD, &status);

  MPI_Send(&ph, 1, MPI_INT, rank, UE_MSG_REQ, MPI_COMM_WORLD);
  
  switch (status.MPI_TAG) {
  case UE_MSG_DATA:
    MPI_Get_count(&status, cfg->MPI_OCTANT, &octcount);
    *eof = 0;
    if (octcount <= 0) {
      q_reset(octq);
      fprintf(stderr, "[%d] Expected at least 1 octant\n", cfg->rank);
      return(1);
    }
    //for (i = 0; i < octcount; i++) {
    //  q_alloc_tail(octq, (void *)&oct);
    //  memcpy(oct, &(msgbuf[i]), sizeof(ue_octant_t));
    //}
    //while (q_get_len(octq) > octcount) {
    //  q_free_tail(octq, 1);
    //}
    q_free_tail(octq, q_get_len(octq) - octcount);
    break;
  case UE_MSG_END:
    *eof = 1;
    octcount = 0;
    q_reset(octq);
    break;
  default:
    *eof = 1;
    octcount = 0;
    q_reset(octq);
    fprintf(stderr, "[%d] Invalid msg type\n", cfg->rank);
    return(1);
  }

  if (q_get_len(octq) != octcount) {
    fprintf(stderr, "[%d] Queue len %d unequal to octcount %d\n", cfg->rank,
	    q_get_len(octq), octcount);
    return(1);
  }

  return(0);
}


int get_etree(ue_cfg_t *cfg, int id, ue_queue_t *octq, int *eof)
{
  ue_octant_t *oct;
  int octcount = 0;
  
  if ((id < 0) || (id > 1)) {
    fprintf(stderr, "[%d] Invalid etree id\n", cfg->rank);
    return(1);
  }

  *eof = 0;
  while ((q_has_space(octq)) && (!(*eof))) {
    q_alloc_tail(octq, (void *)&oct);
    if (etree_getcursor(cfg->ecfg.ep[id], 
			&(oct->addr), "*", &(oct->payload)) != 0) {
      *eof = 1;
      q_free_tail(octq, 1);
    }
    if (!(*eof)) {
      ue_addr2key(oct->addr, (void *)(oct->key));
      octcount++;
      if (etree_advcursor(cfg->ecfg.ep[id]) != 0) {
	if (etree_errno(cfg->ecfg.ep[id]) == ET_END_OF_TREE) {
	  *eof = 1;
	} else {
	  fprintf(stderr, 
		  "[%d] Failed to advance etree cursor: %s\n", cfg->rank, 
		  etree_strerror(etree_errno(cfg->ecfg.ep[id])));
	  return(1);
	}
      }
    }
  }
  
  return(0);
}


int get_flatfile(ue_cfg_t *cfg, int id, ue_queue_t *octq, int *eof)
{
  ue_octant_t *oct;
  int octcount = 0;
  
  if ((id < 0) || (id > 1)) {
    fprintf(stderr, "[%d] Invalid flatfile id\n", cfg->rank);
    return(1);
  }
  
  if (q_get_len(octq) > 0) {
    fprintf(stderr, "[%d] Expected empty queue\n", cfg->rank);
    return(1);
  }
  q_reset(octq);

  if (feof(cfg->ecfg.efp[id])) {
    *eof = 1;
    return(0);
  }
  *eof = 0;

  if (q_alloc_contig_tail(octq, q_get_maxlen(octq), (void *)&oct) != 0) {
      fprintf(stderr, "[%d] Failed to alloc contig queue buf\n", cfg->rank);
      return(1);
  }

  octcount = fread(oct, sizeof(ue_octant_t), q_get_maxlen(octq), 
		   cfg->ecfg.efp[id]);

  q_free_tail(octq, q_get_len(octq) - octcount);

  if (feof(cfg->ecfg.efp[id])) {
    *eof = 1;
  }
  
  return(0);
}


int merge_queues(ue_cfg_t *cfg, ue_octant_t *octbuf, int maxsize, 
		 int *octcount, ue_queue_t *octq1, int eof1,
		 ue_queue_t *octq2, int eof2)

{
  int keysize;
  ue_octant_t *oct1;
  ue_octant_t *oct2;

  if (((q_get_len(octq1) == 0) && 
       (q_get_len(octq2) == 0)) || 
      (*octcount == maxsize)) {
    return(0);
  }

  keysize = 3 * sizeof(etree_tick_t) + 1;

  /* Handle overlap among points */
  while ((q_get_len(octq1) > 0) && 
	 (q_get_len(octq2) > 0) && 
	 (*octcount < maxsize)) {
    if ((q_get_head(octq1, (void *)&oct1) != 0) || 
	(q_get_head(octq2, (void *)&oct2) != 0)) {
      return(1);
    }
    if (code_comparekey((void *)(oct1->key), 
			(void *)(oct2->key), keysize) <= 0) {
      memcpy(&(octbuf[(*octcount)++]), oct1, sizeof(ue_octant_t));
      if (q_pop_head(octq1, (void *)&oct1) != 0) {
	return(1);
      }
    } else {
      memcpy(&(octbuf[(*octcount)++]), oct2, sizeof(ue_octant_t));
      if (q_pop_head(octq2, (void *)&oct2) != 0) {
	return(1);
      }
    }
  }
  
  /* Check for end of stream */
  if (eof2) {
    while ((*octcount < maxsize) && (q_get_len(octq1) > 0) && 
	   (q_get_len(octq2) == 0)) {
      if (q_pop_head(octq1, (void *)&oct1) != 0) {
	return(1);
      }
      memcpy(&(octbuf[(*octcount)++]), oct1, sizeof(ue_octant_t));
    }
  }

  if (eof1) {
    while ((*octcount < maxsize) && (q_get_len(octq2) > 0) && 
	   (q_get_len(octq1) == 0)) {
      if (q_pop_head(octq2, (void *)&oct2) != 0) {
	return(1);
      }
      memcpy(&(octbuf[(*octcount)++]), oct2, sizeof(ue_octant_t));
    }
  }

  return(0);
}
