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
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "core/crc.h"
#include "core/frameAllocator.h"

#include "core/util/str.h"
#include "core/strings/stringFunctions.h"

#include "platform/platformVolume.h"
#include "platformPOSIX/posixVolume.h"

#ifndef PATH_MAX
#include <sys/syslimits.h>
#endif

#ifndef NGROUPS_UMAX
   #define NGROUPS_UMAX 32
#endif


//#define DEBUG_SPEW


namespace Torque
{
namespace Posix
{

//-----------------------------------------------------------------------------

static String buildFileName(const String& prefix,const Path& path)
{
   // Need to join the path (minus the root) with our
   // internal path name.
   String file = prefix;
   file = Path::Join(file,'/',path.getPath());
   file = Path::Join(file,'/',path.getFileName());
   file = Path::Join(file,'.',path.getExtension());
   return file;
}

/*
static bool isFile(const String& file)
{
   struct stat info;
   if (stat(file.c_str(),&info) == 0)
     return S_ISREG(info.st_mode);
   return false;
}

static bool isDirectory(const String& file)
{
   struct stat info;
   if (stat(file.c_str(),&info) == 0)
     return S_ISDIR(info.st_mode);
   return false;
}
*/

//-----------------------------------------------------------------------------

static uid_t _Uid;                     // Current user id
static int _GroupCount;                // Number of groups in the table
static gid_t _Groups[NGROUPS_UMAX+1];  // Table of all the user groups

static void copyStatAttributes(const struct stat& info, FileNode::Attributes* attr)
{
   // We need to user and group id's in order to determin file
   // read-only access permission. This information is only retrieved
   // once per execution.
   if (!_Uid)
   {
      _Uid = getuid();
      _GroupCount = getgroups(NGROUPS_UMAX,_Groups);
      _Groups[_GroupCount++] = getegid();
   }

   // Fill in the return struct. The read-only flag is determined
   // by comparing file user and group ownership.
   attr->flags = 0;
   if (S_ISDIR(info.st_mode))
      attr->flags |= FileNode::Directory;
      
   if (S_ISREG(info.st_mode))
      attr->flags |= FileNode::File;
      
   if (info.st_uid == _Uid)
   {
      if (!(info.st_mode & S_IWUSR))
         attr->flags |= FileNode::ReadOnly;
   }
   else
   {
      S32 i = 0;
      for (; i < _GroupCount; i++)
      {
         if (_Groups[i] == info.st_gid)
            break;
      }
      if (i != _GroupCount)
      {
         if (!(info.st_mode & S_IWGRP))
            attr->flags |= FileNode::ReadOnly;
      }
      else
      {
         if (!(info.st_mode & S_IWOTH))
            attr->flags |= FileNode::ReadOnly;
      }
   }

   attr->size = info.st_size;
   attr->mtime = UnixTimeToTime(info.st_mtime);
   attr->atime = UnixTimeToTime(info.st_atime);
}


//-----------------------------------------------------------------------------

PosixFileSystem::PosixFileSystem(String volume)
{
   _volume = volume;
}

PosixFileSystem::~PosixFileSystem()
{
}

FileNodeRef PosixFileSystem::resolve(const Path& path)
{
   String file = buildFileName(_volume,path);
   struct stat info;
   if (stat(file.c_str(),&info) == 0)
   {
      // Construct the appropriate object
      if (S_ISREG(info.st_mode))
         return new PosixFile(path,file);
         
      if (S_ISDIR(info.st_mode))
         return new PosixDirectory(path,file);
   }
   
   return 0;
}

FileNodeRef PosixFileSystem::create(const Path& path, FileNode::Mode mode)
{
   // The file will be created on disk when it's opened.
   if (mode & FileNode::File)
      return new PosixFile(path,buildFileName(_volume,path));

   // Default permissions are read/write/search/executate by everyone,
   // though this will be modified by the current umask
   if (mode & FileNode::Directory)
   {
      String file = buildFileName(_volume,path);
      
      if (mkdir(file.c_str(),S_IRWXU | S_IRWXG | S_IRWXO))
         return new PosixDirectory(path,file);
   }
   
   return 0;
}

bool PosixFileSystem::remove(const Path& path)
{
   // Should probably check for outstanding files or directory objects.
   String file = buildFileName(_volume,path);

   struct stat info;
   int error = stat(file.c_str(),&info);
   if (error < 0)
      return false;

   if (S_ISDIR(info.st_mode))
      return !rmdir(file);
   
   return !unlink(file);
}

bool PosixFileSystem::rename(const Path& from,const Path& to)
{
   String fa = buildFileName(_volume,from);
   String fb = buildFileName(_volume,to);
   
   if (!rename(fa.c_str(),fb.c_str()))
      return true;
      
   return false;
}

Path PosixFileSystem::mapTo(const Path& path)
{
   return buildFileName(_volume,path);
}


Path PosixFileSystem::mapFrom(const Path& path)
{
   const String::SizeType  volumePathLen = _volume.length();

   String   pathStr = path.getFullPath();

   if ( _volume.compare( pathStr, volumePathLen, String::NoCase ))
      return Path();

   return pathStr.substr( volumePathLen, pathStr.length() - volumePathLen );
}

//-----------------------------------------------------------------------------

PosixFile::PosixFile(const Path& path,String name)
{
   _path = path;
   _name = name;
   _status = Closed;
   _handle = 0;
}

PosixFile::~PosixFile()
{
   if (_handle)
      close();
}

Path PosixFile::getName() const
{
   return _path;
}

FileNode::Status PosixFile::getStatus() const
{
   return _status;
}

bool PosixFile::getAttributes(Attributes* attr)
{
   struct stat info;
   int error = _handle? fstat(fileno(_handle),&info): stat(_name.c_str(),&info);
   
   if (error < 0)
   {
      _updateStatus();
      return false;
   }

   copyStatAttributes(info,attr);
   attr->name = _path;
   
   return true;
}

U32 PosixFile::calculateChecksum()
{
   if (!open( Read ))
      return 0;

   U64 fileSize = getSize();
   U32 bufSize = 1024 * 1024 * 4;
   FrameTemp< U8 > buf( bufSize );
   U32 crc = CRC::INITIAL_CRC_VALUE;

   while( fileSize > 0 )
   {
      U32 bytesRead = getMin( fileSize, bufSize );
      if( read( buf, bytesRead ) != bytesRead )
      {
         close();
         return 0;
      }
      
      fileSize -= bytesRead;
      crc = CRC::calculateCRC( buf, bytesRead, crc );
   }

   close();

   return crc;
}

bool PosixFile::open(AccessMode mode)
{
   close();
      
   if (_name.isEmpty())
   {
      return _status;
   }

   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[PosixFile] opening '%s'", _name.c_str() );
   #endif

   const char* fmode = "r";
   switch (mode)
   {
      case Read:        fmode = "r"; break;
      case Write:       fmode = "w"; break;
      case ReadWrite:
      {
         fmode = "r+";
         // Ensure the file exists.
         FILE* temp = fopen( _name.c_str(), "a+" );
         fclose( temp );
         break;
      }
      case WriteAppend: fmode = "a"; break;
      default:          break;
   }

   if (!(_handle = fopen(_name.c_str(), fmode)))
   {
      _updateStatus();
      return false;
   }
   
   _status = Open;
   return true;
}

bool PosixFile::close()
{
   if (_handle)
   {
      #ifdef DEBUG_SPEW
      Platform::outputDebugString( "[PosixFile] closing '%s'", _name.c_str() );
      #endif
      
      fflush(_handle);
      fclose(_handle);
      _handle = 0;
   }
   
   _status = Closed;
   return true;
}

U32 PosixFile::getPosition()
{
   if (_status == Open || _status == EndOfFile)
      return ftell(_handle);
      
   return 0;
}

U32 PosixFile::setPosition(U32 delta, SeekMode mode)
{
   if (_status != Open && _status != EndOfFile)
      return 0;

   S32 fmode = 0;
   switch (mode)
   {
      case Begin:    fmode = SEEK_SET; break;
      case Current:  fmode = SEEK_CUR; break;
      case End:      fmode = SEEK_END; break;
      default:       break;
   }
   
   if (fseek(_handle, delta, fmode))
   {
      _status = UnknownError;
      return 0;
   }
   
   _status = Open;
   
   return ftell(_handle);
}

U32 PosixFile::read(void* dst, U32 size)
{
   if (_status != Open && _status != EndOfFile)
      return 0;

   U32 bytesRead = fread(dst, 1, size, _handle);
   
   if (bytesRead != size)
   {
      if (feof(_handle))
         _status = EndOfFile;
      else
         _updateStatus();
   }
   
   return bytesRead;
}

U32 PosixFile::write(const void* src, U32 size)
{
   if ((_status != Open && _status != EndOfFile) || !size)
      return 0;

   U32 bytesWritten = fwrite(src, 1, size, _handle);
   
   if (bytesWritten != size)
      _updateStatus();
      
   return bytesWritten;
}

void PosixFile::_updateStatus()
{
   switch (errno)
   {
      case EACCES:   _status = AccessDenied;    break;
      case ENOSPC:   _status = FileSystemFull;  break;
      case ENOTDIR:  _status = NoSuchFile;      break;
      case ENOENT:   _status = NoSuchFile;      break;
      case EISDIR:   _status = AccessDenied;    break;
      case EROFS:    _status = AccessDenied;    break;
      default:       _status = UnknownError;    break;
   }
}

//-----------------------------------------------------------------------------

PosixDirectory::PosixDirectory(const Path& path,String name)
{
   _path = path;
   _name = name;
   _status = Closed;
   _handle = 0;
}

PosixDirectory::~PosixDirectory()
{
   if (_handle)
      close();
}

Path PosixDirectory::getName() const
{
   return _path;
}

bool PosixDirectory::open()
{
   if ((_handle = opendir(_name)) == 0)
   {
      _updateStatus();
      return false;
   }
   
   _status = Open;
   return true;
}

bool PosixDirectory::close()
{
   if (_handle)
   {
      closedir(_handle);
      _handle = NULL;
      return true;
   }
   
   return false;
}

bool PosixDirectory::read(Attributes* entry)
{
   if (_status != Open)
      return false;

   struct dirent* de = readdir(_handle);
   
   if (!de)
   {
      _status = EndOfFile;
      return false;
   }

   // Skip "." and ".." entries
   if (de->d_name[0] == '.' && (de->d_name[1] == '\0' ||
      (de->d_name[1] == '.' && de->d_name[2] == '\0')))
      return read(entry);

   // The dirent structure doesn't actually return much beside
   // the name, so we must call stat for more info.
   struct stat info;
   String file = _name + "/" + de->d_name;
   
   int error = stat(file.c_str(),&info);
   
   if (error < 0)
   {
      _updateStatus();
      return false;
   }
   copyStatAttributes(info,entry);
   entry->name = de->d_name;
   return true;
}

U32 PosixDirectory::calculateChecksum()
{
   // Return checksum of current entry
   return 0;
}

bool PosixDirectory::getAttributes(Attributes* attr)
{
   struct stat info;
   if (stat(_name.c_str(),&info))
   {
      _updateStatus();
      return false;
   }

   copyStatAttributes(info,attr);
   attr->name = _path;
   return true;
}

FileNode::Status PosixDirectory::getStatus() const
{
   return _status;
}

void PosixDirectory::_updateStatus()
{
   switch (errno)
   {
      case EACCES:   _status = AccessDenied; break;
      case ENOTDIR:  _status = NoSuchFile;   break;
      case ENOENT:   _status = NoSuchFile;   break;
      default:       _status = UnknownError; break;
   }
}

} // Namespace POSIX

} // Namespace Torque


//-----------------------------------------------------------------------------

#ifndef TORQUE_OS_MAC // Mac has its own native FS build on top of the POSIX one.

Torque::FS::FileSystemRef  Platform::FS::createNativeFS( const String &volume )
{
   return new Posix::PosixFileSystem( volume );
}

#endif

String   Platform::FS::getAssetDir()
{
   return Platform::getExecutablePath();
}

/// Function invoked by the kernel layer to install OS specific
/// file systems.
bool Platform::FS::InstallFileSystems()
{
   Platform::FS::Mount( "/", Platform::FS::createNativeFS( String() ) );

   // Setup the current working dir.
   char buffer[PATH_MAX];
   if (::getcwd(buffer,sizeof(buffer)))
   {
      // add trailing '/' if it isn't there
      if (buffer[dStrlen(buffer) - 1] != '/')
         dStrcat(buffer, "/");
         
      Platform::FS::SetCwd(buffer);
   }
   
   // Mount the home directory
   if (char* home = getenv("HOME"))
      Platform::FS::Mount( "home", Platform::FS::createNativeFS(home) );

   return true;
}
