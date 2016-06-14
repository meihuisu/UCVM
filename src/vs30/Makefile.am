# Autoconf/Automake binaries and headers

bin_PROGRAMS = vs30_query

if UCVM_HAVE_MPI
bin_PROGRAMS += vs30_query_mpi
endif

# General compiler/linker flags
AM_CFLAGS = -I../ucvm
AM_LDFLAGS = -L../ucvm -lucvm ${LDFLAGS}

if UCVM_HAVE_MPI
AM_CFLAGS += -DUM_ENABLE_MPI
endif

# Dist sources
vs30_query_SOURCES = vs30_query.c

if UCVM_HAVE_MPI
vs30_query_mpi_SOURCES = vs30_query_mpi.c
endif

all: $(bin_PROGRAMS)


############################################
# Executables
############################################

vs30_query: vs30_query.o
	$(CC) -o $@ $^ $(AM_LDFLAGS)
	
vs30_query_mpi: vs30_query_mpi.o
	$(CC) -o $@ $^ $(AM_LDFLAGS)

############################################
# Special build targets
############################################

clean:
	rm -f core *.o *~ $(bin_PROGRAMS)
