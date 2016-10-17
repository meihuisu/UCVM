# GNU Automake config

lib_LIBRARIES = libvxapi.a
bin_PROGRAMS = vx vx_lite vx_slice run_vx.sh run_vx_lite.sh
include_HEADERS = vx_sub.h

# Optional cvmdst program
if VX_ENABLE_GTS
bin_PROGRAMS += cvmdst
endif


# General compiler/linker flags
AM_CFLAGS = -Wall -O3 -std=c99 -D_LARGEFILE_SOURCE \
	-D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64
AM_LDFLAGS = ${LDFLAGS} -L../gctpc/source -lgeo


# Dist sources
libvxapi_a_SOURCES = vx_sub.c scec1d.c vs30_gtl.c vx_io.c utils.c *.h
vx_SOURCES = vx.c
vx_slice_SOURCES = vx_lite.c
vx_lite_SOURCES = vx_slice.c
run_vx_sh_SOURCES = run_vx.sh
run_vx_lite_sh_SOURCES = run_vx_lite.sh

if VX_ENABLE_GTS
cvmdst_SOURCES = cvm_dst.c
endif

all: $(lib_LIBRARIES) $(bin_PROGRAMS)


############################################
# Executables
############################################

libucvm.a: version.h vx_sub.o scec1d.o vs30_gtl.o vx_io.o utils.o
	$(AR) rcs $@ $^

vx: vx.o
	$(CC) -o $@ $^ $(AM_LDFLAGS)

vx_lite: vx_lite.o libvxapi.a
	$(CC) -o $@ $^ $(AM_LDFLAGS)

vx_slice: vx_slice.o libvxapi.a
	$(CC) -o $@ $^ $(AM_LDFLAGS)

run_vx.sh:

run_vx_lite.sh:

cvmdst: cvm_dst.o
	$(CC) -o $@ $^ $(AM_LDFLAGS)


############################################
# Special build targets
############################################

clean:
	rm -f *~ *.a *.o vx$(EXEEXT) vx_lite$(EXEEXT) \
	vx_slice$(EXEEXT) cvmdst$(EXEEXT)
