# Autoconf/Automake binaries and headers
bin_PROGRAMS = patchmodel

# General compiler/linker flags
AM_CFLAGS = -I../ucvm
AM_LDFLAGS = -L../ucvm -lucvm ${LDFLAGS}

# Dist sources
patchmodel_SOURCES = p*.c p*.h


all: $(bin_PROGRAMS)


############################################
# Executables
############################################

patchmodel: patchmodel.o patch_config.o
	$(CC) -o $@ $^ $(AM_LDFLAGS)


############################################
# Special build targets
############################################


clean: 
	rm -f *.o *~ $(bin_PROGRAMS)

