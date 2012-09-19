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

#ifndef _GUI_DIRECTORYFILELISTCTRL_H_
#define _GUI_DIRECTORYFILELISTCTRL_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

#ifndef _GUI_LISTBOXCTRL_H_
#include "gui/controls/guiListBoxCtrl.h"
#endif

class GuiDirectoryFileListCtrl : public GuiListBoxCtrl
{
private:
   typedef GuiListBoxCtrl Parent;
protected:
   StringTableEntry mFilePath;
   StringTableEntry mFilter;

   void openDirectory();
   
   static bool _setFilePath( void *object, const char *index, const char *data )
   {
      GuiDirectoryFileListCtrl* ctrl = ( GuiDirectoryFileListCtrl* ) object;
      ctrl->setCurrentPath( data, ctrl->mFilter );
      return false;
   }
   static bool _setFilter( void *object, const char *index, const char *data )
   {
      GuiDirectoryFileListCtrl* ctrl = ( GuiDirectoryFileListCtrl* ) object;
      ctrl->setCurrentFilter( data );
      return false;
   }
   
public:
   GuiDirectoryFileListCtrl();
   
   DECLARE_CONOBJECT(GuiDirectoryFileListCtrl);
   DECLARE_DESCRIPTION( "A control that displays a list of files from within a single\n"
                        "directory in the game file system." );
   
   static void initPersistFields();
   
   void update() { openDirectory(); }

   /// Set the current path to grab files from
   bool setCurrentPath( const char* path, const char* filter );
   void setCurrentFilter( const char* filter );

   /// Get the currently selected file's name
   StringTableEntry getSelectedFileName();

   virtual void onMouseDown(const GuiEvent &event);
   virtual bool onWake();
};

#endif
