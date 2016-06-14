#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "grd_config.h"

/* Whitespace characters */
const char *GRD_WHITESPACE = " \t\r\n";
const char *GRD_SEPARATOR = " ";


/* Strip whitespace from string */
void grd_strip_whitespace(char *str)
{
  int i1, i2;
  int len;

  i1 = 0;
  i2 = 0;
  len = strlen(str);

  for (i2 = 0; i2 < len; i2++) {
    if (strchr(GRD_WHITESPACE, str[i2]) == NULL) {
      str[i1++] = str[i2];
    }
  }
  str[i1] = '\0';
  return;
}


/* Strip trailing whitespace from string */
void grd_strip_trailing_whitespace(char *str)
{
  int i;

  i = strlen(str);
  while (strchr(GRD_WHITESPACE, str[i-1]) != NULL) {
    str[i-1] = '\0';
    i = i - 1;
  }
  return;
}


/* Convert to lowercase */
void grd_lowercase(char *str)
{
  int i;

  for(i = strlen(str); i >= 0; i--) {
    str[i] = tolower(str[i]);
  }
  
  return;
}


/* Parse config file */
grd_config_t *grd_parse_config(const char *file)
{
  FILE *fp;
  char line[GRD_CONFIG_MAX_STR];
  char *token;
  grd_config_t celem;
  grd_config_t *chead = NULL;
  grd_config_t *cnew;

  fp = fopen(file, "r");
  if (fp == NULL) {
    fprintf(stderr, "Failed to open config %s\n", file);
    return(NULL);
  }

  while (!feof(fp)) {
    fgets(line, GRD_CONFIG_MAX_STR, fp);
    if (strlen(line) > 0) {
      token = strtok(line, GRD_SEPARATOR);
      if (token == NULL) {
	continue;
      }
      grd_lowercase(token);
      strcpy(celem.name, token);
      grd_strip_whitespace(celem.name);
      if (celem.name[0] == '#') {
	continue;
      }
      strcpy(celem.value, &(line[strlen(token) + 1]));
      grd_strip_whitespace(celem.value);
      cnew = (grd_config_t *)malloc(sizeof(grd_config_t));
      memcpy(cnew, &celem, sizeof(grd_config_t));
      cnew->next = chead;
      chead = cnew;
    }

  }

  fclose(fp);
  return(chead);
}


/* Search for a name in the list */
grd_config_t *grd_find_name(grd_config_t *chead, const char *name)
{
  grd_config_t *cptr;

  cptr = chead;
  while (cptr != NULL) {
    if (strcmp(name, cptr->name) == 0) {
      break;
    }
    cptr = cptr->next;
  }

  return(cptr);
}


/* Free config list */
int grd_free_config(grd_config_t *chead)
{
  grd_config_t *cptr;
  grd_config_t *ctmp;

  cptr = chead;
  while (cptr != NULL) {
    ctmp = cptr;
    cptr = cptr->next;
    free(ctmp);
  }

  return(0);
}

