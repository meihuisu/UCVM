# Autoconf/Automake binaries and headers
bin_PROGRAMS = ecoalesce

# General compiler/linker flags
AM_CFLAGS =
AM_LDFLAGS = ${LDFLAGS}


# Dist sources
ecoalesce_SOURCES = e*.c e*.h


all: $(bin_PROGRAMS)


############################################
# Executables
############################################

ecoalesce:  ecoalesce.o ehelper.o
	$(CC) -o $@ $^ $(AM_LDFLAGS)


############################################
# Special build targets
############################################

clean:
	rm -rf $(bin_PROGRAMS) core *.o *~


