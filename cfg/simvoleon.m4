############################################################################
# Usage:
#   SIM_AC_HAVE_SIMVOLEON_IFELSE( IF-FOUND, IF-NOT-FOUND )
#
# Description:
#   This macro locates the SimVoleon development system.  If it is found,
#   the set of variables listed below are set up as described and made
#   available to the configure script.
#
#   The $sim_ac_simvoleon_desired variable can be set to false externally to
#   make SimVoleon default to be excluded.
#
# Autoconf Variables:
# > $sim_ac_simvoleon_desired     true | false (defaults to true)
# < $sim_ac_simvoleon_avail       true | false
# < $sim_ac_simvoleon_cppflags    (extra flags the preprocessor needs)
# < $sim_ac_simvoleon_cflags      (extra flags the C compiler needs)
# < $sim_ac_simvoleon_cxxflags    (extra flags the C++ compiler needs)
# < $sim_ac_simvoleon_ldflags     (extra flags the linker needs)
# < $sim_ac_simvoleon_libs        (link library flags the linker needs)
# < $sim_ac_simvoleon_version     (the libSimVoleon version)
# < $sim_ac_simvoleon_msvcrt      (the MSVC++ C library SimVoleon was built with)
# < $sim_ac_simvoleon_configcmd   (the path to simvoleon-config or "false")
#
# Authors:
#   Lars J. Aas, <larsa@sim.no>
#   Morten Eriksen, <mortene@sim.no>
#

# FIXME: setting up this file was just done by copying the
# 'coin.m4' file + search'n'replace to simvoleon. Should really
# rather collect them in a common template (and the same probably
# goes for at least the so*.m4 files aswell, probably other SIM libraries
# too). 20031017 mortene.


AC_DEFUN([SIM_AC_HAVE_SIMVOLEON_IFELSE], [
AC_PREREQ([2.14a])

# official variables
sim_ac_simvoleon_avail=false
sim_ac_simvoleon_cppflags=
sim_ac_simvoleon_cflags=
sim_ac_simvoleon_cxxflags=
sim_ac_simvoleon_ldflags=
sim_ac_simvoleon_libs=
sim_ac_simvoleon_version=

# internal variables
: ${sim_ac_simvoleon_desired=true}
sim_ac_simvoleon_extrapath=

AC_ARG_WITH([simvoleon],
AC_HELP_STRING([--with-simvoleon], [enable use of SimVoleon [[default=yes]]])
AC_HELP_STRING([--with-simvoleon=DIR], [give prefix location of SimVoleon]),
  [ case $withval in
    no)  sim_ac_simvoleon_desired=false ;;
    yes) sim_ac_simvoleon_desired=true ;;
    *)   sim_ac_simvoleon_desired=true
         sim_ac_simvoleon_extrapath=$withval ;;
    esac],
  [])

case $build in
*-mks ) sim_ac_pathsep=";" ;;
* )     sim_ac_pathsep="${PATH_SEPARATOR}" ;;
esac

if $sim_ac_simvoleon_desired; then
  sim_ac_path=$PATH
  test -z "$sim_ac_simvoleon_extrapath" || ## search in --with-simvoleon path
    sim_ac_path="$sim_ac_simvoleon_extrapath/bin${sim_ac_pathsep}$sim_ac_path"
  test x"$prefix" = xNONE ||          ## search in --prefix path
    sim_ac_path="$sim_ac_path${sim_ac_pathsep}$prefix/bin"

  AC_PATH_PROG(sim_ac_simvoleon_configcmd, simvoleon-config, false, $sim_ac_path)

  if ! test "X$sim_ac_simvoleon_configcmd" = "Xfalse"; then
    test -n "$CONFIG" &&
      $sim_ac_simvoleon_configcmd --alternate=$CONFIG >/dev/null 2>/dev/null &&
      sim_ac_simvoleon_configcmd="$sim_ac_simvoleon_configcmd --alternate=$CONFIG"
  fi

  if $sim_ac_simvoleon_configcmd; then
    sim_ac_simvoleon_version=`$sim_ac_simvoleon_configcmd --version`
    sim_ac_simvoleon_cppflags=`$sim_ac_simvoleon_configcmd --cppflags`
    sim_ac_simvoleon_cflags=`$sim_ac_simvoleon_configcmd --cflags 2>/dev/null`
    sim_ac_simvoleon_cxxflags=`$sim_ac_simvoleon_configcmd --cxxflags`
    sim_ac_simvoleon_ldflags=`$sim_ac_simvoleon_configcmd --ldflags`
    sim_ac_simvoleon_libs=`$sim_ac_simvoleon_configcmd --libs`
    sim_ac_simvoleon_msvcrt=`$sim_ac_simvoleon_configcmd --msvcrt`
    sim_ac_simvoleon_cflags=`$sim_ac_simvoleon_configcmd --cflags`
    AC_CACHE_CHECK(
      [if we can compile and link with the SimVoleon library],
      sim_cv_simvoleon_avail,
      [sim_ac_save_cppflags=$CPPFLAGS
      sim_ac_save_cxxflags=$CXXFLAGS
      sim_ac_save_ldflags=$LDFLAGS
      sim_ac_save_libs=$LIBS
      CPPFLAGS="$CPPFLAGS $sim_ac_simvoleon_cppflags"
      CXXFLAGS="$CXXFLAGS $sim_ac_simvoleon_cxxflags"
      LDFLAGS="$LDFLAGS $sim_ac_simvoleon_ldflags"
      LIBS="$sim_ac_simvoleon_libs $LIBS"
      AC_LANG_PUSH(C++)

      AC_TRY_LINK(
        [#include <VolumeViz/nodes/SoVolumeRendering.h>],
        [SoVolumeRendering::init();],
        [sim_cv_simvoleon_avail=true],
        [sim_cv_simvoleon_avail=false])

      AC_LANG_POP
      CPPFLAGS=$sim_ac_save_cppflags
      CXXFLAGS=$sim_ac_save_cxxflags
      LDFLAGS=$sim_ac_save_ldflags
      LIBS=$sim_ac_save_libs
    ])
    sim_ac_simvoleon_avail=$sim_cv_simvoleon_avail
    if ! $sim_ac_simvoleon_avail; then
      AC_MSG_WARN([
Compilation and/or linking with the SimVoleon main library SDK failed, for
unknown reason. If you are familiar with configure-based configuration
and building, investigate the 'config.log' file for clues.

If you can not figure out what went wrong, please forward the 'config.log'
file to the email address <coin-support@coin3d.org> and ask for help by
describing the situation where this failed.
])
    fi
  else # no 'simvoleon-config' found
    locations=`IFS="${sim_ac_pathsep}"; for p in $sim_ac_path; do echo " -> $p/simvoleon-config"; done`
    AC_MSG_WARN([cannot find 'simvoleon-config' at any of these locations:
$locations])
    AC_MSG_WARN([
Need to be able to run 'simvoleon-config' to figure out how to build and link
against the SimVoleon library. To rectify this problem, you most likely need
to a) install SimVoleon if it has not been installed, b) add the SimVoleon install
bin/ directory to your PATH environment variable.
])
  fi
fi

if $sim_ac_simvoleon_avail; then
  ifelse([$1], , :, [$1])
else
  ifelse([$2], , :, [$2])
fi
]) # SIM_AC_HAVE_SIMVOLEON_IFELSE()

