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

#include "unit/test.h"
#include "unit/memoryTester.h"

#include "core/util/zip/zipArchive.h"
#include "core/util/zip/unitTests/zipTest.h"

#include "core/stringTable.h"
#include "core/crc.h"

using namespace UnitTesting;
using namespace Zip;

CreateUnitTest(ZipTestRead, "Zip/Read")
{
private:
   StringTableEntry mWriteFilename;
   StringTableEntry mBaselineFilename;
   StringTableEntry mWorkingFilename;

public:
   ZipTestRead()
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

      // Read mode tests with the baseline zip
      readTest(mBaselineFilename);

      // Read mode tests with a zip created by us
      test(makeTestZip(), "Failed to make test zip");
      readTest(mWorkingFilename);

      // Do read/write mode tests
      readWriteTest(mWriteFilename);

      test(m.check(), "Zip read test leaked memory!");
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

   bool writeFile(ZipArchive *zip, const char *filename)
   {
      Stream * stream = zip->openFile(filename, ZipArchive::Write);
      if(stream != NULL)
      {
         stream->writeLine((U8 *)"This is a test file for reading from a zip created with the zip code.\r\nIt is pointless by itself, but needs to be long enough that it gets compressed.\r\n");
         zip->closeFile(stream);
         return true;
      }

      return false;
   }

   bool makeTestZip()
   {
      ZipArchive *zip = new ZipArchive;

      if(!zip->openArchive(mWorkingFilename, ZipArchive::Write))
      {
         delete zip;
         fail("Unable to open zip file");

         return false;
      }

      test(writeFile(zip, "test.txt"), "Failed to write file into test zip");
      test(writeFile(zip, "console.log"), "Failed to write file into test zip");
      test(writeFile(zip, "folder/OpenAL32.dll"), "Failed to write file into test zip");

      zip->closeArchive();
      delete zip;

      return true;
   }

   //-----------------------------------------------------------------------------

   bool testReadFile(ZipArchive *zip, const char *filename)
   {
      // [tom, 2/5/2007] Try using openFile() first for fairest real-world test
      Stream *stream;
      if((stream = zip->openFile(filename, ZipArchive::Read)) == NULL)
         return false;

      const CentralDir *cd = zip->findFileInfo(filename);
      if(cd == NULL)
      {
         // [tom, 2/5/2007] This shouldn't happen
         zip->closeFile(stream);

         fail("testReadFile - File opened successfully, but couldn't find central directory. This is bad.");

         return false;
      }

      U32 crc = CRC::INITIAL_CRC_VALUE;
      
      bool ret = true;
      U8 buffer[1024];
      U32 numBytes = stream->getStreamSize() - stream->getPosition();
      while((stream->getStatus() != Stream::EOS) && numBytes > 0)
      {
         U32 numRead = numBytes > sizeof(buffer) ? sizeof(buffer) : numBytes;
         if(! stream->read(numRead, buffer))
         {
            ret = false;
            fail("testReadFile - Couldn't read from stream");
            break;
         }

         crc = CRC::calculateCRC(buffer, numRead, crc);

         numBytes -= numRead;
      }

      if(ret)
      {
         // CRC Check
         crc ^= CRC::CRC_POSTCOND_VALUE;
         if(crc != cd->mCRC32)
         {
            fail("testReadFile - File failed CRC check");
            ret = false;
         }
      }

      zip->closeFile(stream);
      return ret;
   }

   //-----------------------------------------------------------------------------

   bool readTest(const char *zipfile)
   {
      ZipArchive *zip = new ZipArchive;

      if(!zip->openArchive(zipfile, ZipArchive::Read))
      {
         delete zip;
         fail("Unable to open zip file");

         return false;
      }

      // Try reading files that exist in various formats
      testReadFile(zip, "test.txt");
      testReadFile(zip, "console.log");
      testReadFile(zip, "folder/OpenAL32.dll");

      // Try reading a file that doesn't exist
      if(testReadFile(zip, "hopefullyDoesntExist.bla.bla.foo"))
         fail("Opening a file the didn't exist succeeded");

      zip->closeArchive();
      delete zip;

      return true;
   }

   bool readWriteTest(const char *zipfile)
   {
      ZipArchive *zip = new ZipArchive;
      bool ret = true;

      if(!zip->openArchive(zipfile, ZipArchive::ReadWrite))
      {
         delete zip;
         fail("Unable to open zip file");

         return false;
      }

      // Test writing a file and then reading it back before the zip is rebuilt
      test(writeFile(zip, "test.txt"), "Failed to write file to test zip");
      test(testReadFile(zip, "test.txt"), "Failed to read file back from test zip");
      
      // Read from file that is already open for write (should fail)
      Stream *wrStream = zip->openFile("writeTest.txt", ZipArchive::Write);
      if(wrStream != NULL)
      {
         wrStream->writeLine((U8 *)"Test text to make a valid file");

         // This next open should fail
         Stream * rdStream = zip->openFile("writeTest.txt", ZipArchive::Read);
         if(rdStream != NULL)
         {
            fail("Succeeded in opening a file for read that's already open for write");
            ret = false;

            zip->closeFile(rdStream);
         }

         zip->closeFile(wrStream);
      }
      else
      {
         fail("Could not open file for write");
         ret = false;
      }

      zip->closeArchive();
      delete zip;

      return ret;
   }
};
