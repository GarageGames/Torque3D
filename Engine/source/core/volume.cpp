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

#include "platform/platform.h"
#include "core/volume.h"

#include "core/virtualMountSystem.h"
#include "core/strings/findMatch.h"
#include "core/util/journal/process.h"
#include "core/util/safeDelete.h"
#include "console/console.h"


namespace Torque
{
using namespace FS;

//-----------------------------------------------------------------------------

bool FileSystemChangeNotifier::addNotification( const Path &path, ChangeDelegate callback )
{
   // Notifications are for files... if the path is empty 
   // then there is nothing to do. 
   if ( path.isEmpty() )
      return false;

   // strip the filename and extension - we notify on dirs
   Path dir(cleanPath(path));
   if (dir.isEmpty())
      return false;

   dir.setFileName( String() );
   dir.setExtension( String () );
   
   // first lookup the dir to see if we already have an entry for it...
   DirMap::Iterator   itr = mDirMap.find( dir );

   FileInfoList   *fileList = NULL;
   bool           addedDir = false;

   // Note that GetFileAttributes can fail here if the file doesn't 
   // exist, but thats ok and expected.  You can have notifications 
   // on files that don't exist yet.
   FileNode::Attributes attr;
   GetFileAttributes(path, &attr);

   if ( itr != mDirMap.end() )
   {
      fileList = &(itr->value);

      // look for the file and return if we find it
      for ( U32 i = 0; i < fileList->getSize(); i++ )
      {
         FileInfo &fInfo = (*fileList)[i];
         if ( fInfo.filePath == path )
         {
            // NOTE: This is bad... we should store the mod
            // time for each callback seperately in the future.
            //
            fInfo.savedLastModTime = attr.mtime;
            fInfo.signal.notify(callback);
            return true;
         }
      }
   }
   else
   {
      // otherwise we need to add the dir to our map and let the inherited class add it
      itr = mDirMap.insert( dir, FileInfoList() );

      fileList = &(itr->value);

      addedDir = true;
   }

   FileInfo newInfo;
   newInfo.signal.notify(callback);
   newInfo.filePath = path;
   newInfo.savedLastModTime = attr.mtime;

   fileList->pushBack( newInfo );

   return addedDir ? internalAddNotification( dir ) : true;
}

bool FileSystemChangeNotifier::removeNotification( const Path &path, ChangeDelegate callback )
{
   if (path.isEmpty())
      return false;

   // strip the filename and extension - we notify on dirs
   Path dir(cleanPath(path));
   if (dir.isEmpty())
      return false;

   dir.setFileName( String() );
   dir.setExtension( String () );

   DirMap::Iterator   itr = mDirMap.find( dir );

   if ( itr == mDirMap.end() )
      return false;

   FileInfoList   &fileList = itr->value;

   // look for the file and return if we find it
   for ( U32 i = 0; i < fileList.getSize(); i++ )
   {
      FileInfo &fInfo = fileList[i];
      if ( fInfo.filePath == path )
      {
         fInfo.signal.remove(callback);
         if (fInfo.signal.isEmpty())
            fileList.erase( i );
         break;
      }
   }

   // IF we removed the last file
   //    THEN get rid of the dir from our map and notify inherited classes
   if ( fileList.getSize() == 0 )
   {
      mDirMap.erase( dir );

      return internalRemoveNotification( dir );
   }

   return true;
}

void  FileSystemChangeNotifier::startNotifier()
{
   // update the timestamps of all the files we are managing

   DirMap::Iterator   itr = mDirMap.begin();

   for ( ; itr != mDirMap.end(); ++itr )
   {
      FileInfoList   &fileList = itr->value;

      for ( U32 i = 0; i < fileList.getSize(); i++ )
      {
         FileInfo &fInfo = fileList[i];

         // This may fail if the file doesn't exist... thats ok.
         FileNode::Attributes attr;
         GetFileAttributes(fInfo.filePath, &attr);

         fInfo.savedLastModTime = attr.mtime;
      }
   }

   mNotifying = true;

   Process::notify( this, &FileSystemChangeNotifier::process, PROCESS_LAST_ORDER );
}

void FileSystemChangeNotifier::process()
{
   internalProcessOnce();
}

void  FileSystemChangeNotifier::stopNotifier()
{
   mNotifying = false;

   Process::remove( this, &FileSystemChangeNotifier::process );
}

/// Makes sure paths going in and out of the notifier will have the same format
String FileSystemChangeNotifier::cleanPath(const Path& dir)
{
   // This "cleans up" the path, if we don't do this we can get mismatches on the path
   // coming from the notifier
   FileSystemRef fs = Torque::FS::GetFileSystem(dir);
   if (!fs)
      return String::EmptyString;
   return fs->mapFrom(fs->mapTo(dir));
}

void FileSystemChangeNotifier::internalNotifyDirChanged( const Path &dir )
{
   DirMap::Iterator itr = mDirMap.find( dir );
   if ( itr == mDirMap.end() )
      return;

   // Gather the changed file info.
   FileInfoList changedList;
   FileInfoList &fileList = itr->value;
   for ( U32 i = 0; i < fileList.getSize(); i++ )
   {
      FileInfo &fInfo = fileList[i];

      FileNode::Attributes attr;
      bool success = GetFileAttributes(fInfo.filePath, &attr);

      // Ignore the file if we couldn't get the attributes (it must have 
      // been deleted) or the last modification time isn't newer.
      if ( !success || attr.mtime <= fInfo.savedLastModTime )       
         continue;

      // Store the new mod time.
      fInfo.savedLastModTime = attr.mtime;

      // We're taking a copy of the FileInfo struct here so that the
      // callback can safely remove the notification without crashing.
      changedList.pushBack( fInfo );
   }

   // Now signal all the changed files.
   for ( U32 i = 0; i < changedList.getSize(); i++ )
   {
      FileInfo &fInfo = changedList[i];

      Con::warnf( "    : file changed [%s]", fInfo.filePath.getFullPath().c_str() );
      fInfo.signal.trigger( fInfo.filePath );
   }
}

//-----------------------------------------------------------------------------

FileSystem::FileSystem()
   :  mChangeNotifier( NULL ),
   mReadOnly(false)
{
}

FileSystem::~FileSystem()
{
   delete mChangeNotifier;
   mChangeNotifier = NULL;
}

File::File() {}
File::~File() {}
Directory::Directory() {}
Directory::~Directory() {}


FileNode::FileNode()
:  mChecksum(0)
{
}

Time   FileNode::getModifiedTime()
{
   Attributes attrs;

   bool  success = getAttributes( &attrs );

   if ( !success )
      return Time();

   return attrs.mtime;
}

U64    FileNode::getSize()
{
   Attributes attrs;

   bool  success = getAttributes( &attrs );

   if ( !success )
      return 0;

   return attrs.size;
}

U32 FileNode::getChecksum()
{
   bool  calculateCRC = (mLastChecksum == Torque::Time());

   if ( !calculateCRC )
   {
      Torque::Time   modTime = getModifiedTime();

      calculateCRC = (modTime > mLastChecksum);
   }

   if ( calculateCRC )
      mChecksum = calculateChecksum();
   
   if ( mChecksum )
      mLastChecksum = Time::getCurrentTime();

   return mChecksum;

}

//-----------------------------------------------------------------------------

class FileSystemRedirect: public FileSystem
{
   friend class FileSystemRedirectChangeNotifier;
public:
   FileSystemRedirect(MountSystem* mfs,const Path& path);

   String   getTypeStr() const { return "Redirect"; }

   FileNodeRef resolve(const Path& path);
   FileNodeRef create(const Path& path,FileNode::Mode);
   bool remove(const Path& path);
   bool rename(const Path& a,const Path& b);
   Path mapTo(const Path& path);
   Path mapFrom(const Path& path);
   
private:
   Path _merge(const Path& path);

   Path        mPath;
   MountSystem *mMFS;
};

class FileSystemRedirectChangeNotifier : public FileSystemChangeNotifier
{
public:

   FileSystemRedirectChangeNotifier( FileSystem *fs );

   bool addNotification( const Path &path, ChangeDelegate callback );   
   bool removeNotification( const Path &path, ChangeDelegate callback );

protected:

   virtual void  internalProcessOnce() {}
   virtual bool  internalAddNotification( const Path &dir ) { return false; }
   virtual bool  internalRemoveNotification( const Path &dir ) { return false; }
};

FileSystemRedirectChangeNotifier::FileSystemRedirectChangeNotifier( FileSystem *fs ) 
: FileSystemChangeNotifier( fs )
{

}   

bool FileSystemRedirectChangeNotifier::addNotification( const Path &path, ChangeDelegate callback )
{   
   FileSystemRedirect *rfs = (FileSystemRedirect*)mFS;
   Path redirectPath = rfs->_merge( path );

   FileSystemRef fs = rfs->mMFS->getFileSystem( redirectPath );   
   if ( !fs || !fs->getChangeNotifier() )
      return false;

   return fs->getChangeNotifier()->addNotification( redirectPath, callback );
}

bool FileSystemRedirectChangeNotifier::removeNotification( const Path &path, ChangeDelegate callback )
{
   FileSystemRedirect *rfs = (FileSystemRedirect*)mFS;
   Path redirectPath = rfs->_merge( path );

  FileSystemRef fs = rfs->mMFS->getFileSystem( redirectPath ); 
   if ( !fs || !fs->getChangeNotifier() )
      return false;

   return fs->getChangeNotifier()->removeNotification( redirectPath, callback );
}

FileSystemRedirect::FileSystemRedirect(MountSystem* mfs,const Path& path)
{
   mMFS = mfs;
   mPath.setRoot(path.getRoot());
   mPath.setPath(path.getPath());
   mChangeNotifier = new FileSystemRedirectChangeNotifier( this );
}

Path FileSystemRedirect::_merge(const Path& path)
{
   Path p = mPath;
   p.setPath(Path::Join(p.getPath(),'/',Path::CompressPath(path.getPath())));
   p.setFileName(path.getFileName());
   p.setExtension(path.getExtension());
   return p;
}

FileNodeRef FileSystemRedirect::resolve(const Path& path)
{
   Path p = _merge(path);
   FileSystemRef fs = mMFS->getFileSystem(p);
   if (fs != NULL)
      return fs->resolve(p);
   return NULL;
}

FileNodeRef FileSystemRedirect::create(const Path& path,FileNode::Mode mode)
{
   Path p = _merge(path);
   FileSystemRef fs = mMFS->getFileSystem(p);
   if (fs != NULL)
      return fs->create(p,mode);
   return NULL;
}

bool FileSystemRedirect::remove(const Path& path)
{
   Path p = _merge(path);
   FileSystemRef fs = mMFS->getFileSystem(p);
   if (fs != NULL)
      return fs->remove(p);
   return false;
}

bool FileSystemRedirect::rename(const Path& a,const Path& b)
{
   Path na = _merge(a);
   Path nb = _merge(b);
   FileSystemRef fsa = mMFS->getFileSystem(na);
   FileSystemRef fsb = mMFS->getFileSystem(nb);
   if (fsa.getPointer() == fsb.getPointer())
      return fsa->rename(na,nb);
   return false;
}

Path FileSystemRedirect::mapTo(const Path& path)
{
   Path p = _merge(path);
   FileSystemRef fs = mMFS->getFileSystem(p);
   if (fs != NULL)
      return fs->mapTo(p);
   return NULL;
}

Path FileSystemRedirect::mapFrom(const Path& path)
{
   Path p = _merge(path);
   FileSystemRef fs = mMFS->getFileSystem(p);
   if (fs != NULL)
      return fs->mapFrom(p);
   return NULL;
}

//-----------------------------------------------------------------------------

void MountSystem::_log(const String& msg)
{
   String newMsg = "MountSystem: " + msg;
   Con::warnf("%s", newMsg.c_str());
}

FileSystemRef MountSystem::_removeMountFromList(String root)
{
   for (Vector<MountFS>::iterator itr = mMountList.begin(); itr != mMountList.end(); itr++)
   {
      if (root.equal( itr->root, String::NoCase ))
      {
         FileSystemRef fs = itr->fileSystem;
         mMountList.erase(itr);
         return fs;
      }
   }
   return NULL;
}

FileSystemRef MountSystem::_getFileSystemFromList(const Path& path) const
{
   for (Vector<MountFS>::const_iterator itr = mMountList.begin(); itr != mMountList.end(); itr++)
   {
      if (itr->root.equal( path.getRoot(), String::NoCase ))
         return itr->fileSystem;
   }

   return NULL;
}


Path MountSystem::_normalize(const Path& path)
{
   Path po = path;

   // Assign to cwd root if none is specified.   
   if( po.getRoot().isEmpty() )
      po.setRoot( mCWD.getRoot() );

   // Merge in current working directory if the path is relative to
   // the current cwd.
   if( po.getRoot().equal( mCWD.getRoot(), String::NoCase ) && po.isRelative() )
   {
      po.setPath( Path::CompressPath( Path::Join( mCWD.getPath(),'/',po.getPath() ) ) );
   }
   return po;
}

FileRef MountSystem::createFile(const Path& path)
{
   Path np = _normalize(path);
   FileSystemRef fs = _getFileSystemFromList(np);

   if (fs && fs->isReadOnly())
   {
      _log(String::ToString("Cannot create file %s, filesystem is read-only", path.getFullPath().c_str()));
      return NULL;
   }

   if (fs != NULL)
      return static_cast<File*>(fs->create(np,FileNode::File).getPointer());
   return NULL;
}

DirectoryRef MountSystem::createDirectory(const Path& path, FileSystemRef fs)
{
   Path np = _normalize(path);
   if (fs.isNull())
      fs = _getFileSystemFromList(np);

   if (fs && fs->isReadOnly())
   {
      _log(String::ToString("Cannot create directory %s, filesystem is read-only", path.getFullPath().c_str()));
      return NULL;
   }

   if (fs != NULL)
      return static_cast<Directory*>(fs->create(np,FileNode::Directory).getPointer());
   return NULL;
}

FileRef MountSystem::openFile(const Path& path,File::AccessMode mode)
{
   FileNodeRef node = getFileNode(path);
   if (node != NULL)
   {
      FileRef file = dynamic_cast<File*>(node.getPointer());
      if (file != NULL)
      {
         if (file->open(mode))
            return file;
         else
            return NULL;
      }
   }
   else
   {
      if (mode != File::Read)
      {
         FileRef file = createFile(path);

         if (file != NULL)
         {
            file->open(mode);
            return file;
         }
      }
   }
   return NULL;
}

DirectoryRef MountSystem::openDirectory(const Path& path)
{
   FileNodeRef node = getFileNode(path);

   if (node != NULL)
   {
      DirectoryRef dir = dynamic_cast<Directory*>(node.getPointer());
      if (dir != NULL)
      {
         dir->open();
         return dir;
      }
   }
   return NULL;
}

bool MountSystem::remove(const Path& path)
{
   Path np = _normalize(path);
   FileSystemRef fs = _getFileSystemFromList(np);
   if (fs && fs->isReadOnly())
   {
      _log(String::ToString("Cannot remove path %s, filesystem is read-only", path.getFullPath().c_str()));
      return false;
   }
   if (fs != NULL)
      return fs->remove(np);
   return false;
}

bool MountSystem::rename(const Path& from,const Path& to)
{
   // Will only rename files on the same filesystem
   Path pa = _normalize(from);
   Path pb = _normalize(to);
   FileSystemRef fsa = _getFileSystemFromList(pa);
   FileSystemRef fsb = _getFileSystemFromList(pb);
   if (!fsa || !fsb)
      return false;
   if (fsa.getPointer() != fsb.getPointer())
   {
      _log(String::ToString("Cannot rename path %s to a different filesystem", from.getFullPath().c_str()));
      return false;
   }
   if (fsa->isReadOnly() || fsb->isReadOnly())
   {
      _log(String::ToString("Cannot rename path %s; source or target filesystem is read-only", from.getFullPath().c_str()));
      return false;
   }

   return fsa->rename(pa,pb);
}

bool MountSystem::mount(String root,FileSystemRef fs)
{
   MountFS mount;
   mount.root = root;
   mount.path = "/";
   mount.fileSystem = fs;
   mMountList.push_back(mount);
   return true;
}

bool MountSystem::mount(String root,const Path &path)
{
   return mount(root,new FileSystemRedirect(this,_normalize(path)));
}

FileSystemRef MountSystem::unmount(String root)
{
   FileSystemRef first = _removeMountFromList(root);

   // remove remaining FSes on this root
   while (!_removeMountFromList(root).isNull())
      ;
   
   return first;
}

bool MountSystem::unmount(FileSystemRef fs)
{
   if (fs.isNull())
      return false;

   // iterate back to front in case FS is in list multiple times.
   // also check that fs is not null each time since its a strong ref
   // so it could be nulled during removal.
   bool unmounted = false;
   for (int i = mMountList.size() - 1; !fs.isNull() && i >= 0; --i)
   {
      if (mMountList[i].fileSystem.getPointer() == fs.getPointer())
      {
         mMountList.erase(i);
         unmounted = true;
      }
   }
   return unmounted;
}

bool MountSystem::setCwd(const Path& file)
{
   if (file.getPath().isEmpty())
      return false;
   mCWD.setRoot(file.getRoot());
   mCWD.setPath(file.getPath());
   return true;
}

const Path& MountSystem::getCwd() const
{
   return mCWD;
}

FileSystemRef MountSystem::getFileSystem(const Path& path)
{
   return _getFileSystemFromList(_normalize(path));
}

bool MountSystem::getFileAttributes(const Path& path,FileNode::Attributes* attr)
{
   FileNodeRef file = getFileNode(path);

   if (file != NULL)
   {
      bool result = file->getAttributes(attr);
      return result;
   }

   return false;
}

FileNodeRef MountSystem::getFileNode(const Path& path)
{
   Path np = _normalize(path);
   FileSystemRef fs = _getFileSystemFromList(np);
   if (fs != NULL)
      return fs->resolve(np);
   return NULL;
}

bool  MountSystem::mapFSPath( const String &inRoot, const Path &inPath, Path &outPath )
{
   FileSystemRef fs = _getFileSystemFromList(inRoot);

   if ( fs == NULL )
   {
      outPath = Path();
      return false;
   }

   outPath = fs->mapFrom( inPath );

   return outPath.getFullPath() != String();
}

S32 MountSystem::findByPattern( const Path &inBasePath, const String &inFilePattern, bool inRecursive, Vector<String> &outList, bool includeDirs/* =false */, bool multiMatch /* = true */ )
{
   if (mFindByPatternOverrideFS.isNull() && !inBasePath.isDirectory() )
      return -1;

   DirectoryRef   dir = NULL;
   if (mFindByPatternOverrideFS.isNull())
      // open directory using standard mount system search
      dir = openDirectory( inBasePath );
   else
   {
      // use specified filesystem to open directory
      FileNodeRef fNode = mFindByPatternOverrideFS->resolve(inBasePath);
      if (fNode && (dir = dynamic_cast<Directory*>(fNode.getPointer())) != NULL)
         dir->open();
   }

   if ( dir == NULL )
      return -1;

   if (includeDirs)
   {
      // prepend cheesy "DIR:" annotation for directories
      outList.push_back(String("DIR:") + inBasePath.getPath());
   }

   FileNode::Attributes  attrs;

   Vector<String>    recurseDirs;

   while ( dir->read( &attrs ) )
   {
      // skip hidden files
      if ( attrs.name.c_str()[0] == '.' )
         continue;

      String   name( attrs.name );

      if ( (attrs.flags & FileNode::Directory) && inRecursive )
      {
         name += '/';
         String   path = Path::Join( inBasePath, '/', name );
         recurseDirs.push_back( path );
      }
      
      if ( !multiMatch && FindMatch::isMatch( inFilePattern, attrs.name, false ) )
      {
         String   path = Path::Join( inBasePath, '/', name );
         outList.push_back( path );
      }
      
      if ( multiMatch && FindMatch::isMatchMultipleExprs( inFilePattern, attrs.name, false ) )
      {
         String   path = Path::Join( inBasePath, '/', name );
         outList.push_back( path );
      }
   }

   dir->close();

   for ( S32 i = 0; i < recurseDirs.size(); i++ )
      findByPattern( recurseDirs[i], inFilePattern, true, outList, includeDirs, multiMatch );

   return outList.size();
}

bool MountSystem::isFile(const Path& path)
{
   FileNode::Attributes attr;
   if (getFileAttributes(path,&attr))
      return attr.flags & FileNode::File;
   return false;
}

bool MountSystem::isDirectory(const Path& path, FileSystemRef fsRef)
{
   FileNode::Attributes attr;

   if (fsRef.isNull())
   {
      if (getFileAttributes(path,&attr))
         return attr.flags & FileNode::Directory;
      return false;
   }
   else
   {
      FileNodeRef fnRef = fsRef->resolve(path);
      if (fnRef.isNull())
         return false;

      FileNode::Attributes attr;
      if (fnRef->getAttributes(&attr))
         return attr.flags & FileNode::Directory;
      return false;
   }
}

bool MountSystem::isReadOnly(const Path& path)
{
   // first check to see if filesystem is read only
   FileSystemRef fs = getFileSystem(path);
   if ( fs.isNull() )
      // no filesystem owns this file...oh well, return false
      return false;
   if (fs->isReadOnly())
      return true;

   // check the file attributes, note that if the file does not exist,
   // this function returns false.  that should be ok since we know
   // the file system is writable at this point.
   FileNode::Attributes attr;
   if (getFileAttributes(path,&attr))
      return attr.flags & FileNode::ReadOnly;
   return false;
}

void  MountSystem::startFileChangeNotifications()
{
   for ( U32 i = 0; i < mMountList.size(); i++ )
   {
      FileSystemChangeNotifier   *notifier = mMountList[i].fileSystem->getChangeNotifier();

      if ( notifier != NULL && !notifier->isNotifying() )
         notifier->startNotifier();
   }
}

void  MountSystem::stopFileChangeNotifications()
{
   for ( U32 i = 0; i < mMountList.size(); i++ )
   {
      FileSystemChangeNotifier   *notifier = mMountList[i].fileSystem->getChangeNotifier();

      if ( notifier != NULL && notifier->isNotifying() )
         notifier->stopNotifier();
   }
}

bool MountSystem::createPath(const Path& path)
{
   if (path.getPath().isEmpty())
      return true;
   
   // See if the pathectory exists
   Path dir;
   dir.setRoot(path.getRoot());
   dir.setPath(path.getPath());

   // in a virtual mount system, isDirectory may return true if the directory exists in a read only FS,
   // but the directory may not exist on a writeable filesystem that is also mounted.  
   // So get the target filesystem that will
   // be used for the full writable path and and make sure the directory exists on it.
   FileSystemRef fsRef = getFileSystem(path);
   
   if (isDirectory(dir,fsRef))
      return true;
   
   // Start from the top and work our way down
   Path sub;
   dir.setPath(path.isAbsolute()? String("/"): String());
   for (U32 i = 0; i < path.getDirectoryCount(); i++)
   {
      sub.setPath(path.getDirectory(i));
      dir.appendPath(sub);
      if (!isDirectory(dir,fsRef))
      {
         if (!createDirectory(dir,fsRef))
            return false;
      }
   }
   return true;
}


//-----------------------------------------------------------------------------

// Default global mount system
#ifndef TORQUE_DISABLE_VIRTUAL_MOUNT_SYSTEM
// Note that the Platform::FS::MountZips() must be called in platformVolume.cpp for zip support to work.
static VirtualMountSystem   sgMountSystem;
#else
static MountSystem   sgMountSystem;
#endif

namespace FS
{

FileRef CreateFile(const Path &path)
{
   return sgMountSystem.createFile(path);
}

DirectoryRef CreateDirectory(const Path &path)
{
   return sgMountSystem.createDirectory(path);
}

FileRef OpenFile(const Path &path, File::AccessMode mode)
{
   return sgMountSystem.openFile(path,mode);
}

bool ReadFile(const Path &inPath, void *&outData, U32 &outSize, bool inNullTerminate )
{
   FileRef  fileR = OpenFile( inPath, File::Read );
 
   outData = NULL;
   outSize = 0;

   // We'll get a NULL file reference if 
   // the file failed to open.
   if ( fileR == NULL )
      return false;

   outSize = fileR->getSize();

   // Its not a failure to read an empty
   // file... but we can exit early.
   if ( outSize == 0 )
      return true;

   U32 sizeRead = 0;

   if ( inNullTerminate )
   {
      outData = new char [outSize+1];
      sizeRead = fileR->read(outData, outSize);
      static_cast<char *>(outData)[outSize] = '\0';
   }
   else
   {
      outData = new char [outSize];
      sizeRead = fileR->read(outData, outSize);
   }

   if ( sizeRead != outSize )
   {
      delete static_cast<char *>(outData);
      outData = NULL;
      outSize = 0;
      return false;
   }

   return true;
}

DirectoryRef OpenDirectory(const Path &path)
{
   return sgMountSystem.openDirectory(path);
}

bool Remove(const Path &path)
{
   return sgMountSystem.remove(path);
}

bool Rename(const Path &from, const Path &to)
{
   return sgMountSystem.rename(from,to);
}

bool Mount(String root, FileSystemRef fs)
{
   return sgMountSystem.mount(root,fs);
}

bool Mount(String root, const Path &path)
{
   return sgMountSystem.mount(root,path);
}

FileSystemRef Unmount(String root)
{
   return sgMountSystem.unmount(root);
}

bool Unmount(FileSystemRef fs)
{
   return sgMountSystem.unmount(fs);
}

bool SetCwd(const Path &file)
{
   return sgMountSystem.setCwd(file);
}

const Path& GetCwd()
{
   return sgMountSystem.getCwd();
}

FileSystemRef GetFileSystem(const Path &path)
{
   return sgMountSystem.getFileSystem(path);
}

bool GetFileAttributes(const Path &path, FileNode::Attributes* attr)
{
   return sgMountSystem.getFileAttributes(path,attr);
}

S32 CompareModifiedTimes(const Path& p1, const Path& p2)
{
   FileNode::Attributes a1, a2;
   if (!Torque::FS::GetFileAttributes(p1, &a1))
      return -1;
   if (!Torque::FS::GetFileAttributes(p2, &a2))
      return -1;
   if (a1.mtime < a2.mtime)
      return -1;
   if (a1.mtime == a2.mtime)
      return 0;
   return 1;
}

FileNodeRef GetFileNode(const Path &path)
{
   return sgMountSystem.getFileNode(path);
}

bool  MapFSPath( const String &inRoot, const Path &inPath, Path &outPath )
{
   return sgMountSystem.mapFSPath( inRoot, inPath, outPath );
}

bool GetFSPath( const Path &inPath, Path &outPath )
{
   FileSystemRef sys = GetFileSystem( inPath );
   if ( sys )
   {
      outPath = sys->mapTo( inPath );
      return true;
   }

   return false;
}

S32 FindByPattern( const Path &inBasePath, const String &inFilePattern, bool inRecursive, Vector<String> &outList, bool multiMatch )
{
   return sgMountSystem.findByPattern(inBasePath, inFilePattern, inRecursive, outList, false, multiMatch);
}

bool IsFile(const Path &path)
{
   return sgMountSystem.isFile(path);
}

bool IsDirectory(const Path &path)
{
   return sgMountSystem.isDirectory(path);
}

bool IsReadOnly(const Path &path)
{
   return sgMountSystem.isReadOnly(path);
}

String MakeUniquePath( const char *path, const char *fileName, const char *ext )
{
   Path filePath;

   filePath.setPath( path );
   filePath.setFileName( fileName );
   filePath.setExtension( ext );

   // First get an upper bound on the range of filenames to search. This lets us
   // quickly skip past a large number of existing filenames.
   // Note: upper limit of 2^31 added to handle the degenerate case of a folder
   // with files named using the powers of 2, but plenty of space in between!
   U32 high = 1;
   while ( IsFile( filePath ) && ( high < 0x80000000 ) )
   {
      high = high * 2;
      filePath.setFileName( String::ToString( "%s%d", fileName, high ) );
   }

   // Now perform binary search for first filename in the range that doesn't exist
   // Note that the returned name will not be strictly numerically *first* if the
   // existing filenames are non-sequential (eg. 4,6,7), but it will still be unique.
   if ( high > 1 )
   {
      U32 low = high / 2;
      while ( high - low > 1 )
      {
         U32 probe = low + ( high - low ) / 2;
         filePath.setFileName( String::ToString( "%s%d", fileName, probe ) );
         if ( IsFile( filePath ) )
            low = probe;
         else
            high = probe;
      }

      // The 'high' index is guaranteed not to exist
      filePath.setFileName( String::ToString( "%s%d", fileName, high ) );
   }

   return filePath.getFullPath();
}

void  StartFileChangeNotifications() { sgMountSystem.startFileChangeNotifications(); }
void  StopFileChangeNotifications() { sgMountSystem.stopFileChangeNotifications(); }

S32      GetNumMounts() { return sgMountSystem.getNumMounts(); }
String   GetMountRoot( S32 index ) { return sgMountSystem.getMountRoot(index); }
String   GetMountPath( S32 index ) { return sgMountSystem.getMountPath(index); }
String   GetMountType( S32 index ) { return sgMountSystem.getMountType(index); }

bool CreatePath(const Path& path)
{
   return sgMountSystem.createPath(path);
}

} // Namespace FS

} // Namespace Torque


