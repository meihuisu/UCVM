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
	sample-05.0.0.out \
	sample-05.0.1.out \
	sample-05.0.1_fixedres.out \
	sample-05.0.1_waveres.out \
	sample-05.0.x.in \
	sample-05.1.0_maxres.out \
	sample-05.1.0_fixedres.out \
	sample-05.1.0_waveres.out \
	sample-05.1.x.in \
	sample_ext-05.1.x.in \
	sample_ext-05.1.0_maxres.out \
	sample_ext-05.1.0_fixedres.out \
	sample_ext-05.1.0_waveres.out \
	sample-08.2.0_maxres.out \
	sample-08.2.0_fixedres.out \
	sample-08.2.0_waveres.out \
	sample_ext-08.2.0_maxres.out \
	sample_ext-08.2.0_fixedres.out \
	sample_ext-08.2.0_waveres.out \
	sample-08.3.0_maxres.out \
	sample-08.3.0_fixedres.out \
	sample-08.3.0_waveres.out \
	sample_ext-08.3.0_maxres.out \
	sample_ext-08.3.0_fixedres.out \
	sample_ext-08.3.0_waveres.out

noinst_HEADERS = \
	$(data_DATA)

# 'export' the input files by performing a mock install
export_datadir = $(top_builddir)/examples/query/data
export-data: $(data_DATA)
	if [ "X$(top_srcdir)" != "X$(top_builddir)" ]; then for f in $(data_DATA); do $(install_sh_DATA) $(srcdir)/$$f $(export_datadir); done; fi

clean-data:
	if [ "X$(top_srcdir)" != "X$(top_builddir)" ]; then for f in $(data_DATA); do $(RM) $(RM_FLAGS) $(export_datadir)/$$f; done; fi

# export
clean-local: clean-data
BUILT_SOURCES = export-data


# End of file 
