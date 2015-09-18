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

#include "gui/controls/guiFileTreeCtrl.h"
#include "core/strings/findMatch.h"
#include "core/frameAllocator.h"
#include "core/strings/stringUnit.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"


IMPLEMENT_CONOBJECT(GuiFileTreeCtrl);

ConsoleDocClass( GuiFileTreeCtrl,
   "@brief A control that displays a hierarchical tree view of a path in the game file system.\n\n"
   "@note Currently not used, most likely existed for editors. Possibly deprecated.\n\n"
   "@internal"
);


static bool _isDirInMainDotCsPath(const char* dir)
{
   StringTableEntry cs = Platform::getMainDotCsDir();
   U32 len = dStrlen(cs) + dStrlen(dir) + 2;
   FrameTemp<UTF8> fullpath(len);
   dSprintf(fullpath, len, "%s/%s", cs, dir);

   return Platform::isDirectory(fullpath);
}

static bool _hasChildren(const char* path)
{
   if( Platform::hasSubDirectory(path))
      return true;
      
   Vector<StringTableEntry> dummy;
   Platform::dumpDirectories( path, dummy, 0, true);
   
   return dummy.size() > 0;
}

GuiFileTreeCtrl::GuiFileTreeCtrl()
   : Parent()
{
   // Parent configuration
   setBounds(0,0,200,100);
   mDestroyOnSleep = false;
   mSupportMouseDragging = false;
   mMultipleSelections = false;

   mFileFilter = "*.cs *.gui *.ed.cs";
   _initFilters();
}

void GuiFileTreeCtrl::initPersistFields()
{
   addGroup( "File Tree" );
   addField( "rootPath",   TypeRealString,   Offset( mRootPath, GuiFileTreeCtrl ),     "Path in game directory that should be displayed in the control." );
   addProtectedField( "fileFilter", TypeRealString,   Offset( mFileFilter, GuiFileTreeCtrl ),
                      &_setFileFilterValue, &defaultProtectedGetFn, "Vector of file patterns.  If not empty, only files matching the pattern will be shown in the control." );
   endGroup( "File Tree" );
   
   Parent::initPersistFields();
}

static void _dumpFiles(const char *path, Vector<StringTableEntry> &directoryVector, S32 depth = 0)
{
   Vector<Platform::FileInfo> fileVec;
   Platform::dumpPath( path, fileVec, depth);
   
   for(U32 i = 0; i < fileVec.size(); i++)
   {
      directoryVector.push_back( StringTable->insert(fileVec[i].pFileName) );
   }
}

void GuiFileTreeCtrl::updateTree()
{
   // Kill off any existing items
   _destroyTree();

   // Here we're going to grab our system volumes from the platform layer and create them as roots
   //
   // Note : that we're passing a 1 as the last parameter to Platform::dumpDirectories, which tells it
   // how deep to dump in recursion.  This is an optimization to keep from dumping the whole file system
   // to the tree.  The tree will dump more paths as necessary when the virtual parents are expanded,
   // much as windows does.

   // Determine the root path.
   
   String rootPath = Platform::getMainDotCsDir();
   if( !mRootPath.isEmpty() )
      rootPath = String::ToString( "%s/%s", rootPath.c_str(), mRootPath.c_str() );

   // get the files in the main.cs dir
   Vector<StringTableEntry> pathVec;
   Platform::dumpDirectories( rootPath, pathVec, 0, true);
   _dumpFiles( rootPath, pathVec, 0);
   if( ! pathVec.empty() )
   {
      // get the last folder in the path.
      char *dirname = dStrdup(rootPath);
      U32 last = dStrlen(dirname)-1;
      if(dirname[last] == '/')
         dirname[last] = '\0';
      char* lastPathComponent = dStrrchr(dirname,'/');
      if(lastPathComponent)
         *lastPathComponent++ = '\0';
      else
         lastPathComponent = dirname;
      
      // Iterate through the returned paths and add them to the tree
      Vector<StringTableEntry>::iterator j = pathVec.begin();
      for( ; j != pathVec.end(); j++ )
      {
         char fullModPathSub [512];
         dMemset( fullModPathSub, 0, 512 );
         dSprintf( fullModPathSub, 512, "%s/%s", lastPathComponent, (*j) );
         addPathToTree( *j );
      }
      dFree(dirname);
   }
}

bool GuiFileTreeCtrl::onWake()
{
   if( !Parent::onWake() )
      return false;
      
   updateTree();

   return true;
}

bool GuiFileTreeCtrl::onVirtualParentExpand(Item *item)
{
   if( !item || !item->isExpanded() )
      return true;

   const char* pathToExpand = item->getValue();
   if( !pathToExpand )
   {
      Con::errorf("GuiFileTreeCtrl::onVirtualParentExpand - Unable to retrieve item value!");
      return false;
   }

   Vector<StringTableEntry> pathVec;
   _dumpFiles( pathToExpand, pathVec, 0 );
   Platform::dumpDirectories( pathToExpand, pathVec, 0, true);
   if( ! pathVec.empty() )
   {
      // Iterate through the returned paths and add them to the tree
      Vector<StringTableEntry>::iterator i = pathVec.begin();
      for( ; i != pathVec.end(); i++ )
         recurseInsert(item, (*i) );

      item->setExpanded( true );
   }

   item->setVirtualParent( false );

   // Update our tree view
   buildVisibleTree();

   return true;

}

void GuiFileTreeCtrl::addPathToTree( StringTableEntry path )
{
   if( !path )
   {
      Con::errorf("GuiFileTreeCtrl::addPathToTree - Invalid Path!");
      return;
   }

   // Identify which root (volume) this path belongs to (if any)
   S32 root = getFirstRootItem();
   StringTableEntry ourPath = &path[ dStrcspn( path, "/" ) + 1];
   StringTableEntry ourRoot = StringUnit::getUnit( path, 0, "/" );
   // There are no current roots, we can safely create one
   if( root == 0 )
   {
      recurseInsert( NULL, path );
   }
   else
   {
      while( root != 0 )
      {
         if( dStricmp( getItemValue( root ), ourRoot ) == 0 )
         {
            recurseInsert( getItem( root ), ourPath );
            break;
         }
         root = this->getNextSiblingItem( root );
      }
      // We found none so we'll create one
      if ( root == 0 )
      {
         recurseInsert( NULL, path );
      }
   }
}

void GuiFileTreeCtrl::onItemSelected( Item *item )
{
   Con::executef( this, "onSelectPath", avar("%s",item->getValue()) );

   mSelPath = item->getValue();
   if( _hasChildren( mSelPath ) )
      item->setVirtualParent( true );
}

bool GuiFileTreeCtrl::_setFileFilterValue( void *object, const char *index, const char *data )
{
   GuiFileTreeCtrl* ctrl = ( GuiFileTreeCtrl* ) object;
   
   ctrl->mFileFilter = data;
   ctrl->_initFilters();
   
   return false;
}

void GuiFileTreeCtrl::_initFilters()
{
   mFilters.clear();
   
   U32 index = 0;
   while( true )
   {
      const char* pattern = StringUnit::getUnit( mFileFilter, index, " " );
      if( !pattern[ 0 ] )
         break;
         
      mFilters.push_back( pattern );
      ++ index;
   }
}

bool GuiFileTreeCtrl::matchesFilters(const char* filename)
{
   if( !mFilters.size() )
      return true;
      
   for(S32 i = 0; i < mFilters.size(); i++)
   {
      if(FindMatch::isMatch( mFilters[i], filename))
         return true;
   }
   return false;
}

void GuiFileTreeCtrl::recurseInsert( Item* parent, StringTableEntry path )
{
   if( !path )
      return;

   char szPathCopy [ 1024 ];
   dMemset( szPathCopy, 0, 1024 );
   dStrcpy( szPathCopy, path );

   // Jump over the first character if it's a root /
   char *curPos = szPathCopy;
   if( *curPos == '/' )
      curPos++;

   char szValue[1024];
   dMemset( szValue, 0, 1024 );
   if( parent )
   {
      dMemset( szValue, 0, sizeof( szValue ) );
      dSprintf( szValue, sizeof( szValue ), "%s/%s", parent->getValue(), curPos );
   }
   else
   {
      dStrncpy( szValue, curPos, sizeof( szValue ) );
      szValue[ sizeof( szValue ) - 1 ] = 0;
   }
   
   const U32 valueLen = dStrlen( szValue );
   char* value = new char[ valueLen + 1 ];
   dMemcpy( value, szValue, valueLen + 1 );
   
   char *delim = dStrchr( curPos, '/' );
   if ( delim )
   {
      // terminate our / and then move our pointer to the next character (rest of the path)
      *delim = 0x00;
      delim++;
   }
   S32 itemIndex = 0;
   // only insert blindly if we have no root
   if( !parent )
      itemIndex = insertItem( 0, curPos, curPos );
   else
   {
      bool allowed = (_isDirInMainDotCsPath(value) || matchesFilters(value));
      Item *exists = parent->findChildByValue( szValue );
      if( allowed && !exists && dStrcmp( curPos, "" ) != 0 )
      {
         // Since we're adding a child this parent can't be a virtual parent, so clear that flag
         parent->setVirtualParent( false );

         itemIndex = insertItem( parent->getID(), curPos);
         Item *newitem = getItem(itemIndex);
         newitem->setValue( value );
      }
      else
      {
         itemIndex = ( parent != NULL ) ? ( ( exists != NULL ) ? exists->getID() : -1 ) : -1;
      }
   }

   Item *newitem = getItem(itemIndex);
   if(newitem)
   {
      newitem->setValue( value );
      if( _isDirInMainDotCsPath( value ) )
      {
         newitem->setNormalImage( Icon_FolderClosed );
         newitem->setExpandedImage( Icon_Folder );
         newitem->setVirtualParent(true);
         newitem->setExpanded(false);
      }
      else
      {
         newitem->setNormalImage( Icon_Doc );
      }
   }
   // since we're only dealing with volumes and directories, all end nodes will be virtual parents
   // so if we are at the bottom of the rabbit hole, set the item to be a virtual parent
   Item* item = getItem( itemIndex );
   if(item)
   {
      item->setExpanded(false);
      if(parent && _isDirInMainDotCsPath(item->getValue()) && Platform::hasSubDirectory(item->getValue()))
         item->setVirtualParent(true);
   }
   if( delim )
   {
      if( ( dStrcmp( delim, "" ) == 0 ) && item )
      {
         item->setExpanded( false );
         if( parent && _hasChildren( item->getValue() ) )
            item->setVirtualParent( true );
      }
   }
   else
   {
      if( item )
      {
         item->setExpanded( false );
         if( parent &&  _hasChildren( item->getValue() ) )
            item->setVirtualParent( true );
      }
   }
   
   // Down the rabbit hole we go
   recurseInsert( getItem( itemIndex ), delim );

}

DefineConsoleMethod( GuiFileTreeCtrl, getSelectedPath, const char*, (), , "getSelectedPath() - returns the currently selected path in the tree")
{
   const String& path = object->getSelectedPath();
   return Con::getStringArg( path );
}

DefineConsoleMethod( GuiFileTreeCtrl, setSelectedPath, bool, (const char * path), , "setSelectedPath(path) - expands the tree to the specified path")
{
   return object->setSelectedPath( path );
}

DefineConsoleMethod( GuiFileTreeCtrl, reload, void, (), , "() - Reread the directory tree hierarchy." )
{
   object->updateTree();
}

bool GuiFileTreeCtrl::setSelectedPath( const char* path )
{
   if( !path )
      return false;

   // Since we only list one deep on paths, we need to add the path to the tree just incase it isn't already indexed in the tree
   // or else we wouldn't be able to select a path we hadn't previously browsed to. :)
   if( _isDirInMainDotCsPath( path ) )
      addPathToTree( path );

   // see if we have a child that matches what we want
   for(U32 i = 0; i < mItems.size(); i++)
   {
      if( dStricmp( mItems[i]->getValue(), path ) == 0 )
      {
         Item* item = mItems[i];
         AssertFatal(item,"GuiFileTreeCtrl::setSelectedPath - Item Index Bad, Fatal Mistake!!!");
         item->setExpanded( true );
         clearSelection();
         setItemSelected( item->getID(), true );
         // make sure all of it's parents are expanded
         S32 parent = getParentItem( item->getID() );
         while( parent != 0 )
         {
            setItemExpanded( parent, true );
            parent = getParentItem( parent );
         }
         // Rebuild our tree just incase we've oops'd
         buildVisibleTree();
         scrollVisible( item );
      }
   }
   return false;
}
