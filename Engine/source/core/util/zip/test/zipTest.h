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

#include "core/stringTable.h"

#ifndef _ZIPTEST_H_
#define _ZIPTEST_H_

namespace Zip
{

//-----------------------------------------------------------------------------
//
// Things that need to be tested:
//
// Writing Zips:
//    Write to file that doesn't already exist
//    Write to file that we've already written to
//    Write to file that already exists, but we haven't written to
//    Attempt to open a file for write that's already open for write (should fail)
//    ** Attempt to open a file that's open for read (should fail)
//    Writing >= 5000 files to a zip
//    Writing >= 5000 files to a zip in a number of folders
//
// Reading Zips:
//    Open for read from file that doesn't exist (should fail)
//    Read from file that already existed in the zip
//    Read from file that we have written to the zip but not yet rebuilt
//    ** Read from file that is already open for read (should fail)
//    Read from file that is already open for write (should fail)
//
// Miscellaneous:
//    Opening a file in the zip as ReadWrite (should fail)
//    Enumerating files
//    Deleting files
//    ** Adding files
//    Opening Zips with zip comments
//    Opening Zips/Files with file comments
//    Opening large zips that require searching for the EOCD
//
// All tests should be run on zip files that are opened for read, write and readwrite
// All tests need to be run for both forms of openArchive()
//
// All tests that require an existing zip file should be run on a zip created with
// a standard zip application in addition to zip files created by this code.
//
// Tests involving ReadWrite mode need to be done both with and without the zip
// file existing before the test.
//
// Tests marked ** are not possible to test yet as they are not supported by the
// zip code.
//
// [tom, 2/2/2007] I'm using WinZip 11 to create the baseline zips. It is probably
// worthwhile to include zips created in additional applications.
//
//-----------------------------------------------------------------------------

// Directory relative to executable directory to use as a working directory
#define ZIPTEST_WORKING_DIR            "unitTests/zip"

// The filename of the zip file that we create for writing to
#define ZIPTEST_WRITE_FILENAME         "zipUnitTest.zip"

// The filename of the zip file created in a standard zip application
#define ZIPTEST_BASELINE_FILENAME      "zipUnitTestBaseline.zip"

// The filename of our working copy of the above so that we don't destroy the svn copy
#define ZIPTEST_WORKING_FILENAME       "zipUnitTestWorking.zip"

//-----------------------------------------------------------------------------
// Helper Functions
//-----------------------------------------------------------------------------

inline StringTableEntry makeTestPath(const char *path)
{
   char buffer[1024], dir[1024];
  
   Platform::makeFullPathName(ZIPTEST_WORKING_DIR, dir, sizeof(dir), Platform::getMainDotCsDir());
   Platform::makeFullPathName(path, buffer, sizeof(buffer), dir);

   return StringTable->insert(buffer, true);
}

//-----------------------------------------------------------------------------

} // end namespace Zip

#endif // _ZIPTEST_H_
