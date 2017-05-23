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
#include "core/memVolume.h"

#include "core/crc.h"
#include "core/frameAllocator.h"
#include "core/util/str.h"
#include "core/strings/stringFunctions.h"
#include "platform/platformVolume.h"

namespace Torque
{
   namespace Mem
   {

      // Multiple MemFile's can reference the same path, so this is here to contain
      // the actual data at a Path.
      struct MemFileData
      {      
         MemFileData(MemFileSystem* fs, const Path& path)
         {
            mPath = path;
            mBufferSize = 1024;
            mFileSize = 0;
            mBuffer = dMalloc(mBufferSize);
            dMemset(mBuffer, 0, mBufferSize);
            mModified = Time::getCurrentTime();
            mLastAccess = mModified;    
            mFileSystem = fs;
         }

         ~MemFileData()
         {
            dFree(mBuffer);
         }

         bool getAttributes(FileNode::Attributes* attr)
         {
            attr->name = mPath;
            attr->flags = FileNode::File;
            attr->size = mFileSize;
            attr->mtime = mModified;
            attr->atime = mLastAccess;
            return true;
         }

         FileNodeRef resolve(const Path& path)
         {
            // Is it me?
            String sThisPath(mPath);
            String sTargetPath(path);
            if (sThisPath == sTargetPath)
               return new MemFile(mFileSystem, this);
            // Nope
            return NULL;
         }

         Path mPath;         
         void* mBuffer;
         U32 mBufferSize;  // This is the size of the memory buffer >= mFileSize
         U32 mFileSize;    // This is the size of the "file" <= mBufferSize
         Time mModified;      // Last modified
         Time mLastAccess;      // Last access
         MemFileSystem* mFileSystem;
      };

      struct MemDirectoryData
      {
         Path mPath;
         MemFileSystem* mFileSystem;
         Vector<MemFileData*> mFiles;
         Vector<MemDirectoryData*> mDirectories;

         MemDirectoryData(MemFileSystem* fs, const Path& path)
         {
            mFileSystem = fs;
            mPath = path;
         }

         ~MemDirectoryData()
         {
            for (U32 i = 0; i < mFiles.size(); i++)
            {
               delete mFiles[i];
            }
            for (U32 i = 0; i < mDirectories.size(); i++)
            {
               delete mDirectories[i];
            }
         }

         bool getAttributes(FileNode::Attributes* attr)
         {
            attr->name = mPath;
            attr->flags = FileNode::Directory;
            return true;
         }

         FileNodeRef resolve(const Path& path)
         {
            // Is it me?
            String sThisPath(mPath);
            String sTargetPath(path);
            if (sThisPath == sTargetPath)
               return new MemDirectory(mFileSystem, this);
            // Is it one of my children?
            if (sTargetPath.find(sThisPath) == 0)
            {
               FileNodeRef result;
               for (U32 i = 0; i < mDirectories.size() && result.isNull(); i++)
                  result = mDirectories[i]->resolve(path);
               for (U32 i = 0; i < mFiles.size() && result.isNull(); i++)
                  result = mFiles[i]->resolve(path);
               return result;
            }
            // Nope
            return NULL;
         }
      };

      //-----------------------------------------------------------------------------
      MemFileSystem::MemFileSystem(String volume)
      {
         mVolume = volume;                  
         mRootDir = new MemDirectoryData(this, volume);          
      }

      MemFileSystem::~MemFileSystem()
      {
         delete mRootDir;
      }

      FileNodeRef MemFileSystem::resolve(const Path& path)
      {
         return mRootDir->resolve(path);
      }

      // 
      MemDirectory* MemFileSystem::getParentDir(const Path& path, FileNodeRef& parentRef)
      {
         parentRef = mRootDir->resolve(path.getRoot() + ":" + path.getPath());
         if (parentRef.isNull())
            return NULL;

         MemDirectory* result = dynamic_cast<MemDirectory*>(parentRef.getPointer());
         return result;
      }

      FileNodeRef MemFileSystem::create(const Path& path, FileNode::Mode mode)
      {
         // Already exists
         FileNodeRef result = mRootDir->resolve(path);
         if (result.isValid())
            return result;

         // Doesn't exist, try to get parent node.
         FileNodeRef parentRef;
         MemDirectory* mDir = getParentDir(path, parentRef);
         if (mDir)
         {
            MemDirectoryData* mdd = mDir->mDirectoryData;
            switch (mode)
            {
               case FileNode::File :
                  {
                     MemFileData* mfd = new MemFileData(this, path);
                     mdd->mFiles.push_back(mfd);
                     return new MemFile(this, mfd);
                  }
                  break;
               case FileNode::Directory :
                  {
                     MemDirectoryData* mfd = new MemDirectoryData(this, path);
                     mdd->mDirectories.push_back(mfd);
                     return new MemDirectory(this, mfd);
                  }
                  break;
               default:
                  // anything else we ignore
                  break;
            }
         }         
         return NULL;
      }

      bool MemFileSystem::remove(const Path& path)
      {
         FileNodeRef parentRef;
         MemDirectory* mDir = getParentDir(path, parentRef);
         MemDirectoryData* mdd = mDir->mDirectoryData;
         for (U32 i = 0; i < mdd->mDirectories.size(); i++)
         {
            if (mdd->mDirectories[i]->mPath == path)
            {
               delete mdd->mDirectories[i];
               mdd->mDirectories.erase_fast(i);
               return true;
            }
         }
         for (U32 i = 0; i < mdd->mFiles.size(); i++)
         {
            if (mdd->mFiles[i]->mPath == path)
            {
               delete mdd->mFiles[i];
               mdd->mFiles.erase_fast(i);
               return true;
            }
         }
         return false;
      }

      bool MemFileSystem::rename(const Path& from,const Path& to)
      {         
         // Source must exist
         FileNodeRef source = mRootDir->resolve(from);
         if (source.isNull())
            return false;
          
         // Destination must not exist
         FileNodeRef dest = mRootDir->resolve(to);
         if (source.isValid())
            return false;

         // Get source parent
         FileNodeRef sourceParentRef;
         MemDirectory* sourceDir = getParentDir(from, sourceParentRef);

         // Get dest parent
         FileNodeRef destRef;
         MemDirectory* mDir = getParentDir(to, destRef);

         // Now move it/rename it
         if (dynamic_cast<MemDirectory*>(source.getPointer()))
         {
            MemDirectoryData* sourcedd;
            MemDirectoryData* d = sourceDir->mDirectoryData;
            for (U32 i = 0; i < d->mDirectories.size(); i++)
            {
               if (d->mDirectories[i]->mPath == from)
               {
                  sourcedd = d->mDirectories[i];
                  d->mDirectories.erase_fast(i);
                  sourcedd->mPath = to;
                  mDir->mDirectoryData->mDirectories.push_back(sourcedd);            
                  return true;
               }
            }
         } else {
            MemFileData* sourceFile;
            MemDirectoryData* d = sourceDir->mDirectoryData;
            for (U32 i = 0; i < d->mFiles.size(); i++)
            {
               if (d->mFiles[i]->mPath == from)
               {
                  sourceFile = d->mFiles[i];
                  d->mFiles.erase_fast(i);
                  sourceFile->mPath = to;
                  mDir->mDirectoryData->mFiles.push_back(sourceFile);            
                  return true;
               }
            }
         }
         return false;
      }

      Path MemFileSystem::mapTo(const Path& path)
      {
         String file = mVolume;
         file = Path::Join(file, '/', path.getPath());
         file = Path::Join(file, '/', path.getFileName());
         file = Path::Join(file, '.', path.getExtension());
         return file;
      }

      Path MemFileSystem::mapFrom(const Path& path)
      {
         const String::SizeType  volumePathLen = mVolume.length();

         String   pathStr = path.getFullPath();

         if ( mVolume.compare( pathStr, volumePathLen, String::NoCase ))
            return Path();

         return pathStr.substr( volumePathLen, pathStr.length() - volumePathLen );
      }

      //-----------------------------------------------------------------------------

      MemFile::MemFile(MemFileSystem* fs, MemFileData* fileData)
      {
         mFileData = fileData;
         mStatus = Closed;         
         mCurrentPos = U32_MAX;
         mFileSystem = fs;
      }

      MemFile::~MemFile()
      {
      }

      Path MemFile::getName() const
      {
         return mFileData->mPath;
      }

      FileNode::NodeStatus MemFile::getStatus() const
      {
         return mStatus;
      }

      bool MemFile::getAttributes(Attributes* attr)
      {                 
         return mFileData->getAttributes(attr);
      }

      U32 MemFile::calculateChecksum()
      {
         return CRC::calculateCRC(mFileData->mBuffer, mFileData->mFileSize);
      }

      bool MemFile::open(AccessMode mode)
      {         
         mStatus = Open;
         mCurrentPos = 0;
         switch (mode)
         {
            case Read :
            case ReadWrite :
               mCurrentPos = 0;
               break;
            case Write :
               mCurrentPos = 0;
               mFileData->mFileSize = 0;
               break;            
            case WriteAppend :
               mCurrentPos = mFileData->mFileSize; 
               break;
         }
         return true;
      }

      bool MemFile::close()
      {
         mStatus = Closed;
         return true;
      }

      U32 MemFile::getPosition()
      {
         if (mStatus == Open || mStatus == EndOfFile)
            return mCurrentPos;            
         return 0;
      }

      U32 MemFile::setPosition(U32 delta, SeekMode mode)
      {
         if (mStatus != Open && mStatus != EndOfFile)
            return 0;
         
         switch (mode)
         {
         case Begin:    
            mCurrentPos = delta;
            break;            
         case Current:  
            mCurrentPos += delta;
            break;
         case End:
            mCurrentPos = mFileData->mFileSize - delta;
            break;         
         }

         mStatus = Open;

         return mCurrentPos;
      }

      U32 MemFile::read(void* dst, U32 size)
      {
         if (mStatus != Open && mStatus != EndOfFile)
            return 0;

         U32 copyAmount = getMin(size, mFileData->mFileSize - mCurrentPos);
         dMemcpy(dst, (U8*) mFileData->mBuffer + mCurrentPos, copyAmount);
         mCurrentPos += copyAmount;
         mFileData->mLastAccess = Time::getCurrentTime();
         if (mCurrentPos == mFileData->mFileSize)
            mStatus = EndOfFile;
         return copyAmount;
      }

      U32 MemFile::write(const void* src, U32 size)
      {
         if ((mStatus != Open && mStatus != EndOfFile) || !size)
            return 0;

         if (mFileData->mFileSize + size > mFileData->mBufferSize)
         {
            // Keep doubling our buffer size until we're big enough.
            while (mFileData->mFileSize + size > mFileData->mBufferSize)
               mFileData->mBufferSize *= 2;
            mFileData->mBuffer = dRealloc(mFileData->mBuffer, mFileData->mBufferSize);
            if (!mFileData->mBuffer)
            {
               mStatus = FileSystemFull;
               return 0;
            }
         }

         dMemcpy((U8*)mFileData->mBuffer + mCurrentPos, src, size);
         mCurrentPos += size;
         mFileData->mFileSize = getMax(mFileData->mFileSize, mCurrentPos);
         mFileData->mLastAccess = Time::getCurrentTime();
         mFileData->mModified = mFileData->mLastAccess;         

         return size;
      }

      //-----------------------------------------------------------------------------

      MemDirectory::MemDirectory(MemFileSystem* fs, MemDirectoryData* dir)
      {         
         mStatus = Closed;         
         mDirectoryData = dir;
         mFileSystem = fs;
      }

      MemDirectory::~MemDirectory()
      {
      }

      Path MemDirectory::getName() const
      {
         return mDirectoryData->mPath;
      }

      bool MemDirectory::open()
      {
         mSearchIndex = 0;
         mStatus = Open;
         return true;
      }

      bool MemDirectory::close()
      {
         return true;
      }

      bool MemDirectory::read(Attributes* entry)
      {
         if (mStatus != Open)
            return false;

         if (mSearchIndex < mDirectoryData->mDirectories.size())
         {
            mDirectoryData->mDirectories[mSearchIndex]->getAttributes(entry);
            mSearchIndex++;
            return true;
         }

         AssertFatal(mSearchIndex > mDirectoryData->mDirectories.size(), "This should not happen!");
         U32 fileIndex = mSearchIndex - mDirectoryData->mDirectories.size();
         if (fileIndex < mDirectoryData->mFiles.size())
         {
            mDirectoryData->mFiles[mSearchIndex]->getAttributes(entry);
            mSearchIndex++;
            return true;
         }

         return false;
      }

      U32 MemDirectory::calculateChecksum()
      {
         // Return checksum of current entry
         return 0;
      }

      bool MemDirectory::getAttributes(Attributes* attr)
      {
         return mDirectoryData->getAttributes(attr);
      }

      FileNode::NodeStatus MemDirectory::getStatus() const
      {
         return mStatus;
      }
   } // Namespace Mem

} // Namespace Torque