# -*- Makefile -*-
#
# ----------------------------------------------------------------------
#
#                           Brad T. Aagaard
#                        U.S. Geological Survey
#
# <LicenseText>
#
# ----------------------------------------------------------------------
#

ACLOCAL_AMFLAGS = -I m4

noinst_HEADERS = \
	CHANGES \
	README \
	INSTALL \
	TODO

SUBDIRS = \
	libsrc \
	applications \
	examples

if ENABLE_SPATIAL
  SUBDIRS += extensions
endif
if ENABLE_VSGRADER
  SUBDIRS += extensions
endif

if ENABLE_DOCUMENTATION
  SUBDIRS += doc
endif

if ENABLE_TESTING
  SUBDIRS += tests
endif


DISTCHECK_CONFIGURE_FLAGS = \
	--enable-testing \
	--enable-documentation \
	CPPFLAGS="-I${ETREE_INCDIR} -I${PROJ4_INCDIR} -I${CPPUNIT_INCDIR}" \
	LDFLAGS="-L${ETREE_LIBDIR} -L${PROJ4_LIBDIR} -L${CPPUNIT_LIBDIR}"


# End of file 
