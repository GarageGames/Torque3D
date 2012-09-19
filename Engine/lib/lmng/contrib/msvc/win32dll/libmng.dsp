# Microsoft Developer Studio Project File - Name="libmng" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=libmng - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libmng.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libmng.mak" CFG="libmng - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libmng - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "libmng - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libmng - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBMNG_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W2 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBMNG_EXPORTS" /D "MNG_BUILD_DLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "libmng - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBMNG_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W2 /Gm /GX /ZI /Od /I "zlib" /I "jpeg" /I "lcms/include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBMNG_EXPORTS" /D "MNG_BUILD_DLL" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "libmng - Win32 Release"
# Name "libmng - Win32 Debug"
# Begin Group "mng"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\libmng_callback_xs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libmng_chunk_io.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libmng_chunk_prc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libmng_chunk_xs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libmng_cms.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libmng_display.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libmng_dither.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libmng_error.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libmng_filter.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libmng_hlapi.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libmng_jpeg.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libmng_object_prc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libmng_pixels.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libmng_prop_xs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libmng_read.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libmng_trace.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libmng_write.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libmng_zlib.c
# End Source File
# End Group
# Begin Group "jpeg"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jcapimin.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jcapistd.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jccoefct.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jccolor.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jcdctmgr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jchuff.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jcinit.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jcmainct.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jcmarker.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jcmaster.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jcomapi.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jcparam.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jcphuff.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jcprepct.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jcsample.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jctrans.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jdapimin.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jdapistd.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jdatadst.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jdatasrc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jdcoefct.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jdcolor.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jddctmgr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jdhuff.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jdinput.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jdmainct.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jdmarker.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jdmaster.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jdmerge.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jdphuff.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jdpostct.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jdsample.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jdtrans.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jerror.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jfdctflt.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jfdctfst.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jfdctint.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jidctflt.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jidctfst.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jidctint.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jidctred.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jmemmgr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jmemnobs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jquant1.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jquant2.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\jpgsrc6b\jutils.c
# End Source File
# End Group
# Begin Group "lcms"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\lcms\source\CMSCNVRT.C
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lcms\source\CMSERR.C
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lcms\source\CMSGAMMA.C
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lcms\source\CMSGMT.C
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lcms\source\cmsintrp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lcms\source\cmsio1.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lcms\source\CMSLUT.C
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lcms\source\CMSMATSH.C
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lcms\source\cmsmtrx.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lcms\source\CMSPACK.C
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lcms\source\cmspcs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lcms\source\CMSWTPNT.C
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lcms\source\cmsxform.c
# End Source File
# End Group
# Begin Group "zlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\zlib\adler32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\zlib\compress.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\zlib\crc32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\zlib\deflate.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\zlib\infblock.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\zlib\infcodes.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\zlib\inffast.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\zlib\inflate.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\zlib\inftrees.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\zlib\infutil.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\zlib\trees.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\zlib\uncompr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\zlib\zutil.c
# End Source File
# End Group
# End Target
# End Project
