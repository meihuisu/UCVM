# Make libraries
lib_LIBRARIES = libcvms.a libcvms.so

# General compiler/linker flags
FC = gfortran
AR = ar
FFLAGS = -w -O3 -ffixed-line-length-132 -finit-local-zero

TARGETS = libcvms.a libcvms.so

all: $(lib_LIBRARIES)

libcvms.a: cvms_sub.o
	$(AR) rcs $@ $^

libcvms.so: cvms_sub.o
	${FC} -shared -o $@ $^

%.o: %.f
	${FC} ${FFLAGS} -c $^ -o $@

clean:
	rm -rf *.o *.so *.a
