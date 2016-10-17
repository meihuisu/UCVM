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

TESTS = testquery

check_PROGRAMS = testquery

testquery_SOURCES = \
	TestVMQuery.cc \
	testquery.cc

noinst_HEADERS = \
	TestVMQuery.h

testquery_LDFLAGS =

INCLUDES = -I$(top_builddir)/include

testquery_LDADD = \
	-lcppunit -ldl \
	-letree \
	$(top_builddir)/libsrc/libcencalvm.la


# End of file 
