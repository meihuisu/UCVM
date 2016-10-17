# GNU Automake config

EXTRA_DIST = configure

SUBDIRS = aux-config doc scripts/autoconf scripts/docgen scripts/grid \
	scripts/interpolate scripts/misc scripts/ts2gts scripts/viz \
	gctpc gctpc/source src model model/tsurf test test/inputs test/ref

INCLUDES = $(default_includes)

# CruiseControl compatibility declarations
.PHONY = run_unit run_accept
run_unit:
	cd test;$(MAKE) run_unit
run_accept:
	cd test;$(MAKE) run_accept
