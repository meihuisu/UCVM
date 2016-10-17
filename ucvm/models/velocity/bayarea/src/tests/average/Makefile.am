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

SUBDIRS = data

TESTS = testaverage

check_PROGRAMS = testaverage

testaverage_SOURCES = \
	TestAverager.cc \
	testaverage.cc

noinst_HEADERS = \
	TestAverager.h

testaverage_LDFLAGS =

INCLUDES = -I$(top_builddir)/include

testaverage_LDADD = \
	-lcppunit -ldl \
	-letree \
	$(top_builddir)/libsrc/libcencalvm.la


# End of file 
