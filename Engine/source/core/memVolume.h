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
#ifndef _MEMVOLUME_H_
#define _MEMVOLUME_H_

#ifndef _VOLUME_H_
#include "core/volume.h"
#endif

#ifndef _TDICTIONARY_H_
#include "core/util/tDictionary.h"
#endif 

namespace Torque
{
   using namespace FS;

   namespace Mem
   {

      struct MemFileData;
      struct MemDirectoryData;
      class MemDirectory;

      //-----------------------------------------------------------------------------
      class MemFileSystem: public FileSystem
      {
      public:
         MemFileSystem(String volume);
         ~MemFileSystem();

         String   getTypeStr() const { return "Mem"; }

         FileNodeRef resolve(const Path& path);
         FileNodeRef create(const Path& path,FileNode::Mode);
         bool remove(const Path& path);
         bool rename(const Path& from,const Path& to);
         Path mapTo(const Path& path);
         Path mapFrom(const Path& path);

      private:
         String mVolume;
         MemDirectoryData* mRootDir;

         MemDirectory* getParentDir(const Path& path, FileNodeRef& parentRef);
      };

      //-----------------------------------------------------------------------------
      /// Mem stdio file access.
      /// This class makes use the fopen, fread and fwrite for buffered io.
      class MemFile: public File
      {
      public:
         MemFile(MemFileSystem* fs, MemFileData* fileData);
         virtual ~MemFile();

         Path getName() const;
         Status getStatus() const;
         bool getAttributes(Attributes*);

         U32 getPosition();
         U32 setPosition(U32,SeekMode);

         bool open(AccessMode);
         bool close();

         U32 read(void* dst, U32 size);
         U32 write(const void* src, U32 size);

      private:
         U32 calculateChecksum();

         MemFileSystem* mFileSystem;
         MemFileData* mFileData;
         Status   mStatus;
         U32 mCurrentPos;

         bool _updateInfo();
         void _updateStatus();
      };


      //-----------------------------------------------------------------------------

      class MemDirectory: public Directory
      {
      public:
         MemDirectory(MemFileSystem* fs, MemDirectoryData* dir);
         ~MemDirectory();

         Path getName() const;
         Status getStatus() const;
         bool getAttributes(Attributes*);

         bool open();
         bool close();
         bool read(Attributes*);

      private:
         friend class MemFileSystem;
         MemFileSystem* mFileSystem;
         MemDirectoryData* mDirectoryData;

         U32 calculateChecksum();         
         
         Status   mStatus;
         U32 mSearchIndex;         
      };

   } // Namespace
} // Namespace

#endif