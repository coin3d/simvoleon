
m4_define([SIMVOLEON_MAJOR], [1])
m4_define([SIMVOLEON_MINOR], [0])
m4_define([SIMVOLEON_MICRO], [0])

# Converted to Libtool versioning.
m4_define([SC_ABI_CURRENT], [m4_eval((SIMVOLEON_MAJOR*20)+SIMVOLEON_MINOR)])
m4_define([SC_ABI_REVISION], [SIMVOLEON_MICRO])
m4_define([SC_ABI_AGE], [SIMVOLEON_MINOR])

AC_INIT([SimVoleon], [SIMVOLEON_MAJOR.SIMVOLEON_MINOR.SIMVOLEON_MICRO], [coin-support@coin3d.org])

AC_CONFIG_SRCDIR(lib/VolumeViz/nodes/VolumeRendering.cpp)
AC_CONFIG_AUX_DIR(cfg)

# If the Microsoft Visual C++ cl.exe compiler is available, set us up for
# compiling with it and to generate an MSWindows .dll file.

AC_CANONICAL_SYSTEM

SIM_AC_MSVC_DSP_ENABLE_OPTION

# If the Microsoft Visual C++ cl.exe compiler is available, set us up for
# compiling with it and to generate an MSWindows .dll file.

SIM_AC_SETUP_MSVCPP_IFELSE
AM_CONDITIONAL(BUILD_WITH_MSVC, $BUILD_WITH_MSVC)

if $BUILD_WITH_MSVC; then
  SIM_AC_SETUP_MSVCRT([
    CPPFLAGS="$CPPFLAGS $sim_ac_msvcrt_CPPFLAGS"
    CFLAGS="$CFLAGS $sim_ac_msvcrt_CFLAGS"
    CXXFLAGS="$CXXFLAGS $sim_ac_msvcrt_CXXFLAGS"
    LDFLAGS="$CFLAGS $sim_ac_msvcrt_LDFLAGS"
    LIBS="$CXXFLAGS $sim_ac_msvcrt_LIBS"
  ])
  AC_SUBST([SIMVOLEON_MSVC_LIBC], [$sim_ac_msvcrt])
fi

AC_SUBST([SIMVOLEON_MAJOR_VERSION], [SIMVOLEON_MAJOR])
AC_SUBST([SIMVOLEON_MINOR_VERSION], [SIMVOLEON_MINOR])
AC_SUBST([SIMVOLEON_MICRO_VERSION], [SIMVOLEON_MICRO])
AC_SUBST([SIMVOLEON_VERSION], [SIMVOLEON_MAJOR.SIMVOLEON_MINOR.SIMVOLEON_MICRO])

VERSION=$SIMVOLEON_VERSION

# Libtool versioning
AC_SUBST([LT_CURRENT], [SC_ABI_CURRENT])
AC_SUBST([LT_REVISION], [SC_ABI_REVISION])
AC_SUBST([LT_AGE], [SC_ABI_AGE])

#  These are for lib/VolumeViz/C/basic.h and lib/VolumeViz/Makefile.am.

AC_DEFINE_UNQUOTED([SIMVOLEON_MAJOR_VERSION], [$SIMVOLEON_MAJOR_VERSION],
  [define this to the libSimVoleon major version number])
AC_DEFINE_UNQUOTED([SIMVOLEON_MINOR_VERSION], [$SIMVOLEON_MINOR_VERSION],
  [define this to the libSimVoleon minor version number])
AC_DEFINE_UNQUOTED([SIMVOLEON_MICRO_VERSION], [$SIMVOLEON_MICRO_VERSION],
  [define this to the libSimVoleon release version number])
AC_DEFINE_UNQUOTED([SIMVOLEON_VERSION], ["$SIMVOLEON_VERSION"],
  [define this to the full libSimVoleon major.minor.micro version number])

# **************************************************************************
#  Locate C++ compiler and set C++ as the default language to use
#  in tests. The configure script will automatically terminate if
#  it doesn't find a C++ compiler.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_LANG_CPLUSPLUS

# *******************************************************************

AM_INIT_AUTOMAKE(SimVoleon, $VERSION)

#  Default to not building a static library. Can be overridden by the
#  user with --enable-static.
AM_DISABLE_STATIC

#  Files to cram results from the configure tests into.
AM_CONFIG_HEADER(lib/config.h)

# *******************************************************************
# Compiler checking.

## Make it possible for the user to turn off optimization flags for
## the compiler before running the fatal error checks below.
SIM_AC_COMPILER_OPTIMIZATION

## Smoke out some known defunct compilers.
SIM_AC_COMPILER_CPLUSPLUS_FATAL_ERRORS

# *******************************************************************

AC_CHECK_HEADERS([unistd.h sys/types.h])

#  Turn off default maintainer make-rules -- use ./bootstrap instead.
AM_MAINTAINER_MODE

if $BUILD_WITH_MSVC; then
  RANLIB=true
  export RANLIB
fi

# Initialize libtool.
#
# FIXME: this must be done even when we're not using libtool (as when
# building with MSVC++), because that's where at least the enable_static
# and enable_shared flags are set up. 20030324 mortene.
AC_PROG_LIBTOOL

SIM_AC_COMPILE_DEBUG([
  CPPFLAGS="$CPPFLAGS -DCVR_DEBUG=1"
], [
  CPPFLAGS="$CPPFLAGS -DCVR_DEBUG=0"
])

SIM_AC_DEBUGSYMBOLS
SIM_AC_RTTI_SUPPORT
SIM_AC_EXCEPTION_HANDLING
SIM_AC_PROFILING_SUPPORT
SIM_AC_COMPILER_WARNINGS

# Used to discern between private and public header files, among other
# things.

CPPFLAGS="$CPPFLAGS -DSIMVOLEON_INTERNAL"

# *******************************************************************

case $enable_static in
yes | true)  SIMVOLEON_STATIC=true;;
*)           SIMVOLEON_STATIC=false;;
esac

case $enable_shared in
yes | true)  SIMVOLEON_STATIC=false;;
*)           SIMVOLEON_STATIC=true;;
esac

# *******************************************************************
# Check that we can compile and link against a Coin version >= 2.0.0.

SIM_AC_HAVE_COIN_IFELSE([
  CPPFLAGS="$CPPFLAGS $sim_ac_coin_cppflags"
  LDFLAGS="$LDFLAGS $sim_ac_coin_ldflags"
  LIBS="$sim_ac_coin_libs $LIBS"
], [AC_MSG_ERROR(could not compile and link against Coin)])

AC_MSG_CHECKING([if Coin version is >= 2.0.0])
AC_TRY_COMPILE(
   [#include <Inventor/SbBasic.h>],
   [#if (COIN_MAJOR_VERSION < 2)
    #error hepp
    #endif],
   [AC_MSG_RESULT(yes)],
   [AC_MSG_RESULT(no)
    AC_MSG_ERROR(SimVoleon needs Coin v2.0.0 or older.)]
)

# *******************************************************************

AC_SUBST([LIBFLAGS], [])

if $BUILD_WITH_MSVC; then
  rm -f vc60.pdb
  LIBFLAGS="$LIBFLAGS $sim_ac_msvcrt_LIBLDFLAGS"
  LIBS="$LIBS $sim_ac_msvcrt_LIBLIBS"
  if $SIMVOLEON_STATIC; then
    SIMVOLEON_EXTRA_CPPFLAGS="$SIMVOLEON_EXTRA_CPPFLAGS -DSIMVOLEON_NOT_DLL"
  else
    CPPFLAGS="-DSIMVOLEON_MAKE_DLL $CPPFLAGS"
    SIMVOLEON_EXTRA_CPPFLAGS="$SIMVOLEON_EXTRA_CPPFLAGS -DSIMVOLEON_DLL"
  fi
  debugfile="`pwd`/lib/VolumeViz/simvoleon${SIMVOLEON_MAJOR_VERSION}.pdb"
  debugfile=`cygpath -w "$debugfile" | sed 's,\\\\,\\\\\\\\,g'`
  if $SIMVOLEON_STATIC; then
    SIM_AC_CONFIGURATION_SETTING([SimVoleon build type], [static .lib])
    LIBFLAGS="-LIB $LIBFLAGS"
    LIBFLAGS="$LIBFLAGS /OUT:simvoleon$SIMVOLEON_MAJOR_VERSION.lib"
  else
    LIBFLAGS="-DLL /INCREMENTAL:NO $LIBFLAGS"
    case $enable_symbols in
    no | false)
      SIM_AC_CONFIGURATION_SETTING([SimVoleon build type],
                                   [dynamic .dll, no symbols])
      LIBFLAGS="$LIBFLAGS /RELEASE"
      ;;
    *)
      SIM_AC_CONFIGURATION_SETTING([SimVoleon build type],
                                   [dynamic .dll + .pdb])
      CFLAGS="/Fd$debugfile -g $CFLAGS"
      CXXFLAGS="/Fd$debugfile -g $CXXFLAGS"
      LIBFLAGS="$LIBFLAGS /DEBUG /PDB:simvoleon$SIMVOLEON_MAJOR_VERSION.pdb"
      ;;
    esac

    LIBFLAGS="$LIBFLAGS /OUT:simvoleon$SIMVOLEON_MAJOR_VERSION.dll"
  fi
  SIM_AC_CONFIGURATION_SETTING([C library type], [$sim_ac_msvcrt])

  SIMVOLEON_EXTRA_LIBS="-lsimvoleon$SIMVOLEON_MAJOR_VERSION $SIMVOLEON_EXTRA_LIBS"
else
  SIMVOLEON_EXTRA_LIBS="-lSimVoleon $SIMVOLEON_EXTRA_LIBS"
fi


SIM_AC_UNIQIFY_LIST(CPPFLAGS, $CPPFLAGS)
SIM_AC_UNIQIFY_LIST(LDFLAGS, $LDFLAGS)
SIM_AC_UNIQIFY_LIST(LIBS, $LIBS)

AC_OUTPUT([
        Makefile
        lib/Makefile
        lib/VolumeViz/Makefile
        lib/VolumeViz/C/Makefile
	lib/VolumeViz/details/Makefile
	lib/VolumeViz/elements/Makefile
	lib/VolumeViz/misc/Makefile
	lib/VolumeViz/nodes/Makefile
	lib/VolumeViz/nodes/gradients/Makefile
	lib/VolumeViz/readers/Makefile
	lib/VolumeViz/render/Makefile
	lib/VolumeViz/render/2D/Makefile
])

SIM_AC_CONFIGURATION_SETTING([Install prefix], [$prefix])

SIM_AC_CONFIGURATION_SUMMARY

echo ""
echo "Now, run 'make' and 'make install' to install $PACKAGE"
echo ""
