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

#include "core/strings/stringFunctions.h"
#include "util/tempAlloc.h"
#include "console/console.h"
#include "core/stringTable.h"

//-----------------------------------------------------------------------------

StringTableEntry Platform::getTemporaryDirectory()
{
   StringTableEntry path = osGetTemporaryDirectory();

   if(! Platform::isDirectory(path))
      path = Platform::getCurrentDirectory();

   return path;
}

ConsoleFunction(getTemporaryDirectory, const char *, 1, 1, "()"
				"@brief Returns the OS temporary directory, \"C:/Users/Mich/AppData/Local/Temp\" for example\n\n"
				"@note This can be useful to adhering to OS standards and practices, "
				"but not really used in Torque 3D right now.\n"
				"@note Be very careful when getting into OS level File I/O."
				"@return String containing path to OS temp directory\n"
				"@note This is legacy function brought over from TGB, and does not appear "
				"to have much use. Possibly deprecate?\n"
				"@ingroup FileSystem\n"
				"@internal")
{
   return Platform::getTemporaryDirectory();
}

StringTableEntry Platform::getTemporaryFileName()
{
   char buf[512];
   StringTableEntry path = Platform::getTemporaryDirectory();

   dSprintf(buf, sizeof(buf), "%s/tgb.%08x.%02x.tmp", path, Platform::getRealMilliseconds(), U32(Platform::getRandom() * 255));

   // [tom, 9/7/2006] This shouldn't be needed, but just in case
   if(Platform::isFile(buf))
      return Platform::getTemporaryFileName();

   return StringTable->insert(buf);
}

ConsoleFunction(getTemporaryFileName, const char *, 1, 1, "()"
				"@brief Creates a name and extension for a potential temporary file\n\n"
				"This does not create the actual file. It simply creates a random name "
				"for a file that does not exist.\n\n"
				"@note This is legacy function brought over from TGB, and does not appear "
				"to have much use. Possibly deprecate?\n"
				"@ingroup FileSystem\n"
				"@internal")
{
   return Platform::getTemporaryFileName();
}

//-----------------------------------------------------------------------------

static StringTableEntry sgMainCSDir = NULL;

StringTableEntry Platform::getMainDotCsDir()
{
   if(sgMainCSDir == NULL)
      sgMainCSDir = Platform::getExecutablePath();

   return sgMainCSDir;
}

void Platform::setMainDotCsDir(const char *dir)
{
   sgMainCSDir = StringTable->insert(dir);
}

//-----------------------------------------------------------------------------

typedef Vector<char*> CharVector;
static CharVector gPlatformDirectoryExcludeList( __FILE__, __LINE__ );

void Platform::addExcludedDirectory(const char *pDir)
{
   gPlatformDirectoryExcludeList.push_back(dStrdup(pDir));
}

void Platform::clearExcludedDirectories()
{
   while(gPlatformDirectoryExcludeList.size())
   {
      dFree(gPlatformDirectoryExcludeList.last());
      gPlatformDirectoryExcludeList.pop_back();
   }
}

bool Platform::isExcludedDirectory(const char *pDir)
{
   for(CharVector::iterator i=gPlatformDirectoryExcludeList.begin(); i!=gPlatformDirectoryExcludeList.end(); i++)
      if(!dStrcmp(pDir, *i))
         return true;

   return false;
}

//-----------------------------------------------------------------------------

inline void catPath(char *dst, const char *src, U32 len)
{
   if(*dst != '/')
   {
      ++dst; --len;
      *dst = '/';
   }

   ++dst; --len;

   dStrncpy(dst, src, len);
   dst[len - 1] = 0;
}

// converts the posix root path "/" to "c:/" for win32
// FIXME: this is not ideal. the c: drive is not guaranteed to exist.
#if defined(TORQUE_OS_WIN32)
static inline void _resolveLeadingSlash(char* buf, U32 size)
{
   if(buf[0] != '/')
      return;

   AssertFatal(dStrlen(buf) + 2 < size, "Expanded path would be too long");
   dMemmove(buf + 2, buf, dStrlen(buf));
   buf[0] = 'c';
   buf[1] = ':';
}
#endif

static void makeCleanPathInPlace( char* path )
{
   U32 pathDepth = 0;
   char* fromPtr = path;
   char* toPtr = path;

   bool isAbsolute = false;
   if( *fromPtr == '/' )
   {
      fromPtr ++;
      toPtr ++;
      isAbsolute = true;
   }
   else if( fromPtr[ 0 ] != '\0' && fromPtr[ 1 ] == ':' )
   {
      toPtr += 3;
      fromPtr += 3;
      isAbsolute = true;
   }

   while( *fromPtr )
   {
      if( fromPtr[ 0 ] == '.' && fromPtr[ 1 ] == '.' && fromPtr[ 2 ] == '/' )
      {
         // Back up from '../'

         if( pathDepth > 0 )
         {
            pathDepth --;
            toPtr -= 2;
            while( toPtr >= path && *toPtr != '/' )
               toPtr --;
            toPtr ++;
         }
         else if( !isAbsolute )
         {
            dMemcpy( toPtr, fromPtr, 3 );
            toPtr += 3;
         }

         fromPtr += 3;
      }
      else if( fromPtr[ 0 ] == '.' && fromPtr[ 1 ] == '/' )
      {
         // Ignore.
         fromPtr += 2;
      }
      else
      {
         if( fromPtr[ 0 ] == '/' )
            pathDepth ++;

         *toPtr ++ = *fromPtr ++;
      }
   }

   *toPtr = '\0';
}

char * Platform::makeFullPathName(const char *path, char *buffer, U32 size, const char *cwd /* = NULL */)
{
   char bspath[1024];
   dStrncpy(bspath, path, sizeof(bspath));
   bspath[sizeof(bspath)-1] = 0;
   
   for(S32 i = 0;i < dStrlen(bspath);++i)
   {
      if(bspath[i] == '\\')
         bspath[i] = '/';
   }

   if(Platform::isFullPath(bspath))
   {
      // Already a full path
      #if defined(TORQUE_OS_WIN32)
         _resolveLeadingSlash(bspath, sizeof(bspath));
      #endif
      dStrncpy(buffer, bspath, size);
      buffer[size-1] = 0;
      return buffer;
   }

   // [rene, 05/05/2008] Based on overall file handling in Torque, it does not seem to make
   //    that much sense to me to base things off the current working directory here.

   if(cwd == NULL)
      cwd = Con::isCurrentScriptToolScript() ? Platform::getMainDotCsDir() : Platform::getCurrentDirectory();

   dStrncpy(buffer, cwd, size);
   buffer[size-1] = 0;

   const char* defaultDir = Con::getVariable("defaultGame");

   char *ptr = bspath;
   char *slash = NULL;
   char *endptr = buffer + dStrlen(buffer) - 1;

   do
   {
      slash = dStrchr(ptr, '/');
      if(slash)
      {
         *slash = 0;

         // Directory

         if(dStrcmp(ptr, "..") == 0)
         {
            // Parent
            endptr = dStrrchr(buffer, '/');
            if (endptr)
               *endptr-- = 0;
         }
         else if(dStrcmp(ptr, ".") == 0)
         {
            // Current dir
         }
         else if(dStrcmp(ptr, "~") == 0)
         {
            catPath(endptr, defaultDir, size - (endptr - buffer));
            endptr += dStrlen(endptr) - 1;
         }
         else if(endptr)
         {
            catPath(endptr, ptr, size - (endptr - buffer));
            endptr += dStrlen(endptr) - 1;
         }
         
         ptr = slash + 1;
      }
      else if(endptr)
      {
         // File

         catPath(endptr, ptr, size - (endptr - buffer));
         endptr += dStrlen(endptr) - 1;
      }

   } while(slash);

   return buffer;
}

bool Platform::isFullPath(const char *path)
{
   // Quick way out
   if(path[0] == '/' || path[1] == ':')
      return true;

   return false;
}

//-----------------------------------------------------------------------------

/// Return "fileName" stripped of its extension.  Only extensions contained
/// in "validExtensions" will be stripped from the filename.
///
/// @note Extensions in "validExtension" should include the dot.
String Platform::stripExtension( String fileName, Vector< String >& validExtensions )
{
   // See if we have a valid extension to strip off
   String ext;
   S32 dotPos = fileName.find( '.', 0, String::Right );
   if( dotPos != String::NPos )
      ext = fileName.substr( dotPos );

   U32 numValidExt = validExtensions.size();
   if( ext.isNotEmpty() && numValidExt )
   {
      bool validExt = false;
      for( U32 i = 0; i < numValidExt; i++ )
      {
         if( ext.equal( validExtensions[ i ], String::NoCase ) )
         {
            validExt = true;
            break;
         }
      }

      if( !validExt )
         ext = String::EmptyString;
   }

   if( ext.isEmpty() )
      return fileName;
   else
      return fileName.substr( 0, fileName.length() - ext.length() );
}

//-----------------------------------------------------------------------------
// TODO: wow really shouldn't be adding everything to the string table, use the string class!
StringTableEntry Platform::makeRelativePathName(const char *path, const char *to)
{
   // Make sure 'to' is a proper absolute path terminated with a forward slash.

   char buffer[ 2048 ];
   if( !to )
   {
      dSprintf( buffer, sizeof( buffer ), "%s/", Platform::getMainDotCsDir() );
      to = buffer;
   }
   else if( !Platform::isFullPath( to ) )
   {
      dSprintf( buffer, sizeof( buffer ), "%s/%s/", Platform::getMainDotCsDir(), to );
      makeCleanPathInPlace( buffer );
      to = buffer;
   }
   else if( to[ dStrlen( to ) - 1 ] != '/' )
   {
      U32 length = getMin( (U32)dStrlen( to ), sizeof( buffer ) - 2 );
      dMemcpy( buffer, to, length );
      buffer[ length ] = '/';
      buffer[ length + 1 ] = '\0';
      to = buffer;
   }

   // If 'path' isn't absolute, make it now.  Let's us use a single
   // absolute/absolute merge path from here on.

   char buffer2[ 1024 ];
   if( !Platform::isFullPath( path ) )
   {
      dSprintf( buffer2, sizeof( buffer2 ), "%s/%s", Platform::getMainDotCsDir(), path );
      makeCleanPathInPlace( buffer2 );
      path = buffer2;
   }

   // First, find the common prefix and see where 'path' branches off from 'to'.

   const char *pathPtr, *toPtr, *branch = path;
   for(pathPtr = path, toPtr = to;*pathPtr && *toPtr && dTolower(*pathPtr) == dTolower(*toPtr);++pathPtr, ++toPtr)
   {
      if(*pathPtr == '/')
         branch = pathPtr;
   }

   // If there's no common part, the two paths are on different drives and
   // there's nothing we can do.

   if( pathPtr == path )
      return StringTable->insert( path );

   // If 'path' and 'to' are identical (minus trailing slash or so), we can just return './'.

   else if((*pathPtr == 0 || (*pathPtr == '/' && *(pathPtr + 1) == 0)) &&
      (*toPtr == 0 || (*toPtr == '/' && *(toPtr + 1) == 0)))
   {
      char* bufPtr = buffer;
      *bufPtr ++ = '.';

      if(*pathPtr == '/' || *(pathPtr - 1) == '/')
         *bufPtr++ = '/';

      *bufPtr = 0;
      return StringTable->insert(buffer);
   }

   // If 'to' is a proper prefix of 'path', the remainder of 'path' is our relative path.

   else if( *toPtr == '\0' && toPtr[ -1 ] == '/' )
      return StringTable->insert( pathPtr );

   // Otherwise have to step up the remaining directories in 'to' and then
   // append the remainder of 'path'.

   else
   {
      if((*pathPtr == 0 && *toPtr == '/') || (*toPtr == '/' && *pathPtr == 0))
         branch = pathPtr;

      // Allocate a new temp so we aren't prone to buffer overruns.

      TempAlloc< char > temp( dStrlen( toPtr ) + dStrlen( branch ) + 1 );
      char* bufPtr = temp;

      // Figure out parent dirs

      for(toPtr = to + (branch - path);*toPtr;++toPtr)
      {
         if(*toPtr == '/' && *(toPtr + 1) != 0)
         {
            *bufPtr++ = '.';
            *bufPtr++ = '.';
            *bufPtr++ = '/';
         }
      }
      *bufPtr = 0;

      // Copy the rest
      if(*branch)
         dStrcpy(bufPtr, branch + 1);
      else
         *--bufPtr = 0;

      return StringTable->insert( temp );
   }
}

//-----------------------------------------------------------------------------

static StringTableEntry tryStripBasePath(const char *path, const char *base)
{
   U32 len = dStrlen(base);
   if(dStrnicmp(path, base, len) == 0)
   {
      if(*(path + len) == '/') ++len;
      return StringTable->insert(path + len, true);
   }
   return NULL;
}

StringTableEntry Platform::stripBasePath(const char *path)
{
   if(path == NULL)
      return NULL;

   StringTableEntry str = tryStripBasePath(path, Platform::getMainDotCsDir());
   
   if(str != NULL )
      return str;

   str = tryStripBasePath(path, Platform::getCurrentDirectory());
   if(str != NULL )
      return str;

   str = tryStripBasePath(path, Platform::getPrefsPath());
   if(str != NULL )
      return str;

   return path;
}

//-----------------------------------------------------------------------------

StringTableEntry Platform::getPrefsPath(const char *file /* = NULL */)
{
#ifndef TORQUE2D_TOOLS_FIXME
   return StringTable->insert(file ? file : "");
#else
   char buf[1024];
   const char *company = Con::getVariable("$Game::CompanyName");
   if(company == NULL || *company == 0)
      company = "GarageGames";

   const char *appName = Con::getVariable("$Game::GameName");
   if(appName == NULL || *appName == 0)
      appName = TORQUE_APP_NAME;

   if(file)
   {
      if(dStrstr(file, ".."))
      {
         Con::errorf("getPrefsPath - filename cannot be relative");
         return NULL;
      }

      dSprintf(buf, sizeof(buf), "%s/%s/%s/%s", Platform::getUserDataDirectory(), company, appName, file);
   }
   else
      dSprintf(buf, sizeof(buf), "%s/%s/%s", Platform::getUserDataDirectory(), company, appName);

   return StringTable->insert(buf, true);
#endif
}

//-----------------------------------------------------------------------------

ConsoleFunction(getUserDataDirectory, const char*, 1, 1, "getUserDataDirectory()")
{
   return Platform::getUserDataDirectory();
}

ConsoleFunction(getUserHomeDirectory, const char*, 1, 1, "getUserHomeDirectory()")
{
   return Platform::getUserHomeDirectory();
}
