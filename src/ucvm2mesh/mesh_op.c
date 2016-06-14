/**
 * mesh_op.c - Perform operation on two input
 *             meshes and save result as new mesh.
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

/* Usage information */
void usage(char *arg)
{
  printf("Usage: %s op inmesh1 inmesh2 format outmesh\n\n",arg);
  printf("where:\n");
  printf("\top: operation to perform: subtract\n");
  printf("\tinmesh1: path to the first input mesh file\n");
  printf("\tinmesh2: path to the second input mesh file\n");
  printf("\tformat: mesh format: IJK-12, IJK-20, IJK-32\n");
  printf("\toutmesh: path to the output meshfile\n\n");

  printf("Version: %s\n\n", VERSION);
}


/* Perform math operation on two input meshes */
int doOperation(void *nodes1, void *nodes2, size_t num, 
		mesh_format_t format, const char *op, void *nodeso)
{
  int i;
  mesh_ijk12_t *n12_1, *n12_2, *n12_o;
  mesh_ijk20_t *n20_1, *n20_2, *n20_o;
  mesh_ijk32_t *n32_1, *n32_2, *n32_o;

  if (strcmp(op, "diff") != 0) {
    fprintf(stderr, "Unsupported mesh operation %s\n", op);
    return(1);
  }

  switch (format) {
  case MESH_FORMAT_IJK12:
    n12_1 = nodes1;
    n12_2 = nodes2;
    n12_o = nodeso;
    for (i = 0; i < num; i++) {
      n12_o[i].vp = n12_1[i].vp - n12_2[i].vp;
      n12_o[i].vs = n12_1[i].vs - n12_2[i].vs;
      n12_o[i].rho = n12_1[i].rho - n12_2[i].rho;
    }
    break;
  case MESH_FORMAT_IJK20:
    n20_1 = nodes1;
    n20_2 = nodes2;
    n20_o = nodeso;
    for (i = 0; i < num; i++) {
      n20_o[i].vp = n20_1[i].vp - n20_2[i].vp;
      n20_o[i].vs = n20_1[i].vs - n20_2[i].vs;
      n20_o[i].rho = n20_1[i].rho - n20_2[i].rho;
      n20_o[i].qp = n20_1[i].qp - n20_2[i].qp;
      n20_o[i].qs = n20_1[i].qs - n20_2[i].qs;
    }
    break;
  case MESH_FORMAT_IJK32:
    n32_1 = nodes1;
    n32_2 = nodes2;
    n32_o = nodeso;
    for (i = 0; i < num; i++) {
      n32_o[i].i = n32_1[i].i;
      n32_o[i].j = n32_1[i].j;
      n32_o[i].k = n32_1[i].k;
      n32_o[i].vp = n32_1[i].vp - n32_2[i].vp;
      n32_o[i].vs = n32_1[i].vs - n32_2[i].vs;
      n32_o[i].rho = n32_1[i].rho - n32_2[i].rho;
      n32_o[i].qp = n32_1[i].qp - n32_2[i].qp;
      n32_o[i].qs = n32_1[i].qs - n32_2[i].qs;
    }
    break;
  default:
    fprintf(stderr, "Unsupported mesh format\n");
    return(1);
  }

  return(0);
}


int main(int argc, char **argv)
{
  int i;
  char op[256], input1[256], input2[256];
  char formatstr[256], output[256];
  mesh_format_t format;

  mesh_ijk12_t node12_1[MAX_NODES];
  mesh_ijk12_t node12_2[MAX_NODES];
  mesh_ijk12_t node12_o[MAX_NODES];
  mesh_ijk20_t node20_1[MAX_NODES];
  mesh_ijk20_t node20_2[MAX_NODES];
  mesh_ijk20_t node20_o[MAX_NODES];
  mesh_ijk32_t node32_1[MAX_NODES];
  mesh_ijk32_t node32_2[MAX_NODES];
  mesh_ijk32_t node32_o[MAX_NODES];

  void *nodes1, *nodes2, *nodeso;
  size_t recsize;

  FILE *os;
  FILE *is1, *is2;
  size_t num_read1, num_read2;
  size_t file_read1, file_read2;

  /* Parse args */
  if(argc != 6) {
    usage(argv[0]);
    exit(1);
  }

  format = MESH_FORMAT_UNKNOWN;
  strcpy(op, argv[1]);
  strcpy(input1, argv[2]);
  strcpy(input2, argv[3]);
  strcpy(formatstr, argv[4]);
  strcpy(output, argv[5]);

  for (i = 0; i < MAX_MESH_FORMATS; i++) {
    if (strcmp(formatstr, MESH_FORMAT_NAMES[i]) == 0) {
      format = i;
      break;
    }
  }

  switch (format) {
  case MESH_FORMAT_IJK12:
    nodes1 = &(node12_1[0]);
    nodes2 = &(node12_2[0]);
    nodeso = &(node12_o[0]);
    recsize = 12;
    break;
  case MESH_FORMAT_IJK20:
    nodes1 = &(node20_1[0]);
    nodes2 = &(node20_2[0]);
    nodeso = &(node20_o[0]);
    recsize = 20;
    break;
  case MESH_FORMAT_IJK32:
    nodes1 = &(node32_1[0]);
    nodes2 = &(node32_2[0]);
    nodeso = &(node32_o[0]);
    recsize = 32;
    break;
  default:
    fprintf(stderr, "Unsupported mesh format %s\n", formatstr);
    exit(1);
  }

  printf("Opening input file 1: %s\n", input1);
  is1 = fopen(input1, "rb");
  if (is1 == NULL) {
    perror("fopen");
    exit(1);
  }

  printf("Opening input file 2: %s\n", input2);
  is2 = fopen(input2, "rb");
  if (is2 == NULL) {
    perror("fopen");
    exit(1);
  }
  
  printf("Opening output file: %s\n", output);
  os = fopen(output, "wb");
  if (os == NULL) {
    perror("fopen");
    exit(1);
  }

  file_read1 = 0;
  file_read2 = 0;
  while ((!feof(is1)) && (!feof(is2))) {

    /* Read next chunk */
    num_read1 = fread(nodes1, recsize, MAX_NODES, is1);
    num_read2 = fread(nodes2, recsize, MAX_NODES, is2);
    if (num_read1 != num_read2) {
      fprintf(stderr, "Mesh files differ in size\n");
      exit(1);
    }
    if ((num_read1 > 0) && (num_read2 > 0)) {
      /* Perform operation */
      if (doOperation(nodes1, nodes2, num_read1, format,
		      op, nodeso) != 0) {
	fprintf(stderr, "Operation %s failed\n", op);
	exit(1);
      }
      
      /* Write to output mesh */
      if (fwrite(nodeso, recsize, num_read1, os) != num_read1) {
	fprintf(stderr, "Error writing output values\n");
	perror("fwrite");
	exit(1);
      }
    }
    file_read1 += num_read1; 
    file_read2 += num_read2; 
  }
  
  fclose(is1);
  fclose(is2);
  fclose(os);
  printf("Processed %zd mesh points total\n", file_read1);

  return 0;
}
