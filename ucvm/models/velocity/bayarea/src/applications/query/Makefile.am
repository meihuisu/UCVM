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

bin_PROGRAMS = cencalvminfo cencalvmisosurface

INCLUDES = -I$(top_builddir)/include

cencalvminfo_SOURCES = cencalvminfo.cc
cencalvminfo_LDADD = -letree


cencalvmisosurface_SOURCES = cencalvmisosurface.cc
cencalvmisosurface_LDADD = $(top_builddir)/libsrc/libcencalvm.la


# End of file 
