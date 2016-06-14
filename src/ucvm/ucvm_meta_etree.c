#include <stdio.h>
#include <string.h>
#include "ucvm_meta_etree.h"
#include "ucvm_utils.h"

/* Constants */
#define UCVM_META_ETREE_SEP "|"


/* Etree schemas */
#define UCVM_META_ETREE_MODEL_SCHEMA "float Vp; float Vs; float density;"
#define UCVM_META_ETREE_MAP_SCHEMA "float surf; float vs30;"


/* Etree metadatas */
/* CMU Etrees */
#define UCVM_META_ETREE_CMU_NUM_FIELDS 3
#define UCVM_META_ETREE_CMU_METASCHEMA "Vp(float);Vs(float);density(float)"
/* UCVM Etrees */
#define UCVM_META_ETREE_UCVM_VERSION "SCEC_CVM_V1"
#define UCVM_META_ETREE_UCVM_METASCHEMA "vp,float,4,m/s;vs,float,4,m/s;rho,float,4,kg/m^3"
/* Map Etrees */
#define UCVM_META_ETREE_MAP_VERSION "SCEC_MAP_V1"
#define UCVM_META_ETREE_MAP_METASCHEMA "surf,float,4,m;vs30,float,4,m/s;"



/* Pack CMU Etree schema */
int ucvm_schema_etree_cmu_pack(char *schemastr, int len)
{
  int slen;

  if ((schemastr == NULL) || (len < UCVM_META_MIN_SCHEMA_LEN)) {
    return(UCVM_CODE_ERROR);
  }

  slen = strlen(UCVM_META_ETREE_MODEL_SCHEMA);
  if (slen > len) {
    slen = len;
  }
  memset(schemastr, 0, len);
  strncpy(schemastr, UCVM_META_ETREE_MODEL_SCHEMA, slen);
  return(UCVM_CODE_SUCCESS);
}


/* Pack UCVM Etree schema */
int ucvm_schema_etree_ucvm_pack(char *schemastr, int len)
{
  int slen;

  if ((schemastr == NULL) || (len < UCVM_META_MIN_SCHEMA_LEN)) {
    return(UCVM_CODE_ERROR);
  }

  slen = strlen(UCVM_META_ETREE_MODEL_SCHEMA);
  if (slen > len) {
    slen = len;
  }
  memset(schemastr, 0, len);
  strncpy(schemastr, UCVM_META_ETREE_MODEL_SCHEMA, slen);
  return(UCVM_CODE_SUCCESS);
}


/* Pack UCVM Map Etree schema */
int ucvm_schema_etree_map_pack(char *schemastr, int len)
{
  int slen;

  if ((schemastr == NULL) || (len < UCVM_META_MIN_SCHEMA_LEN)) {
    return(UCVM_CODE_ERROR);
  }

  slen = strlen(UCVM_META_ETREE_MAP_SCHEMA);
  if (slen > len) {
    slen = len;
  }
  memset(schemastr, 0, len);
  strncpy(schemastr, UCVM_META_ETREE_MAP_SCHEMA, slen);
  return(UCVM_CODE_SUCCESS);
}


/* Pack CMU Etree metadata */
int ucvm_meta_etree_cmu_pack(ucvm_meta_cmu_t *meta,
			     char *metastr, int len)
{
  if ((meta == NULL) || (metastr == NULL) || 
      (len < UCVM_META_MIN_META_LEN)) {
    return(UCVM_CODE_ERROR);
  }

  sprintf(metastr,
	  "Title:%s Author:%s Date:%s %u %s %f %f %f %f %f %f %u %u %u", 
	  meta->title, meta->author, meta->date,
	  UCVM_META_ETREE_CMU_NUM_FIELDS, UCVM_META_ETREE_CMU_METASCHEMA,
	  meta->origin.coord[1], meta->origin.coord[0],
	  meta->dims_xyz.coord[0], meta->dims_xyz.coord[1], 
	  meta->origin.coord[2], meta->dims_xyz.coord[2], 
	  meta->ticks_xyz.dim[0], meta->ticks_xyz.dim[1], 
	  meta->ticks_xyz.dim[2]);

  /* If string overflow, too late the damage has been done. Cap string
     length anyway. */
  if (strlen(metastr) > len) {
    metastr[len-1] = '\0';
  }
  return(UCVM_CODE_SUCCESS);
}


/* Unpack CMU Etree metadata */
int ucvm_meta_etree_cmu_unpack(char *metastr, ucvm_meta_cmu_t *meta)
{
  unsigned int numvar;
  char schema[UCVM_META_MIN_SCHEMA_LEN];

  if ((metastr == NULL) || (meta == NULL)) {
    return(UCVM_CODE_ERROR);
  }

  if (sscanf(metastr,
	     "Title:%s Author:%s Date:%s %u %s %lf %lf %lf %lf %lf %lf %u %u %u", 
	     meta->title, meta->author, meta->date, &numvar, schema,
	     &(meta->origin.coord[1]), &(meta->origin.coord[0]),
	     &(meta->dims_xyz.coord[0]), &(meta->dims_xyz.coord[1]), 
	     &(meta->origin.coord[2]), &(meta->dims_xyz.coord[2]), 
	     &(meta->ticks_xyz.dim[0]), &(meta->ticks_xyz.dim[1]), 
	     &(meta->ticks_xyz.dim[2])) != 14) {
    fprintf(stderr, "Failed to parse CMU meta data\n");
    return(UCVM_CODE_ERROR);
  }

  if (numvar != UCVM_META_ETREE_CMU_NUM_FIELDS) {
    fprintf(stderr, "Unsupported number of variables in CMU meta data\n");
    return(UCVM_CODE_ERROR);
  }

  if (strcmp(schema, UCVM_META_ETREE_CMU_METASCHEMA) != 0) {
    fprintf(stderr, "Unsupported schema in CMU meta data\n");
    return(UCVM_CODE_ERROR);
  }

  return(UCVM_CODE_SUCCESS);
}


/* Pack UCVM Etree metadata */
int ucvm_meta_etree_ucvm_pack(ucvm_meta_ucvm_t *meta,
			      char *metastr, int len)
{

  if ((meta == NULL) || (metastr == NULL) || 
      (len < UCVM_META_MIN_META_LEN)) {
    return(UCVM_CODE_ERROR);
  }

  /* Verify no separators in input strings */
  if ((strstr(meta->title, "|") != NULL) || 
      (strstr(meta->author, "|") != NULL) ||
      (strstr(meta->date, "|") != NULL) ||
      (strstr(meta->projstr, "|") != NULL)) {
    return(UCVM_CODE_ERROR);
  }

  sprintf(metastr,
	  "%s|%s|%s|%s|%lf|%lf|%lf|%s|%s|%lf,%lf,%lf|%lf|%lf,%lf,%lf|%u,%u,%u", 
	  UCVM_META_ETREE_UCVM_VERSION, meta->title, 
	  meta->author, meta->date,
	  meta->vs_min, meta->max_freq, meta->ppwl, 
	  UCVM_META_ETREE_UCVM_METASCHEMA,
	  meta->projstr, meta->origin.coord[0], 
	  meta->origin.coord[1], meta->origin.coord[2],
	  meta->rot, meta->dims_xyz.coord[0], 
	  meta->dims_xyz.coord[1], meta->dims_xyz.coord[2],
	  meta->ticks_xyz.dim[0], meta->ticks_xyz.dim[1], 
	  meta->ticks_xyz.dim[2]);

  /* If string overflow, too late the damage has been done. Cap string
     length anyway. */
  if (strlen(metastr) > len) {
    metastr[len-1] = '\0';
  }

  return(UCVM_CODE_SUCCESS);
}


/* Unpack UCVM Etree metadata */
int ucvm_meta_etree_ucvm_unpack(char *metastr, ucvm_meta_ucvm_t *meta)
{
  int index;
  char *token;

  if ((metastr == NULL) || (meta == NULL)) {
    return(UCVM_CODE_ERROR);
  }

  /* Looks overlay elaborate, but allows spaces to be present 
     in any string */
  index = 0;
  token = strtok(metastr, UCVM_META_ETREE_SEP);
  while (token != NULL) {
    switch (index) {
    case 0:
      if (strcmp(token, UCVM_META_ETREE_UCVM_VERSION) != 0) {
	fprintf(stderr, "Etree version mismatch, found %s expected %s\n",
		token, UCVM_META_ETREE_UCVM_VERSION);
	return(UCVM_CODE_ERROR);
      }
      break;
    case 1:
      ucvm_strcpy(meta->title, token, UCVM_META_MAX_STRING_LEN);
      break;
    case 2:
      ucvm_strcpy(meta->author, token, UCVM_META_MAX_STRING_LEN);
      break;
    case 3:
      ucvm_strcpy(meta->date, token, UCVM_META_MAX_STRING_LEN);
      break;
    case 4:
      if (sscanf(token, "%lf", &(meta->vs_min)) != 1) {
	fprintf(stderr, "Failed to parse vs_min in etree metadata\n");
	return(UCVM_CODE_ERROR);
      }
      break;
    case 5:
      if (sscanf(token, "%lf", &(meta->max_freq)) != 1) {
	fprintf(stderr, "Failed to parse max_freq in etree metadata\n");
	return(UCVM_CODE_ERROR);
      }
      break;
    case 6:
      if (sscanf(token, "%lf", &(meta->ppwl)) != 1) {
	fprintf(stderr, "Failed to parse ppwl in etree metadata\n");
	return(UCVM_CODE_ERROR);
      }
      break;
    case 7:
      if (strcmp(token, UCVM_META_ETREE_UCVM_METASCHEMA) != 0) {
	fprintf(stderr, "Etree schema mismatch, found %s expected %s\n",
		token, UCVM_META_ETREE_UCVM_METASCHEMA);
	return(UCVM_CODE_ERROR);
      }
      break;
    case 8:
      ucvm_strcpy(meta->projstr, token, UCVM_MAX_PROJ_LEN);
      break;
    case 9:
      if (sscanf(token, "%lf,%lf,%lf", &(meta->origin.coord[0]),
		 &(meta->origin.coord[1]), 
		 &(meta->origin.coord[2])) != 3) {
	fprintf(stderr, "Failed to parse origin in etree metadata\n");
	return(UCVM_CODE_ERROR);
      }
      break;
    case 10:
      if (sscanf(token, "%lf", &(meta->rot)) != 1) {
	fprintf(stderr, "Failed to parse rot in etree metadata\n");
	return(UCVM_CODE_ERROR);
      }
      break;
    case 11:
      if (sscanf(token, "%lf,%lf,%lf", &(meta->dims_xyz.coord[0]),
		 &(meta->dims_xyz.coord[1]), 
		 &(meta->dims_xyz.coord[2])) != 3) {
	fprintf(stderr, "Failed to parse dims_xyz in etree metadata\n");
	return(UCVM_CODE_ERROR);
      }
      break;
    case 12:
      if (sscanf(token, "%u,%u,%u", &(meta->ticks_xyz.dim[0]),
		 &(meta->ticks_xyz.dim[1]), 
		 &(meta->ticks_xyz.dim[2])) != 3) {
	fprintf(stderr, "Failed to parse ticks_xyz in etree metadata\n");
	return(UCVM_CODE_ERROR);
      }
      break;
    default:
      fprintf(stderr, "Unexpected elements in etree metadata\n");
      return(UCVM_CODE_ERROR);
      break;
    }
    index++;
    token = strtok(NULL, UCVM_META_ETREE_SEP);
  }

  if (index != 13) {
    fprintf(stderr, "Unexpected format in etree metadata\n");
    return(UCVM_CODE_ERROR);
  }

  return(UCVM_CODE_SUCCESS);
}


/* Pack Map Etree metadata */
int ucvm_meta_etree_map_pack(ucvm_meta_map_t *meta,
			     char *metastr, int len)
{

  if ((meta == NULL) || (metastr == NULL) || 
      (len < UCVM_META_MIN_META_LEN)) {
    return(UCVM_CODE_ERROR);
  }

  /* Verify no separators in input strings */
  if ((strstr(meta->title, "|") != NULL) || 
      (strstr(meta->author, "|") != NULL) ||
      (strstr(meta->date, "|") != NULL) ||
      (strstr(meta->projstr, "|") != NULL)) {
    return(UCVM_CODE_ERROR);
  }

  sprintf(metastr,
	  "%s|%s|%s|%s|%lf|%s|%s|%lf,%lf,%lf|%lf|%lf,%lf,%lf|%u,%u,%u", 
	  UCVM_META_ETREE_MAP_VERSION, meta->title, 
	  meta->author, meta->date,
	  meta->spacing, 
	  UCVM_META_ETREE_MAP_METASCHEMA,
	  meta->projstr, meta->origin.coord[0], 
	  meta->origin.coord[1], meta->origin.coord[2],
	  meta->rot, meta->dims_xyz.coord[0], 
	  meta->dims_xyz.coord[1], meta->dims_xyz.coord[2],
	  meta->ticks_xyz.dim[0], meta->ticks_xyz.dim[1], 
	  meta->ticks_xyz.dim[2]);

  /* If string overflow, too late the damage has been done. Cap string
     length anyway. */
  if (strlen(metastr) > len) {
    metastr[len-1] = '\0';
  }

  return(UCVM_CODE_SUCCESS);
}


/* Unpack Map Etree metadata */
int ucvm_meta_etree_map_unpack(char *metastr, ucvm_meta_map_t *meta)
{
  int index;
  char *token;

  if ((metastr == NULL) || (meta == NULL)) {
    return(UCVM_CODE_ERROR);
  }

  /* Looks overlay elaborate, but allows spaces to be present 
     in any string */
  index = 0;
  token = strtok(metastr, UCVM_META_ETREE_SEP);
  while (token != NULL) {
    switch (index) {
    case 0:
      if (strcmp(token, UCVM_META_ETREE_MAP_VERSION) != 0) {
	fprintf(stderr, "Etree version mismatch, found %s expected %s\n",
		token, UCVM_META_ETREE_MAP_VERSION);
	return(UCVM_CODE_ERROR);
      }
      break;
    case 1:
      ucvm_strcpy(meta->title, token, UCVM_META_MAX_STRING_LEN);
      break;
    case 2:
      ucvm_strcpy(meta->author, token, UCVM_META_MAX_STRING_LEN);
      break;
    case 3:
      ucvm_strcpy(meta->date, token, UCVM_META_MAX_STRING_LEN);
      break;
    case 4:
      if (sscanf(token, "%lf", &(meta->spacing)) != 1) {
	fprintf(stderr, "Failed to parse spacing in etree metadata\n");
	return(UCVM_CODE_ERROR);
      }
      break;
    case 5:
      if (strcmp(token, UCVM_META_ETREE_MAP_METASCHEMA) != 0) {
	fprintf(stderr, "Etree schema mismatch, found %s expected %s\n",
		token, UCVM_META_ETREE_MAP_METASCHEMA);
	return(UCVM_CODE_ERROR);
      }
      break;
    case 6:
      ucvm_strcpy(meta->projstr, token, UCVM_MAX_PROJ_LEN);
      break;
    case 7:
      if (sscanf(token, "%lf,%lf,%lf", &(meta->origin.coord[0]),
		 &(meta->origin.coord[1]), 
		 &(meta->origin.coord[2])) != 3) {
	fprintf(stderr, "Failed to parse origin in etree metadata\n");
	return(UCVM_CODE_ERROR);
      }
      break;
    case 8:
      if (sscanf(token, "%lf", &(meta->rot)) != 1) {
	fprintf(stderr, "Failed to parse rot in etree metadata\n");
	return(UCVM_CODE_ERROR);
      }
      break;
    case 9:
      if (sscanf(token, "%lf,%lf,%lf", &(meta->dims_xyz.coord[0]),
		 &(meta->dims_xyz.coord[1]), 
		 &(meta->dims_xyz.coord[2])) != 3) {
	fprintf(stderr, "Failed to parse dims_xyz in etree metadata\n");
	return(UCVM_CODE_ERROR);
      }
      break;
    case 10:
      if (sscanf(token, "%u,%u,%u", &(meta->ticks_xyz.dim[0]),
		 &(meta->ticks_xyz.dim[1]), 
		 &(meta->ticks_xyz.dim[2])) != 3) {
	fprintf(stderr, "Failed to parse ticks_xyz in etree metadata\n");
	return(UCVM_CODE_ERROR);
      }
      break;
    default:
      fprintf(stderr, "Unexpected elements in etree metadata\n");
      return(UCVM_CODE_ERROR);
      break;
    }
    index++;
    token = strtok(NULL, UCVM_META_ETREE_SEP);
  }

  if (index != 11) {
    fprintf(stderr, "Unexpected format in etree metadata\n");
    return(UCVM_CODE_ERROR);
  }

  return(UCVM_CODE_SUCCESS);
}
