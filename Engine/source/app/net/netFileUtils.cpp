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

#include "torqueConfig.h"

#ifdef TORQUE_ENABLE_ASSET_FILE_CLIENT_REPLICATION

#include "netFileUtils.h"
#include "console/engineAPI.h"
#include "core/fileObject.h"


namespace netFileCommands
{
const String requestsubmit = String("requestsubmit");
const String finished = String("finished");
const String get = String("get");
const String list = String("list");
const String writefile = String("writefile");
const String denyWrite = String("denyWrite");
const String acceptWrite = String("acceptWrite");
const String send = String("send");
}

char* netFileUtils::uinttochar( U32 n)
{
   char* a = Con::getReturnBuffer(20);
   if (n == 0)
   {
      *a = '0';
      *(a+1) = '\0';
      return a;
   }
   char aux[20];
   aux[19] = '\0';
   char* auxp = aux + 19;
   int c = 1;
   while (n != 0)
   {
      int mod = n % 10;
      *(--auxp) = mod | 0x30;
      n /=  10;
      c++;
   }
   memcpy(a, auxp, c);
   return a;
}

bool netFileUtils::isWriteable(const char* fileName)
   {
   String filename(Torque::Path::CleanSeparators(fileName));
   Con::expandScriptFilename(sgScriptFilenameBuffer, sizeof(sgScriptFilenameBuffer), filename.c_str());

   Torque::Path givenPath(Torque::Path::CompressPath(sgScriptFilenameBuffer));
   Torque::FS::FileSystemRef fs = Torque::FS::GetFileSystem(givenPath);
   Torque::Path path = fs->mapTo(givenPath);

   return !Torque::FS::IsReadOnly(path);
   }

#endif