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

#ifndef _PLATFORMVOLUME_H_
#define _PLATFORMVOLUME_H_

#include "core/volume.h"


namespace Platform
{
namespace FS
{
   using namespace Torque;
   using namespace Torque::FS;

   FileSystemRef  createNativeFS( const String &volume );

   String   getAssetDir();

   /// Mount default OS file systems.
   /// On POSIX environment this means mounting a root FileSystem "/", mounting
   /// the $HOME environment variable as the "home:/" file system and setting the
   /// current working directory to the current OS working directory.
   bool InstallFileSystems();

   bool MountDefaults();
   bool MountZips(const String &root);
   
   bool Touch( const Path &path );

} // Namespace FS
} // Namespace Platform

#endif

