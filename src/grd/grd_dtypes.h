#ifndef GRD_DTYPES_H
#define GRD_DTYPES_H


/* Byte order */
typedef enum { GRD_BYTEORDER_LSB = 0, 
	       GRD_BYTEORDER_MSB } grd_byteorder_t;


/* Data source */
typedef enum { GRD_SRC_DATA = 0, 
	       GRD_SRC_BKG } grd_src_t;


/* Heuristics */
typedef enum { GRD_HEUR_NONE = 0, 
	       GRD_HEUR_DEM,
	       GRD_HEUR_VS30} grd_heur_t;


/* Geo lon/lat point */
typedef struct grd_point_t 
{
  double coord[2];
} grd_point_t;


/* Data value */
typedef struct grd_data_t 
{
  char source[16];
  double data;
  int valid;
} grd_data_t;


/* Grid file info */
typedef struct grd_info_t 
{
  int dims[2];
  double llcorner[2];
  double urcorner[2];
  double spacing;
  double no_data;
  grd_byteorder_t  byteorder;
  char file[512];
  FILE *fp;
  float *cache;
} grd_info_t;


#endif
