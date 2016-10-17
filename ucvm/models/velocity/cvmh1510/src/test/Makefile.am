# GNU Automake config

bin_PROGRAMS = unittest accepttest


# General compiler/linker flags
AM_CFLAGS = -Wall -O3 -std=c99 -D_LARGEFILE_SOURCE \
	-D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -I../src
AM_LDFLAGS = ${LDFLAGS} -L../src -lvxapi -L../gctpc/source -lgeo -lm

# Dist sources
unittest_SOURCES = *.c *.h
accepttest_SOURCES = *.c *.h

.PHONY = run_unit run_accept

all: $(bin_PROGRAMS)


############################################
# Executables
############################################

unittest: unittest.o unittest_defs.o test_helper.o test_vx_sub.o \
	test_vx_exec.o test_vx_lite_exec.o
	$(CC) -o $@ $^ $(AM_LDFLAGS)

accepttest: accepttest.o unittest_defs.o test_helper.o test_grid.o
	$(CC) -o $@ $^ $(AM_LDFLAGS)

test: $(bin_PROGRAMS)


############################################
# Special build targets
############################################

install:

run_unit: unittest
	./unittest

run_accept: accepttest
	./accepttest

check: unittest accepttest
	./unittest
	./accepttest

clean:
	rm -rf *~ *.o $(bin_PROGRAMS)

