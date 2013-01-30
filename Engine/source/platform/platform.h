//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include <stdlib.h>

#ifndef _TORQUECONFIG_H_
#include "torqueConfig.h"
#endif
#ifndef _TORQUE_TYPES_H_
#include "platform/types.h"
#endif
#ifndef _PLATFORMASSERT_H_
#include "platform/platformAssert.h"
#endif
#ifndef _MSGBOX_H_
#include "platform/nativeDialogs/msgBox.h"
#endif
#ifndef _VERSION_H_
#include "app/version.h"
#endif
#ifndef _TORQUE_STRING_H_
#include "core/util/str.h"
#endif
#ifndef _TORQUE_SAFEDELETE_H_
#include "core/util/safeDelete.h"
#endif

#include <new>
#include <typeinfo>

/// Global processor identifiers.
///
/// @note These enums must be globally scoped so that they work with the inline assembly
enum ProcessorType
{
   // x86
   CPU_X86Compatible,
   CPU_Intel_Unknown,
   CPU_Intel_486,
   CPU_Intel_Pentium,
   CPU_Intel_PentiumMMX,
   CPU_Intel_PentiumPro,
   CPU_Intel_PentiumII,
   CPU_Intel_PentiumCeleron,
   CPU_Intel_PentiumIII,
   CPU_Intel_Pentium4,
   CPU_Intel_PentiumM,
   CPU_Intel_Core,
   CPU_Intel_Core2,
   CPU_Intel_Corei7Xeon, // Core i7 or Xeon
   CPU_AMD_K6,
   CPU_AMD_K6_2,
   CPU_AMD_K6_3,
   CPU_AMD_Athlon,
   CPU_AMD_Unknown,
   CPU_Cyrix_6x86,
   CPU_Cyrix_MediaGX,
   CPU_Cyrix_6x86MX,
   CPU_Cyrix_GXm,          ///< Media GX w/ MMX
   CPU_Cyrix_Unknown,

   // PowerPC
   CPU_PowerPC_Unknown,
   CPU_PowerPC_601,
   CPU_PowerPC_603,
   CPU_PowerPC_603e,
   CPU_PowerPC_603ev,
   CPU_PowerPC_604,
   CPU_PowerPC_604e,
   CPU_PowerPC_604ev,
   CPU_PowerPC_G3,
   CPU_PowerPC_G4,
   CPU_PowerPC_G4_7450,
   CPU_PowerPC_G4_7455,
   CPU_PowerPC_G4_7447, 
   CPU_PowerPC_G5,

   // Xenon
   CPU_Xenon,

};

/// Properties for CPU.
enum ProcessorProperties
{ 
   CPU_PROP_C         = (1<<0),  ///< We should use C fallback math functions.
   CPU_PROP_FPU       = (1<<1),  ///< Has an FPU. (It better!)
   CPU_PROP_MMX       = (1<<2),  ///< Supports MMX instruction set extension.
   CPU_PROP_3DNOW     = (1<<3),  ///< Supports AMD 3dNow! instruction set extension.
   CPU_PROP_SSE       = (1<<4),  ///< Supports SSE instruction set extension.
   CPU_PROP_RDTSC     = (1<<5),  ///< Supports Read Time Stamp Counter op.
   CPU_PROP_SSE2      = (1<<6),  ///< Supports SSE2 instruction set extension.
   CPU_PROP_SSE3      = (1<<7),  ///< Supports SSE3 instruction set extension.  
   CPU_PROP_SSE3xt    = (1<<8),  ///< Supports extended SSE3 instruction set  
   CPU_PROP_SSE4_1    = (1<<9),  ///< Supports SSE4_1 instruction set extension.  
   CPU_PROP_SSE4_2    = (1<<10), ///< Supports SSE4_2 instruction set extension.  
   CPU_PROP_MP        = (1<<11), ///< This is a multi-processor system.
   CPU_PROP_LE        = (1<<12), ///< This processor is LITTLE ENDIAN.  
   CPU_PROP_64bit     = (1<<13), ///< This processor is 64-bit capable
   CPU_PROP_ALTIVEC   = (1<<14),  ///< Supports AltiVec instruction set extension (PPC only).
};

/// Processor info manager. 
struct Processor
{
   /// Gather processor state information.
   static void init();
};

#if defined(TORQUE_SUPPORTS_GCC_INLINE_X86_ASM)
#define TORQUE_DEBUGBREAK() { asm ( "int 3"); }
#elif defined (TORQUE_SUPPORTS_VC_INLINE_X86_ASM) // put this test second so that the __asm syntax doesn't break the Visual Studio Intellisense parser
#define TORQUE_DEBUGBREAK() { __asm { int 3 }; } 
#else
/// Macro to do in-line debug breaks, used for asserts.  Does inline assembly when possible.
#define TORQUE_DEBUGBREAK() Platform::debugBreak();
#endif

/// Physical type of a drive.
enum DriveType
{
   DRIVETYPE_FIXED = 0,       ///< Non-removable fixed drive.
   DRIVETYPE_REMOVABLE = 1,   ///< Removable drive.
   DRIVETYPE_REMOTE = 2,      ///< Networked/remote drive.
   DRIVETYPE_CDROM = 3,       ///< CD-Rom.
   DRIVETYPE_RAMDISK = 4,     ///< A ramdisk!
   DRIVETYPE_UNKNOWN = 5      ///< Don't know.
};

// Some forward declares for later.
class Point2I;
template<class T> class Vector;
template<typename Signature> class Signal;
struct InputEventInfo;

namespace Platform
{
   // Time
   struct LocalTime
   {
      U8  sec;        ///< Seconds after minute (0-59)
      U8  min;        ///< Minutes after hour (0-59)
      U8  hour;       ///< Hours after midnight (0-23)
      U8  month;      ///< Month (0-11; 0=january)
      U8  monthday;   ///< Day of the month (1-31)
      U8  weekday;    ///< Day of the week (0-6, 6=sunday)
      U16 year;       ///< Current year minus 1900
      U16 yearday;    ///< Day of year (0-365)
      bool isdst;     ///< True if daylight savings time is active
   };

   void getLocalTime(LocalTime &);
   
   /// Converts the local time to a formatted string appropriate
   /// for the current platform.
   String localTimeToString( const LocalTime &lt );
   
   U32  getTime();
   U32  getVirtualMilliseconds();

   /// Returns the milliseconds since the system was started.  You should
   /// not depend on this for high precision timing.
   /// @see PlatformTimer
   U32 getRealMilliseconds();

   void advanceTime(U32 delta);
   S32 getBackgroundSleepTime();

   // Platform control
   void init();
   void initConsole();
   void shutdown();
   void process();

   // Math control state
   U32 getMathControlState();
   void setMathControlState(U32 state);
   void setMathControlStateKnown();
   
   // Process control
   void sleep(U32 ms);
   bool excludeOtherInstances(const char *string);
   bool checkOtherInstances(const char *string);
   void restartInstance();
   void postQuitMessage(const U32 in_quitVal);
   void forceShutdown(S32 returnValue);

   // Debug
   void outputDebugString(const char *string, ...);
   void debugBreak();
   
   // Random
   float getRandom();
   
   // Window state
   void setWindowLocked(bool locked);
   void minimizeWindow();
   //const Point2I &getWindowSize();
   void setWindowSize( U32 newWidth, U32 newHeight, bool fullScreen );
   void closeWindow();

   // File stuff
   bool doCDCheck();
   StringTableEntry createPlatformFriendlyFilename(const char *filename);
   struct FileInfo
   {
      const char* pFullPath;
      const char* pFileName;
      U32 fileSize;
   };
   bool cdFileExists(const char *filePath, const char *volumeName, S32 serialNum);
   void fileToLocalTime(const FileTime &ft, LocalTime *lt);
   /// compare file times returns < 0 if a is earlier than b, >0 if b is earlier than a
   S32 compareFileTimes(const FileTime &a, const FileTime &b);
   bool stringToFileTime(const char * string, FileTime * time);
   bool fileTimeToString(FileTime * time, char * string, U32 strLen);

   /// Compares the last modified time between two file paths.  Returns < 0 if
   /// the first file is earlier than the second, > 0 if the second file is earlier
   /// than the first, and 0 if the files are equal.
   ///
   /// If either of the files doesn't exist it returns -1.
   S32 compareModifiedTimes( const char *firstPath, const char *secondPath );

   // Directory functions.  Dump path returns false iff the directory cannot be
   //  opened.
   
   StringTableEntry getCurrentDirectory();
   bool             setCurrentDirectory(StringTableEntry newDir);

   StringTableEntry getTemporaryDirectory();
   StringTableEntry getTemporaryFileName();

   /// Returns the filename of the torque executable.
   /// On Win32, this is the .exe file.
   /// On Mac, this is the .app/ directory bundle.
   StringTableEntry getExecutableName();
   /// Returns full pathname of the torque executable without filename
   StringTableEntry getExecutablePath();
   
   /// Returns the full path to the directory that contains main.cs.
   /// Tools scripts are validated as such if they are in this directory or a
   /// subdirectory of this directory.
   StringTableEntry getMainDotCsDir();

   /// Set main.cs directory. Used in runEntryScript()
   void setMainDotCsDir(const char *dir);

   StringTableEntry getPrefsPath(const char *file = NULL);

   char *makeFullPathName(const char *path, char *buffer, U32 size, const char *cwd = NULL);
   StringTableEntry stripBasePath(const char *path);
   bool isFullPath(const char *path);
   StringTableEntry makeRelativePathName(const char *path, const char *to);

   String stripExtension( String fileName, Vector< String >& validExtensions );

   bool dumpPath(const char *in_pBasePath, Vector<FileInfo>& out_rFileVector, S32 recurseDepth = -1);
   bool dumpDirectories( const char *path, Vector<StringTableEntry> &directoryVector, S32 depth = 0, bool noBasePath = false );
   bool hasSubDirectory( const char *pPath );
   bool getFileTimes(const char *filePath, FileTime *createTime, FileTime *modifyTime);
   bool isFile(const char *pFilePath);
   S32  getFileSize(const char *pFilePath);
   bool isDirectory(const char *pDirPath);
   bool isSubDirectory(const char *pParent, const char *pDir);

   void addExcludedDirectory(const char *pDir);
   void clearExcludedDirectories();
   bool isExcludedDirectory(const char *pDir);

   /// Given a directory path, create all necessary directories for that path to exist.
   bool createPath(const char *path); // create a directory path

   // Alerts
   void AlertOK(const char *windowTitle, const char *message);
   bool AlertOKCancel(const char *windowTitle, const char *message);
   bool AlertRetry(const char *windowTitle, const char *message);

   // Volumes
   struct VolumeInformation
   {
      StringTableEntry  RootPath;
      StringTableEntry  Name;
      StringTableEntry  FileSystem;
      U32               SerialNumber;
      U32               Type;
      bool              ReadOnly;
   };
   extern struct VolumeInformation  *PVolumeInformation;

   // Volume functions.
   void getVolumeNamesList( Vector<const char*>& out_rNameVector, bool bOnlyFixedDrives = false );
   void getVolumeInformationList( Vector<VolumeInformation>& out_rVolumeInfoVector, bool bOnlyFixedDrives = false );

   struct SystemInfo_struct
   {
         struct Processor
         {
            ProcessorType  type;
            const char*    name;
            U32            mhz;
            bool           isMultiCore;
            bool           isHyperThreaded;
            U32            numLogicalProcessors;
            U32            numPhysicalProcessors;
            U32            numAvailableCores;
            U32            properties;      // CPU type specific enum
         } processor;
   };
   extern Signal<void(void)> SystemInfoReady;
   extern SystemInfo_struct  SystemInfo;

   // Web page launch function:
   bool openWebBrowser( const char* webAddress );

   // display Splash Window
   bool displaySplashWindow( String path );

   void openFolder( const char* path );

   // Open file at the OS level, according to registered file-types.
   void openFile( const char* path );

   const char* getLoginPassword();
   bool setLoginPassword( const char* password );

   const char* getClipboard();
   bool setClipboard(const char *text);

   // User Specific Functions
   StringTableEntry getUserHomeDirectory();
   StringTableEntry getUserDataDirectory();
   bool getUserIsAdministrator();
   
   // Displays a fancy platform specific message box
   S32 messageBox(const UTF8 *title, const UTF8 *message, MBButtons buttons = MBOkCancel, MBIcons icon = MIInformation);
   
   /// Description of a keyboard input we want to ignore.
   struct KeyboardInputExclusion
   {
      KeyboardInputExclusion()
      {
         key = 0;
         orModifierMask = 0;
         andModifierMask = 0;
      }

      /// The key code to ignore, e.g. KEY_TAB. If this and the other
      /// conditions match, ignore the key.
      S32 key;

      /// if(modifiers | orModifierMask) and the other conditions match,
      /// ignore the key.
      U32 orModifierMask;

      /// if((modifiers & andModifierMask) == andModifierMask) and the
      /// other conditions match, ignore the key stroke.
      U32 andModifierMask;

      /// Based on the values above, determine if a given input event
      /// matchs this exclusion rule.
      const bool checkAgainstInput(const InputEventInfo *info) const;
   };

   /// Reset the keyboard input exclusion list.
   void clearKeyboardInputExclusion();
   
   /// Add a new keyboard exclusion.
   void addKeyboardInputExclusion(const KeyboardInputExclusion &kie);

   /// Check if a given input event should be excluded.
   const bool checkKeyboardInputExclusion(const InputEventInfo *info);
	
   
   /// Set/Get whether this is a web deployment 
	bool getWebDeployment();
   void setWebDeployment(bool v);
   
};

//------------------------------------------------------------------------------
// Unicode string conversions
// UNICODE is a windows platform API switching flag. Don't define it on other platforms.
#ifdef UNICODE
#define dT(s)    L##s
#else
#define dT(s)    s
#endif

//------------------------------------------------------------------------------
// Misc StdLib functions
#define QSORT_CALLBACK FN_CDECL
inline void dQsort(void *base, U32 nelem, U32 width, int (QSORT_CALLBACK *fcmp)(const void *, const void *))
{
   qsort(base, nelem, width, fcmp);
}

//-------------------------------------- Some all-around useful inlines and globals
//

///@defgroup ObjTrickery Object Management Trickery
///
/// These functions are to construct and destruct objects in memory
/// without causing a free or malloc call to occur. This is so that
/// we don't have to worry about allocating, say, space for a hundred
/// NetAddresses with a single malloc call, calling delete on a single
/// NetAdress, and having it try to free memory out from under us.
///
/// @{

/// Constructs an object that already has memory allocated for it.
template <class T>
inline T* constructInPlace(T* p)
{
   return new ( p ) T;
}
template< class T >
inline T* constructArrayInPlace( T* p, U32 num )
{
   return new ( p ) T[ num ];
}

/// Copy constructs an object that already has memory allocated for it.
template <class T>
inline T* constructInPlace(T* p, const T* copy)
{
   return new ( p ) T( *copy );
}

template <class T, class T2> inline T* constructInPlace(T* ptr, T2 t2)
{
   return new ( ptr ) T( t2 );
}

template <class T, class T2, class T3> inline T* constructInPlace(T* ptr, T2 t2, T3 t3)
{
   return new ( ptr ) T( t2, t3 );
}

template <class T, class T2, class T3, class T4> inline T* constructInPlace(T* ptr, T2 t2, T3 t3, T4 t4)
{
   return new ( ptr ) T( t2, t3, t4 );
}

template <class T, class T2, class T3, class T4, class T5> inline T* constructInPlace(T* ptr, T2 t2, T3 t3, T4 t4, T5 t5)
{
   return new ( ptr ) T( t2, t3, t4, t5 );
}

/// Destructs an object without freeing the memory associated with it.
template <class T>
inline void destructInPlace(T* p)
{
   p->~T();
}


//------------------------------------------------------------------------------
/// Memory functions

#if !defined(TORQUE_DISABLE_MEMORY_MANAGER)
#  define TORQUE_TMM_ARGS_DECL   , const char* fileName, const U32 lineNum
#  define TORQUE_TMM_ARGS        , fileName, lineNum
#  define TORQUE_TMM_LOC         , __FILE__, __LINE__
   extern void* FN_CDECL operator new(dsize_t size, const char*, const U32);
   extern void* FN_CDECL operator new[](dsize_t size, const char*, const U32);
   extern void  FN_CDECL operator delete(void* ptr);
   extern void  FN_CDECL operator delete[](void* ptr);
#  define _new new(__FILE__, __LINE__)
#  define new  _new
#else
#  define TORQUE_TMM_ARGS_DECL
#  define TORQUE_TMM_ARGS
#  define TORQUE_TMM_LOC
#endif

#define dMalloc(x) dMalloc_r(x, __FILE__, __LINE__)
#define dRealloc(x, y) dRealloc_r(x, y, __FILE__, __LINE__)

extern void  setBreakAlloc(dsize_t);
extern void  setMinimumAllocUnit(U32);
extern void* dMalloc_r(dsize_t in_size, const char*, const dsize_t);
extern void  dFree(void* in_pFree);
extern void* dRealloc_r(void* in_pResize, dsize_t in_size, const char*, const dsize_t);
extern void* dRealMalloc(dsize_t);
extern void  dRealFree(void*);

extern void *dMalloc_aligned(dsize_t in_size, int alignment);
extern void dFree_aligned(void *);


inline void dFree( const void* p )
{
   dFree( ( void* ) p );
}

// Helper function to copy one array into another of different type
template<class T,class S> void dCopyArray(T *dst, const S *src, dsize_t size)
{
   for (dsize_t i = 0; i < size; i++)
      dst[i] = (T)src[i];
}

extern void* dMemcpy(void *dst, const void *src, dsize_t size);
extern void* dMemmove(void *dst, const void *src, dsize_t size);
extern void* dMemset(void *dst, int c, dsize_t size);
extern int   dMemcmp(const void *ptr1, const void *ptr2, dsize_t size);

// Special case of the above function when the arrays are the same type (use memcpy)
template<class T> void dCopyArray(T *dst, const T *src, dsize_t size)
{
   dMemcpy(dst, src, size * sizeof(T));
}

/// The dALIGN macro ensures the passed declaration is
/// data aligned at 16 byte boundaries.
#if defined( TORQUE_COMPILER_VISUALC )
   #define dALIGN( decl ) __declspec( align( 16 ) ) decl
   #define dALIGN_BEGIN __declspec( align( 16 ) )
   #define dALIGN_END
#elif defined( TORQUE_COMPILER_GCC )
   #define dALIGN( decl ) decl __attribute__( ( aligned( 16 ) ) )
   #define dALIGN_BEGIN
   #define dALIGN_END __attribute__( ( aligned( 16 ) ) )
#else
   #define dALIGN( decl ) decl
   #define dALIGN_BEGIN()
   #define dALIGN_END()
#endif

//------------------------------------------------------------------------------
// FileIO functions
extern bool dFileDelete(const char *name);
extern bool dFileRename(const char *oldName, const char *newName);
extern bool dFileTouch(const char *name);
extern bool dPathCopy(const char *fromName, const char *toName, bool nooverwrite = true);

typedef void* FILE_HANDLE;
enum DFILE_STATUS
{
   DFILE_OK = 1
};

extern FILE_HANDLE dOpenFileRead(const char *name, DFILE_STATUS &error);
extern FILE_HANDLE dOpenFileReadWrite(const char *name, bool append, DFILE_STATUS &error);
extern int dFileRead(FILE_HANDLE handle, U32 bytes, char *dst, DFILE_STATUS &error);
extern int dFileWrite(FILE_HANDLE handle, U32 bytes, const char *dst, DFILE_STATUS &error);
extern void dFileClose(FILE_HANDLE handle);

extern StringTableEntry osGetTemporaryDirectory();

//------------------------------------------------------------------------------
struct Math
{
   /// Initialize the math library with the appropriate libraries
   /// to support hardware acceleration features.
   ///
   /// @param properties Leave zero to detect available hardware. Otherwise,
   ///                   pass CPU instruction set flags that you want to load
   ///                   support for.
   static void init(U32 properties = 0);
};

/// @}

#endif


