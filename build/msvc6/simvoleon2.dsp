# Microsoft Developer Studio Project File - Name="simvoleon2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=simvoleon2 - Win32 DLL (Release)
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE
!MESSAGE NMAKE /f "simvoleon2.mak".
!MESSAGE
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "simvoleon2.mak" CFG="simvoleon2 - Win32 DLL (Debug)"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "simvoleon2 - Win32 LIB (Release)" (based on "Win32 (x86) Static Library")
!MESSAGE "simvoleon2 - Win32 LIB (Debug)" (based on "Win32 (x86) Static Library")
!MESSAGE "simvoleon2 - Win32 DLL (Release)" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "simvoleon2 - Win32 DLL (Debug)" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "StaticRelease"
# PROP BASE Intermediate_Dir "StaticRelease"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "StaticRelease"
# PROP Intermediate_Dir "StaticRelease"
# PROP Target_Dir ""
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /MD /W3 /GX /Ox /Gy /Zi /I "lib" /I "..\..\lib" /I "$(COINDIR)\include" /I "$(COINDIR)\include\Inventor\annex" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_LIB" /D SIMVOLEON_DEBUG=0  /D "HAVE_CONFIG_H" /D "CVR_DEBUG=0" /D "SIMVOLEON_INTERNAL" /D "COIN_NOT_DLL" /FD /c
# ADD CPP /nologo /MD /W3 /GX /Ox /Gy /Zi /I "lib" /I "..\..\lib" /I "$(COINDIR)\include" /I "$(COINDIR)\include\Inventor\annex" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_LIB" /D SIMVOLEON_DEBUG=0  /D "HAVE_CONFIG_H" /D "CVR_DEBUG=0" /D "SIMVOLEON_INTERNAL" /D "COIN_NOT_DLL" /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x414 /d "NDEBUG"
# ADD RSC /l 0x414 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /machine:I386 /out:"simvoleon2s.lib"
# ADD LIB32 /nologo /machine:I386 /out:"simvoleon2s.lib"

!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "StaticDebug"
# PROP BASE Intermediate_Dir "StaticDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "StaticDebug"
# PROP Intermediate_Dir "StaticDebug"
# PROP Target_Dir ""
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /MDd /W3 /GX /GZ /Od /Zi /I "lib" /I "..\..\lib" /I "$(COINDIR)\include" /I "$(COINDIR)\include\Inventor\annex" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_LIB" /D SIMVOLEON_DEBUG=1  /D "HAVE_CONFIG_H" /D "CVR_DEBUG=0" /D "SIMVOLEON_INTERNAL" /D "COIN_NOT_DLL" /FD /c
# ADD CPP /nologo /MDd /W3 /GX /GZ /Od /Zi /I "lib" /I "..\..\lib" /I "$(COINDIR)\include" /I "$(COINDIR)\include\Inventor\annex" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_LIB" /D SIMVOLEON_DEBUG=1  /D "HAVE_CONFIG_H" /D "CVR_DEBUG=0" /D "SIMVOLEON_INTERNAL" /D "COIN_NOT_DLL" /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x414 /d "_DEBUG"
# ADD RSC /l 0x414 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /machine:I386 /out:"simvoleon2sd.lib"
# ADD LIB32 /nologo /machine:I386 /out:"simvoleon2sd.lib"

!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /MD /W3 /GX /Ox /Gy /Zi /I "lib" /I "..\..\lib" /I "$(COINDIR)\include" /I "$(COINDIR)\include\Inventor\annex" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D SIMVOLEON_DEBUG=0 /D "HAVE_CONFIG_H" /D "SIMVOLEON_MAKE_DLL" /D "CVR_DEBUG=0" /D "SIMVOLEON_INTERNAL" /D "COIN_DLL" /FD /c
# ADD CPP /nologo /MD /W3 /GX /Ox /Gy /Zi /I "lib" /I "..\..\lib" /I "$(COINDIR)\include" /I "$(COINDIR)\include\Inventor\annex" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D SIMVOLEON_DEBUG=0 /D "HAVE_CONFIG_H" /D "SIMVOLEON_MAKE_DLL" /D "CVR_DEBUG=0" /D "SIMVOLEON_INTERNAL" /D "COIN_DLL" /FD /c
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
RCS=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 $(COINDIR)\lib\coin3.lib opengl32.lib gdi32.lib winmm.lib user32.lib /nologo /dll /release /machine:I386 /pdbtype:sept
# ADD LINK32 $(COINDIR)\lib\coin3.lib opengl32.lib gdi32.lib winmm.lib user32.lib /nologo /dll /release /machine:I386 /pdbtype:sept /out:"simvoleon2.dll" /opt:nowin98
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /GZ /Zi /Od /I "lib" /I "..\..\lib" /I "$(COINDIR)\include" /I "$(COINDIR)\include\Inventor\annex" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D SIMVOLEON_DEBUG=1 /D "HAVE_CONFIG_H" /D "SIMVOLEON_MAKE_DLL" /D "CVR_DEBUG=0" /D "SIMVOLEON_INTERNAL" /D "COIN_DLL" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /GZ /Zi /Od /I "lib" /I "..\..\lib" /I "$(COINDIR)\include" /I "$(COINDIR)\include\Inventor\annex" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D SIMVOLEON_DEBUG=1 /D "HAVE_CONFIG_H" /D "SIMVOLEON_MAKE_DLL" /D "CVR_DEBUG=0" /D "SIMVOLEON_INTERNAL" /D "COIN_DLL" /FD /c
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
RCS=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 $(COINDIR)\lib\coin3d.lib opengl32.lib gdi32.lib winmm.lib user32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 $(COINDIR)\lib\coin3d.lib opengl32.lib gdi32.lib winmm.lib user32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept /out:"simvoleon2d.dll" /opt:nowin98

!ENDIF

# Begin Target

# Name "simvoleon2 - Win32 DLL (Release)"
# Name "simvoleon2 - Win32 DLL (Debug)"
# Name "simvoleon2 - Win32 LIB (Release)"
# Name "simvoleon2 - Win32 LIB (Debug)"
# Begin Group "Documents"
# PROP Default_Filter ";txt"
# Begin Source File

SOURCE=..\..\README
# End Source File
# Begin Source File

SOURCE=..\..\NEWS
# End Source File
# Begin Source File

SOURCE=..\..\RELNOTES
# End Source File
# Begin Source File

SOURCE=..\..\LICENSE.GPL
# End Source File
# Begin Source File

SOURCE=..\..\COPYING
# End Source File
# Begin Source File

SOURCE=..\..\ChangeLog
# End Source File
# End Group
# Begin Group "Template Files"
# PROP Default_Filter "in"
# End Group
# Begin Group "Source Files"
# PROP Default_Filter "c;cpp;ic;icc;h"

# Begin Group "VolumeViz/caches sources"
# PROP Default_Filter "c;cpp;ic;icc;h"
# Begin Source File

SOURCE=..\..\lib\VolumeViz\caches\GLTextureCache.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\caches"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\caches"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\caches"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\caches"
!ENDIF
# End Source File
# End Group
# Begin Group "VolumeViz/details sources"
# PROP Default_Filter "c;cpp;ic;icc;h"
# Begin Source File

SOURCE=..\..\lib\VolumeViz\details\VolumeDetail.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\details"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\details"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\details"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\details"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\details\VolumeRenderDetail.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\details"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\details"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\details"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\details"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\details\VolumeSkinDetail.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\details"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\details"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\details"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\details"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\details\OrthoSliceDetail.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\details"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\details"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\details"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\details"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\details\ObliqueSliceDetail.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\details"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\details"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\details"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\details"
!ENDIF
# End Source File
# End Group
# Begin Group "VolumeViz/elements sources"
# PROP Default_Filter "c;cpp;ic;icc;h"
# Begin Source File

SOURCE=..\..\lib\VolumeViz\elements\CompressedTexturesElement.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\elements"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\elements"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\elements"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\elements"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\elements\GLInterpolationElement.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\elements"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\elements"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\elements"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\elements"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\elements\PalettedTexturesElement.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\elements"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\elements"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\elements"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\elements"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\elements\PageSizeElement.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\elements"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\elements"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\elements"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\elements"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\elements\StorageHintElement.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\elements"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\elements"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\elements"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\elements"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\elements\VoxelBlockElement.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\elements"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\elements"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\elements"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\elements"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\elements\TransferFunctionElement.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\elements"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\elements"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\elements"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\elements"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\elements\LightingElement.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\elements"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\elements"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\elements"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\elements"
!ENDIF
# End Source File
# End Group
# Begin Group "VolumeViz/nodes sources"
# PROP Default_Filter "c;cpp;ic;icc;h"
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\volumeraypickintersection.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\TransferFunction.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\VolumeData.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\VolumeRender.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\VolumeRendering.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\ObliqueSlice.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\OrthoSlice.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\VolumeSkin.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\VolumeFaceSet.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\VolumeIndexedFaceSet.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\VolumeTriangleStripSet.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\VolumeIndexedTriangleStripSet.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\CvrIndexedSetRenderBaseP.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\CvrNonIndexedSetRenderBaseP.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\CvrFaceSetRenderP.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\CvrIndexedFaceSetRenderP.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\CvrTriangleStripSetRenderP.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\CvrIndexedTriangleStripSetRenderP.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\nodes"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\nodes"
!ENDIF
# End Source File
# End Group
# Begin Group "VolumeViz/misc sources"
# PROP Default_Filter "c;cpp;ic;icc;h"
# Begin Source File

SOURCE=..\..\lib\VolumeViz\misc\VoxelChunk.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\misc"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\misc"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\misc"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\misc"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\misc\CLUT.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\misc"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\misc"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\misc"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\misc"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\misc\Util.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\misc"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\misc"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\misc"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\misc"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\misc\ResourceManager.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\misc"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\misc"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\misc"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\misc"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\misc\GlobalRenderLock.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\misc"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\misc"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\misc"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\misc"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\misc\GIMPGradient.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\misc"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\misc"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\misc"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\misc"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\misc\Gradient.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\misc"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\misc"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\misc"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\misc"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\misc\CentralDifferenceGradient.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\misc"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\misc"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\misc"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\misc"
!ENDIF
# End Source File
# End Group
# Begin Group "VolumeViz/readers sources"
# PROP Default_Filter "c;cpp;ic;icc;h"
# Begin Source File

SOURCE=..\..\lib\VolumeViz\readers\VolumeReader.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\readers"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\readers"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\readers"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\readers"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\readers\VRVolFileReader.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\readers"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\readers"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\readers"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\readers"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\readers\VRMemReader.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz\readers"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz\readers"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz\readers"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz\readers"
!ENDIF
# End Source File
# End Group
# Begin Group "render/2D sources"
# PROP Default_Filter "c;cpp;ic;icc;h"
# Begin Source File

SOURCE=..\..\lib\VolumeViz\render\2D\PageHandler.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\render\2D"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\render\2D"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\render\2D"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\render\2D"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\render\2D\2DTexSubPage.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\render\2D"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\render\2D"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\render\2D"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\render\2D"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\render\2D\2DTexPage.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\render\2D"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\render\2D"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\render\2D"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\render\2D"
!ENDIF
# End Source File
# End Group
# Begin Group "render/3D sources"
# PROP Default_Filter "c;cpp;ic;icc;h"
# Begin Source File

SOURCE=..\..\lib\VolumeViz\render\3D\CubeHandler.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\render\3D"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\render\3D"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\render\3D"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\render\3D"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\render\3D\3DTexSubCube.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\render\3D"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\render\3D"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\render\3D"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\render\3D"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\render\3D\3DTexCube.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\render\3D"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\render\3D"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\render\3D"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\render\3D"
!ENDIF
# End Source File
# End Group
# Begin Group "render/common sources"
# PROP Default_Filter "c;cpp;ic;icc;h"
# Begin Source File

SOURCE=..\..\lib\VolumeViz\render\common\CvrTextureObject.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\render\common"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\render\common"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\render\common"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\render\common"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\render\common\CvrRGBATexture.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\render\common"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\render\common"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\render\common"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\render\common"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\render\common\CvrPaletteTexture.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\render\common"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\render\common"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\render\common"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\render\common"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\render\common\Cvr2DPaletteTexture.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\render\common"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\render\common"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\render\common"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\render\common"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\render\common\Cvr3DPaletteTexture.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\render\common"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\render\common"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\render\common"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\render\common"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\render\common\Cvr3DPaletteGradientTexture.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\render\common"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\render\common"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\render\common"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\render\common"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\render\common\Cvr2DRGBATexture.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\render\common"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\render\common"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\render\common"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\render\common"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\render\common\Cvr3DRGBATexture.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\render\common"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\render\common"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\render\common"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\render\common"
!ENDIF
# End Source File
# End Group
# Begin Group "render/Pointset sources"
# PROP Default_Filter "c;cpp;ic;icc;h"
# Begin Source File

SOURCE=..\..\lib\VolumeViz\render\Pointset\PointRendering.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\render\Pointset"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\render\Pointset"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\render\Pointset"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\render\Pointset"
!ENDIF
# End Source File
# End Group
# Begin Group "VolumeViz sources"
# PROP Default_Filter "c;cpp;ic;icc;h"
# Begin Source File

SOURCE=..\..\lib\VolumeViz\dummy.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\VolumeViz"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\VolumeViz"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\VolumeViz"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\VolumeViz"
!ENDIF
# End Source File
# End Group
# Begin Group "Coin/gl sources"
# PROP Default_Filter "c;cpp;ic;icc;h"
# Begin Source File

SOURCE=..\..\lib\VolumeViz\Coin\gl\CoinGLPerformance.cpp
!IF  "$(CFG)" == "simvoleon2 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\Coin\gl"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\Coin\gl"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\Coin\gl"
!ELSEIF  "$(CFG)" == "simvoleon2 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\Coin\gl"
!ENDIF
# End Source File
# End Group
# End Group
# Begin Group "Public Headers"

# PROP Default_Filter "h;ic;icc"
# Begin Group "VolumeViz\C headers"
# Set Default_Filter "h"
# Begin Source File

SOURCE=lib\VolumeViz\C\basic.h
# End Source File
# End Group
# Begin Group "VolumeViz\details headers"
# Set Default_Filter "h"
# Begin Source File

SOURCE=..\..\lib\VolumeViz\details\SoVolumeDetail.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\details\SoVolumeRenderDetail.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\details\SoVolumeSkinDetail.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\details\SoOrthoSliceDetail.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\details\SoObliqueSliceDetail.h
# End Source File
# End Group
# Begin Group "VolumeViz\nodes headers"
# Set Default_Filter "h"
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\SoOrthoSlice.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\SoVolumeSkin.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\SoObliqueSlice.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\SoTransferFunction.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\SoVolumeData.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\SoVolumeRender.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\SoVolumeRendering.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\SoVolumeFaceSet.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\SoVolumeIndexedFaceSet.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\SoVolumeTriangleStripSet.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\nodes\SoVolumeIndexedTriangleStripSet.h
# End Source File
# End Group
# Begin Group "VolumeViz\readers headers"
# Set Default_Filter "h"
# Begin Source File

SOURCE=..\..\lib\VolumeViz\readers\SoVolumeReader.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\VolumeViz\readers\SoVRVolFileReader.h
# End Source File
# End Group
# End Group
# Begin Group "Private Headers"

# PROP Default_Filter "h;ic;icc"
# Begin Group "3D local includes"

# PROP Default_Filter "h;ic;icc"
# Begin Source File

SOURCE=..\..\lib\VolumeViz\render\3D\Cvr3DTexCube.h

# PROP Exclude_From_Build 1
# End Source File
# End Group
# End Group
# End Target
# End Project
