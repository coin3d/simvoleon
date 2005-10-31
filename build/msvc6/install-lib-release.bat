@echo off

echo Installing to %COINDIR%

if not "%COINDIR%"=="" goto coindirset
echo The COINDIR environment variable must be set to point to a directory
echo to be able to perform the installation procedure.
exit
:coindirset
if exist %COINDIR%\*.* goto coindirexists
echo The COINDIR environment variable must point to an existing directory
echo to be able to perform the installation procedure.
exit
:coindirexists

pushd %COINDIR%

REM **********************************************************************
REM * Create all the directories for the installed files

if exist bin\*.* goto binexists
echo mkdir %COINDIR%\bin
mkdir bin
:binexists
if exist lib\*.* goto libexists
echo mkdir %COINDIR%\lib
mkdir lib
:libexists
if exist include\*.* goto includeexists
echo mkdir %COINDIR%\include
mkdir include
:includeexists
chdir include
if exist VolumeViz\*.* goto volumevizexists
echo mkdir %COINDIR%\include\VolumeViz
mkdir VolumeViz
:volumevizexists
chdir VolumeViz
if exist C\*.* goto cexists
echo mkdir %COINDIR%\include\VolumeViz\C
mkdir C
:cexists
if exist details\*.* goto detailsexists
echo mkdir %COINDIR%\include\VolumeViz\details
mkdir details
:detailsexists
if exist nodes\*.* goto nodesexists
echo mkdir %COINDIR%\include\VolumeViz\nodes
mkdir nodes
:nodesexists
if exist readers\*.* goto readersexists
echo mkdir %COINDIR%\include\VolumeViz\readers
mkdir readers
:readersexists

popd

REM **********************************************************************
REM * Copy files

echo Installing header files...
call install-headers.bat

echo Installing binaries...
xcopy simvoleon2s.lib %COINDIR%\lib\ /R /Y

echo Done.

