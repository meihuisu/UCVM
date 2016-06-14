# Autoconf/Automake binaries and headers
bin_PROGRAMS = ecompact

# General compiler/linker flags
AM_CFLAGS =
AM_LDFLAGS = ${LDFLAGS}


# Dist sources
ecompact_SOURCES = e*.c e*.h


all: $(bin_PROGRAMS)


############################################
# Executables
############################################

ecompact: ehelper.o ecompact.o
	$(CC) -o $@ $^ $(AM_LDFLAGS)


############################################
# Special build targets
############################################

clean:
	rm -rf $(bin_PROGRAMS) core *.o *~
