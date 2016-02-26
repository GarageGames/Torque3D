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

// Buffer for expanding script filenames.
static char sgScriptFilenameBuffer[1024];

//-------------------------------------- Helper Functions
static void forwardslash(char *str)
{
   while(*str)
   {
      if(*str == '\\')
         *str = '/';
      str++;
   }
}

//----------------------------------------------------------------

static Vector<String>   sgFindFilesResults;
static U32              sgFindFilesPos = 0;

static S32 buildFileList(const char* pattern, bool recurse, bool multiMatch)
{
   static const String sSlash( "/" );

   sgFindFilesResults.clear();

   String sPattern(Torque::Path::CleanSeparators(pattern));
   if(sPattern.isEmpty())
   {
      Con::errorf("findFirstFile() requires a search pattern");
      return -1;
   }

   if(!Con::expandScriptFilename(sgScriptFilenameBuffer, sizeof(sgScriptFilenameBuffer), sPattern.c_str()))
   {
      Con::errorf("findFirstFile() given initial directory cannot be expanded: '%s'", pattern);
      return -1;
   }
   sPattern = String::ToString(sgScriptFilenameBuffer);

   String::SizeType slashPos = sPattern.find('/', 0, String::Right);
//    if(slashPos == String::NPos)
//    {
//       Con::errorf("findFirstFile() missing search directory or expression: '%s'", sPattern.c_str());
//       return -1;
//    }

   // Build the initial search path
   Torque::Path givenPath(Torque::Path::CompressPath(sPattern));
   givenPath.setFileName("*");
   givenPath.setExtension("*");

   if(givenPath.getPath().length() > 0 && givenPath.getPath().find('*', 0, String::Right) == givenPath.getPath().length()-1)
   {
      // Deal with legacy searches of the form '*/*.*'
      String suspectPath = givenPath.getPath();
      String::SizeType newLen = suspectPath.length()-1;
      if(newLen > 0 && suspectPath.find('/', 0, String::Right) == suspectPath.length()-2)
      {
         --newLen;
      }
      givenPath.setPath(suspectPath.substr(0, newLen));
   }

   Torque::FS::FileSystemRef fs = Torque::FS::GetFileSystem(givenPath);
   //Torque::Path path = fs->mapTo(givenPath);
   Torque::Path path = givenPath;
   
   // Make sure that we have a root so the correct file system can be determined when using zips
   if(givenPath.isRelative())
      path = Torque::Path::Join(Torque::FS::GetCwd(), '/', givenPath);
   
   path.setFileName(String::EmptyString);
   path.setExtension(String::EmptyString);
   if(!Torque::FS::IsDirectory(path))
   {
      Con::errorf("findFirstFile() invalid initial search directory: '%s'", path.getFullPath().c_str());
      return -1;
   }

   // Build the search expression
   const String expression(slashPos != String::NPos ? sPattern.substr(slashPos+1) : sPattern);
   if(expression.isEmpty())
   {
      Con::errorf("findFirstFile() requires a search expression: '%s'", sPattern.c_str());
      return -1;
   }

   S32 results = Torque::FS::FindByPattern(path, expression, recurse, sgFindFilesResults, multiMatch );
   if(givenPath.isRelative() && results > 0)
   {
      // Strip the CWD out of the returned paths
      // MakeRelativePath() returns incorrect results (it adds a leading ..) so doing this the dirty way
      const String cwd = Torque::FS::GetCwd().getFullPath();
      for(S32 i = 0;i < sgFindFilesResults.size();++i)
      {
         String str = sgFindFilesResults[i];
         if(str.compare(cwd, cwd.length(), String::NoCase) == 0)
            str = str.substr(cwd.length());
         sgFindFilesResults[i] = str;
      }
   }
   return results;
}

//-----------------------------------------------------------------------------

DefineEngineFunction( findFirstFile, String, ( const char* pattern, bool recurse ), ( "", true ),
   "@brief Returns the first file in the directory system matching the given pattern.\n\n"

   "Use the corresponding findNextFile() to step through "
   "the results.  If you're only interested in the number of files returned by the "
   "pattern match, use getFileCount().\n\n"

   "This function differs from findFirstFileMultiExpr() in that it supports a single search "
   "pattern being passed in.\n\n"

   "@note You cannot run multiple simultaneous file system searches with these functions.  Each "
   "call to findFirstFile() and findFirstFileMultiExpr() initiates a new search and renders "
   "a previous search invalid.\n\n"

   "@param pattern The path and file name pattern to match against.\n"
   "@param recurse If true, the search will exhaustively recurse into subdirectories of the given path and match the given filename pattern.\n"
   "@return The path of the first file matched by the search or an empty string if no matching file could be found.\n\n"

   "@tsexample\n"
      "// Execute all .cs files in a subdirectory and its subdirectories.\n"
      "for( %file = findFirstFile( \"subdirectory/*.cs\" ); %file !$= \"\"; %file = findNextFile() )\n"
      "   exec( %file );\n"
   "@endtsexample\n\n"

   "@see findNextFile()"
   "@see getFileCount()"
   "@see findFirstFileMultiExpr()"
   "@ingroup FileSearches" )
{
   S32 numResults = buildFileList( pattern, recurse, false);

   // For Debugging
   //for ( S32 i = 0; i < sgFindFilesResults.size(); i++ )
   //   Con::printf( " [%i] [%s]", i, sgFindFilesResults[i].c_str() );

   sgFindFilesPos = 1;

   if(numResults < 0)
   {
      Con::errorf("findFirstFile() search directory not found: '%s'", pattern);
      return String();
   }

   return numResults ? sgFindFilesResults[0] : String();
}

//-----------------------------------------------------------------------------

DefineEngineFunction( findNextFile, String, ( const char* pattern ), ( "" ),
   "@brief Returns the next file matching a search begun in findFirstFile().\n\n"

   "@param pattern The path and file name pattern to match against.  This is optional "
   "and may be left out as it is not used by the code.  It is here for legacy reasons.\n"
   "@return The path of the next filename matched by the search or an empty string if no more files match.\n\n"

   "@tsexample\n"
      "// Execute all .cs files in a subdirectory and its subdirectories.\n"
      "for( %file = findFirstFile( \"subdirectory/*.cs\" ); %file !$= \"\"; %file = findNextFile() )\n"
      "   exec( %file );\n"
   "@endtsexample\n\n"

   "@see findFirstFile()"
   "@ingroup FileSearches" )
{
   if ( sgFindFilesPos + 1 > sgFindFilesResults.size() )
      return String();

   return sgFindFilesResults[sgFindFilesPos++];
}

//-----------------------------------------------------------------------------

DefineEngineFunction( getFileCount, S32, ( const char* pattern, bool recurse ), ( "", true ),
	"@brief Returns the number of files in the directory tree that match the given patterns\n\n"

   "This function differs from getFileCountMultiExpr() in that it supports a single search "
   "pattern being passed in.\n\n"

   "If you're interested in a list of files that match the given pattern and not just "
   "the number of files, use findFirstFile() and findNextFile().\n\n"

   "@param pattern The path and file name pattern to match against.\n"
   "@param recurse If true, the search will exhaustively recurse into subdirectories of the given path and match the given filename pattern "
      "counting files in subdirectories.\n"
   "@return Number of files located using the pattern\n\n"

   "@tsexample\n"
      "// Count the number of .cs files in a subdirectory and its subdirectories.\n"
      "getFileCount( \"subdirectory/*.cs\" );\n"
   "@endtsexample\n\n"

   "@see findFirstFile()"
   "@see findNextFile()"
   "@see getFileCountMultiExpr()"
   "@ingroup FileSearches" )
{
   S32 numResults = buildFileList( pattern, recurse, false );

   if(numResults < 0)
   {
      return 0;
   }

   return numResults;
}

//-----------------------------------------------------------------------------

DefineEngineFunction(findFirstFileMultiExpr, String, ( const char* pattern, bool recurse ), ( "", true),
	"@brief Returns the first file in the directory system matching the given patterns.\n\n"

   "Use the corresponding findNextFileMultiExpr() to step through "
   "the results.  If you're only interested in the number of files returned by the "
   "pattern match, use getFileCountMultiExpr().\n\n"

   "This function differs from findFirstFile() in that it supports multiple search patterns "
   "to be passed in.\n\n"

   "@note You cannot run multiple simultaneous file system searches with these functions.  Each "
   "call to findFirstFile() and findFirstFileMultiExpr() initiates a new search and renders "
   "a previous search invalid.\n\n"

	"@param pattern The path and file name pattern to match against, such as *.cs.  Separate "
   "multiple patterns with TABs.  For example: \"*.cs\" TAB \"*.dso\"\n"
	"@param recurse If true, the search will exhaustively recurse into subdirectories "
	"of the given path and match the given filename patterns.\n"
   "@return String of the first matching file path, or an empty string if no matching "
   "files were found.\n\n"

   "@tsexample\n"
      "// Find all DTS or Collada models\n"
      "%filePatterns = \"*.dts\" TAB \"*.dae\";\n"
      "%fullPath = findFirstFileMultiExpr( %filePatterns );\n"
      "while ( %fullPath !$= \"\" )\n"
      "{\n"
      "   echo( %fullPath );\n"
      "   %fullPath = findNextFileMultiExpr( %filePatterns );\n"
      "}\n"
   "@endtsexample\n\n"

   "@see findNextFileMultiExpr()"
   "@see getFileCountMultiExpr()"
   "@see findFirstFile()"
	"@ingroup FileSearches")
{
   S32 numResults = buildFileList(pattern, recurse, true);

   // For Debugging
   //for ( S32 i = 0; i < sgFindFilesResults.size(); i++ )
   //   Con::printf( " [%i] [%s]", i, sgFindFilesResults[i].c_str() );

   sgFindFilesPos = 1;

   if(numResults < 0)
   {
      Con::errorf("findFirstFileMultiExpr() search directory not found: '%s'", pattern);
      return String();
   }

   return numResults ? sgFindFilesResults[0] : String();
}

DefineEngineFunction(findNextFileMultiExpr, String, ( const char* pattern ), (""),
   "@brief Returns the next file matching a search begun in findFirstFileMultiExpr().\n\n"

	"@param pattern The path and file name pattern to match against.  This is optional "
   "and may be left out as it is not used by the code.  It is here for legacy reasons.\n"
   "@return String of the next matching file path, or an empty string if no matching "
   "files were found.\n\n"

   "@tsexample\n"
      "// Find all DTS or Collada models\n"
      "%filePatterns = \"*.dts\" TAB \"*.dae\";\n"
      "%fullPath = findFirstFileMultiExpr( %filePatterns );\n"
      "while ( %fullPath !$= \"\" )\n"
      "{\n"
      "   echo( %fullPath );\n"
      "   %fullPath = findNextFileMultiExpr( %filePatterns );\n"
      "}\n"
   "@endtsexample\n\n"

   "@see findFirstFileMultiExpr()"
	"@ingroup FileSearches")
{
   if ( sgFindFilesPos + 1 > sgFindFilesResults.size() )
      return String();

   return sgFindFilesResults[sgFindFilesPos++];
}

DefineEngineFunction(getFileCountMultiExpr, S32, ( const char* pattern, bool recurse ), ( "", true),
	"@brief Returns the number of files in the directory tree that match the given patterns\n\n"

   "If you're interested in a list of files that match the given patterns and not just "
   "the number of files, use findFirstFileMultiExpr() and findNextFileMultiExpr().\n\n"

	"@param pattern The path and file name pattern to match against, such as *.cs.  Separate "
   "multiple patterns with TABs.  For example: \"*.cs\" TAB \"*.dso\"\n"
	"@param recurse If true, the search will exhaustively recurse into subdirectories "
	"of the given path and match the given filename pattern.\n"
	"@return Number of files located using the patterns\n\n"

   "@tsexample\n"
      "// Count all DTS or Collada models\n"
      "%filePatterns = \"*.dts\" TAB \"*.dae\";\n"
      "echo( \"Nunmer of shape files:\" SPC getFileCountMultiExpr( %filePatterns ) );\n"
   "@endtsexample\n\n"

   "@see findFirstFileMultiExpr()"
   "@see findNextFileMultiExpr()"
	"@ingroup FileSearches")
{
   S32 numResults = buildFileList(pattern, recurse, true);

   if(numResults < 0)
   {
      return 0;
   }

   return numResults;
}

DefineEngineFunction(getFileCRC, S32, ( const char* fileName ),,
   "@brief Provides the CRC checksum of the given file.\n\n"
   
   "@param fileName The path to the file.\n"
   "@return The calculated CRC checksum of the file, or -1 if the file "
   "could not be found.\n"
   
   "@ingroup FileSystem")
{
   String cleanfilename(Torque::Path::CleanSeparators(fileName));
   Con::expandScriptFilename(sgScriptFilenameBuffer, sizeof(sgScriptFilenameBuffer), cleanfilename.c_str());

   Torque::Path givenPath(Torque::Path::CompressPath(sgScriptFilenameBuffer));
   Torque::FS::FileNodeRef fileRef = Torque::FS::GetFileNode( givenPath );

   if ( fileRef == NULL )
   {
      Con::errorf("getFileCRC() - could not access file: [%s]", givenPath.getFullPath().c_str() );
      return -1;
   }

   return fileRef->getChecksum();
}

DefineEngineFunction(isFile, bool, ( const char* fileName ),,
   "@brief Determines if the specified file exists or not\n\n"
   
   "@param fileName The path to the file.\n"
   "@return Returns true if the file was found.\n"
   
   "@ingroup FileSystem")
{
   String cleanfilename(Torque::Path::CleanSeparators(fileName));
   Con::expandScriptFilename(sgScriptFilenameBuffer, sizeof(sgScriptFilenameBuffer), cleanfilename.c_str());

   Torque::Path givenPath(Torque::Path::CompressPath(sgScriptFilenameBuffer));
   return Torque::FS::IsFile(givenPath);
}

DefineEngineFunction( IsDirectory, bool, ( const char* directory ),,
	"@brief Determines if a specified directory exists or not\n\n"

	"@param directory String containing path in the form of \"foo/bar\"\n"
   "@return Returns true if the directory was found.\n"

	"@note Do not include a trailing slash '/'.\n"

	"@ingroup FileSystem")
{
   String dir(Torque::Path::CleanSeparators(directory));
   Con::expandScriptFilename(sgScriptFilenameBuffer, sizeof(sgScriptFilenameBuffer), dir.c_str());

   Torque::Path givenPath(Torque::Path::CompressPath(sgScriptFilenameBuffer));
   return Torque::FS::IsDirectory( givenPath );
}

DefineEngineFunction(isWriteableFileName, bool, ( const char* fileName ),,
	"@brief Determines if a file name can be written to using File I/O\n\n"

	"@param fileName Name and path of file to check\n"
	"@return Returns true if the file can be written to.\n"

	"@ingroup FileSystem")
{
   String filename(Torque::Path::CleanSeparators(fileName));
   Con::expandScriptFilename(sgScriptFilenameBuffer, sizeof(sgScriptFilenameBuffer), filename.c_str());

   Torque::Path givenPath(Torque::Path::CompressPath(sgScriptFilenameBuffer));
   Torque::FS::FileSystemRef fs = Torque::FS::GetFileSystem(givenPath);
   Torque::Path path = fs->mapTo(givenPath);

   return !Torque::FS::IsReadOnly(path);
}

DefineEngineFunction(startFileChangeNotifications, void, (),,
	"@brief Start watching resources for file changes\n\n"
   "Typically this is called during initializeCore().\n\n"
   "@see stopFileChangeNotifications()\n"
	"@ingroup FileSystem")
{
   Torque::FS::StartFileChangeNotifications();
}

DefineEngineFunction(stopFileChangeNotifications, void, (),,
	"@brief Stop watching resources for file changes\n\n"
   "Typically this is called during shutdownCore().\n\n"
   "@see startFileChangeNotifications()\n"
	"@ingroup FileSystem")
{
   Torque::FS::StopFileChangeNotifications();
}


DefineEngineFunction(getDirectoryList, String, ( const char* path, S32 depth ), ( "", 0 ),
	"@brief Gathers a list of directories starting at the given path.\n\n"

	"@param path String containing the path of the directory\n"
	"@param depth Depth of search, as in how many subdirectories to parse through\n"
	"@return Tab delimited string containing list of directories found during search, \"\" if no files were found\n"

	"@ingroup FileSystem")
{
   // Grab the full path.
   char fullpath[1024];
   Platform::makeFullPathName(dStrcmp(path, "/") == 0 ? "" : path, fullpath, sizeof(fullpath));

   //dSprintf(fullpath, 511, "%s/%s", Platform::getWorkingDirectory(), path);

   // Append a trailing backslash if it's not present already.
   if (fullpath[dStrlen(fullpath) - 1] != '/')
   {
      S32 pos = dStrlen(fullpath);
      fullpath[pos] = '/';
      fullpath[pos + 1] = '\0';
   }

   // Dump the directories.
   Vector<StringTableEntry> directories;
   Platform::dumpDirectories(fullpath, directories, depth, true);

   if( directories.empty() )
      return "";

   // Grab the required buffer length.
   S32 length = 0;

   for (S32 i = 0; i < directories.size(); i++)
      length += dStrlen(directories[i]) + 1;

   // Get a return buffer.
   char* buffer = Con::getReturnBuffer(length);
   char* p = buffer;

   // Copy the directory names to the buffer.
   for (S32 i = 0; i < directories.size(); i++)
   {
      dStrcpy(p, directories[i]);
      p += dStrlen(directories[i]);
      // Tab separated.
      p[0] = '\t';
      p++;
   }
   p--;
   p[0] = '\0';

   return buffer;
}

DefineEngineFunction(fileSize, S32, ( const char* fileName ),,
	"@brief Determines the size of a file on disk\n\n"

	"@param fileName Name and path of the file to check\n"
	"@return Returns filesize in bytes, or -1 if no file\n"

	"@ingroup FileSystem")
{
   Con::expandScriptFilename(sgScriptFilenameBuffer, sizeof(sgScriptFilenameBuffer), fileName);
   return Platform::getFileSize( sgScriptFilenameBuffer );
}

DefineEngineFunction( fileModifiedTime, String, ( const char* fileName ),,
	"@brief Returns a platform specific formatted string with the last modified time for the file.\n\n"

	"@param fileName Name and path of file to check\n"
	"@return Formatted string (OS specific) containing modified time, \"9/3/2010 12:33:47 PM\" for example\n"
	"@ingroup FileSystem")
{
   Con::expandScriptFilename(sgScriptFilenameBuffer, sizeof(sgScriptFilenameBuffer), fileName);

   FileTime ft = {0};
   Platform::getFileTimes( sgScriptFilenameBuffer, NULL, &ft );

   Platform::LocalTime lt = {0};
   Platform::fileToLocalTime( ft, &lt );   
   
   String fileStr = Platform::localTimeToString( lt );
   
   char *buffer = Con::getReturnBuffer( fileStr.size() );
   dStrcpy( buffer, fileStr );   
   
   return buffer;
}

DefineEngineFunction( fileCreatedTime, String, ( const char* fileName ),,
   "@brief Returns a platform specific formatted string with the creation time for the file."

   "@param fileName Name and path of file to check\n"
   "@return Formatted string (OS specific) containing created time, \"9/3/2010 12:33:47 PM\" for example\n"
   "@ingroup FileSystem")
{
   Con::expandScriptFilename( sgScriptFilenameBuffer, sizeof(sgScriptFilenameBuffer), fileName );

   FileTime ft = {0};
   Platform::getFileTimes( sgScriptFilenameBuffer, &ft, NULL );

   Platform::LocalTime lt = {0};
   Platform::fileToLocalTime( ft, &lt );   

   String fileStr = Platform::localTimeToString( lt );

   char *buffer = Con::getReturnBuffer( fileStr.size() );
   dStrcpy( buffer, fileStr );  

   return buffer;
}

DefineEngineFunction(fileDelete, bool, ( const char* path ),,
	"@brief Delete a file from the hard drive\n\n"

	"@param path Name and path of the file to delete\n"
	"@note THERE IS NO RECOVERY FROM THIS. Deleted file is gone for good.\n"
	"@return True if file was successfully deleted\n"
	"@ingroup FileSystem")
{
   static char fileName[1024];
   static char sandboxFileName[1024];

   Con::expandScriptFilename( fileName, sizeof( fileName ), path );
   Platform::makeFullPathName(fileName, sandboxFileName, sizeof(sandboxFileName));

   return dFileDelete(sandboxFileName);
}


//----------------------------------------------------------------

DefineEngineFunction(fileExt, String, ( const char* fileName ),,
	"@brief Get the extension of a file\n\n"

	"@param fileName Name and path of file\n"
	"@return String containing the extension, such as \".exe\" or \".cs\"\n"
	"@ingroup FileSystem")
{
   const char *ret = dStrrchr(fileName, '.');
   if(ret)
      return ret;
   return "";
}

DefineEngineFunction(fileBase, String, ( const char* fileName ),,
   "@brief Get the base of a file name (removes extension and path)\n\n"

   "@param fileName Name and path of file to check\n"
   "@return String containing the file name, minus extension and path\n"
   "@ingroup FileSystem")
{

   S32 pathLen = dStrlen( fileName );
   FrameTemp<char> szPathCopy( pathLen + 1);

   dStrcpy( szPathCopy, fileName );
   forwardslash( szPathCopy );

   const char *path = dStrrchr(szPathCopy, '/');
   if(!path)
      path = szPathCopy;
   else
      path++;
   char *ret = Con::getReturnBuffer(dStrlen(path) + 1);
   dStrcpy(ret, path);
   char *ext = dStrrchr(ret, '.');
   if(ext)
      *ext = 0;
   return ret;
}

DefineEngineFunction(fileName, String, ( const char* fileName ),,
	"@brief Get only the file name of a path and file name string (removes path)\n\n"

	"@param fileName Name and path of file to check\n"
	"@return String containing the file name, minus the path\n"
	"@ingroup FileSystem")
{
   S32 pathLen = dStrlen( fileName );
   FrameTemp<char> szPathCopy( pathLen + 1);

   dStrcpy( szPathCopy, fileName );
   forwardslash( szPathCopy );

   const char *name = dStrrchr(szPathCopy, '/');
   if(!name)
      name = szPathCopy;
   else
      name++;
   char *ret = Con::getReturnBuffer(dStrlen(name));
   dStrcpy(ret, name);
   return ret;
}

DefineEngineFunction(filePath, String, ( const char* fileName ),,
	"@brief Get the path of a file (removes name and extension)\n\n"

	"@param fileName Name and path of file to check\n"
	"@return String containing the path, minus name and extension\n"
	"@ingroup FileSystem")
{
   S32 pathLen = dStrlen( fileName );
   FrameTemp<char> szPathCopy( pathLen + 1);

   dStrcpy( szPathCopy, fileName );
   forwardslash( szPathCopy );

   const char *path = dStrrchr(szPathCopy, '/');
   if(!path)
      return "";
   U32 len = path - (char*)szPathCopy;
   char *ret = Con::getReturnBuffer(len + 1);
   dStrncpy(ret, szPathCopy, len);
   ret[len] = 0;
   return ret;
}

DefineEngineFunction(getWorkingDirectory, String, (),,
	"@brief Reports the current directory\n\n"

	"@return String containing full file path of working directory\n"
	"@ingroup FileSystem")
{
   return Platform::getCurrentDirectory();
}

//-----------------------------------------------------------------------------

// [tom, 5/1/2007] I changed these to be ordinary console functions as they
// are just string processing functions. They are needed by the 3D tools which
// are not currently built with TORQUE_TOOLS defined.

DefineEngineFunction(makeFullPath, String, ( const char* path, const char* cwd ), ( "", ""),
	"@brief Converts a relative file path to a full path\n\n"

	"For example, \"./console.log\" becomes \"C:/Torque/t3d/examples/FPS Example/game/console.log\"\n"
	"@param path Name of file or path to check\n"
   "@param cwd Optional current working directory from which to build the full path.\n"
	"@return String containing non-relative directory of path\n"
	"@ingroup FileSystem")
{
   static const U32 bufSize = 512;
   char *buf = Con::getReturnBuffer(bufSize);
   Platform::makeFullPathName(path, buf, bufSize, dStrlen(cwd) > 1 ? cwd : NULL);
   return buf;
}

DefineEngineFunction(makeRelativePath, String, ( const char* path, const char* to ), ( "", ""),
	"@brief Turns a full or local path to a relative one\n\n"

   "For example, \"./game/art\" becomes \"game/art\"\n"
   "@param path Full path (may include a file) to convert\n"
   "@param to Optional base path used for the conversion.  If not supplied the current "
   "working directory is used.\n"
	"@returns String containing relative path\n"
	"@ingroup FileSystem")
{
   return Platform::makeRelativePathName( path, dStrlen(to) > 1 ? to : NULL );
}

DefineEngineFunction(pathConcat, String, ( const char* path, const char* file), ( "", ""),
	"@brief Combines two separate strings containing a file path and file name together into a single string\n\n"

	"@param path String containing file path\n"
	"@param file String containing file name\n"
	"@return String containing concatenated file name and path\n"
	"@ingroup FileSystem")
{
   static const U32 bufSize = 1024;
   char *buf = Con::getReturnBuffer(bufSize);
   Platform::makeFullPathName(file, buf, bufSize, path);
   return buf;
}

//-----------------------------------------------------------------------------

DefineEngineFunction(getExecutableName, String, (),,
	"@brief Gets the name of the game's executable\n\n"

	"@return String containing this game's executable name\n"
	"@ingroup FileSystem")
{
   return Platform::getExecutableName();
}

//-----------------------------------------------------------------------------

DefineEngineFunction( getMainDotCsDir, String, (),,
   "@brief Get the absolute path to the directory that contains the main.cs script from which the engine was started.\n\n"

   "This directory will usually contain all the game assets and, in a user-side game installation, will usually be "
   "read-only.\n\n"
   "@return The path to the main game assets.\n\n"
   "@ingroup FileSystem\n")
{
   return Platform::getMainDotCsDir();
}

//-----------------------------------------------------------------------------
// Tools Only Functions
//-----------------------------------------------------------------------------

#ifdef TORQUE_TOOLS

//-----------------------------------------------------------------------------

DefineEngineFunction( openFolder, void, ( const char* path ),,
   "@brief Open the given folder in the system's file manager.\n\n"
   "@param path full path to a directory.\n\n"
   "@note Only present in a Tools build of Torque.\n"
   "@ingroup FileSystem\n")
{
   Platform::openFolder( path );
}

//-----------------------------------------------------------------------------

DefineEngineFunction( openFile, void, ( const char* file ),,
   "@brief Open the given @a file through the system.  This will usually open the file in its "
   "associated application.\n"
   "@param file %Path of the file to open.\n\n"
   "@note Only present in a Tools build of Torque.\n"
   "@ingroup FileSystem\n")
{
   Platform::openFile( file );
}

//-----------------------------------------------------------------------------

DefineEngineFunction( pathCopy, bool, ( const char* fromFile, const char* toFile, bool noOverwrite ), ( "", "", true ),
   "@brief Copy a file to a new location.\n"
   "@param fromFile %Path of the file to copy.\n"
   "@param toFile %Path where to copy @a fromFile to.\n"
   "@param noOverwrite If true, then @a fromFile will not overwrite a file that may already exist at @a toFile.\n"
   "@return True if the file was successfully copied, false otherwise.\n"
   "@note Only present in a Tools build of Torque.\n"
   "@ingroup FileSystem")
{
   char qualifiedFromFile[ 2048 ];
   char qualifiedToFile[ 2048 ];
   
   Platform::makeFullPathName( fromFile, qualifiedFromFile, sizeof( qualifiedFromFile ) );
   Platform::makeFullPathName( toFile, qualifiedToFile, sizeof( qualifiedToFile ) );

   return dPathCopy( qualifiedFromFile, qualifiedToFile, noOverwrite );
}

//-----------------------------------------------------------------------------

DefineEngineFunction( getCurrentDirectory, String, (),,
   "@brief Return the current working directory.\n\n"
   "@return The absolute path of the current working directory.\n\n"
   "@note Only present in a Tools build of Torque.\n"
   "@see getWorkingDirectory()"
   "@ingroup FileSystem")
{
   return Platform::getCurrentDirectory();
}

//-----------------------------------------------------------------------------

DefineEngineFunction( setCurrentDirectory, bool, ( const char* path ),,
   "@brief Set the current working directory.\n\n"
   "@param path The absolute or relative (to the current working directory) path of the directory which should be made the new "
      "working directory.\n\n"
   "@return True if the working directory was successfully changed to @a path, false otherwise.\n\n"
   "@note Only present in a Tools build of Torque.\n"
   "@ingroup FileSystem")
{
   return Platform::setCurrentDirectory( StringTable->insert( path ) );

}

//-----------------------------------------------------------------------------

DefineEngineFunction( createPath, bool, ( const char* path ),,
   "@brief Create the given directory or the path leading to the given filename.\n\n"
   "If @a path ends in a trailing slash, then all components in the given path will be created as directories (if not already in place).  If @a path, "
   "does @b not end in a trailing slash, then the last component of the path is taken to be a file name and only the directory components "
   "of the path will be created.\n\n"
   "@param path The path to create.\n\n"
   "@note Only present in a Tools build of Torque.\n"
   "@ingroup FileSystem" )
{
   static char pathName[1024];

   Con::expandScriptFilename( pathName, sizeof( pathName ), path );

   return Platform::createPath( pathName );
}

#endif // TORQUE_TOOLS
