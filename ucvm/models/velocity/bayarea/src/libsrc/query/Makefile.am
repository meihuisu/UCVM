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

subpackage = query
include $(top_srcdir)/subpackage.am

subpkginclude_HEADERS = \
	VMQuery.h \
	VMQuery.icc \
	cvmerror.h \
	cvmquery.h

noinst_HEADERS = \
	f77vmerror.h \
	f77vmquery.h

# export
clean-local: clean-subpkgincludeHEADERS
BUILT_SOURCES = export-subpkgincludeHEADERS
CLEANFILES = export-subpkgincludeHEADERS


# End of file 
