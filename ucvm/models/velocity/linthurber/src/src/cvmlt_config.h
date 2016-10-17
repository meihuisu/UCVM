#ifndef CVMLT_CONFIG_H
#define CVMLT_CONFIG_H

/* Maximum string length */
#define CVMLT_CONFIG_MAX_STR 512

/* Config entry */
typedef struct cvmlt_config_t {
  char name[CVMLT_CONFIG_MAX_STR];
  char value[CVMLT_CONFIG_MAX_STR];
  struct cvmlt_config_t *next;
} cvmlt_config_t;

/* Parse config file */
cvmlt_config_t *cvmlt_parse_config(const char *file);

/* Return next entry containing name as a key */
cvmlt_config_t *cvmlt_find_name(cvmlt_config_t *chead, const char *name);

/* Dump config to screen */
int cvmlt_dump_config(cvmlt_config_t *chead);

/* Free config list */
int cvmlt_free_config(cvmlt_config_t *chead);

#endif
