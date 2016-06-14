#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ucvm_dtypes.h"
#include "ucvm_config.h"
#include "ucvm_utils.h"

/* Whitespace characters */
const char *UCVM_WHITESPACE = " \t\n";


/* Strip whitespace from string */
void ucvm_strip_whitespace(char *str)
{
  int i1, i2;
  int len;

  i1 = 0;
  i2 = 0;
  len = strlen(str);

  for (i2 = 0; i2 < len; i2++) {
    if (strchr(UCVM_WHITESPACE, str[i2]) == NULL) {
      str[i1++] = str[i2];
    }
  }
  str[i1] = '\0';
  return;
}


/* Strip trailing whitespace from string */
void ucvm_strip_trailing_whitespace(char *str)
{
  int i;

  i = strlen(str);
  while (strchr(UCVM_WHITESPACE, str[i-1]) != NULL) {
    str[i-1] = '\0';
    i = i - 1;
  }
  return;
}


/* Parse config file */
ucvm_config_t *ucvm_parse_config(const char *file)
{
  FILE *fp;
  char line[UCVM_CONFIG_MAX_STR];
  char *name, *value;
  ucvm_config_t celem;
  ucvm_config_t *chead = NULL;
  ucvm_config_t *cnew;

  if (!ucvm_is_file(file)) {
    fprintf(stderr, "Config file %s is not a valid file\n", file);
    return(NULL);
  }

  fp = fopen(file, "r");
  if (fp == NULL) {
    fprintf(stderr, "Failed to open config %s\n", file);
    return(NULL);
  }

  while (!feof(fp)) {
    if ((fgets(line, UCVM_CONFIG_MAX_STR, fp) != NULL) && 
	(strlen(line) > 0)) {
      memset(&celem, 0, sizeof(ucvm_config_t));
      name = strtok(line, "=");
      if (name == NULL) {
	continue;
      }
      ucvm_strcpy(celem.name, name, UCVM_CONFIG_MAX_STR);
      ucvm_strip_whitespace(celem.name);
      if ((celem.name[0] == '#') || (strlen(celem.name) == 0)) {
        continue;
      }
      value = name + strlen(name) + 1;
      if (strlen(value) == 0) {
	continue;
      }
      ucvm_strcpy(celem.value, value, UCVM_CONFIG_MAX_STR);
      ucvm_strip_trailing_whitespace(celem.value);
      cnew = (ucvm_config_t *)malloc(sizeof(ucvm_config_t));
      memcpy(cnew, &celem, sizeof(ucvm_config_t));
      cnew->next = chead;
      chead = cnew;
    }
  }

  fclose(fp);
  return(chead);
}


/* Search for a name in the list */
ucvm_config_t *ucvm_find_name(ucvm_config_t *chead, const char *name)
{
  ucvm_config_t *cptr;

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
int ucvm_dump_config(ucvm_config_t *chead)
{
  ucvm_config_t *cptr;

  cptr = chead;
  fprintf(stderr, "Parsed Config:\n");
  while (cptr != NULL) {
    fprintf(stderr, "\t%s : %s\n", cptr->name, cptr->value);
    cptr = cptr->next;
  }
  return(UCVM_CODE_SUCCESS);
}


/* Free config list */
int ucvm_free_config(ucvm_config_t *chead)
{
  ucvm_config_t *cptr;
  ucvm_config_t *ctmp;

  cptr = chead;
  while (cptr != NULL) {
    ctmp = cptr;
    cptr = cptr->next;
    free(ctmp);
  }

  return(UCVM_CODE_SUCCESS);
}

