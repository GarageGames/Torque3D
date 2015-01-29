//=============================================================================
//  General component library for WIN32
//  Copyright (C) 2000, UAB BBDSoft ( http://www.bbdsoft.com/ )
//
// This material is provided "as is", with absolutely no warranty expressed
// or implied. Any use is at your own risk.
//
// Permission to use or copy this software for any purpose is hereby granted 
// without fee, provided the above notices are retained on all copies.
// Permission to modify the code and to distribute modified code is granted,
// provided the above notices are retained, and a notice that the code was
// modified is included with the above copyright notice.
//
//  The author of this program may be contacted at developers@bbdsoft.com
//=============================================================================

#ifndef _VERSINFO_
   #include "versinfo.h"
#endif

#ifndef _STDEXCEPT_
   #include "stdexcept"
#endif

#ifndef _WINDOWS_
   #define WIN32_LEAN_AND_MEAN
   #include <windows.h>
#endif

//nbowhay - when porting our old build TGE to TGEA I need this cause I was getting
//a link error not sure if there was another way to fix it or if this is ok
#pragma comment(lib, "version.lib")

using namespace std;

//--------------------------------------------------------------------------------
VersionInfo::VersionInfo (const char * const inFileName)
   : theVersionInfo (NULL)
   , theFixedInfo (NULL)
{
unsigned long aVersionInfoSize
   = GetFileVersionInfoSizeA ( const_cast<char*> (inFileName)
                            , &aVersionInfoSize);
if (aVersionInfoSize)
{
   theVersionInfo = new char [aVersionInfoSize];
   if (!GetFileVersionInfoA ( const_cast<char*> (inFileName)
                           , 0
                           , aVersionInfoSize
                           , theVersionInfo
                           ) )
   {
      throw runtime_error ("VersionInfo: can not retrieve version information");
   } // endif
   unsigned int aSize = 0;
   if (!VerQueryValueA( theVersionInfo
                     , "\\"
                     , &theFixedInfo
                     , &aSize))
   {
      throw runtime_error ("VersionInfo: can not retrieve version information");
   } // endif
} // endif
} // end constructor


//--------------------------------------------------------------------------------
VersionInfo::~VersionInfo ()
{
delete theVersionInfo;
} // end destructor


//--------------------------------------------------------------------------------
unsigned short VersionInfo::majorVersion () const
{
if (!theFixedInfo) return 0;
VS_FIXEDFILEINFO * aInfo = (VS_FIXEDFILEINFO *) theFixedInfo;
return (aInfo -> dwFileVersionMS >> 16);
} // end VersionInfo::majorVersion () const


//--------------------------------------------------------------------------------
unsigned short VersionInfo::minorVersion () const
{
if (!theFixedInfo) return 0;
VS_FIXEDFILEINFO * aInfo = (VS_FIXEDFILEINFO *) theFixedInfo;
return (unsigned short)(aInfo -> dwFileVersionMS);
} // end VersionInfo::minorVersion () const


//--------------------------------------------------------------------------------
unsigned short VersionInfo::build () const
{
if (!theFixedInfo) return 0;
VS_FIXEDFILEINFO * aInfo = (VS_FIXEDFILEINFO *) theFixedInfo;
return (aInfo -> dwFileVersionLS >> 16);
} // end VersionInfo::build () const


//--------------------------------------------------------------------------------
unsigned short VersionInfo::subBuild () const
{
if (!theFixedInfo) return 0;
VS_FIXEDFILEINFO * aInfo = (VS_FIXEDFILEINFO *) theFixedInfo;
return (unsigned short)(aInfo -> dwFileVersionLS);
} // end VersionInfo::subBuild () const


//--------------------------------------------------------------------------------
unsigned short VersionInfo::productMajorVersion () const
{
if (!theFixedInfo) return 0;
VS_FIXEDFILEINFO * aInfo = (VS_FIXEDFILEINFO *) theFixedInfo;
return (aInfo -> dwProductVersionMS >> 16);
} // end VersionInfo::productMajorVersion () const


//--------------------------------------------------------------------------------
unsigned short VersionInfo::productMinorVersion () const
{
if (!theFixedInfo) return 0;
VS_FIXEDFILEINFO * aInfo = (VS_FIXEDFILEINFO *) theFixedInfo;
return (unsigned short)(aInfo -> dwProductVersionMS);
} // end VersionInfo::productMinorVersion () const


//--------------------------------------------------------------------------------
unsigned short VersionInfo::productBuild () const
{
if (!theFixedInfo) return 0;
VS_FIXEDFILEINFO * aInfo = (VS_FIXEDFILEINFO *) theFixedInfo;
return (aInfo -> dwProductVersionLS >> 16);
} // end VersionInfo::productBuild () const


//--------------------------------------------------------------------------------
unsigned short VersionInfo::productSubBuild () const
{
if (!theFixedInfo) return 1;
VS_FIXEDFILEINFO * aInfo = (VS_FIXEDFILEINFO *) theFixedInfo;
return (unsigned short)(aInfo -> dwProductVersionLS);
} // end VersionInfo::productSubBuild () const


