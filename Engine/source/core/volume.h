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

#ifndef _VOLUME_H_
#define _VOLUME_H_

#ifndef _TSIGNAL_H_
#include "core/util/tSignal.h"
#endif

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

#ifndef _REFBASE_H_
#include "core/util/refBase.h"
#endif

#ifndef _TDICTIONARY_H_
#include "core/util/tDictionary.h"
#endif

#ifndef _TORQUE_LIST_
#include "core/util/tList.h"
#endif

#ifndef _PATH_H_
#include "core/util/path.h"
#endif

#ifndef _TIMECLASS_H_
#include "core/util/timeClass.h"
#endif

namespace Torque
{
namespace FS
{

///@defgroup VolumeSystem Volume System
/// Volume access.

//-----------------------------------------------------------------------------

/// Base class for all FileIO objects.
/// @ingroup VolumeSystem
class FileBase : public StrongRefBase
{
public:
   virtual ~FileBase() {}
};


//-----------------------------------------------------------------------------

/// Base class for objects in a FileSystem.
/// This class provides the functionality required by all file system
/// files, which is basically name and attributes.
/// @ingroup VolumeSystem
class FileNode : public FileBase
{
public:
   enum Status
   {
      Open,                   ///< In an open state
      Closed,                 ///< In a closed state
      EndOfFile,              ///< End of file reached
      UnknownError,           ///< An undetermined error has occurred
      FileSystemFull,         ///< File system full
      NoSuchFile,             ///< File or path does not exist
      AccessDenied,           ///< File access denied
      IllegalCall,            ///< An unsupported operation was used.
      SharingViolation,       ///< File being used by another process
      NoDisk,                 ///< No disk or dvd in drive
      DriveOpen,              ///< Disk or DVD drive open
      WrongDisk,              ///< Disk or DVD has been swapped
   };

   enum Mode
   {
      File        = 1 << 0,   ///< Normal file
      Directory   = 1 << 1,   ///< Directory
      System      = 1 << 2,   ///< OS specific system file
      Hidden      = 1 << 3,   ///< Hidden file or directory
      ReadOnly    = 1 << 4,   ///< Read only
      Compressed  = 1 << 5,   ///< Part of a compressed archive?
      Encrypted   = 1 << 6,   ///< Part of an encrypted archive?
      Library     = 1 << 7,   ///< Dynamic Library
      Executable  = 1 << 8,   ///< Executable file
   };

   struct Attributes
   {
      U32      flags;   ///< File/Directory modes
      String   name;    ///< File/Directory name
      Time     mtime;   ///< Last modified time
      Time     atime;   ///< Last access time
      U64      size;
   };

public:
   FileNode();

   // Properties
   virtual Path   getName() const = 0;
   virtual Status getStatus() const = 0;

   virtual bool   getAttributes(Attributes*) = 0;

   // Convenience routines - may be overridden for optimal access
   virtual Time   getModifiedTime();   ///< @note This will return Time() on failure
   virtual U64    getSize();           ///< @note This will return 0 on failure
   virtual U32    getChecksum();       ///< @note This will return 0 on failure

protected:
   virtual U32    calculateChecksum() = 0;   ///< return 0 on failure

private:
   U32            mChecksum;
   Torque::Time   mLastChecksum;
};

typedef WeakRefPtr<FileNode> FileNodePtr;
typedef StrongRefPtr<FileNode>  FileNodeRef;


//-----------------------------------------------------------------------------

/// File object in a FileSystem.
/// File object in a FileSystem.  When a file is initially obtained from a
/// FileSystem it is in a closed state.
/// @ingroup VolumeSystem
class File : public FileNode
{
public:
   enum AccessMode
   {
      Read         = 0,    ///< Open for read only.
      Write        = 1,    ///< Open for write only.
      ReadWrite    = 2,    ///< Open for read-write.
      WriteAppend  = 3     ///< Write-only, starting at end of file.
   };

   enum SeekMode
   {
      Begin,               ///< Relative to the start of the file
      Current,             ///< Relative to the current position
      End,                 ///< Relative to the end of the file
   };

   File();
   virtual ~File();

   // Properties
   virtual U32 getPosition() = 0;
   virtual U32 setPosition(U32 pos, SeekMode mode) = 0;

   // Functions
   virtual bool open(AccessMode mode) = 0;
   virtual bool close() = 0;

   virtual U32 read(void* dst, U32 size) = 0;
   virtual U32 write(const void* src, U32 size) = 0;
};

typedef WeakRefPtr<File> FilePtr;
typedef StrongRefPtr<File>  FileRef;


//-----------------------------------------------------------------------------

/// Directory in a FileSystem.
/// Directory object in a FileSystem.  When a directory is initially obtained from a
/// FileSystem it is in a closed state.
/// @ingroup VolumeSystem
class Directory : public FileNode
{
public:
   Directory();
   virtual ~Directory();

   // Functions
   virtual bool open() = 0;
   virtual bool close() = 0;
   virtual bool read(Attributes*) = 0;   
};

typedef WeakRefPtr<Directory> DirectoryPtr;
typedef StrongRefPtr<Directory>  DirectoryRef;


//-----------------------------------------------------------------------------

class FileSystem;

class FileSystemChangeNotifier
{
public:
   typedef Delegate<void(const Path &)> ChangeDelegate;
   typedef  Signal<void(const Path &)> ChangeSignal;

public:
   FileSystemChangeNotifier( FileSystem *fs )
      :  mFS( fs ),
         mNotifying( false )
   {
   }

   virtual ~FileSystemChangeNotifier() {}

   /// Adds a file change notification.
   /// @see FS::AddChangeNotification
   virtual bool addNotification( const Path &path, ChangeDelegate callback );

   /// Removes an existing file change notification.
   /// @see FS::RemoveChangeNotification
   virtual bool removeNotification( const Path &path, ChangeDelegate callback );

   void startNotifier();
   void stopNotifier();

   /// Returns true if the notifier is enabled and file 
   /// change notifications will be sent.
   bool isNotifying() const { return mNotifying; }

protected:
   struct FileInfo
   {
      /// The full path to the file.
      Path filePath;

      /// The last known modification time.
      Time savedLastModTime;

      /// The notifications and reference count.
      ChangeSignal signal;
   };

   typedef List<FileInfo>           FileInfoList;
   typedef Map<Path, FileInfoList>  DirMap;  ///< map a directory to a list of files and their mod times

   void process();

   virtual void  internalProcessOnce() = 0;

   /// This is called so the inherited class can do its own bookkeeping on addNotification()
   /// @note We pass the directory here, not the file
   virtual bool  internalAddNotification( const Path &dir ) = 0;

   /// This is called so the inherited class can do its own bookkeeping on removeNotification()
   /// @note We pass the directory here, not the file
   virtual bool  internalRemoveNotification( const Path &dir ) = 0;

   /// Called by the inherited class to let us know a directory has changed
   /// so we can find the file which changed and notify on it
   void  internalNotifyDirChanged( const Path &dir );

   /// Makes sure paths going in and out of the notifier will have the same format
   String cleanPath(const Path& dir);

   FileSystem  *mFS;

   DirMap   mDirMap;

   bool  mNotifying;
};

//-----------------------------------------------------------------------------

/// Collection of FileNode objects.
/// File systems represent collections of FileNode objects. Functions are
/// provided for manipulating FileNode objects but the internal organization
/// and representation is opaque.
/// Path names must be fully specified relative to the file system root and
/// names cannot contain relative path information.
/// @ingroup VolumeSystem
class FileSystem : public FileBase
{
public:
   FileSystem();
   virtual ~FileSystem();

   virtual String   getTypeStr() const = 0; ///< Used for describing the file system type

   virtual FileNodeRef resolve(const Path& path) = 0;
   virtual FileNodeRef create(const Path& path,FileNode::Mode) = 0;
   virtual bool remove(const Path& path) = 0;
   virtual bool rename(const Path& a,const Path& b) = 0;
   virtual Path mapTo(const Path& path) = 0;
   virtual Path mapFrom(const Path& path) = 0;

   /// Returns the file change notifier.
   /// @see FS::AddChangeNotification
   /// @see FS::RemoveChangeNotification
   FileSystemChangeNotifier *getChangeNotifier() { return mChangeNotifier; }

   bool isReadOnly() { return mReadOnly; }

protected:
   FileSystemChangeNotifier   *mChangeNotifier;
   bool mReadOnly;
};

typedef WeakRefPtr<FileSystem> FileSystemPtr;
typedef StrongRefPtr<FileSystem>  FileSystemRef;


//-----------------------------------------------------------------------------
///@name File System Access
/// A collection of file systems.
/// @ingroup VolumeSystem
class MountSystem
{
public:
   virtual ~MountSystem() {}

   FileRef createFile(const Path& path);
   DirectoryRef createDirectory(const Path& path, FileSystemRef fs = NULL);
   virtual bool createPath(const Path& path);

   FileRef openFile(const Path& path,File::AccessMode mode);
   DirectoryRef openDirectory(const Path& path);

   bool remove(const Path& path);

   bool rename(const Path& from,const Path& to);

   virtual bool mount(String root, FileSystemRef fs);
   virtual bool mount(String root, const Path &path);
   virtual FileSystemRef unmount(String root);
   virtual bool unmount(FileSystemRef fs);

   bool setCwd(const Path& file);
   const Path &getCwd() const;

   FileSystemRef getFileSystem(const Path& path);
   bool getFileAttributes(const Path& path,FileNode::Attributes* attr);
   FileNodeRef getFileNode(const Path& path);

   bool  mapFSPath( const String &inRoot, const Path &inPath, Path &outPath );

   virtual S32 findByPattern( const Path &inBasePath, const String &inFilePattern, bool inRecursive, Vector<String> &outList, bool includeDirs=false, bool multiMatch = true );

   bool isFile(const Path &path);
   bool isDirectory(const Path &path, FileSystemRef fsRef = NULL);
   bool isReadOnly(const Path &path);

   S32      getNumMounts() const { return mMountList.size(); }
   String   getMountRoot( S32 index ) const { return mMountList[index].root; }
   String   getMountPath( S32 index ) const { return mMountList[index].fileSystem->mapTo(mMountList[index].path); }
   String   getMountType( S32 index ) const { return mMountList[index].fileSystem->getTypeStr(); }

   // File system notifications
   void  startFileChangeNotifications();
   void  stopFileChangeNotifications();

protected:
   virtual void _log(const String& msg);

protected:
   struct MountFS
   {
      String root;               // Root for file system
      String path;               // File system path
      FileSystemRef fileSystem;
   };
   
   virtual FileSystemRef _removeMountFromList(String root);
   virtual FileSystemRef _getFileSystemFromList(const Path& path) const ;
   void _setFindByPatternOverrideFS(FileSystemRef fs) { mFindByPatternOverrideFS = fs; } 

   Path _normalize(const Path& path);

   Vector<MountFS>   mMountList;
   Path        mCWD;
   FileSystemRef mFindByPatternOverrideFS;
};

///@name File System Access
/// Functions for mounting file systems and dealing with files and directories.
/// The kernel provides FileSystem mounting, the concept of a current working
/// directory as well as relative paths.
///@{

/// Mount file system
///@ingroup VolumeSystem
bool Mount(String root, FileSystemRef fs);

/// Mount file system redirect
///@ingroup VolumeSystem
bool Mount(String root, const Path &path);

/// Remove mounted file system.
/// The file system object associated with the given root is unmounted.
/// Open files associated with this file system are unaffected.
///@return The unmounted file system.
///@ingroup VolumeSystem
FileSystemRef Unmount(String root);

/// Remove mounted file system.
/// Open files associated with this file system are unaffected.
///@return true if the filesystem was successfully unmounted, false otherwise (most likely, the FS was not mounted)
bool Unmount(FileSystemRef fs);

/// Find the the file system which owns the given file.
///@ingroup VolumeSystem
FileSystemRef GetFileSystem(const Path &file);

/// Find the file system node for the given file.
///@return Null if the file doesn't exist
///@ingroup VolumeSystem
FileNodeRef GetFileNode(const Path &path);

/// Adds a file change notification callback.
///@ingroup VolumeSystem
template <class T,class U>
inline bool AddChangeNotification( const Path &path, T obj, U func )
{
   FileSystemRef fs = GetFileSystem( path );
   if ( !fs || !fs->getChangeNotifier() )
      return false;

   FileSystemChangeNotifier::ChangeDelegate dlg( obj, func );
   return fs->getChangeNotifier()->addNotification( path, dlg );
}

/// Removes an existing file change notification callback.
///@ingroup VolumeSystem
template <class T,class U>
inline bool RemoveChangeNotification( const Path &path, T obj, U func )
{
   FileSystemRef fs = GetFileSystem( path );
   if ( !fs || !fs->getChangeNotifier() )
      return false;

   FileSystemChangeNotifier::ChangeDelegate dlg( obj, func );
   return fs->getChangeNotifier()->removeNotification( path, dlg );
}

///  Map a real file system path to a virtual one based on a root.
///  This is useful when we get a real path back from an OS file dialog for example.
///  e.g.  If we have a root "gumby" which points at "C:/foo/bar",
///   MapFSPath("gumby", "C:/foo/bar/blat/picture.png", path ); 
///   will map "C:/foo/bar/blat/picture.png" to "gumby:/blat/picture.png"
///@param inRoot     The root to check
///@param inPath     The real file system path
///@param outPath    The resulting volume system path
///@return           Success or failure
bool  MapFSPath( const String &inRoot, const Path &inPath, Path &outPath );

/// Returns a true file system path without virtual mounts.
///
///@param inPath     The path to convert.
///@param outPath    The resulting real file system path.
///
bool GetFSPath( const Path &inPath, Path &outPath );

/// Find files matching a pattern starting in a given dir.
///@param inBasePath    path to start in
///@param inFilePattern the file pattern [it uses the FindMatch class]
///@param inRecursive   do we search recursively?
///@param outList       the list of files as Strings [Paths are more expensive to compute, so these may be converted on demand]
///@param multiMatch    match against multiple file patterns given in inFilePattern?
///@return number of files which matched
///@ingroup VolumeSystem
S32 FindByPattern( const Path &inBasePath, const String &inFilePattern, bool inRecursive, Vector<String> &outList, bool multiMatch = false );

/// Set current working directory.
///@ingroup VolumeSystem
bool SetCwd(const Path &file);

/// Get the current working directory.
///@ingroup VolumeSystem
const Path& GetCwd();

/// Remove (or delete) a file from the file system.
///@ingroup VolumeSystem
bool Remove(const Path &file);

/// Rename a file or directory.
///@ingroup VolumeSystem
bool Rename(const Path &from, const Path &to);

/// Get the file attributes.
/// @return success
///@ingroup VolumeSystem
bool GetFileAttributes(const Path &path, FileNode::Attributes *attr);

/// Compare modified times of p1 & p2
/// @return -1 if p1 < p2, 0 if p1 == p2, 1 if p1 > p2
S32 CompareModifiedTimes(const Path& p1, const Path& p2);

/// Open a file.
/// If the file exists a file object will be returned even if the
/// open operation fails.
///@return Null if the file does not exist
///@ingroup VolumeSystem
FileRef OpenFile(const Path &file, File::AccessMode mode);

/// Read in an entire file
///   @note Caller is responsible for freeing memory
///@param inPath  the file
///@param outData  the pointer to return the data
///@param outSize  the size of the data returned
///@param inNullTerminate  add an extra '\0' byte to the return buffer
///@return successful read?  If not, outData will be NULL and outSize will be 0
bool  ReadFile(const Path &inPath, void *&outData, U32 &outSize, bool inNullTerminate = false );

/// Open a directory.
/// If the directory exists a directory object will be returned even if the
/// open operation fails.
///@return Null if the file does not exist
///@ingroup VolumeSystem
DirectoryRef OpenDirectory(const Path &file);

/// Create a file.
/// The file object is returned in a closed state.
///@ingroup VolumeSystem
FileRef CreateFile(const Path &file);

/// Create a directory.
/// The directory object is returned in a closed state.
///@ingroup VolumeSystem
DirectoryRef CreateDirectory(const Path &file);

/// Create all the directories in the path if they don't already exist
///@ingroup VolumeSystem
bool CreatePath(const Path &path);

bool IsReadOnly(const Path &path);
bool IsDirectory(const Path &path);
bool IsFile(const Path &path);
bool VerifyWriteAccess(const Path &path);

/// This returns a unique file path from the components 
/// by appending numbers to the end of the file name if
/// a file with the same name already exists.
///
/// @param path The root and directory for the file.
/// @param fileName The file name without extension.
/// @param ext The dot-less extension.
String MakeUniquePath( const char *path, const char *fileName, const char *ext );

void  StartFileChangeNotifications();
void  StopFileChangeNotifications();

S32      GetNumMounts();
String   GetMountRoot( S32 index );
String   GetMountPath( S32 index );
String   GetMountType( S32 index );

///@}

} // Namespace FS
} // Namespace Torque

#endif

