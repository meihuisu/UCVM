#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "ue_dtypes.h"
#include "ue_mpi.h"


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
  fprintf(stderr,"%s\n",s);
  MPI_Finalize();
}


void mpi_register_octant(MPI_Datatype *dt)
{
  int i;

  /* MPI data type that encapsulated etree addr, key, payload value */
  int num_fields = 10;
  MPI_Datatype ftype[10] = { MPI_UNSIGNED, MPI_UNSIGNED, MPI_UNSIGNED, 
			    MPI_UNSIGNED, MPI_INT, MPI_UNSIGNED,
			    MPI_BYTE, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT };
  int blocklen[10] = { 1, 1, 1, 1, 1, 1, UE_MAX_KEYSIZE, 1, 1, 1 };
  int sizes[10] = { sizeof(unsigned int),
		    sizeof(unsigned int),
		    sizeof(unsigned int),
		    sizeof(unsigned int),
		    sizeof(int),
		    sizeof(unsigned int),
		    sizeof(char),
		    sizeof(float),
		    sizeof(float),
		    sizeof(float) };
  MPI_Aint disp[10];

  disp[0] = 0;
  for (i = 1; i < num_fields; i++) {
    disp[i] = disp[i-1] + sizes[i-1]*blocklen[i-1];
  }

  MPI_Type_struct(num_fields, blocklen, disp, ftype, dt);
  MPI_Type_commit(dt);
  return;
}


void mpi_register_dispatch(MPI_Datatype *dt)
{
  int i;

  /* MPI data type that encapsulates column assignment, 
     oct count, status */
  int num_fields = 3;
  MPI_Datatype ftype[3] = { MPI_INT,
			    MPI_INT,
			    MPI_UNSIGNED };
  int blocklen[3] = { 1, 1, 1 };
  int sizes[3] = { sizeof(int),
		   sizeof(int),
		   sizeof(unsigned long) };
  MPI_Aint disp[3];

  disp[0] = 0;
  for (i = 1; i < num_fields; i++) {
    disp[i] = disp[i-1] + sizes[i-1]*blocklen[i-1];
  }

  MPI_Type_struct(num_fields, blocklen, disp, ftype, dt);
  MPI_Type_commit(dt);
  return;
}


void mpi_file_open(MPI_File *fh, char *filename, MPI_Offset offset)
{
  if (MPI_File_open(MPI_COMM_WORLD, filename,
		    MPI_MODE_CREATE | MPI_MODE_RDWR, 
		    MPI_INFO_NULL, fh) != MPI_SUCCESS) {
    fprintf(stderr, "Error opening file %s\n", filename);
    exit(1);
  }
  if (MPI_File_seek(*fh, offset, MPI_SEEK_SET) != MPI_SUCCESS) {
    fprintf(stderr, "Error seeking in file %s\n", filename);
    exit(1);
  }
  return;
}


void mpi_file_close(MPI_File *fh)
{
  MPI_File_close(fh);
  return;
}


void mpi_file_write(MPI_File *fh, void *buf, int count, 
		    int num_fields, MPI_Datatype *dt)
{
  MPI_Status status;
  int num_wrote;

  if (MPI_File_write(*fh, buf, count, *dt, &status) != MPI_SUCCESS) {
    fprintf(stderr, "Error writing to file\n");
    exit(1);
  }

  MPI_Get_count(&status, MPI_INT, &num_wrote);
  if (num_wrote != count * num_fields) {
    fprintf(stderr, "Error writing output, wrote %d of %d\n", 
	    num_wrote, count * num_fields);
    exit(1);
  }

  return;
}

