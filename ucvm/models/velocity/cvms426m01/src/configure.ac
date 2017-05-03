#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT("CVM-S Improved", "4.26", "davidgil@usc.edu")
AC_CONFIG_SRCDIR([src/cvmsi.h])
AM_INIT_AUTOMAKE()
AC_PROG_RANLIB

# Checks for programs.
AC_PROG_CC
AC_PROG_FC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [main])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset pow sqrt strchr])

AC_CONFIG_FILES([Makefile src/Makefile model/cvms/Makefile test/Makefile])
AC_OUTPUT
