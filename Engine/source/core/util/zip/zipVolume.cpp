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
#include "core/util/zip/zipVolume.h"

#include "core/util/zip/zipSubStream.h"
#include "core/util/noncopyable.h"
#include "console/console.h"

namespace Torque
{
   using namespace FS;
   using namespace Zip;

class ZipFileNode : public Torque::FS::File, public Noncopyable
{
   typedef FileNode Parent;

   //--------------------------------------------------------------------------
   // ZipFileNode class (Internal)
   //--------------------------------------------------------------------------
public:
   ZipFileNode(StrongRefPtr<ZipArchive>& archive, String zipFilename, Stream* zipStream, ZipArchive::ZipEntry* ze) 
   {
      mZipStream = zipStream;
      mArchive = archive;
      mZipFilename = zipFilename;
      mByteCount = dynamic_cast<IStreamByteCount*>(mZipStream);
      AssertFatal(mByteCount, "error, zip stream interface does not implement IStreamByteCount");
      mZipEntry = ze;
   }
   virtual ~ZipFileNode()
   {
      close();
   }

   virtual Path   getName() const { return mZipFilename; }
   virtual Status getStatus() const
   { 
      if (mZipStream) 
      {
         // great, Stream Status is different from FileNode Status...
         switch (mZipStream->getStatus())
         {
         case Stream::Ok:
            return FileNode::Open;
         case Stream::EOS:
            return FileNode::EndOfFile;
         case Stream::IOError:
            return FileNode::UnknownError;
         default:
            return FileNode::UnknownError;
         }
      }
      else
         return FileNode::Closed;
   }

   virtual bool   getAttributes(Attributes* attr) 
   { 
      if (!attr)
         return false;

      attr->flags = FileNode::File | FileNode::Compressed | FileNode::ReadOnly;
      attr->name = mZipFilename;
      // use the mod time for both mod and access time, since we only have mod time in the CD
      attr->mtime = ZipArchive::DOSTimeToTime(mZipEntry->mCD.mModTime, mZipEntry->mCD.mModDate);
      attr->atime = ZipArchive::DOSTimeToTime(mZipEntry->mCD.mModTime, mZipEntry->mCD.mModDate);
      attr->size = mZipEntry->mCD.mUncompressedSize;

      return true; 
   }

   virtual U32 getPosition()
   {
      if (mZipStream)
         return mZipStream->getPosition();
      else
         return 0;
   }

   virtual U32 setPosition(U32 pos, SeekMode mode)
   {
      if (!mZipStream || mode != Begin)
         return 0;
      else 
         return mZipStream->setPosition(pos);
   }

   virtual bool open(AccessMode mode)
   {
      // stream is already open so just check to make sure that they are using a valid mode
      if (mode == Read)
         return mZipStream != NULL;
      else 
      {
         Con::errorf("ZipFileSystem: Write access denied for file %s", mZipFilename.c_str());
         return false;
      }
   }

   virtual bool close()
   {
      if (mZipStream != NULL && mArchive != NULL)
      {
         mArchive->closeFile(mZipStream);
         mZipStream = NULL;
         mByteCount = NULL;
      }
      return true;
   }

   virtual U32 read(void* dst, U32 size)
   {
      if (mZipStream && mZipStream->read(size, dst) && mByteCount)
         return mByteCount->getLastBytesRead();
      else
         return 0;
   }

   virtual U32 write(const void* src, U32 size)
   {
      if (mZipStream && mZipStream->write(size, src) && mByteCount)
         return mByteCount->getLastBytesWritten();
      else
         return 0;
   }

   protected:
      virtual U32    calculateChecksum()
      {
         // JMQ: implement
         return 0;
      };

      Stream* mZipStream;
      StrongRefPtr<ZipArchive> mArchive;
      ZipArchive::ZipEntry* mZipEntry;
      String mZipFilename;
      IStreamByteCount* mByteCount;
};

//--------------------------------------------------------------------------
// ZipDirectoryNode class (Internal)
//--------------------------------------------------------------------------

class ZipDirectoryNode : public Torque::FS::Directory, public Noncopyable
{
public:
   ZipDirectoryNode(StrongRefPtr<ZipArchive>& archive, const Torque::Path& path, ZipArchive::ZipEntry* ze)
   {
      mPath = path;
      mArchive = archive;
      mZipEntry = ze;
      if (mZipEntry)
         mChildIter = mZipEntry->mChildren.end();
   }
   ~ZipDirectoryNode()
   {
   }

   Torque::Path getName() const { return mPath; }

   // getStatus() doesn't appear to be used for directories
   Status getStatus() const
   {
      return FileNode::Open;
   }

   bool getAttributes(Attributes* attr)
   {
      if (!attr)
         return false;

      attr->flags = FileNode::Directory | FileNode::Compressed | FileNode::ReadOnly;
      attr->name = mPath.getFullPath();
      // use the mod time for both mod and access time, since we only have mod time in the CD
      attr->mtime = ZipArchive::DOSTimeToTime(mZipEntry->mCD.mModTime, mZipEntry->mCD.mModDate);
      attr->atime = ZipArchive::DOSTimeToTime(mZipEntry->mCD.mModTime, mZipEntry->mCD.mModDate);
      attr->size = mZipEntry->mCD.mUncompressedSize;

      return true; 
   }

   bool open()
   {
      // reset iterator
      if (mZipEntry)
         mChildIter = mZipEntry->mChildren.begin();
      return (mZipEntry != NULL && mArchive.getPointer() != NULL);
   }
   bool close()
   {
      if (mZipEntry)
         mChildIter = mZipEntry->mChildren.end();
      return true;
   }
   bool read(Attributes* attr)
   {
      if (!attr)
         return false;

      if (mChildIter == mZipEntry->mChildren.end())
         return false;

      ZipArchive::ZipEntry* ze = (*mChildIter).value;

      attr->flags = FileNode::Compressed;
      if (ze->mIsDirectory)
         attr->flags |= FileNode::Directory;
      else
         attr->flags |= FileNode::File;

      attr->name = ze->mName;
      attr->mtime = ZipArchive::DOSTimeToTime(ze->mCD.mModTime, ze->mCD.mModDate);
      attr->atime = ZipArchive::DOSTimeToTime(ze->mCD.mModTime, ze->mCD.mModDate);
      attr->size = 0; // we don't know the size until we open a stream, so we'll have to use size 0

      mChildIter++;

      return true;
   }

private:
   U32 calculateChecksum() 
   {
      return 0;
   }

   Torque::Path mPath;
   Map<String,ZipArchive::ZipEntry*>::Iterator mChildIter;
   StrongRefPtr<ZipArchive> mArchive;
   ZipArchive::ZipEntry* mZipEntry;
};

//--------------------------------------------------------------------------
// ZipFakeRootNode class (Internal)
//--------------------------------------------------------------------------

class ZipFakeRootNode : public Torque::FS::Directory, public Noncopyable
{
public:
   ZipFakeRootNode(StrongRefPtr<ZipArchive>& archive, const Torque::Path& path, const String &fakeRoot)
   {
      mPath = path;
      mArchive = archive;
      mRead = false;
      mFakeRoot = fakeRoot;
   }
   ~ZipFakeRootNode()
   {
   }

   Torque::Path getName() const { return mPath; }

   // getStatus() doesn't appear to be used for directories
   Status getStatus() const
   {
      return FileNode::Open;
   }

   bool getAttributes(Attributes* attr)
   {
      if (!attr)
         return false;

      attr->flags = FileNode::Directory | FileNode::Compressed | FileNode::ReadOnly;
      attr->name = mPath.getFullPath();
      // use the mod time for both mod and access time, since we only have mod time in the CD
      
      ZipArchive::ZipEntry* zipEntry = mArchive->getRoot();
      attr->mtime = ZipArchive::DOSTimeToTime(zipEntry->mCD.mModTime, zipEntry->mCD.mModDate);
      attr->atime = ZipArchive::DOSTimeToTime(zipEntry->mCD.mModTime, zipEntry->mCD.mModDate);
      attr->size = zipEntry->mCD.mUncompressedSize;

      return true; 
   }

   bool open()
   {
      mRead = false;
      return (mArchive.getPointer() != NULL);
   }
   bool close()
   {
      mRead = false;
      return true;
   }
   bool read(Attributes* attr)
   {
      if (!attr)
         return false;

      if (mRead)
         return false;

      ZipArchive::ZipEntry* ze = mArchive->getRoot();

      attr->flags = FileNode::Compressed;
      if (ze->mIsDirectory)
         attr->flags |= FileNode::Directory;
      else
         attr->flags |= FileNode::File;

      attr->name = mFakeRoot;
      attr->mtime = ZipArchive::DOSTimeToTime(ze->mCD.mModTime, ze->mCD.mModDate);
      attr->atime = ZipArchive::DOSTimeToTime(ze->mCD.mModTime, ze->mCD.mModDate);
      attr->size = 0; // we don't know the size until we open a stream, so we'll have to use size 0

      mRead = true;

      return true;
   }

private:
   U32 calculateChecksum() 
   {
      return 0;
   }

   Torque::Path mPath;
   Map<String,ZipArchive::ZipEntry*>::Iterator mChildIter;
   StrongRefPtr<ZipArchive> mArchive;
   bool mRead;
   String mFakeRoot;
};

//--------------------------------------------------------------------------
// ZipFileSystem
//--------------------------------------------------------------------------

ZipFileSystem::ZipFileSystem(String& zipFilename, bool zipNameIsDir /* = false */)
{
   mZipFilename = zipFilename;
   mInitted = false;
   mZipNameIsDir = zipNameIsDir;
   if(mZipNameIsDir)
   {
      Path path(zipFilename);
      mFakeRoot = Path::Join(path.getPath(), '/', path.getFileName());
   }

   // open the file now but don't read it yet, since we want construction to be lightweight
   // we open the file now so that whatever filesystems are mounted right now (which may be temporary)
   // can be umounted without affecting this file system.
   mZipArchiveStream = new FileStream();
   mZipArchiveStream->open(mZipFilename, Torque::FS::File::Read);
   
   // As far as the mount system is concerned, ZFSes are read only write now (even though 
   // ZipArchive technically support read-write, we don't expose this to the mount system because we 
   // don't want to support that as a standard run case right now.)
   mReadOnly = true;
}

ZipFileSystem::~ZipFileSystem()
{
   if (mZipArchiveStream)
   {
      mZipArchiveStream->close();
      delete mZipArchiveStream;
   }
   mZipArchive = NULL;
}

FileNodeRef ZipFileSystem::resolve(const Path& path)
{
   if (!mInitted)
      _init();

   if (mZipArchive.isNull())
      return NULL;

   // eat leading "/"
   String name = path.getFullPathWithoutRoot();
   if (name.find("/") == 0)
      name = name.substr(1, name.length() - 1);

   if(name.isEmpty() && mZipNameIsDir)
      return new ZipFakeRootNode(mZipArchive, path, mFakeRoot);

   if(mZipNameIsDir)
   {
      // Remove the fake root from the name so things can be found
      if(name.find(mFakeRoot) == 0)
         name = name.substr(mFakeRoot.length());

#ifdef TORQUE_DISABLE_FIND_ROOT_WITHIN_ZIP
      else
         // If a zip file's name isn't the root of the path we're looking for
         // then do not continue.  Otherwise, we'll continue to look for the
         // path's root within the zip file itself.  i.e. we're looking for the
         // path "scripts/test.cs".  If the zip file itself isn't called scripts.zip
         // then we won't look within the archive for a "scripts" directory.
         return NULL;
#endif

      if (name.find("/") == 0)
         name = name.substr(1, name.length() - 1);
   }

   // first check to see if input path is a directory
   // check for request of root directory
   if (name.isEmpty())
   {
      ZipDirectoryNode* zdn = new ZipDirectoryNode(mZipArchive, path, mZipArchive->getRoot());
      return zdn;
   }

   ZipArchive::ZipEntry* ze = mZipArchive->findZipEntry(name);
   if (ze == NULL)
      return NULL;

   if (ze->mIsDirectory)
   {
      ZipDirectoryNode* zdn = new ZipDirectoryNode(mZipArchive, path, ze);
      return zdn;
   }

   // pass in the zip entry so that openFile() doesn't need to look it up again.
   Stream* stream = mZipArchive->openFile(name, ze, ZipArchive::Read);
   if (stream == NULL)
      return NULL;

   ZipFileNode* zfn = new ZipFileNode(mZipArchive, name, stream, ze);
   return zfn;
}

void ZipFileSystem::_init()
{
   if (mInitted)
      return;
   mInitted = true;

   if (!mZipArchive.isNull())
      return;
   if (mZipArchiveStream->getStatus() != Stream::Ok)
      return;

   mZipArchive = new ZipArchive();
   if (!mZipArchive->openArchive(mZipArchiveStream, ZipArchive::Read))
   {
      Con::errorf("ZipFileSystem: failed to open zip archive %s", mZipFilename.c_str());
      return;
   }

   // tell the archive that it owns the zipStream now
   mZipArchive->setDiskStream(mZipArchiveStream);
   // and null it out because we don't own it anymore
   mZipArchiveStream = NULL;

   // for debugging
   //mZipArchive->dumpCentralDirectory();
}

};