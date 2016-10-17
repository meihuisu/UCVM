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

TESTS = testgrader

check_PROGRAMS = testgrader

testgrader_SOURCES = \
	$(top_srcdir)/extensions/applications/vsgrader/VsGrader.cc \
	TestVsGrader.cc \
	testgrader.cc

noinst_HEADERS = \
	TestVsGrader.h

testgrader_LDFLAGS =

INCLUDES = -I$(top_builddir)/include \
	-I$(top_srcdir)/extensions/applications/vsgrader

testgrader_LDADD = \
	-lcppunit -ldl \
	-letree \
	$(top_builddir)/libsrc/libcencalvm.la


# End of file 
