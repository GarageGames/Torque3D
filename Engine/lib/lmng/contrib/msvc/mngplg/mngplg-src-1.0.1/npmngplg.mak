# Microsoft Developer Studio Generated NMAKE File, Based on npmngplg.dsp
!IF "$(CFG)" == ""
CFG=npmngplg - Win32 Debug
!MESSAGE No configuration specified. Defaulting to npmngplg - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "npmngplg - Win32 Release" && "$(CFG)" != "npmngplg - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "npmngplg.mak" CFG="npmngplg - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "npmngplg - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "npmngplg - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "npmngplg - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\npmngplg.dll"


CLEAN :
	-@erase "$(INTDIR)\npmngplg.obj"
	-@erase "$(INTDIR)\npmngplg.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\npmngplg.dll"
	-@erase "$(OUTDIR)\npmngplg.exp"
	-@erase "$(OUTDIR)\npmngplg.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\libmng-1.0.5" /I "..\zlib-1.1.4" /I "..\jpgsrc6b" /I "..\lcms-1.09b\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\npmngplg.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\npmngplg.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\npmngplg.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\libmng-1.0.5\Release\libmng.lib ..\zlib-1.1.4\Release\zlib.lib ..\jpgsrc6b\Release\libjpeg.lib ..\lcms-1.09b\src\Release\lcms.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\npmngplg.pdb" /machine:I386 /def:".\npmngplg.def" /out:"$(OUTDIR)\npmngplg.dll" /implib:"$(OUTDIR)\npmngplg.lib" 
DEF_FILE= \
	".\npmngplg.def"
LINK32_OBJS= \
	"$(INTDIR)\npmngplg.obj" \
	"$(INTDIR)\npmngplg.res"

"$(OUTDIR)\npmngplg.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "npmngplg - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "..\..\program files\opera\program\plugins\npmngplg.dll"


CLEAN :
	-@erase "$(INTDIR)\npmngplg.obj"
	-@erase "$(INTDIR)\npmngplg.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\npmngplg.exp"
	-@erase "$(OUTDIR)\npmngplg.lib"
	-@erase "$(OUTDIR)\npmngplg.pdb"
	-@erase "..\..\program files\opera\program\plugins\npmngplg.dll"
	-@erase "..\..\program files\opera\program\plugins\npmngplg.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\libmng-1.0.5" /I "..\zlib-1.1.4" /I "..\jpgsrc6b" /I "..\lcms-1.09b\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\npmngplg.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\npmngplg.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\npmngplg.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\libmng-1.0.5\Debug\libmng.lib ..\zlib-1.1.4\Debug\zlib.lib ..\jpgsrc6b\Debug\libjpeg.lib ..\lcms-1.09b\src\Debug\lcms.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\npmngplg.pdb" /debug /machine:I386 /def:".\npmngplg.def" /out:"c:\program files\opera\program\plugins\npmngplg.dll" /implib:"$(OUTDIR)\npmngplg.lib" /pdbtype:sept 
DEF_FILE= \
	".\npmngplg.def"
LINK32_OBJS= \
	"$(INTDIR)\npmngplg.obj" \
	"$(INTDIR)\npmngplg.res"

"..\..\program files\opera\program\plugins\npmngplg.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("npmngplg.dep")
!INCLUDE "npmngplg.dep"
!ELSE 
!MESSAGE Warning: cannot find "npmngplg.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "npmngplg - Win32 Release" || "$(CFG)" == "npmngplg - Win32 Debug"
SOURCE=.\npmngplg.c

!IF  "$(CFG)" == "npmngplg - Win32 Release"

CPP_SWITCHES=/nologo /MT /W3 /GX /O2 /I "..\libmng-1.0.5" /I "..\zlib-1.1.4" /I "..\jpgsrc6b" /I "..\lcms-1.09b\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\npmngplg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "npmngplg - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\libmng-1.0.5" /I "..\zlib-1.1.4" /I "..\jpgsrc6b" /I "..\lcms-1.09b\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\npmngplg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\npmngplg.rc

"$(INTDIR)\npmngplg.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

