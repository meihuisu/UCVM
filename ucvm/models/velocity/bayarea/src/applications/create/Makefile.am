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

bin_PROGRAMS = \
	gencencalvm \
	packcencalvm

gencencalvm_SOURCES = \
	gencencalvm.cc

INCLUDES = -I$(top_builddir)/include

gencencalvm_LDADD = \
	-letree \
	$(top_builddir)/libsrc/libcencalvm.la

packcencalvm_SOURCES = \
	packcencalvm.cc

packcencalvm_LDADD = \
	-letree \
	$(top_builddir)/libsrc/libcencalvm.la


# End of file 
