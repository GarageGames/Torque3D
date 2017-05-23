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
/*
#include "core/crc.h"
#include "core/strings/stringFunctions.h"
#include "core/util/zip/zipArchive.h"
#include "core/util/zip/unitTests/zipTest.h"

#include "unit/test.h"
#include "unit/memoryTester.h"


using namespace UnitTesting;
using namespace Zip;

CreateUnitTest(ZipTestMisc, "Zip/Misc")
{
private:
   StringTableEntry mWriteFilename;
   StringTableEntry mBaselineFilename;
   StringTableEntry mWorkingFilename;

public:
   ZipTestMisc()
   {
      mWriteFilename = makeTestPath(ZIPTEST_WRITE_FILENAME);
      mBaselineFilename = makeTestPath(ZIPTEST_BASELINE_FILENAME);
      mWorkingFilename = makeTestPath(ZIPTEST_WORKING_FILENAME);
   }

   void run()
   {
      MemoryTester m;
      m.mark();

      // Clean up from a previous run
      cleanup();

      test(makeTestZip(), "Failed to create test zip");

      miscTest(mWorkingFilename);

      // [tom, 2/7/2007] extractFile() uses the resource manager so it will
      // create a resource object that will be detected as a "leak" since it
      // won't be freed until much later.

      test(m.check(), "Zip misc test leaked memory! (Unless it says it's from the Resource Manager above, see comments in core/zip/unitTests/zipTestMisc.cc)");
   }

private:

   //-----------------------------------------------------------------------------

   void cleanup()
   {
      if(Platform::isFile(mWriteFilename))
         dFileDelete(mWriteFilename);
      if(Platform::isFile(mWorkingFilename))
         dFileDelete(mWorkingFilename);
   }

   bool makeTestZip()
   {
      FileStream source, dest;

      if(! source.open(mBaselineFilename, Torque::FS::File::Read))
      {
         fail("Failed to open baseline zip for read");
         return false;
      }

      // [tom, 2/7/2007] FileStream's d'tor calls close() so we don't really have to do it here

      if(! dest.open(mWorkingFilename, Torque::FS::File::Write))
      {
         fail("Failed to open working zip for write");
         return false;
      }

      if(! dest.copyFrom(&source))
      {
         fail("Failed to copy baseline zip");
         return false;
      }

      dest.close();
      source.close();
      return true;
   }

   //-----------------------------------------------------------------------------

   bool miscTest(const char *zipfile)
   {
      ZipArchive *zip = new ZipArchive;
      bool ret = true;

      if(! zip->openArchive(zipfile, ZipArchive::ReadWrite))
      {
         delete zip;
         fail("Unable to open zip file");

         return false;
      }

      // Opening a file in the zip as ReadWrite (should fail)
      Stream *stream;
      if((stream = zip->openFile("readWriteTest.txt", ZipArchive::ReadWrite)) != NULL)
      {
         zip->closeFile(stream);

         fail("File opened successfully as read/write");
         ret = false;
      }

      // Enumerating, Extracting and Deleting files
      U32 curSize = zip->mDiskStream->getStreamSize();

      if(zip->numEntries() > 0)
      {
         // Find the biggest file in the zip
         const CentralDir *del = NULL;
         for(S32 i = 0;i < zip->numEntries();++i)
         {
            const CentralDir &cd = (*zip)[i];

            if(del == NULL || (del && cd.mUncompressedSize > del->mUncompressedSize))
               del = &cd;
         }

         if(del)
         {
            // Extract the file
            const char *ptr = dStrrchr(del->mFilename, '/');
            if(ptr)
               ++ptr;
            else
               ptr = del->mFilename;
            StringTableEntry fn = makeTestPath(ptr);

            ret = test(zip->extractFile(del->mFilename, fn), "Failed to extract file.");

            // Then delete it
            ret = test(zip->deleteFile(del->mFilename), "ZipArchive::deleteFile() failed?!");

            // Close and reopen the zip to force it to rebuild
            zip->closeArchive();

            if(! zip->openArchive(zipfile, ZipArchive::ReadWrite))
            {
               delete zip;
               fail("Unable to re-open zip file?!");

               return false;
            }
         }
         else
         {
            fail("Couldn't find a file to delete?!");
            ret = false;
         }
      }
      else
      {
         fail("Baseline zip has no files.");
         ret = false;
      }

      U32 newSize = zip->mDiskStream->getStreamSize();
      test(newSize != curSize, "Zip file size didn't change when deleting a file ?!");

      zip->closeArchive();
      delete zip;

      return ret;
   }
};
*/