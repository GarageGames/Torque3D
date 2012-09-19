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

#include "platformWin32/platformWin32.h"
#include "platform/platformVFS.h"
#include "console/console.h"

#include "core/stream/memstream.h"
#include "core/util/zip/zipArchive.h"

#include "core/util/safeDelete.h"

#include "VFSRes.h"

//-----------------------------------------------------------------------------

struct Win32VFSState
{
   S32 mRefCount;

   HGLOBAL mResData;

   MemStream *mZipStream;
   Zip::ZipArchive *mZip;

   Win32VFSState() : mResData(NULL), mZip(NULL), mRefCount(0), mZipStream(NULL)
   {
   }
};

static Win32VFSState gVFSState;

//-----------------------------------------------------------------------------

Zip::ZipArchive *openEmbeddedVFSArchive()
{
   if(gVFSState.mZip)
   {
      ++gVFSState.mRefCount;
      return gVFSState.mZip;
   }

   HRSRC hRsrc = FindResource(NULL, MAKEINTRESOURCE(IDR_ZIPFILE), dT("RT_RCDATA"));

   if(hRsrc == NULL)
      return NULL;
   
   if((gVFSState.mResData = LoadResource(NULL, hRsrc)) == NULL)
      return NULL;

   void * mem = LockResource(gVFSState.mResData);
   if(mem != NULL)
   {
      U32 size = SizeofResource(NULL, hRsrc);
      gVFSState.mZipStream = new MemStream(size, mem, true, false);
      gVFSState.mZip = new Zip::ZipArchive;

      if(gVFSState.mZip->openArchive(gVFSState.mZipStream))
      {
         ++gVFSState.mRefCount;
         return gVFSState.mZip;
      }

      SAFE_DELETE(gVFSState.mZip);
      SAFE_DELETE(gVFSState.mZipStream);
   }

   FreeResource(gVFSState.mResData);
   gVFSState.mResData = NULL;

   return NULL;
}

void closeEmbeddedVFSArchive()
{
   if(gVFSState.mRefCount == 0)
      return;

   --gVFSState.mRefCount;

   if(gVFSState.mRefCount < 1)
   {
      SAFE_DELETE(gVFSState.mZip);
      SAFE_DELETE(gVFSState.mZipStream);
      
      if(gVFSState.mResData)
      {
         FreeResource(gVFSState.mResData);
         gVFSState.mResData = NULL;
      }
   }
}
