
# Compiler/linker flags
CFLAGS = -Wall -O3 -std=c99  \
	-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE \
	-D_FILE_OFFSET_BITS=64 -fPIC
LDFLAGS = -lm


# HPCC/Shock
CC=gcc


TARGET = cvmlt_query libcvmlt.a libcvmlt.so

all: $(TARGET)


############################################
# Executables
############################################

cvmlt_query: cvmlt_query.o cvmlt.o cvmlt_proj_bilinear.o cvmlt_config.o \
	cvmlt_utils.o
	$(CC) -o $@ $^ $(LDFLAGS)

libcvmlt.a: cvmlt.o cvmlt_proj_bilinear.o cvmlt_config.o cvmlt_utils.o
	$(AR) rcs $@ $^

libcvmlt.so: cvmlt.o cvmlt_proj_bilinear.o cvmlt_config.o cvmlt_utils.o
	$(CC) -o $@ $^ -shared

%.o : %.c
	$(CC) -c $^ $(CFLAGS)


############################################
# Special build targets
############################################

clean:
	rm -f core *~ *.o ${TARGET} ../include/*.h ../lib/lib*.a
