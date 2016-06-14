#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <math.h>
#include <errno.h>
#include "grd_config.h"
#include "grd.h"

/* Maximum number of grds to allow */
#define GRD_MAX_GRDS 100


/* DATA header list */
int num_datas = 0;
grd_info_t datalist[GRD_MAX_GRDS];

/* BKG header list */
int num_bkgs = 0;
grd_info_t bkglist[GRD_MAX_GRDS];

/* Query heuristic */
grd_heur_t grd_heur = GRD_HEUR_NONE;


/* Determine system endianness */
int systemEndian()
{
  int num = 1;
  if(*(char *)&num == 1) {
    return GRD_BYTEORDER_LSB;
  } else {
    return GRD_BYTEORDER_MSB;
  }
}


/* Check if file exists */
int fileExists(const char *file)
{
  struct stat st;

  if (stat(file, &st) == 0) {
    return(1);
  } else {
    return(0);
  }
}


/* Read the header file */
int grd_read_hdr(const char *hfile, grd_info_t *hdr)
{
  grd_config_t *grd_cfg = NULL;
  grd_config_t *cfgentry = NULL;

  memset(hdr, 0, sizeof(grd_info_t));

  /* Read in the header */
  grd_cfg = grd_parse_config(hfile);
  if (grd_cfg == NULL) {
    fprintf(stderr, "Failed to read config file\n");
    return(1);
  }

  cfgentry = grd_find_name(grd_cfg, "ncols");
  if (cfgentry == NULL) {
    fprintf(stderr, "%s: ncols not found\n", hfile);
    return(1);
  }
  hdr->dims[0] = atoi(cfgentry->value);

  cfgentry = grd_find_name(grd_cfg, "nrows");
  if (cfgentry == NULL) {
    fprintf(stderr, "%s: nrows not found\n", hfile);
    return(1);
  }
  hdr->dims[1] = atoi(cfgentry->value);

  cfgentry = grd_find_name(grd_cfg, "xllcorner");
  if (cfgentry == NULL) {
    fprintf(stderr, "%s: xllcorner not found\n", hfile);
    return(1);
  }
  hdr->llcorner[0] = atof(cfgentry->value);

  cfgentry = grd_find_name(grd_cfg, "yllcorner");
  if (cfgentry == NULL) {
    fprintf(stderr, "%s: yllcorner not found\n", hfile);
    return(1);
  }
  hdr->llcorner[1] = atof(cfgentry->value);

  cfgentry = grd_find_name(grd_cfg, "cellsize");
  if (cfgentry == NULL) {
    fprintf(stderr, "%s: cellsize not found\n", hfile);
    return(1);
  }
  hdr->spacing = atof(cfgentry->value);

  cfgentry = grd_find_name(grd_cfg, "nodata_value");
  if (cfgentry == NULL) {
    fprintf(stderr, "%s: NODATA_value not found\n", hfile);
    return(1);
  }
  hdr->no_data = atof(cfgentry->value);

  cfgentry = grd_find_name(grd_cfg, "byteorder");
  if (cfgentry == NULL) {
    fprintf(stderr, "%s: byteorder not found\n", hfile);
    return(1);
  }
  if (strcmp(cfgentry->value, "LSBFIRST") == 0) {
    hdr->byteorder = GRD_BYTEORDER_LSB;
  } else if (strcmp(cfgentry->value, "MSBFIRST") == 0) {
    hdr->byteorder = GRD_BYTEORDER_MSB;
  } else {
    fprintf(stderr, "Invalid byte order '%s'\n", cfgentry->value);
    return(1);
  }

  /* Handle endian-ness here */
  if (hdr->byteorder != systemEndian()) {
    fprintf(stderr, "Unsupported byte order\n");
    return(1);
  }

  grd_free_config(grd_cfg);

  /* Compute and save upper corner */
  hdr->urcorner[0] = hdr->llcorner[0] + hdr->dims[0]*hdr->spacing;
  hdr->urcorner[1] = hdr->llcorner[1] + hdr->dims[1]*hdr->spacing;

  /* Save name of DATA GridFloat formatted file */
  strcpy(hdr->file, hfile);
  strcpy(strstr(hdr->file, "hdr"), "flt");
  if (!fileExists(hdr->file)) {
    fprintf(stderr, "GridFloat file %s does not exist\n", hdr->file);
    return(1);
  }

  /* Open data file */
  hdr->fp = fopen(hdr->file, "rb");
  if (hdr->fp == NULL) {
    perror("grd_find_hdr");
    fprintf(stderr, "Failed to open file %s\n", hdr->file);
    return(1);
  }

  return(0);
}


/* Initializer */
int grd_init(const char *datadir, const char *bkgdir, grd_heur_t heur)
{
  DIR *dir;
  struct dirent *ent;
  char hfile[512];

  grd_heur = heur;

  /* Get list of DATA files from directory */
  dir = opendir (datadir);
  if (dir == NULL) {
    fprintf(stderr, "Failed to open datadir %s\n", datadir);
    return(1);
  }

  fprintf(stderr, "Reading DATA GRDs:\n");
  while ((ent = readdir(dir)) != NULL) {
    if (strstr(ent->d_name, ".hdr") != NULL) {
      sprintf(hfile, "%s/%s", datadir, ent->d_name);
      if (grd_read_hdr(hfile, &(datalist[num_datas])) != 0) {
	fprintf(stderr, "Failed to read in header %s\n", hfile);
	return(1);
      }

      fprintf(stderr, "DATA %s: %dx%d, %lf, (%lf, %lf)->(%lf, %lf), %lf %d %s\n",
	     hfile, 
	     datalist[num_datas].dims[0], datalist[num_datas].dims[1], 
	     datalist[num_datas].llcorner[0], datalist[num_datas].llcorner[1], 
	     datalist[num_datas].urcorner[0], datalist[num_datas].urcorner[1], 
	     datalist[num_datas].spacing, datalist[num_datas].no_data,
	     datalist[num_datas].byteorder, datalist[num_datas].file);

      num_datas++;
    }

  }   
  closedir(dir);

  if (num_datas == 0) {
    fprintf(stderr, "No DATAs found in %s\n", datadir);
    return(0);
  }

  if ((bkgdir != NULL) && (strcmp(bkgdir, "") != 0)) {
    /* Get list of Bkg files from directory */
    dir = opendir (bkgdir);
    if (dir == NULL) {
      fprintf(stderr, "Failed to open bkgdir %s\n", bkgdir);
      return(1);
    }
    
    fprintf(stderr, "Reading Background:\n");
    while ((ent = readdir(dir)) != NULL) {
      if (strstr(ent->d_name, ".hdr") != NULL) {
	sprintf(hfile, "%s/%s", bkgdir, ent->d_name);
	if (grd_read_hdr(hfile, &(bkglist[num_bkgs])) != 0) {
	  fprintf(stderr, "Failed to read in header %s\n", hfile);
	  return(1);
	}
	fprintf(stderr, 
		"BKG %s: %dx%d, (%lf, %lf)->(%lf, %lf), %lf, %lf %d %s\n",
		hfile, 
		bkglist[num_bkgs].dims[0], bkglist[num_bkgs].dims[1], 
		bkglist[num_bkgs].llcorner[0], bkglist[num_bkgs].llcorner[1], 
		bkglist[num_bkgs].urcorner[0], bkglist[num_bkgs].urcorner[1], 
	     bkglist[num_bkgs].spacing, bkglist[num_bkgs].no_data,
		bkglist[num_bkgs].byteorder, bkglist[num_bkgs].file);
      num_bkgs++;
      }
    }   
    closedir(dir);

    if (num_bkgs == 0) {
      fprintf(stderr, "Warning: No BKGs found in %s\n", bkgdir);
      return(0);
    }
  }

  return(0);
}


/* Finalizer */
int grd_finalize()
{
  int i;

  for (i = 0; i < num_datas; i++) {
    fclose(datalist[i].fp);
  }

  for (i = 0; i < num_bkgs; i++) {
    fclose(bkglist[i].fp);
  }

  num_datas = 0;
  num_bkgs = 0;
  grd_heur = GRD_HEUR_NONE;

  return(0);
}


/* Find header that contains point of interest */
int grd_find_hdr(grd_point_t *p, int num_hdr, grd_info_t * hlist, 
		 grd_info_t **hdr, int *x, int *y)
{
  int i;
  grd_info_t *thdr = NULL;
  grd_info_t *oldhdr = NULL;

  *x = -1;
  *y = -1;
  oldhdr = *hdr;
  *hdr = NULL;

  /* Check if point falls in current header */
  if (oldhdr != NULL) {
    if ((p->coord[0] >= oldhdr->llcorner[0]) && 
	(p->coord[1] >= oldhdr->llcorner[1]) && 
	(p->coord[0] < oldhdr->urcorner[0]) && 
	(p->coord[1] < oldhdr->urcorner[1])) {
      *hdr = oldhdr;
    }
  }

  if (*hdr == NULL) {
    /* Find header that contains this point */
    for (i = 0; i < num_hdr; i++) {
      thdr = &(hlist[i]);
      if ((p->coord[0] >= thdr->llcorner[0]) && 
	  (p->coord[1] >= thdr->llcorner[1]) && 
	  (p->coord[0] < thdr->urcorner[0]) && 
	  (p->coord[1] < thdr->urcorner[1])) {
	*hdr = thdr;
	break;
      }
    }
  }

  /* Search with slightly looser bounds to account for floating
     point precision error */
  if (*hdr == NULL) {
    for (i = 0; i < num_hdr; i++) {
      thdr = &(hlist[i]);
      if ((p->coord[0] >= thdr->llcorner[0]) && 
	  (p->coord[1] >= thdr->llcorner[1]) && 
	  (p->coord[0] < thdr->urcorner[0] + 0.0001) && 
	  (p->coord[1] < thdr->urcorner[1] + 0.0001)) {
	*hdr = thdr;
	break;
      }
    }
  }

  if (*hdr != NULL) {
    *x = (int)((p->coord[0] - (*hdr)->llcorner[0]) / (*hdr)->spacing);
    *y = (int)((p->coord[1] - (*hdr)->llcorner[1]) / (*hdr)->spacing);
    /* Nudge back into bounds if on the edge */
    if (*x == (*hdr)->dims[0]) {
      *x = *x - 1;
    }
    if (*y == (*hdr)->dims[1]) {
      *y = *y - 1;
    }
  }

  return(0);
}


/* Get data value at offset x,y from current file */
int grd_get_data(grd_info_t *hdr, int x, int y, float *data)
{
  size_t offset;

  *data = 0.0;

  if (hdr == NULL) {
    fprintf(stderr, "Invalid hdr pointer\n");
    return(1);
  }

  if (hdr->fp == NULL) {
    fprintf(stderr, "Invalid file pointer\n");
    return(1);
  }

  /* Find data val at this point */
  offset = ((size_t)(hdr->dims[1]-y-1) * 
	    (size_t)(hdr->dims[0]) + 
	    (size_t)(x));

  /* Fallback to seeking within file */
  if (fseek(hdr->fp, offset * sizeof(float), SEEK_SET) != 0) {
    fprintf(stderr, "Failed to seek to %zu within GRD %s\n", 
	    offset * sizeof(float), hdr->file);
    return(1);
  }
  if (fread(data, sizeof(float), 1, hdr->fp) != 1) {
    fprintf(stderr, "Failed to read from GRD %s\n", hdr->file);
	return(1);
  }

  //if (hdr->cache == NULL) {
  //  hdr->cache = malloc(hdr->dims[0] * hdr->dims[1] * sizeof(float));
  //  if (hdr->cache == NULL) {
  //
  //    return(0);
  //  }

  //  retval = fread(hdr->cache, sizeof(float), 
	//	   hdr->dims[0] * hdr->dims[1], hdr->fp);
  //  if (retval != (hdr->dims[0] * hdr->dims[1])) {
  //    perror("grd_get_val");
  //    fprintf(stderr, "Failed to read %s into cache, read %d, expected %d\n", 
  //	      hdr->file, retval, hdr->dims[0] * hdr->dims[1]);
  //    free(hdr->cache);
  //    hdr->cache = NULL;
  //    return(1);
  //  }
  //
  //}

  //*data = hdr->cache[offset];

  //printf("data is %f\n", *data);
  return(0);
}


/* Query underlying models */
int grd_query(int n, grd_point_t *pnt, grd_data_t *data)
{
  int i;
  int data_x, data_y, bkg_x, bkg_y;
  float data_val, bkg_val;
  int is_zero;

  grd_info_t *data_hdr = NULL;
  grd_info_t *bkg_hdr = NULL;

  for (i = 0; i < n; i++) {

    /* Find DATA/BKG files containing point */
    strcpy(data[i].source, "none");
    data[i].data = 0.0;
    data[i].valid = 0;
    if (grd_find_hdr(&(pnt[i]), num_datas, datalist, 
		     &data_hdr, &data_x, &data_y) != 0) {
      fprintf(stderr, "Failed to find DATA header\n");
      return(1);
    }
    if (grd_find_hdr(&(pnt[i]), num_bkgs, bkglist, 
		     &bkg_hdr, &bkg_x, &bkg_y) != 0) {
      fprintf(stderr, "Failed to find BKG header\n");
      return(1);
    }

    /* Save DATA data if valid */
    if (data_hdr != NULL) {
      grd_get_data(data_hdr, data_x, data_y, &data_val);
      if (fabs(data_val - data_hdr->no_data) > 0.01) {
	strcpy(data[i].source, "data");
	data[i].data = (double)data_val;
	data[i].valid = 1;
      }
    }

    if ((data[i].valid == 1) && (fabs(data[i].data - 0.0) < 0.001)) {
      is_zero = 1;
    } else {
      is_zero = 0;
    }

    if ((bkg_hdr != NULL) && ((data[i].valid == 0) || 
			      ((grd_heur != GRD_HEUR_NONE) && (is_zero)))) {
      /* Fall back to secondary dataset */
      grd_get_data(bkg_hdr, bkg_x, bkg_y, &bkg_val);
      if (fabs(bkg_val - bkg_hdr->no_data) > 0.01) {
	if (((grd_heur == GRD_HEUR_DEM) && (is_zero) && (bkg_val < 0.0)) || 
	    ((grd_heur == GRD_HEUR_VS30) && (is_zero)) || 
	    (data[i].valid == 0)) {
	  strcpy(data[i].source, "bkg");
	  data[i].data = (double)bkg_val;
	  data[i].valid = 1;
	}
      }
    }
  }

  return(0);
}
