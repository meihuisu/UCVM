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

data_DATA = \
	paramfile.txt

data_TMP = \
	test.etree

noinst_HEADERS = \
	TestVsGrader.dat \
	$(data_DATA)

# 'export' the input files by performing a mock install
export_datadir = $(top_builddir)/extensions/tests/vsgrader/data
export-data: $(data_DATA)
	if [ "X$(top_srcdir)" != "X$(top_builddir)" ]; then for f in $(data_DATA); do $(install_sh_DATA) $(srcdir)/$$f $(export_datadir); done; fi

clean-data:
	if [ "X$(top_srcdir)" != "X$(top_builddir)" ]; then for f in $(data_DATA); do $(RM) $(RM_FLAGS) $(export_datadir)/$$f; done; fi

# export
clean-local: clean-data
BUILT_SOURCES = export-data
CLEANFILES = \
	$(export_datadir)/$(data_TMP)


# End of file 
