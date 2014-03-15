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

#if defined(TORQUE_OS_WIN) || defined(TORQUE_OS_XBOX) || defined(TORQUE_OS_XENON)
#include <sys/utime.h>
#else
#include <sys/time.h>
#endif

#include "platform/platformVolume.h"
#include "core/util/zip/zipVolume.h"

using namespace Torque;
using namespace Torque::FS;

namespace Platform
{
namespace FS
{

bool  MountDefaults()
{
   String  path = getAssetDir();

   bool  mounted = Mount( "game", createNativeFS( path ));

   if ( !mounted )
      return false;

#ifndef TORQUE_DISABLE_VIRTUAL_MOUNT_SYSTEM
   // Note that the VirtualMountSystem must be enabled in volume.cpp for zip support to work.
   return MountZips("game");
#else
   return true;
#endif
}

bool MountZips(const String &root)
{
   Path basePath;
   basePath.setRoot(root);
   Vector<String> outList;

   S32 num = FindByPattern(basePath, "*.zip", true, outList);
   if(num == 0)
      return true; // not an error

   S32 mounted = 0;
   for(S32 i = 0;i < outList.size();++i)
   {
      String &zipfile = outList[i];
#ifdef TORQUE_ZIP_DISK_LAYOUT
      mounted += (S32)Mount(root, new ZipFileSystem(zipfile, false));
#else 
      mounted += (S32)Mount(root, new ZipFileSystem(zipfile, true));
#endif
   }

   return mounted == outList.size();
}

//-----------------------------------------------------------------------------

bool  Touch( const Path &path )
{
#if defined(TORQUE_OS_WIN) || defined(TORQUE_OS_XBOX) || defined(TORQUE_OS_XENON)
   return( utime( path.getFullPath(), 0 ) != -1 );
#else
   return( utimes( path.getFullPath(), NULL) == 0 ); // utimes returns 0 on success.
#endif
}

} // namespace FS
} // namespace Platform
