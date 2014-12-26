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
#include "console/console.h"
#include "console/consoleInternal.h"
#include "console/engineAPI.h"
#include "console/ast.h"
#include "core/stream/fileStream.h"
#include "console/compiler.h"
#include "platform/platformInput.h"
#include "torqueConfig.h"
#include "core/frameAllocator.h"

namespace Torque
{
   namespace FileSystem
   {
      // Buffer for expanding script filenames.
      extern char sgScriptFilenameBuffer[1024];
      extern  Vector<String>   sgFindFilesResults;
      extern  U32              sgFindFilesPos;

      //-------------------------------------- Helper Functions
      static void forwardslash(char *str);
      //----------------------------------------------------------------

      String findFirstFile ( const char* pattern, bool recurse =true);
      S32 buildFileList(const char* pattern, bool recurse, bool multiMatch);
      String findFirstFilefindFirstFile ( const char* pattern, bool recurse = true );
      String findNextFile ( const char* pattern = "");
      String findFirstFileMultiExpr( const char* pattern, bool recurse = true);
      String findNextFileMultiExpr( const char* pattern = "");
      S32 getFileCountMultiExpr( const char* pattern, bool recurse =true);
      U32 getFileCRC( const char* fileName );
      bool isFile ( const char* fileName );
      bool IsDirectory ( const char* directory );
      bool isWriteableFileName ( const char* fileName );

      void startFileChangeNotifications();
      void stopFileChangeNotifications();

      String getDirectoryList( const char* path, S32 depth = 0 );
      S32 fileSize( const char* fileName );
      String fileModifiedTime ( const char* fileName );
      String fileCreatedTime ( const char* fileName );
      bool fileDelete ( const char* path );
      String fileExt ( const char* fileName );
      String fileBase ( const char* fileName );
      String fileName  ( const char* fileName );
      String filePath ( const char* fileName );
      String getWorkingDirectory  ();
      String makeFullPath ( const char* path, const char* cwd = "");
      String makeRelativePath ( const char* path, const char* to = "");
      String pathConcat ( const char* path, const char* file);
      String getExecutableName();
      String getMainDotCsDir();
      S32 getFileCount( const char* pattern, bool recurse = true);
   }
}