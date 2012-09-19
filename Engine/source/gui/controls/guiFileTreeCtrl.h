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

#ifndef _GUI_FILETREECTRL_H_
#define _GUI_FILETREECTRL_H_

#include "platform/platform.h"
#include "gui/controls/guiTreeViewCtrl.h"

class GuiFileTreeCtrl : public GuiTreeViewCtrl
{
private:

   // Utility functions
   void recurseInsert( Item* parent, StringTableEntry path );
   void addPathToTree( StringTableEntry path );

protected:
   String               mSelPath;
   String               mFileFilter;
   String               mRootPath;
   Vector< String >     mFilters;

   void _initFilters();
   
   static bool _setFileFilterValue( void *object, const char *index, const char *data );

public:

   typedef GuiTreeViewCtrl Parent;
   
   enum
   {
      Icon_Folder = 1,
      Icon_FolderClosed = 2,
      Icon_Doc = 3
   };
   
   GuiFileTreeCtrl();

   bool onWake();
   bool onVirtualParentExpand(Item *item);
   void onItemSelected( Item *item );
   const String& getSelectedPath() { return mSelPath; }
   bool setSelectedPath( const char* path );
      
   bool matchesFilters(const char* filename);
   void updateTree();

   DECLARE_CONOBJECT( GuiFileTreeCtrl );
   DECLARE_DESCRIPTION( "A control that displays a hierarchical tree view of a path in the game file system.\n"
                        "Note that to enable expanding/collapsing of directories, the control must be\n"
                        "placed inside a GuiScrollCtrl." );

   static void initPersistFields();
};

#endif //_GUI_FILETREECTRL_H_
