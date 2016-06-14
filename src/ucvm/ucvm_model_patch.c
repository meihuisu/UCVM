#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ucvm_utils.h"
#include "ucvm_config.h"
#include "ucvm_meta_patch.h"
#include "ucvm_model_patch.h"
#include "ucvm_proj_ucvm.h"


/* Constants */
#define UCVM_PATCH_POWER_FACTOR 2.0


/* Data bucket */
typedef struct ucvm_bucket_t {
  double vp;
  double vs;
  double rho;
  double dist;
} ucvm_bucket_t;


/* Patch record */
typedef struct ucvm_patch_t {
  int valid;
  ucvm_modelconf_t conf;
  char version[UCVM_MAX_VERSION_LEN];
  double spacing;
  ucvm_dim_t dims;
  ucvm_point_t sizes;
  ucvm_psurf_t surfs[2][2];
  ucvm_proj_t proj;
  int ucvm_num_buckets;
  ucvm_bucket_t *bucketlist;
} ucvm_patch_t;


/* Patch list */
int ucvm_num_patches = 0;
ucvm_patch_t ucvm_patch_list[UCVM_MAX_MODELS];


/* Init Patch */
int ucvm_patch_model_init(int id, ucvm_modelconf_t *conf)
{
  int i, j, n;
  ucvm_patch_t *mptr;
  ucvm_config_t *chead;
  ucvm_config_t *cptr;

  /* Config params */
  char projstr[UCVM_MAX_PROJ_LEN];
  ucvm_point_t origin;
  double rot;

  /* File IO */
  FILE *fp;
  char key[UCVM_CONFIG_MAX_STR];

  /* Startup initialization */
  if (ucvm_num_patches == 0) {
    memset(ucvm_patch_list, 0, sizeof(ucvm_patch_t) * UCVM_MAX_MODELS);
  }

  if ((conf->config == NULL) || (strlen(conf->config) == 0)) {
    fprintf(stderr, "No config path defined for model %s\n", conf->label);
    return(UCVM_CODE_ERROR);
  }

  if ((id < 0) || (id >= UCVM_MAX_MODELS)) {
    fprintf(stderr, "Model ID is outside of max range\n");
    return(UCVM_CODE_ERROR);
  }

  if (!ucvm_is_file(conf->config)) {
    fprintf(stderr, "Patch conf file %s is not a valid file\n", 
	    conf->config);
    return(UCVM_CODE_ERROR);
  }

  /* Save model conf */
  memcpy(&ucvm_patch_list[id].conf, conf, sizeof(ucvm_modelconf_t));

  mptr = &(ucvm_patch_list[id]);

  /* Read conf file */
  chead = ucvm_parse_config(conf->config);
  if (chead == NULL) {
    fprintf(stderr, "Failed to parse config file %s\n", conf->config);
    return(UCVM_CODE_ERROR);
  }

  cptr = ucvm_find_name(chead, "version");
  if (cptr == NULL) {
    fprintf(stderr, "Failed to find version in config\n");
    return(UCVM_CODE_ERROR);
  }
  snprintf(mptr->version, UCVM_MAX_VERSION_LEN, "%s", cptr->value);

  cptr = ucvm_find_name(chead, "proj");
  if (cptr == NULL) {
    fprintf(stderr, "Failed to find proj in config\n");
    return(UCVM_CODE_ERROR);
  }
  snprintf(projstr, UCVM_MAX_PROJ_LEN, "%s", cptr->value);

  /* Parse origin longitude */
  cptr = ucvm_find_name(chead, "lon_0");
  if (cptr == NULL) {
    fprintf(stderr, "Failed to find lon_0 in config\n");
    return(UCVM_CODE_ERROR);
  }
  if(sscanf(cptr->value, "%lf", &origin.coord[0]) != 1) {
    fprintf(stderr, "Failed to parse lon_0 in config\n");
    return(UCVM_CODE_ERROR);
  }

  /* Parse origin latitude */
  cptr = ucvm_find_name(chead, "lat_0");
  if (cptr == NULL) {
    fprintf(stderr, "Failed to find lat_0 in config\n");
    return(UCVM_CODE_ERROR);
  }
  if(sscanf(cptr->value, "%lf", &origin.coord[1]) != 1) {
    fprintf(stderr, "Failed to parse lat_0 in config\n");
    return(UCVM_CODE_ERROR);
  }

  /* Parse rotation angle */
  cptr = ucvm_find_name(chead, "rot");
  if (cptr == NULL) {
    fprintf(stderr, "Failed to find rot in config\n");
    return(UCVM_CODE_ERROR);
  }
  if(sscanf(cptr->value, "%lf", &rot) != 1) {
    fprintf(stderr, "Failed to parse rot in config\n");
    return(UCVM_CODE_ERROR);
  }

  cptr = ucvm_find_name(chead, "x-size");
  if (cptr == NULL) {
    fprintf(stderr, "Failed to find x-size in config\n");
    return(UCVM_CODE_ERROR);
  }
  if(sscanf(cptr->value, "%lf", &(mptr->sizes.coord[0])) != 1) {
    fprintf(stderr, "Failed to find nx in config\n");
    return(UCVM_CODE_ERROR);
  }
    
  cptr = ucvm_find_name(chead, "y-size");
  if (cptr == NULL) {
    fprintf(stderr, "Failed to find y-size in config\n");
    return(UCVM_CODE_ERROR);
  }
  if(sscanf(cptr->value, "%lf", &(mptr->sizes.coord[1])) != 1){
    fprintf(stderr, "Failed to find y-size in config\n");
    return(UCVM_CODE_ERROR);
  }
    
  cptr = ucvm_find_name(chead, "z-size");
  if (cptr == NULL) {
    fprintf(stderr, "Failed to find z-size in config\n");
    return(UCVM_CODE_ERROR);
  }
  if(sscanf(cptr->value, "%lf", &(mptr->sizes.coord[2])) != 1){
    fprintf(stderr, "Failed to find z-size in config\n");
    return(UCVM_CODE_ERROR);
  }

  cptr = ucvm_find_name(chead, "spacing");
  if (cptr == NULL) {
    fprintf(stderr, "Failed to find spacing in config\n");
    return(UCVM_CODE_ERROR);
  }
  if(sscanf(cptr->value, "%lf", &(mptr->spacing)) != 1) {
    fprintf(stderr, "Failed to find spacing in config\n");
    return(UCVM_CODE_ERROR);
  }

  /* Setup surface dimensions */
  ucvm_meta_patch_getsurf(&(mptr->sizes), mptr->spacing, mptr->surfs);
  for (i = 0; i < 3; i++) {
    mptr->dims.dim[i] = mptr->sizes.coord[i]/mptr->spacing;
  }

  //printf("Surface Information:\n");
  //for (j = 0; j < 2; j++) {
  //  for (i = 0; i < 2; i++) {
  //    printf("\tSurf(%d,%d) num_points=%d\n", i,j, 
  //	     ucvm_patch_list[id].surfs[j][i].num_points);
  //    printf("\tSurf(%d,%d) p1->p2: %lf,%lf,%lf -> %lf,%lf,%lf\n",
  //	     i,j, 
  //	     mptr->surfs[j][i].corners[0].coord[0],
  //	     mptr->surfs[j][i].corners[0].coord[1],
  //	     mptr->surfs[j][i].corners[0].coord[2],
  //	     mptr->surfs[j][i].corners[1].coord[0],
  //	     mptr->surfs[j][i].corners[1].coord[1],
  //	     mptr->surfs[j][i].corners[1].coord[2]);
  // }
  //}
  //printf("\n");
  
  /* Allocate bucket list */
  mptr->ucvm_num_buckets = ((mptr->surfs[0][0].dims.dim[0]*
			     mptr->surfs[0][0].dims.dim[1]) + 
			    (mptr->surfs[0][1].dims.dim[0]*
			     mptr->surfs[0][1].dims.dim[1]) + 
			    (mptr->surfs[1][0].dims.dim[0]*
			     mptr->surfs[1][0].dims.dim[1]) +
			    (mptr->surfs[1][1].dims.dim[0]*
			     mptr->surfs[1][1].dims.dim[1]));
  mptr->bucketlist = malloc(mptr->ucvm_num_buckets * 
			    sizeof(ucvm_bucket_t));
  if (mptr->bucketlist == NULL) {
	fprintf(stderr, "Failed to allocate bucket buffer\n");
	return(UCVM_CODE_ERROR);
  }

  /* Allocate buffers and read surface files */
  for (j = 0; j < 2; j++) {
    for (i = 0; i < 2; i++) {
      mptr->surfs[j][i].props = malloc(mptr->surfs[j][i].num_points * 
				       sizeof(ucvm_ppayload_t));
      if (mptr->surfs[j][i].props == NULL) {
	fprintf(stderr, "Failed to allocate surface buffer %d,%d\n", i, j);
	return(UCVM_CODE_ERROR);
      }

      snprintf(key, UCVM_CONFIG_MAX_STR, "surf_%d_%d_path", i, j);
      cptr = ucvm_find_name(chead, key);
      if (cptr == NULL) {
	fprintf(stderr, "Failed to find %s in config\n", key);
	return(UCVM_CODE_ERROR);
      }

      fp = fopen(cptr->value, "rb");
      if (fread(mptr->surfs[j][i].props, sizeof(ucvm_ppayload_t),
		mptr->surfs[j][i].num_points, fp) != 
	  mptr->surfs[j][i].num_points) {
	fprintf(stderr, "Failed to read surf file %s\n", cptr->value);
	return(UCVM_CODE_ERROR);
      }
      fclose(fp);    

      /* Swap endian from LSB to MSB if required */
      if (system_endian() != UCVM_BYTEORDER_LSB) {
	for (n = 0; n < mptr->surfs[j][i].num_points; n++) {
	  mptr->surfs[j][i].props[n].vp = 
	    swap_endian_float(mptr->surfs[j][i].props[n].vp);
	  mptr->surfs[j][i].props[n].vs = 
	    swap_endian_float(mptr->surfs[j][i].props[n].vs);
	  mptr->surfs[j][i].props[n].rho = 
	    swap_endian_float(mptr->surfs[j][i].props[n].rho);
	}
      }  
    }
  }

  /* Setup projection */
  if (ucvm_proj_ucvm_init(projstr, &(origin), rot,
			  &(mptr->sizes), &(mptr->proj)) != UCVM_CODE_SUCCESS) {
    fprintf(stderr, "Failed to setup proj %s.\n", projstr);
     return(UCVM_CODE_ERROR);
  }

  mptr->valid = 1;
  ucvm_num_patches++;
  return(UCVM_CODE_SUCCESS);
}


/* Finalize Patch */
int ucvm_patch_model_finalize()
{
  int i, j, m;

  /* Free buffers */
  for (m = 0; m < UCVM_MAX_MODELS; m++) {
    if (ucvm_patch_list[m].valid) {
      /* Free bucket list */
      if (ucvm_patch_list[m].bucketlist != NULL) {
	free(ucvm_patch_list[m].bucketlist);
      }
      /* Free surface data buffers */
      for (j = 0; j < 2; j++) {
	for (i = 0; i < 2; i++) {
	  free(ucvm_patch_list[m].surfs[j][i].props);
	}
      }
    }
  }

  ucvm_num_patches = 0;
  memset(ucvm_patch_list, 0, sizeof(ucvm_patch_t) * UCVM_MAX_MODELS);

  return(UCVM_CODE_SUCCESS);
}


/* Version Patch */
int ucvm_patch_model_version(int id, char *ver, int len)
{
  if ((id < 0) || (id >= UCVM_MAX_MODELS) || 
      (ucvm_patch_list[id].valid == 0)) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  ucvm_strcpy(ver, ucvm_patch_list[id].version, len);
  return(UCVM_CODE_SUCCESS);
}


/* Label Patch */
int ucvm_patch_model_label(int id, char *lab, int len)
{
  if ((id < 0) || (id >= UCVM_MAX_MODELS) || 
      (ucvm_patch_list[id].valid == 0)) {
    fprintf(stderr, "Invalid model id %d\n", id);
    return(UCVM_CODE_ERROR);
  }

  ucvm_strcpy(lab, ucvm_patch_list[id].conf.label, len);
  return(UCVM_CODE_SUCCESS);
}


/* Setparam Patch */
int ucvm_patch_model_setparam(int id, int param, ...)
{
  va_list ap;

  if ((id < 0) || (id >= UCVM_MAX_MODELS) || 
      (ucvm_patch_list[id].valid == 0)) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  va_start(ap, param);
  switch (param) {
  default:
    break;
  }

  va_end(ap);

  return(UCVM_CODE_SUCCESS);
}


/* Qsort bucket comparitor */
int ucvm_patch_sort_comp(const void *v1, const void *v2)
{
  ucvm_bucket_t *b1;  
  ucvm_bucket_t *b2;

  b1 = (ucvm_bucket_t *)v1;
  b2 = (ucvm_bucket_t *)v2;

  if (b1->dist < b2->dist) {
    return(-1);
  } else if (b1->dist == b2->dist) {
    return(0);
  } else {
    return(1);
  }
}


/* Get smoothed material properties at index */
int ucvm_patch_getvals(int id, ucvm_point_t *xy, ucvm_prop_t *prop)
{
  ucvm_patch_t *mptr;
  double i0, j0, k0;
  int i, j, n, offset;
  int a, b;
  int fastdim, fastdimval;
  ucvm_bucket_t ib[2];
  double denom, weight;

  mptr = &(ucvm_patch_list[id]);

  /* Check if point falls outside of model region */
  if ((xy->coord[0] < 0.0) || 
      (xy->coord[1] < 0.0) || 
      (xy->coord[2] < 0.0) || 
      (xy->coord[0] >= mptr->sizes.coord[0]) || 
      (xy->coord[1] >= mptr->sizes.coord[1]) ||
      (xy->coord[2] >= mptr->sizes.coord[2])) {
    return(UCVM_CODE_ERROR);
  }

  i0 = xy->coord[0]/mptr->spacing;
  j0 = xy->coord[1]/mptr->spacing;
  k0 = xy->coord[2]/mptr->spacing;

  /* Copy ring of points at this depth into bucket list */
  n = 0;
  for (j = 0; j < 2; j++) {
    for (i = 0; i < 2; i++) {

      for (b = 0; b < mptr->surfs[j][i].dims.dim[1]; b++) {
	for (a = 0; a < mptr->surfs[j][i].dims.dim[0]; a++) {
	  if (i == 0) {
	    fastdim = 0;
	    fastdimval = a;
	  } else {
	    fastdim = 1;
	    fastdimval = b;
	  }

	  /* Compute distance^2 between query point and edge point */
	  switch (j*2+i) {
	  case 0:
	    mptr->bucketlist[n].dist = ((i0-a)*(i0-a) +
					(j0-0)*(j0-0));
	    break;
	  case 1:
	    mptr->bucketlist[n].dist = ((i0-0)*(i0-0) +
					(j0-b)*(j0-b));
	    break;
	  case 2:
	    mptr->bucketlist[n].dist = ((i0-a)*(i0-a) +
					(j0-mptr->dims.dim[1])*
					(j0-mptr->dims.dim[1]));
	    break;
	  case 3:
	    mptr->bucketlist[n].dist = ((i0-mptr->dims.dim[0])*
					(i0-mptr->dims.dim[0]) +
					(j0-b)*(j0-b));
	    break;
	  }

	  /* Interpolate between cells along z-axis */
	  offset = (int)(k0) * mptr->surfs[j][i].dims.dim[fastdim] 
	    + fastdimval;
	  ib[0].vp = mptr->surfs[j][i].props[offset].vp;
	  ib[0].vs = mptr->surfs[j][i].props[offset].vs;
	  ib[0].rho = mptr->surfs[j][i].props[offset].rho;
	  if ((int)(k0) + 1 < mptr->surfs[j][i].dims.dim[2]) {
	    offset = ((int)(k0) + 1) * mptr->surfs[j][i].dims.dim[fastdim] 
	      + fastdimval;
	  }
	  ib[1].vp = mptr->surfs[j][i].props[offset].vp;
	  ib[1].vs = mptr->surfs[j][i].props[offset].vs;
	  ib[1].rho = mptr->surfs[j][i].props[offset].rho;
	  mptr->bucketlist[n].vp = interpolate_linear(ib[0].vp, ib[1].vp,
						      k0-(int)k0);
	  mptr->bucketlist[n].vs = interpolate_linear(ib[0].vs, ib[1].vs,
						      k0-(int)k0);
	  mptr->bucketlist[n].rho = interpolate_linear(ib[0].rho, ib[1].rho,
						       k0-(int)k0);
	  n++;
	}
      }
    }
  }

  if (n != mptr->ucvm_num_buckets) {
    fprintf(stderr, "Failed to fill bucket list (%d, %d)\n", 
	    n, mptr->ucvm_num_buckets);
    return(UCVM_CODE_ERROR);
  }

  /* Sort by distance^2 */
  qsort(&(mptr->bucketlist[0]), mptr->ucvm_num_buckets, 
	sizeof(ucvm_bucket_t), ucvm_patch_sort_comp);

  /* Compute inverse distance weighting */
  if (mptr->bucketlist[0].dist == 0.0) {
    prop->vp = mptr->bucketlist[0].vp;
    prop->vs = mptr->bucketlist[0].vs;
    prop->rho = mptr->bucketlist[0].rho;
  } else {
    denom = 0.0;
    prop->vp = 0.0;    
    prop->vs = 0.0;
    prop->rho = 0.0;
    for (n = 0; n < mptr->ucvm_num_buckets/10; n++) {
      denom += 1.0/pow(sqrt(mptr->bucketlist[n].dist), 
		       UCVM_PATCH_POWER_FACTOR);
    }
    for (n = 0; n < mptr->ucvm_num_buckets/10; n++) {
      weight = 1.0/pow(sqrt(mptr->bucketlist[n].dist), 
		       UCVM_PATCH_POWER_FACTOR);
      prop->vp += mptr->bucketlist[n].vp * weight / denom;
      prop->vs += mptr->bucketlist[n].vs * weight / denom;
      prop->rho += mptr->bucketlist[n].rho * weight / denom;
    }
  }

  //if ((i0 < 25) || (j0 < 25) ||
  //    (i0 >= mptr->dims.dim[0] - 25) ||
  //    (j0 >= mptr->dims.dim[1] - 25)) {
  //   prop->vs = 8500.0;
  //  prop->vs = 5000.0;
   // prop->rho = 3000.0;
  //}

  return(UCVM_CODE_SUCCESS);
}


/* Query Patch */
int ucvm_patch_model_query(int id, ucvm_ctype_t cmode,
			   int n, ucvm_point_t *pnt, ucvm_data_t *data)
{
  int i;
  ucvm_point_t xy;
  double depth;
  int datagap = 0;

  if ((id < 0) || (id >= UCVM_MAX_MODELS) || 
      (ucvm_patch_list[id].valid == 0)) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  /* Check query mode */
  switch (cmode) {
  case UCVM_COORD_GEO_DEPTH:
  case UCVM_COORD_GEO_ELEV:
    break;
  default:
    fprintf(stderr, "Unsupported coord type\n");
    return(UCVM_CODE_ERROR);
    break;
  }

  for (i = 0; i < n; i++) {
    if ((data[i].crust.source == UCVM_SOURCE_NONE) && 
	((data[i].domain == UCVM_DOMAIN_INTERP) || 
	 (data[i].domain == UCVM_DOMAIN_CRUST)) &&
	(region_contains_null(&(ucvm_patch_list[id].conf.region), 
			      cmode, &(pnt[i])))) {

      /* Modify pre-computed depth to account for GTL interp range */
      depth = data[i].depth + data[i].shift_cr;

      /* Patch extends from free surface on down */
      if (depth >= 0.0) {

	/* Convert point to address */
	if (ucvm_proj_ucvm_geo2xy(&(ucvm_patch_list[id].proj), 
				  &(pnt[i]),
				  &xy) == UCVM_CODE_SUCCESS) {

	  switch (cmode) {
	  case UCVM_COORD_GEO_DEPTH:
	    break;
	  case UCVM_COORD_GEO_ELEV:
	    xy.coord[2] = (data[i].surf - xy.coord[2]);
	    break;
	  }

	  /* Query patch */
	  if (ucvm_patch_getvals(id, &xy, 
				 &(data[i].crust)) == UCVM_CODE_SUCCESS) {
	    data[i].crust.source = id;
	  } else {
	    datagap = 1;
	  }
	} else {
	  datagap = 1;
	}
      } else {
	datagap = 1;
      }
    } else {
      if (data[i].crust.source == UCVM_SOURCE_NONE) {
        datagap = 1;
      }
    }
  }

  if (datagap) {
    return(UCVM_CODE_DATAGAP);
  }

  return(UCVM_CODE_SUCCESS);
}


/* Fill model structure with Patch */
int ucvm_patch_get_model(ucvm_model_t *m)
{
  m->mtype = UCVM_MODEL_CRUSTAL;
  m->init = ucvm_patch_model_init;
  m->finalize = ucvm_patch_model_finalize;
  m->setparam = ucvm_patch_model_setparam;
  m->getversion = ucvm_patch_model_version;
  m->getlabel = ucvm_patch_model_label;
  m->query = ucvm_patch_model_query;

  return(UCVM_CODE_SUCCESS);
}

