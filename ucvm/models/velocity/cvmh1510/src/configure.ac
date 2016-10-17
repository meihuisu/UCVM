# GNU Autoconf config

AC_INIT(cvmh, 11.9.0)
AC_CONFIG_AUX_DIR([./aux-config])
AM_INIT_AUTOMAKE()
AC_PROG_RANLIB

# Determine MPI/CC compiler
: ${CFLAGS=""}
AC_PROG_CC

# Check for GNU compiler
if test "x$ac_compiler_gnu" = "xno"; then
AC_MSG_ERROR(["GNU C compiler or MPI wrapper based on GNU is required. Please check your programming environment."])
fi


# Parse command-line arguments
AC_ARG_WITH([gts],
        [AS_HELP_STRING([--with-gts],
        [build GTS based targets like cvmdst])],
        [with_gts=yes],
        [with_gts=no])

if test "x$with_gts" = xyes; then
   # Get GTS library and include locations
   AC_ARG_WITH([gts-include-path],
   [AS_HELP_STRING([--with-gts-include-path],
	 [location of the GTS headers])],
	 [GTS_INCL="-I$withval"],
	 [GTS_INCL="-I/usr/include/gts"])
   AC_ARG_WITH([gts-lib-path],
   [AS_HELP_STRING([--with-gts-lib-path], [location of the GTS libraries])],
   [GTS_LIB="-L$withval -lgts"],
   [GTS_LIB="-lgts"])
fi


# Check optional GTS installation
if test "x$with_gts" = xyes; then
   # Setup compiler/linker flags
   CFLAGS="$CFLAGS $GTS_INCL `pkg-config --cflags glib-2.0`"
   LDFLAGS="$LDFLAGS $GTS_LIB `pkg-config --libs glib-2.0`"

   echo "Checking for GTS library"
   AC_CHECK_LIB(gts, gts_file_read,[AM_CONDITIONAL(VX_ENABLE_GTS, true)], [AC_MSG_ERROR(["GTS library not found; use --with-gts-lib-path"])], [-lm])
   AC_CHECK_HEADER(gts.h, [], [AC_MSG_ERROR(["GTS header not found; use --with-gts-include-path"])], [AC_INCLUDES_DEFAULT])
else
   AM_CONDITIONAL(VX_ENABLE_GTS, false)
fi

CFLAGS="$CFLAGS"
LDFLAGS="$LDFLAGS -lm"

AC_CONFIG_FILES([Makefile aux-config/Makefile doc/Makefile scripts/autoconf/Makefile scripts/docgen/Makefile scripts/grid/Makefile scripts/interpolate/Makefile scripts/misc/Makefile scripts/ts2gts/Makefile scripts/viz/Makefile gctpc/Makefile gctpc/source/Makefile src/Makefile model/Makefile model/tsurf/Makefile test/Makefile test/inputs/Makefile test/ref/Makefile])

AC_OUTPUT
