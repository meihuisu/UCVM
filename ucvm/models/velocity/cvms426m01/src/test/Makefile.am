# Autoconf/Automake binaries and headers
bin_PROGRAMS = accepttest unittest

# General compiler/linker flags
AM_CFLAGS = ${CFLAGS} -I ../src/
AM_LDFLAGS = ${LDFLAGS} -L../src -lcvmsi
AM_FCFLAGS = ${FCFLAGS}

TARGETS = accepttest unittest

all: $(bin_PROGRAMS)

accepttest: accepttest.c
	$(CC) -o $@ $^ $(AM_CFLAGS) $(AM_LDFLAGS)
	
unittest: unittest.c
	$(CC) -o $@ $^ $(AM_CFLAGS) $(AM_LDFLAGS)
	
install:
	mkdir -p ${prefix}/test
	cp ./accepttest ${prefix}/test
	cp ./unittest ${prefix}/test
	cp -R ./data ${prefix}/test
	
clean:
	rm -rf *.o accepttest unittest
