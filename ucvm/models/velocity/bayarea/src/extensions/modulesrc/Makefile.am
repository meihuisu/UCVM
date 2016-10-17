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

package = cencalvm

pkgpyexec_LTLIBRARIES = _cencalvmmodule.la

pkgpyexec_PYTHON = cencalvm.py

swig_sources = \
	cencalvm.i \
	CenCalVMDB.i

swig_generated = \
	cencalvm_wrap.cxx \
	cencalvm.py

_cencalvmmodule_la_LDFLAGS = -module -avoid-version \
	$(AM_LDFLAGS) $(PYTHON_LA_LDFLAGS)

dist__cencalvmmodule_la_SOURCES = $(swig_sources) $(swig_generated)

_cencalvmmodule_la_LIBADD = \
	$(top_builddir)/extensions/libsrc/libcencalvmextras.la \
	$(top_builddir)/libsrc/libcencalvm.la
if NO_UNDEFINED
_cencalvmmodule_la_LIBADD += \
	$(PYTHON_BLDLIBRARY) $(PYTHON_LIBS) $(PYTHON_SYSLIBS)
endif

INCLUDES = -I$(NUMPY_INCDIR) -I$(PYTHON_INCDIR) \
	-I$(top_builddir)/include \
	-I$(top_srcdir)/extensions/libsrc/cencalvmdb

$(srcdir)/cencalvm_wrap.cxx $(srcdir)/cencalvm.py: $(swig_sources)
	$(SWIG) -Wall -c++ -python $(SWIG_FLAGS) $<

MAINTAINERCLEANFILES = $(swig_generated)


# End of file 
