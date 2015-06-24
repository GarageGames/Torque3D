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

#include "platform/nativeDialogs/fileDialog.h"

#ifdef TORQUE_TOOLS
//-----------------------------------------------------------------------------
// PlatformFileDlgData Implementation
//-----------------------------------------------------------------------------
FileDialogData::FileDialogData()
{
    AssertFatal(0, "Not Implemented");
}

FileDialogData::~FileDialogData()
{
}

//-----------------------------------------------------------------------------
// FileDialog Implementation
//-----------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(FileDialog);

ConsoleDocClass( FileDialog,
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
    AssertFatal(0, "Not Implemented");
}

FileDialog::~FileDialog()
{
}

void FileDialog::initPersistFields()
{
    Parent::initPersistFields();
}

//
// Execute Method
//
bool FileDialog::Execute()
{
    return false;
}

//-----------------------------------------------------------------------------
// Dialog Filters
//-----------------------------------------------------------------------------
bool FileDialog::setFilters( void *object, const char *index, const char *data )
{
   return true;
};


//-----------------------------------------------------------------------------
// Default Path Property - String Validated on Write
//-----------------------------------------------------------------------------
bool FileDialog::setDefaultPath( void *object, const char *index, const char *data )
{
   return false;
};

//-----------------------------------------------------------------------------
// Default File Property - String Validated on Write
//-----------------------------------------------------------------------------
bool FileDialog::setDefaultFile( void *object, const char *index, const char *data )
{
    return false;
};

//-----------------------------------------------------------------------------
// ChangePath Property - Change working path on successful file selection
//-----------------------------------------------------------------------------
bool FileDialog::setChangePath( void *object, const char *index, const char *data )
{
    return true;
};

const char* FileDialog::getChangePath(void* obj, const char* data)
{
    return 0;
}

bool FileDialog::setFile( void *object, const char *index, const char *data )
{
    return false;
};

//-----------------------------------------------------------------------------
// OpenFileDialog Implementation
//-----------------------------------------------------------------------------

ConsoleDocClass( OpenFileDialog,
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
    AssertFatal(0, "Not Implemented");
}

OpenFileDialog::~OpenFileDialog()
{
}

IMPLEMENT_CONOBJECT(OpenFileDialog);

//-----------------------------------------------------------------------------
// Console Properties
//-----------------------------------------------------------------------------
void OpenFileDialog::initPersistFields()
{
}

//-----------------------------------------------------------------------------
// File Must Exist - Boolean
//-----------------------------------------------------------------------------
bool OpenFileDialog::setMustExist( void *object, const char *index, const char *data )
{
    return true;
};

const char* OpenFileDialog::getMustExist(void* obj, const char* data)
{
    return 0;
}

//-----------------------------------------------------------------------------
// Can Select Multiple Files - Boolean
//-----------------------------------------------------------------------------
bool OpenFileDialog::setMultipleFiles( void *object, const char *index, const char *data )
{
   return true;
};

const char* OpenFileDialog::getMultipleFiles(void* obj, const char* data)
{
    return 0;
}

//-----------------------------------------------------------------------------
// SaveFileDialog Implementation
//-----------------------------------------------------------------------------
ConsoleDocClass( SaveFileDialog,
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
    AssertFatal(0, "Not Implemented");
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
    Parent::initPersistFields();
}

//-----------------------------------------------------------------------------
// Prompt on Overwrite - Boolean
//-----------------------------------------------------------------------------
bool SaveFileDialog::setOverwritePrompt( void *object, const char *index, const char *data )
{
    return true;
};

const char* SaveFileDialog::getOverwritePrompt(void* obj, const char* data)
{
    return 0;
}

//-----------------------------------------------------------------------------
// OpenFolderDialog Implementation
//-----------------------------------------------------------------------------

OpenFolderDialog::OpenFolderDialog()
{    
    AssertFatal(0, "Not Implemented");
}

IMPLEMENT_CONOBJECT(OpenFolderDialog);

ConsoleDocClass( OpenFolderDialog,
   "@brief OS level dialog used for browsing folder structures.\n\n"

   "This is essentially an OpenFileDialog, but only used for returning directory paths, not files.\n\n"

   "@note FileDialog and its related classes are only availble in a Tools build of Torque.\n\n"

   "@see OpenFileDialog for more details on functionality.\n\n"

   "@ingroup FileSystem\n"
);

void OpenFolderDialog::initPersistFields()
{
    Parent::initPersistFields();
}

#endif