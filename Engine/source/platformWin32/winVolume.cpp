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

#include <windows.h>

#include "core/crc.h"
#include "core/frameAllocator.h"
#include "core/util/str.h"
#include "core/strings/stringFunctions.h"
#include "core/strings/unicode.h"

#include "platform/platformVolume.h"

#include "platformWin32/winVolume.h"

#include "console/console.h"


#ifndef NGROUPS_UMAX
   #define NGROUPS_UMAX 32
#endif

namespace Torque
{
namespace Win32
{

   // If the file is a Directory, Offline, System or Temporary then FALSE
#define S_ISREG(Flags) \
   !((Flags) & \
   (FILE_ATTRIBUTE_DIRECTORY | \
   FILE_ATTRIBUTE_OFFLINE | \
   FILE_ATTRIBUTE_SYSTEM | \
   FILE_ATTRIBUTE_TEMPORARY))

#define S_ISDIR(Flags) \
   ((Flags) & FILE_ATTRIBUTE_DIRECTORY)

//-----------------------------------------------------------------------------

   class Win32FileSystemChangeNotifier : public FileSystemChangeNotifier
   {
   public:
      Win32FileSystemChangeNotifier( FileSystem *fs )
         :  FileSystemChangeNotifier( fs )
      {
         VECTOR_SET_ASSOCIATION( mHandleList );
         VECTOR_SET_ASSOCIATION( mDirs );
      }

      // for use in the thread itself
      U32      getNumHandles() const { return mHandleList.size(); }
      HANDLE   *getHANDLES() { return mHandleList.address(); }

   private:
      virtual void   internalProcessOnce();

      virtual bool   internalAddNotification( const Path &dir );
      virtual bool   internalRemoveNotification( const Path &dir );

      Vector<Path>   mDirs;
      Vector<HANDLE> mHandleList;
   };

//-----------------------------------------------------------------------------

static String _BuildFileName(const String& prefix,const Path& path)
{
   // Need to join the path (minus the root) with our
   // internal path name.
   String file = prefix;
   file = Path::Join(file, '/', path.getPath());
   file = Path::Join(file, '/', path.getFileName());
   file = Path::Join(file, '.', path.getExtension());
   return file;
}

/*
static bool _IsFile(const String& file)
{
   // Get file info
   WIN32_FIND_DATA info;
   HANDLE handle = ::FindFirstFile(PathToOS(file).utf16(), &info);
   ::FindClose(handle);
   if (handle == INVALID_HANDLE_VALUE)
      return false;

   return S_ISREG(info.dwFileAttributes);
}
*/

static bool _IsDirectory(const String& file)
{
   // Get file info
   WIN32_FIND_DATAW info;
   HANDLE handle = ::FindFirstFileW(PathToOS(file).utf16(), &info);
   ::FindClose(handle);
   if (handle == INVALID_HANDLE_VALUE)
      return false;

   return S_ISDIR(info.dwFileAttributes);
}


//-----------------------------------------------------------------------------

static void _CopyStatAttributes(const WIN32_FIND_DATAW& info, FileNode::Attributes* attr)
{
   // Fill in the return struct.
   attr->flags = 0;
   if (S_ISDIR(info.dwFileAttributes))
      attr->flags |= FileNode::Directory;
   if (S_ISREG(info.dwFileAttributes))
      attr->flags |= FileNode::File;

   if (info.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
      attr->flags |= FileNode::ReadOnly;

   attr->size = info.nFileSizeLow;
   attr->mtime = Win32FileTimeToTime(
      info.ftLastWriteTime.dwLowDateTime,
      info.ftLastWriteTime.dwHighDateTime);

   attr->atime = Win32FileTimeToTime(
      info.ftLastAccessTime.dwLowDateTime,
      info.ftLastAccessTime.dwHighDateTime);
}


//-----------------------------------------------------------------------------

bool Win32FileSystemChangeNotifier::internalAddNotification( const Path &dir )
{
   for ( U32 i = 0; i < mDirs.size(); ++i )
   {
      if ( mDirs[i] == dir )
         return false;
   }

   Path     fullFSPath = mFS->mapTo( dir );
   String   osPath = PathToOS( fullFSPath );

//   Con::printf( "[Win32FileSystemChangeNotifier::internalAddNotification] : [%s]", osPath.c_str() );

   HANDLE   changeHandle = ::FindFirstChangeNotificationW( 
                                 osPath.utf16(),      // directory to watch 
                                 FALSE,                           // do not watch subtree 
                                 FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_ATTRIBUTES);  // watch file write changes

   if (changeHandle == INVALID_HANDLE_VALUE || changeHandle == NULL) 
   {
      Con::errorf("[Win32FileSystemChangeNotifier::internalAddNotification] : failed on [%s] [%d]", osPath.c_str(), GetLastError());
      
      return false; 
   }

   mDirs.push_back( dir );
   mHandleList.push_back( changeHandle );

   return true;
}

bool Win32FileSystemChangeNotifier::internalRemoveNotification( const Path &dir )
{
   for ( U32 i = 0; i < mDirs.size(); ++i )
   {
      if ( mDirs[i] != dir )
         continue;

      ::FindCloseChangeNotification( mHandleList[i] );
      mDirs.erase( i );
      mHandleList.erase( i );

      return true;
   }

   return false;
}

void  Win32FileSystemChangeNotifier::internalProcessOnce()
{
   // WaitForMultipleObjects has a limit of MAXIMUM_WAIT_OBJECTS (64 at 
   // the moment), so we have to loop till we've handled the entire set.

   for ( U32 i=0; i < mHandleList.size(); i += MAXIMUM_WAIT_OBJECTS )
   {
      U32 numHandles = getMin( (U32)MAXIMUM_WAIT_OBJECTS, (U32)mHandleList.size() - i );

      DWORD dwWaitStatus = WaitForMultipleObjects( numHandles, mHandleList.address()+i, FALSE, 0);
      if ( dwWaitStatus == WAIT_FAILED || dwWaitStatus == WAIT_TIMEOUT )
         continue;

      if ( dwWaitStatus >= WAIT_OBJECT_0 && dwWaitStatus <= (WAIT_OBJECT_0 + numHandles - 1))
      {
         U32 index = i + dwWaitStatus;

         // reset our notification
         // NOTE: we do this before letting the volume system check mod times so we don't miss any.
         //    It is going to loop over the files and check their mod time vs. the saved time.
         //    This may result in extra calls to internalNotifyDirChanged(), but it will simpley check mod times again.
         ::FindNextChangeNotification( mHandleList[index] );

         internalNotifyDirChanged( mDirs[index] );
      }
   }
}

//-----------------------------------------------------------------------------

Win32FileSystem::Win32FileSystem(String volume)
{
   mVolume = volume;
   mChangeNotifier = new Win32FileSystemChangeNotifier( this );
}

Win32FileSystem::~Win32FileSystem()
{
}

FileNodeRef Win32FileSystem::resolve(const Path& path)
{
   String file = _BuildFileName(mVolume,path);

   WIN32_FIND_DATAW info;
   HANDLE handle = ::FindFirstFileW(PathToOS(file).utf16(), &info);
   ::FindClose(handle);
   if (handle != INVALID_HANDLE_VALUE)
   {
      if (S_ISREG(info.dwFileAttributes))
         return new Win32File(path,file);
      if (S_ISDIR(info.dwFileAttributes))
         return new Win32Directory(path,file);
   }
   return 0;
}

FileNodeRef Win32FileSystem::create(const Path& path, FileNode::Mode mode)
{
   // The file will be created on disk when it's opened.
   if (mode & FileNode::File)
      return new Win32File(path,_BuildFileName(mVolume,path));

   // Create with default permissions.
   if (mode & FileNode::Directory)
   {
      String file = PathToOS(_BuildFileName(mVolume,path));

      if (::CreateDirectoryW(file.utf16(), 0))
         return new Win32Directory(path, file);
   }
   return 0;
}

bool Win32FileSystem::remove(const Path& path)
{
   // Should probably check for outstanding files or directory objects.
   String file = PathToOS(_BuildFileName(mVolume,path));

   WIN32_FIND_DATAW info;
   HANDLE handle = ::FindFirstFileW(file.utf16(), &info);
   ::FindClose(handle);
   if (handle == INVALID_HANDLE_VALUE)
      return false;

   if (S_ISDIR(info.dwFileAttributes))
      return ::RemoveDirectoryW(file.utf16());

   return ::DeleteFileW(file.utf16());
}

bool Win32FileSystem::rename(const Path& from,const Path& to)
{
   String fa = PathToOS(_BuildFileName(mVolume,from));
   String fb = PathToOS(_BuildFileName(mVolume,to));

   return MoveFile(fa.utf16(),fb.utf16());
}

Path Win32FileSystem::mapTo(const Path& path)
{
   return _BuildFileName(mVolume,path);
}

Path Win32FileSystem::mapFrom(const Path& path)
{
   const String::SizeType  volumePathLen = mVolume.length();
   
   String   pathStr = path.getFullPath();

   if ( mVolume.compare( pathStr, volumePathLen, String::NoCase ))
      return Path();

   return pathStr.substr( volumePathLen, pathStr.length() - volumePathLen );
}

//-----------------------------------------------------------------------------

Win32File::Win32File(const Path& path,String name)
{
   mPath = path;
   mName = name;
   mStatus = Closed;
   mHandle = 0;
}

Win32File::~Win32File()
{
   if (mHandle)
      close();
}

Path Win32File::getName() const
{
   return mPath;
}

FileNode::Status Win32File::getStatus() const
{
   return mStatus;
}

bool Win32File::getAttributes(Attributes* attr)
{
   WIN32_FIND_DATAW info;
   HANDLE handle = ::FindFirstFileW(PathToOS(mName).utf16(), &info);
   ::FindClose(handle);
   if (handle == INVALID_HANDLE_VALUE)
      return false;

   _CopyStatAttributes(info,attr);
   attr->name = mPath;
   return true;
}

U64 Win32File::getSize()
{
   U64 size;

   if (mStatus == Open)
   {
      // Special case if file is open (handles unflushed buffers)
      if ( !GetFileSizeEx(mHandle, (PLARGE_INTEGER)&size) )
         size = 0;
      return size;
   }
   else
   {
      // Fallback to generic function
      size = File::getSize();
   }

   return size;
}

U32 Win32File::calculateChecksum()
{
   if (!open( Read ))
      return 0;

   U64 fileSize = getSize();
   U32 bufSize = 1024 * 1024 * 4; // 4MB
   FrameTemp<U8> buf( bufSize );
   U32 crc = CRC::INITIAL_CRC_VALUE;

   while ( fileSize > 0 )
   {      
      U32 bytesRead = getMin( fileSize, bufSize );
      if ( read( buf, bytesRead ) != bytesRead )
      {
         close();
         return 0;
      }

      fileSize -= bytesRead;
      crc = CRC::calculateCRC(buf, bytesRead, crc);
   }   

   close();

   return crc;
}

bool Win32File::open(AccessMode mode)
{
   close();

   if (mName.isEmpty())
      return mStatus;

   struct Mode
   {
      DWORD mode,share,open;
   } Modes[] =
   {
      { GENERIC_READ,FILE_SHARE_READ,OPEN_EXISTING }, // Read
      { GENERIC_WRITE,0,CREATE_ALWAYS },              // Write
      { GENERIC_WRITE | GENERIC_READ,0,OPEN_ALWAYS }, // ReadWrite
      { GENERIC_WRITE,0,OPEN_ALWAYS }                 // WriteAppend
   };

   Mode& m = (mode == Read)? Modes[0]: (mode == Write)? Modes[1]:
         (mode == ReadWrite)? Modes[2]: Modes[3];

   mHandle = (void*)::CreateFileW(PathToOS(mName).utf16(),
               m.mode, m.share,
               NULL, m.open,
               FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
               NULL);

   if ( mHandle == INVALID_HANDLE_VALUE || mHandle == NULL )
   {
      _updateStatus();
      return false;
   }

   mStatus = Open;
   return true;
}

bool Win32File::close()
{
   if (mHandle)
   {
      ::CloseHandle((HANDLE)mHandle);
      mHandle = 0;
   }

   mStatus = Closed;
   return true;
}

U32 Win32File::getPosition()
{
   if (mStatus == Open || mStatus == EndOfFile)
      return ::SetFilePointer((HANDLE)mHandle,0,0,FILE_CURRENT);
   return 0;
}

U32 Win32File::setPosition(U32 delta, SeekMode mode)
{
   if (mStatus != Open && mStatus != EndOfFile)
      return 0;

   DWORD fmode;
   switch (mode)
   {
      case Begin:    fmode = FILE_BEGIN; break;
      case Current:  fmode = FILE_CURRENT; break;
      case End:      fmode = FILE_END; break;
      default:       fmode = 0; break;
   }

   DWORD pos = ::SetFilePointer((HANDLE)mHandle,delta,0,fmode);
   if (pos == INVALID_SET_FILE_POINTER)
   {
      mStatus = UnknownError;
      return 0;
   }

   mStatus = Open;

   return pos;
}

U32 Win32File::read(void* dst, U32 size)
{
   if (mStatus != Open && mStatus != EndOfFile)
      return 0;

   DWORD bytesRead;
   if (!::ReadFile((HANDLE)mHandle,dst,size,&bytesRead,0))
      _updateStatus();
   else if (bytesRead != size)
      mStatus = EndOfFile;

   return bytesRead;
}

U32 Win32File::write(const void* src, U32 size)
{
   if ((mStatus != Open && mStatus != EndOfFile) || !size)
      return 0;

   DWORD bytesWritten;
   if (!::WriteFile((HANDLE)mHandle,src,size,&bytesWritten,0))
      _updateStatus();
   return bytesWritten;
}

void Win32File::_updateStatus()
{
   switch (::GetLastError())
   {
      case ERROR_INVALID_ACCESS:       mStatus = AccessDenied;     break;
      case ERROR_TOO_MANY_OPEN_FILES:  mStatus = UnknownError;     break;
      case ERROR_PATH_NOT_FOUND:       mStatus = NoSuchFile;       break;
      case ERROR_FILE_NOT_FOUND:       mStatus = NoSuchFile;       break;
      case ERROR_SHARING_VIOLATION:    mStatus = SharingViolation; break;
      case ERROR_HANDLE_DISK_FULL:     mStatus = FileSystemFull;   break;
      case ERROR_ACCESS_DENIED:        mStatus = AccessDenied;     break;
      default:                         mStatus = UnknownError;     break;
   }
}

//-----------------------------------------------------------------------------

Win32Directory::Win32Directory(const Path& path,String name)
{
   mPath = path;
   mName = name;
   mStatus = Closed;
   mHandle = 0;
}

Win32Directory::~Win32Directory()
{
   if (mHandle)
      close();
}

Path Win32Directory::getName() const
{
   return mPath;
}

bool Win32Directory::open()
{
   if (!_IsDirectory(mName))
   {
      mStatus = NoSuchFile;
      return false;
   }
   mStatus = Open;
   return true;
}

bool Win32Directory::close()
{
   if (mHandle)
   {
      ::FindClose((HANDLE)mHandle);
      mHandle = 0;
      return true;
   }
   return false;
}

bool Win32Directory::read(Attributes* entry)
{
   if (mStatus != Open)
      return false;

   WIN32_FIND_DATA info;
   if (!mHandle)
   {
      mHandle = ::FindFirstFileW((PathToOS(mName) + "\\*").utf16(), &info);

      if (mHandle == NULL)
      {
         _updateStatus();
         return false;
      }
   }
   else
      if (!::FindNextFileW((HANDLE)mHandle, &info))
      {
         _updateStatus();
         return false;
      }

   // Skip "." and ".." entries
   if (info.cFileName[0] == '.' && (info.cFileName[1] == '\0' ||
      (info.cFileName[1] == '.' && info.cFileName[2] == '\0')))
      return read(entry);

   _CopyStatAttributes(info,entry);
   entry->name = info.cFileName;
   return true;
}


U32 Win32Directory::calculateChecksum()
{
   // Return checksum of current entry
   return 0;
}

bool Win32Directory::getAttributes(Attributes* attr)
{
   WIN32_FIND_DATA info;
   HANDLE handle = ::FindFirstFileW(PathToOS(mName).utf16(), &info);
   ::FindClose(handle);
   if (handle == INVALID_HANDLE_VALUE)
   {
      _updateStatus();
      return false;
   }

   _CopyStatAttributes(info,attr);
   attr->name = mPath;
   return true;
}

FileNode::Status Win32Directory::getStatus() const
{
   return mStatus;
}

void Win32Directory::_updateStatus()
{
   switch (::GetLastError())
   {
      case ERROR_NO_MORE_FILES:     mStatus = EndOfFile;        break;
      case ERROR_INVALID_ACCESS:    mStatus = AccessDenied;     break;
      case ERROR_PATH_NOT_FOUND:    mStatus = NoSuchFile;       break;
      case ERROR_SHARING_VIOLATION: mStatus = SharingViolation; break;
      case ERROR_ACCESS_DENIED:     mStatus = AccessDenied;     break;
      default:                      mStatus = UnknownError;     break;
   }
}

} // Namespace Win32

bool FS::VerifyWriteAccess(const Path &path)
{
   // due to UAC's habit of creating "virtual stores" when permission isn't actually available
   // actually create, write, read, verify, and delete a file to the folder being tested

   String temp = path.getFullPath();
   temp += "\\torque_writetest.tmp";

   // first, (try and) delete the file if it exists   
   ::DeleteFileW(temp.utf16());

   // now, create the file

   HANDLE hFile = ::CreateFileW(PathToOS(temp).utf16(),
               GENERIC_WRITE, 0,
               NULL, CREATE_ALWAYS,
               FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
               NULL);

   if ( hFile == INVALID_HANDLE_VALUE || hFile == NULL )
      return false;

   U32 t = Platform::getTime();

   DWORD bytesWritten;
   if (!::WriteFile(hFile,&t,sizeof(t),&bytesWritten,0))
   {
      ::CloseHandle(hFile);
      ::DeleteFileW(temp.utf16());
      return false;
   }

   // close the file
   ::CloseHandle(hFile);

   // open for read

   hFile = ::CreateFileW(PathToOS(temp).utf16(),
               GENERIC_READ, FILE_SHARE_READ,
               NULL, OPEN_EXISTING,
               FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
               NULL);

   if ( hFile == INVALID_HANDLE_VALUE || hFile == NULL )
      return false;

   U32 t2 = 0;

   DWORD bytesRead;
   if (!::ReadFile(hFile,&t2,sizeof(t2),&bytesRead,0))
   {
      ::CloseHandle(hFile);
      ::DeleteFileW(temp.utf16());
      return false;
   }

   ::CloseHandle(hFile);
   ::DeleteFileW(temp.utf16());

   return t == t2;
}


} // Namespace Torque

//-----------------------------------------------------------------------------

Torque::FS::FileSystemRef  Platform::FS::createNativeFS( const String &volume )
{
   return new Win32::Win32FileSystem( volume );
}

String   Platform::FS::getAssetDir()
{
   char cen_buf[2048];
#ifdef TORQUE_UNICODE
   if (!Platform::getWebDeployment())
   {
      TCHAR buf[ 2048 ];
      ::GetModuleFileNameW( NULL, buf, sizeof( buf ) );
      convertUTF16toUTF8( buf, cen_buf, sizeof( cen_buf ) );
   }
   else
   {
      TCHAR buf[ 2048 ];
      GetCurrentDirectoryW( sizeof( buf ) / sizeof( buf[ 0 ] ), buf );
      convertUTF16toUTF8( buf, cen_buf, sizeof( cen_buf ) );
      return Path::CleanSeparators(cen_buf);
   }
#else
   ::GetModuleFileNameA( NULL, cen_buf, 2047);
#endif

   char *delimiter = dStrrchr( cen_buf, '\\' );

   if( delimiter != NULL )
      *delimiter = '\0';

   return Path::CleanSeparators(cen_buf);
}

/// Function invoked by the kernel layer to install OS specific
/// file systems.
bool Platform::FS::InstallFileSystems()
{
   WCHAR buffer[1024];

   // [8/24/2009 tomb] This stops Windows from complaining about drives that have no disks in
   SetErrorMode(SEM_FAILCRITICALERRORS);

   // Load all the Win32 logical drives.
   DWORD mask = ::GetLogicalDrives();
   char drive[] = "A";
   char volume[] = "A:/";
   while (mask)
   {
      if (mask & 1)
      {
         volume[0] = drive[0];
         Platform::FS::Mount(drive, Platform::FS::createNativeFS(volume));
      }
      mask >>= 1;
      drive[0]++;
   }

   // Set the current working dir.  Windows normally returns
   // upper case driver letters, but the cygwin bash shell
   // seems to make it return lower case drives. Force upper
   // to be consistent with the mounts.
   ::GetCurrentDirectory(sizeof(buffer), buffer);

   if (buffer[1] == ':')
      buffer[0] = dToupper(buffer[0]);

   String   wd = buffer;
   
   wd += '/';

   Platform::FS::SetCwd(wd);

   return true;
}


