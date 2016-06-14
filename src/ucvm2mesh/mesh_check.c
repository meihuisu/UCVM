/**
 * mesh_strip_ijk.c - Strip fields from existing mesh
 *
 * Based on code written by Ricardo Taborda 
 * <ricardotaborda@gmail.com> and Tiankai Tu
 * <tutk@cs.cmu.edu>.
 *
 * Modified by Gideon Juve <juve@usc.edu>
 * Modified by Patrick Small <patrices@usc.edu>
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
  printf("Usage: %s input format\n\n",arg);
  printf("input: path to the input file\n");
  printf("format: format of file, IJK-12, IJK-20, IJK-32\n\n");

  printf("Version: %s\n\n", VERSION);
}


int nodeOK(char *n, int rec_size, size_t offset)
{
  mesh_ijk12_t node;

  switch (rec_size) {
  case 12:
  case 20:
    memcpy(&node, n, 12);
    break;
  case 32:
    memcpy(&node, (n+12), 12);
    break;
  default:
    fprintf(stderr, "Invalid rec size\n");
    return(1);
  }

  if (isnan(node.vp) || isnan(node.vs) || isnan(node.rho)) {
    fprintf(stderr, "NAN mat prop detected at %zd\n", offset);
    return(1);
  }
  if (isinf(node.vp) || isinf(node.vs) || isinf(node.rho)) {
    fprintf(stderr, "Inf mat prop detected at %zd\n", offset);
    return(1);
  }

  if (node.vp <= 0.0) {
    fprintf(stderr, "Neg Vp at %zd\n", offset);
    return(1);
  }
  if (node.vs <= 0.0) {
    fprintf(stderr, "Neg Vs at %zd\n", offset);
    return(1);
  }
  if (node.rho <= 0.0) {
    fprintf(stderr, "Neg Rho at %zd\n", offset);
    return(1);
  }

  return(0);
}


int dumpNode(char *n, int rec_size)
{
  mesh_ijk12_t node;
  mesh_ijk20_t node_med;

  switch (rec_size) {
  case 12:
    memcpy(&node, n, 12);
    break;
  case 20:
    memcpy(&node, n, 12);
    memcpy(&node_med, n, 20);
    break;
  case 32:
    memcpy(&node, (n+12), 12);
    memcpy(&node_med, (n+12), 20);
    break;
  default:
    fprintf(stderr, "Invalid rec size\n");
    return(1);
  }

  fprintf(stderr, "Node:\n");
  fprintf(stderr, "\tVp, Vs, Rho: %f, %f, %f\n", node.vp, node.vs, node.rho);
  if (rec_size > 12) {
    fprintf(stderr, "\tQp, Qs: %f, %f\n", node_med.qp, node_med.qs);
  }

  return(0);
}


int main(int argc, char **argv)
{
  int a;
  char input[256];
  char format[256];

  char *node_buf;
  int rec_size;
  FILE *is;
  size_t num_read;
  size_t file_read;

  // Parse args
  if(argc != 3) {
    usage(argv[0]);
    exit(1);
  }

  strcpy(input, argv[1]);
  strcpy(format, argv[2]);

  for (a = 0; a < MAX_MESH_FORMATS; a++) {
    if (strcmp(format, MESH_FORMAT_NAMES[a]) == 0) {
      rec_size = MESH_FORMAT_LENS[a];
      break;
    }
  }
  if (a == MAX_MESH_FORMATS) {
    fprintf(stderr, "Invalid format specified\n");
    return(1);
  }

  node_buf = (char *)malloc(MAX_NODES * rec_size);

  printf("Record size is %d bytes\n", rec_size);
  printf("Opening input file %s\n", input);
  is = fopen(input, "rb");
  if (is == NULL) {
    perror("fopen");
    exit(1);
  }
  
  file_read = 0;
  while (!feof(is)) {
    num_read = fread(node_buf, rec_size, MAX_NODES, is);
    if (num_read > 0) {
      for (a = 0; a < num_read; a++) {
	if (nodeOK((node_buf + (a * rec_size)), rec_size, file_read + (size_t)a) != 0) {
	  dumpNode((node_buf + (a * rec_size)), rec_size);
	  exit(1);
	}
      }
      
    }
    file_read += num_read; 
  }
  
  free(node_buf);
  fclose(is);
  printf("Checked %zd vals total\n", file_read);

  return 0;
}
