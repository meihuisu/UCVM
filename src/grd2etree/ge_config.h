#ifndef GE_CONFIG_H
#define GE_CONFIG_H

#include "ge_dtypes.h"

/* Read in configuration file and populate config structure */
int read_config(int myid, int nproc, const char *cfgfile, ge_cfg_t *cfg);

/* Dump config to stdout */
int disp_config(ge_cfg_t *cfg);

#endif

