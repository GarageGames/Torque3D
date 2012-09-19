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

#include "core/strings/stringFunctions.h"
#include "core/util/zip/zipArchive.h"
#include "core/util/zip/unitTests/zipTest.h"

#include "unit/test.h"
#include "unit/memoryTester.h"


using namespace UnitTesting;
using namespace Zip;

CreateUnitTest(ZipTestWrite, "Zip/Write")
{
private:
   StringTableEntry mWriteFilename;
   StringTableEntry mBaselineFilename;
   StringTableEntry mWorkingFilename;

public:
   /// Number of files to write for testing large numbers of files in a zip
   static const U32 mTons = 5000;

   ZipTestWrite()
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
      
      // Test writing to zip files without the zip file existing
      testWriting(mWriteFilename, ZipArchive::Read);
      testWriting(mWriteFilename, ZipArchive::ReadWrite);

      // Cleanup to try write without existing file
      cleanup();
      testWriting(mWriteFilename, ZipArchive::Write);

      // Now use previous file to test everything again with an existing file
      testWriting(mWriteFilename, ZipArchive::ReadWrite);
      testWriting(mWriteFilename, ZipArchive::Write);
      testWriting(mWriteFilename, ZipArchive::Read);

      testWritingTons(makeTestPath("WriteTons.zip"));

      test(m.check(), "Zip write test leaked memory!");
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

   //-----------------------------------------------------------------------------

   bool writeFile(ZipArchive *zip, const char *filename, char *fileBuf, U32 bufSize, bool mustNotExist = false, const char *contents = NULL)
   {
      if(mustNotExist && fileBuf && bufSize > 0)
      {
         // Find a unique filename
         U32 count = 1;
         dStrcpy(fileBuf, filename);
         
         while(zip->findFileInfo(fileBuf))
         {
            dSprintf(fileBuf, bufSize, "%s.%04x", filename, count++);

            if(count == 0)
            {
               fail("writeFile - got stuck in an infinite loop looking for a unique filename");
               return false;
            }
         }
      }
      else if(fileBuf && bufSize > 0)
         dStrcpy(fileBuf, filename);

      // Try and write to the file
      Stream * stream = zip->openFile(fileBuf ? fileBuf : filename, ZipArchive::Write);
      if(stream != NULL)
      {
         stream->writeLine(contents ? (U8 *)contents : (U8 *)"This is a test of writing to a file.");
         zip->closeFile(stream);

         return true;
      }

      return false;
   }

   //-----------------------------------------------------------------------------

   bool testWriting(const char *zipfile, ZipArchive::AccessMode mode)
   {
      ZipArchive *zip = new ZipArchive;

      if(! zip->openArchive(zipfile, mode))
      {
         delete zip;
         
         // This is only an error if we're not trying to open as read

         if(mode != ZipArchive::Read)
            fail("Unable to open zip file");
         
         return mode == ZipArchive::Read;
      }

      char fileBuf[1024];

      // Write to file that doesn't already exist
      if(!writeFile(zip, "testWriteNew.txt", fileBuf, sizeof(fileBuf), true))
      {
         fail("Couldn't write to a file that didn't already exist");
         goto bail;
      }

      // Write to file that we've already written to
      if(!writeFile(zip, fileBuf, NULL, 0, false, "This is a test of overwriting the file."))
      {
         if(mode != ZipArchive::Read)
            fail("Couldn't write to a file that we've already written to");
         goto bail;
      }

      // Write to file that already exists, but we haven't written to
      // To do this, we need to close and re-open the zipfile

      zip->closeArchive();
      delete zip;

      zip = new ZipArchive;
      if(! zip->openArchive(zipfile, mode))
      {
         delete zip;
         fail("Unable to re-open zip file. Strange!");
         return false;
      }
      
      // Use the file we already overwrote since we are sure of it's filename
      if(!writeFile(zip, fileBuf, NULL, 0, false, "This is a test of overwriting the file again."))
      {
         if(mode != ZipArchive::Read)
            fail("Couldn't write to a file that already existed");
         goto bail;
      }

      {
         // Move this into its own scope to deal with goto 'crosses initialization' errors
         // Attempt to open a file for write that's already open for write (should fail)
         Stream * stream1 = zip->openFile("writeLockTest.txt", ZipArchive::Write);
         if(stream1 != NULL)
         {
            stream1->writeLine((U8 *)"Test text to make a valid file");

            // This next open should fail
            Stream  * stream2 = zip->openFile("writeLockTest.txt", ZipArchive::Write);
            if(stream2 != NULL)
            {
               if(mode != ZipArchive::Read)
                  fail("Opening a file for write multiple times should have failed");
               zip->closeFile(stream2);
            }
            zip->closeFile(stream1);
         }
      }
      
bail:
      zip->closeArchive();
      delete zip;
      
      return true;
   }

   //-----------------------------------------------------------------------------

   bool testWritingTons(const char *zipfile)
   {
      ZipArchive zip;

      if(! zip.openArchive(zipfile, ZipArchive::Write))
      {
         fail("Unable to open zip file");
         return false;
      }

      bool ret = true;

      for(U32 i = 0;i < mTons;++i)
      {
         char fname[256];
         dSprintf(fname, sizeof(fname), "file%04x.txt", i);

         if(! writeFile(&zip, fname, NULL, 0))
         {
            fail("Failed to write file");
            fail(fname);

            ret = false;

            break;
         }
      }

      zip.closeArchive();
      return ret;
   }
};
