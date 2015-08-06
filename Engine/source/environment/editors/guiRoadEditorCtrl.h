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

#ifndef _GUIROADEDITORCTRL_H_
#define _GUIROADEDITORCTRL_H_

#ifndef _EDITTSCTRL_H_
#include "gui/worldEditor/editTSCtrl.h"
#endif
#ifndef _UNDO_H_
#include "util/undo.h"
#endif
#ifndef _DECALROAD_H_
#include "environment/decalRoad.h"
#endif

class GameBase;
class DecalRoad;


class GuiRoadEditorCtrl : public EditTSCtrl
{
   typedef EditTSCtrl Parent;

   public:
   
      friend class GuiRoadEditorUndoAction;
		
		String mSelectRoadMode;
		String mAddRoadMode;
		String mAddNodeMode;
		String mInsertPointMode;
		String mRemovePointMode;
		String mMovePointMode;
		String mScalePointMode;

      GuiRoadEditorCtrl();
      ~GuiRoadEditorCtrl();

      DECLARE_CONOBJECT(GuiRoadEditorCtrl);

      // SimObject
      bool onAdd();
      static void initPersistFields();

      // GuiControl
      virtual void onSleep();
      virtual void onRender(Point2I offset, const RectI &updateRect);

      // EditTSCtrl      
		bool onKeyDown(const GuiEvent& event);
      void get3DCursor( GuiCursor *&cursor, bool &visible, const Gui3DMouseEvent &event_ );
      void on3DMouseDown(const Gui3DMouseEvent & event);
      void on3DMouseUp(const Gui3DMouseEvent & event);
      void on3DMouseMove(const Gui3DMouseEvent & event);
      void on3DMouseDragged(const Gui3DMouseEvent & event);
      void on3DMouseEnter(const Gui3DMouseEvent & event);
      void on3DMouseLeave(const Gui3DMouseEvent & event);
      void on3DRightMouseDown(const Gui3DMouseEvent & event);
      void on3DRightMouseUp(const Gui3DMouseEvent & event);
      void updateGuiInfo();      
      void renderScene(const RectI & updateRect);
      void renderGui(Point2I offset, const RectI &updateRect);

      bool getTerrainPos( const Gui3DMouseEvent & event, Point3F &tpos );
      void deleteSelectedNode();
      void deleteSelectedRoad( bool undoAble = true );
      
      void setMode( String mode, bool sourceShortcut );
      String getMode() { return mMode; }

      void setSelectedRoad( DecalRoad *road );
      DecalRoad* getSelectedRoad() { return mSelRoad; };
      void setSelectedNode( S32 node );
      S32 getSelectedNode() { return mSelNode; };

      F32 getNodeWidth();
      void setNodeWidth( F32 width );

		Point3F getNodePosition();
      void setNodePosition(const Point3F& pos);

      void setTextureFile( StringTableEntry file );
	
	public:

		StringTableEntry mMaterialName;
   protected:

      void _drawRoadSpline( DecalRoad *road, const ColorI &color );
      void _drawRoadControlNodes( DecalRoad *road, const ColorI &color );

      void submitUndo( const UTF8 *name );

      bool mSavedDrag;
      bool mIsDirty;

      SimSet *mRoadSet;
      S32 mSelNode;
      S32 mHoverNode;
      U32 mAddNodeIdx;
      SimObjectPtr<DecalRoad> mSelRoad;      
      SimObjectPtr<DecalRoad> mHoverRoad;

      ColorI mHoverSplineColor;
      ColorI mSelectedSplineColor;
      ColorI mHoverNodeColor;

      String mMode;

      F32 mDefaultWidth;
      S32 mInsertIdx;

      F32 mStartWidth;
      S32 mStartX;
      Point3F mStartWorld;

      Point2I mNodeHalfSize;

      // StateBlock for rendering road splines.
      GFXStateBlockRef mZDisableSB;
};

class GuiRoadEditorUndoAction : public UndoAction
{
   public:

      GuiRoadEditorUndoAction( const UTF8* actionName ) : UndoAction( actionName )
      {
      }

      GuiRoadEditorCtrl *mRoadEditor;

      Vector<RoadNode> mNodes;

      SimObjectId mObjId;
      String mMaterialName;
      F32 mBreakAngle;
      U32 mSegmentsPerBatch;
      F32 mTextureLength;

      virtual void undo();
      virtual void redo() { undo(); }
};

#endif



