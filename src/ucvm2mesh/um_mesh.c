#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "um_mesh.h"

const int MESH_FORMAT_LENS[MAX_MESH_FORMATS] = {0, 12, 20, 32, 4};

const char* MESH_FORMAT_NAMES[MAX_MESH_FORMATS] = {"UNKNOWN", 
						   "IJK-12", 
						   "IJK-20", 
						   "IJK-32", 
						   "SORD"};

/* Mesh file type */
int writer_init_flag_serial = 0;
mesh_format_t meshtype_serial = MESH_FORMAT_UNKNOWN;
size_t meshrecsize_serial = 0;
FILE *meshfp1 = NULL;
FILE *meshfp2 = NULL;
FILE *meshfp3 = NULL;

/* Node Buffers */
void *node_buf_serial1 = NULL;
void *node_buf_serial2 = NULL;
void *node_buf_serial3 = NULL;
int node_buf_size_serial = 0;


/* Check if node is valid */
int mesh_node_valid(int i, int j, int k, mesh_ijk32_t *node)
{
  if (isnan(node->vp) || isnan(node->vs) || isnan(node->rho)) {
    fprintf(stderr, "NAN mat prop detected at %d, %d, %d\n", i, j, k);
    return(1);
  }
  if (isinf(node->vp) || isinf(node->vs) || isinf(node->rho)) {
    fprintf(stderr, "Inf mat prop detected at %d, %d, %d\n", i, j, k);
    return(1);
  }

  if (node->vp <= 0.0) {
    fprintf(stderr, "Neg Vp at %d, %d, %d\n", i, j, k);
    return(1);
  }
  if (node->vs <= 0.0) {
    fprintf(stderr, "Neg Vs at %d, %d, %d\n", i, j, k);
    return(1);
  }
  if (node->rho <= 0.0) {
    fprintf(stderr, "Neg Rho at %d, %d, %d\n", i, j, k);
    return(1);
  }

  return(0);
}


/* Convert UCVM data list to mesh node list and check that it is valid */
int mesh_data_to_node(int myid, int i_start, int i_end,
		      int j_start, int j_end, int k,
		      ucvm_point_t *pntbuf, ucvm_data_t *propbuf, 
		      mesh_ijk32_t *node_buf, double vp_min, double vs_min)
{
  int i, j, n;

  n = 0;
  for (j = j_start; j < j_end; j++) {  
    for (i = i_start; i < i_end; i++) {
      /* Copy payload */
      node_buf[n].i = i + 1;
      node_buf[n].j = j + 1;
      node_buf[n].k = k + 1;
      node_buf[n].vp = propbuf[n].cmb.vp;
      node_buf[n].vs = propbuf[n].cmb.vs;
      node_buf[n].rho = propbuf[n].cmb.rho;
      
      /* Apply min Vs */
      if (node_buf[n].vs < vs_min) {
	node_buf[n].vs = vs_min;
	node_buf[n].vp = vp_min;
      }
	
      /* Qp/Qs via Kim Olsen */
      node_buf[n].qs = 50.0 * (node_buf[n].vs / 1000.0);
      node_buf[n].qp = 2.0 * node_buf[n].qs;

      /* Check the node */
      if (mesh_node_valid(i+1, j+1, k+1, &(node_buf[n])) != 0) {
	fprintf(stderr, "[%d] Node:\n", myid);
	fprintf(stderr, "\t[%d] i,j,k: %d, %d, %d\n", myid, 
		i + 1, j + 1, k + 1);
	fprintf(stderr, "\t[%d] lon,lat,dep: %lf, %lf, %lf\n", myid, 
		pntbuf[n].coord[0], 
		pntbuf[n].coord[1],
		pntbuf[n].coord[2]);
	fprintf(stderr, 
		"\t[%d] Crust, GTL, Cmb, Vp, Vs, Rho: %d, %d, %d, %f, %f, %f\n", 
		myid,
		propbuf[n].crust.source,
		propbuf[n].gtl.source,
		propbuf[n].cmb.source,
		node_buf[n].vp, 
		node_buf[n].vs, 
		node_buf[n].rho);
	return(1);
      }
      n++;
    }
  }

  return(0);
}


int mesh_open_serial(ucvm_dim_t *mesh_dims, char *output, 
		     mesh_format_t mtype, int bufsize)
{
  char output_vp[UCVM_MAX_PATH_LEN], output_vs[UCVM_MAX_PATH_LEN], 
    output_rho[UCVM_MAX_PATH_LEN];

  /* Determine record size */
  meshtype_serial = mtype;
  node_buf_size_serial = bufsize;
  switch (meshtype_serial) {
  case MESH_FORMAT_IJK12:
      meshrecsize_serial = sizeof(mesh_ijk12_t);
      break;
  case MESH_FORMAT_IJK20:
      meshrecsize_serial = sizeof(mesh_ijk20_t);
      break;
  case MESH_FORMAT_IJK32:
      meshrecsize_serial = sizeof(mesh_ijk32_t);
      break;
  case MESH_FORMAT_SORD:
      meshrecsize_serial = sizeof(mesh_sord_t);
      break;
  default:
    fprintf(stderr, "Unrecognized mesh type\n");
    return(1);
    break;
  }

  sprintf(output_vp, "%s_vp", output);
  sprintf(output_vs, "%s_vs", output);
  sprintf(output_rho, "%s_rho", output);

  switch(meshtype_serial) {
  case MESH_FORMAT_IJK12:
  case MESH_FORMAT_IJK20:
  case MESH_FORMAT_IJK32:
    /* Open file */
    meshfp1 = fopen(output, "wb");
    if (meshfp1 == NULL) {
      fprintf(stderr, "Failed to open output file\n");
      return(1);
    }
    node_buf_serial1 = malloc(meshrecsize_serial * node_buf_size_serial);
    if (node_buf_serial1 == NULL) {
      fprintf(stderr, "Failed to allocate node_buf_serial1\n");
      return(1);
    }
    break;
  case MESH_FORMAT_SORD:
    /* Open files */
    meshfp1 = fopen(output_vp, "wb");
    meshfp2 = fopen(output_vs, "wb");
    meshfp3 = fopen(output_rho, "wb");
    if ((meshfp1 == NULL) || (meshfp2 == NULL) || (meshfp3 == NULL)) {
      fprintf(stderr, "Failed to open output file\n");
      return(1);
    }

    node_buf_serial1 = malloc(meshrecsize_serial * node_buf_size_serial);
    node_buf_serial2 = malloc(meshrecsize_serial * node_buf_size_serial);
    node_buf_serial3 = malloc(meshrecsize_serial * node_buf_size_serial);
    if ((node_buf_serial1 == NULL) || (node_buf_serial2 == NULL) || 
	(node_buf_serial3 == NULL)) {
      fprintf(stderr, "Failed to allocate node_bufs 1,2,3\n");
      return(1);
    }
    break;
  default:
    fprintf(stderr, "Unrecognized mesh type\n");
    return(1);
    break;
  }

  writer_init_flag_serial = 1;
  return(0);
}


int mesh_write_serial(mesh_ijk32_t *nodes, int node_count)
{
  mesh_ijk12_t * ptr1_ijk12;
  mesh_ijk20_t * ptr1_ijk20;  
  mesh_ijk32_t * ptr1_ijk32;
  mesh_sord_t * ptr1_sord;
  mesh_sord_t * ptr2_sord;
  mesh_sord_t * ptr3_sord;
  int i;

  if (!writer_init_flag_serial) {
    fprintf(stderr, "Mesh writer not initialized\n");
    return(1);
  }

  if (node_count > node_buf_size_serial) {
    fprintf(stderr, "Node_count exceeds bufsize from init function\n");
    return(1);
  }

  /* Transform node list and perform write */
  switch(meshtype_serial) {
  case MESH_FORMAT_IJK12:
    ptr1_ijk12 = (mesh_ijk12_t *)node_buf_serial1;
    for (i = 0; i < node_count; i++) {
      ptr1_ijk12[i].vp = nodes[i].vp;
      ptr1_ijk12[i].vs = nodes[i].vs;
      ptr1_ijk12[i].rho = nodes[i].rho;
    }
    if (fwrite(node_buf_serial1, meshrecsize_serial, node_count, 
	       meshfp1) != node_count) {
      fprintf(stderr, "Failed to write to mesh file\n");
      return(1);
    }
    break;
  case MESH_FORMAT_IJK20:
    ptr1_ijk20 = (mesh_ijk20_t *)node_buf_serial1;
    for (i = 0; i < node_count; i++) {
      ptr1_ijk20[i].vp = nodes[i].vp;
      ptr1_ijk20[i].vs = nodes[i].vs;
      ptr1_ijk20[i].rho = nodes[i].rho;
      ptr1_ijk20[i].qp = nodes[i].qp;
      ptr1_ijk20[i].qs = nodes[i].qs;
    }
    if (fwrite(node_buf_serial1, meshrecsize_serial, node_count,
	       meshfp1) != node_count) {
      fprintf(stderr, "Failed to write to mesh file\n");
      return(1);
    }
    break;
  case MESH_FORMAT_IJK32:
    ptr1_ijk32 = (mesh_ijk32_t *)node_buf_serial1;
    for (i = 0; i < node_count; i++) {
      ptr1_ijk32[i].i = nodes[i].i;
      ptr1_ijk32[i].j = nodes[i].j;
      ptr1_ijk32[i].k = nodes[i].k;
      ptr1_ijk32[i].vp = nodes[i].vp;
      ptr1_ijk32[i].vs = nodes[i].vs;
      ptr1_ijk32[i].rho = nodes[i].rho;
      ptr1_ijk32[i].qp = nodes[i].qp;
      ptr1_ijk32[i].qs = nodes[i].qs;
    }
    if (fwrite(node_buf_serial1, meshrecsize_serial, node_count,
	       meshfp1) != node_count) {
      fprintf(stderr, "Failed to write to mesh file\n");
      return(1);
    }
    break;
  case MESH_FORMAT_SORD:
    ptr1_sord = (mesh_sord_t *)node_buf_serial1;
    ptr2_sord = (mesh_sord_t *)node_buf_serial2;
    ptr3_sord = (mesh_sord_t *)node_buf_serial3;
    for (i = 0; i < node_count; i++) {
      ptr1_sord[i].val = nodes[i].vp;
      ptr2_sord[i].val = nodes[i].vs;
      ptr3_sord[i].val = nodes[i].rho;
    }
    if (fwrite(node_buf_serial1, meshrecsize_serial, node_count,
	       meshfp1) != node_count) {
      fprintf(stderr, "Failed to write to vp mesh file\n");
      return(1);
    }
    if (fwrite(node_buf_serial2, meshrecsize_serial, node_count,
	       meshfp2) != node_count) {
      fprintf(stderr, "Failed to write to vps mesh file\n");
      return(1);
    }
    if (fwrite(node_buf_serial3, meshrecsize_serial, node_count,
	       meshfp3) != node_count) {
      fprintf(stderr, "Failed to write to rho mesh file\n");
      return(1);
    }
    break;
  default:
    fprintf(stderr, "Unrecognized mesh type\n");
    return(1);
    break;
  }

  return(0);
}


int mesh_close_serial()
{
  switch(meshtype_serial) {
  case MESH_FORMAT_IJK12:
  case MESH_FORMAT_IJK20:
  case MESH_FORMAT_IJK32:
    free(node_buf_serial1);
    fclose(meshfp1);
    break;
  case MESH_FORMAT_SORD:
    free(node_buf_serial1);
    free(node_buf_serial2);
    free(node_buf_serial3);
    fclose(meshfp1);
    fclose(meshfp2);
    fclose(meshfp3);
    break;
  default:
    fprintf(stderr, "Unrecognized mesh type\n");
    return(1);
    break;
  }

  node_buf_size_serial = 0;
  node_buf_serial1 = NULL;
  node_buf_serial2 = NULL;
  node_buf_serial3 = NULL;
  meshtype_serial = MESH_FORMAT_UNKNOWN;
  writer_init_flag_serial = 0;

  return(0);
}
