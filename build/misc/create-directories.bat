@echo off

REM **********************************************************************
REM * Create all the directories for the installed files

pushd %COINDIR%

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
