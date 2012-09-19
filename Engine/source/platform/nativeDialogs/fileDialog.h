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
#ifndef _FILEDIALOG_H_
#define _FILEDIALOG_H_
#include "console/simBase.h"

// [03/14/07] The file dialogs need refactoring, and will be refactored in Jugg.
// Things that might need to change:
// - The interface is not in fact platform agnotsic, it is win32 oriented.
// - Filter format is highly windows specific, and is a little fragile, both for
//   win32 and for other platforms.
// - Platform specific path strings are exposed to the console, because the 
//   protected validators save them as such.
// - Several of the FDS_XXX values are not options we want to give the user, such
//   as NOT warning on file overwrite. The values FDS_OVERWRITEPROMPT,
//   FDS_MUSTEXIST, and FDS_CHANGEPATH are not good things to give the user.
// - The Execute method is virtual for good reason. It should be implemented for
//   each subclass. If common behavior is needed for Execute(), it can be 
//   factored out in hidden platform specific code.


/// @defgroup SystemDialogs Using System Dialogs

/// @ingroup SystemDialogs
/// FileDialogOpaqueData is both defined and implemented on a platform specific
///   basis. 
class FileDialogOpaqueData;

/// @ingroup SystemDialogs 
/// @internal
/// Platform Agnostic Structure for holding information about a file dialog.
struct FileDialogData
{

public:
   FileDialogData();
   ~FileDialogData();

   enum DialogStyle
   {
      FDS_OPEN             = BIT(0),///< This is an open dialog.
      FDS_SAVE             = BIT(1),///< This is a save dialog.
      FDS_OVERWRITEPROMPT  = BIT(2),///< Can only be used in conjunction with style SaveDialog: prompt for a confirmation if a file will be overwritten.
      FDS_MUSTEXIST        = BIT(3),///< The user may only select files that actually exist.
      FDS_MULTIPLEFILES    = BIT(4),///< Can only be used in conjunction with style OpenDialog: allows selecting multiple files.
      FDS_CHANGEPATH       = BIT(5),///< Change the current working path to the directory where the file(s) chosen by the user are.
      FDS_BROWSEFOLDER     = BIT(6) ///< Select folders instead of files
   };
   U8 mStyle;      ///< Specifies the Style of the File Dialog @see DialogStyle

   StringTableEntry mFilters;             ///< List of Filters pipe separated e.g. "BMP Files (*.bmp)|*.bmp|JPG Files (*.jpg)|*.jpg"
   //StringTableEntry mFiles;             // this is never used   ///< Should only be referenced when using dialogStyle OpenDialog AND MultipleFiles: List of Files returned pipe separated
   StringTableEntry mFile;                ///< Should be referenced when dialogStyle MultipleFiles is NOT used: the file path of the user selected file.
   StringTableEntry mDefaultPath;         ///< Default path of dialog
   StringTableEntry mDefaultFile;         ///< Default selected file of dialog
   StringTableEntry mTitle;               ///< Title to display in file dialog

   FileDialogOpaqueData *mOpaqueData;     ///< Stores platform specific info about the dialog
   
};

/// @ingroup SystemDialogs
/// FileDialog is a platform agnostic dialog interface for querying the user for
///              file locations. It is designed to be used through the exposed
///              scripting interface.
///
/// FileDialog is the base class for Native File Dialog controls in Torque. It provides these
/// basic areas of functionality:
///
///      - Inherits from SimObject and is exposed to the scripting interface
///      - Provides blocking interface to allow instant return to script execution
///      - Simple object configuration makes practical use easy and effective
///
/// @attention
/// FileDialog is *NOT* intended to be used directly in script and is only exposed to script
/// to expose generic file dialog attributes. 
/// @see OpenFileDialog for a practical example on opening a file
/// @see SaveFileDialog for a practical example of saving a file
///
///
/// @{
class FileDialog : public SimObject
{
   typedef SimObject Parent;

protected:
   FileDialogData mData; ///< Stores platform agnostic information about the dialogs properties
   bool mChangePath; ///< Exposed ChangePath Property
   bool mBoolTranslator; ///< Internally used to translate boolean values into their respective bits of dialog style
public:

   FileDialog();
   virtual ~FileDialog();
   DECLARE_CONOBJECT(FileDialog);

   static void initPersistFields();

   virtual bool Execute();

   FileDialogData &getData() { return mData; };
protected:
   /// @name FileDialog Properties
   /// @{
   /// @@property DefaultPath (String) : <i>Path to use as the default when the dialog is shown.</i>
   /// @code %fd.DefaultPath = "/source/myGameProject/data/images"; @endcode
   ///
   /// @li @b ChangePath (bool) : <c>Will change the working path of the tools to the selected path when not canceled</c>
   /// @code %fd.ChangePath = true; // Change Working Path on Success @endcode
   /// @internal
   static bool setDefaultPath( void *object, const char *index, const char *data );
   static bool setDefaultFile( void *object, const char *index, const char *data );
   static bool setFilters( void *object, const char *index, const char *data );
   static bool setChangePath( void *object, const char *index, const char *data );
   static const char* getChangePath(void* obj, const char* data);
   ///
   /// @}

   static bool setFile( void *object, const char *index, const char *data );
};
/// @}

class OpenFileDialog : public FileDialog
{
   typedef FileDialog Parent;

   /// Field Values
   /// @{
   /// @internal
   bool mMustExist; ///< Corresponds to FDS_MUSTEXIST flag on the PlatformFileDlgData structure
   bool mMultipleFiles; ///< Corresponds to the FDS_MULTIPLEFILES flag on the PlatformFileDlgData structure
   /// @}

public:
   
   OpenFileDialog();
   virtual ~OpenFileDialog();

   DECLARE_CONOBJECT(OpenFileDialog); /// @internal

   static void initPersistFields();

protected:
   ///
   /// @}

   /// Must Exist Property
   static bool setMustExist( void *object, const char *index, const char *data );
   static const char*getMustExist(void* obj, const char* data);

   /// Multiple Files Property
   static bool setMultipleFiles( void *object, const char *index, const char *data );
   static const char* getMultipleFiles(void* obj, const char* data);
};

class OpenFolderDialog : public OpenFileDialog
{
   typedef OpenFileDialog Parent;

public:
   StringTableEntry mMustExistInDir;

   OpenFolderDialog();
   DECLARE_CONOBJECT(OpenFolderDialog);

   static void initPersistFields();
};

class SaveFileDialog : public FileDialog
{
   typedef FileDialog Parent;

public:

   SaveFileDialog();
   virtual ~SaveFileDialog();
   DECLARE_CONOBJECT(SaveFileDialog);
   
   bool mOverwritePrompt;

   static void initPersistFields();

protected:
   // Overwrite Prompt Property
   static bool setOverwritePrompt( void *object, const char *index, const char *data );
   static const char* getOverwritePrompt(void* obj, const char* data);

};

#endif // _FILEDIALOG_H_
