#! /bin/sh
#
# This script generates the Visual Studio 6 build files for Windows.
# The MSVC7 conversion is currently done manually.
#
# 20041214 larsa

rm -rf simvoleon2.dsp simvoleon2.dsw installsimvoleonheaders.bat

../../configure --enable-msvcdsp --with-msvcrt=mt
make

build_pwd=`pwd`
build="`cygpath -w $build_pwd | sed -e 's/\\\\/\\\\\\\\/g'`"
source_pwd=`cd ../..; pwd`
source="`cygpath -w $source_pwd | sed -e 's/\\\\/\\\\\\\\/g'`"

sed \
  -e "s/$build/./g" \
  -e "s/$source/..\\\\../g" \
  -e 's/$/\r/g' \
  <simvoleon2.dsp >new.dsp
mv new.dsp simvoleon2.dsp

# How can I avoid the modal upgrade prompt-dialog for MSVC7.1 here???
# devenv /command "File.OpenProject $build\\coin2.dsp"

