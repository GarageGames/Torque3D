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

#ifndef _FOREST_EDITOR_SELECTIONTOOL_H_
#define _FOREST_EDITOR_SELECTIONTOOL_H_

#ifndef _FOREST_EDITOR_TOOL_H_
#include "forest/editor/forestTool.h"
#endif
#ifndef _FORESTITEM_H_
#include "forest/forestItem.h"
#endif
#ifndef _TSELECTION_H_
#include "gui/worldEditor/tSelection.h"
#endif


class Forest;
class Gizmo;
class GizmoProfile;
class ForestUpdateAction;

class ForestItemSelection : public Selection<ForestItem>
{
public:

   void setForestData( ForestData* data ) { mData = data; }

protected:

   void offsetObject( ForestItem &object, const Point3F &delta );
   void rotateObject( ForestItem &object, const EulerF &delta, const Point3F &origin );
   void scaleObject( ForestItem &object, const Point3F &delta );

protected:

   ForestData *mData;
};


class ForestSelectionTool : public ForestTool
{
   typedef ForestTool Parent;

   protected:

      Gizmo *mGizmo;
      GizmoProfile *mGizmoProfile;

      ForestItem mHoverItem;

      ForestItemSelection mSelection;
      
      ForestItemSelection mDragSelection;
      bool mDragSelect;
      RectI mDragRect;
      Point2I mDragStart;
      bool mMouseDown;
      bool mMouseDragged;
      ColorI mDragRectColor;
      bool mUsingGizmo;
      
      Box3F mBounds;

      ForestUpdateAction *mCurrAction;

      void _selectItem( const ForestItem &item );

   public:

      ForestSelectionTool();
      virtual ~ForestSelectionTool();

      DECLARE_CONOBJECT( ForestSelectionTool );

      // ForestTool
      virtual void setParentEditor( ForestEditorCtrl *editor );
      virtual void setActiveForest( Forest *forest );
      virtual void on3DMouseDown( const Gui3DMouseEvent &evt );
      virtual void on3DMouseUp( const Gui3DMouseEvent &evt );
      virtual void on3DMouseMove( const Gui3DMouseEvent &evt );
      virtual void on3DMouseDragged( const Gui3DMouseEvent &evt );
      virtual void onRender3D();
      virtual void onRender2D();
      virtual bool updateGuiInfo();
      virtual void updateGizmo();
      virtual void onUndoAction();

      S32 getSelectionCount() const { return mSelection.size(); }

      void deleteSelection();
      void clearSelection();
      void cutSelection();
      void copySelection();
      void pasteSelection();                
};


#endif // _FOREST_EDITOR_SELECTIONTOOL_H_



