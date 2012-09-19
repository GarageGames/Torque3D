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

#ifndef _FOREST_EDITOR_FORESTEDITORCTRL_H_
#define _FOREST_EDITOR_FORESTEDITORCTRL_H_

#ifndef _EDITTSCTRL_H_
#include "gui/worldEditor/editTSCtrl.h"
#endif

#ifndef _H_FOREST_
#include "forest/forest.h"
#endif

#ifndef _FOREST_EDITOR_TOOL_H_
#include "forest/editor/forestTool.h"
#endif


class ForestEditorCtrl : public EditTSCtrl
{
   typedef EditTSCtrl Parent;
   
   friend class ForestPaintEvent;

   protected:

      /// The server Forest we're editing.
      SimObjectPtr<Forest> mForest;

      /// The active tool in used by the editor.
      SimObjectPtr<ForestTool> mTool;

   public:

      ForestEditorCtrl();
      virtual ~ForestEditorCtrl();

      DECLARE_CONOBJECT( ForestEditorCtrl );

      // SimObject
      bool onAdd();
      static void initPersistFields();

      // GuiControl
      virtual bool onWake();
      virtual void onSleep();      
      virtual void onMouseUp( const GuiEvent &event_ );

      // EditTSCtrl      
      void get3DCursor( GuiCursor *&cursor, bool &visible, const Gui3DMouseEvent &event_ );
      void on3DMouseDown( const Gui3DMouseEvent &event_ );
      void on3DMouseUp( const Gui3DMouseEvent &event_ );
      void on3DMouseMove( const Gui3DMouseEvent &event_ );
      void on3DMouseDragged( const Gui3DMouseEvent &event_ );
      void on3DMouseEnter( const Gui3DMouseEvent &event_ );
      void on3DMouseLeave( const Gui3DMouseEvent &event_ );
      void on3DRightMouseDown( const Gui3DMouseEvent &event_ );
      void on3DRightMouseUp( const Gui3DMouseEvent &event_ );
      bool onMouseWheelUp(const GuiEvent &event_);
      bool onMouseWheelDown(const GuiEvent &event_);
      void updateGuiInfo();      
      void updateGizmo();
      void renderScene( const RectI &updateRect );
      void renderGui( Point2I offset, const RectI &updateRect );

      /// Causes the editor to reselect the active forest.
      bool updateActiveForest( bool createNew );

      /// Returns the active Forest.
      Forest *getActiveForest() const { return mForest; }

      void setActiveTool( ForestTool *tool );
      ForestTool* getActiveTool() { return mTool; }

      void onUndoAction();

      void deleteMeshSafe( ForestItemData *itemData );      

      void updateCollision();

      bool isDirty();
};

#endif // _FOREST_EDITOR_FORESTEDITORCTRL_H_



