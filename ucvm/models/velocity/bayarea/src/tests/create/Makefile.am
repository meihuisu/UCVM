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

TESTS = testcreate

check_PROGRAMS = testcreate

testcreate_SOURCES = \
	TestGridIngester.cc \
	TestVMCreator.cc \
	testcreate.cc

noinst_HEADERS = \
	TestGridIngester.h \
	TestVMCreator.h

testcreate_LDFLAGS =

INCLUDES = -I$(top_builddir)/include

testcreate_LDADD = \
	-lcppunit -ldl \
	-letree \
	$(top_builddir)/libsrc/libcencalvm.la


# End of file 
