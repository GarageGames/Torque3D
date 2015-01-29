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
#define _VERSINFO_

class VersionInfo
{
public:

   VersionInfo (const char * const inFilename );
   ~VersionInfo();

   bool hasInfo () const
          {return theVersionInfo!=0 ;}

   unsigned short majorVersion () const;
   unsigned short minorVersion () const;
   unsigned short build () const;
   unsigned short subBuild () const;

   unsigned short productMajorVersion () const;
   unsigned short productMinorVersion () const;
   unsigned short productBuild () const;
   unsigned short productSubBuild () const;

   unsigned long fileFlagsMask() const;
   unsigned long fileFlags() const;
   unsigned long fileOS() const;
   unsigned long fileType() const;
   unsigned long fileSubType() const;

protected:
private:

   void * theVersionInfo;
   void * theFixedInfo;
}; // end VersionInfo class declaration

#endif
