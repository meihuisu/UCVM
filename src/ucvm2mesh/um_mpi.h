#ifndef UM_MPI_H
#define UM_MPI_H

#include <mpi.h>
#include "ucvm.h"
#include "um_mesh.h"

void mpi_exit(int);
void mpi_barrier();
void mpi_init(int *ac,char ***av,int *np,int *id,char *pname,int *len);
void mpi_final(char *s);

/* MPI Datatype registration functions */
void mpi_register_mesh_ijk32(MPI_Datatype *MPI_MESH_8_T, int *num_fields);
void mpi_register_mesh_ijk20(MPI_Datatype *MPI_MESH_5_T, int *num_fields);
void mpi_register_mesh_ijk12(MPI_Datatype *MPI_MESH_3_T, int *num_fields);
void mpi_register_mesh_sord(MPI_Datatype *MPI_MESH_1_T, int *num_fields);
void mpi_register_stat_4(MPI_Datatype *MPI_STAT_4_T, int *num_fields);

/* MPI I/O helper functions */
int mpi_file_open(MPI_File *fh, char *filename, MPI_Offset offset);
void mpi_file_close(MPI_File *fh);
int mpi_file_write(MPI_File *fh, void *buf, int count, 
		   int num_fields, MPI_Datatype *dt);
int mpi_file_write_at(MPI_File *fh, MPI_Offset offset, 
		      void *buf, int count, 
		      int num_fields, MPI_Datatype *dt);

/* MPI I/O 3D partitioned mesh writer */
int mesh_open_mpi(int myid, int nproc, 
		     ucvm_dim_t *mesh_dims, ucvm_dim_t *proc_dims,
		     char *output, mesh_format_t mtype, int bufsize);
int mesh_write_mpi(mesh_ijk32_t *nodes, int node_count);
int mesh_close_mpi();


#endif
