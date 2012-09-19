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
#ifndef _CORE_ZIP_VOLUME_H_
#define _CORE_ZIP_VOLUME_H_

#include "core/volume.h"
#include "core/util/str.h"
#include "core/util/zip/zipArchive.h"
#include "core/util/autoPtr.h"

namespace Torque
{
   using namespace FS;
   using namespace Zip;

class ZipFileSystem: public FileSystem
{
public:
   ZipFileSystem(String& zipFilename, bool zipNameIsDir = false);
   virtual ~ZipFileSystem();

   String   getTypeStr() const { return "Zip"; }

   FileNodeRef resolve(const Path& path);

   // these are unsupported, ZipFileSystem is currently read only access
   FileNodeRef create(const Path& path,FileNode::Mode) { return 0; }
   bool remove(const Path& path) { return 0; }
   bool rename(const Path& a,const Path& b) { return 0; }

   // these are unsupported
   Path mapTo(const Path& path) { return path; }
   Path mapFrom(const Path& path) { return path; }

public:
   /// Private interface for use by unit test only. 
   StrongRefPtr<ZipArchive> getArchive() { return mZipArchive; }

private:
   void _init();

   bool mInitted;
   bool mZipNameIsDir;
   String mZipFilename;
   String mFakeRoot;
   FileStream* mZipArchiveStream;
   StrongRefPtr<ZipArchive> mZipArchive;
};

}

#endif