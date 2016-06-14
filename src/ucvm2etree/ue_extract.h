#ifndef UE_EXTRACT_H
#define UE_EXTRACT_H

#include "ue_dtypes.h"

/* Insert 2D grid at required resolution in a buffer */
int insert_grid_buf(ue_cfg_t *cfg,
		     etree_addr_t *pnts, ucvm_data_t *props, 
		     int num_points);


/* Insert 2D grid at required resolution in etree */
int insert_grid_etree(ue_cfg_t *cfg,
		      etree_addr_t *pnts, ucvm_data_t *props, 
		      int num_points);


/* Perform extraction */
int extract(ue_cfg_t *cfg, 
	    int (*etree_writer)(ue_cfg_t *cfg, 
				etree_addr_t *pnts, 
				ucvm_data_t *props, 
				int num_points),
	    int col, 
	    ucvm_point_t *cvm_pnts,
	    etree_addr_t *etree_pnts,
	    ucvm_data_t *props,
	    unsigned long *num_extracted);


#endif

