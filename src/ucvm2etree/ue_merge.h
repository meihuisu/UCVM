#ifndef UE_MERGE_H
#define UE_MERGE_H

#include "ue_dtypes.h"
#include "ue_queue.h"

/* MPI message types */
#define UE_MSG_REQ 0
#define UE_MSG_DATA 1
#define UE_MSG_END 2

/* Max number of queued msgs to parent */
#define MAX_QUEUED_MSG 5

int get_child(ue_cfg_t *cfg, int rank, ue_queue_t *octq, int *eof);

int get_etree(ue_cfg_t *cfg, int id,
	      ue_queue_t *octq, int *eof);

int get_flatfile(ue_cfg_t *cfg, int id,
		 ue_queue_t *octq, int *eof);

int merge_queues(ue_cfg_t *cfg, ue_octant_t *octbuf, int maxsize, 
		 int *octcount, ue_queue_t *octq1, int eof1,
		 ue_queue_t *octq2, int eof2);


#endif

