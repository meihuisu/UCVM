#ifndef UE_CONFIG_H
#define UE_CONFIG_H

#include "ue_dtypes.h"

/* Read in configuration file and populate config structure */
int read_config(int myid, int nproc, const char *cfgfile, ue_cfg_t *cfg);

/* Dump config to stdout */
int disp_config(ue_cfg_t *cfg);

#endif

