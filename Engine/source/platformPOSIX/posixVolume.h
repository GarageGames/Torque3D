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

#ifndef _POSIXVOLUME_H_
#define _POSIXVOLUME_H_

#ifndef _VOLUME_H_
#include "core/volume.h"
#endif

#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>

namespace Torque
{
using namespace FS;

namespace Posix
{

//-----------------------------------------------------------------------------

class PosixFileSystem: public FileSystem
{
   String _volume;

public:
   PosixFileSystem(String volume);
   ~PosixFileSystem();

   String   getTypeStr() const { return "POSIX"; }

   FileNodeRef resolve(const Path& path);
   FileNodeRef create(const Path& path,FileNode::Mode);
   bool remove(const Path& path);
   bool rename(const Path& from,const Path& to);
   Path mapTo(const Path& path);
   Path mapFrom(const Path& path);
};


//-----------------------------------------------------------------------------
/// Posix stdio file access.
/// This class makes use the fopen, fread and fwrite for buffered io.
class PosixFile: public File
{
   friend class PosixFileSystem;
   Path _path;
   String _name;
   FILE* _handle;
   NodeStatus _status;

   PosixFile(const Path& path,String name);
   bool _updateInfo();
   void _updateStatus();

public:
   ~PosixFile();

   Path getName() const;
   NodeStatus getStatus() const;
   bool getAttributes(Attributes*);

   U32 getPosition();
   U32 setPosition(U32,SeekMode);

   bool open(AccessMode);
   bool close();

   U32 read(void* dst, U32 size);
   U32 write(const void* src, U32 size);

private:
   U32 calculateChecksum();
};


//-----------------------------------------------------------------------------

class PosixDirectory: public Directory
{
   friend class PosixFileSystem;
   Path _path;
   String _name;
   DIR* _handle;
   NodeStatus _status;

   PosixDirectory(const Path& path,String name);
   void _updateStatus();

public:
   ~PosixDirectory();

   Path getName() const;
   NodeStatus getStatus() const;
   bool getAttributes(Attributes*);

   bool open();
   bool close();
   bool read(Attributes*);

private:
   U32 calculateChecksum();
};


} // Namespace
} // Namespace
#endif
