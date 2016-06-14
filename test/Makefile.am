bin_PROGRAMS = unittest accepttest

# General compiler/linker flags
AM_CFLAGS = -I../src/ucvm
AM_LDFLAGS = -L../src/ucvm -lucvm ${LDFLAGS}

# CENCALVM includes
if UCVM_AM_ENABLE_CENCAL
AM_CFLAGS += -D_UCVM_ENABLE_CENCAL
endif

# CVM-H includes
if UCVM_AM_ENABLE_CVMH
AM_CFLAGS += -D_UCVM_ENABLE_CVMH
endif

# CVM-S4 includes
if UCVM_AM_ENABLE_CVMS
AM_CFLAGS += -D_UCVM_ENABLE_CVMS
endif

# CVM-SI includes
if UCVM_AM_ENABLE_CVMSI
AM_CFLAGS += -D_UCVM_ENABLE_CVMSI
endif

# CVM-NCI includes
if UCVM_AM_ENABLE_CVMNCI
AM_CFLAGS += -D_UCVM_ENABLE_CVMNCI
endif

# WFCVM includes
if UCVM_AM_ENABLE_WFCVM
AM_CFLAGS += -D_UCVM_ENABLE_WFCVM
endif

# CVMLT includes
if UCVM_AM_ENABLE_CVMLT
AM_CFLAGS += -D_UCVM_ENABLE_CVMLT
endif

# CMRG includes
if UCVM_AM_ENABLE_CMRG
AM_CFLAGS += -D_UCVM_ENABLE_CMRG
endif

# TAPE includes
if UCVM_AM_ENABLE_TAPE
AM_CFLAGS += -D_UCVM_ENABLE_TAPE
endif


# Dist sources
unittest_SOURCES = *.c *.h
accepttest_SOURCES = *.c *.h

all: $(bin_PROGRAMS)


############################################
# Executables
############################################

unittest: unittest.o test_defs.o unittest_suite_lib.o
	$(CC) -o $@ $^ $(AM_LDFLAGS)

accepttest: accepttest.o test_defs.o accepttest_suite_grid.o
	$(CC) -o $@ $^ $(AM_LDFLAGS)


############################################
# Special build targets
############################################

install:
	mkdir -p ${prefix}/tests
	cp ./cvms_input.txt ${prefix}/tests

check: unittest accepttest
	./unittest
	./accepttest
	./accept_test.py ${prefix}

clean:
	rm -rf *~ *.o unittest accepttest


