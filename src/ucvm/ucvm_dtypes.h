#ifndef UCVM_DTYPES_H
#define UCVM_DTYPES_H

#include <stdarg.h>


/* Maximum number of supported models */
#define UCVM_MAX_MODELS 20


/* Maximum number of model flags */
#define UCVM_MAX_FLAGS 10


/* Maximum model label length */
#define UCVM_MAX_LABEL_LEN 64


/* Maximum model version length */
#define UCVM_MAX_VERSION_LEN 64


/* Maximum path length */
#define UCVM_MAX_PATH_LEN 256


/* Maximum projection description length */
#define UCVM_MAX_PROJ_LEN 256


/* Maximum model flag key length */
#define UCVM_MAX_FLAG_LEN 64


/* Maximum model list length */
#define UCVM_MAX_MODELLIST_LEN 1024


/* Default depth (m) for GTL/Crustal transition */
#define UCVM_DEFAULT_INTERP_ZMIN 0.0
#define UCVM_DEFAULT_INTERP_ZMAX 0.0


/* Special source model/ifunc flags */
#define UCVM_SOURCE_NONE -1
#define UCVM_SOURCE_CRUST -2
#define UCVM_SOURCE_GTL -3


/* Predefined crustal model interfaces */
#define UCVM_MODEL_NONE "none"
#define UCVM_MODEL_CVMH "cvmh"
#define UCVM_MODEL_CVMS "cvms"
#define UCVM_MODEL_CENCAL "cencal"
#define UCVM_MODEL_1D "1d"
#define UCVM_MODEL_BBP1D "bbp1d"
#define UCVM_MODEL_CMRG "cmrg"
#define UCVM_MODEL_CMUETREE "cmuetree"
#define UCVM_MODEL_CVMSI "cvmsi"
#define UCVM_MODEL_CVMNCI "cvmnci"
#define UCVM_MODEL_WFCVM "wfcvm"
#define UCVM_MODEL_CVMLT "cvmlt"
#define UCVM_MODEL_TAPE "tape"
#define UCVM_MODEL_JAPAN "japan"


/* Predefined GTL model interfaces */
#define UCVM_MODEL_1DGTL "1dgtl"
#define UCVM_MODEL_ELYGTL "elygtl"


/* These model interfaces allow user defined 
   models to be declared in conf at runtime */
#define UCVM_MODEL_PATCH "model_patch"
#define UCVM_MODEL_ETREE "model_etree"


/* Predefined map interfaces */
#define UCVM_MAP_UCVM "ucvm"
#define UCVM_MAP_YONG "yong"


/* These map interfaces allow user defined 
   maps to be declared in conf at runtime */
#define UCVM_MAP_ETREE "map_etree"


/* Predefined interpolation functions */
#define UCVM_IFUNC_NONE "none"
#define UCVM_IFUNC_CRUST "crust"
#define UCVM_IFUNC_GTL "gtl"
#define UCVM_IFUNC_LINEAR "linear"
#define UCVM_IFUNC_ELY "ely"


/* Predefined projection strings */
#define UCVM_PROJ_GEO "+proj=latlong +datum=WGS84"


/* Byte order */
typedef enum { UCVM_BYTEORDER_LSB = 0, 
               UCVM_BYTEORDER_MSB } ucvm_byteorder_t;


/* Supported error codes */
typedef enum { UCVM_CODE_SUCCESS = 0, 
	       UCVM_CODE_ERROR,
	       UCVM_CODE_DATAGAP,
	       UCVM_CODE_NODATA } ucvm_code_t;


/* Supported coordinate query modes */
typedef enum { UCVM_COORD_GEO_DEPTH = 0, 
	       UCVM_COORD_GEO_ELEV } ucvm_ctype_t;


/* Supported model types */
typedef enum { UCVM_MODEL_CRUSTAL = 0, 
	       UCVM_MODEL_GTL } ucvm_mtype_t;


/* Supported resource types */
typedef enum { UCVM_RESOURCE_MODEL = 0, 
	       UCVM_RESOURCE_IFUNC,
	       UCVM_RESOURCE_MAP,
	       UCVM_RESOURCE_MODEL_IF,
	       UCVM_RESOURCE_MAP_IF} ucvm_rtype_t;


/* Supported UCVM parameters visible to user. Argument list:

UCVM_PARAM_QUERY_MODE    : ucvm_ctype_t qmode
UCVM_PARAM_IFUNC_ZRANGE  : double min, double max (meters)
UCVM_PARAM_MODEL_CONF    : char *mlabel, char *param, char *value
  CVM-H:
    "USE_1D_BKG": "True"/"False"

*/
typedef enum { UCVM_PARAM_QUERY_MODE = 0,
	       UCVM_PARAM_IFUNC_ZRANGE,
	       UCVM_PARAM_MODEL_CONF } ucvm_param_t;


/* Supported model parameters. Used internally by UCVM 

UCVM_MODEL_PARAM_FORCE_DEPTH_ABOVE_SURF : Force elevation points to be
  treated as offset relative to regional model surface when it is 
  above the ucvm dem surface. Removes discontinuities between model 
  surface and GTL.

*/
typedef enum { UCVM_MODEL_PARAM_FORCE_DEPTH_ABOVE_SURF } ucvm_mparam_t;


/* Supported coordinate query modes. Used internally by UCVM */
typedef enum { UCVM_OPMODE_CRUSTAL = 0, 
	       UCVM_OPMODE_GTL } ucvm_opmode_t;


/* Supported domains for query points. Used internally by UCVM */
typedef enum { UCVM_DOMAIN_NONE = 0,
	       UCVM_DOMAIN_GTL, 
	       UCVM_DOMAIN_INTERP,
	       UCVM_DOMAIN_CRUST} ucvm_domain_t;


/* For swapping endian */
typedef union ucvm_fdata_t {
    float f;
    unsigned char b[4];
} ucvm_fdata_t;


/* 3D point */
typedef struct ucvm_point_t 
{
  double coord[3];
} ucvm_point_t;


/* 3D dims */
typedef struct ucvm_dim_t 
{
  unsigned int dim[3];
} ucvm_dim_t;


/* Material properties */
typedef struct ucvm_prop_t 
{
  int source;
  double vp;
  double vs;
  double rho;
} ucvm_prop_t;


/* Model data */
typedef struct ucvm_data_t 
{
  double surf;
  double vs30;
  double depth;
  ucvm_domain_t domain;
  double shift_cr;
  double shift_gtl;
  ucvm_prop_t crust;
  ucvm_prop_t gtl;
  ucvm_prop_t cmb;
} ucvm_data_t;


/* Region box */
typedef struct ucvm_region_t 
{
  ucvm_ctype_t cmode;
  double p1[3];
  double p2[3];
} ucvm_region_t;


/* Model config */
typedef struct ucvm_modelconf_t 
{
  char label[UCVM_MAX_LABEL_LEN];
  ucvm_region_t region;
  char config[UCVM_MAX_PATH_LEN];
  char extconfig[UCVM_MAX_PATH_LEN];
} ucvm_modelconf_t;


/* Model interface specification */
typedef struct ucvm_model_t 
{
  ucvm_mtype_t mtype;
  int (*init)(int id, ucvm_modelconf_t *conf);
  int (*finalize)();
  int (*getversion)(int id, char *ver, int len);
  int (*getlabel)(int id, char *lab, int len);
  int (*setparam)(int id, int param, ...);
  int (*query)(int id, ucvm_ctype_t cmode,
	       int n, ucvm_point_t *pnt, 
	       ucvm_data_t *data);
} ucvm_model_t;


/* Interp function */
typedef struct ucvm_ifunc_t 
{
  char label[UCVM_MAX_LABEL_LEN];
  int (*interp)(double zmin, double zmax, ucvm_ctype_t cmode,
		ucvm_point_t *pnt, ucvm_data_t *data);
} ucvm_ifunc_t;


/* Resource flag description */
typedef struct ucvm_flag_t 
{
  char key[UCVM_MAX_FLAG_LEN];
  char value[UCVM_MAX_FLAG_LEN];
} ucvm_flag_t;


/* Resource description */
typedef struct ucvm_resource_t 
{
  ucvm_rtype_t rtype;
  ucvm_mtype_t mtype; /* Applicable to models only */
  int active; /* Applicable to models/maps only */
  char label[UCVM_MAX_LABEL_LEN];
  char version[UCVM_MAX_VERSION_LEN];
  char config[UCVM_MAX_PATH_LEN];
  char extconfig[UCVM_MAX_PATH_LEN];
  int numflags;
  ucvm_flag_t flags[UCVM_MAX_FLAGS]; /* Applicable to models only */
} ucvm_resource_t;

#endif
