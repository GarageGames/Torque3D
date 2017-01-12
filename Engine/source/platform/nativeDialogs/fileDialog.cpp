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

#include "console/simBase.h"
#include "platform/nativeDialogs/fileDialog.h"
#include "platform/threads/mutex.h"
#include "core/util/safeDelete.h"
#include "math/mMath.h"
#include "core/strings/unicode.h"
#include "console/consoleTypes.h"
#include "platform/profiler.h"
#include "console/engineAPI.h"
#include <nfd.h>
#include "core/strings/stringUnit.h"
#include "core/frameAllocator.h"

#if defined(TORQUE_SDL)
//-----------------------------------------------------------------------------
// PlatformFileDlgData Implementation
//-----------------------------------------------------------------------------
FileDialogData::FileDialogData()
{
   // Default Path
   //
   //  Try to provide consistent experience by recalling the last file path
   // - else
   //  Default to Working Directory if last path is not set or is invalid
   mDefaultPath = StringTable->insert(Con::getVariable("Tools::FileDialogs::LastFilePath"));
   if (mDefaultPath == StringTable->lookup("") || !Platform::isDirectory(mDefaultPath))
      mDefaultPath = Platform::getCurrentDirectory();

   mDefaultFile = StringTable->EmptyString();
   mFilters = StringTable->EmptyString();
   mFile = StringTable->EmptyString();
   mTitle = StringTable->EmptyString();

   mStyle = 0;

}
FileDialogData::~FileDialogData()
{

}

//-----------------------------------------------------------------------------
// FileDialog Implementation
//-----------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(FileDialog);

ConsoleDocClass(FileDialog,
   "@brief Base class responsible for displaying an OS file browser.\n\n"

   "FileDialog is a platform agnostic dialog interface for querying the user for "
   "file locations. It is designed to be used through the exposed scripting interface.\n\n"

   "FileDialog is the base class for Native File Dialog controls in Torque. It provides these basic areas of functionality:\n\n"
   "   - Inherits from SimObject and is exposed to the scripting interface\n"
   "   - Provides blocking interface to allow instant return to script execution\n"
   "   - Simple object configuration makes practical use easy and effective\n\n"

   "FileDialog is *NOT* intended to be used directly in script and is only exposed to script to expose generic file dialog attributes.\n\n"

   "This base class is usable in TorqueScript, but is does not specify what functionality is intended (open or save?). "
   "Its children, OpenFileDialog and SaveFileDialog, do make use of DialogStyle flags and do make use of specific funcationality. "
   "These are the preferred classes to use\n\n"

   "However, the FileDialog base class does contain the key properties and important method for file browing. The most "
   "important function is Execute(). This is used by both SaveFileDialog and OpenFileDialog to initiate the browser.\n\n"

   "@tsexample\n"
   "// NOTE: This is not he preferred class to use, but this still works\n\n"
   "// Create the file dialog\n"
   "%baseFileDialog = new FileDialog()\n"
   "{\n"
   "   // Allow browsing of all file types\n"
   "   filters = \"*.*\";\n\n"
   "   // No default file\n"
   "   defaultFile = "";\n\n"
   "   // Set default path relative to project\n"
   "   defaultPath = \"./\";\n\n"
   "   // Set the title\n"
   "   title = \"Durpa\";\n\n"
   "   // Allow changing of path you are browsing\n"
   "   changePath = true;\n"
   "};\n\n"
   " // Launch the file dialog\n"
   " %baseFileDialog.Execute();\n"
   " \n"
   " // Don't forget to cleanup\n"
   " %baseFileDialog.delete();\n\n\n"
   "@endtsexample\n\n"

   "@note FileDialog and its related classes are only availble in a Tools build of Torque.\n\n"

   "@see OpenFileDialog for a practical example on opening a file\n"
   "@see SaveFileDialog for a practical example of saving a file\n"

   "@ingroup FileSystem\n"
   );

FileDialog::FileDialog() : mData()
{
   // Default to File Must Exist Open Dialog style
   mData.mStyle = FileDialogData::FDS_OPEN | FileDialogData::FDS_MUSTEXIST;
   mChangePath = false;
}

FileDialog::~FileDialog()
{
}

void FileDialog::initPersistFields()
{
   addProtectedField("defaultPath", TypeString, Offset(mData.mDefaultPath, FileDialog), &setDefaultPath, &defaultProtectedGetFn,
      "The default directory path when the dialog is shown.");

   addProtectedField("defaultFile", TypeString, Offset(mData.mDefaultFile, FileDialog), &setDefaultFile, &defaultProtectedGetFn,
      "The default file path when the dialog is shown.");

   addProtectedField("fileName", TypeString, Offset(mData.mFile, FileDialog), &setFile, &defaultProtectedGetFn,
      "The default file name when the dialog is shown.");

   addProtectedField("filters", TypeString, Offset(mData.mFilters, FileDialog), &setFilters, &defaultProtectedGetFn,
      "The filter string for limiting the types of files visible in the dialog.  It makes use of the pipe symbol '|' "
      "as a delimiter.  For example:\n\n"
      "'All Files|*.*'\n\n"
      "'Image Files|*.png;*.jpg|Png Files|*.png|Jepg Files|*.jpg'");

   addField("title", TypeString, Offset(mData.mTitle, FileDialog),
      "The title for the dialog.");

   addProtectedField("changePath", TypeBool, Offset(mChangePath, FileDialog), &setChangePath, &getChangePath,
      "True/False whether to set the working directory to the directory returned by the dialog.");

   Parent::initPersistFields();
}

static const U32 convertUTF16toUTF8DoubleNULL(const UTF16 *unistring, UTF8  *outbuffer, U32 len)
{
   AssertFatal(len >= 1, "Buffer for unicode conversion must be large enough to hold at least the null terminator.");
   PROFILE_START(convertUTF16toUTF8DoubleNULL);
   U32 walked, nCodeunits, codeunitLen;
   UTF32 middleman;

   nCodeunits = 0;
   while (!(*unistring == '\0' && *(unistring + 1) == '\0') && nCodeunits + 3 < len)
   {
      walked = 1;
      middleman = oneUTF16toUTF32(unistring, &walked);
      codeunitLen = oneUTF32toUTF8(middleman, &outbuffer[nCodeunits]);
      unistring += walked;
      nCodeunits += codeunitLen;
   }

   nCodeunits = getMin(nCodeunits, len - 1);
   outbuffer[nCodeunits] = '\0';
   outbuffer[nCodeunits + 1] = '\0';

   PROFILE_END();
   return nCodeunits;
}

//
// Execute Method
//
bool FileDialog::Execute()
{
   String strippedFilters;

   U32 filtersCount = StringUnit::getUnitCount(mData.mFilters, "|");

   for (U32 i = 1; i < filtersCount; ++i)
   {
      //The first of each pair is the name, which we'll skip because NFD doesn't support named filters atm
      String filter = StringUnit::getUnit(mData.mFilters, i, "|");

      if (!dStrcmp(filter.c_str(), "*.*"))
         continue;

      U32 subFilterCount = StringUnit::getUnitCount(filter, ";");

      //if we have a 'super filter', break it down to sub-options as well
      if (subFilterCount > 1)
      {
         String suffixFilter;
         String subFilters;

         for (U32 f = 0; f < subFilterCount; ++f)
         {
            String subFilter = StringUnit::getUnit(filter, f, ";");

            suffixFilter += String::ToLower(subFilter) + "," + String::ToUpper(subFilter) + ",";
            subFilters += String::ToLower(subFilter) + "," + String::ToUpper(subFilter) + ";";
         }

         suffixFilter = suffixFilter.substr(0, suffixFilter.length() - 1);
         suffixFilter += ";";

         strippedFilters += suffixFilter + subFilters;
      }
      else //otherwise, just add the filter
      {
         strippedFilters += String::ToLower(filter) + "," + String::ToUpper(filter) + ";";
      }

      ++i;
      if (i < filtersCount - 2)
         strippedFilters += String(";");
   }

   //strip the last character, if it's unneeded
   if (strippedFilters.endsWith(";"))
   {
      strippedFilters = strippedFilters.substr(0, strippedFilters.length() - 1);
   }

   strippedFilters.replace("*.", "");

   // Get the current working directory, so we can back up to it once Windows has
   // done its craziness and messed with it.
   StringTableEntry cwd = Platform::getCurrentDirectory();
   if (mData.mDefaultPath == StringTable->lookup("") || !Platform::isDirectory(mData.mDefaultPath))
      mData.mDefaultPath = cwd;
   String rootDir = String(cwd);
   // Execute Dialog (Blocking Call)
   nfdchar_t *outPath = NULL;
   nfdpathset_t pathSet;

   nfdresult_t result = NFD_ERROR;
   String defaultPath = String(mData.mDefaultPath);
#if defined(TORQUE_OS_WIN)
   defaultPath.replace("/", "\\");
   rootDir.replace("/", "\\");
#endif

   if (mData.mStyle & FileDialogData::FDS_OPEN)
      result = NFD_OpenDialog(strippedFilters.c_str(), defaultPath.c_str(), &outPath);
   else if (mData.mStyle & FileDialogData::FDS_SAVE)
      result = NFD_SaveDialog(strippedFilters.c_str(), defaultPath.c_str(), &outPath);
   else if (mData.mStyle & FileDialogData::FDS_MULTIPLEFILES)
      result = NFD_OpenDialogMultiple(strippedFilters.c_str(), defaultPath.c_str(), &pathSet);

   if (result == NFD_CANCEL)
   {
      return false;
   }

   String resultPath = String(outPath).replace(rootDir, String(""));
   resultPath = resultPath.replace(0, 1, String("")).c_str(); //kill '\\' prefix
   resultPath = resultPath.replace(String("\\"), String("/"));

   // Did we select a file?
   if (result != NFD_OKAY)
   {
      Con::errorf("NFD plugin error: %s", NFD_GetError());
      return false;
   }
   // Store the result on our object
   if (mData.mStyle & FileDialogData::FDS_OPEN || mData.mStyle & FileDialogData::FDS_SAVE)
   {
      // Single file selection, do it the easy way
      mData.mFile = Platform::makeRelativePathName(resultPath.c_str(), NULL);
   }
   else if (mData.mStyle & FileDialogData::FDS_MULTIPLEFILES)
   {
      //check if we have multiple files actually selected or not
      U32 fileCount = NFD_PathSet_GetCount(&pathSet);
      if (fileCount > 1)
      {
         //yep, so parse through them and prep our return
         for (U32 i = 0; i < fileCount; ++i)
         {
            nfdchar_t *path = NFD_PathSet_GetPath(&pathSet, i);
            setDataField(StringTable->insert("files"), Con::getIntArg(i), path);
         }

         setDataField(StringTable->insert("fileCount"), NULL, Con::getIntArg(fileCount));
      }
      else
      {
         //nope, just one file, so set it as normal
         setDataField(StringTable->insert("files"), "0", Platform::makeRelativePathName(resultPath.c_str(), NULL));
         setDataField(StringTable->insert("fileCount"), NULL, "1");
      }
   }

   // Return success.
   return true;
}

DefineEngineMethod(FileDialog, Execute, bool, (), ,
   "@brief Launches the OS file browser\n\n"

   "After an Execute() call, the chosen file name and path is available in one of two areas.  "
   "If only a single file selection is permitted, the results will be stored in the @a fileName "
   "attribute.\n\n"

   "If multiple file selection is permitted, the results will be stored in the "
   "@a files array.  The total number of files in the array will be stored in the "
   "@a fileCount attribute.\n\n"

   "@tsexample\n"
   "// NOTE: This is not he preferred class to use, but this still works\n\n"
   "// Create the file dialog\n"
   "%baseFileDialog = new FileDialog()\n"
   "{\n"
   "   // Allow browsing of all file types\n"
   "   filters = \"*.*\";\n\n"
   "   // No default file\n"
   "   defaultFile = "";\n\n"
   "   // Set default path relative to project\n"
   "   defaultPath = \"./\";\n\n"
   "   // Set the title\n"
   "   title = \"Durpa\";\n\n"
   "   // Allow changing of path you are browsing\n"
   "   changePath = true;\n"
   "};\n\n"
   " // Launch the file dialog\n"
   " %baseFileDialog.Execute();\n"
   " \n"
   " // Don't forget to cleanup\n"
   " %baseFileDialog.delete();\n\n\n"

   " // A better alternative is to use the \n"
   " // derived classes which are specific to file open and save\n\n"
   " // Create a dialog dedicated to opening files\n"
   " %openFileDlg = new OpenFileDialog()\n"
   " {\n"
   "    // Look for jpg image files\n"
   "    // First part is the descriptor|second part is the extension\n"
   "    Filters = \"Jepg Files|*.jpg\";\n"
   "    // Allow browsing through other folders\n"
   "    ChangePath = true;\n\n"
   "    // Only allow opening of one file at a time\n"
   "    MultipleFiles = false;\n"
   " };\n\n"
   " // Launch the open file dialog\n"
   " %result = %openFileDlg.Execute();\n\n"
   " // Obtain the chosen file name and path\n"
   " if ( %result )\n"
   " {\n"
   "    %seletedFile = %openFileDlg.file;\n"
   " }\n"
   " else\n"
   " {\n"
   "    %selectedFile = \"\";\n"
   " }\n"
   " // Cleanup\n"
   " %openFileDlg.delete();\n\n\n"

   " // Create a dialog dedicated to saving a file\n"
   " %saveFileDlg = new SaveFileDialog()\n"
   " {\n"
   "    // Only allow for saving of COLLADA files\n"
   "    Filters = \"COLLADA Files (*.dae)|*.dae|\";\n\n"
   "    // Default save path to where the WorldEditor last saved\n"
   "    DefaultPath = $pref::WorldEditor::LastPath;\n\n"
   "    // No default file specified\n"
   "    DefaultFile = \"\";\n\n"
   "    // Do not allow the user to change to a new directory\n"
   "    ChangePath = false;\n\n"
   "    // Prompt the user if they are going to overwrite an existing file\n"
   "    OverwritePrompt = true;\n"
   " };\n\n"
   " // Launch the save file dialog\n"
   " %result = %saveFileDlg.Execute();\n\n"
   " // Obtain the file name\n"
   " %selectedFile = \"\";\n"
   " if ( %result )\n"
   "    %selectedFile = %saveFileDlg.file;\n\n"
   " // Cleanup\n"
   " %saveFileDlg.delete();\n"
   "@endtsexample\n\n"

   "@return True if the file was selected was successfully found (opened) or declared (saved).")
{
   return object->Execute();
}

//-----------------------------------------------------------------------------
// Dialog Filters
//-----------------------------------------------------------------------------
bool FileDialog::setFilters(void *object, const char *index, const char *data)
{
   // Will do validate on write at some point.
   if (!data)
      return true;

   return true;

};


//-----------------------------------------------------------------------------
// Default Path Property - String Validated on Write
//-----------------------------------------------------------------------------
bool FileDialog::setDefaultPath(void *object, const char *index, const char *data)
{
   if (!data || !dStrncmp(data, "", 1))
      return true;

   // Copy and Backslash the path (Windows dialogs are VERY picky about this format)
   static char szPathValidate[512];
   dStrcpy(szPathValidate, data);

   Platform::makeFullPathName(data, szPathValidate, sizeof(szPathValidate));
   //backslash( szPathValidate );

   // Remove any trailing \'s
   S8 validateLen = dStrlen(szPathValidate);
   if (szPathValidate[validateLen - 1] == '\\')
      szPathValidate[validateLen - 1] = '\0';

   // Now check
   if (Platform::isDirectory(szPathValidate))
   {
      // Finally, assign in proper format.
      FileDialog *pDlg = static_cast<FileDialog*>(object);
      pDlg->mData.mDefaultPath = StringTable->insert(szPathValidate);
   }
#ifdef TORQUE_DEBUG
   else
      Con::errorf(ConsoleLogEntry::GUI, "FileDialog - Invalid Default Path Specified!");
#endif

   return false;

};

//-----------------------------------------------------------------------------
// Default File Property - String Validated on Write
//-----------------------------------------------------------------------------
bool FileDialog::setDefaultFile(void *object, const char *index, const char *data)
{
   if (!data || !dStrncmp(data, "", 1))
      return true;

   // Copy and Backslash the path (Windows dialogs are VERY picky about this format)
   static char szPathValidate[512];
   Platform::makeFullPathName(data, szPathValidate, sizeof(szPathValidate));
   //backslash( szPathValidate );

   // Remove any trailing \'s
   S8 validateLen = dStrlen(szPathValidate);
   if (szPathValidate[validateLen - 1] == '\\')
      szPathValidate[validateLen - 1] = '\0';

   // Finally, assign in proper format.
   FileDialog *pDlg = static_cast<FileDialog*>(object);
   pDlg->mData.mDefaultFile = StringTable->insert(szPathValidate);

   return false;
};

//-----------------------------------------------------------------------------
// ChangePath Property - Change working path on successful file selection
//-----------------------------------------------------------------------------
bool FileDialog::setChangePath(void *object, const char *index, const char *data)
{
   bool bMustExist = dAtob(data);

   FileDialog *pDlg = static_cast<FileDialog*>(object);

   if (bMustExist)
      pDlg->mData.mStyle |= FileDialogData::FDS_CHANGEPATH;
   else
      pDlg->mData.mStyle &= ~FileDialogData::FDS_CHANGEPATH;

   return true;
};

const char* FileDialog::getChangePath(void* obj, const char* data)
{
   FileDialog *pDlg = static_cast<FileDialog*>(obj);
   if (pDlg->mData.mStyle & FileDialogData::FDS_CHANGEPATH)
      return StringTable->insert("true");
   else
      return StringTable->insert("false");
}

bool FileDialog::setFile(void *object, const char *index, const char *data)
{
   return false;
};

//-----------------------------------------------------------------------------
// OpenFileDialog Implementation
//-----------------------------------------------------------------------------

ConsoleDocClass(OpenFileDialog,
   "@brief Derived from FileDialog, this class is responsible for opening a file browser with the intention of opening a file.\n\n"

   "The core usage of this dialog is to locate a file in the OS and return the path and name. This does not handle "
   "the actual file parsing or data manipulation. That functionality is left up to the FileObject class.\n\n"

   "@tsexample\n"
   " // Create a dialog dedicated to opening files\n"
   " %openFileDlg = new OpenFileDialog()\n"
   " {\n"
   "    // Look for jpg image files\n"
   "    // First part is the descriptor|second part is the extension\n"
   "    Filters = \"Jepg Files|*.jpg\";\n"
   "    // Allow browsing through other folders\n"
   "    ChangePath = true;\n\n"
   "    // Only allow opening of one file at a time\n"
   "    MultipleFiles = false;\n"
   " };\n\n"
   " // Launch the open file dialog\n"
   " %result = %openFileDlg.Execute();\n\n"
   " // Obtain the chosen file name and path\n"
   " if ( %result )\n"
   " {\n"
   "    %seletedFile = %openFileDlg.file;\n"
   " }\n"
   " else\n"
   " {\n"
   "    %selectedFile = \"\";\n"
   " }\n\n"
   " // Cleanup\n"
   " %openFileDlg.delete();\n\n\n"
   "@endtsexample\n\n"

   "@note FileDialog and its related classes are only availble in a Tools build of Torque.\n\n"

   "@see FileDialog\n"
   "@see SaveFileDialog\n"
   "@see FileObject\n"

   "@ingroup FileSystem\n"
   );
OpenFileDialog::OpenFileDialog()
{
   // Default File Must Exist
   mData.mStyle = FileDialogData::FDS_OPEN | FileDialogData::FDS_MUSTEXIST;
}

OpenFileDialog::~OpenFileDialog()
{
   mMustExist = true;
   mMultipleFiles = false;
}

IMPLEMENT_CONOBJECT(OpenFileDialog);

//-----------------------------------------------------------------------------
// Console Properties
//-----------------------------------------------------------------------------
void OpenFileDialog::initPersistFields()
{
   addProtectedField("MustExist", TypeBool, Offset(mMustExist, OpenFileDialog), &setMustExist, &getMustExist, "True/False whether the file returned must exist or not");
   addProtectedField("MultipleFiles", TypeBool, Offset(mMultipleFiles, OpenFileDialog), &setMultipleFiles, &getMultipleFiles, "True/False whether multiple files may be selected and returned or not");

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------
// File Must Exist - Boolean
//-----------------------------------------------------------------------------
bool OpenFileDialog::setMustExist(void *object, const char *index, const char *data)
{
   bool bMustExist = dAtob(data);

   OpenFileDialog *pDlg = static_cast<OpenFileDialog*>(object);

   if (bMustExist)
      pDlg->mData.mStyle |= FileDialogData::FDS_MUSTEXIST;
   else
      pDlg->mData.mStyle &= ~FileDialogData::FDS_MUSTEXIST;

   return true;
};

const char* OpenFileDialog::getMustExist(void* obj, const char* data)
{
   OpenFileDialog *pDlg = static_cast<OpenFileDialog*>(obj);
   if (pDlg->mData.mStyle & FileDialogData::FDS_MUSTEXIST)
      return StringTable->insert("true");
   else
      return StringTable->insert("false");
}

//-----------------------------------------------------------------------------
// Can Select Multiple Files - Boolean
//-----------------------------------------------------------------------------
bool OpenFileDialog::setMultipleFiles(void *object, const char *index, const char *data)
{
   bool bMustExist = dAtob(data);

   OpenFileDialog *pDlg = static_cast<OpenFileDialog*>(object);

   if (bMustExist)
      pDlg->mData.mStyle |= FileDialogData::FDS_MULTIPLEFILES;
   else
      pDlg->mData.mStyle &= ~FileDialogData::FDS_MULTIPLEFILES;

   return true;
};

const char* OpenFileDialog::getMultipleFiles(void* obj, const char* data)
{
   OpenFileDialog *pDlg = static_cast<OpenFileDialog*>(obj);
   if (pDlg->mData.mStyle & FileDialogData::FDS_MULTIPLEFILES)
      return StringTable->insert("true");
   else
      return StringTable->insert("false");
}

//-----------------------------------------------------------------------------
// SaveFileDialog Implementation
//-----------------------------------------------------------------------------
ConsoleDocClass(SaveFileDialog,
   "@brief Derived from FileDialog, this class is responsible for opening a file browser with the intention of saving a file.\n\n"

   "The core usage of this dialog is to locate a file in the OS and return the path and name. This does not handle "
   "the actual file writing or data manipulation. That functionality is left up to the FileObject class.\n\n"

   "@tsexample\n"
   " // Create a dialog dedicated to opening file\n"
   " %saveFileDlg = new SaveFileDialog()\n"
   " {\n"
   "    // Only allow for saving of COLLADA files\n"
   "    Filters        = \"COLLADA Files (*.dae)|*.dae|\";\n\n"
   "    // Default save path to where the WorldEditor last saved\n"
   "    DefaultPath    = $pref::WorldEditor::LastPath;\n\n"
   "    // No default file specified\n"
   "    DefaultFile    = \"\";\n\n"
   "    // Do not allow the user to change to a new directory\n"
   "    ChangePath     = false;\n\n"
   "    // Prompt the user if they are going to overwrite an existing file\n"
   "    OverwritePrompt   = true;\n"
   " };\n\n"
   " // Launch the save file dialog\n"
   " %saveFileDlg.Execute();\n\n"
   " if ( %result )\n"
   " {\n"
   "    %seletedFile = %openFileDlg.file;\n"
   " }\n"
   " else\n"
   " {\n"
   "    %selectedFile = \"\";\n"
   " }\n\n"
   " // Cleanup\n"
   " %saveFileDlg.delete();\n"
   "@endtsexample\n\n"

   "@note FileDialog and its related classes are only availble in a Tools build of Torque.\n\n"

   "@see FileDialog\n"
   "@see OpenFileDialog\n"
   "@see FileObject\n"

   "@ingroup FileSystem\n"
   );
SaveFileDialog::SaveFileDialog()
{
   // Default File Must Exist
   mData.mStyle = FileDialogData::FDS_SAVE | FileDialogData::FDS_OVERWRITEPROMPT;
   mOverwritePrompt = true;
}

SaveFileDialog::~SaveFileDialog()
{
}

IMPLEMENT_CONOBJECT(SaveFileDialog);

//-----------------------------------------------------------------------------
// Console Properties
//-----------------------------------------------------------------------------
void SaveFileDialog::initPersistFields()
{
   addProtectedField("OverwritePrompt", TypeBool, Offset(mOverwritePrompt, SaveFileDialog), &setOverwritePrompt, &getOverwritePrompt, "True/False whether the dialog should prompt before accepting an existing file name");

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------
// Prompt on Overwrite - Boolean
//-----------------------------------------------------------------------------
bool SaveFileDialog::setOverwritePrompt(void *object, const char *index, const char *data)
{
   bool bMustExist = dAtob(data);

   SaveFileDialog *pDlg = static_cast<SaveFileDialog*>(object);

   if (bMustExist)
      pDlg->mData.mStyle |= FileDialogData::FDS_OVERWRITEPROMPT;
   else
      pDlg->mData.mStyle &= ~FileDialogData::FDS_OVERWRITEPROMPT;

   return true;
};

const char* SaveFileDialog::getOverwritePrompt(void* obj, const char* data)
{
   SaveFileDialog *pDlg = static_cast<SaveFileDialog*>(obj);
   if (pDlg->mData.mStyle & FileDialogData::FDS_OVERWRITEPROMPT)
      return StringTable->insert("true");
   else
      return StringTable->insert("false");
}

//-----------------------------------------------------------------------------
// OpenFolderDialog Implementation
//-----------------------------------------------------------------------------

OpenFolderDialog::OpenFolderDialog()
{
   mData.mStyle = FileDialogData::FDS_OPEN | FileDialogData::FDS_OVERWRITEPROMPT | FileDialogData::FDS_BROWSEFOLDER;

   mMustExistInDir = "";
}

IMPLEMENT_CONOBJECT(OpenFolderDialog);

ConsoleDocClass(OpenFolderDialog,
   "@brief OS level dialog used for browsing folder structures.\n\n"

   "This is essentially an OpenFileDialog, but only used for returning directory paths, not files.\n\n"

   "@note FileDialog and its related classes are only availble in a Tools build of Torque.\n\n"

   "@see OpenFileDialog for more details on functionality.\n\n"

   "@ingroup FileSystem\n"
   );

void OpenFolderDialog::initPersistFields()
{
   addField("fileMustExist", TypeFilename, Offset(mMustExistInDir, OpenFolderDialog), "File that must be in selected folder for it to be valid");

   Parent::initPersistFields();
}
#endif
