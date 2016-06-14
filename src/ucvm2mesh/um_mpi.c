#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mpi.h>
#include "um_mpi.h"
#include "um_mesh.h"

/* MPI state */
int writer_id;
int writer_init_flag = 0;

/* MPI-IO vars */
MPI_Offset cur_offset = 0;
MPI_File fh1, fh2, fh3;
MPI_Datatype MPI_MESH_T;
int num_fields_mesh;
MPI_Datatype MPI_MESH_FILE_T;

/* Mesh file type */
mesh_format_t meshtype = MESH_FORMAT_UNKNOWN;
size_t meshrecsize = 0;

/* Node Buffers */
void *node_buf1 = NULL;
void *node_buf2 = NULL;
void *node_buf3 = NULL;
int node_buf_size = 0;


void mpi_exit(int val)
{
  MPI_Finalize();
  exit(val);
}


void mpi_barrier()
{
  MPI_Barrier(MPI_COMM_WORLD);
  return;
}


void mpi_init(int *ac,char ***av,int *np,int *id,char *pname,int *len)
{
  MPI_Init(ac,av);
  MPI_Comm_size(MPI_COMM_WORLD,np);
  MPI_Comm_rank(MPI_COMM_WORLD,id);

  MPI_Get_processor_name(pname,len);
}


void mpi_final(char *s)
{
  //fprintf(stderr,"%s\n",s);
  MPI_Finalize();
}


void mpi_register_mesh_ijk32(MPI_Datatype *MPI_MESH_8_T, int *num_fields)
{
  // Register new mesh data type for iiifffff
  *num_fields = 8;
  MPI_Datatype mesh_type[8] = { MPI_INT, MPI_INT, MPI_INT,
                                MPI_FLOAT, MPI_FLOAT,
                                MPI_FLOAT, MPI_FLOAT, MPI_FLOAT };
  int blocklen[8] = { 1, 1, 1, 1, 1, 1, 1, 1 };
  MPI_Aint disp[8] = { 0, 4, 8, 12, 16, 20, 24, 28 };
  MPI_Type_struct(*num_fields, blocklen, disp, mesh_type, MPI_MESH_8_T);
  MPI_Type_commit(MPI_MESH_8_T);
  return;
}

void mpi_register_mesh_ijk20(MPI_Datatype *MPI_MESH_5_T, int *num_fields)
{
  // Register new mesh data type fffff
  *num_fields = 5;
  MPI_Datatype mesh_type[5] = { MPI_FLOAT, MPI_FLOAT,
                                MPI_FLOAT, MPI_FLOAT, MPI_FLOAT };
  int blocklen[5] = { 1, 1, 1, 1, 1 };
  MPI_Aint disp[5] = { 0, 4, 8, 12, 16 };
  MPI_Type_struct(*num_fields, blocklen, disp, mesh_type, MPI_MESH_5_T);
  MPI_Type_commit(MPI_MESH_5_T);
  return;
}


void mpi_register_mesh_ijk12(MPI_Datatype *MPI_MESH_3_T, int *num_fields)
{
  // Register new mesh data type fff
  *num_fields = 3;
  MPI_Datatype mesh_type[3] = { MPI_FLOAT, MPI_FLOAT, MPI_FLOAT };
  int blocklen[3] = { 1, 1, 1 };
  MPI_Aint disp[3] = { 0, 4, 8 };
  MPI_Type_struct(*num_fields, blocklen, disp, mesh_type, MPI_MESH_3_T);
  MPI_Type_commit(MPI_MESH_3_T);
  return;
}


void mpi_register_mesh_sord(MPI_Datatype *MPI_MESH_1_T, int *num_fields)
{
  // Register new mesh data type f
  *num_fields = 1;
  MPI_Datatype mesh_type[1] = { MPI_FLOAT };
  int blocklen[1] = { 1 };
  MPI_Aint disp[1] = { 0 };
  MPI_Type_struct(*num_fields, blocklen, disp, mesh_type, MPI_MESH_1_T);
  MPI_Type_commit(MPI_MESH_1_T);
  return;
}


void mpi_register_stat_4(MPI_Datatype *MPI_STAT_4_T, int *num_fields)
{
  // Register new mesh data type for iiif
  *num_fields = 4;
  MPI_Datatype mesh_type[4] = { MPI_INT, MPI_INT, MPI_INT,
                                MPI_FLOAT };
  int blocklen[4] = { 1, 1, 1, 1 };
  MPI_Aint disp[4] = { 0, 4, 8, 12 };
  MPI_Type_struct(*num_fields, blocklen, disp, mesh_type, MPI_STAT_4_T);
  MPI_Type_commit(MPI_STAT_4_T);
  return;
}


int mpi_file_open(MPI_File *fh, char *filename, MPI_Offset offset)
{
  if (MPI_File_open(MPI_COMM_WORLD, filename,
		    MPI_MODE_CREATE | MPI_MODE_WRONLY, 
		    MPI_INFO_NULL, fh) != MPI_SUCCESS) {
    fprintf(stderr, "Error opening file %s\n", filename);
    return(1);
  }
  if (offset > 0) {
    if (MPI_File_seek(*fh, offset, MPI_SEEK_SET) != MPI_SUCCESS) {
      fprintf(stderr, "Error seeking in file %s\n", filename);
      return(1);
    }
  }
  return(0);
}


void mpi_file_close(MPI_File *fh)
{
  MPI_File_close(fh);
  return;
}


int mpi_file_write(MPI_File *fh, void *buf, int count, 
		   int num_fields, MPI_Datatype *dt)
{
  MPI_Status status;
  int num_wrote;

  if (MPI_File_write(*fh, buf, count, *dt, &status) != MPI_SUCCESS) {
    fprintf(stderr, "Error writing to file\n");
    return(1);
  }

  MPI_Get_count(&status, MPI_INT, &num_wrote);
  if (num_wrote != count * num_fields) {
    fprintf(stderr, "Error writing output, wrote %d of %d\n", 
	    num_wrote, count);
    return(1);
  }

  return(0);
}


int mpi_file_write_at(MPI_File *fh, MPI_Offset offset,
		      void *buf, int count, int num_fields, 
		      MPI_Datatype *dt)
{
  MPI_Status status;
  int num_wrote;

  /* Disable collective IO if directed */
#ifndef UCVM_ENABLE_MPI_COLL_IO
  if (MPI_File_write_at(*fh, offset, buf, count, *dt, 
  			&status) != MPI_SUCCESS) {
    fprintf(stderr, "Error writing to file\n");
    return(1);
  }
#else
  if (MPI_File_write_at_all(*fh, offset, buf, count, *dt, 
  			    &status) != MPI_SUCCESS) {
    fprintf(stderr, "Error writing to file\n");
    return(1);
  }
#endif
  
  MPI_Get_count(&status, MPI_INT, &num_wrote);
  if (num_wrote != count * num_fields) {
    fprintf(stderr, "Error writing output, wrote %d of %d\n", 
	    num_wrote, count);
    return(1);
  }

  return(0);
}


int mesh_open_mpi(int myid, int nproc, 
		     ucvm_dim_t *mesh_dims, ucvm_dim_t *proc_dims,
		     char *output, mesh_format_t mtype, int bufsize)
{
  int i, retval, errstrlen;
  int mdim[3], pdim[3], distribs[3], dargs[3], partdim[3];
  char output_vp[UCVM_MAX_PATH_LEN], output_vs[UCVM_MAX_PATH_LEN], 
    output_rho[UCVM_MAX_PATH_LEN];
  char errstr[256];
  size_t part_size;

  errstrlen = 256;
  writer_id = myid;

  if (writer_id == 0) {
#ifndef UCVM_ENABLE_MPI_COLL_IO
    printf("[%d] MPI/IO collective IO is disabled\n", writer_id);
#else
    printf("[%d] MPI/IO collective IO is enabled\n", writer_id);
#endif
  }

  /* MPI dimensions are row-major */
  mdim[0] = mesh_dims->dim[2];
  mdim[1] = mesh_dims->dim[1];
  mdim[2] = mesh_dims->dim[0];
  pdim[0] = proc_dims->dim[2];
  pdim[1] = proc_dims->dim[1];
  pdim[2] = proc_dims->dim[0];

  for (i = 0; i < 3; i++) {
    distribs[i] = MPI_DISTRIBUTE_BLOCK;
    dargs[i] = MPI_DISTRIBUTE_DFLT_DARG;
  }

  /* Compute partition dimensions */
  partdim[0] = mdim[0]/pdim[0];
  partdim[1] = mdim[1]/pdim[1];
  partdim[2] = mdim[2]/pdim[2];

  /* Determine record size */
  meshtype = mtype;
  node_buf_size = bufsize;
  switch (meshtype) {
  case MESH_FORMAT_IJK12:
      mpi_register_mesh_ijk12(&MPI_MESH_T, &num_fields_mesh);
      meshrecsize = sizeof(mesh_ijk12_t);
      break;
  case MESH_FORMAT_IJK20:
      mpi_register_mesh_ijk20(&MPI_MESH_T, &num_fields_mesh);
      meshrecsize = sizeof(mesh_ijk20_t);
      break;
  case MESH_FORMAT_IJK32:
      mpi_register_mesh_ijk32(&MPI_MESH_T, &num_fields_mesh);
      meshrecsize = sizeof(mesh_ijk32_t);
      break;
  case MESH_FORMAT_SORD:
      mpi_register_mesh_sord(&MPI_MESH_T, &num_fields_mesh);
      meshrecsize = sizeof(mesh_sord_t);
      break;
  default:
    fprintf(stderr, "[%d] Unrecognized mesh type\n", writer_id);
    return(1);
    break;
  }

  /* Check partition size */
  part_size = partdim[0] * partdim[1] * partdim[2] * (size_t)meshrecsize;
  if (part_size >= (size_t)(1 << 31)) {
    fprintf(stderr, "[%d] Partition size must be less than %zu bytes\n", 
	    writer_id, (size_t)(1 << 31));
    return(1);
  }

  /* Create file view data type */
  retval = MPI_Type_create_darray(nproc, myid, 3, mdim, 
				  distribs, dargs, pdim, MPI_ORDER_C,
				  MPI_MESH_T, &MPI_MESH_FILE_T);
  if (retval != MPI_SUCCESS) {
    MPI_Error_string(retval, errstr, &errstrlen);
    fprintf(stderr, "[%d] Failed to create file darray: %s\n", 
	    writer_id, errstr);
    return(1);
  }

  MPI_Type_commit(&MPI_MESH_FILE_T);

  MPI_Type_size(MPI_MESH_FILE_T, &retval);
  if (myid == 0) {
    fprintf(stdout, "[%d] Partition file type size: %d bytes\n", 
	    myid, retval);
  }

  cur_offset = 0;
  sprintf(output_vp, "%s_vp", output);
  sprintf(output_vs, "%s_vs", output);
  sprintf(output_rho, "%s_rho", output);

  switch(meshtype) {
  case MESH_FORMAT_IJK12:
  case MESH_FORMAT_IJK20:
  case MESH_FORMAT_IJK32:
    /* Open file */
    if (mpi_file_open(&fh1, output, cur_offset) != 0) {
      fprintf(stderr, "[%d] Failed to open output file\n", writer_id);
      return(1);
    }
    node_buf1 = malloc(meshrecsize * node_buf_size);
    if (node_buf1 == NULL) {
      fprintf(stderr, "[%d] Failed to allocate node_buf1\n", writer_id);
      return(1);
    }

    /* Set file view */
    retval = MPI_File_set_view(fh1, 0, MPI_MESH_T, MPI_MESH_FILE_T, 
			       "native", MPI_INFO_NULL);
    if (retval != MPI_SUCCESS) {
      MPI_Error_string(retval, errstr, &errstrlen);
      fprintf(stderr, "[%d] Failed to create file view: %s\n", 
	      writer_id, errstr);
      return(1);
    }
    break;
  case MESH_FORMAT_SORD:
    /* Open files */
    if (mpi_file_open(&fh1, output_vp, cur_offset) != 0) {
      fprintf(stderr, "[%d] Failed to open output vp file\n",
	      writer_id);
      return(1);
    }
    if (mpi_file_open(&fh2, output_vs, cur_offset) != 0) {
      fprintf(stderr, "[%d] Failed to open output vs file\n",
	      writer_id);
      return(1);
    }
    if (mpi_file_open(&fh3, output_rho, cur_offset) != 0) {
      fprintf(stderr, "[%d] Failed to open output rho file\n",
	      writer_id);
      return(1);
    }
    node_buf1 = malloc(meshrecsize * node_buf_size);
    node_buf2 = malloc(meshrecsize * node_buf_size);
    node_buf3 = malloc(meshrecsize * node_buf_size);
    if ((node_buf1 == NULL) || (node_buf2 == NULL) || (node_buf3 == NULL)) {
      fprintf(stderr, "[%d] Failed to allocate node_bufs 1,2,3\n",
	      writer_id);
      return(1);
    }
    /* Set file views */
    MPI_File_set_view(fh1, 0, MPI_MESH_T, MPI_MESH_FILE_T, "native", 
		      MPI_INFO_NULL);
    MPI_File_set_view(fh2, 0, MPI_MESH_T, MPI_MESH_FILE_T, "native", 
		      MPI_INFO_NULL);
    MPI_File_set_view(fh3, 0, MPI_MESH_T, MPI_MESH_FILE_T, "native", 
		      MPI_INFO_NULL);
    break;
  default:
    fprintf(stderr, "[%d] Unrecognized mesh type\n", writer_id);
    return(1);
    break;
  }

  writer_init_flag = 1;
  return(0);
}


int mesh_write_mpi(mesh_ijk32_t *nodes, int node_count)
{
  mesh_ijk12_t * ptr1_ijk12;
  mesh_ijk20_t * ptr1_ijk20;  
  mesh_ijk32_t * ptr1_ijk32;
  mesh_sord_t * ptr1_sord;
  mesh_sord_t * ptr2_sord;
  mesh_sord_t * ptr3_sord;
  int i;

  if (!writer_init_flag) {
    fprintf(stderr, "[%d] Mesh writer not initialized\n", writer_id);
    return(1);
  }

  if (node_count > node_buf_size) {
    fprintf(stderr, "[%d] Node_count exceeds bufsize from init function\n",
	    writer_id);
    return(1);
  }

  /* Transform node list and perform write */
  switch(meshtype) {
  case MESH_FORMAT_IJK12:
    ptr1_ijk12 = (mesh_ijk12_t *)node_buf1;
    for (i = 0; i < node_count; i++) {
      ptr1_ijk12[i].vp = nodes[i].vp;
      ptr1_ijk12[i].vs = nodes[i].vs;
      ptr1_ijk12[i].rho = nodes[i].rho;
    }
    if (mpi_file_write_at(&fh1, cur_offset, 
			  node_buf1, node_count, 
			  num_fields_mesh, &MPI_MESH_T) != 0) {
      fprintf(stderr, "[%d] Failed to write to mesh file\n", writer_id);
      return(1);
    }
    break;
  case MESH_FORMAT_IJK20:
    ptr1_ijk20 = (mesh_ijk20_t *)node_buf1;
    for (i = 0; i < node_count; i++) {
      ptr1_ijk20[i].vp = nodes[i].vp;
      ptr1_ijk20[i].vs = nodes[i].vs;
      ptr1_ijk20[i].rho = nodes[i].rho;
      ptr1_ijk20[i].qp = nodes[i].qp;
      ptr1_ijk20[i].qs = nodes[i].qs;
    }
    if (mpi_file_write_at(&fh1, cur_offset, 
			  node_buf1, node_count, 
			  num_fields_mesh, &MPI_MESH_T) != 0) {
      fprintf(stderr, "[%d] Failed to write to mesh file\n", writer_id);
      return(1);
    }
    break;
  case MESH_FORMAT_IJK32:
    ptr1_ijk32 = (mesh_ijk32_t *)node_buf1;
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
    if (mpi_file_write_at(&fh1, cur_offset, 
			  node_buf1, node_count, 
			  num_fields_mesh, &MPI_MESH_T) != 0) {
      fprintf(stderr, "[%d] Failed to write to mesh file\n", writer_id);
      return(1);
    }
    break;
  case MESH_FORMAT_SORD:
    ptr1_sord = (mesh_sord_t *)node_buf1;
    ptr2_sord = (mesh_sord_t *)node_buf2;
    ptr3_sord = (mesh_sord_t *)node_buf3;
    for (i = 0; i < node_count; i++) {
      ptr1_sord[i].val = nodes[i].vp;
      ptr2_sord[i].val = nodes[i].vs;
      ptr3_sord[i].val = nodes[i].rho;
    }
    if (mpi_file_write_at(&fh1, cur_offset, 
			  node_buf1, node_count, 
			  num_fields_mesh, &MPI_MESH_T) != 0) {
      fprintf(stderr, "[%d] Failed to write to vp mesh file\n", writer_id);
      return(1);
    }
    if (mpi_file_write_at(&fh2, cur_offset, 
			  node_buf2, node_count, 
			  num_fields_mesh, &MPI_MESH_T) != 0) {
      fprintf(stderr, "[%d] Failed to write to vs mesh file\n", writer_id);
      return(1);
    }
    if (mpi_file_write_at(&fh3, cur_offset, 
			  node_buf3, node_count, 
			  num_fields_mesh, &MPI_MESH_T) != 0) {
      fprintf(stderr, "[%d] Failed to write to rho mesh file\n", writer_id);
      return(1);
    }
    break;
  default:
    fprintf(stderr, "[%d] Unrecognized mesh type\n", writer_id);
    return(1);
    break;
  }

  cur_offset = cur_offset + node_count;

  return(0);
}


int mesh_close_mpi()
{
  switch(meshtype) {
  case MESH_FORMAT_IJK12:
  case MESH_FORMAT_IJK20:
  case MESH_FORMAT_IJK32:
    free(node_buf1);
    mpi_file_close(&fh1);
    break;
  case MESH_FORMAT_SORD:
    free(node_buf1);
    free(node_buf2);
    free(node_buf3);
    mpi_file_close(&fh1);
    mpi_file_close(&fh2);
    mpi_file_close(&fh3);
    break;
  default:
    fprintf(stderr, "[%d] Unrecognized mesh type\n", writer_id);
    return(1);
    break;
  }

  node_buf_size = 0;
  node_buf1 = NULL;
  node_buf2 = NULL;
  node_buf3 = NULL;
  meshtype = MESH_FORMAT_UNKNOWN;
  writer_init_flag = 0;

  return(0);
}
