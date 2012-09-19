//--------------------------------------------------------------------------
// 
// 
// 
//--------------------------------------------------------------------------


#ifndef _H_JCONFIG_
#define _H_JCONFIG_


#if (defined(__MWERKS__) && !defined(macintosh))
   #include "jconfig.cw.win.h"
   #define JCONFIG_INCLUDED
#endif

#if (defined(__MWERKS__) && defined(macintosh)) || defined(__APPLE__)
   #include "jconfig.cw.mac.h"
   #define JCONFIG_INCLUDED
#endif

#if (defined(_MSC_VER) && !defined(__MWERKS__) && !defined(macintosh))
   #include "jconfig.vc.win.h"
   #define JCONFIG_INCLUDED
#endif

#if (( __GNUC__ >= 2 ) && (defined (__CYGWIN32__) || defined (__linux__) || defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__WIN32__) || defined(SN_TARGET_PS3)) )
   #include "jconfig.gcc.linux.h"
   #define JCONFIG_INCLUDED
#endif


#ifndef JCONFIG_INCLUDED
#error No jconfig.h file was included!
#endif

#undef JCONFIG_INCLUDED

#endif  // _H_JCONFIG_
