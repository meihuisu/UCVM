#include "um_stat.h"

/* Valid statistic types */
const char *STAT_NAMES[STAT_MAX_STATS] = { "Max Vp", 
					   "Max Vs", 
					   "Max Rho", 
					   "Min Vp", 
					   "Min Vs", 
					   "Min Rho", 
					   "Min Ratio" };


/* Get statistic label from id */
const char *stat_get_label(stat_type_t s)
{
  return(STAT_NAMES[s]);
}


/* Calculate and store statistics */
int calc_stats(int i, int j, int k, mesh_ijk32_t *node, stat_t *stats)
{
  /* Find maximum vp,vs,rho */
  if (node->vp > stats[STAT_MAX_VP].val) {
    stats[STAT_MAX_VP].i = i;
    stats[STAT_MAX_VP].j = j;
    stats[STAT_MAX_VP].k = k;
    stats[STAT_MAX_VP].val = node->vp;
  }
  if (node->vs > stats[STAT_MAX_VS].val) {
    stats[STAT_MAX_VS].i = i;
    stats[STAT_MAX_VS].j = j;
    stats[STAT_MAX_VS].k = k;
    stats[STAT_MAX_VS].val = node->vs;
  }
  if (node->rho > stats[STAT_MAX_RHO].val) {
    stats[STAT_MAX_RHO].i = i;
    stats[STAT_MAX_RHO].j = j;
    stats[STAT_MAX_RHO].k = k;
    stats[STAT_MAX_RHO].val = node->rho;
  }
  
  /* Find minimum vp,vs,rho */
  if (node->vp < stats[STAT_MIN_VP].val) {
    stats[STAT_MIN_VP].i = i;
    stats[STAT_MIN_VP].j = j;
    stats[STAT_MIN_VP].k = k;
    stats[STAT_MIN_VP].val = node->vp;
  }
  if (node->vs < stats[STAT_MIN_VS].val) {
    stats[STAT_MIN_VS].i = i;
    stats[STAT_MIN_VS].j = j;
    stats[STAT_MIN_VS].k = k;
    stats[STAT_MIN_VS].val = node->vs;
  }
  if (node->rho < stats[STAT_MIN_RHO].val) {
    stats[STAT_MIN_RHO].i = i;
    stats[STAT_MIN_RHO].j = j;
    stats[STAT_MIN_RHO].k = k;
    stats[STAT_MIN_RHO].val = node->rho;
  }
  
  /* Find minimum vp/vs ratio */
  if (node->vp/node->vs < stats[STAT_MIN_RATIO].val) {
    stats[STAT_MIN_RATIO].i = i;
    stats[STAT_MIN_RATIO].j = j;
    stats[STAT_MIN_RATIO].k = k;
    stats[STAT_MIN_RATIO].val = node->vp/node->vs;
  }
  
  return(0);
}


/* Calculate and store statistics */
int calc_stats_list(int i_start, int i_end, int j_start, int j_end, int k, 
		    mesh_ijk32_t *node_buf, stat_t *stats)
{
  int i, j, n;

  n = 0;
  for (j = j_start; j < j_end; j++) {  
    for (i = i_start; i < i_end; i++) {

      /* Find maximum vp,vs,rho */
      if (node_buf[n].vp > stats[STAT_MAX_VP].val) {
	stats[STAT_MAX_VP].i = i;
	stats[STAT_MAX_VP].j = j;
	stats[STAT_MAX_VP].k = k;
	stats[STAT_MAX_VP].val = node_buf[n].vp;
      }
      if (node_buf[n].vs > stats[STAT_MAX_VS].val) {
	stats[STAT_MAX_VS].i = i;
	stats[STAT_MAX_VS].j = j;
	stats[STAT_MAX_VS].k = k;
	stats[STAT_MAX_VS].val = node_buf[n].vs;
      }
      if (node_buf[n].rho > stats[STAT_MAX_RHO].val) {
	stats[STAT_MAX_RHO].i = i;
	stats[STAT_MAX_RHO].j = j;
	stats[STAT_MAX_RHO].k = k;
	stats[STAT_MAX_RHO].val = node_buf[n].rho;
      }
      
      /* Find minimum vp,vs,rho */
      if (node_buf[n].vp < stats[STAT_MIN_VP].val) {
	stats[STAT_MIN_VP].i = i;
	stats[STAT_MIN_VP].j = j;
	stats[STAT_MIN_VP].k = k;
	stats[STAT_MIN_VP].val = node_buf[n].vp;
      }
      if (node_buf[n].vs < stats[STAT_MIN_VS].val) {
	stats[STAT_MIN_VS].i = i;
	stats[STAT_MIN_VS].j = j;
	stats[STAT_MIN_VS].k = k;
	stats[STAT_MIN_VS].val = node_buf[n].vs;
      }
      if (node_buf[n].rho < stats[STAT_MIN_RHO].val) {
	stats[STAT_MIN_RHO].i = i;
	stats[STAT_MIN_RHO].j = j;
	stats[STAT_MIN_RHO].k = k;
	stats[STAT_MIN_RHO].val = node_buf[n].rho;
      }
      
      /* Find minimum vp/vs ratio */
      if (node_buf[n].vp/node_buf[n].vs < stats[STAT_MIN_RATIO].val) {
	stats[STAT_MIN_RATIO].i = i;
	stats[STAT_MIN_RATIO].j = j;
	stats[STAT_MIN_RATIO].k = k;
	stats[STAT_MIN_RATIO].val = node_buf[n].vp/node_buf[n].vs;
      }
      n++;
    }
  }

  return(0);
}
