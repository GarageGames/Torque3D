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

#ifndef _GUIMESHROADEDITORCTRL_H_
#define _GUIMESHROADEDITORCTRL_H_

#ifndef _EDITTSCTRL_H_
#include "gui/worldEditor/editTSCtrl.h"
#endif
#ifndef _UNDO_H_
#include "util/undo.h"
#endif
#ifndef _GIZMO_H_
#include "gui/worldEditor/gizmo.h"
#endif
#ifndef _MESHROAD_H_
#include "environment/meshRoad.h"
#endif

class GameBase;

class GuiMeshRoadEditorCtrl : public EditTSCtrl
{
   typedef EditTSCtrl Parent;

   public:
   
      friend class GuiMeshRoadEditorUndoAction;
		
		String mSelectMeshRoadMode;
		String mAddMeshRoadMode;
		String mAddNodeMode;
		String mInsertPointMode;
		String mRemovePointMode;
		String mMovePointMode;
		String mScalePointMode;
		String mRotatePointMode;

      GuiMeshRoadEditorCtrl();
      ~GuiMeshRoadEditorCtrl();

      DECLARE_CONOBJECT(GuiMeshRoadEditorCtrl);

      // SimObject
      bool onAdd();
      static void initPersistFields();

      // GuiControl
      virtual void onSleep();

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

      // GuiRiverEditorCtrl      
      bool getStaticPos( const Gui3DMouseEvent & event, Point3F &tpos );
      void deleteSelectedNode();
      void deleteSelectedRoad( bool undoAble = true );

      void setMode( String mode, bool sourceShortcut  );
      String getMode() { return mMode; }

      void setSelectedRoad( MeshRoad *road );
      MeshRoad* getSelectedRoad() { return mSelRoad; };
      void setSelectedNode( S32 node );

      F32 getNodeWidth();
      void setNodeWidth( F32 width );

      F32 getNodeDepth();
      void setNodeDepth( F32 depth );
		
		Point3F getNodePosition();
      void setNodePosition(const Point3F& pos);

      VectorF getNodeNormal();
      void setNodeNormal( const VectorF &normal );

      void matchTerrainToRoad();

   protected:
		
		enum { 
			Top = 0, 
			Bottom = 1, 
			Side = 2,
			SurfaceCount = 3
		};

      S32 _getNodeAtScreenPos( const MeshRoad *pRoad, const Point2I &posi );
      void _drawSpline( MeshRoad *road, const ColorI &color );
      void _drawControlNodes( MeshRoad *road, const ColorI &color );

      void submitUndo( const UTF8 *name = "Action" );

      GFXStateBlockRef mZDisableSB;
      GFXStateBlockRef mZEnableSB;

      bool mSavedDrag;
      bool mIsDirty;

      SimSet *mRoadSet;
      S32 mSelNode;
      S32 mHoverNode;
      U32 mAddNodeIdx;
      SimObjectPtr<MeshRoad> mSelRoad;      
      SimObjectPtr<MeshRoad> mHoverRoad;

      String mMode;

      F32 mDefaultWidth;
      F32 mDefaultDepth;
      VectorF mDefaultNormal;

      Point2I mNodeHalfSize;      

      ColorI mHoverSplineColor;
      ColorI mSelectedSplineColor;
      ColorI mHoverNodeColor;

      bool mHasCopied;
	public:
		
		StringTableEntry mMaterialName[SurfaceCount];
};

class GuiMeshRoadEditorUndoAction : public UndoAction
{
   public:

      GuiMeshRoadEditorUndoAction( const UTF8* actionName ) : UndoAction( actionName )
      {
      }

      GuiMeshRoadEditorCtrl *mEditor;         

      Vector<MeshRoadNode> mNodes;

      SimObjectId mObjId;
      F32 mMetersPerSegment;

      virtual void undo();
      virtual void redo() { undo(); }
};

#endif // _GUIMESHROADEDITORCTRL_H_



