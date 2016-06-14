# Autoconf/Automake binaries and headers

bin_PROGRAMS = grd2etree

# General compiler/linker flags
AM_CFLAGS =  -I../ucvm -I../grd
AM_LDFLAGS = -L../ucvm -lucvm -L../grd -lgrd ${LDFLAGS}

# Dist sources
grd2etree_SOURCES = g*.c g*.h

all: $(bin_PROGRAMS)


############################################
# Executables
############################################

grd2etree: grd2etree.o ge_config.o
	$(CC) -o $@ $^ $(AM_LDFLAGS)


############################################
# Special build targets
############################################

clean: 
	rm -f *.o *~ $(bin_PROGRAMS)

