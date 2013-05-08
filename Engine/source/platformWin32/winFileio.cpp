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
#include "platformWin32/platformWin32.h"
#include "core/fileio.h"
#include "core/util/tVector.h"
#include "core/stringTable.h"
#include "console/console.h"
#include "core/strings/unicode.h"
#include "util/tempAlloc.h"
#include "core/util/safeDelete.h"
#include "core/volume.h"

// Microsoft VC++ has this POSIX header in the wrong directory
#if defined(TORQUE_COMPILER_VISUALC)
#include <sys/utime.h>
#elif defined (TORQUE_COMPILER_GCC)
#include <time.h>
#include <sys/utime.h>
#else
#include <utime.h>
#endif

StringTableEntry Platform::createPlatformFriendlyFilename( const char *filename )
{
   return StringTable->insert( filename );
}

//-----------------------------------------------------------------------------
bool dFileDelete(const char * name)
{
   AssertFatal( name != NULL, "dFileDelete - NULL file name" );

   TempAlloc< TCHAR > buf( dStrlen( name ) + 1 );

#ifdef UNICODE
   convertUTF8toUTF16( name, buf, buf.size );
#else
   dStrcpy( buf, name );
#endif

   backslash( buf );
   if( Platform::isFile( name ) )
      return DeleteFile( buf );
   else
      return RemoveDirectory( buf );
}

bool dFileRename(const char *oldName, const char *newName)
{
   AssertFatal( oldName != NULL && newName != NULL, "dFileRename - NULL file name" );

   TempAlloc< TCHAR > oldf( dStrlen( oldName ) + 1 );
   TempAlloc< TCHAR > newf( dStrlen( newName ) + 1 );

#ifdef UNICODE
   convertUTF8toUTF16( oldName, oldf, oldf.size );
   convertUTF8toUTF16( newName, newf, newf.size );
#else
   dStrcpy(oldf, oldName);
   dStrcpy(newf, newName);
#endif
   backslash(oldf);
   backslash(newf);

   return MoveFile( oldf, newf );
}

bool dFileTouch(const char * name)
{
   AssertFatal( name != NULL, "dFileTouch - NULL file name" );

   TempAlloc< TCHAR > buf( dStrlen( name ) + 1 );

#ifdef UNICODE
   convertUTF8toUTF16( name, buf, buf.size );
#else
   dStrcpy( buf, name );
#endif

   backslash( buf );
   FILETIME ftime;
   GetSystemTimeAsFileTime( &ftime );
   HANDLE handle = CreateFile( buf, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
      NULL, OPEN_EXISTING, 0, NULL );
   if( handle == INVALID_HANDLE_VALUE )
      return false;
   bool result = SetFileTime( handle, NULL, NULL, &ftime );
   CloseHandle( handle );

   return result;
};

bool dPathCopy(const char *fromName, const char *toName, bool nooverwrite)
{
   AssertFatal( fromName != NULL && toName != NULL, "dPathCopy - NULL file name" );

   TempAlloc< TCHAR > from( dStrlen( fromName ) + 1 );
   TempAlloc< TCHAR > to( dStrlen( toName ) + 1 );

#ifdef UNICODE
   convertUTF8toUTF16( fromName, from, from.size );
   convertUTF8toUTF16( toName, to, to.size );
#else
   dStrcpy( from, fromName );
   dStrcpy( to, toName );
#endif

   backslash( from );
   backslash( to );

   // Copy File
   if (Platform::isFile(fromName))
      return CopyFile( from, to, nooverwrite );
   // Copy Path
   else if (Platform::isDirectory(fromName))
   {
      // If the destination path exists and we don't want to overwrite, return.
      if ((Platform::isDirectory(toName) || Platform::isFile(toName)) && nooverwrite)
         return false;

      Vector<StringTableEntry> directoryInfo;
      Platform::dumpDirectories(fromName, directoryInfo, -1);

      Vector<Platform::FileInfo> fileInfo;
      Platform::dumpPath(fromName, fileInfo);

      Platform::clearExcludedDirectories();

      TempAlloc< char > tempBuf( to.size * 3 + MAX_PATH * 3 );

      // Create all the directories.
      for (S32 i = 0; i < directoryInfo.size(); i++)
      {
         const char* fromDir = directoryInfo[i];

         char* toDir = tempBuf;
         Platform::makeFullPathName(fromDir + dStrlen(fromName) + (dStricmp(fromDir, fromName) ? 1 : 0), tempBuf, tempBuf.size, toName);
         if(*(toDir + dStrlen(toDir) - 1) != '/')
            dStrcat(toDir, "/");
         forwardslash(toDir);

         if (!Platform::createPath(toDir))
         {
            //TODO: New directory should be deleted here.
            return false;
         }
      }

      TempAlloc< char > tempBuf1( from.size * 3 + MAX_PATH * 3 );
#ifdef UNICODE
      TempAlloc< WCHAR > wtempBuf( tempBuf.size / 3 );
      TempAlloc< WCHAR > wtempBuf1( tempBuf1.size / 3 );
#endif

      for (S32 i = 0; i < fileInfo.size(); i++)
      {
         char* fromFile = tempBuf1;
         dSprintf( tempBuf1, tempBuf1.size, "%s/%s", fileInfo[i].pFullPath, fileInfo[i].pFileName);

         char* toFile = tempBuf;
         Platform::makeFullPathName(fileInfo[i].pFullPath + dStrlen(fromName) + (dStricmp(fileInfo[i].pFullPath, fromName) ? 1 : 0), tempBuf, tempBuf.size, toName);
         dStrcat(toFile, "/");
         dStrcat(toFile, fileInfo[i].pFileName);

         backslash(fromFile);
         backslash(toFile);
         
#ifdef UNICODE
         convertUTF8toUTF16( tempBuf, wtempBuf, wtempBuf.size );
         convertUTF8toUTF16( tempBuf1, wtempBuf1, wtempBuf1.size );
         WCHAR* f = wtempBuf1;
         WCHAR* t = wtempBuf;
#else
         char *f = (char*)fromFile;
         char *t = (char*)toFile;
#endif

         if (!::CopyFile(f, t, nooverwrite))
         {
            // New directory should be deleted here.
            return false;
         }

      }

      return true;
   }

   return false;
}

//-----------------------------------------------------------------------------
// Constructors & Destructor
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// After construction, the currentStatus will be Closed and the capabilities
// will be 0.
//-----------------------------------------------------------------------------
File::File()
: currentStatus(Closed), capability(0)
{
    AssertFatal(sizeof(HANDLE) == sizeof(void *), "File::File: cannot cast void* to HANDLE");

    handle = (void *)INVALID_HANDLE_VALUE;
}

//-----------------------------------------------------------------------------
// insert a copy constructor here... (currently disabled)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
File::~File()
{
    close();
    handle = (void *)INVALID_HANDLE_VALUE;
}


//-----------------------------------------------------------------------------
// Open a file in the mode specified by openMode (Read, Write, or ReadWrite).
// Truncate the file if the mode is either Write or ReadWrite and truncate is
// true.
//
// Sets capability appropriate to the openMode.
// Returns the currentStatus of the file.
//-----------------------------------------------------------------------------
File::Status File::open(const char *filename, const AccessMode openMode)
{
   AssertFatal(NULL != filename, "File::open: NULL fname");
   AssertWarn(INVALID_HANDLE_VALUE == (HANDLE)handle, "File::open: handle already valid");

   TempAlloc< TCHAR > fname( dStrlen( filename ) + 1 );

#ifdef UNICODE
   convertUTF8toUTF16( filename, fname, fname.size );
#else
   dStrcpy(fname, filename);
#endif
   backslash( fname );

   // Close the file if it was already open...
   if (Closed != currentStatus)
      close();

   // create the appropriate type of file...
   switch (openMode)
   {
   case Read:
      handle = (void *)CreateFile(fname,
         GENERIC_READ,
         FILE_SHARE_READ,
         NULL,
         OPEN_EXISTING,
         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
         NULL);
      break;
   case Write:
      handle = (void *)CreateFile(fname,
         GENERIC_WRITE,
         0,
         NULL,
         CREATE_ALWAYS,
         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
         NULL);
      break;
   case ReadWrite:
      handle = (void *)CreateFile(fname,
         GENERIC_WRITE | GENERIC_READ,
         0,
         NULL,
         OPEN_ALWAYS,
         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
         NULL);
      break;
   case WriteAppend:
      handle = (void *)CreateFile(fname,
         GENERIC_WRITE,
         0,
         NULL,
         OPEN_ALWAYS,
         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
         NULL);
      break;

   default:
      AssertFatal(false, "File::open: bad access mode");    // impossible
   }

   if (INVALID_HANDLE_VALUE == (HANDLE)handle)                // handle not created successfully
   {
      return setStatus();
   }
   else
   {
      // successfully created file, so set the file capabilities...
      switch (openMode)
      {
      case Read:
         capability = U32(FileRead);
         break;
      case Write:
      case WriteAppend:
         capability = U32(FileWrite);
         break;
      case ReadWrite:
         capability = U32(FileRead)  |
            U32(FileWrite);
         break;
      default:
         AssertFatal(false, "File::open: bad access mode");
      }
      return currentStatus = Ok;                                // success!
   }
}

//-----------------------------------------------------------------------------
// Get the current position of the file pointer.
//-----------------------------------------------------------------------------
U32 File::getPosition() const
{
    AssertFatal(Closed != currentStatus, "File::getPosition: file closed");
    AssertFatal(INVALID_HANDLE_VALUE != (HANDLE)handle, "File::getPosition: invalid file handle");

    return SetFilePointer((HANDLE)handle,
                          0,                                    // how far to move
                          NULL,                                    // pointer to high word
                          FILE_CURRENT);                        // from what point
}

//-----------------------------------------------------------------------------
// Set the position of the file pointer.
// Absolute and relative positioning is supported via the absolutePos
// parameter.
//
// If positioning absolutely, position MUST be positive - an IOError results if
// position is negative.
// Position can be negative if positioning relatively, however positioning
// before the start of the file is an IOError.
//
// Returns the currentStatus of the file.
//-----------------------------------------------------------------------------
File::Status File::setPosition(S32 position, bool absolutePos)
{
    AssertFatal(Closed != currentStatus, "File::setPosition: file closed");
    AssertFatal(INVALID_HANDLE_VALUE != (HANDLE)handle, "File::setPosition: invalid file handle");

    if (Ok != currentStatus && EOS != currentStatus)
        return currentStatus;

    U32 finalPos;
    if (absolutePos)
    {
        AssertFatal(0 <= position, "File::setPosition: negative absolute position");

        // position beyond EOS is OK
        finalPos = SetFilePointer((HANDLE)handle,
                                  position,
                                  NULL,
                                  FILE_BEGIN);
    }
    else
    {
       AssertFatal((getPosition() >= (U32)abs(position) && 0 > position) || 0 <= position, "File::setPosition: negative relative position");

        // position beyond EOS is OK
        finalPos = SetFilePointer((HANDLE)handle,
                                  position,
                                  NULL,
                                  FILE_CURRENT);
    }

    if (0xffffffff == finalPos)
        return setStatus();                                        // unsuccessful
    else if (finalPos >= getSize())
        return currentStatus = EOS;                                // success, at end of file
    else
        return currentStatus = Ok;                                // success!
}

//-----------------------------------------------------------------------------
// Get the size of the file in bytes.
// It is an error to query the file size for a Closed file, or for one with an
// error status.
//-----------------------------------------------------------------------------
U32 File::getSize() const
{
    AssertWarn(Closed != currentStatus, "File::getSize: file closed");
    AssertFatal(INVALID_HANDLE_VALUE != (HANDLE)handle, "File::getSize: invalid file handle");

    if (Ok == currentStatus || EOS == currentStatus)
    {
        DWORD high;
        return GetFileSize((HANDLE)handle, &high);                // success!
    }
    else
        return 0;                                                // unsuccessful
}

//-----------------------------------------------------------------------------
// Flush the file.
// It is an error to flush a read-only file.
// Returns the currentStatus of the file.
//-----------------------------------------------------------------------------
File::Status File::flush()
{
    AssertFatal(Closed != currentStatus, "File::flush: file closed");
    AssertFatal(INVALID_HANDLE_VALUE != (HANDLE)handle, "File::flush: invalid file handle");
    AssertFatal(true == hasCapability(FileWrite), "File::flush: cannot flush a read-only file");

    if (0 != FlushFileBuffers((HANDLE)handle))
        return setStatus();                                        // unsuccessful
    else
        return currentStatus = Ok;                                // success!
}

//-----------------------------------------------------------------------------
// Close the File.
//
// Returns the currentStatus
//-----------------------------------------------------------------------------
File::Status File::close()
{
    // check if it's already closed...
    if (Closed == currentStatus)
        return currentStatus;

    // it's not, so close it...
    if (INVALID_HANDLE_VALUE != (HANDLE)handle)
    {
        if (0 == CloseHandle((HANDLE)handle))
            return setStatus();                                    // unsuccessful
    }
    handle = (void *)INVALID_HANDLE_VALUE;
    return currentStatus = Closed;
}

//-----------------------------------------------------------------------------
// Self-explanatory.
//-----------------------------------------------------------------------------
File::Status File::getStatus() const
{
    return currentStatus;
}

//-----------------------------------------------------------------------------
// Sets and returns the currentStatus when an error has been encountered.
//-----------------------------------------------------------------------------
File::Status File::setStatus()
{
    switch (GetLastError())
    {
    case ERROR_INVALID_HANDLE:
    case ERROR_INVALID_ACCESS:
    case ERROR_TOO_MANY_OPEN_FILES:
    case ERROR_FILE_NOT_FOUND:
    case ERROR_SHARING_VIOLATION:
    case ERROR_HANDLE_DISK_FULL:
          return currentStatus = IOError;

    default:
          return currentStatus = UnknownError;
    }
}

//-----------------------------------------------------------------------------
// Sets and returns the currentStatus to status.
//-----------------------------------------------------------------------------
File::Status File::setStatus(File::Status status)
{
    return currentStatus = status;
}

//-----------------------------------------------------------------------------
// Read from a file.
// The number of bytes to read is passed in size, the data is returned in src.
// The number of bytes read is available in bytesRead if a non-Null pointer is
// provided.
//-----------------------------------------------------------------------------
File::Status File::read(U32 size, char *dst, U32 *bytesRead)
{
    AssertFatal(Closed != currentStatus, "File::read: file closed");
    AssertFatal(INVALID_HANDLE_VALUE != (HANDLE)handle, "File::read: invalid file handle");
    AssertFatal(NULL != dst, "File::read: NULL destination pointer");
    AssertFatal(true == hasCapability(FileRead), "File::read: file lacks capability");
    AssertWarn(0 != size, "File::read: size of zero");

    if (Ok != currentStatus || 0 == size)
        return currentStatus;
    else
    {
        DWORD lastBytes;
        DWORD *bytes = (NULL == bytesRead) ? &lastBytes : (DWORD *)bytesRead;
        if (0 != ReadFile((HANDLE)handle, dst, size, bytes, NULL))
        {
            if(*((U32 *)bytes) != size)
                return currentStatus = EOS;                        // end of stream
        }
        else
            return setStatus();                                    // unsuccessful
    }
    return currentStatus = Ok;                                    // successfully read size bytes
}

//-----------------------------------------------------------------------------
// Write to a file.
// The number of bytes to write is passed in size, the data is passed in src.
// The number of bytes written is available in bytesWritten if a non-Null
// pointer is provided.
//-----------------------------------------------------------------------------
File::Status File::write(U32 size, const char *src, U32 *bytesWritten)
{
    AssertFatal(Closed != currentStatus, "File::write: file closed");
    AssertFatal(INVALID_HANDLE_VALUE != (HANDLE)handle, "File::write: invalid file handle");
    AssertFatal(NULL != src, "File::write: NULL source pointer");
    AssertFatal(true == hasCapability(FileWrite), "File::write: file lacks capability");
    AssertWarn(0 != size, "File::write: size of zero");

    if ((Ok != currentStatus && EOS != currentStatus) || 0 == size)
        return currentStatus;
    else
    {
        DWORD lastBytes;
        DWORD *bytes = (NULL == bytesWritten) ? &lastBytes : (DWORD *)bytesWritten;
        if (0 != WriteFile((HANDLE)handle, src, size, bytes, NULL))
            return currentStatus = Ok;                            // success!
        else
            return setStatus();                                    // unsuccessful
    }
}

//-----------------------------------------------------------------------------
// Self-explanatory.
//-----------------------------------------------------------------------------
bool File::hasCapability(Capability cap) const
{
    return (0 != (U32(cap) & capability));
}

S32 Platform::compareFileTimes(const FileTime &a, const FileTime &b)
{
   if(a.v2 > b.v2)
      return 1;
   if(a.v2 < b.v2)
      return -1;
   if(a.v1 > b.v1)
      return 1;
   if(a.v1 < b.v1)
      return -1;
   return 0;
}

static bool recurseDumpPath(const char *path, const char *pattern, Vector<Platform::FileInfo> &fileVector, S32 recurseDepth )
{
   WIN32_FIND_DATA findData;

   TempAlloc< char > fullPath( dStrlen( path ) * 3 + MAX_PATH * 3 + 1 );
   Platform::makeFullPathName( path, fullPath, fullPath.size );

   U32 lenFullPath = dStrlen( fullPath );
   TempAlloc< char > buf( lenFullPath + MAX_PATH * 3 + 2 );
   dSprintf( buf, buf.size, "%s/%s", fullPath.ptr, pattern );

#ifdef UNICODE
   TempAlloc< WCHAR > searchBuf( buf.size );
   convertUTF8toUTF16( buf, searchBuf, searchBuf.size );
   WCHAR* search = searchBuf;
#else
   char *search = buf;
#endif

   backslash( search );
   
   HANDLE handle = FindFirstFile(search, &findData);
   if (handle == INVALID_HANDLE_VALUE)
      return false;

   do
   {
#ifdef UNICODE
      convertUTF16toUTF8( findData.cFileName, buf, buf.size );
      char* fnbuf = buf;
#else
      char *fnbuf = findData.cFileName;
#endif

      if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
         // make sure it is a directory
         if (findData.dwFileAttributes & (FILE_ATTRIBUTE_OFFLINE|FILE_ATTRIBUTE_SYSTEM) )                             
            continue;

         // skip . and .. directories
         if (dStrcmp( findData.cFileName, TEXT( "." ) ) == 0 || dStrcmp( findData.cFileName, TEXT( ".." ) ) == 0)
            continue;

         // Skip excluded directores
         if(Platform::isExcludedDirectory(fnbuf))
            continue;

         dSprintf( fullPath, fullPath.size, "%s/%s", path, fnbuf);
         char* child = fullPath;
         if( recurseDepth > 0 )
            recurseDumpPath(child, pattern, fileVector, recurseDepth - 1);
         else if (recurseDepth == -1)
            recurseDumpPath(child, pattern, fileVector, -1);
      }      
      else
      {
         // make sure it is the kind of file we're looking for
         if (findData.dwFileAttributes & 
             (FILE_ATTRIBUTE_DIRECTORY|                                      
              FILE_ATTRIBUTE_OFFLINE|
              FILE_ATTRIBUTE_SYSTEM|
              FILE_ATTRIBUTE_TEMPORARY) )                             
            continue;
         
         // add it to the list
         fileVector.increment();
         Platform::FileInfo& rInfo = fileVector.last();

         forwardslash( fnbuf );

         rInfo.pFullPath = StringTable->insert(path);
         rInfo.pFileName = StringTable->insert(fnbuf);
         rInfo.fileSize  = findData.nFileSizeLow;
      }

   }while(FindNextFile(handle, &findData));

   FindClose(handle);
   return true;
}


//--------------------------------------

bool Platform::getFileTimes(const char *filePath, FileTime *createTime, FileTime *modifyTime)
{
   WIN32_FIND_DATA findData;

   TempAlloc< TCHAR > fp( dStrlen( filePath ) + 1 );

#ifdef UNICODE
   convertUTF8toUTF16( filePath, fp, fp.size );
#else
   dStrcpy( fp, filePath );
#endif

   backslash( fp );

   HANDLE h = FindFirstFile(fp, &findData);
   if(h == INVALID_HANDLE_VALUE)
      return false;

   if(createTime)
   {
      createTime->v1 = findData.ftCreationTime.dwLowDateTime;
      createTime->v2 = findData.ftCreationTime.dwHighDateTime;
   }
   if(modifyTime)
   {
      modifyTime->v1 = findData.ftLastWriteTime.dwLowDateTime;
      modifyTime->v2 = findData.ftLastWriteTime.dwHighDateTime;
   }
   FindClose(h);
   return true;
}

//--------------------------------------
bool Platform::createPath(const char *file)
{
   TempAlloc< TCHAR > pathbuf( dStrlen( file ) + 1 );

#ifdef UNICODE
   TempAlloc< WCHAR > fileBuf( pathbuf.size );
   convertUTF8toUTF16( file, fileBuf, fileBuf.size );
   const WCHAR* fileName = fileBuf;
   const WCHAR* dir;
#else
   const char* fileName = file;
   const char* dir;
#endif

   pathbuf[ 0 ] = 0;
   U32 pathLen = 0;

   while((dir = dStrchr(fileName, '/')) != NULL)
   {
      TCHAR* pathptr = pathbuf;
      dMemcpy( pathptr + pathLen, fileName, ( dir - fileName ) * sizeof( TCHAR ) );
      pathbuf[pathLen + dir-fileName] = 0;
 
      // ignore return value because we are fine with already existing directory
      CreateDirectory(pathbuf, NULL);

      pathLen += dir - fileName;
      pathbuf[pathLen++] = '\\';
      fileName = dir + 1;
   }
   return true;
}

// [rene, 04/05/2008] Not used currently so did not bother updating.
#if 0
// [tom, 7/12/2005] Rather then converting this to unicode, just using the ANSI
// versions of the Win32 API as its quicker for testing.
bool Platform::cdFileExists(const char *filePath, const char *volumeName, S32 serialNum)
{
   if (!filePath || !filePath[0])
      return true;

   //first find the CD device...
   char fileBuf[1024];
   char drivesBuf[256];
   S32 length = GetLogicalDriveStringsA(256, drivesBuf);
   char *drivePtr = drivesBuf;
   while (S32(drivePtr - drivesBuf) < length)
   {
      char driveVolume[256], driveFileSystem[256];
      U32 driveSerial, driveFNLength, driveFlags;
      if ((dStricmp(drivePtr, "A:\\") != 0 && dStricmp(drivePtr, "B:\\") != 0) &&
          GetVolumeInformationA((const char*)drivePtr, &driveVolume[0], (unsigned long)255,
                               (unsigned long*)&driveSerial, (unsigned long*)&driveFNLength,
                               (unsigned long*)&driveFlags, &driveFileSystem[0], (unsigned long)255))
      {
#if defined (TORQUE_DEBUG) || !defined (TORQUE_SHIPPING)
         Con::printf("Found Drive: %s, vol: %s, serial: %d", drivePtr, driveVolume, driveSerial);
#endif
         //see if the volume and serial number match
         if (!dStricmp(volumeName, driveVolume) && (!serialNum || (serialNum == driveSerial)))
         {
            //see if the file exists on this volume
            if(dStrlen(drivePtr) == 3 && drivePtr[2] == '\\' && filePath[0] == '\\')
               dSprintf(fileBuf, sizeof(fileBuf), "%s%s", drivePtr, filePath + 1);
            else
               dSprintf(fileBuf, sizeof(fileBuf), "%s%s", drivePtr, filePath);
#if defined (TORQUE_DEBUG) || !defined (TORQUE_SHIPPING)
            Con::printf("Looking for file: %s on %s", fileBuf, driveVolume);
#endif
            WIN32_FIND_DATAA findData;
            HANDLE h = FindFirstFileA(fileBuf, &findData);
            if(h != INVALID_HANDLE_VALUE)
            {
               FindClose(h);
               return true;
            }
            FindClose(h);
         }
      }

      //check the next drive
      drivePtr += dStrlen(drivePtr) + 1;
   }

   return false;
}
#endif

//--------------------------------------
bool Platform::dumpPath(const char *path, Vector<Platform::FileInfo> &fileVector, S32 recurseDepth)
{
   return recurseDumpPath(path, "*", fileVector, recurseDepth );
}


//--------------------------------------

//StringTableEntry Platform::getWorkingDirectory()
//{
//   return getCurrentDirectory();
//}

StringTableEntry Platform::getCurrentDirectory()
{
   TempAlloc< TCHAR > buf( 2048 );

   GetCurrentDirectory( buf.size, buf );
   forwardslash( buf );

#ifdef UNICODE
   char* utf8 = convertUTF16toUTF8( buf );
   StringTableEntry result = StringTable->insert( utf8 );
   SAFE_DELETE_ARRAY( utf8 );
   return result;
#else
   return StringTable->insert( buf );
#endif
}

bool Platform::setCurrentDirectory(StringTableEntry newDir)
{

   if (Platform::getWebDeployment())
      return true;

   TempAlloc< TCHAR > buf( dStrlen( newDir ) + 2 );

#ifdef UNICODE
   convertUTF8toUTF16( newDir, buf, buf.size - 1 );
#else
   dStrcpy( buf, newDir );
#endif

   backslash( buf );
   return SetCurrentDirectory( buf );
}

#ifdef UNICODE
static void getExecutableInfo( StringTableEntry* path, StringTableEntry* exe )
{
   static StringTableEntry pathEntry = NULL;
   static StringTableEntry exeEntry = NULL;

   if( !pathEntry )
   {
      if (!Platform::getWebDeployment())
      {
         WCHAR cen_buf[ 2048 ];
         GetModuleFileNameW( NULL, cen_buf, sizeof( cen_buf ) / sizeof( cen_buf[ 0 ] ) );
         forwardslash( cen_buf );

         WCHAR* delimiter = dStrrchr( cen_buf, '/' );
         if( delimiter )
            *delimiter = '\0';

         char* pathBuf = convertUTF16toUTF8( cen_buf );
         char* exeBuf = convertUTF16toUTF8( delimiter + 1 );

         pathEntry = StringTable->insert( pathBuf );
         exeEntry = StringTable->insert( exeBuf );

         SAFE_DELETE_ARRAY( pathBuf );
         SAFE_DELETE_ARRAY( exeBuf );
      }
      else
      {
         char cdir[4096];
         GetCurrentDirectoryA(4096, cdir);
         pathEntry = StringTable->insert(cdir);
         exeEntry = StringTable->insert("WebGameCtrl.exe");
      }
   }

   if( path )
      *path = pathEntry;
   if( exe )
      *exe = exeEntry;
}
#endif

StringTableEntry Platform::getExecutableName()
{
#ifdef UNICODE
   StringTableEntry exe;
   getExecutableInfo( NULL, &exe );
   return exe;
#else
   static StringTableEntry cen = NULL;
   if (!cen)
   {
      char cen_buf[2048];
      GetModuleFileNameA( NULL, cen_buf, 2047);
      forwardslash(cen_buf);

      char *delimiter = dStrrchr( cen_buf, '/' );

      if( delimiter != NULL )
      {
         *delimiter = 0x00;
         delimiter++;
         cen = StringTable->insert(delimiter);
      }
      else
         cen = StringTable->insert(cen_buf);
   }
   return cen;
#endif
}

StringTableEntry Platform::getExecutablePath()
{
#ifdef UNICODE
   StringTableEntry path;
   getExecutableInfo( &path, NULL );
   return path;
#else
   static StringTableEntry cen = NULL;
   if (!cen)
   {
      char cen_buf[2048];
      GetModuleFileNameA( NULL, cen_buf, 2047);
      forwardslash(cen_buf);

      char *delimiter = dStrrchr( cen_buf, '/' );

      if( delimiter != NULL )
         *delimiter = 0x00;

      cen = StringTable->insert(cen_buf);
   }
   return cen;
#endif
}

//--------------------------------------
bool Platform::isFile(const char *pFilePath)
{
   if (!pFilePath || !*pFilePath)
      return false;

   TempAlloc< TCHAR > buf( dStrlen( pFilePath ) + 1 );

#ifdef UNICODE
   convertUTF8toUTF16( pFilePath, buf, buf.size );
#else
   dStrcpy( buf, pFilePath );
#endif
   backslash( buf );

   // Get file info
   WIN32_FIND_DATA findData;
   HANDLE handle = FindFirstFile(buf, &findData);
   FindClose(handle);

   if(handle == INVALID_HANDLE_VALUE)
   {
    
      // Since file does not exist on disk see if it exists in a zip file loaded
      return Torque::FS::IsFile(pFilePath);
   }

   // if the file is a Directory, Offline, System or Temporary then FALSE
   if (findData.dwFileAttributes &
       (FILE_ATTRIBUTE_DIRECTORY|
        FILE_ATTRIBUTE_OFFLINE|
        FILE_ATTRIBUTE_SYSTEM|
        FILE_ATTRIBUTE_TEMPORARY) )
      return false;

   // must be a real file then
   return true;
}

//--------------------------------------
S32 Platform::getFileSize(const char *pFilePath)
{
   if (!pFilePath || !*pFilePath)
      return -1;

   TempAlloc< TCHAR > buf( dStrlen( pFilePath ) + 1 );

#ifdef UNICODE
   convertUTF8toUTF16( pFilePath, buf, buf.size );
#else
   dStrcpy( buf, pFilePath );
#endif
   backslash( buf );

   // Get file info
   WIN32_FIND_DATA findData;
   HANDLE handle = FindFirstFile(buf, &findData);

   if(handle == INVALID_HANDLE_VALUE)
      return -1;

   FindClose(handle);

   // if the file is a Directory, Offline, System or Temporary then FALSE
   if (findData.dwFileAttributes &
       (FILE_ATTRIBUTE_DIRECTORY|
        FILE_ATTRIBUTE_OFFLINE|
        FILE_ATTRIBUTE_SYSTEM|
        FILE_ATTRIBUTE_TEMPORARY) )
      return -1;

   // must be a real file then
   return findData.nFileSizeLow;
}


//--------------------------------------
bool Platform::isDirectory(const char *pDirPath)
{
   if (!pDirPath || !*pDirPath)
      return false;

   TempAlloc< TCHAR > buf( dStrlen( pDirPath ) + 1 );

#ifdef UNICODE
   convertUTF8toUTF16( pDirPath, buf, buf.size );
#else
   dStrcpy( buf, pDirPath );
#endif
   backslash( buf );

   // Get file info
   WIN32_FIND_DATA findData;
   HANDLE handle = FindFirstFile(buf, &findData);

   // [neo, 5/15/2007]
   // This check was AFTER FindClose for some reason - this is most probably the
   // original intent.
   if(handle == INVALID_HANDLE_VALUE)
      return false;

   FindClose(handle);
   
   // if the file is a Directory, Offline, System or Temporary then FALSE
   if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
   {
      // make sure it's a valid game directory
      if (findData.dwFileAttributes & (FILE_ATTRIBUTE_OFFLINE|FILE_ATTRIBUTE_SYSTEM) )
         return false;

      // must be a directory
      return true;
   }

   return false;
}



//--------------------------------------
bool Platform::isSubDirectory(const char *pParent, const char *pDir)
{
   if (!pParent || !*pDir)
      return false;

   const char* fileName = avar("%s/*", pParent);

   TempAlloc< TCHAR > file( dStrlen( fileName ) + 1 );
   TempAlloc< TCHAR > dir( dStrlen( pDir ) + 1 );

#ifdef UNICODE
   convertUTF8toUTF16( fileName, file, file.size );
   convertUTF8toUTF16( pDir, dir, dir.size );
#else
   dStrcpy( file, fileName );
   dStrcpy( dir, pDir );
#endif

   backslash( file );
   backslash( dir );

   // this is somewhat of a brute force method but we need to be 100% sure
   // that the user cannot enter things like ../dir or /dir etc,...
   WIN32_FIND_DATA findData;
   HANDLE handle = FindFirstFile(file, &findData);
   if (handle == INVALID_HANDLE_VALUE)
      return false;
   do
   {
      // if it is a directory...
      if (findData.dwFileAttributes &
          (FILE_ATTRIBUTE_DIRECTORY|
           FILE_ATTRIBUTE_OFFLINE|
           FILE_ATTRIBUTE_SYSTEM|
           FILE_ATTRIBUTE_TEMPORARY) )
      {
         //FIXME: this has to be dStrcasecmp but there's no implementation for Unicode

         // and the names match
         if (dStrcmp(dir, findData.cFileName ) == 0)
         {
            // then we have a real sub directory
            FindClose(handle);
            return true;
         }
      }
   }while(FindNextFile(handle, &findData));

   FindClose(handle);
   return false;
}

//------------------------------------------------------------------------------

bool Platform::fileTimeToString(FileTime * time, char * string, U32 strLen)
{
   if(!time || !string)
      return(false);

   dSprintf(string, strLen, "%d:%d", time->v2, time->v1);
   return(true);
}

bool Platform::stringToFileTime(const char * string, FileTime * time)
{
   if(!time || !string)
      return(false);

   char buf[80];
   dSprintf(buf, sizeof(buf), (char *)string);

   char * sep = (char *)dStrstr((const char *)buf, (const char *)":");
   if(!sep)
      return(false);

   *sep = 0;
   sep++;

   time->v2 = dAtoi(buf);
   time->v1 = dAtoi(sep);

   return(true);
}

// Volume Functions

void Platform::getVolumeNamesList( Vector<const char*>& out_rNameVector, bool bOnlyFixedDrives )
{
	DWORD dwDrives = GetLogicalDrives();
	DWORD dwMask = 1;
	char driveLetter[12];

   out_rNameVector.clear();
		
	for(int i = 0; i < 32; i++ )
	{
		dMemset(driveLetter,0,12);
		if( dwDrives & dwMask )
		{
			dSprintf(driveLetter, 12, "%c:", (i + 'A'));

			if( bOnlyFixedDrives && GetDriveTypeA(driveLetter) == DRIVE_FIXED )
            out_rNameVector.push_back( StringTable->insert( driveLetter ) );
         else if ( !bOnlyFixedDrives )
            out_rNameVector.push_back( StringTable->insert( driveLetter ) );
		}
		dwMask <<= 1;
	}
}

void Platform::getVolumeInformationList( Vector<VolumeInformation>& out_rVolumeInfoVector, bool bOnlyFixedDrives )
{
   Vector<const char*> drives;

   getVolumeNamesList( drives, bOnlyFixedDrives );

   if( ! drives.empty() )
   {
      Vector<StringTableEntry>::iterator i;
      for( i = drives.begin(); i != drives.end(); i++ )
      {
         VolumeInformation info;
         TCHAR lpszVolumeName[ 256 ];
         TCHAR lpszFileSystem[ 256 ];
         DWORD dwSerial = 0;
         DWORD dwMaxComponentLength = 0;
         DWORD dwFileSystemFlags = 0;

         dMemset( lpszVolumeName, 0, sizeof( lpszVolumeName ) );
         dMemset( lpszFileSystem, 0, sizeof( lpszFileSystem ) );
         dMemset( &info, 0, sizeof( VolumeInformation ) );

         // More volume information
         UINT uDriveType = GetDriveTypeA( (*i) );
         if( uDriveType == DRIVE_UNKNOWN )
            info.Type = DRIVETYPE_UNKNOWN;
         else if( uDriveType == DRIVE_REMOVABLE )
            info.Type = DRIVETYPE_REMOVABLE;
         else if( uDriveType == DRIVE_FIXED )
            info.Type = DRIVETYPE_FIXED;
         else if( uDriveType == DRIVE_CDROM )
            info.Type = DRIVETYPE_CDROM;
         else if( uDriveType == DRIVE_RAMDISK )
            info.Type = DRIVETYPE_RAMDISK;
         else if( uDriveType == DRIVE_REMOTE )
            info.Type = DRIVETYPE_REMOTE;

         info.RootPath = StringTable->insert( (*i) );

         // We don't retrieve drive volume info for removable drives, because it's loud :(
         if( info.Type != DRIVETYPE_REMOVABLE )
         {
#ifdef UNICODE
            WCHAR ibuf[ 3 ];
            ibuf[ 0 ] = ( *i )[ 0 ];
            ibuf[ 1 ] = ':';
            ibuf[ 2 ] = '\0';
#else
            char* ibuf = *i;
#endif
            // Standard volume information
            GetVolumeInformation( ibuf, lpszVolumeName, sizeof( lpszVolumeName ) / sizeof( lpszVolumeName[ 0 ] ),
               &dwSerial, &dwMaxComponentLength, &dwFileSystemFlags, lpszFileSystem,
               sizeof( lpszFileSystem ) / sizeof( lpszFileSystem[ 0 ] ) );

#ifdef UNICODE
            char buf[ sizeof( lpszFileSystem ) / sizeof( lpszFileSystem[ 0 ] ) * 3 + 1 ];
            convertUTF16toUTF8( lpszFileSystem, buf, sizeof( buf ) / sizeof( buf[ 0 ] ) );
            info.FileSystem = StringTable->insert( buf );

            convertUTF16toUTF8( lpszVolumeName, buf, sizeof( buf ) / sizeof( buf[ 0 ] ) );
            info.Name = StringTable->insert( buf );
#else
            info.FileSystem = StringTable->insert( lpszFileSystem );
            info.Name = StringTable->insert( lpszVolumeName );
#endif
            info.SerialNumber = dwSerial;
            // Won't compile on something prior to XP.
            info.ReadOnly = dwFileSystemFlags & FILE_READ_ONLY_VOLUME;
         }
         out_rVolumeInfoVector.push_back( info );

         // I opted not to get free disk space because of the overhead of the calculations required for it

      }
   }
}


bool Platform::hasSubDirectory(const char *pPath)
{
   if( !pPath )
      return false;

   char searchBuf[1024];

   // Compose our search string - Format : ([path]/[subpath]/*)
   char trail = pPath[ dStrlen(pPath) - 1 ];
   if( trail == '/' )
      dStrcpy( searchBuf, pPath );
   else
      dSprintf(searchBuf, 1024, "%s/*", pPath );

#ifdef UNICODE
   WCHAR buf[ 1024 ];
   convertUTF8toUTF16( searchBuf, buf, sizeof( buf ) / sizeof( buf[ 0 ] ) );
   WCHAR* search = buf;
#else
   char* search = searchBuf;
#endif

   backslash( search );

   // See if we get any hits
   WIN32_FIND_DATA findData;
   HANDLE handle = FindFirstFile(search, &findData);
   if (handle == INVALID_HANDLE_VALUE)
      return false;

   bool result = false;
   do
   {
      if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
         // skip . and .. directories
         if (dStrcmp(findData.cFileName, TEXT( "." ) ) == 0 || dStrcmp(findData.cFileName, TEXT( ".." ) ) == 0)
            continue;

#ifdef UNICODE
         char fileName[ 1024 ];
         convertUTF16toUTF8( findData.cFileName, fileName, sizeof( fileName ) / sizeof( fileName[ 0 ] ) );
#else
         char* fileName = findData.cFileName;
#endif

         if( Platform::isExcludedDirectory( fileName ) )
            continue;

         result = true;
         break;
      }      
   }
   while(FindNextFile(handle, &findData));

   FindClose(handle);

   Platform::clearExcludedDirectories();

   return result;
}


static bool recurseDumpDirectories(const char *basePath, const char *subPath, Vector<StringTableEntry> &directoryVector, S32 currentDepth, S32 recurseDepth, bool noBasePath)
{
   TempAlloc< char > search( 1024 );

   //-----------------------------------------------------------------------------
   // Compose our search string - Format : ([path]/[subpath]/*)
   //-----------------------------------------------------------------------------

   char trail = basePath[ dStrlen(basePath) - 1 ];
   char subTrail = subPath ? subPath[ dStrlen(subPath) - 1 ] : '\0';

   if( trail == '/' )
   {
      // we have a sub path and it's not an empty string
      if(  subPath  && ( dStrncmp( subPath, "", 1 ) != 0 ) )
      {
         if( subTrail == '/' )
            dSprintf(search, search.size, "%s%s*", basePath,subPath );
         else
            dSprintf(search, search.size, "%s%s/*", basePath,subPath );
      }
      else
         dSprintf( search, search.size, "%s*", basePath );
   }
   else
   {
      if(  subPath  && ( dStrncmp( subPath, "", 1 ) != 0 ) )
         if( subTrail == '/' )
            dSprintf(search, search.size, "%s%s*", basePath,subPath );
         else
            dSprintf(search, search.size, "%s%s/*", basePath,subPath );
      else
         dSprintf(search, search.size, "%s/*", basePath );
   }

#ifdef UNICODE
   TempAlloc< WCHAR > searchStr( dStrlen( search ) + 1 );
   convertUTF8toUTF16( search, searchStr, searchStr.size );
#else
   char* searchStr = search;
#endif

   backslash( searchStr );

   //-----------------------------------------------------------------------------
   // See if we get any hits
   //-----------------------------------------------------------------------------

   WIN32_FIND_DATA findData;
   HANDLE handle = FindFirstFile(searchStr, &findData);
   if (handle == INVALID_HANDLE_VALUE)
      return false;

   //-----------------------------------------------------------------------------
   // add path to our return list ( provided it is valid )
   //-----------------------------------------------------------------------------
   if( !Platform::isExcludedDirectory( subPath ) )
   {

      if( noBasePath )
      {
         // We have a path and it's not an empty string or an excluded directory
         if( ( subPath  && ( dStrncmp( subPath, "", 1 ) != 0 ) ) )
            directoryVector.push_back( StringTable->insert( subPath ) );
      }
      else
      {
         if( ( subPath  && ( dStrncmp( subPath, "", 1 ) != 0 ) ) )
         {
            char szPath [ 1024 ];
            dMemset( szPath, 0, 1024 );
            if( trail != '/' )
               dSprintf( szPath, 1024, "%s%s", basePath, subPath );
            else
               dSprintf( szPath, 1024, "%s%s", basePath, &subPath[1] );
            directoryVector.push_back( StringTable->insert( szPath ) );
         }
         else
            directoryVector.push_back( StringTable->insert( basePath ) );
      }
   }

   //-----------------------------------------------------------------------------
   // Iterate through and grab valid directories
   //-----------------------------------------------------------------------------

#ifdef UNICODE
   TempAlloc< char > fileName( 1024 );
#endif

   do
   {
      if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
         // skip . and .. directories
         if (dStrcmp(findData.cFileName, TEXT( "." )) == 0 || dStrcmp(findData.cFileName, TEXT( ".." )) == 0)
            continue;

#ifdef UNICODE
         convertUTF16toUTF8( findData.cFileName, fileName, fileName.size );
#else
         char* fileName = findData.cFileName;
#endif

         // skip excluded directories
         if( Platform::isExcludedDirectory( fileName ) )
            continue;

         if( ( subPath  && ( dStrncmp( subPath, "", 1 ) != 0 ) ))
         {
            if( subTrail == '/' )
               dSprintf(search, search.size, "%s%s", subPath, fileName);
            else
               dSprintf(search, search.size, "%s/%s", subPath, fileName);
            char* child = search;

            if( currentDepth < recurseDepth || recurseDepth == -1 )
               recurseDumpDirectories(basePath, child, directoryVector, currentDepth+1, recurseDepth, noBasePath );

         }
         else
         {
            char* child;
            if( trail == '/' )
               child = fileName;
            else
            {
               dSprintf(search, search.size, "/%s", fileName);
               child = search;
            }

            if( currentDepth < recurseDepth || recurseDepth == -1 )
               recurseDumpDirectories(basePath, child, directoryVector, currentDepth+1, recurseDepth, noBasePath );
         }
      }      
   }
   while(FindNextFile(handle, &findData));

   FindClose(handle);
   return true;
}

bool Platform::dumpDirectories( const char *path, Vector<StringTableEntry> &directoryVector, S32 depth, bool noBasePath )
{
   bool retVal = recurseDumpDirectories( path, "", directoryVector, -1, depth, noBasePath );

   clearExcludedDirectories();

   return retVal;
}

//-----------------------------------------------------------------------------

StringTableEntry osGetTemporaryDirectory()
{
   TCHAR buf[ 1024 ];
   const U32 bufSize = sizeof( buf ) / sizeof( buf[ 0 ] );
   DWORD len = GetTempPath( sizeof( buf ) / sizeof( buf[ 0 ] ), buf );

   TempAlloc< TCHAR > temp;
   TCHAR* buffer = buf;
   if( len > bufSize - 1 )
   {
      temp = TempAlloc< TCHAR >( len + 1 );
      buffer = temp;
      GetTempPath( len + 1, buffer );
   }

   // Remove the trailing slash
   buffer[len-1] = 0;

#ifdef UNICODE
   TempAlloc< char > dirBuffer( len * 3 + 1 );
   char* dir = dirBuffer;
   convertUTF16toUTF8( buffer, dir, dirBuffer.size );
#else
   char* dir = buf;
#endif

   forwardslash(dir);
   return StringTable->insert(dir);
}
