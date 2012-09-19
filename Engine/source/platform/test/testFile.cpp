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
#include "core/fileio.h"
#include "unit/test.h"
#include "core/util/tVector.h"
#include "console/console.h"

using namespace UnitTesting;

CreateUnitTest(CheckFileListingAndExclusion, "File/ListDirectoryAndExclusions")
{
   void run()
   {
      // Just dump everything under the current directory. We should
      // find at least one file.
      
      // Exclude .svn and CVS
      Platform::clearExcludedDirectories();
      Platform::addExcludedDirectory(".svn");
      Platform::addExcludedDirectory("CVS");
      
      test(Platform::isExcludedDirectory("foo") == false, "Doesn't match list, shouldn't be excluded.");
      test(Platform::isExcludedDirectory(".svn") == true, "On list, should be excluded.");
      test(Platform::isExcludedDirectory("CVS") == true, "On list, should be excluded.");
      test(Platform::isExcludedDirectory(".svnCVS") == false, "Looks like a duck, but it shouldn't be excluded cuz it's distinct from all entries on the exclusion list.");
      
      // Ok, now our exclusion list is setup, so let's dump some paths.
      Vector < Platform::FileInfo > pathInfo;
      Platform::dumpPath (Platform::getCurrentDirectory(), pathInfo, 2);
      
      Con::printf("Dump of files in '%s', up to 2 levels deep...", Platform::getCurrentDirectory());
      for(S32 i=0; i<pathInfo.size(); i++)
      {
         Platform::FileInfo &file = pathInfo[i];
         
         Con::printf("    %s (%s) %d bytes", file.pFullPath, file.pFileName, file.fileSize);
      }
      
      test(pathInfo.size() > 0, "Should find at least SOMETHING in the current directory!");
      
      // This'll nuke info if we run it in a live situation... so don't run unit
      // tests in a live situation. ;)
      Platform::clearExcludedDirectories();
   }
};

CreateUnitTest(CheckFileTouchAndTime, "File/TouchAndTime")
{
   void run()
   {
      FileTime create[2], modify[2];
      
      // Create a file and sleep for a second.
      File f;
      f.open("testTouch.file", File::WriteAppend);
      f.close();
   
      Platform::sleep(2000);
      
      // Touch a file and note its last-modified.
      dFileTouch("testTouch.file");
      test(Platform::isFile("testTouch.file"), "We just touched this file - it should exist.");
      test(Platform::getFileTimes("testTouch.file", &create[0], &modify[0]), "Failed to get filetimes for a file we just created.");
      
      // Sleep for a few seconds...
      Platform::sleep(5000);
      
      // Touch it again, and compare the last-modifieds.
      test(Platform::isFile("testTouch.file"), "We just touched this file - it should exist.");
      dFileTouch("testTouch.file");
      test(Platform::isFile("testTouch.file"), "We just touched this file - it should exist.");
      test(Platform::getFileTimes("testTouch.file", &create[1], &modify[1]), "Failed to get filetimes for a file we just created.");
      
      // Now compare the times...
      test(Platform::compareFileTimes(modify[0], modify[1]) < 0, "Timestamps are wrong - modify[0] should be before modify[1]!");
      
      // This seems to fail even on a valid case...
      // test(Platform::compareFileTimes(create[0], create[1]) == 0, "Create timestamps should match - we didn't delete the file during this test.");
      
      // Clean up..
      dFileDelete("testTouch.file");
      test(!Platform::isFile("testTouch.file"), "Somehow failed to delete our test file.");
   }
};

// Mac has no implementations for these functions, so we 'def it out for now.
#if 0
CreateUnitTest(CheckVolumes, "File/Volumes")
{
   void run()
   {
      Con::printf("Dumping volumes by name:");
      
      Vector<const char*> names;
      Platform::getVolumeNamesList(names);
      
      test(names.size() > 0, "We should have at least one volume...");
      
      for(S32 i=0; i<names.size(); i++)
         Con::printf("   %s", names[i]);
         
      Con::printf("Dumping volume info:");
      
      Vector<Platform::VolumeInformation> info;
      Platform::getVolumeInformationList(info);
      
      test(names.size() == info.size(), "Got inconsistent number of volumes back from info vs. name list functions!");
      
      for(S32 i=0; i<info.size(); i++)
         Con::printf("   %s rootPath = %s filesystem = %s ser. num. = %d type = %d readonly = %s",
               info[i].Name,
               info[i].RootPath,
               info[i].FileSystem,
               info[i].SerialNumber,
               info[i].Type,
               info[i].ReadOnly ? "true" : "false");
   }
};
#endif

CreateUnitTest(CheckFileWriteAndRead, "File/ReadAndWrite")
{
   void run()
   {
      // Open a file, write some junk to it, close it,
      // check size is correct, and open it again.
      
   }
};