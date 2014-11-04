#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _CONSOLE_H_
#include "console/console.h"
#endif
#ifndef _CONSOLEINTERNAL_H_
#include "console/consoleInternal.h"
#endif
#ifndef _ENGINEAPI_H_
#include "console/engineAPI.h"
#endif
#ifndef _AST_H_
#include "console/ast.h"
#endif
#ifndef _FILESTREAM_H_
#include "core/stream/fileStream.h"
#endif
#ifndef _COMPILER_H_
#include "console/compiler.h"
#endif
#ifndef _PLATFORMINPUT_H_
#include "platform/platformInput.h"
#endif
#ifndef _TORQUECONFIG_H_
#include "torqueConfig.h"
#endif
#ifndef _FRAMEALLOCATOR_H_
#include "core/frameAllocator.h"
#endif

namespace Torque
{
   namespace FS
   {
      // Buffer for expanding script filenames.
      static char sgScriptFilenameBuffer[1024];
      static Vector<String>   sgFindFilesResults;
      static U32              sgFindFilesPos = 0;

      static S32 buildFileList(const char* pattern, bool recurse, bool multiMatch);

      static void forwardslash(char *str);

      // Actual functions for the console members
      String findFirstFile(const char* pattern, bool recurse = true);

      String findNextFile(const char* pattern);

      S32 getFileCount(const char* pattern, bool recurse = true);

      String findFirstFileMultiExpr(const char* pattern, bool recurse = true);

      String findNextFileMultiExpr(const char* pattern);

      S32 getFileCountMultiExpr(const char* pattern, bool recurse = true);

      S32 getFileCRC(const char* fileName);

      bool isFile(const char* fileName);

      bool IsDirectory(const char* directory);

      bool isWriteableFileName(const char* fileName);

      String getDirectoryList(const char* path, S32 depth = 0);

      S32 fileSize(const char* fileName);

      String fileModifiedTime(const char* fileName);

      String fileCreatedTime(const char* fileName);

      bool fileDelete(const char* path);

      String fileExt(const char* fileName);

      String fileBase(const char* fileName);

      String fileName(const char* fileName);

      String filePath(const char* fileName);

      String getWorkingDirectory();

      String makeFullPath(const char* path, const char* cwd);

      String makeRelativePath(const char* path, const char* to);

      String pathConcat(const char* path, const char* file);

      String getExecutableName();

      String getMainDotCsDir();

      //-----------------------------------------------------------------------------
      // Tools Only Functions
      //-----------------------------------------------------------------------------

#ifdef TORQUE_TOOLS

      void openFolder(const char* path);

      void openFile(const char* file);

      bool pathCopy(const char* fromFile, const char* toFile, bool noOverwrite = true);

      String getCurrentDirectory();

      bool setCurrentDirectory(const char* path);

      bool createPath(const char* path);
#endif

   }
}