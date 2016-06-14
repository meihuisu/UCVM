# Autoconf/Automake binaries and headers
lib_LIBRARIES = libucvm.a
bin_PROGRAMS = ucvm_query run_ucvm.sh
include_HEADERS = ucvm.h ucvm_dtypes.h ucvm_config.h \
		ucvm_grid.h ucvm_proj_bilinear.h \
		ucvm_proj_ucvm.h ucvm_meta_etree.h \
		ucvm_meta_patch.h ucvm_utils.h

# General compiler/linker flags
AM_CFLAGS =
AM_LDFLAGS = ${LDFLAGS}

# Default set of cvms to include
MODEL_TARGS = ucvm_model_1d.o ucvm_model_1dgtl.o ucvm_model_bbp1d.o \
		ucvm_model_plugin.o ucvm_model_elygtl.o ucvm_model_etree.o \
		ucvm_model_cmuetree.o ucvm_model_patch.o

# CENCALVM includes
if UCVM_AM_ENABLE_CENCAL
AM_CFLAGS += -D_UCVM_ENABLE_CENCAL
MODEL_TARGS += ucvm_model_cencal.o
endif

# CVM-H includes
if UCVM_AM_ENABLE_CVMH
AM_CFLAGS += -D_UCVM_ENABLE_CVMH
MODEL_TARGS += ucvm_model_cvmh.o
endif

# CVM-S4 includes
if UCVM_AM_ENABLE_CVMS
AM_CFLAGS += -D_UCVM_ENABLE_CVMS
MODEL_TARGS += ucvm_model_cvms.o
endif

# CVM-S5 includes
if UCVM_AM_ENABLE_CVMS5
AM_CFLAGS += -D_UCVM_ENABLE_CVMS5
endif

# CVM-SI includes
if UCVM_AM_ENABLE_CVMSI
AM_CFLAGS += -D_UCVM_ENABLE_CVMSI
MODEL_TARGS += ucvm_model_cvmsi.o
endif

# CVM-NCI includes
if UCVM_AM_ENABLE_CVMNCI
AM_CFLAGS += -D_UCVM_ENABLE_CVMNCI
MODEL_TARGS += ucvm_model_cvmnci.o
endif

# WFCVM includes
if UCVM_AM_ENABLE_WFCVM
AM_CFLAGS += -D_UCVM_ENABLE_WFCVM
MODEL_TARGS += ucvm_model_wfcvm.o
endif

# CVMLT includes
if UCVM_AM_ENABLE_CVMLT
AM_CFLAGS += -D_UCVM_ENABLE_CVMLT
MODEL_TARGS += ucvm_model_cvmlt.o
endif

# CMRG includes
if UCVM_AM_ENABLE_CMRG
AM_CFLAGS += -D_UCVM_ENABLE_CMRG
MODEL_TARGS += ucvm_model_cmrg.o
endif

# TAPE includes
if UCVM_AM_ENABLE_TAPE
AM_CFLAGS += -D_UCVM_ENABLE_TAPE
MODEL_TARGS += ucvm_model_tape.o
endif

if UCVM_AM_STATIC 
AM_CFLAGS += -D_UCVM_AM_STATIC
endif

# Dist sources
libucvm_a_SOURCES = ucvm*.c ucvm*.h
ucvm_query_SOURCES = ucvm_query.c
run_ucvm_sh_SOURCES = run_ucvm.sh


all: $(lib_LIBRARIES) $(bin_PROGRAMS)


############################################
# Executables
############################################

#test_ucvm: test_ucvm.o ucvm.o ucvm_utils.o ucvm_config.o libucvm.a
#	$(CC) -o $@ $^ $(AM_LDFLAGS)

#test_grid: test_grid.o ucvm_grid.o ucvm.o ucvm_utils.o ucvm_config.o \
#		ucvm_proj_bilinear.o libucvm.a
#	$(CC) -o $@ $^ $(AM_LDFLAGS)

libucvm.a: ucvm.o ucvm_config.o ucvm_grid.o \
		ucvm_proj_bilinear.o ucvm_proj_ucvm.o \
		ucvm_interp.o ucvm_map.o ucvm_utils.o \
		ucvm_meta_etree.o ucvm_meta_patch.o \
		$(MODEL_TARGS)
	$(AR) rcs $@ $^

ucvm_query: ucvm_query.o ucvm.o ucvm_config.o ucvm_utils.o libucvm.a 
	$(CC) -o $@ $^ $(AM_LDFLAGS)

run_ucvm.sh:


############################################
# Special build targets
############################################

clean:
	rm -f core *.o *~ $(lib_LIBRARIES) ucvm_query


