#ifndef PATCH_CONFIG_H
#define PATCH_CONFIG_H

#include "patch_dtypes.h"

/* Read in configuration file and populate config structure */
int read_config(const char *cfgfile, patch_cfg_t *cfg);

/* Dump config to stdout */
int disp_config(patch_cfg_t *cfg);

#endif

