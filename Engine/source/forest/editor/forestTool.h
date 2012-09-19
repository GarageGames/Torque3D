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

#ifndef _FOREST_EDITOR_TOOL_H_
#define _FOREST_EDITOR_TOOL_H_

#ifndef _FORESTITEM_H_
#include "forest/forestItem.h"
#endif
#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _GUITYPES_H_
#include "gui/core/guiTypes.h"
#endif

class Forest;
class ForestEditorCtrl;
class ForestData;
class UndoAction;
class GuiTSCtrl;
class Point3F;
struct Gui3DMouseEvent;


class ForestTool : public SimObject
{
   typedef SimObject Parent;

   protected:

      SimObjectPtr<Forest> mForest;
      ForestEditorCtrl *mEditor;

      void _submitUndo( UndoAction *action );

   public:

      ForestTool();
      virtual ~ForestTool();

      DECLARE_CONOBJECT( ForestTool );

      virtual void setActiveForest( Forest *forest ) { mForest = forest; }
      virtual void setParentEditor( ForestEditorCtrl *editor ) { mEditor = editor; }

      virtual void onActivated( const Gui3DMouseEvent &lastEvent ) {}
      virtual void onDeactivated() {}

      virtual void on3DMouseDown( const Gui3DMouseEvent &evt ) {}
      virtual void on3DMouseUp( const Gui3DMouseEvent &evt ) {}
      virtual void on3DMouseMove( const Gui3DMouseEvent &evt ) {}
      virtual void on3DMouseDragged( const Gui3DMouseEvent &evt ) {}
      virtual void on3DMouseEnter( const Gui3DMouseEvent &evt ) {}
      virtual void on3DMouseLeave( const Gui3DMouseEvent &evt ) {}
      virtual bool onMouseWheel( const GuiEvent &evt ) { return false; }      
      virtual void onRender3D() {}
      virtual void onRender2D() {}      
      virtual void updateGizmo() {}
      virtual bool updateGuiInfo() { return false; }
      virtual void onUndoAction() {}
};


#endif // _FOREST_EDITOR_TOOL_H_



