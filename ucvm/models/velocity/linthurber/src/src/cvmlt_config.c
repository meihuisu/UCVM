#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cvmlt_config.h"
#include "cvmlt_utils.h"

/* Whitespace characters */
const char *CVMLT_WHITESPACE = " \t\n";


/* Strip whitespace from string */
void cvmlt_strip_whitespace(char *str)
{
  int i1, i2;
  int len;

  i1 = 0;
  i2 = 0;
  len = strlen(str);

  for (i2 = 0; i2 < len; i2++) {
    if (strchr(CVMLT_WHITESPACE, str[i2]) == NULL) {
      str[i1++] = str[i2];
    }
  }
  str[i1] = '\0';
  return;
}


/* Strip trailing whitespace from string */
void cvmlt_strip_trailing_whitespace(char *str)
{
  int i;

  i = strlen(str);
  while (strchr(CVMLT_WHITESPACE, str[i-1]) != NULL) {
    str[i-1] = '\0';
    i = i - 1;
  }
  return;
}


/* Parse config file */
cvmlt_config_t *cvmlt_parse_config(const char *file)
{
  FILE *fp;
  char line[CVMLT_CONFIG_MAX_STR];
  char *name, *value;
  cvmlt_config_t celem;
  cvmlt_config_t *chead = NULL;
  cvmlt_config_t *cnew;

  fp = fopen(file, "r");
  if (fp == NULL) {
    fprintf(stderr, "Failed to open config %s\n", file);
    return(NULL);
  }

  while (!feof(fp)) {
    if ((fgets(line, CVMLT_CONFIG_MAX_STR, fp) != NULL) && 
	(strlen(line) > 0)) {
      memset(&celem, 0, sizeof(cvmlt_config_t));
      name = strtok(line, "=");
      if (name == NULL) {
	continue;
      }
      strcpy(celem.name, name);
      cvmlt_strip_whitespace(celem.name);
      if ((celem.name[0] == '#') || (strlen(celem.name) == 0)) {
        continue;
      }
      value = name + strlen(name) + 1;
      if (strlen(value) == 0) {
	continue;
      }
      strcpy(celem.value, value);
      cvmlt_strip_trailing_whitespace(celem.value);
      cnew = (cvmlt_config_t *)malloc(sizeof(cvmlt_config_t));
      memcpy(cnew, &celem, sizeof(cvmlt_config_t));
      cnew->next = chead;
      chead = cnew;
    }
  }

  fclose(fp);
  return(chead);
}


/* Search for a name in the list */
cvmlt_config_t *cvmlt_find_name(cvmlt_config_t *chead, const char *name)
{
  cvmlt_config_t *cptr;

  cptr = chead;
  while (cptr != NULL) {
    if (strcmp(name, cptr->name) == 0) {
      break;
    }
    cptr = cptr->next;
  }

  return(cptr);
}


/* Dump config to screen */
int cvmlt_dump_config(cvmlt_config_t *chead)
{
  cvmlt_config_t *cptr;

  cptr = chead;
  fprintf(stderr, "Parsed Config:\n");
  while (cptr != NULL) {
    fprintf(stderr, "\t%s : %s\n", cptr->name, cptr->value);
    cptr = cptr->next;
  }
  return(0);
}


/* Free config list */
int cvmlt_free_config(cvmlt_config_t *chead)
{
  cvmlt_config_t *cptr;
  cvmlt_config_t *ctmp;

  cptr = chead;
  while (cptr != NULL) {
    ctmp = cptr;
    cptr = cptr->next;
    free(ctmp);
  }

  return(0);
}

