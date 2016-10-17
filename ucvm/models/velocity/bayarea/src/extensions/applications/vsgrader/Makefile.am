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

bin_PROGRAMS = gradecencalvm

gradecencalvm_SOURCES = \
	gradecencalvm.cc \
	VsGrader.cc

INCLUDES = -I$(top_builddir)/include

gradecencalvm_LDADD = \
	-letree \
	$(top_builddir)/libsrc/libcencalvm.la


noinst_HEADERS = \
	VsGrader.h \
	VsGrader.icc


# End of file 
