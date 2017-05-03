# Autoconf/Automake binaries and headers
bin_PROGRAMS = cvms_sub.o

# General compiler/linker flags
AM_CFLAGS = ${CFLAGS} -fPIC
AM_LDFLAGS = ${LDFLAGS}
AM_FCFLAGS = ${FCFLAGS} -fPIC

FC = gfortran

TARGETS = cvms_sub.o

all: $(bin_PROGRAMS)
	
cvms_sub.o: cvms_sub.f
	$(FC) $(AM_FCFLAGS) -o $@ -c $^
	cp cvms_sub.o ../../src/cvms_sub.o

clean:
	rm -rf *.o