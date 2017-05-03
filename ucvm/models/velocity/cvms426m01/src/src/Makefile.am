# Autoconf/Automake binaries and headers
lib_LIBRARIES = libcvmsi.a
bin_PROGRAMS = cvmsi_query FD_GRID_XYZ_rect lonlat2grd utm2lonlat
objects = cvmsi_geo2xy.o utm_geo.o utm2grd.o utm2lonlat.o grd2utm.o FD_GRID_XYZ_rect.o lonlat2grd.o findnode.o

# General compiler/linker flags
AM_CFLAGS = ${CFLAGS} -fPIC
AM_LDFLAGS = ${LDFLAGS}
AM_FCFLAGS = ${FCFLAGS} -fPIC

TARGETS = cvmsi_query libcvmsi.a libcvmsi.so FD_GRID_XYZ_rect lonlat2grd utm2lonlat

all: $(bin_PROGRAMS) $(lib_LIBRARIES) libcvmsi.so

cvmsi_query: cvmsi_query.o cvmsi.o cvmsi_utils.o cvmsi_geo2xy.o \
		utm_geo.o utm2grd.o grd2utm.o cvms_sub.o vs30_gtl.o
	$(FC) -o $@ $^ $(AM_FCFLAGS)

libcvmsi.a: cvmsi.o cvmsi_utils.o cvmsi_geo2xy.o \
		utm_geo.o utm2grd.o grd2utm.o cvms_sub.o vs30_gtl.o
	$(AR) rcs $@ $^ 

libcvmsi.so: cvmsi.o cvmsi_utils.o cvmsi_geo2xy.o \
		utm_geo.o utm2grd.o grd2utm.o cvms_sub.o vs30_gtl.o
	$(FC) -shared $(AM_CFLAGS) -o libcvmsi.so $^

FD_GRID_XYZ_rect: FD_GRID_XYZ_rect.o utm2grd.o grd2utm.o utm_geo.o
	$(FC) -o $@ $^ $(AM_FCFLAGS)

lonlat2grd: lonlat2grd.o findnode.o utm_geo.o utm2grd.o grd2utm.o
	$(FC) -o $@ $^ $(AM_FCFLAGS)

utm2lonlat: utm2lonlat.o utm_geo.o
	$(FC) -o $@ $^ $(AM_FCFLAGS)
	
$(objects): %.o: %.f90
	$(FC) -o $@ -c $^ $(AM_FCFLAGS)	

install: all
	mkdir -p ${prefix}
	mkdir -p ${libdir}
	mkdir -p ${includedir}
	mkdir -p ${bindir}
	mkdir -p ${prefix}/model
	cp libcvmsi.a ${libdir}
	cp libcvmsi.so ${libdir}
	cp cvmsi.h ${includedir}
	cp cvmsi_query ${bindir}
	cp -R ../model/i26 ${prefix}/model
	cp -R ../model/cvms ${prefix}/model

clean:
	rm -rf *.o *~ *.a *.so cvmsi_query