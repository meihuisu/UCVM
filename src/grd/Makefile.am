# Autoconf/Automake binaries and headers
lib_LIBRARIES = libgrd.a
bin_PROGRAMS = grd_query run_grd.sh
include_HEADERS = grd.h grd_dtypes.h grd_config.h


# General compiler/linker flags
AM_CFLAGS =
AM_LDFLAGS = ${LDFLAGS}


# Dist sources
libgrd_a_SOURCES = grd*.c grd*.h
grd_query_SOURCES = grd_query.c
run_grd_sh_SOURCES = run_grd.sh


all: $(lib_LIBRARIES) $(bin_PROGRAMS)


############################################
# Executables
############################################

libgrd.a: grd.o grd_config.o
	$(AR) rcs $@ $^

grd_query: grd_query.o libgrd.a
	$(CC) -o $@ $^ $(AM_LDFLAGS)

run_grd.sh:


############################################
# Special build targets
############################################

clean:
	rm -f core *.o *~ $(lib_LIBRARIES) grd_query

