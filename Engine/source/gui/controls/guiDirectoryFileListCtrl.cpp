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

#include "console/engineAPI.h"
#include "core/strings/findMatch.h"
#include "gui/controls/guiDirectoryFileListCtrl.h"


IMPLEMENT_CONOBJECT( GuiDirectoryFileListCtrl );

ConsoleDocClass( GuiDirectoryFileListCtrl,
   "@brief A control that displays a list of files from within a single directory "
   "in the game file system.\n\n"

   "@tsexample\n\n"
   "new GuiDirectoryFileListCtrl()\n"
   "{\n"
   "   filePath = \"art/shapes\";\n"
   "   fileFilter = \"*.dts\" TAB \"*.dae\";\n"
   "   //Properties not specific to this control have been omitted from this example.\n"
   "};\n"
   "@endtsexample\n\n"

   "@ingroup GuiControls\n"
);

GuiDirectoryFileListCtrl::GuiDirectoryFileListCtrl()
{
   mFilePath = StringTable->insert( "" );
   mFilter = StringTable->insert( "*.*" );
}

void GuiDirectoryFileListCtrl::initPersistFields()
{
   addProtectedField( "filePath", TypeString, Offset( mFilePath, GuiDirectoryFileListCtrl ),
                      &_setFilePath, &defaultProtectedGetFn, "Path in game directory from which to list files." );
   addProtectedField( "fileFilter", TypeString, Offset( mFilter, GuiDirectoryFileListCtrl ),
                      &_setFilter, &defaultProtectedGetFn, "Tab-delimited list of file name patterns. Only matched files will be displayed." );
                      
   Parent::initPersistFields();
}

bool GuiDirectoryFileListCtrl::onWake()
{
   if( !Parent::onWake() )
      return false;
      
   update();
   
   return true;
}

void GuiDirectoryFileListCtrl::onMouseDown(const GuiEvent &event)
{
   Parent::onMouseDown( event );

   if( event.mouseClickCount == 2 )
      onDoubleClick_callback();
}


void GuiDirectoryFileListCtrl::openDirectory()
{
   String path;
   if( mFilePath && mFilePath[ 0 ] )
      path = String::ToString( "%s/%s", Platform::getMainDotCsDir(), mFilePath );
   else
      path = Platform::getMainDotCsDir();
   
   Vector<Platform::FileInfo> fileVector;
   Platform::dumpPath( path, fileVector, 0 );

   // Clear the current file listing
   clearItems();

   // Does this dir have any files?
   if( fileVector.empty() )
      return;

   // If so, iterate through and list them
   Vector<Platform::FileInfo>::iterator i = fileVector.begin();
   for( S32 j=0 ; i != fileVector.end(); i++, j++ )
   {
      if( !mFilter[ 0 ] || FindMatch::isMatchMultipleExprs( mFilter, (*i).pFileName,false ) )
         addItem( (*i).pFileName );
   }
}


void GuiDirectoryFileListCtrl::setCurrentFilter( const char* filter )
{
   if( !filter )
      filter = "";

   mFilter = StringTable->insert( filter );

   // Update our view
   openDirectory();
}

DefineEngineMethod( GuiDirectoryFileListCtrl, setFilter, void, ( const char* filter ),,
   "Set the file filter.\n\n"
   "@param filter Tab-delimited list of file name patterns. Only matched files will be displayed.\n" )
{
   object->setCurrentFilter( filter );
}

bool GuiDirectoryFileListCtrl::setCurrentPath( const char* path, const char* filter )
{
   if( !path )
      return false;

   const U32 pathLen = dStrlen( path );
   if( pathLen > 0 && path[ pathLen - 1 ] == '/' )
      mFilePath = StringTable->insertn( path, pathLen - 1 );
   else
      mFilePath = StringTable->insert( path );

   if( filter )
      mFilter  = StringTable->insert( filter );

   // Update our view
   openDirectory();

   return true;
}

DefineEngineMethod( GuiDirectoryFileListCtrl, reload, void, (),,
   "Update the file list." )
{
   object->update();
}

DefineEngineMethod( GuiDirectoryFileListCtrl, setPath, bool, ( const char* path, const char* filter ),,
   "Set the search path and file filter.\n\n"
   "@param path   Path in game directory from which to list files.\n"
   "@param filter Tab-delimited list of file name patterns. Only matched files will be displayed.\n" )
{
   return object->setCurrentPath( path, filter );
}

DefineEngineMethod( GuiDirectoryFileListCtrl, getSelectedFiles, const char*, (),,
   "Get the list of selected files.\n\n"
   "@return A space separated list of selected files" )
{
   Vector<S32> ItemVector;
   object->getSelectedItems( ItemVector );

   if( ItemVector.empty() )
      return StringTable->insert( "" );

   // Get an adequate buffer
   static const U32 itemBufSize = 256;
   char itemBuffer[itemBufSize];

   static const U32 bufSize = ItemVector.size() * 64;
   char* returnBuffer = Con::getReturnBuffer( bufSize );
   dMemset( returnBuffer, 0, bufSize );

   // Fetch the first entry
   StringTableEntry itemText = object->getItemText( ItemVector[0] );
   if( !itemText )
      return StringTable->lookup("");
   dSprintf( returnBuffer, bufSize, "%s", itemText );

   // If only one entry, return it.
   if( ItemVector.size() == 1 )
      return returnBuffer;

   // Fetch the remaining entries
   for( S32 i = 1; i < ItemVector.size(); i++ )
   {
      StringTableEntry itemText = object->getItemText( ItemVector[i] );
      if( !itemText )
         continue;

      dMemset( itemBuffer, 0, itemBufSize );
      dSprintf( itemBuffer, itemBufSize, " %s", itemText );
      dStrcat( returnBuffer, itemBuffer );
   }

   return returnBuffer;

}

StringTableEntry GuiDirectoryFileListCtrl::getSelectedFileName()
{
   S32 item = getSelectedItem();
   if( item == -1 )
      return StringTable->lookup("");

   StringTableEntry itemText = getItemText( item );
   if( !itemText )
      return StringTable->lookup("");

   return itemText;
}

DefineEngineMethod( GuiDirectoryFileListCtrl, getSelectedFile, const char*, (),,
   "Get the currently selected filename.\n\n"
   "@return The filename of the currently selected file\n" )
{
   return object->getSelectedFileName();
}
