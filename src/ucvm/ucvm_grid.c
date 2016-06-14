#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ucvm_dtypes.h"
#include "ucvm_grid.h"
#include "ucvm_proj_bilinear.h"
#include "ucvm_utils.h"
#include "proj_api.h"


/* Maximum grid points to buffer from files */
#define UCVM_GRID_MAX_POINTS 2048

/* CMU projection parameters */
double cmu_xi[4] = { -121.0, -118.951292, -113.943965, -116.032285 };
double cmu_yi[4] = {   34.5,   36.621696,   33.122341,   31.082920 };
double cmu_dims[2] = {600000.0, 300000.0};


/* Private: Generate grid from projection and dimensions */
int ucvm_grid_gen_private(ucvm_projdef_t *iproj, ucvm_trans_t *trans,
			  ucvm_projdef_t *oproj,
			  ucvm_dim_t *dims, double spacing, 
			  ucvm_point_t *pnts, const char *filename)
{
  int i, j, c;
  ucvm_bilinear_t cmu;
  projPJ opj = NULL;
  projPJ ipj = NULL;
  double x, y, z, x_offset, y_offset, theta, gridding;
  FILE *ofp;
  ucvm_point_t pbuf[UCVM_GRID_MAX_POINTS];
  int convert_deg_in = 0;
  int convert_deg_out = 0;

  if ((pnts == NULL) && (filename == NULL)) {
    return(UCVM_CODE_ERROR);
  }

  /* Init CMU proj */
  for (i = 0; i < 4; i++) {
    cmu.xi[i] = cmu_xi[i];
    cmu.yi[i] = cmu_yi[i];
  }
  cmu.dims[0] = cmu_dims[0];
  cmu.dims[1] = cmu_dims[1];

  if (strstr(iproj->proj, "latlong") == NULL) {
    fprintf(stderr, "Only latlong input projection supported\n");
    return(UCVM_CODE_ERROR);
  } else {
    convert_deg_in = 1;
  }

  if (strstr(oproj->proj, "latlong") != NULL) {
    convert_deg_out = 1;
  }

  theta = trans->rotate * DEG_TO_RAD;
  if (trans->gtype == UCVM_GRID_CELL_CENTER) {
    gridding = 0.5;
  } else {
    gridding = 0.0;
  }

  /* Determine origin point within projection */
  if (strcmp(oproj->proj, "cmu") == 0) {
    x_offset = trans->origin[0] + trans->translate[0];
    y_offset = trans->origin[1] + trans->translate[1];
  } else {

    ipj = pj_init_plus(iproj->proj);
    if (ipj == NULL) {
      fprintf(stderr, "Failed to create input projection\n");
      return(UCVM_CODE_ERROR);
    }
    
    opj = pj_init_plus(oproj->proj);
    if (opj == NULL) {
      fprintf(stderr, "Failed to create output projection\n");
      return(UCVM_CODE_ERROR);
    }

    /* Convert origin point to output projection */
    if (convert_deg_in) {
      x_offset = trans->origin[0] * DEG_TO_RAD;
      y_offset = trans->origin[1] * DEG_TO_RAD;
    } else {
      x_offset = trans->origin[0];
      y_offset = trans->origin[1];
    }
    if (pj_transform(ipj, opj, 1, 1, &x_offset, &y_offset, NULL) != 0) {
      fprintf(stderr, "Failed to project origin point\n");
      return(UCVM_CODE_ERROR);
    }

    if (convert_deg_out) {
      x_offset = (x_offset * RAD_TO_DEG) + trans->translate[0];
      y_offset = (y_offset * RAD_TO_DEG) + trans->translate[1];
    } else {
      x_offset = x_offset + trans->translate[0];
      y_offset = y_offset + trans->translate[1];
    }

    //printf("Converted UTM: %lf %lf\n", x_offset, y_offset);

    pj_free(ipj);
    pj_free(opj);
  }

  if (pnts != NULL) {
    /* Generate grid with rotation */
    for (j = 0; j < dims->dim[1]; j++) {
      for (i = 0; i < dims->dim[0]; i++) {
	x = ((i + gridding) * spacing);
	y = ((j + gridding) * spacing);
	z = 0.0;

        pnts[j*dims->dim[0]+i].coord[0] = x_offset + 
          x * cos(theta) - y * sin(theta);
        pnts[j*dims->dim[0]+i].coord[1] = y_offset + 
          x * sin(theta) + y * cos(theta);
        pnts[j*dims->dim[0]+i].coord[2] = z;
      }
    }
  }

  if (filename != NULL) {
    /* Open file */
    ofp = fopen(filename, "wb");
    if (ofp == NULL) {
      fprintf(stderr, "Failed to open gridfile %s\n", filename);
      return(UCVM_CODE_ERROR);
    }

    /* Generate grid with rotation */
    c = 0;
    for (j = 0; j < dims->dim[1]; j++) {
      for (i = 0; i < dims->dim[0]; i++) {
	x = ((i + gridding) * spacing);
	y = ((j + gridding) * spacing);
	z = 0.0;

	pbuf[c].coord[0] = x_offset + x * cos(theta) - y * sin(theta);
	pbuf[c].coord[1] = y_offset + x * sin(theta) + y * cos(theta);
	pbuf[c].coord[2] = z;
	c++;

	if (c == UCVM_GRID_MAX_POINTS) {
	  fwrite(pbuf, sizeof(ucvm_point_t), c, ofp);
	  c = 0;
	}
      }
    }
    if (c > 0) {
      fwrite(pbuf, sizeof(ucvm_point_t), c, ofp);
    }

    /* Close file */
    fclose(ofp);
  }

  return(UCVM_CODE_SUCCESS);
}


/* Private: Convert point list from one projection to another */
int ucvm_grid_convert_private(ucvm_projdef_t *iproj, 
			      ucvm_projdef_t *oproj, 
			      size_t n, ucvm_point_t *pnts, 
			      const char *filename)
{
  int i;
  ucvm_point_t xy;
  ucvm_bilinear_t cmu;
  projPJ ipj = NULL;
  projPJ opj = NULL;
  FILE *fp;
  ucvm_point_t pbuf[UCVM_GRID_MAX_POINTS];
  char ipdesc[UCVM_MAX_PROJ_LEN];
  char opdesc[UCVM_MAX_PROJ_LEN];
  int convert_deg_in = 0;
  int convert_deg_out = 0;
  size_t num_read;
  size_t num_buffered;

  /* Init CMU proj */
  for (i = 0; i < 4; i++) {
    cmu.xi[i] = cmu_xi[i];
    cmu.yi[i] = cmu_yi[i];
  }
  cmu.dims[0] = cmu_dims[0];
  cmu.dims[1] = cmu_dims[1];

  /* If input or output projections are CMU, change them to latlong
     and then perform an initial cmu->latlong and/or additional 
     latlong->cmu conversion */

  /* Create appropriate input projection */
  if (strcmp(iproj->proj, "cmu") == 0) {
    /* Convert from CMU to GEO */

    if (pnts != NULL) {
      for (i = 0; i < n; i++) {
	ucvm_bilinear_xy2geo(&cmu, &(pnts[i]), &xy);
	pnts[i].coord[0] = xy.coord[0];
	pnts[i].coord[1] = xy.coord[1];
      }
    }

    if (filename != NULL) {
      /* Open file */
      fp = fopen(filename, "rb+");
      if (fp == NULL) {
	fprintf(stderr, "Failed to open gridfile %s\n", filename);
	return(UCVM_CODE_ERROR);
      }

      num_read = 0;
      while (!feof(fp)) {
	num_buffered = fread(&pbuf, sizeof(ucvm_point_t), 
			     UCVM_GRID_MAX_POINTS, fp);

	if (num_buffered > 0) {
	  for (i = 0; i < num_buffered; i++) {
	    /* Translate point */
	    ucvm_bilinear_xy2geo(&cmu, &(pbuf[i]), &xy);
	    pbuf[i].coord[0] = xy.coord[0];
	    pbuf[i].coord[1] = xy.coord[1];
	  }

	  /* Write points */
	  fseek(fp, num_read * sizeof(ucvm_point_t), SEEK_SET);
	  fwrite(&pbuf, sizeof(ucvm_point_t), num_buffered, fp);
	  fflush(fp);
	  num_read = num_read + num_buffered;
	}
      }
      if (n != num_read) {
	fprintf(stderr, "Gridfile generated size (%zu) and n mismatch (%zu)\n",
		num_read, n);
	  fclose(fp);
	  return(UCVM_CODE_ERROR);
      }

      fclose(fp); 
    }
    ucvm_strcpy(ipdesc, UCVM_PROJ_GEO, UCVM_MAX_PROJ_LEN);
    ipj = pj_init_plus(ipdesc);
  } else {
    ucvm_strcpy(ipdesc, iproj->proj, UCVM_MAX_PROJ_LEN);
    ipj = pj_init_plus(iproj->proj);
  }
 
  if (ipj == NULL) {
    fprintf(stderr, "Failed to create input projection\n");
    return(UCVM_CODE_ERROR);
  }

  /* Create appropriate output projection */
  if (strcmp(oproj->proj, "cmu") == 0) {
    ucvm_strcpy(opdesc, UCVM_PROJ_GEO, UCVM_MAX_PROJ_LEN);
    opj = pj_init_plus(opdesc);
  } else {
    ucvm_strcpy(opdesc, oproj->proj, UCVM_MAX_PROJ_LEN);
    opj = pj_init_plus(oproj->proj);
  }

  if (oproj == NULL) {
    fprintf(stderr, "Failed to create output projection\n");
    return(UCVM_CODE_ERROR);
  }
  
  //printf("iproj: %s\n", iproj->proj);
  //printf("oproj: %s\n", oproj->proj);
  //printf("ipdesc: %s\n", ipdesc);
  //printf("opdesc: %s\n", opdesc);

  if (strstr(ipdesc, "latlong") != NULL) {
    convert_deg_in = 1;
  }

  if (strstr(opdesc, "latlong") != NULL) {
    convert_deg_out = 1;
  }

  /* Convert point list */
  if (pnts != NULL) {
    for (i = 0; i < n; i++) {
      //printf("in : %lf, %lf\n", pnts[i].coord[0], pnts[i].coord[1]);

      if (convert_deg_in) {
	pnts[i].coord[0] = pnts[i].coord[0] * DEG_TO_RAD;
	pnts[i].coord[1] = pnts[i].coord[1] * DEG_TO_RAD;
      }

      /* Translate point */
      if (pj_transform(ipj, opj, 1, 1, 
		       &(pnts[i].coord[0]), 
		       &(pnts[i].coord[1]), NULL) != 0) {
	fprintf(stderr, "Failed to project point\n");
	return(UCVM_CODE_ERROR);
      }

      if (convert_deg_out) {
	pnts[i].coord[0] = pnts[i].coord[0] * RAD_TO_DEG;
	pnts[i].coord[1] = pnts[i].coord[1] * RAD_TO_DEG;
      }

      if (strcmp(oproj->proj, "cmu") == 0) {
	ucvm_bilinear_geo2xy(&cmu, &(pnts[i]), &xy);
	pnts[i].coord[0] = xy.coord[0];
	pnts[i].coord[1] = xy.coord[1];
      }

      //printf("out: %lf, %lf\n", pnts[i].coord[0], pnts[i].coord[1]);
    }
  }

  if (filename != NULL) {
    /* Open file */
    fp = fopen(filename, "r+");
    if (fp == NULL) {
      fprintf(stderr, "Failed to open gridfile %s\n", filename);
      return(UCVM_CODE_ERROR);
    }

    num_read = 0;
    while (!feof(fp)) {
      num_buffered = fread(&pbuf, sizeof(ucvm_point_t), 
			   UCVM_GRID_MAX_POINTS, fp);
      
      //printf("num_buffered=%d\n", num_buffered);
      if (num_buffered > 0) {
	for (i = 0; i < num_buffered; i++) {
	  if (convert_deg_in) {
	    pbuf[i].coord[0] = pbuf[i].coord[0] * DEG_TO_RAD;
	    pbuf[i].coord[1] = pbuf[i].coord[1] * DEG_TO_RAD;
	  }

	  /* Translate point */
	  if (pj_transform(ipj, opj, 1, 1, 
			   &(pbuf[i].coord[0]), 
			   &(pbuf[i].coord[1]), NULL) != 0) {
	    fprintf(stderr, "Failed to project point from gridfile\n");
	    fclose(fp);
	    return(UCVM_CODE_ERROR);
	  }
      
	  if (convert_deg_out) {
	    pbuf[i].coord[0] = pbuf[i].coord[0] * RAD_TO_DEG;
	    pbuf[i].coord[1] = pbuf[i].coord[1] * RAD_TO_DEG;
	  }

	  if (strcmp(oproj->proj, "cmu") == 0) {
	    ucvm_bilinear_geo2xy(&cmu, &(pbuf[i]), &xy);
	    pbuf[i].coord[0] = xy.coord[0];
	    pbuf[i].coord[1] = xy.coord[1];
	  }
	}

	/* Write points */
	fseek(fp, num_read * sizeof(ucvm_point_t), SEEK_SET);
	fwrite(&pbuf, sizeof(ucvm_point_t), num_buffered, fp);
	fflush(fp);
	num_read = num_read + num_buffered;
      }
    }

    if (n != num_read) {
      fprintf(stderr, "Gridfile converted size (%zu) and n mismatch (%zu)\n",
	      num_read, n);
      fclose(fp);
      return(UCVM_CODE_ERROR);
    }

    fclose(fp);
  }

  pj_free(ipj);
  pj_free(opj);

  return(UCVM_CODE_SUCCESS);
}


/* Generate grid from projection and dimensions */
int ucvm_grid_gen(ucvm_projdef_t *iproj, ucvm_trans_t *trans,
		  ucvm_projdef_t *oproj,
                  ucvm_dim_t *dims, double spacing, 
                  ucvm_point_t *pnts)
{
  return(ucvm_grid_gen_private(iproj, trans, oproj,
			       dims, spacing, pnts, NULL));
}


/* Generate grid from projection and dimensions */
int ucvm_grid_gen_file(ucvm_projdef_t *iproj, ucvm_trans_t *trans,
		       ucvm_projdef_t *oproj,
                       ucvm_dim_t *dims, double spacing, 
                       const char *filename)
{
  return(ucvm_grid_gen_private(iproj, trans, oproj, 
			       dims, spacing, NULL, filename));
}


/* Convert point list from one projection to another */
int ucvm_grid_convert(ucvm_projdef_t *iproj, 
		      ucvm_projdef_t *oproj, 
		      size_t n, ucvm_point_t *pnts)
{
  return(ucvm_grid_convert_private(iproj, oproj, n, pnts, NULL));
}


/* Convert point list from one projection to another */
int ucvm_grid_convert_file(ucvm_projdef_t *iproj, 
			   ucvm_projdef_t *oproj, 
			   size_t n, const char *filename)
{
  return(ucvm_grid_convert_private(iproj, oproj, n, NULL, filename));
}
