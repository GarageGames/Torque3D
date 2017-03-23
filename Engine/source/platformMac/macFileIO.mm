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

#import <Cocoa/Cocoa.h>
#import <stdio.h>
#import <stdlib.h>
#import <errno.h>
#import <utime.h>
#import <sys/time.h>
#import <sys/types.h>
#import <dirent.h>
#import <unistd.h>
#import <sys/stat.h>

#import "core/fileio.h"
#import "core/util/tVector.h"
#import "core/stringTable.h"
#import "core/strings/stringFunctions.h"
#import "console/console.h"
#import "platform/profiler.h"
#import "cinterface/cinterface.h"
#import "core/volume.h"

//TODO: file io still needs some work...

#define MAX_MAC_PATH_LONG     2048

bool dFileDelete(const char * name)
{
   if(!name )
      return(false);
   
   if (dStrlen(name) > MAX_MAC_PATH_LONG)
      Con::warnf("dFileDelete: Filename length is pretty long...");
   
   return(remove(name) == 0); // remove returns 0 on success
}


//-----------------------------------------------------------------------------
bool dFileTouch(const char *path)
{
   if (!path || !*path)
      return false;
   
   // set file at path's modification and access times to now.
   return( utimes( path, NULL) == 0); // utimes returns 0 on success.
}
//-----------------------------------------------------------------------------
bool dPathCopy(const char* source, const char* dest, bool nooverwrite)
{
   if(source == NULL || dest == NULL)
      return false;
   
   @autoreleasepool {
      NSFileManager *manager = [NSFileManager defaultManager];
      
      NSString *nsource = [manager stringWithFileSystemRepresentation:source length:dStrlen(source)];
      NSString *ndest   = [manager stringWithFileSystemRepresentation:dest length:dStrlen(dest)];
      NSString *ndestFolder = [ndest stringByDeletingLastPathComponent];
      
      if(! [manager fileExistsAtPath:nsource])
      {
         Con::errorf("dPathCopy: no file exists at %s",source);
         return false;
      }
      
      if( [manager fileExistsAtPath:ndest] )
      {
         if(nooverwrite)
         {
            Con::errorf("dPathCopy: file already exists at %s",dest);
            return false;
         }
         Con::warnf("Deleting files at path: %s", dest);
         if(![manager removeItemAtPath:ndest error:nil] || [manager fileExistsAtPath:ndest])
         {
            Con::errorf("Copy failed! Could not delete files at path: %s", dest);
            return false;
         }
      }
      
      if([manager fileExistsAtPath:ndestFolder] == NO)
      {
         ndestFolder = [ndestFolder stringByAppendingString:@"/"]; // createpath requires a trailing slash
         Platform::createPath([ndestFolder UTF8String]);
      }
      
      bool ret = [manager copyItemAtPath:nsource toPath:ndest error:nil];
      // n.b.: The "success" semantics don't guarantee a copy actually took place, so we'll verify
      // because this is surprising behavior for a method called copy.
      if( ![manager fileExistsAtPath:ndest] )
      {
         Con::warnf("The filemanager returned success, but the file was not copied. Something strange is happening");
         ret = false;
      }
      return ret;
   }
   
}

//-----------------------------------------------------------------------------

bool dFileRename(const char *source, const char *dest)
{
   if(source == NULL || dest == NULL)
      return false;
   
   @autoreleasepool {
      NSFileManager *manager = [NSFileManager defaultManager];
      
      NSString *nsource = [manager stringWithFileSystemRepresentation:source length:dStrlen(source)];
      NSString *ndest   = [manager stringWithFileSystemRepresentation:dest length:dStrlen(dest)];
      
      if(! [manager fileExistsAtPath:nsource])
      {
         Con::errorf("dFileRename: no file exists at %s",source);
         return false;
      }
      
      if( [manager fileExistsAtPath:ndest] )
      {
         Con::warnf("dFileRename: Deleting files at path: %s", dest);
      }
      
      bool ret = [manager moveItemAtPath:nsource toPath:ndest error:nil];
      // n.b.: The "success" semantics don't guarantee a move actually took place, so we'll verify
      // because this is surprising behavior for a method called rename.
      
      if( ![manager fileExistsAtPath:ndest] )
      {
         Con::warnf("The filemanager returned success, but the file was not moved. Something strange is happening");
         ret = false;
      }
      
      return ret;
   }
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
   handle = NULL;
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
   handle = NULL;
}


//-----------------------------------------------------------------------------
// Open a file in the mode specified by openMode (Read, Write, or ReadWrite).
// Truncate the file if the mode is either Write or ReadWrite and truncate is
// true.
//
// Sets capability appropriate to the openMode.
// Returns the currentStatus of the file.
//-----------------------------------------------------------------------------
File::FileStatus File::open(const char *filename, const AccessMode openMode)
{
   if (dStrlen(filename) > MAX_MAC_PATH_LONG)
      Con::warnf("File::open: Filename length is pretty long...");
   
   // Close the file if it was already open...
   if (currentStatus != Closed)
      close();
   
   // create the appropriate type of file...
   switch (openMode)
   {
      case Read:
         handle = (void *)fopen(filename, "rb"); // read only
         break;
      case Write:
         handle = (void *)fopen(filename, "wb"); // write only
         break;
      case ReadWrite:
         handle = (void *)fopen(filename, "ab+"); // write(append) and read
         break;
      case WriteAppend:
         handle = (void *)fopen(filename, "ab"); // write(append) only
         break;
      default:
         AssertFatal(false, "File::open: bad access mode");
   }
   
   // handle not created successfully
   if (handle == NULL)
      return setStatus();
   
   // successfully created file, so set the file capabilities...
   switch (openMode)
   {
      case Read:
         capability = FileRead;
         break;
      case Write:
      case WriteAppend:
         capability = FileWrite;
         break;
      case ReadWrite:
         capability = FileRead | FileWrite;
         break;
      default:
         AssertFatal(false, "File::open: bad access mode");
   }
   
   // must set the file status before setting the position.
   currentStatus = Ok;
   
   if (openMode == ReadWrite)
      setPosition(0);
   
   // success!
   return currentStatus;
}

//-----------------------------------------------------------------------------
// Get the current position of the file pointer.
//-----------------------------------------------------------------------------
U32 File::getPosition() const
{
   AssertFatal(currentStatus != Closed , "File::getPosition: file closed");
   AssertFatal(handle != NULL, "File::getPosition: invalid file handle");
   
   return ftell((FILE*)handle);
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
File::FileStatus File::setPosition(S32 position, bool absolutePos)
{
   AssertFatal(Closed != currentStatus, "File::setPosition: file closed");
   AssertFatal(handle != NULL, "File::setPosition: invalid file handle");
   
   if (currentStatus != Ok && currentStatus != EOS )
      return currentStatus;
   
   U32 finalPos;
   if(absolutePos)
   {
      // absolute position
      AssertFatal(0 <= position, "File::setPosition: negative absolute position");
      // position beyond EOS is OK
      fseek((FILE*)handle, position, SEEK_SET);
      finalPos = ftell((FILE*)handle);
   }
   else
   {
      // relative position
      AssertFatal((getPosition() + position) >= 0, "File::setPosition: negative relative position");
      // position beyond EOS is OK
      fseek((FILE*)handle, position, SEEK_CUR);
      finalPos = ftell((FILE*)handle);
   }
   
   // ftell returns -1 on error. set error status
   if (0xffffffff == finalPos)
      return setStatus();
   
   // success, at end of file
   else if (finalPos >= getSize())
      return currentStatus = EOS;
   
   // success!
   else
      return currentStatus = Ok;
}

//-----------------------------------------------------------------------------
// Get the size of the file in bytes.
// It is an error to query the file size for a Closed file, or for one with an
// error status.
//-----------------------------------------------------------------------------
U32 File::getSize() const
{
   AssertWarn(Closed != currentStatus, "File::getSize: file closed");
   AssertFatal(handle != NULL, "File::getSize: invalid file handle");
   
   if (Ok == currentStatus || EOS == currentStatus)
   {
      struct stat statData;
      
      if(fstat(fileno((FILE*)handle), &statData) != 0)
         return 0;
      
      // return the size in bytes
      return statData.st_size;
   }
   
   return 0;
}

//-----------------------------------------------------------------------------
// Flush the file.
// It is an error to flush a read-only file.
// Returns the currentStatus of the file.
//-----------------------------------------------------------------------------
File::FileStatus File::flush()
{
   AssertFatal(Closed != currentStatus, "File::flush: file closed");
   AssertFatal(handle != NULL, "File::flush: invalid file handle");
   AssertFatal(true == hasCapability(FileWrite), "File::flush: cannot flush a read-only file");
   
   if (fflush((FILE*)handle) != 0)
      return setStatus();
   else
      return currentStatus = Ok;
}

//-----------------------------------------------------------------------------
// Close the File.
//
// Returns the currentStatus
//-----------------------------------------------------------------------------
File::FileStatus File::close()
{
   // check if it's already closed...
   if (Closed == currentStatus)
      return currentStatus;
   
   // it's not, so close it...
   if (handle != NULL)
   {
      if (fclose((FILE*)handle) != 0)
         return setStatus();
   }
   handle = NULL;
   return currentStatus = Closed;
}

//-----------------------------------------------------------------------------
// Self-explanatory.
//-----------------------------------------------------------------------------
File::FileStatus File::getStatus() const
{
   return currentStatus;
}

//-----------------------------------------------------------------------------
// Sets and returns the currentStatus when an error has been encountered.
//-----------------------------------------------------------------------------
File::FileStatus File::setStatus()
{
   switch (errno)
   {
      case EACCES:   // permission denied
         currentStatus = IOError;
         break;
      case EBADF:   // Bad File Pointer
      case EINVAL:   // Invalid argument
      case ENOENT:   // file not found
      case ENAMETOOLONG:
      default:
         currentStatus = UnknownError;
   }
   
   return currentStatus;
}

//-----------------------------------------------------------------------------
// Sets and returns the currentStatus to status.
//-----------------------------------------------------------------------------
File::FileStatus File::setStatus(File::FileStatus status)
{
   return currentStatus = status;
}

//-----------------------------------------------------------------------------
// Read from a file.
// The number of bytes to read is passed in size, the data is returned in src.
// The number of bytes read is available in bytesRead if a non-Null pointer is
// provided.
//-----------------------------------------------------------------------------
File::FileStatus File::read(U32 size, char *dst, U32 *bytesRead)
{
   AssertFatal(Closed != currentStatus, "File::read: file closed");
   AssertFatal(handle != NULL, "File::read: invalid file handle");
   AssertFatal(NULL != dst, "File::read: NULL destination pointer");
   AssertFatal(true == hasCapability(FileRead), "File::read: file lacks capability");
   AssertWarn(0 != size, "File::read: size of zero");
   
   if (Ok != currentStatus || 0 == size)
      return currentStatus;
   
   // read from stream
   U32 nBytes = fread(dst, 1, size, (FILE*)handle);
   
   // did we hit the end of the stream?
   if( nBytes != size)
      currentStatus = EOS;
   
   // if bytesRead is a valid pointer, send number of bytes read there.
   if(bytesRead)
      *bytesRead = nBytes;
   
   // successfully read size bytes
   return currentStatus;
}

//-----------------------------------------------------------------------------
// Write to a file.
// The number of bytes to write is passed in size, the data is passed in src.
// The number of bytes written is available in bytesWritten if a non-Null
// pointer is provided.
//-----------------------------------------------------------------------------
File::FileStatus File::write(U32 size, const char *src, U32 *bytesWritten)
{
   AssertFatal(Closed != currentStatus, "File::write: file closed");
   AssertFatal(handle != NULL, "File::write: invalid file handle");
   AssertFatal(NULL != src, "File::write: NULL source pointer");
   AssertFatal(true == hasCapability(FileWrite), "File::write: file lacks capability");
   AssertWarn(0 != size, "File::write: size of zero");
   
   if ((Ok != currentStatus && EOS != currentStatus) || 0 == size)
      return currentStatus;
   
   // write bytes to the stream
   U32 nBytes = fwrite(src, 1, size,(FILE*)handle);
   
   // if we couldn't write everything, we've got a problem. set error status.
   if(nBytes != size)
      setStatus();
   
   // if bytesWritten is a valid pointer, put number of bytes read there.
   if(bytesWritten)
      *bytesWritten = nBytes;
   
   // return current File status, whether good or ill.
   return currentStatus;
}


//-----------------------------------------------------------------------------
// Self-explanatory.
//-----------------------------------------------------------------------------
bool File::hasCapability(Capability cap) const
{
   return (0 != (U32(cap) & capability));
}

//-----------------------------------------------------------------------------
S32 Platform::compareFileTimes(const FileTime &a, const FileTime &b)
{
   if(a > b)
      return 1;
   if(a < b)
      return -1;
   return 0;
}


//-----------------------------------------------------------------------------
// either time param COULD be null.
//-----------------------------------------------------------------------------
bool Platform::getFileTimes(const char *path, FileTime *createTime, FileTime *modifyTime)
{
   // MacOSX is NOT guaranteed to be running off a HFS volume,
   // and UNIX does not keep a record of a file's creation time anywhere.
   // So instead of creation time we return changed time,
   // just like the Linux platform impl does.
   
   if (!path || !*path)
      return false;
   
   struct stat statData;
   
   if (stat(path, &statData) == -1)
      return false;
   
   if(createTime)
      *createTime = statData.st_ctime;
   
   if(modifyTime)
      *modifyTime = statData.st_mtime;
   
   return true;
}


//-----------------------------------------------------------------------------
bool Platform::createPath(const char *file)
{
   // if the path exists, we're done.
   struct stat statData;
   if( stat(file, &statData) == 0 )
   {
      return true;               // exists, rejoice.
   }
   
   Con::warnf( "creating path %s", file );
   
   // get the parent path.
   // we're not using basename because it's not thread safe.
   U32 len = dStrlen(file);
   char parent[len];
   bool isDirPath = false;
   
   dStrncpy(parent,file,len);
   parent[len] = '\0';
   if(parent[len - 1] == '/')
   {
      parent[len - 1] = '\0';    // cut off the trailing slash, if there is one
      isDirPath = true;          // we got a trailing slash, so file is a directory.
   }
   
   // recusively create the parent path.
   // only recurse if newpath has a slash that isn't a leading slash.
   char *slash = dStrrchr(parent,'/');
   if( slash  && slash != parent)
   {
      // snip the path just after the last slash.
      slash[1] = '\0';
      // recusively create the parent path. fail if parent path creation failed.
      if(!Platform::createPath(parent))
         return false;
   }
   
   // create *file if it is a directory path.
   if(isDirPath)
   {
      // try to create the directory
      if( mkdir(file, 0777) != 0) // app may reside in global apps dir, and so must be writable to all.
         return false;
   }
   
   return true;
}


//-----------------------------------------------------------------------------
bool Platform::cdFileExists(const char *filePath, const char *volumeName, S32 serialNum)
{
   return true;
}

#pragma mark ---- Directories ----
//-----------------------------------------------------------------------------
StringTableEntry Platform::getCurrentDirectory()
{
   // get the current directory, the one that would be opened if we did a fopen(".")
   char* cwd = getcwd(NULL, 0);
   StringTableEntry ret = StringTable->insert(cwd);
   free(cwd);
   return ret;
}

//-----------------------------------------------------------------------------
bool Platform::setCurrentDirectory(StringTableEntry newDir)
{
   return (chdir(newDir) == 0);
}

//-----------------------------------------------------------------------------

// helper func for getWorkingDirectory
bool isMainDotCsPresent(NSString* dir)
{
   return [[NSFileManager defaultManager] fileExistsAtPath:[dir stringByAppendingPathComponent:@"main.cs"]] == YES;
}

//-----------------------------------------------------------------------------
/// Finds and sets the current working directory.
/// Torque tries to automatically detect whether you have placed the game files
/// inside or outside the application's bundle. It checks for the presence of
/// the file 'main.cs'. If it finds it, Torque will assume that the other game
/// files are there too. If Torque does not see 'main.cs' inside its bundle, it
/// will assume the files are outside the bundle.
/// Since you probably don't want to copy the game files into the app every time
/// you build, you will want to leave them outside the bundle for development.
///
/// Placing all content inside the application bundle gives a much better user
/// experience when you distribute your app.
StringTableEntry Platform::getExecutablePath()
{
   static const char* cwd = NULL;
   
   // this isn't actually being used due to some static constructors at bundle load time
   // calling this method (before there is a chance to set it)
   // for instance, FMOD sound provider (this should be fixed in FMOD as it is with windows)
   if (!cwd && torque_getexecutablepath())
   {
      // we're in a plugin using the cinterface
      cwd = torque_getexecutablepath();
      chdir(cwd);
   }
   else if(!cwd)
   {
      NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
      
      //first check the cwd for main.cs
      static char buf[4096];
      NSString* currentDir = [[NSString alloc ] initWithUTF8String:getcwd(buf,(4096 * sizeof(char))) ];
      
      if (isMainDotCsPresent(currentDir))
      {
         cwd = buf;
         [pool release];
         [currentDir release];
         return cwd;
      }
      
      NSString* string = [[NSBundle mainBundle] pathForResource:@"main" ofType:@"cs"];
      if(!string)
         string = [[NSBundle mainBundle] bundlePath];
      
      string = [string stringByDeletingLastPathComponent];
      AssertISV(isMainDotCsPresent(string), "Platform::getExecutablePath - Failed to find main.cs!");
      cwd = dStrdup([string UTF8String]);
      chdir(cwd);
      [pool release];
      [currentDir release];
   }
   
   return cwd;
}

//-----------------------------------------------------------------------------
StringTableEntry Platform::getExecutableName()
{
   static const char* name = NULL;
   if(!name)
      name = [[[[NSBundle mainBundle] bundlePath] lastPathComponent] UTF8String];
   
   return name;
}

//-----------------------------------------------------------------------------
bool Platform::isFile(const char *path)
{
   if (!path || !*path)
      return false;
   
   // make sure we can stat the file
   struct stat statData;
   if( stat(path, &statData) < 0 )
   {
      // Since file does not exist on disk see if it exists in a zip file loaded
      return Torque::FS::IsFile(path);
   }
   
   // now see if it's a regular file
   if( (statData.st_mode & S_IFMT) == S_IFREG)
      return true;
   
   return false;
}


//-----------------------------------------------------------------------------
bool Platform::isDirectory(const char *path)
{
   if (!path || !*path)
      return false;
   
   // make sure we can stat the file
   struct stat statData;
   if( stat(path, &statData) < 0 )
      return false;
   
   // now see if it's a directory
   if( (statData.st_mode & S_IFMT) == S_IFDIR)
      return true;
   
   return false;
}


S32 Platform::getFileSize(const char* pFilePath)
{
   if (!pFilePath || !*pFilePath)
      return 0;
   
   struct stat statData;
   if( stat(pFilePath, &statData) < 0 )
      return 0;
   
   // and return it's size in bytes
   return (S32)statData.st_size;
}


//-----------------------------------------------------------------------------
bool Platform::isSubDirectory(const char *pathParent, const char *pathSub)
{
   char fullpath[MAX_MAC_PATH_LONG];
   dStrcpyl(fullpath, MAX_MAC_PATH_LONG, pathParent, "/", pathSub, NULL);
   return isDirectory((const char *)fullpath);
}

//-----------------------------------------------------------------------------
// utility for platform::hasSubDirectory() and platform::dumpDirectories()
// ensures that the entry is a directory, and isnt on the ignore lists.
inline bool isGoodDirectory(dirent* entry)
{
   return (entry->d_type == DT_DIR                          // is a dir
         && dStrcmp(entry->d_name,".") != 0                 // not here
         && dStrcmp(entry->d_name,"..") != 0                // not parent
         && !Platform::isExcludedDirectory(entry->d_name)); // not excluded
}

//-----------------------------------------------------------------------------
bool Platform::hasSubDirectory(const char *path)
{
   DIR *dir;
   dirent *entry;
   
   dir = opendir(path);
   if(!dir)
      return false; // we got a bad path, so no, it has no subdirectory.
   
   while( (entry = readdir(dir)) )
   {
      if(isGoodDirectory(entry) )
      {
         closedir(dir);
         return true; // we have a subdirectory, that isnt on the exclude list.
      }
   }
   
   closedir(dir);
   return false; // either this dir had no subdirectories, or they were all on the exclude list.
}

bool Platform::fileDelete(const char * name)
{
   return dFileDelete(name);
}

static bool recurseDumpDirectories(const char *basePath, const char *subPath, Vector<StringTableEntry> &directoryVector, S32 currentDepth, S32 recurseDepth, bool noBasePath)
{
   char Path[1024];
   DIR *dip;
   struct dirent *d;
   
   dsize_t trLen = basePath ? dStrlen(basePath) : 0;
   dsize_t subtrLen = subPath ? dStrlen(subPath) : 0;
   char trail = trLen > 0 ? basePath[trLen - 1] : '\0';
   char subTrail = subtrLen > 0 ? subPath[subtrLen - 1] : '\0';
   
   if (trail == '/')
   {
      if (subPath && (dStrncmp(subPath, "", 1) != 0))
      {
         if (subTrail == '/')
            dSprintf(Path, 1024, "%s%s", basePath, subPath);
         else
            dSprintf(Path, 1024, "%s%s/", basePath, subPath);
      }
      else
         dSprintf(Path, 1024, "%s", basePath);
   }
   else
   {
      if (subPath && (dStrncmp(subPath, "", 1) != 0))
      {
         if (subTrail == '/')
            dSprintf(Path, 1024, "%s%s", basePath, subPath);
         else
            dSprintf(Path, 1024, "%s%s/", basePath, subPath);
      }
      else
         dSprintf(Path, 1024, "%s/", basePath);
   }
   
   dip = opendir(Path);
   if (dip == NULL)
      return false;
   
   //////////////////////////////////////////////////////////////////////////
   // add path to our return list ( provided it is valid )
   //////////////////////////////////////////////////////////////////////////
   if (!Platform::isExcludedDirectory(subPath))
   {
      if (noBasePath)
      {
         // We have a path and it's not an empty string or an excluded directory
         if ( (subPath && (dStrncmp (subPath, "", 1) != 0)) )
            directoryVector.push_back(StringTable->insert(subPath));
      }
      else
      {
         if ( (subPath && (dStrncmp(subPath, "", 1) != 0)) )
         {
            char szPath[1024];
            dMemset(szPath, 0, 1024);
            if (trail == '/')
            {
               if ((basePath[dStrlen(basePath) - 1]) != '/')
                  dSprintf(szPath, 1024, "%s%s", basePath, &subPath[1]);
               else
                  dSprintf(szPath, 1024, "%s%s", basePath, subPath);
            }
            else
            {
               if ((basePath[dStrlen(basePath) - 1]) != '/')
                  dSprintf(szPath, 1024, "%s%s", basePath, subPath);
               else
                  dSprintf(szPath, 1024, "%s/%s", basePath, subPath);
            }
            
            directoryVector.push_back(StringTable->insert(szPath));
         }
         else
            directoryVector.push_back(StringTable->insert(basePath));
      }
   }
   //////////////////////////////////////////////////////////////////////////
   // Iterate through and grab valid directories
   //////////////////////////////////////////////////////////////////////////
   
   while (d = readdir(dip))
   {
      bool  isDir;
      isDir = false;
      if (d->d_type == DT_UNKNOWN)
      {
         char child [1024];
         if ((Path[dStrlen(Path) - 1] == '/'))
            dSprintf(child, 1024, "%s%s", Path, d->d_name);
         else
            dSprintf(child, 1024, "%s/%s", Path, d->d_name);
         isDir = Platform::isDirectory (child);
      }
      else if (d->d_type & DT_DIR)
         isDir = true;
      
      if ( isDir )
      {
         if (dStrcmp(d->d_name, ".") == 0 ||
            dStrcmp(d->d_name, "..") == 0)
            continue;
         if (Platform::isExcludedDirectory(d->d_name))
            continue;
         if ( (subPath && (dStrncmp(subPath, "", 1) != 0)) )
         {
            char child[1024];
            if ((subPath[dStrlen(subPath) - 1] == '/'))
               dSprintf(child, 1024, "%s%s", subPath, d->d_name);
            else
               dSprintf(child, 1024, "%s/%s", subPath, d->d_name);
            if (currentDepth < recurseDepth || recurseDepth == -1 )
               recurseDumpDirectories(basePath, child, directoryVector,
                                 currentDepth + 1, recurseDepth,
                                 noBasePath);
         }
         else
         {
            char child[1024];
            if ( (basePath[dStrlen(basePath) - 1]) == '/')
               dStrcpy (child, d->d_name);
            else
               dSprintf(child, 1024, "/%s", d->d_name);
            if (currentDepth < recurseDepth || recurseDepth == -1)
               recurseDumpDirectories(basePath, child, directoryVector,
                                 currentDepth + 1, recurseDepth,
                                 noBasePath);
         }
      }
   }
   closedir(dip);
   return true;
}

//-----------------------------------------------------------------------------
bool Platform::dumpDirectories(const char *path, Vector<StringTableEntry> &directoryVector, S32 depth, bool noBasePath)
{
   bool retVal = recurseDumpDirectories(path, "", directoryVector, 0, depth, noBasePath);
   clearExcludedDirectories();
   return retVal;
}

//-----------------------------------------------------------------------------
static bool recurseDumpPath(const char* curPath, Vector<Platform::FileInfo>& fileVector, U32 depth)
{
   DIR *dir;
   dirent *entry;
   
   // be sure it opens.
   dir = opendir(curPath);
   if(!dir)
      return false;
   
   // look inside the current directory
   while( (entry = readdir(dir)) )
   {
      // construct the full file path. we need this to get the file size and to recurse
      U32 len = dStrlen(curPath) + entry->d_namlen + 2;
      char pathbuf[len];
      dSprintf( pathbuf, len, "%s/%s", curPath, entry->d_name);
      pathbuf[len] = '\0';
      
      // ok, deal with directories and files seperately.
      if( entry->d_type == DT_DIR )
      {
         if( depth == 0)
            continue;
         
         // filter out dirs we dont want.
         if( !isGoodDirectory(entry) )
            continue;
         
         // recurse into the dir
         recurseDumpPath( pathbuf, fileVector, depth-1);
      }
      else
      {
         //add the file entry to the list
         // unlike recurseDumpDirectories(), we need to return more complex info here.
         U32 fileSize = Platform::getFileSize(pathbuf);
         fileVector.increment();
         Platform::FileInfo& rInfo = fileVector.last();
         rInfo.pFullPath = StringTable->insert(curPath);
         rInfo.pFileName = StringTable->insert(entry->d_name);
         rInfo.fileSize  = fileSize;
      }
   }
   closedir(dir);
   return true;
   
}


//-----------------------------------------------------------------------------
bool Platform::dumpPath(const char *path, Vector<Platform::FileInfo>& fileVector, S32 depth)
{
   PROFILE_START(dumpPath);
   int len = dStrlen(path);
   char newpath[len+1];
   
   dStrncpy(newpath,path,len);
   newpath[len] = '\0'; // null terminate
   if(newpath[len - 1] == '/')
      newpath[len - 1] = '\0'; // cut off the trailing slash, if there is one
   
   bool ret = recurseDumpPath( newpath, fileVector, depth);
   PROFILE_END();
   
   return ret;
}

// TODO: implement stringToFileTime()
bool Platform::stringToFileTime(const char * string, FileTime * time) { return false;}
// TODO: implement fileTimeToString()
bool Platform::fileTimeToString(FileTime * time, char * string, U32 strLen) { return false;}

//-----------------------------------------------------------------------------
#if defined(TORQUE_DEBUG)
ConsoleFunction(testHasSubdir,void,2,2,"tests platform::hasSubDirectory") {
   Con::printf("testing %s",(const char*)argv[1]);
   Platform::addExcludedDirectory(".svn");
   if(Platform::hasSubDirectory(argv[1]))
      Con::printf(" has subdir");
   else
      Con::printf(" does not have subdir");
}

ConsoleFunction(testDumpDirectories,void,4,4,"testDumpDirectories('path', int depth, bool noBasePath)") {
   Vector<StringTableEntry> paths;
   const S32 depth = dAtoi(argv[2]);
   const bool noBasePath = dAtob(argv[3]);
   
   Platform::addExcludedDirectory(".svn");
   
   Platform::dumpDirectories(argv[1], paths, depth, noBasePath);
   
   Con::printf("Dumping directories starting from %s with depth %i", (const char*)argv[1],depth);
   
   for(Vector<StringTableEntry>::iterator itr = paths.begin(); itr != paths.end(); itr++) {
      Con::printf(*itr);
   }
   
}

ConsoleFunction(testDumpPaths, void, 3, 3, "testDumpPaths('path', int depth)")
{
   Vector<Platform::FileInfo> files;
   S32 depth = dAtoi(argv[2]);
   
   Platform::addExcludedDirectory(".svn");
   
   Platform::dumpPath(argv[1], files, depth);
   
   for(Vector<Platform::FileInfo>::iterator itr = files.begin(); itr != files.end(); itr++) {
      Con::printf("%s/%s",itr->pFullPath, itr->pFileName);
   }
}

//-----------------------------------------------------------------------------
ConsoleFunction(testFileTouch, bool , 2,2, "testFileTouch('path')")
{
   return dFileTouch(argv[1]);
}

ConsoleFunction(testGetFileTimes, bool, 2,2, "testGetFileTimes('path')")
{
   FileTime create, modify;
   bool ok = Platform::getFileTimes(argv[1], &create, &modify);
   Con::printf("%s Platform::getFileTimes %i, %i", ok ? "+OK" : "-FAIL", create, modify);
   return ok;
}

#endif
