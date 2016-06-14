#ifndef UCVM_CONFIG_H
#define UCVM_CONFIG_H

/* Maximum string length */
#define UCVM_CONFIG_MAX_STR 512

/* Config entry */
typedef struct ucvm_config_t {
  char name[UCVM_CONFIG_MAX_STR];
  char value[UCVM_CONFIG_MAX_STR];
  struct ucvm_config_t *next;
} ucvm_config_t;

/* Parse config file */
ucvm_config_t *ucvm_parse_config(const char *file);

/* Return next entry containing name as a key */
ucvm_config_t *ucvm_find_name(ucvm_config_t *chead, const char *name);

/* Dump config to screen */
int ucvm_dump_config(ucvm_config_t *chead);

/* Free config list */
int ucvm_free_config(ucvm_config_t *chead);

#endif
