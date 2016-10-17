# GNU Automake config

lib_LIBRARIES = libgeo.a

# Set compiler/linker flags
AM_CFLAGS = -D_GNU_SOURCE -fno-builtin
AM_LDFLAGS = $(LDFLAGS)


# Dist sources
libgeo_a_SOURCES = gctp.c alberfor.c alberinv.c alconfor.c alconinv.c azimfor.c \
	  aziminv.c eqconfor.c eqconinv.c equifor.c equiinv.c gnomfor.c \
	  gnominv.c goodfor.c goodinv.c gvnspfor.c gvnspinv.c hamfor.c \
	  haminv.c imolwfor.c imolwinv.c lamazfor.c lamazinv.c merfor.c \
	  merinv.c millfor.c millinv.c molwfor.c molwinv.c obleqfor.c  \
	  obleqinv.c omerfor.c omerinv.c orthfor.c orthinv.c polyfor.c \
	  polyinv.c psfor.c psinv.c robfor.c robinv.c sinfor.c sininv.c \
	  somfor.c sominv.c sterfor.c sterinv.c stplnfor.c stplninv.c \
	  tmfor.c tminv.c utmfor.c utminv.c vandgfor.c vandginv.c \
	  wivfor.c wivinv.c wviifor.c wviiinv.c for_init.c inv_init.c \
	  cproj.c report.c lamccfor.c lamccinv.c paksz.c untfz.c sphdz.c \
	  br_gctp.c cproj.h proj.h

# Object files
LIB_OBJECTS = gctp.o alberfor.o alberinv.o alconfor.o alconinv.o azimfor.o \
	  aziminv.o eqconfor.o eqconinv.o equifor.o equiinv.o gnomfor.o \
	  gnominv.o goodfor.o goodinv.o gvnspfor.o gvnspinv.o hamfor.o \
	  haminv.o imolwfor.o imolwinv.o lamazfor.o lamazinv.o merfor.o \
	  merinv.o millfor.o millinv.o molwfor.o molwinv.o obleqfor.o  \
	  obleqinv.o omerfor.o omerinv.o orthfor.o orthinv.o polyfor.o \
	  polyinv.o psfor.o psinv.o robfor.o robinv.o sinfor.o sininv.o \
	  somfor.o sominv.o sterfor.o sterinv.o stplnfor.o stplninv.o \
	  tmfor.o tminv.o utmfor.o utminv.o vandgfor.o vandginv.o \
	  wivfor.o wivinv.o wviifor.o wviiinv.o for_init.o inv_init.o \
	  cproj.o report.o lamccfor.o lamccinv.o paksz.o untfz.o sphdz.o \
	  br_gctp.o


all: $(lib_LIBRARIES)


############################################
# Executables
############################################

libgeo.a: $(LIB_OBJECTS)
	$(AR) rcs $@ $^


############################################
# Special build targets
############################################

clean:
	rm -rf *.o *~ $(lib_LIBRARIES)
