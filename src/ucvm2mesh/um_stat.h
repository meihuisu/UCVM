#ifndef UM_STAT_H
#define UM_STAT_H

#include "um_mesh.h"

/* Possible statistic types */
#define STAT_MAX_STATS 7


typedef enum { STAT_MAX_VP = 0, 
               STAT_MAX_VS,
               STAT_MAX_RHO,
               STAT_MIN_VP,
               STAT_MIN_VS,
               STAT_MIN_RHO,
               STAT_MIN_RATIO } stat_type_t;


/* Statistic bucket */
typedef struct stat_t {
  int i;
  int j;
  int k;
  float val;
} stat_t;


/* Get statistic label from id */
const char *stat_get_label(stat_type_t s);


/* Calculate and store statistics for a node */
int calc_stats(int i, int j, int k, mesh_ijk32_t *node, stat_t *stats);
int calc_stats_list(int i_start, int i_end, int j_start, int j_end, int k, 
		    mesh_ijk32_t *node_buf, stat_t *stats);


#endif
