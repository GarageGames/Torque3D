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
#include "platformWin32/platformWin32.h"
#include "core/util/safeDelete.h"
#include "math/mMath.h"
#include "core/strings/unicode.h"
#include "console/consoleTypes.h"
#include "platform/profiler.h"
#include <ShlObj.h>
#include <WindowsX.h>
#include "console/engineAPI.h"

#ifdef TORQUE_TOOLS
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
   mDefaultPath = StringTable->insert( Con::getVariable("Tools::FileDialogs::LastFilePath") );
   if( mDefaultPath == StringTable->lookup("") || !Platform::isDirectory( mDefaultPath ) )
      mDefaultPath = Platform::getCurrentDirectory();

   mDefaultFile = StringTable->insert("");
   mFilters = StringTable->insert("");
   mFile = StringTable->insert("");
   mTitle = StringTable->insert("");

   mStyle = 0;

}
FileDialogData::~FileDialogData()
{

}

static LRESULT PASCAL OKBtnFolderHackProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   WNDPROC oldProc = (WNDPROC)GetProp(hWnd, dT("OldWndProc"));

   switch(uMsg)
   {
      case WM_COMMAND:
         if(LOWORD(wParam) == IDOK)
         {
            LPOPENFILENAME ofn = (LPOPENFILENAME)GetProp(hWnd, dT("OFN"));
            if(ofn == NULL)
               break;

            SendMessage(hWnd, CDM_GETFILEPATH, ofn->nMaxFile, (LPARAM)ofn->lpstrFile);

            char *filePath;
#ifdef UNICODE
            char fileBuf[MAX_PATH];
            convertUTF16toUTF8(ofn->lpstrFile, fileBuf, sizeof(fileBuf));
            filePath = fileBuf;
#else
            filePath = ofn->lpstrFile;
#endif

            if(Platform::isDirectory(filePath))
            {
               // Got a directory
               EndDialog(hWnd, IDOK);
            }
         }
         break;
   }

   if(oldProc)
      return CallWindowProc(oldProc, hWnd, uMsg, wParam, lParam);
   else
      return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static UINT_PTR CALLBACK FolderHookProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam){
   HWND hParent = GetParent(hdlg);
   
   switch(uMsg)
   {
      case WM_INITDIALOG:
         {
            LPOPENFILENAME lpofn = (LPOPENFILENAME)lParam;

            SendMessage(hParent, CDM_SETCONTROLTEXT, stc3, (LPARAM)dT("Folder name:"));
            SendMessage(hParent, CDM_HIDECONTROL, cmb1, 0);
            SendMessage(hParent, CDM_HIDECONTROL, stc2, 0);

            LONG oldProc = SetWindowLong(hParent, GWL_WNDPROC, (LONG)OKBtnFolderHackProc);
            SetProp(hParent, dT("OldWndProc"), (HANDLE)oldProc);
            SetProp(hParent, dT("OFN"), (HANDLE)lpofn);
         }
         break;

      case WM_NOTIFY:
         {
            LPNMHDR nmhdr = (LPNMHDR)lParam;
            switch(nmhdr->code)
            {
               case CDN_FOLDERCHANGE:
                  {
                     LPOFNOTIFY lpofn = (LPOFNOTIFY)lParam;

                     OpenFolderDialog *ofd = (OpenFolderDialog *)lpofn->lpOFN->lCustData;
                     
#ifdef UNICODE
                     UTF16 buf[MAX_PATH];
#else
                     char buf[MAX_PATH];
#endif

                     SendMessage(hParent, CDM_GETFOLDERPATH, sizeof(buf), (LPARAM)buf);

                     char filePath[MAX_PATH];
#ifdef UNICODE
                     convertUTF16toUTF8(buf, filePath, sizeof(filePath));
#else
                     dStrcpy( filePath, buf );
#endif

                     // [tom, 12/8/2006] Hack to remove files from the list because
                     // CDN_INCLUDEITEM doesn't work for regular files and folders.
                     HWND shellView = GetDlgItem(hParent, lst2);
                     HWND listView = FindWindowEx(shellView, 0, WC_LISTVIEW, NULL);
                     if(listView)
                     {
                        S32 count = ListView_GetItemCount(listView);
                        for(S32 i = count - 1;i >= 0;--i)
                        {
                           ListView_GetItemText(listView, i, 0, buf, sizeof(buf));
                           
#ifdef UNICODE
                           char buf2[MAX_PATH];
                           convertUTF16toUTF8(buf, buf2, sizeof(buf2));
#else
                           char *buf2 = buf;
#endif
                           char full[MAX_PATH];
                           dSprintf(full, sizeof(full), "%s\\%s", filePath, buf2);

                           if(!Platform::isDirectory(full))
                           {
                              ListView_DeleteItem(listView, i);
                           }
                        }
                     }

                     if(ofd->mMustExistInDir == NULL || *ofd->mMustExistInDir == 0)
                        break;

                     HWND hOK = GetDlgItem(hParent, IDOK);
                     if(hOK == NULL)
                        break;

                     char checkPath[MAX_PATH];
                     dSprintf(checkPath, sizeof(checkPath), "%s\\%s", filePath, ofd->mMustExistInDir);

                     EnableWindow(hOK, Platform::isFile(checkPath));
                  }
                  break;
            }
         }
         break;
   }
   return 0;
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
   // Default to File Must Exist Open Dialog style
   mData.mStyle = FileDialogData::FDS_OPEN | FileDialogData::FDS_MUSTEXIST;
   mChangePath = false;
}

FileDialog::~FileDialog()
{
}

void FileDialog::initPersistFields()
{
   addProtectedField( "defaultPath", TypeString, Offset(mData.mDefaultPath, FileDialog), &setDefaultPath, &defaultProtectedGetFn, 
      "The default directory path when the dialog is shown." );
      
   addProtectedField( "defaultFile", TypeString, Offset(mData.mDefaultFile, FileDialog), &setDefaultFile, &defaultProtectedGetFn, 
      "The default file path when the dialog is shown." );
            
   addProtectedField( "fileName", TypeString, Offset(mData.mFile, FileDialog), &setFile, &defaultProtectedGetFn, 
      "The default file name when the dialog is shown." );
      
   addProtectedField( "filters", TypeString, Offset(mData.mFilters, FileDialog), &setFilters, &defaultProtectedGetFn, 
      "The filter string for limiting the types of files visible in the dialog.  It makes use of the pipe symbol '|' "
      "as a delimiter.  For example:\n\n"
      "'All Files|*.*'\n\n"
      "'Image Files|*.png;*.jpg|Png Files|*.png|Jepg Files|*.jpg'" );
      
   addField( "title", TypeString, Offset(mData.mTitle, FileDialog), 
      "The title for the dialog." );
   
   addProtectedField( "changePath", TypeBool, Offset(mChangePath, FileDialog), &setChangePath, &getChangePath,
      "True/False whether to set the working directory to the directory returned by the dialog." );
   
   Parent::initPersistFields();
}

static const U32 convertUTF16toUTF8DoubleNULL( const UTF16 *unistring, UTF8  *outbuffer, U32 len)
{
   AssertFatal(len >= 1, "Buffer for unicode conversion must be large enough to hold at least the null terminator.");
   PROFILE_START(convertUTF16toUTF8DoubleNULL);
   U32 walked, nCodeunits, codeunitLen;
   UTF32 middleman;

   nCodeunits=0;
   while( ! (*unistring == '\0' && *(unistring + 1) == '\0') && nCodeunits + 3 < len )
   {
      walked = 1;
      middleman  = oneUTF16toUTF32(unistring,&walked);
      codeunitLen = oneUTF32toUTF8(middleman, &outbuffer[nCodeunits]);
      unistring += walked;
      nCodeunits += codeunitLen;
   }

   nCodeunits = getMin(nCodeunits,len - 1);
   outbuffer[nCodeunits] = '\0';
   outbuffer[nCodeunits+1] = '\0';

   PROFILE_END();
   return nCodeunits;
}

//
// Execute Method
//
bool FileDialog::Execute()
{
   static char pszResult[MAX_PATH];
#ifdef UNICODE
   UTF16 pszFile[MAX_PATH];
   UTF16 pszInitialDir[MAX_PATH];
   UTF16 pszTitle[MAX_PATH];
   UTF16 pszFilter[1024];
   UTF16 pszFileTitle[MAX_PATH];
   UTF16 pszDefaultExtension[MAX_PATH];
   // Convert parameters to UTF16*'s
   convertUTF8toUTF16((UTF8 *)mData.mDefaultFile, pszFile, sizeof(pszFile));
   convertUTF8toUTF16((UTF8 *)mData.mDefaultPath, pszInitialDir, sizeof(pszInitialDir));
   convertUTF8toUTF16((UTF8 *)mData.mTitle, pszTitle, sizeof(pszTitle));
   convertUTF8toUTF16((UTF8 *)mData.mFilters, pszFilter, sizeof(pszFilter) );
#else
   // Not Unicode, All char*'s!
   char pszFile[MAX_PATH];
   char pszFilter[1024];
   char pszFileTitle[MAX_PATH];
   dStrcpy( pszFile, mData.mDefaultFile );
   dStrcpy( pszFilter, mData.mFilters );
   const char* pszInitialDir = mData.mDefaultPath;
   const char* pszTitle = mData.mTitle;
   
#endif

   pszFileTitle[0] = 0;

   // Convert Filters
   U32 filterLen = dStrlen( pszFilter );
   S32 dotIndex = -1;
   for( U32 i = 0; i < filterLen; i++ )
   {
      if( pszFilter[i] == '|' )
         pszFilter[i] = '\0';

      if( pszFilter[ i ] == '.' && dotIndex == -1 )
         dotIndex = i;
   }
   // Add second NULL terminator at the end
   pszFilter[ filterLen + 1 ] = '\0';

   // Get default extension.
   dMemset( pszDefaultExtension, 0, sizeof( pszDefaultExtension ) );
   if( dotIndex != -1 )
   {
      for( U32 i = 0; i < MAX_PATH; ++ i )
      {
         UTF16 ch = pszFilter[ dotIndex + 1 + i ];
         if( !ch || ch == ';' || ch == '|' || dIsspace( ch ) )
            break;

         pszDefaultExtension[ i ] = ch;
      }
   }

   OPENFILENAME ofn;
   dMemset(&ofn, 0, sizeof(ofn));
   ofn.lStructSize      = sizeof(ofn);
   ofn.hwndOwner        = getWin32WindowHandle();
   ofn.lpstrFile        = pszFile;


   if( !dStrncmp( mData.mDefaultFile, "", 1 ) )
      ofn.lpstrFile[0]     = '\0';


   ofn.nMaxFile         = sizeof(pszFile);
   ofn.lpstrFilter      = pszFilter;
   ofn.nFilterIndex     = 1;
   ofn.lpstrInitialDir  = pszInitialDir;
   ofn.lCustData        = (LPARAM)this;
   ofn.lpstrFileTitle   = pszFileTitle;
   ofn.nMaxFileTitle    = sizeof(pszFileTitle);
   ofn.lpstrDefExt      = pszDefaultExtension[ 0 ] ? pszDefaultExtension : NULL;

   if( mData.mTitle != StringTable->lookup("") )
      ofn.lpstrTitle = pszTitle;

   // Build Proper Flags.
   ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_HIDEREADONLY;

   if(mData.mStyle & FileDialogData::FDS_BROWSEFOLDER)
   {
      ofn.lpfnHook = FolderHookProc;
      ofn.Flags |= OFN_ENABLEHOOK;
   }
   
   if( !(mData.mStyle & FileDialogData::FDS_CHANGEPATH) )
      ofn.Flags |= OFN_NOCHANGEDIR;

   if( mData.mStyle & FileDialogData::FDS_MUSTEXIST )
      ofn.Flags |= OFN_FILEMUSTEXIST;

   if( mData.mStyle & FileDialogData::FDS_OPEN && mData.mStyle & FileDialogData::FDS_MULTIPLEFILES )
      ofn.Flags |= OFN_ALLOWMULTISELECT;

   if( mData.mStyle & FileDialogData::FDS_OVERWRITEPROMPT )
      ofn.Flags |= OFN_OVERWRITEPROMPT;


   // Flag we're showing file browser so we can do some render hacking
   winState.renderThreadBlocked = true;

   // Get the current working directory, so we can back up to it once Windows has
   // done its craziness and messed with it.
   StringTableEntry cwd = Platform::getCurrentDirectory();

   // Execute Dialog (Blocking Call)
   bool dialogSuccess = false;
   if( mData.mStyle & FileDialogData::FDS_OPEN )
      dialogSuccess = GetOpenFileName(&ofn);
   else if( mData.mStyle & FileDialogData::FDS_SAVE )
      dialogSuccess = GetSaveFileName(&ofn);

   // Dialog is gone.
   winState.renderThreadBlocked = false;

   // Restore the working directory.
   Platform::setCurrentDirectory( cwd );

   // Did we select a file?
   if( !dialogSuccess )
      return false;

   // Handle Result Properly for Unicode as well as ANSI
#ifdef UNICODE
   if(pszFileTitle[0] || ! ( mData.mStyle & FileDialogData::FDS_OPEN && mData.mStyle & FileDialogData::FDS_MULTIPLEFILES ))
      convertUTF16toUTF8( (UTF16*)pszFile, (UTF8*)pszResult, sizeof(pszResult));
   else
      convertUTF16toUTF8DoubleNULL( (UTF16*)pszFile, (UTF8*)pszResult, sizeof(pszResult));
#else
   if(pszFileTitle[0] || ! ( mData.mStyle & FileDialogData::FDS_OPEN && mData.mStyle & FileDialogData::FDS_MULTIPLEFILES ))
      dStrcpy(pszResult,pszFile);
   else
   {
      // [tom, 1/4/2007] pszResult is a double-NULL terminated, NULL separated list in this case so we can't just dSstrcpy()
      char *sptr = pszFile, *dptr = pszResult;
      while(! (*sptr == 0 && *(sptr+1) == 0))
         *dptr++ = *sptr++;
      *dptr++ = 0;
   }
#endif

   forwardslash(pszResult);

   // [tom, 1/5/2007] Windows is ridiculously dumb. If you select a single file in a multiple
   // select file dialog then it will return the file the same way as it would in a single
   // select dialog. The only difference is pszFileTitle is empty if multiple files
   // are selected.

   // Store the result on our object
   if( mData.mStyle & FileDialogData::FDS_BROWSEFOLDER || ( pszFileTitle[0] && ! ( mData.mStyle & FileDialogData::FDS_OPEN && mData.mStyle & FileDialogData::FDS_MULTIPLEFILES )))
   {
      // Single file selection, do it the easy way
      mData.mFile = StringTable->insert( pszResult );
   }
   else if(pszFileTitle[0] && ( mData.mStyle & FileDialogData::FDS_OPEN && mData.mStyle & FileDialogData::FDS_MULTIPLEFILES ))
   {
      // Single file selection in a multiple file selection dialog
      setDataField(StringTable->insert("files"), "0", pszResult);
      setDataField(StringTable->insert("fileCount"), NULL, "1");
   }
   else
   {
      // Multiple file selection, break out into an array
      S32 numFiles = 0;
      const char *dir = pszResult;
      const char *file = dir + dStrlen(dir) + 1;
      char buffer[1024];

      while(*file)
      {
         Platform::makeFullPathName(file, buffer, sizeof(buffer), dir);
         setDataField(StringTable->insert("files"), Con::getIntArg(numFiles++), buffer);

         file = file + dStrlen(file) + 1;
      }

      setDataField(StringTable->insert("fileCount"), NULL, Con::getIntArg(numFiles));
   }

   // Return success.
   return true;

}
DefineEngineMethod( FileDialog, Execute, bool, (),,
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
bool FileDialog::setFilters( void *object, const char *index, const char *data )
{
   // Will do validate on write at some point.
   if( !data )
      return true;

   return true;

};


//-----------------------------------------------------------------------------
// Default Path Property - String Validated on Write
//-----------------------------------------------------------------------------
bool FileDialog::setDefaultPath( void *object, const char *index, const char *data )
{

   if( !data || !dStrncmp( data, "", 1 ) )
      return true;

   // Copy and Backslash the path (Windows dialogs are VERY picky about this format)
   static char szPathValidate[512];
   dStrcpy( szPathValidate, data );

   Platform::makeFullPathName( data,szPathValidate, sizeof(szPathValidate));
   backslash( szPathValidate );

   // Remove any trailing \'s
   S8 validateLen = dStrlen( szPathValidate );
   if( szPathValidate[ validateLen - 1 ] == '\\' )
      szPathValidate[ validateLen - 1 ] = '\0';

   // Now check 
   if( Platform::isDirectory( szPathValidate ) )
   {
      // Finally, assign in proper format.
      FileDialog *pDlg = static_cast<FileDialog*>( object );
      pDlg->mData.mDefaultPath = StringTable->insert( szPathValidate );
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
bool FileDialog::setDefaultFile( void *object, const char *index, const char *data )
{
   if( !data || !dStrncmp( data, "", 1 ) )
      return true;

   // Copy and Backslash the path (Windows dialogs are VERY picky about this format)
   static char szPathValidate[512];
   Platform::makeFullPathName( data,szPathValidate, sizeof(szPathValidate) );
   backslash( szPathValidate );

   // Remove any trailing \'s
   S8 validateLen = dStrlen( szPathValidate );
   if( szPathValidate[ validateLen - 1 ] == '\\' )
      szPathValidate[ validateLen - 1 ] = '\0';

   // Finally, assign in proper format.
   FileDialog *pDlg = static_cast<FileDialog*>( object );
   pDlg->mData.mDefaultFile = StringTable->insert( szPathValidate );

   return false;
};

//-----------------------------------------------------------------------------
// ChangePath Property - Change working path on successful file selection
//-----------------------------------------------------------------------------
bool FileDialog::setChangePath( void *object, const char *index, const char *data )
{
   bool bMustExist = dAtob( data );

   FileDialog *pDlg = static_cast<FileDialog*>( object );

   if( bMustExist )
      pDlg->mData.mStyle |= FileDialogData::FDS_CHANGEPATH;
   else
      pDlg->mData.mStyle &= ~FileDialogData::FDS_CHANGEPATH;

   return true;
};

const char* FileDialog::getChangePath(void* obj, const char* data)
{
   FileDialog *pDlg = static_cast<FileDialog*>( obj );
   if( pDlg->mData.mStyle & FileDialogData::FDS_CHANGEPATH )
      return StringTable->insert("true");
   else
      return StringTable->insert("false");
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
   addProtectedField("MustExist", TypeBool, Offset(mMustExist, OpenFileDialog), &setMustExist, &getMustExist, "True/False whether the file returned must exist or not" );
   addProtectedField("MultipleFiles", TypeBool, Offset(mMultipleFiles, OpenFileDialog), &setMultipleFiles, &getMultipleFiles, "True/False whether multiple files may be selected and returned or not" );
   
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------
// File Must Exist - Boolean
//-----------------------------------------------------------------------------
bool OpenFileDialog::setMustExist( void *object, const char *index, const char *data )
{
   bool bMustExist = dAtob( data );

   OpenFileDialog *pDlg = static_cast<OpenFileDialog*>( object );
   
   if( bMustExist )
      pDlg->mData.mStyle |= FileDialogData::FDS_MUSTEXIST;
   else
      pDlg->mData.mStyle &= ~FileDialogData::FDS_MUSTEXIST;

   return true;
};

const char* OpenFileDialog::getMustExist(void* obj, const char* data)
{
   OpenFileDialog *pDlg = static_cast<OpenFileDialog*>( obj );
   if( pDlg->mData.mStyle & FileDialogData::FDS_MUSTEXIST )
      return StringTable->insert("true");
   else
      return StringTable->insert("false");
}

//-----------------------------------------------------------------------------
// Can Select Multiple Files - Boolean
//-----------------------------------------------------------------------------
bool OpenFileDialog::setMultipleFiles( void *object, const char *index, const char *data )
{
   bool bMustExist = dAtob( data );

   OpenFileDialog *pDlg = static_cast<OpenFileDialog*>( object );

   if( bMustExist )
      pDlg->mData.mStyle |= FileDialogData::FDS_MULTIPLEFILES;
   else
      pDlg->mData.mStyle &= ~FileDialogData::FDS_MULTIPLEFILES;

   return true;
};

const char* OpenFileDialog::getMultipleFiles(void* obj, const char* data)
{
   OpenFileDialog *pDlg = static_cast<OpenFileDialog*>( obj );
   if( pDlg->mData.mStyle & FileDialogData::FDS_MULTIPLEFILES )
      return StringTable->insert("true");
   else
      return StringTable->insert("false");
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
   addProtectedField("OverwritePrompt", TypeBool, Offset(mOverwritePrompt, SaveFileDialog), &setOverwritePrompt, &getOverwritePrompt, "True/False whether the dialog should prompt before accepting an existing file name" );
   
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------
// Prompt on Overwrite - Boolean
//-----------------------------------------------------------------------------
bool SaveFileDialog::setOverwritePrompt( void *object, const char *index, const char *data )
{
   bool bMustExist = dAtob( data );

   SaveFileDialog *pDlg = static_cast<SaveFileDialog*>( object );

   if( bMustExist )
      pDlg->mData.mStyle |= FileDialogData::FDS_OVERWRITEPROMPT;
   else
      pDlg->mData.mStyle &= ~FileDialogData::FDS_OVERWRITEPROMPT;

   return true;
};

const char* SaveFileDialog::getOverwritePrompt(void* obj, const char* data)
{
   SaveFileDialog *pDlg = static_cast<SaveFileDialog*>( obj );
   if( pDlg->mData.mStyle & FileDialogData::FDS_OVERWRITEPROMPT )
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

ConsoleDocClass( OpenFolderDialog,
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