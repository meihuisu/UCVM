# GNU Autoconf config

AC_INIT(UCVM, 15.10.0)
AC_CONFIG_AUX_DIR([./aux-config])
AM_INIT_AUTOMAKE()
AC_PROG_RANLIB

# Determine platform
AC_CANONICAL_HOST
# Check for which host we are on and setup a few things
# specifically based on the host
case $host_os in
  darwin* )
		AM_CONDITIONAL(USER_LINUX, false)
		AM_CONDITIONAL(USER_MAC, true)
        ;;
  linux* )
		AM_CONDITIONAL(USER_LINUX, true)
		AM_CONDITIONAL(USER_MAC, false)
		;;
  *)
        #Default Case
        AC_MSG_ERROR([Your platform is not currently supported])
        ;;
esac

# Determine MPI/CC compiler
#: ${CFLAGS=""}
#AC_PROG_CC
AX_PROG_CC_MPI([true],[use_mpi=yes],[use_mpi=no])

# Check for GNU compiler
if test "x$ac_compiler_gnu" = "xno"; then
AC_MSG_ERROR(["GNU C compiler or MPI wrapper based on GNU is required. Please check your programming environment."])
fi

# Set MPI flag
AM_CONDITIONAL(UCVM_HAVE_MPI, [test x"$use_mpi" = xyes])

# Parse command-line arguments
AC_ARG_ENABLE([static],
        [AS_HELP_STRING([--enable-static],
        [enable static linking])],
        [enable_static=yes],
        [enable_static=no])

if test "x$enable_static" = xyes; then
	AM_CONDITIONAL(UCVM_AM_STATIC, true)
else
	AM_CONDITIONAL(UCVM_AM_STATIC, false)
fi

AC_ARG_ENABLE([netcdf],
        [AS_HELP_STRING([--enable-netcdf],
        [enable netCDF module])],
        [enable_netcdf=yes],
        [enable_netcdf=no])

AC_ARG_ENABLE([iobuf],
        [AS_HELP_STRING([--enable-iobuf],
        [enable IOBUF module])],
        [enable_iobuf=yes],
        [enable_iobuf=no])

AC_ARG_ENABLE([model-cvmh],
        [AS_HELP_STRING([--enable-model-cvmh],
        [enable model SCEC CVM-H])],
        [enable_model_cvmh=yes],
        [enable_model_cvmh=no])

AC_ARG_ENABLE([model-cvms],
        [AS_HELP_STRING([--enable-model-cvms],
        [enable model SCEC CVM-S])],
        [enable_model_cvms=yes],
        [enable_model_cvms=no])
        
AC_ARG_ENABLE([model-cvms5],
        [AS_HELP_STRING([--enable-model-cvms5],
        [enable model SCEC CVM-S5])],
        [enable_model_cvms5=yes],
        [enable_model_cvms5=no])

AC_ARG_ENABLE([model-cencal],
        [AS_HELP_STRING([--enable-model-cencal],
        [enable model USGS CenCalVM])],
        [enable_model_cencal=yes],
        [enable_model_cencal=no])

AC_ARG_ENABLE([model-cvmsi],
        [AS_HELP_STRING([--enable-model-cvmsi],
        [enable model SCEC CVM-SI])],
        [enable_model_cvmsi=yes],
        [enable_model_cvmsi=no])

AC_ARG_ENABLE([model-cvmnci],
        [AS_HELP_STRING([--enable-model-cvmnci],
        [enable model SCEC CVM-NCI])],
        [enable_model_cvmnci=yes],
        [enable_model_cvmnci=no])

AC_ARG_ENABLE([model-wfcvm],
        [AS_HELP_STRING([--enable-model-wfcvm],
        [enable model WFCVM])],
        [enable_model_wfcvm=yes],
        [enable_model_wfcvm=no])

AC_ARG_ENABLE([model-cvmlt],
        [AS_HELP_STRING([--enable-model-cvmlt],
        [enable model Lin-Thurber])],
        [enable_model_cvmlt=yes],
        [enable_model_cvmlt=no])

AC_ARG_ENABLE([model-cmrg],
        [AS_HELP_STRING([--enable-model-cmrg],
        [enable model CMRG])],
        [enable_model_cmrg=yes],
        [enable_model_cmrg=no])

AC_ARG_ENABLE([model-tape],
        [AS_HELP_STRING([--enable-model-tape],
        [enable model Tape])],
        [enable_model_tape=yes],
        [enable_model_tape=no])


# Get required Etree library and include locations
AC_ARG_WITH([etree-include-path],
  [AS_HELP_STRING([--with-etree-include-path],
    [location of the Etree headers])],
  [ETREE_INCL="-I$withval"],
  [ETREE_INCL=""])
AC_ARG_WITH([etree-lib-path],
  [AS_HELP_STRING([--with-etree-lib-path], [location of the Etree libraries])],
  [ETREE_LIB="-L$withval -letree"],
  [ETREE_LIB="-letree"])

# Get required Proj.4 library and include locations
AC_ARG_WITH([proj4-include-path],
  [AS_HELP_STRING([--with-proj4-include-path],
    [location of the Proj.4 headers])],
  [PROJ4_INCL="-I$withval"],
  [PROJ4_INCL=""])
AC_ARG_WITH([proj4-lib-path],
  [AS_HELP_STRING([--with-proj4-lib-path], [location of the Proj.4 libraries])],
  [PROJ4_LIB="-L$withval -lproj -pthread"],
  [PROJ4_LIB="-lproj -pthread"])

# Get optional netCDF library and include locations
if test "x$enable_netcdf" = xyes; then
   AC_ARG_WITH([netcdf-include-path],
   [AS_HELP_STRING([--with-netcdf-include-path],
   [location of the netCDF headers])],
   [NETCDF_INCL="-I$withval"],
   [NETCDF_INCL=""])
   AC_ARG_WITH([netcdf-lib-path],
   [AS_HELP_STRING([--with-netcdf-lib-path], [location of the netCDF libraries])],
   [NETCDF_LIB="-L$withval -lnetcdf -lhdf5_hl -lhdf5 -lz -lm -lcurl -ldl"],
   [NETCDF_LIB="-lnetcdf -lhdf5_hl -lhdf5 -lz -lm -lcurl -ldl"])
fi

# Get required FFTW library and include locations
AC_ARG_WITH([fftw-include-path],
  [AS_HELP_STRING([--with-fftw-include-path],
    [location of the FFTW headers])],
  [FFTW_INCL="-I$withval"],
  [FFTW_INCL=""])
AC_ARG_WITH([fftw-lib-path],
  [AS_HELP_STRING([--with-fftw-lib-path], [location of the FFTW libraries])],
  [FFTW_LIB="-L$withval -lfftw3"],
  [FFTW_LIB="-lfftw3"])

# Get USGS CenCalVM library, include, model locations
if test "x$enable_model_cencal" = xyes; then
AC_ARG_WITH([cencal-include-path],
  [AS_HELP_STRING([--with-cencal-include-path],
    [location of the USGS CenCalVM headers])],
  [CENCAL_INCL="-I$withval"],
  [CENCAL_INCL="/usr/include/cencalvm"])
AC_ARG_WITH([cencal-lib-path],
  [AS_HELP_STRING([--with-cencal-lib-path], [location of the USGS CenCalVM libraries])],
  [CENCAL_LIB="-L$withval -lcencalvm -lstdc++"],
  [CENCAL_LIB="-lcencalvm -lstdc++"])
AC_ARG_WITH([cencal-model-path],
  [AS_HELP_STRING([--with-cencal-model-path], [location of the USGS CenCalVM high-rez etree])],
  [CENCAL_MODEL="$withval"],
  [CENCAL_MODEL="/usr/cencalvm/high-rez.e"])
AC_ARG_WITH([cencal-extmodel-path],
  [AS_HELP_STRING([--with-cencal-extmodel-path], [location of the USGS CenCalVM extended etree])],
  [CENCAL_EXTMODEL="$withval"],
  [CENCAL_EXTMODEL="/usr/cencalvm/ext-model.e"])
fi

# Get SCEC CVM-H library and include locations
if test "x$enable_model_cvmh" = xyes; then
AC_ARG_WITH([cvmh-include-path],
  [AS_HELP_STRING([--with-cvmh-include-path],
    [location of the SCEC CVM-H headers])],
  [CVMH_INCL="-I$withval"],
  [CVMH_INCL="/usr/include/cvmh"])
AC_ARG_WITH([cvmh-lib-path],
  [AS_HELP_STRING([--with-cvmh-lib-path], [location of the SCEC CVM-H libraries])],
  [CVMH_LIB="-L$withval -lvxapi"],
  [CVMH_LIB="-lvxapi"])
AC_ARG_WITH([cvmh-model-path],
  [AS_HELP_STRING([--with-cvmh-model-path], [location of the SCEC CVM-H model files])],
  [CVMH_MODEL="$withval"],
  [CVMH_MODEL="/usr/cvmh/bin"])

# Get GCTPC projection library location (CVM-H dependency)
AC_ARG_WITH([gctpc-lib-path],
  [AS_HELP_STRING([--with-gctpc-lib-path], [location of the CVM-H GCTPC libraries (CVM-H 11.2.0)])],
  [CVMH_LIB="$CVMH_LIB -L$withval -lgeo"],
  [CVMH_LIB="$CVMH_LIB"])
fi

# Get SCEC CVM-S library and include locations
if test "x$enable_model_cvms" = xyes; then
AC_ARG_WITH([cvms-include-path],
  [AS_HELP_STRING([--with-cvms-include-path],
    [location of the SCEC CVM-S headers])],
  [CVMS_INCL="-I$withval"],
  [CVMS_INCL="/usr/include/cvms"])
AC_ARG_WITH([cvms-lib-path],
  [AS_HELP_STRING([--with-cvms-lib-path], [location of the SCEC CVM-S libraries])],
  [CVMS_LIB="-L$withval -lcvms -lgfortran"],
  [CVMS_LIB="-lcvms -gfortran"])
AC_ARG_WITH([cvms-model-path],
  [AS_HELP_STRING([--with-cvms-model-path], [location of the SCEC CVM-S model files])],
  [CVMS_MODEL="$withval"],
  [CVMS_MODEL="/usr/cvms/src"])
fi

# Get SCEC CVM-SI library and include locations
if test "x$enable_model_cvmsi" = xyes; then
AC_ARG_WITH([cvmsi-include-path],
  [AS_HELP_STRING([--with-cvmsi-include-path],
    [location of the SCEC CVM-SI headers])],
  [CVMSI_INCL="-I$withval"],
  [CVMSI_INCL="/usr/include/cvmsi"])
AC_ARG_WITH([cvmsi-lib-path],
  [AS_HELP_STRING([--with-cvmsi-lib-path], [location of the SCEC CVM-SI libraries])],
  [CVMSI_LIB="-L$withval -lcvmsi -lgfortran"],
  [CVMSI_LIB="-lcvmsi -lgfortran"])
AC_ARG_WITH([cvmsi-model-path],
  [AS_HELP_STRING([--with-cvmsi-model-path], [location of the SCEC CVM-SI model files])],
  [CVMSI_MODEL="$withval"],
  [CVMSI_MODEL="/usr/cvmsi/model"])
fi

# Get SCEC CVM-S5 library and include locations
if test "x$enable_model_cvms5" = xyes; then
AC_ARG_WITH([cvms5-include-path],
  [AS_HELP_STRING([--with-cvms5-include-path],
    [location of the SCEC CVM-S5 headers])],
  [CVMS5_INCL="-I$withval"],
  [CVMS5_INCL="/usr/include/cvms5"])
AC_ARG_WITH([cvms5-lib-path],
  [AS_HELP_STRING([--with-cvms5-lib-path], [location of the SCEC CVM-S5 libraries])],
  [CVMS5_LIB="-L$withval -lcvms5 -lgfortran -lproj -letree"],
  [CVMS5_LIB="-lcvms5 -lgfortran -lproj -letree"])
fi

# Get SCEC CVM-NCI library and include locations
if test "x$enable_model_cvmnci" = xyes; then
AC_ARG_WITH([cvmnci-include-path],
  [AS_HELP_STRING([--with-cvmnci-include-path],
    [location of the SCEC CVM-NCI headers])],
  [CVMNCI_INCL="-I$withval"],
  [CVMNCI_INCL="/usr/include/cvmnci"])
AC_ARG_WITH([cvmnci-lib-path],
  [AS_HELP_STRING([--with-cvmnci-lib-path], [location of the SCEC CVM-NCI libraries])],
  [CVMNCI_LIB="-L$withval -lcvmnci"],
  [CVMNCI_LIB="-lcvmnci"])
AC_ARG_WITH([cvmnci-model-path],
  [AS_HELP_STRING([--with-cvmnci-model-path], [location of the SCEC CVM-NCI model files])],
  [CVMNCI_MODEL="$withval"],
  [CVMNCI_MODEL="/usr/cvmnci/model"])
fi

# Get WFCVM library and include locations
if test "x$enable_model_wfcvm" = xyes; then
AC_ARG_WITH([wfcvm-include-path],
  [AS_HELP_STRING([--with-wfcvm-include-path],
    [location of the WFCVM headers])],
  [WFCVM_INCL="-I$withval"],
  [WFCVM_INCL="/usr/include/wfcvm"])
AC_ARG_WITH([wfcvm-lib-path],
  [AS_HELP_STRING([--with-wfcvm-lib-path], [location of the WFCVM libraries])],
  [WFCVM_LIB="-L$withval -lwfcvm -lgfortran"],
  [WFCVM_LIB="-lwfcvm -lgfortan"])
AC_ARG_WITH([wfcvm-model-path],
  [AS_HELP_STRING([--with-wfcvm-model-path], [location of the WFCVM model files])],
  [WFCVM_MODEL="$withval"],
  [WFCVM_MODEL="/usr/wfcvm/src"])
fi

# Get CVMLT library and include locations
if test "x$enable_model_cvmlt" = xyes; then
AC_ARG_WITH([cvmlt-include-path],
  [AS_HELP_STRING([--with-cvmlt-include-path],
    [location of the CVMLT headers])],
  [CVMLT_INCL="-I$withval"],
  [CVMLT_INCL="/usr/include/cvmlt"])
AC_ARG_WITH([cvmlt-lib-path],
  [AS_HELP_STRING([--with-cvmlt-lib-path], [location of the CVMLT libraries])],
  [CVMLT_LIB="-L$withval -lcvmlt"],
  [CVMLT_LIB="-lcvmlt"])
AC_ARG_WITH([cvmlt-model-path],
  [AS_HELP_STRING([--with-cvmlt-model-path], [location of the CVMLT model files])],
  [CVMLT_MODEL="$withval"],
  [CVMLT_MODEL="/usr/cvmlt/model"])
fi

# Get CMRG library and include locations
if test "x$enable_model_cmrg" = xyes; then
AC_ARG_WITH([cmrg-include-path],
  [AS_HELP_STRING([--with-cmrg-include-path],
    [location of the CMRG headers])],
  [CMRG_INCL="-I$withval"],
  [CMRG_INCL="/usr/include/cmrg"])
AC_ARG_WITH([cmrg-lib-path],
  [AS_HELP_STRING([--with-cmrg-lib-path], [location of the CMRG libraries])],
  [CMRG_LIB="-L$withval -lcmrg"],
  [CMRG_LIB="-lcmrg"])
AC_ARG_WITH([cmrg-model-path],
  [AS_HELP_STRING([--with-cmrg-model-path], [location of the CMRG model files])],
  [CMRG_MODEL="$withval"],
  [CMRG_MODEL="/usr/cmrg/model"])
fi

# Get Tape library and include locations
if test "x$enable_model_tape" = xyes; then
AC_ARG_WITH([tape-include-path],
  [AS_HELP_STRING([--with-tape-include-path],
    [location of the TAPE headers])],
  [TAPE_INCL="-I$withval"],
  [TAPE_INCL="/usr/include/tape"])
AC_ARG_WITH([tape-lib-path],
  [AS_HELP_STRING([--with-tape-lib-path], [location of the TAPE libraries])],
  [TAPE_LIB="-L$withval -ltape"],
  [TAPE_LIB="-ltape"])
AC_ARG_WITH([tape-model-path],
  [AS_HELP_STRING([--with-tape-model-path], [location of the TAPE model files])],
  [TAPE_MODEL="$withval"],
  [TAPE_MODEL="/usr/tape/model"])
fi

# Make model paths visible to makefiles
AC_SUBST(CENCAL_MODEL)
AC_SUBST(CENCAL_EXTMODEL)
AC_SUBST(CVMH_MODEL)
AC_SUBST(CVMS_MODEL)
AC_SUBST(CVMSI_MODEL)
AC_SUBST(CVMNCI_MODEL)
AC_SUBST(WFCVM_MODEL)
AC_SUBST(CVMLT_MODEL)
AC_SUBST(CMRG_MODEL)
AC_SUBST(TAPE_MODEL)

# Setup compiler/linker flags
if test "x$enable_static" = xyes; then
   LDFLAGS="-static $LDFLAGS"
fi
if test "x$enable_iobuf" = xyes; then
   AC_DEFINE(USE_IOBUF_MACROS, 1)
fi

# Save copy of CFLAGS and LDFLAGS
CHECK_CFLAGS="-Wall -O3 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 $CFLAGS"
CHECK_LDFLAGS="$LDFLAGS"

# Check required ETREE installation
CFLAGS="$ETREE_INCL $CHECK_CFLAGS"
LDFLAGS="$CHECK_LDFLAGS $ETREE_LIB"
AC_CHECK_LIB(etree, etree_open, [AC_CHECK_HEADER([etree.h], [], [AC_MSG_ERROR(["Etree headers not found in $ETREE_INCL; use --with-etree-include-path"])], [AC_INCLUDES_DEFAULT])],[AC_MSG_ERROR(["Etree library not found; use --with-etree-lib-path"])])

# Check required FFTW installation
CFLAGS="$FFTW_INCL $CHECK_CFLAGS"
LDFLAGS="$CHECK_LDFLAGS $FFTW_LIB"
AC_CHECK_LIB(fftw3, fftw_execute, [AC_CHECK_HEADER([fftw3.h], [], [AC_MSG_ERROR(["FFTW headers not found in $FFTW_INCL; use --with-fftw-include-path"])], [AC_INCLUDES_DEFAULT])],[AC_MSG_ERROR(["FFTW library not found; use --with-fftw-lib-path"])])

# Check required PROJ4 installation
CFLAGS="$PROJ4_INCL $CHECK_CFLAGS"
LDFLAGS="$CHECK_LDFLAGS $PROJ4_LIB"
AC_CHECK_LIB(proj, pj_init_plus, [AC_CHECK_HEADER([proj_api.h], [], [AC_MSG_ERROR(["Proj4 header not found in $PROJ4_INCL; use --with-proj4-include-path"])
  ], [AC_INCLUDES_DEFAULT])],[AC_MSG_ERROR(["Proj4 library not found; use --with-proj4-lib-path"])], [-pthread -lm])

# Check optional NetCDF library
if test "x$enable_netcdf" = xyes; then
   echo "Checking for NetCDF library"
   CFLAGS="$NETCDF_INCL $CHECK_CFLAGS"
   LDFLAGS="$CHECK_LDFLAGS $NETCDF_LIB"
   AC_CHECK_LIB(netcdf, nc_create,
	[AM_CONDITIONAL(UCVM_HAVE_NETCDF, true)], 
	[AC_MSG_ERROR([NetCDF library not found; use --with-netcdf-include-path"])],
	[-lhdf5_hl -lhdf5 -lz -lm -lcurl -ldl])
   AC_CHECK_HEADER(netcdf.h,
	[],
	[AC_MSG_ERROR([NetCDF header not found; use --with-netcdf-lib-path"])],
	[AC_INCLUDES_DEFAULT])

else
	AM_CONDITIONAL(UCVM_HAVE_NETCDF, false)
fi

# Set final CFLAGS and LDFLAGS
CFLAGS="$CHECK_CFLAGS $ETREE_INCL $PROJ4_INCL $FFTW_INCL"
LDFLAGS="$CHECK_LDFLAGS $ETREE_LIB $PROJ4_LIB $FFTW_LIB"
if test "x$enable_netcdf" = xyes; then
   CFLAGS="$CFLAGS $NETCDF_INCL"
   LDFLAGS="$LDFLAGS $NETCDF_LIB"
fi
LDFLAGS="$LDFLAGS -lm"

# Check optional USGS CenCalVM installation
if test "x$enable_model_cencal" = xyes; then
   # Setup compiler/linker flags
   CFLAGS="$CENCAL_INCL $CFLAGS"
   LDFLAGS="$CENCAL_LIB $LDFLAGS"

   echo "Checking for USGS CenCalVM"
   AC_CHECK_LIB(cencalvm, cencalvm_createQuery, [AM_CONDITIONAL(UCVM_AM_ENABLE_CENCAL, true)], [AC_MSG_ERROR(["USGS CenCalVM library not found; use --with-cencal-lib-path"])], [-lstdc++ -pthread -letree -lproj -lm])
   AC_CHECK_HEADER(cvmquery.h, [], [AC_MSG_ERROR(["USGS CenCalVM header not found; use --with-cencal-include-path"])], [AC_INCLUDES_DEFAULT])

   AC_CHECK_FILE([$CENCAL_MODEL], [], [AC_MSG_ERROR(["USGS CenCalVM model etree not found; use --with-cencal-model-path"])])
   AC_CHECK_FILE([$CENCAL_EXTMODEL], [], [AC_MSG_ERROR(["USGS CenCalVM extmodel etree not found; use --with-cencal-extmodel-path"])])

else
   AM_CONDITIONAL(UCVM_AM_ENABLE_CENCAL, false)
fi

# Check optional SCEC CVM-H installation
if test "x$enable_model_cvmh" = xyes; then
   # Setup compiler/linker flags
   CFLAGS="$CVMH_INCL $CFLAGS"
   LDFLAGS="$CVMH_LIB $LDFLAGS"

   echo "Checking for SCEC CVM-H"
   AC_CHECK_LIB(vxapi, vx_getcoord,[AM_CONDITIONAL(UCVM_AM_ENABLE_CVMH, true)], [AC_MSG_ERROR(["SCEC CVM-H / GCTPC library not found; use --with-cvmh-lib-path and --with-gctpc-lib-path"])], [-lgeo -lm])
   AC_CHECK_HEADER(vx_sub.h, [], [AC_MSG_ERROR(["SCEC CVM-H header not found; use --with-cvmh-include-path"])], [AC_INCLUDES_DEFAULT])

   AC_CHECK_FILE([$CVMH_MODEL], [], [AC_MSG_ERROR(["SCEC CVM-H model dir not found; use --with-cvmh-model-path"])])
else
   AM_CONDITIONAL(UCVM_AM_ENABLE_CVMH, false)
fi

# Check optional SCEC CVM-S installation
if test "x$enable_model_cvms" = xyes; then
   # Setup compiler/linker flags
   CFLAGS="$CVMS_INCL $CFLAGS"
   LDFLAGS="$CVMS_LIB $LDFLAGS"

   echo "Checking for SCEC CVM-S"
   AC_CHECK_LIB(cvms, cvms_init_,[AM_CONDITIONAL(UCVM_AM_ENABLE_CVMS, true)], [AC_MSG_ERROR(["SCEC CVM-S library not found; use --with-cvms-lib-path"])],[-lgfortran -lm])
   AC_CHECK_HEADER(cvms.h,[],[AC_MSG_ERROR(["SCEC CVM-S header not found; use --with-cvms-include-path"])],[AC_INCLUDES_DEFAULT])

   AC_CHECK_FILE([$CVMS_MODEL], [], [AC_MSG_ERROR(["SCEC CVM-S model dir not found; use --with-cvms-model-path"])])
else
   AM_CONDITIONAL(UCVM_AM_ENABLE_CVMS, false)
fi

# Check optional SCEC CVM-SI installation
if test "x$enable_model_cvmsi" = xyes; then
   # Setup compiler/linker flags
   CFLAGS="$CVMSI_INCL $CFLAGS"
   LDFLAGS="$CVMSI_LIB $LDFLAGS"

   echo "Checking for SCEC CVM-SI"
   AC_CHECK_LIB(cvmsi, cvmsi_init,[AM_CONDITIONAL(UCVM_AM_ENABLE_CVMSI, true)], [AC_MSG_ERROR(["SCEC CVM-SI library not found; use --with-cvmsi-lib-path"])],[-lgfortran -lm])
   AC_CHECK_HEADER(cvmsi.h,[],[AC_MSG_ERROR(["SCEC CVM-SI header not found; use --with-cvmsi-include-path"])],[AC_INCLUDES_DEFAULT])

   AC_CHECK_FILE([$CVMSI_MODEL], [], [AC_MSG_ERROR(["SCEC CVM-SI model dir not found; use --with-cvmsi-model-path"])])
else
   AM_CONDITIONAL(UCVM_AM_ENABLE_CVMSI, false)
fi

# Check optional SCEC CVM-S5 installation
if test "x$enable_static" = xyes; then
if test "x$enable_model_cvms5" = xyes; then
   # Setup compiler/linker flags
   CFLAGS="$CVMS5_INCL $CFLAGS"
   LDFLAGS="$CVMS5_LIB $LDFLAGS"

   echo "Checking for SCEC CVM-S5"
   AC_CHECK_LIB(cvms5, cvms5_init,[AM_CONDITIONAL(UCVM_AM_ENABLE_CVMS5, true)], [AC_MSG_ERROR(["SCEC CVM-S5 library not found; use --with-cvms5-lib-path"])],[-lproj -letree -lgfortran -lm])
   AC_CHECK_HEADER(cvms5.h,[],[AC_MSG_ERROR(["SCEC CVM-S5 header not found; use --with-cvms5-include-path"])],[AC_INCLUDES_DEFAULT])
else
   AM_CONDITIONAL(UCVM_AM_ENABLE_CVMS5, false)
fi
else
   AM_CONDITIONAL(UCVM_AM_ENABLE_CVMS5, false)   
fi

# Check optional SCEC CVM-NCI installation
if test "x$enable_model_cvmnci" = xyes; then
   # Setup compiler/linker flags
   CFLAGS="$CVMNCI_INCL $CFLAGS"
   LDFLAGS="$CVMNCI_LIB $LDFLAGS"

   echo "Checking for SCEC CVM-NCI"
   AC_CHECK_LIB(cvmnci, cvmnci_init,[AM_CONDITIONAL(UCVM_AM_ENABLE_CVMNCI, true)], [AC_MSG_ERROR(["SCEC CVM-NCI library not found; use --with-cvmnci-lib-path"])],[-lm])
   AC_CHECK_HEADER(cvmnci.h,[],[AC_MSG_ERROR(["SCEC CVM-NCI header not found; use --with-cvmnci-include-path"])],[AC_INCLUDES_DEFAULT])

   AC_CHECK_FILE([$CVMNCI_MODEL], [], [AC_MSG_ERROR(["SCEC CVM-NCI model dir not found; use --with-cvmnci-model-path"])])
else
   AM_CONDITIONAL(UCVM_AM_ENABLE_CVMNCI, false)
fi

# Check optional WFCVM installation
if test "x$enable_model_wfcvm" = xyes; then
   # Setup compiler/linker flags
   CFLAGS="$WFCVM_INCL $CFLAGS"
   LDFLAGS="$WFCVM_LIB $LDFLAGS"

   echo "Checking for WFCVM"
   AC_CHECK_LIB(wfcvm, wfcvm_init_,[AM_CONDITIONAL(UCVM_AM_ENABLE_WFCVM, true)], [AC_MSG_ERROR(["WFCVM library not found; use --with-wfcvm-lib-path"])], [-lgfortran -lm])
   AC_CHECK_HEADER(wfcvm.h,[],[AC_MSG_ERROR(["WFCVM header not found; use --with-wfcvm-include-path"])],[AC_INCLUDES_DEFAULT])

   AC_CHECK_FILE([$WFCVM_MODEL], [], [AC_MSG_ERROR(["WFCVM model dir not found; use --with-wfcvm-model-path"])])
else
   AM_CONDITIONAL(UCVM_AM_ENABLE_WFCVM, false)
fi

# Check optional CVMLT installation
if test "x$enable_model_cvmlt" = xyes; then
   # Setup compiler/linker flags
   CFLAGS="$CVMLT_INCL $CFLAGS"
   LDFLAGS="$CVMLT_LIB $LDFLAGS"

   echo "Checking for CVMLT"
   AC_CHECK_LIB(cvmlt, cvmlt_init,[AM_CONDITIONAL(UCVM_AM_ENABLE_CVMLT, true)], [AC_MSG_ERROR(["CVMLT library not found; use --with-cvmlt-lib-path"])], [-lm])
   AC_CHECK_HEADER(cvmlt.h,[],[AC_MSG_ERROR(["CVMLT header not found; use --with-cvmlt-include-path"])],[AC_INCLUDES_DEFAULT])

   AC_CHECK_FILE([$CVMLT_MODEL], [], [AC_MSG_ERROR(["CVMLT model dir not found; use --with-cvmlt-model-path"])])
else
   AM_CONDITIONAL(UCVM_AM_ENABLE_CVMLT, false)
fi

# Check optional CMRG installation
if test "x$enable_model_cmrg" = xyes; then
   # Setup compiler/linker flags
   CFLAGS="$CMRG_INCL $CFLAGS"
   LDFLAGS="$CMRG_LIB $LDFLAGS"

   echo "Checking for CMRG"
   AC_CHECK_LIB(cmrg, cmrg_init,[AM_CONDITIONAL(UCVM_AM_ENABLE_CMRG, true)], [AC_MSG_ERROR(["CMRG library not found; use --with-cmrg-lib-path"])], [-lm])
   AC_CHECK_HEADER(cmrg.h,[],[AC_MSG_ERROR(["CMRG header not found; use --with-cmrg-include-path"])],[AC_INCLUDES_DEFAULT])

   AC_CHECK_FILE([$CMRG_MODEL], [], [AC_MSG_ERROR(["CMRG model dir not found; use --with-cmrg-model-path"])])
else
   AM_CONDITIONAL(UCVM_AM_ENABLE_CMRG, false)
fi

# Check optional Tape installation
if test "x$enable_model_tape" = xyes; then
   # Setup compiler/linker flags
   CFLAGS="$TAPE_INCL $CFLAGS"
   LDFLAGS="$TAPE_LIB $LDFLAGS"

   echo "Checking for TAPE"
   AC_CHECK_LIB(tape, tape_init,[AM_CONDITIONAL(UCVM_AM_ENABLE_TAPE, true)], [AC_MSG_ERROR(["TAPE library not found; use --with-tape-lib-path"])], [-pthread -lproj -lm])
   AC_CHECK_HEADER(tape.h,[],[AC_MSG_ERROR(["TAPE header not found; use --with-tape-include-path"])],[AC_INCLUDES_DEFAULT])

   AC_CHECK_FILE([$TAPE_MODEL], [], [AC_MSG_ERROR(["TAPE model dir not found; use --with-tape-model-path"])])
else
   AM_CONDITIONAL(UCVM_AM_ENABLE_TAPE, false)
fi

# Link -ldl if needed.
if test "x$enable_static" = xno; then
   LDFLAGS="$LDFLAGS -ldl"
fi

AC_CONFIG_FILES([Makefile aux-config/Makefile conf/Makefile conf/test/Makefile conf/example/Makefile model/ucvm/Makefile model/1d/Makefile src/ucvm/Makefile src/ucvm2etree/Makefile src/ucvm2mesh/Makefile src/grd/Makefile src/grd2etree/Makefile src/patch/Makefile src/ecoalesce/Makefile src/ecompact/Makefile src/basin/Makefile test/Makefile test/inputs/Makefile test/ref/Makefile test/logs/Makefile examples/Makefile man/Makefile src/ssh/Makefile src/vs30/Makefile utilities/Makefile])

AC_OUTPUT

