
m4_define([SIMVOLEON_MAJOR], [2])
m4_define([SIMVOLEON_MINOR], [0])
m4_define([SIMVOLEON_MICRO], [1])
m4_define([SIMVOLEON_BETA], [])

# Converted to Libtool versioning.
m4_define([SC_ABI_CURRENT], [m4_eval((SIMVOLEON_MAJOR*20)+SIMVOLEON_MINOR)])
m4_define([SC_ABI_REVISION], [SIMVOLEON_MICRO])
m4_define([SC_ABI_AGE], [SIMVOLEON_MINOR])

AC_INIT([SIM Voleon],
        [SIMVOLEON_MAJOR.SIMVOLEON_MINOR.SIMVOLEON_MICRO[]SIMVOLEON_BETA],
        [coin-support@coin3d.org])

AC_CONFIG_SRCDIR(lib/VolumeViz/nodes/VolumeRendering.cpp)
AC_CONFIG_AUX_DIR(cfg)

# If the Microsoft Visual C++ cl.exe compiler is available, set us up for
# compiling with it and to generate an MSWindows .dll file.

AC_CANONICAL_SYSTEM

simvoleon_build_dir=`pwd`
simvoleon_src_dir=`cd $srcdir; pwd`

AC_SUBST([simvoleon_src_dir])
AC_SUBST([simvoleon_build_dir])

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
    # These are set later, as they should only be set if we're building as
    # a DLL, and not when building a static lib:
    #
    # LDFLAGS="$CFLAGS $sim_ac_msvcrt_LDFLAGS"
    # LIBS="$CXXFLAGS $sim_ac_msvcrt_LIBS"
  ])
  AC_SUBST([SIMVOLEON_MSVC_LIBC], [$sim_ac_msvcrt])
fi

AC_SUBST([SIMVOLEON_MAJOR_VERSION], [SIMVOLEON_MAJOR])
AC_SUBST([SIMVOLEON_MINOR_VERSION], [SIMVOLEON_MINOR])
AC_SUBST([SIMVOLEON_MICRO_VERSION], [SIMVOLEON_MICRO])
AC_SUBST([SIMVOLEON_BETA_VERSION], [SIMVOLEON_BETA])
AC_SUBST([SIMVOLEON_VERSION], [$SIMVOLEON_MAJOR_VERSION.$SIMVOLEON_MINOR_VERSION.$SIMVOLEON_MICRO_VERSION$SIMVOLEON_BETA_VERSION])
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
if test x"$SIMVOLEON_BETA_VERSION" = x""; then :; else
  AC_DEFINE_UNQUOTED([SIMVOLEON_BETA_VERSION], [$SIMVOLEON_BETA_VERSION],
    [define this to the libSimVoleon beta version letter])
fi

# **************************************************************************
# FIXME: Mac OS X support not in place yet. 20040506 mortene.

AC_SUBST([MAC_FRAMEWORK], false)
AM_CONDITIONAL([MAC_FRAMEWORK], false)

# **************************************************************************
#  Locate C++ compiler and set C++ as the default language to use
#  in tests. The configure script will automatically terminate if
#  it doesn't find a C++ compiler.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_LANG_CPLUSPLUS

# *******************************************************************
#  These are used for constructing the simvoleon-config file.

AC_SUBST([SIMVOLEON_EXTRA_CPPFLAGS], [$CPPFLAGS])
AC_SUBST([SIMVOLEON_EXTRA_CFLAGS], [])
AC_SUBST([SIMVOLEON_EXTRA_CXXFLAGS], [])
AC_SUBST([SIMVOLEON_EXTRA_LDFLAGS], [$LDFLAGS])
AC_SUBST([SIMVOLEON_EXTRA_LIBS], [$LIBS])

# *******************************************************************

AM_INIT_AUTOMAKE(SIMVoleon, $VERSION)

#  Default to not building a static library. Can be overridden by the
#  user with --enable-static.
AM_DISABLE_STATIC

#  Files to cram results from the configure tests into.
AM_CONFIG_HEADER(lib/config.h)

# *******************************************************************
# Configure an alternate installation?

AC_ARG_WITH(
  [alternate],
  [AC_HELP_STRING([--with-alternate=string],
                  [specify name of alternate configuration])],
  [CONFIG=$withval],
  [CONFIG=default])

AC_ARG_WITH(
  [suffix],
  [AC_HELP_STRING([--with-suffix=string],
                  [specify library name suffix (e.g. "_g")])],
  [SUFFIX=$withval],
  [SUFFIX=])

AC_SUBST(CONFIG)
AC_SUBST(SUFFIX)

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
SIM_AC_DETECT_COMMON_COMPILER_FLAGS

# Used to discern between private and public header files, among other
# things.

CPPFLAGS="$CPPFLAGS -DSIMVOLEON_INTERNAL"

# *******************************************************************

# **************************************************************************
# Figure out if library should be built static or dynamic.

case ${enable_static-no} in
yes | true) SIMVOLEON_STATIC=true
            enable_shared=no ;; # fix --enable-static bug? in autoconf 2.58
*)          SIMVOLEON_STATIC=false ;;
esac

case ${enable_shared-default} in
default)    ;;
yes | true) SIMVOLEON_STATIC=false ;;
*)          SIMVOLEON_STATIC=true ;;
esac

# *******************************************************************
# Option to disable potential error condition in configure script
# (e.g. for when only building documentation on a Coin-install-less
# system).

sim_ac_build_library=true
AC_ARG_ENABLE(
  [build],
  [AC_HELP_STRING([--disable-build], [disable configuration for library build])],
  [case $enableval in
  no | false)
   sim_ac_build_library=false
   SIM_AC_CONFIGURATION_SETTING([SIM Voleon build type], [build disabled])
   ;;
  esac])

AM_CONDITIONAL(BUILD_LIBRARY, $sim_ac_build_library)

# Documentation generation settings.

AC_ARG_VAR([htmldir],
           [destination for HTML docs (default ${datadir}/SIMVoleon/html)])

AC_ARG_ENABLE(html,
  AC_HELP_STRING([--enable-html], [build and install SIM Voleon HTML documentation]),
  [case $enableval in
    yes | true) want_html=yes ;;
    *)          want_html=no ;;
  esac],
  [want_html=no])

case $htmldir in
"")
  htmldir="$datadir/SIMVoleon/html"
  ;;
/*)
  # do nothing - absolute path
  ;;
*)
  htmldir="\${prefix}/$htmldir"
  ;;
esac

AC_SUBST(htmldir)

AC_ARG_ENABLE(man,
  AC_HELP_STRING([--enable-man], [build and install SIM Voleon man pages]),
  [case $enableval in
    yes | true) want_man=yes ;;
    *)          want_man=no ;;
  esac],
  [want_man=no])

# Used in the Doxygen parameter file.
AC_SUBST([VOLEON_DOC_HTML], [`echo $want_html | tr '[a-z]' '[A-Z]'`])
AC_SUBST([voleon_html_dir], [`pwd`/html])
AC_SUBST([VOLEON_DOC_MAN], [`echo $want_man | tr '[a-z]' '[A-Z]'`])
AC_SUBST([voleon_man_dir], [`pwd`/man])

AM_CONDITIONAL(BUILD_MANPAGES, test x"$want_man" = x"yes")
AM_CONDITIONAL(BUILD_HTMLPAGES, test x"$want_html" = x"yes")

if test x"$want_man" = x"yes"; then
  SIM_AC_CONFIGURATION_SETTING([manpage installation], [$mandir])
fi

if test x"$want_html" = x"yes"; then
  SIM_AC_CONFIGURATION_SETTING([HTML installation], [$htmldir])
fi

if test x"$want_html" != xno -o x"$want_man" != xno; then
  SIM_AC_DOXYGEN_TOOL([], [SIM_AC_ERROR([no-doxygen])])
  AC_PATH_PROG(sim_ac_perl_exe, perl, false, $PATH)
  if test x"$sim_ac_perl_exe" = xfalse; then
    AC_MSG_WARN(Could not find the Perl executable)
  fi
fi

# path_tag is used to identify paths in docs/simvoleon.doxygen that needs to be
# transformed using cygpath under cygwin.

case $host in
*-cygwin) path_tag="<PATH>" ;;
*)        path_tag= ;;
esac

AC_SUBST(path_tag)

voleon_build_dir=`pwd`
voleon_src_dir=`cd "$srcdir"; pwd`
AC_SUBST([voleon_build_dir])
AC_SUBST([voleon_src_dir])

# *******************************************************************
# Check that we can compile and link against a Coin version >= 2.0.0.

if $sim_ac_build_library; then
  sim_ac_configure_save_ldflags=$LDFLAGS
  sim_ac_configure_save_libs=$LIBS

  SIM_AC_HAVE_COIN_IFELSE(
    [CPPFLAGS="$CPPFLAGS $sim_ac_coin_cppflags"
     LDFLAGS="$LDFLAGS $sim_ac_coin_ldflags"
     LIBS="$sim_ac_coin_libs $LIBS"

     SIMVOLEON_EXTRA_CPPFLAGS="$SIMVOLEON_EXTRA_CPPFLAGS $sim_ac_coin_cppflags"
     SIMVOLEON_EXTRA_LDFLAGS="$SIMVOLEON_EXTRA_LDFLAGS $sim_ac_coin_ldflags"
     SIMVOLEON_EXTRA_LIBS="$sim_ac_coin_libs $SIMVOLEON_EXTRA_LIBS"
    ],
    [AC_MSG_ERROR(could not compile and link against Coin)]
  )

  # Coin 2.3.0 is needed for the cc_glglue_has_arb_fragment_program()
  # function, and the SoGLCacheContextElement::scheduleDeleteCallback()
  # method.
  AC_MSG_CHECKING([if Coin version is >= 2.3.0])
  AC_TRY_COMPILE(
     [#include <Inventor/SbBasic.h>],
     [#if (COIN_MAJOR_VERSION < 2)
      #error hepp
      #endif
      #if (COIN_MAJOR_VERSION == 2)
        #if (COIN_MINOR_VERSION < 3)
        #error hepp
        #endif
      #endif
     ],
     [AC_MSG_RESULT(yes)],
     [AC_MSG_RESULT(no)
      AC_MSG_ERROR(SIM Voleon needs Coin v2.3.0 or newer.)
     ]
  )

  if $SIMVOLEON_STATIC && $BUILD_WITH_MSVC; then
    # don't link with coin and friends when building SIMVoleon as a static lib
    LDFLAGS=$sim_ac_configure_save_ldflags
    LIBS=$sim_ac_configure_save_libs
  fi
fi # sim_ac_build_library

# **************************************************************************

# append compiler flags detected earlier, now that all configure tests
# are done...

CFLAGS="$CFLAGS $sim_ac_compiler_CFLAGS"
CXXFLAGS="$CXXFLAGS $sim_ac_compiler_CXXFLAGS"

# *******************************************************************

AC_SUBST([LIBFLAGS], [])

if $BUILD_WITH_MSVC; then
  rm -f vc60.pdb
  if $SIMVOLEON_STATIC; then
    SIMVOLEON_EXTRA_CPPFLAGS="$SIMVOLEON_EXTRA_CPPFLAGS -DSIMVOLEON_NOT_DLL"
  else
    CPPFLAGS="-DSIMVOLEON_MAKE_DLL $CPPFLAGS"
    SIMVOLEON_EXTRA_CPPFLAGS="$SIMVOLEON_EXTRA_CPPFLAGS -DSIMVOLEON_DLL"
    LIBFLAGS="$sim_ac_msvcrt_LIBLDFLAGS $LIBFLAGS"
    LIBS="$sim_ac_msvcrt_LIBLIBS $LIBS"
  fi
  debugfile="`pwd`/lib/VolumeViz/simvoleon${SIMVOLEON_MAJOR_VERSION}${SUFFIX}.pdb"
  debugfile=`cygpath -w "$debugfile" | sed 's,\\\\,\\\\\\\\,g'`
  if $SIMVOLEON_STATIC; then
    SIM_AC_CONFIGURATION_SETTING([SIM Voleon build type], [static .lib])
    LIBFLAGS="-LIB $LIBFLAGS"
    LIBFLAGS="$LIBFLAGS /OUT:simvoleon${SIMVOLEON_MAJOR_VERSION}${SUFFIX}.lib"
  else
    LIBFLAGS="-DLL /INCREMENTAL:NO $LIBFLAGS"
    case $enable_symbols in
    no | false)
      SIM_AC_CONFIGURATION_SETTING([SIM Voleon build type],
                                   [dynamic .dll, no symbols])
      LIBFLAGS="$LIBFLAGS /RELEASE"
      ;;
    *)
      SIM_AC_CONFIGURATION_SETTING([SIM Voleon build type],
                                   [dynamic .dll + .pdb])
      CFLAGS="/Fd$debugfile -g $CFLAGS"
      CXXFLAGS="/Fd$debugfile -g $CXXFLAGS"
      LIBFLAGS="$LIBFLAGS /DEBUG /PDB:simvoleon${SIMVOLEON_MAJOR_VERSION}${SUFFIX}.pdb"
      ;;
    esac

    LIBFLAGS="$LIBFLAGS /OUT:simvoleon${SIMVOLEON_MAJOR_VERSION}${SUFFIX}.dll"
  fi
  SIM_AC_CONFIGURATION_SETTING([C library type], [$sim_ac_msvcrt])

  SIMVOLEON_EXTRA_LIBS="-lsimvoleon${SIMVOLEON_MAJOR_VERSION}${SUFFIX} $SIMVOLEON_EXTRA_LIBS"
else
  SIMVOLEON_EXTRA_LIBS="-lSimVoleon${SUFFIX} $SIMVOLEON_EXTRA_LIBS"
fi

# *******************************************************************

AC_SUBST([COMPILER], ["$CXX"])
# don't report source hierarchy path to wrapmsvc.exe from simvoleon-config
if $BUILD_WITH_MSVC; then COMPILER="wrapmsvc.exe"; fi

AC_SUBST([ac_compiler_gnu])

# *******************************************************************

SIM_AC_UNIQIFY_OPTION_LIST(SIMVOLEON_EXTRA_CPPFLAGS, -I$includedir $SIMVOLEON_EXTRA_CPPFLAGS)
SIM_AC_UNIQIFY_OPTION_LIST(SIMVOLEON_EXTRA_LDFLAGS, -L$libdir $SIMVOLEON_EXTRA_LDFLAGS)
SIM_AC_UNIQIFY_OPTION_LIST(SIMVOLEON_EXTRA_LIBS, $SIMVOLEON_EXTRA_LIBS)
SIM_AC_UNIQIFY_LIST(CPPFLAGS, $CPPFLAGS)
SIM_AC_UNIQIFY_LIST(LDFLAGS, $LDFLAGS)
SIM_AC_UNIQIFY_LIST(LIBS, $LIBS)

# **************************************************************************
# Microsoft Developer Studio Project files

SIM_AC_MSVC_DSP_SETUP([SIMVOLEON], [SIM Voleon], [simvoleon], [-I$simvoleon_build_dir/lib -I$simvoleon_src_dir/lib])

# *******************************************************************

AC_OUTPUT([
        Makefile
        html/Makefile
        bin/Makefile
        bin/simvoleon-config
        lib/Makefile
        lib/VolumeViz/Makefile
        lib/VolumeViz/C/Makefile
	lib/VolumeViz/caches/Makefile
	lib/VolumeViz/details/Makefile
	lib/VolumeViz/elements/Makefile
	lib/VolumeViz/misc/Makefile
	lib/VolumeViz/nodes/Makefile
	lib/VolumeViz/nodes/gradients/Makefile
	lib/VolumeViz/readers/Makefile
	lib/VolumeViz/render/Makefile
	lib/VolumeViz/render/2D/Makefile
        lib/VolumeViz/render/3D/Makefile
        lib/VolumeViz/render/common/Makefile
        lib/VolumeViz/render/Pointset/Makefile
])

# *******************************************************************

SIM_AC_CONFIGURATION_SETTING([Install prefix], [$prefix])

SIM_AC_CONFIGURATION_SUMMARY

echo ""
echo "Now, run 'make' and 'make install' to install $PACKAGE"
echo ""

# *******************************************************************
