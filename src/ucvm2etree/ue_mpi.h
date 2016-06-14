#ifndef UE_MPI_H
#define UE_MPI_H

#include <mpi.h>

void mpi_exit(int);
void mpi_barrier();
void mpi_init(int *ac,char ***av,int *np,int *id,char *pname,int *len);
void mpi_final(char *s);

void mpi_register_octant(MPI_Datatype *dt);
void mpi_register_dispatch(MPI_Datatype *dt);

void mpi_file_open(MPI_File *fh, char *filename, MPI_Offset offset);
void mpi_file_close(MPI_File *fh);
void mpi_file_write(MPI_File *fh, void *buf, int count, int num_fields, MPI_Datatype *dt);

#endif
