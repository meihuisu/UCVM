#ifndef UM_MESH_H
#define UM_MESH_H

#include "ucvm.h"

#define MAX_MESH_FORMATS 5

typedef enum mesh_format_t { MESH_FORMAT_UNKNOWN = 0,
			     MESH_FORMAT_IJK12,
			     MESH_FORMAT_IJK20,
			     MESH_FORMAT_IJK32,
			     MESH_FORMAT_SORD} mesh_format_t;


extern const int MESH_FORMAT_LENS[MAX_MESH_FORMATS];
extern const char* MESH_FORMAT_NAMES[MAX_MESH_FORMATS];


typedef struct mesh_ijk32_t {
  int i;
  int j;
  int k;
  float vp;
  float vs;
  float rho;
  float qp;
  float qs;
} mesh_ijk32_t;

typedef struct mesh_ijk20_t {
  float vp;
  float vs;
  float rho;
  float qp;
  float qs;
} mesh_ijk20_t;

typedef struct mesh_ijk12_t {
  float vp;
  float vs;
  float rho;
} mesh_ijk12_t;

typedef struct mesh_sord_t {
  float val;
} mesh_sord_t;


/* Check if node is valid */
int mesh_node_valid(int i, int j, int k, mesh_ijk32_t *node);

/* Convert UCVM data list to mesh node list and check that it is valid */
int mesh_data_to_node(int myid, int i_start, int i_end,
		      int j_start, int j_end, int k,
		      ucvm_point_t *pntbuf, ucvm_data_t *propbuf, 
		      mesh_ijk32_t *node_buf, double vp_min, double vs_min);


/* Serial mesh writer */
int mesh_open_serial(ucvm_dim_t *mesh_dims, char *output, 
		     mesh_format_t mtype, int bufsize);
int mesh_write_serial(mesh_ijk32_t *nodes, int node_count);
int mesh_close_serial();

#endif
