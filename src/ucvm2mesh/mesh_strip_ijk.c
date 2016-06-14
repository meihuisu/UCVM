/**
 * mesh_strip_ijk.c - Strip fields from existing
 *                    AWP-20, AWP-32 mesh
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>
#include "um_mesh.h"


#define MAX_NODES 10000


void usage(char *arg)
{
  printf("Usage: %s inmesh format outmesh\n\n",arg);
  printf("where:\n");
  printf("\tinmesh: path to the input mesh file\n");
  printf("\tformat: mesh format, IJK-20, IJK-32\n");
  printf("\toutmesh: path to the output meshfile\n\n");

  printf("Version: %s\n\n", VERSION);
}


int nodeOK(mesh_ijk12_t *node, size_t offset)
{
  if (isnan(node->vp) || isnan(node->vs) || isnan(node->rho)) {
    fprintf(stderr, "NAN mat prop detected at %zd\n", offset);
    return(1);
  }
  if (isinf(node->vp) || isinf(node->vs) || isinf(node->rho)) {
    fprintf(stderr, "Inf mat prop detected at %zd\n", offset);
    return(1);
  }

  if (node->vp <= 0.0) {
    fprintf(stderr, "Neg Vp at %zd\n", offset);
    return(1);
  }
  if (node->vs <= 0.0) {
    fprintf(stderr, "Neg Vs at %zd\n", offset);
    return(1);
  }
  if (node->rho <= 0.0) {
    fprintf(stderr, "Neg Rho at %zd\n", offset);
    return(1);
  }

  return(0);
}



int main(int argc, char **argv)
{
  int i, a;
  char input[256], formatstr[256], output[256];
  mesh_format_t format;

  mesh_ijk20_t node20[MAX_NODES];
  mesh_ijk32_t node32[MAX_NODES];
  mesh_ijk12_t node12[MAX_NODES];
  void *nodes;
  size_t recsize;

  FILE *os;
  FILE *is;
  size_t num_read;
  size_t file_read;

  /* Parse args */
  if(argc != 4) {
    usage(argv[0]);
    exit(1);
  }

  format = MESH_FORMAT_UNKNOWN;
  strcpy(input, argv[1]);
  strcpy(formatstr, argv[2]);
  strcpy(output, argv[3]);


  for (i = 0; i < MAX_MESH_FORMATS; i++) {
    if (strcmp(formatstr, MESH_FORMAT_NAMES[i]) == 0) {
      format = i;
      break;
    }
  }

  switch (format) {
  case MESH_FORMAT_IJK20:
    nodes = &(node20[0]);
    recsize = 20;
    break;
  case MESH_FORMAT_IJK32:
    nodes = &(node32[0]);
    recsize = 32;
    break;
  default:
    fprintf(stderr, "Unsupported mesh format %s\n", formatstr);
    exit(1);
  }

  printf("Opening input file %s\n", input);
  is = fopen(input, "rb");
  if (is == NULL) {
    perror("fopen");
    exit(1);
  }
  
  printf("Opening output file %s\n", output);
  os = fopen(output, "wb");
  if (os == NULL) {
    perror("fopen");
    exit(1);
  }

  file_read = 0;
  while (!feof(is)) {

    /* Read next chunk */
    num_read = fread(nodes, recsize, MAX_NODES, is);
    if (num_read > 0) {
      for (a = 0; a < num_read; a++) {
	switch (format) {
	case MESH_FORMAT_IJK20:
	  memcpy(&node12[a], &(node20[a].vp), sizeof(mesh_ijk12_t));
	  break;
	case MESH_FORMAT_IJK32:
	  memcpy(&node12[a], &(node32[a].vp), sizeof(mesh_ijk12_t));
	  break;
	default:
	  fprintf(stderr, "Unsupported mesh format %s\n", formatstr);
	  exit(1);
	}

	/* Check node */
	if (nodeOK(&(node12[a]), file_read + (size_t)a) != 0) {
	  fprintf(stderr, "Node:\n");
	  fprintf(stderr, "\tVp, Vs, Rho: %f, %f, %f\n", node12[a].vp,
		  node12[a].vs, node12[a].rho);
	  exit(1);
	}
      }
      
      /* Write to output mesh */
      if (fwrite(&node12[0], sizeof(mesh_ijk12_t), 
		 num_read, os) != num_read) {
	fprintf(stderr, "Error writing output values\n");
	perror("fwrite");
	exit(1);
      }
    }
    file_read += num_read; 
  }
  
  fclose(is);
  fclose(os);
  printf("Stripped %zd vals total\n", file_read);

  return 0;
}
