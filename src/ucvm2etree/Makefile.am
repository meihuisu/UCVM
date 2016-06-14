# Autoconf/Automake binaries and headers

bin_PROGRAMS = ucvm2etree

if UCVM_HAVE_MPI
bin_PROGRAMS += ucvm2etree-extract-MPI \
		ucvm2etree-sort-MPI \
		ucvm2etree-merge-MPI
endif

# General compiler/linker flags
AM_CFLAGS = -I../ucvm
AM_LDFLAGS = -L../ucvm -lucvm ${LDFLAGS}

if UCVM_HAVE_MPI
AM_CFLAGS += -DUE_ENABLE_MPI
endif

# Dist sources
ucvm2etree_SOURCES = ucvm2etree*.c ue_*.c ue_*.h
ucvm2etree_extract_MPI_SOURCES = ucvm2etree*.c ue_*.c ue_*.h
ucvm2etree_sort_MPI_SOURCES = ucvm2etree*.c ue_*.c ue_*.h
ucvm2etree_merge_MPI_SOURCES = ucvm2etree*.c ue_*.c ue_*.h


all: $(bin_PROGRAMS)


############################################
# Executables
############################################

ucvm2etree: ucvm2etree.o ue_extract.o ue_utils.o ue_config.o
	$(CC) -o $@ $^ $(AM_LDFLAGS)

ucvm2etree-extract-MPI: ucvm2etree_extract_MPI.o ue_extract.o \
		ue_mpi.o ue_utils.o ue_config.o
	$(CC) -o $@ $^ $(AM_LDFLAGS)

ucvm2etree-sort-MPI: ucvm2etree_sort_MPI.o ue_mpi.o ue_utils.o \
		ue_config.o
	$(CC) -o $@ $^ $(AM_LDFLAGS)

ucvm2etree-merge-MPI: ucvm2etree_merge_MPI.o ue_merge.o ue_mpi.o \
		ue_queue.o ue_utils.o ue_config.o
	$(CC) -o $@ $^ $(AM_LDFLAGS)


############################################
# Special build targets
############################################

clean: 
	rm -f *.o *~ $(bin_PROGRAMS)


