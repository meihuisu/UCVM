# Autoconf/Automake binaries and headers
bin_PROGRAMS = ssh_generate ssh_merge

# General compiler/linker flags
AM_CFLAGS =
AM_LDFLAGS = ${LDFLAGS}

all: $(bin_PROGRAMS)


############################################
# Executables
############################################

ssh_generate: ssh_generate.o
	$(CC) -o $@ $^ $(AM_LDFLAGS)

ssh_merge: ssh_merge.o
	$(CC) -o $@ $^ $(AM_LDFLAGS)

############################################
# Special build targets
############################################


clean: 
	rm -f *.o *~ $(bin_PROGRAMS)

