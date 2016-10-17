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

bin_PROGRAMS = cencalvmcppquery

cencalvmcppquery_SOURCES = \
	cencalvmcppquery.cc

cencalvmcppquery_LDFLAGS =

INCLUDES = -I$(top_builddir)/include

cencalvmcppquery_LDADD = \
	$(top_builddir)/libsrc/libcencalvm.la

# ----------------------------------------------------------------------

VELMODEL = USGSBayAreaVM-08.3.0.etree
VELMODELEXT = USGSBayAreaVMExt-08.3.0.etree

testmaxres:
	cencalvmcppquery \
		-d $(VELMODEL) \
		-i ../data/sample-05.1.x.in \
		-o test_maxres.out \
		-l test.log

testfixedres:
	cencalvmcppquery \
		-d $(VELMODEL) \
		-i ../data/sample-05.1.x.in \
		-o test_fixedres.out \
		-t fixedres \
		-r 200.0 \
		-l test.log

testwaveres:
	cencalvmcppquery \
		-d $(VELMODEL) \
		-i ../data/sample-05.1.x.in \
		-o test_waveres.out \
		-t waveres \
		-r 0.2 \
		-l test.log

testextmaxres:
	cencalvmcppquery \
		-d $(VELMODEL) \
		-e $(VELMODELEXT) \
		-i ../data/sample_ext-05.1.x.in \
		-o testext_maxres.out \
		-l test.log

testextfixedres:
	cencalvmcppquery \
		-d $(VELMODEL) \
		-e $(VELMODELEXT) \
		-i ../data/sample_ext-05.1.x.in \
		-o testext_fixedres.out \
		-t fixedres \
		-r 200.0 \
		-l test.log

testextwaveres:
	cencalvmcppquery \
		-d $(VELMODEL) \
		-e $(VELMODELEXT) \
		-i ../data/sample_ext-05.1.x.in \
		-t waveres \
		-r 0.2 \
		-o testext_waveres.out \
		-l test.log

CLEANFILES = \
	test_maxres.out \
	test_fixedres.out \
	test_waveres.out \
	test.log \
	testext_maxres.out \
	testext_fixedres.out \
	testext_waveres.out


# End of file 
