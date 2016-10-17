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

subpackage = storage
include $(top_srcdir)/subpackage.am

subpkginclude_HEADERS = \
	ErrorHandler.h \
	ErrorHandler.icc \
	GeomCenCA.h \
	GeomCenCA.icc \
	Geometry.h \
	Payload.h \
	Projector.h \
	etreefwd.h \
	proj4fwd.h

noinst_HEADERS =

# export
clean-local: clean-subpkgincludeHEADERS
BUILT_SOURCES = export-subpkgincludeHEADERS
CLEANFILES = export-subpkgincludeHEADERS


# End of file 
