
m4_define([VOLUMEVIZ_MAJOR], [1])
m4_define([VOLUMEVIZ_MINOR], [0])
m4_define([VOLUMEVIZ_MICRO], [0])

# this is bogus, but policy has not been set for this yet
m4_define([SC_ABI_CURRENT], [1])
m4_define([SC_ABI_AGE], [0])
m4_define([SC_ABI_REVISION], [0])


AC_INIT([VolumeViz], [1.0.0], [coin-bugs@coin3d.org])

AC_CONFIG_SRCDIR(lib/VolumeViz/nodes/VolumeRendering.cpp)
AC_CONFIG_AUX_DIR(cfg)

# If the Microsoft Visual C++ cl.exe compiler is available, set us up for
# compiling with it and to generate an MSWindows .dll file.

AC_CANONICAL_SYSTEM

SIM_AC_SETUP_MSVC_IFELSE([], [
  case $host in
  *-cygwin)
    AC_MSG_ERROR([You need Microsoft Visual C++ to build VolumeViz on Cygwin])
    ;;
  esac
])

if $BUILD_WITH_MSVC; then
  SIM_AC_SETUP_MSVCRT([
    CPPFLAGS="$CPPFLAGS $sim_ac_msvcrt_CPPFLAGS"
    CFLAGS="$CFLAGS $sim_ac_msvcrt_CFLAGS"
    CXXFLAGS="$CXXFLAGS $sim_ac_msvcrt_CXXFLAGS"
    LDFLAGS="$CFLAGS $sim_ac_msvcrt_LDFLAGS"
    LIBS="$CXXFLAGS $sim_ac_msvcrt_LIBS"
  ])
fi

AM_CONDITIONAL(BUILD_WITH_MSVC, $BUILD_WITH_MSVC)

AC_SUBST([VOLUMEVIZ_MAJOR_VERSION], [VOLUMEVIZ_MAJOR])
AC_SUBST([VOLUMEVIZ_MINOR_VERSION], [VOLUMEVIZ_MINOR])
AC_SUBST([VOLUMEVIZ_MICRO_VERSION], [VOLUMEVIZ_MICRO])
AC_SUBST([VOLUMEVIZ_VERSION], [VOLUMEVIZ_MAJOR.VOLUMEVIZ_MINOR.VOLUMEVIZ_MICRO])

VERSION=$VOLUMEVIZ_VERSION

SC_MAJOR=$VOLUMEVIZ_MAJOR_VERSION

# Libtool versioning
AC_SUBST([LT_CURRENT], [SC_ABI_CURRENT])
AC_SUBST([LT_REVISION], [SC_ABI_REVISION])
AC_SUBST([LT_AGE], [SC_ABI_AGE])

AC_DEFINE_UNQUOTED([VOLUMEVIZ_MAJOR_VERSION], [$VOLUMEVIZ_MAJOR_VERSION],
  [define this to the libVolumeViz major version number])
AC_DEFINE_UNQUOTED([VOLUMEVIZ_MINOR_VERSION], [$VOLUMEVIZ_MINOR_VERSION],
  [define this to the libVolumeViz minor version number])
AC_DEFINE_UNQUOTED([VOLUMEVIZ_MICRO_VERSION], [$VOLUMEVIZ_MICRO_VERSION],
  [define this to the libVolumeViz release version number])
AC_DEFINE_UNQUOTED([VOLUMEVIZ_VERSION], ["$VOLUMEVIZ_VERSION"],
  [define this to the full libVolumeViz major.minor.micro version number])

# *******************************************************************
# Miscellaneous options and initializations.

volumeviz_build_dir=`pwd`

# **************************************************************************
#  Locate C++ compiler and set C++ as the default language to use
#  in tests. The configure script will automatically terminate if
#  it doesn't find a C++ compiler.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_LANG_CPLUSPLUS

AC_CHECK_HEADERS([windows.h unistd.h ctype.h sys/types.h dirent.h])

AM_INIT_AUTOMAKE(VolumeViz, $VERSION)

#  Default to not building a static library. Can be overridden by the
#  user with --enable-static.
AM_DISABLE_STATIC

#  Files to cram results from the configure tests into.
AM_CONFIG_HEADER(config.h)

#  Check for pthread (required for multithreaded streaming sound)
SIM_AC_CHECK_PTHREAD([
  CPPFLAGS="$CPPFLAGS $sim_ac_pthread_cppflags"
  LDFLAGS="$LDFLAGS $sim_ac_pthread_ldflags"
  LIBS="$sim_ac_pthread_libs $LIBS"
  AC_DEFINE(HAVE_PTHREAD, 1, [pthread extensions available])
  SIM_AC_CONFIGURATION_SETTING([pthread support], [Yes])
], 
[
  AC_MSG_WARN([pthread is needed for multithreaded streaming sound])
  SIM_AC_CONFIGURATION_SETTING([pthread support], [No])
]
)

#  Turn off default maintainer make-rules -- use ./bootstrap instead.
AM_MAINTAINER_MODE

if $BUILD_WITH_MSVC; then
  RANLIB=true
  export RANLIB
else

#  Initialize libtool
AC_PROG_LIBTOOL

fi

SIM_AC_DEBUGSYMBOLS
SIM_AC_RTTI_SUPPORT
SIM_AC_EXCEPTION_HANDLING
SIM_AC_PROFILING_SUPPORT
SIM_AC_COMPILER_WARNINGS

case $enable_static in
yes | true)  VOLUMEVIZ_STATIC=true;;
*)           VOLUMEVIZ_STATIC=false;;
esac

case $enable_shared in
yes | true)  VOLUMEVIZ_STATIC=false;;
*)           VOLUMEVIZ_STATIC=true;;
esac

AC_MSG_CHECKING([for strcasecmp() function])
AC_TRY_LINK([
#include <string.h>
], [
  (void)strcasecmp(NULL, NULL);
], [
  AC_MSG_RESULT([yes])
  AC_DEFINE([HAVE_STRCASECMP],, [Define to use BSD4.3 strncasecmp()])
], [
  AC_MSG_RESULT([no])
])


AC_MSG_CHECKING([for strcmpi() function])
AC_TRY_LINK([
#include <string.h>
], [
  (void)strcmpi(NULL, NULL);
], [
  AC_MSG_RESULT([yes])
  AC_DEFINE([HAVE_STRCMPI],, [Define to use strcmpi()])
], [
  AC_MSG_RESULT([no])
])

SIM_AC_COMPILE_DEBUG([
#  CPPFLAGS="$CPPFLAGS"
], [
#  CPPFLAGS="$CPPFLAGS -DNDEBUG"
])

if test x"${with_inventor+set}" = x"set" &&
   test x"${with_inventor}" != x"no"; then
  SIM_AC_HAVE_INVENTOR_IFELSE([
    CPPFLAGS="$sim_ac_inventor_cppflags $CPPFLAGS"
    LDFLAGS="$sim_ac_inventor_ldflags $LDFLAGS"
    LIBS="$sim_ac_inventor_libs $LIBS"
  ], [AC_MSG_ERROR(couldn't compile and link against Open Inventor)])
  SIM_AC_CONFIGURATION_SETTING([Open Inventor brand], [SGI/TGS Inventor])
else
  SIM_AC_HAVE_COIN_IFELSE([
    CPPFLAGS="$CPPFLAGS $sim_ac_coin_cppflags"
    LDFLAGS="$LDFLAGS $sim_ac_coin_ldflags"
    LIBS="$sim_ac_coin_libs $LIBS"
  ], [AC_MSG_ERROR(couldn't compile and link against Coin)])
  SIM_AC_CONFIGURATION_SETTING([Open Inventor brand], [Coin])
fi

AC_SUBST([LIBFLAGS], [])

if $BUILD_WITH_MSVC; then
  rm -f vc60.pdb
  LIBFLAGS="$LIBFLAGS $sim_ac_msvcrt_LIBLDFLAGS"
  LIBS="$LIBS $sim_ac_msvcrt_LIBLIBS"
  if $VOLUMEVIZ_STATIC; then :; else
    CPPFLAGS="-DVOLUMEVIZ_MAKE_DLL $CPPFLAGS"
    VOLUMEVIZ_EXTRA_CPPFLAGS="$VOLUMEVIZ_EXTRA_CPPFLAGS -DVOLUMEVIZ_DLL"
  fi
  debugfile="`pwd`/lib/VolumeViz/VolumeViz${SC_MAJOR}.pdb"
  debugfile=`cygpath -w "$debugfile" | sed 's,\\\\,\\\\\\\\,g'`
  if $VOLUMEVIZ_STATIC; then
    SIM_AC_CONFIGURATION_SETTING([VolumeViz build type], [static .lib])
    LIBFLAGS="-LIB $LIBFLAGS"
    LIBFLAGS="$LIBFLAGS /OUT:VolumeViz$SC_MAJOR.lib"
    VOLUMEVIZ_EXTRA_CPPFLAGS="-I$includedir"
    VOLUMEVIZ_EXTRA_LDFLAGS="-L$libdir"
    VOLUMEVIZ_EXTRA_LIBS=
  else
    LIBFLAGS="-DLL /INCREMENTAL:NO $LIBFLAGS"
    case $enable_symbols in
    no | false)
      SIM_AC_CONFIGURATION_SETTING([VolumeViz build type],
                                   [dynamic .dll, no symbols])
      LIBFLAGS="$LIBFLAGS /RELEASE"
      ;;
    *)
      SIM_AC_CONFIGURATION_SETTING([VolumeViz build type],
                                   [dynamic .dll + .pdb])
      CFLAGS="/Fd$debugfile $CFLAGS"
      CXXFLAGS="/Fd$debugfile $CXXFLAGS"
      LIBFLAGS="$LIBFLAGS /DEBUG /PDB:VolumeViz$VOLUMEVIZ_MAJOR_VERSION.pdb"
      ;;
    esac

    LIBFLAGS="$LIBFLAGS /OUT:VolumeViz$SC_MAJOR.dll"
  fi
  SIM_AC_CONFIGURATION_SETTING([C library type], [$sim_ac_msvcrt])

  VOLUMEVIZ_EXTRA_LIBS="-lVolumeViz$SC_MAJOR $VOLUMEVIZ_EXTRA_LIBS"
else
  VOLUMEVIZ_EXTRA_LIBS="-lVolumeViz $VOLUMEVIZ_EXTRA_LIBS"
fi


SIM_AC_UNIQIFY_LIST(CPPFLAGS, $CPPFLAGS)
SIM_AC_UNIQIFY_LIST(LDFLAGS, $LDFLAGS)
SIM_AC_UNIQIFY_LIST(LIBS, $LIBS)

AC_OUTPUT([
        Makefile
        lib/Makefile
        lib/VolumeViz/Makefile
	lib/VolumeViz/nodes/Makefile
	lib/VolumeViz/elements/Makefile
	lib/VolumeViz/readers/Makefile
	lib/VolumeViz/misc/Makefile
])

SIM_AC_CONFIGURATION_SETTING([Install prefix], [$prefix])

SIM_AC_CONFIGURATION_SUMMARY

echo ""
echo "Now, run 'make' and 'make install' to install $PACKAGE"
echo ""

