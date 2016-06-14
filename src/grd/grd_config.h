#ifndef GRD_CONFIG_H
#define GRD_CONFIG_H

/* Maximum string length */
#define GRD_CONFIG_MAX_STR 512

/* Config entry */
typedef struct grd_config_t {
  char name[GRD_CONFIG_MAX_STR];
  char value[GRD_CONFIG_MAX_STR];
  struct grd_config_t *next;
} grd_config_t;

/* Parse config file */
grd_config_t *grd_parse_config(const char *file);

/* Search for a name in the list */
grd_config_t *grd_find_name(grd_config_t *chead, const char *name);

/* Free config list */
int grd_free_config(grd_config_t *chead);

#endif
