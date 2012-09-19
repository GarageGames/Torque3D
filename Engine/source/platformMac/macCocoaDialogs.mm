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

// Get our GL header included before Apple's
#include "platformMac/platformMacCarb.h"
// Don't include Apple's
#define __gl_h_

#include "platform/tmm_off.h"
#include <Cocoa/Cocoa.h>
#include "platform/tmm_on.h"

#include "console/simBase.h"
#include "platform/nativeDialogs/fileDialog.h"
#include "platform/threads/mutex.h"
#include "core/util/safeDelete.h"
#include "math/mMath.h"
#include "core/strings/unicode.h"
#include "console/consoleTypes.h"
#include "platform/threads/thread.h"
#include "platform/threads/semaphore.h"

class FileDialogOpaqueData
{
public:
   Semaphore *sem;
   FileDialogOpaqueData() { sem = new Semaphore(0);  }
   ~FileDialogOpaqueData() { delete sem; }
};

class FileDialogFileExtList
{
public:
   Vector<UTF8*> list;
   UTF8* data;
   
   FileDialogFileExtList(const char* exts) { data = dStrdup(exts); }
   ~FileDialogFileExtList() { SAFE_DELETE(data); }
};

class FileDialogFileTypeList
{
public:
   UTF8* filterData;
   Vector<UTF8*> names;
   Vector<FileDialogFileExtList*> exts;
   bool any;
   
   FileDialogFileTypeList(const char* filter) { filterData = dStrdup(filter); any = false;}
   ~FileDialogFileTypeList()
   { 
      SAFE_DELETE(filterData);
      for(U32 i = 0; i < exts.size(); i++)
         delete exts[i];
   }
};

#undef new

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
   
   mOpaqueData = new FileDialogOpaqueData();

}
FileDialogData::~FileDialogData()
{
   delete mOpaqueData;
}

//-----------------------------------------------------------------------------
// FileDialog Implementation
//-----------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(FileDialog);

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
   // why is this stuff buried in another class?
   addProtectedField("DefaultPath", TypeString, Offset(mData.mDefaultPath, FileDialog), &setDefaultPath, &defaultProtectedGetFn, "Default Path when Dialog is shown");
   addProtectedField("DefaultFile", TypeString, Offset(mData.mDefaultFile, FileDialog), &setDefaultFile, &defaultProtectedGetFn, "Default File when Dialog is shown");
   addProtectedField("FileName", TypeString, Offset(mData.mFile, FileDialog), &setFile, &defaultProtectedGetFn, "Default File when Dialog is shown");
   addProtectedField("Filters", TypeString, Offset(mData.mFilters, FileDialog), &setFilters, &defaultProtectedGetFn, "Default File when Dialog is shown");
   addField("Title", TypeString, Offset(mData.mTitle, FileDialog), "Default File when Dialog is shown");
   addProtectedField("ChangePath", TypeBool, Offset(mChangePath, FileDialog), &setChangePath, &getChangePath, "True/False whether to set the working directory to the directory returned by the dialog" );
   Parent::initPersistFields();
}


static FileDialogFileExtList* _MacCarbGetFileExtensionsFromString(const char* filter)
{
   FileDialogFileExtList* list = new FileDialogFileExtList(filter);
   
   char* token = list->data;
   char* place = list->data;
   
   for( ; *place; place++)
   {
      if(*place != ';')
         continue;
      
      *place = '\0';
      
      list->list.push_back(token);
      
      ++place;
      token = place;
   }
   // last token   
   list->list.push_back(token);
   
   return list;
   
}

static FileDialogFileTypeList* _MacCarbGetFileTypesFromString(const char* filter)
{
   FileDialogFileTypeList &list = *(new FileDialogFileTypeList(filter));

   char* token = list.filterData;
   char* place = list.filterData;
   
   // scan the filter list until we hit a null.
   // when we see the separator '|', replace it with a null, and save the token
   // format is description|extension|description|extension
   bool isDesc = true;
   for( ; *place; place++)
   {
      if(*place != '|')
         continue;
      
      *place = '\0';

      if(isDesc)
         list.names.push_back(token);
      else
      {
         // detect *.*
         if(dStrstr((const char*)token, "*.*"))
            list.any = true;
         
         list.exts.push_back(_MacCarbGetFileExtensionsFromString(token));
      }
   
      
      isDesc = !isDesc;
      ++place;
      token = place;
   }
   list.exts.push_back(_MacCarbGetFileExtensionsFromString(token));
   
   return &list;
}

static NSArray* _MacCocoaCreateAndRunSavePanel(FileDialogData &mData)
{
   NSSavePanel* panel = [NSSavePanel savePanel];

   // User freedom niceties
   [panel setCanCreateDirectories:YES];
   [panel setCanSelectHiddenExtension:YES];
   [panel setTreatsFilePackagesAsDirectories:YES];

   NSString *initialFile = [[NSString stringWithUTF8String:mData.mDefaultFile] lastPathComponent];

   // we only use mDefaultDir if mDefault path is not set.
   NSString *dir;
   if(dStrlen(mData.mDefaultPath) < 1)
      dir = [[NSString stringWithUTF8String:mData.mDefaultFile] stringByDeletingLastPathComponent];
   else
      dir = [NSString stringWithUTF8String: mData.mDefaultPath];
   [panel setDirectory:dir];
   
   // todo: move file type handling to an accessory view.
   // parse file types
   FileDialogFileTypeList *fTypes = _MacCarbGetFileTypesFromString(mData.mFilters);
   
   // fill an array with the possible file types
   NSMutableArray* types = [NSMutableArray arrayWithCapacity:10];
   for(U32 i = 0; i < fTypes->exts.size(); i++)
   {
      for(U32 j = 0; j < fTypes->exts[i]->list.size(); j++)
      {
         char* ext = fTypes->exts[i]->list[j];
         if(ext)
         {
         	if(dStrlen(ext) == 0)
            	continue;
            if(dStrncmp(ext, "*.*", 3) == 0)
            	continue;
            if(dStrncmp(ext, "*.", 2) == 0)
               ext+=2;
               
            [types addObject:[NSString stringWithUTF8String:ext]];
         }
      }
   }
   if([types count] > 0)
      [panel setAllowedFileTypes:types];

    // if any file type was *.*, user may select any file type.
   [panel setAllowsOtherFileTypes:fTypes->any];

   
   //---------------------------------------------------------------------------
   // Display the panel, enter a modal loop. This blocks.
   //---------------------------------------------------------------------------   
   U32 button = [panel runModalForDirectory:dir file:initialFile];
  
   // return the file name
   NSMutableArray *array = [NSMutableArray arrayWithCapacity:10];
   if(button != NSFileHandlingPanelCancelButton)
      [array addObject:[panel filename]];
      
   delete fTypes;

   return array;
   
   // TODO: paxorr: show as sheet
   // crashes when we try to display the window as a sheet. Not sure why.
   // the sheet is instantly dismissed, and crashes as it's dismissing itself.
   // here's the code snippet to get an nswindow from our carbon WindowRef
   //NSWindow *nsAppWindow = [[NSWindow alloc] initWithWindowRef:platState.appWindow];
}

NSArray* _MacCocoaCreateAndRunOpenPanel(FileDialogData &mData)
{
   NSOpenPanel* panel = [NSOpenPanel openPanel];

   // User freedom niceties
   [panel setCanCreateDirectories:YES];
   [panel setCanSelectHiddenExtension:YES];
   [panel setTreatsFilePackagesAsDirectories:YES];

   [panel setAllowsMultipleSelection:(mData.mStyle & FileDialogData::FDS_MULTIPLEFILES)];

   // 
   bool chooseDir = (mData.mStyle & FileDialogData::FDS_BROWSEFOLDER);
   [panel setCanChooseFiles: !chooseDir ];
   [panel setCanChooseDirectories: chooseDir ];
   if(chooseDir)
   {
      [panel setPrompt:@"Choose"];
      [panel setTitle:@"Choose Folder"];
   }

   NSString *initialFile = [[NSString stringWithUTF8String:mData.mDefaultFile] lastPathComponent];

   // we only use mDefaultDir if mDefault path is not set.
   NSString *dir;
   if(dStrlen(mData.mDefaultPath) < 1)
      dir = [[NSString stringWithUTF8String:mData.mDefaultFile] stringByDeletingLastPathComponent];
   else
      dir = [NSString stringWithUTF8String: mData.mDefaultPath];
   [panel setDirectory:dir];
   
   // todo: move file type handling to an accessory view.
   // parse file types
   FileDialogFileTypeList *fTypes = _MacCarbGetFileTypesFromString(mData.mFilters);
   
   // fill an array with the possible file types
   NSMutableArray* types = [NSMutableArray arrayWithCapacity:10];
   for(U32 i = 0; i < fTypes->exts.size(); i++)
   {
      for(U32 j = 0; j < fTypes->exts[i]->list.size(); j++)
      {
         char* ext = fTypes->exts[i]->list[j];
         if(ext)
         {
            if(dStrncmp(ext, "*.", 2) == 0)
               ext+=2;
               
            [types addObject:[NSString stringWithUTF8String:ext]];
         }
      }
   }
   if([types count] > 0)
      [panel setAllowedFileTypes:types];

    // if any file type was *.*, user may select any file type.
   [panel setAllowsOtherFileTypes:fTypes->any];

   
   //---------------------------------------------------------------------------
   // Display the panel, enter a modal loop. This blocks.
   //---------------------------------------------------------------------------   
   U32 button = [panel runModalForDirectory:dir file:initialFile ];
  
   // return the file name
   NSMutableArray *array = [NSMutableArray arrayWithCapacity:10];
   if(button != NSFileHandlingPanelCancelButton)
      [array addObject:[panel filename]];
      
   delete fTypes;
   
   return array;
}

void MacCarbShowDialog(void* dialog)
{
   FileDialog* d = static_cast<FileDialog*>(dialog);
   d->Execute();
}
//
// Execute Method
//
bool FileDialog::Execute()
{
//   if(! ThreadManager::isCurrentThread(platState.firstThreadId))
//   {
//      MacCarbSendTorqueEventToMain(kEventTorqueModalDialog,this);
//      mData.mOpaqueData->sem->acquire();
//      return;
//   }
   
   NSArray* nsFileArray;
   if(mData.mStyle & FileDialogData::FDS_OPEN)
      nsFileArray = _MacCocoaCreateAndRunOpenPanel(mData);
   else if(mData.mStyle & FileDialogData::FDS_SAVE)
      nsFileArray = _MacCocoaCreateAndRunSavePanel(mData);
   else
   {
      Con::errorf("Bad File Dialog Setup.");
      mData.mOpaqueData->sem->release();
      return false;
   }
   
   if([nsFileArray count] == 0)
      return false;
   
   if(! (mData.mStyle & FileDialogData::FDS_MULTIPLEFILES) && [nsFileArray count] >= 1)
   {
      const UTF8* f = [(NSString*)[nsFileArray objectAtIndex:0] UTF8String];
      mData.mFile = StringTable->insert(f);
   }
   else
   {
      for(U32 i = 0; i < [nsFileArray count]; i++)
      {
         const UTF8* f = [(NSString*)[nsFileArray objectAtIndex:i] UTF8String];
         setDataField(StringTable->insert("files"), Con::getIntArg(i), StringTable->insert(f));
      }
      setDataField(StringTable->insert("fileCount"), NULL, Con::getIntArg([nsFileArray count]));
   }
   mData.mOpaqueData->sem->release();
   
   
   return true;

}

ConsoleMethod( FileDialog, Execute, bool, 2, 2, "%fileDialog.Execute();" )
{
   return object->Execute();
}

//-----------------------------------------------------------------------------
// Dialog Filters
//-----------------------------------------------------------------------------
bool FileDialog::setFilters(void* obj, const char* index, const char* data)
{
   // Will do validate on write at some point.
   if( !data )
      return true;

   return true;

};


//-----------------------------------------------------------------------------
// Default Path Property - String Validated on Write
//-----------------------------------------------------------------------------
bool FileDialog::setDefaultPath(void* obj, const char* index, const char* data)
{

   if( !data )
      return true;

   return true;

};

//-----------------------------------------------------------------------------
// Default File Property - String Validated on Write
//-----------------------------------------------------------------------------
bool FileDialog::setDefaultFile(void* obj, const char* index, const char* data)
{
   if( !data )
      return true;

   return true;
};

//-----------------------------------------------------------------------------
// ChangePath Property - Change working path on successful file selection
//-----------------------------------------------------------------------------
bool FileDialog::setChangePath(void* obj, const char* index, const char* data)
{
   bool bChangePath = dAtob( data );

   FileDialog *pDlg = static_cast<FileDialog*>( obj );

   if( bChangePath )
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

bool FileDialog::setFile(void* obj, const char* index, const char* data)
{
   return false;
};

//-----------------------------------------------------------------------------
// OpenFileDialog Implementation
//-----------------------------------------------------------------------------
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
bool OpenFileDialog::setMustExist(void* obj, const char* index, const char* data)
{
   bool bMustExist = dAtob( data );

   OpenFileDialog *pDlg = static_cast<OpenFileDialog*>( obj );
   
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
bool OpenFileDialog::setMultipleFiles(void* obj, const char* index, const char* data)
{
   bool bMustExist = dAtob( data );

   OpenFileDialog *pDlg = static_cast<OpenFileDialog*>( obj );

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
bool SaveFileDialog::setOverwritePrompt(void* obj, const char* index, const char* data)
{
   bool bOverwrite = dAtob( data );

   SaveFileDialog *pDlg = static_cast<SaveFileDialog*>( obj );

   if( bOverwrite )
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

void OpenFolderDialog::initPersistFields()
{
   addField("fileMustExist", TypeFilename, Offset(mMustExistInDir, OpenFolderDialog), "File that must in selected folder for it to be valid");

   Parent::initPersistFields();
}
