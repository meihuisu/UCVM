#ifndef UM_CONFIG_H
#define UM_CONFIG_H

/* Read in configuration file and populate config structure */
int read_config(int myid, int nproc, const char *cfgfile, mesh_config_t *cfg);

/* Dump config to stdout */
int disp_config(mesh_config_t *cfg);

#endif

