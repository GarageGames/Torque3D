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

#ifndef _GUICONVEXSHAPEEDITORCTRL_H_
#define _GUICONVEXSHAPEEDITORCTRL_H_

#ifndef _EDITTSCTRL_H_
#include "gui/worldEditor/editTSCtrl.h"
#endif
#ifndef _UNDO_H_
#include "util/undo.h"
#endif
#ifndef _GIZMO_H_
#include "gui/worldEditor/gizmo.h"
#endif
#ifndef _CONVEXSHAPE_H_
#include "T3D/convexShape.h"
#endif

class GameBase;
class GuiConvexEditorUndoAction;
class ConvexEditorTool;
class ConvexEditorCreateTool;

class GuiConvexEditorCtrl : public EditTSCtrl
{
   typedef EditTSCtrl Parent;

   friend class GuiConvexEditorUndoAction;   

public:   

   GuiConvexEditorCtrl();
   virtual ~GuiConvexEditorCtrl();

   DECLARE_CONOBJECT( GuiConvexEditorCtrl );

   // SimObject
   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();

   // GuiControl
   virtual bool onWake();
   virtual void onSleep();
   virtual void setVisible(bool value);

   // EditTSCtrl      
   bool onKeyDown( const GuiEvent &event );
   bool onKeyUp( const GuiEvent &event );
   void get3DCursor( GuiCursor *&cursor, bool &visible, const Gui3DMouseEvent &event_ );
   void on3DMouseDown( const Gui3DMouseEvent &event );
   void on3DMouseUp( const Gui3DMouseEvent &event );
   void on3DMouseMove( const Gui3DMouseEvent &event );
   void on3DMouseDragged( const Gui3DMouseEvent &event );
   void on3DMouseEnter( const Gui3DMouseEvent &event );
   void on3DMouseLeave( const Gui3DMouseEvent &event );
   void on3DRightMouseDown( const Gui3DMouseEvent &event );
   void on3DRightMouseUp( const Gui3DMouseEvent &event );
   void renderScene(const RectI & updateRect);   
   void updateGizmo();

   void updateShape( ConvexShape *shape, S32 offsetFace = -1 );
   static void synchClientObject( const ConvexShape *serverConvex );

   void updateGizmoPos();

   bool setActiveTool( ConvexEditorTool *tool );

   void drawFacePlane( ConvexShape *shape, S32 faceId );

   void scaleFace( ConvexShape *shape, S32 faceId, Point3F scale );

   void translateFace( ConvexShape *shape, S32 faceId, const Point3F &displace );

   void updateModifiedFace( ConvexShape *shape, S32 faceId );

   bool isShapeValid( ConvexShape *shape );

   void setupShape( ConvexShape *shape );

   void setPivotPos( ConvexShape *shape, S32 faceId, const Gui3DMouseEvent &event );

   void cleanMatrix( MatrixF &mat );

   S32 getEdgeByPoints( ConvexShape *shape, S32 faceId, S32 pId0, S32 pId1 );

   bool getEdgesTouchingPoint( ConvexShape *shape, S32 faceId, S32 pId, Vector< U32 > &edgeIdxList, S32 excludeEdge = -1 );

   void hollowSelection();
   void hollowShape( ConvexShape *shape, F32 thickness );   

   void recenterSelection();
   void recenterShape( ConvexShape *shape );
   void dropSelectionAtScreenCenter();
   void splitSelectedFace();

   /// Interface with Tools.
   /// @{ 

      MatrixF getCameraMat() const { return mLastCameraQuery.cameraMatrix; }

      enum UndoType
      {
         ModifyShape = 0,
         CreateShape,
         DeleteShape,
         HollowShape
      };

      void submitUndo( UndoType type, ConvexShape *shape );   
      void submitUndo( UndoType type, const Vector< ConvexShape* > &shape );

   /// @}

   bool hasSelection() const;
   void clearSelection();
   void setSelection( ConvexShape *shape, S32 faceId );
   void handleDeselect();
   bool handleEscape();
   bool handleDelete();   
   bool handleTab();
public:

	StringTableEntry mMaterialName;
protected:

   void _prepRenderImage( SceneManager* sceneGraph, const SceneRenderState* sceneState );
   void _renderObject( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *matInst );

   bool _cursorCast( const Gui3DMouseEvent &event, ConvexShape **hitShape, S32 *hitFace );
   static bool _cursorCastCallback( RayInfo* ri );

protected:

   bool mIsDirty;

   U32 mSavedGizmoFlags;

   /// The selected ConvexShape.
   SimObjectPtr<ConvexShape> mConvexSEL;      

   /// The highlighted ConvexShape ( mouse over ).
   SimObjectPtr<ConvexShape> mConvexHL;

   S32 mFaceSEL;
   S32 mFaceHL;

   MatrixF mFaceSavedXfm;

   ConvexShape::Geometry mSavedGeometry;
   Vector< MatrixF > mSavedSurfaces;
   Vector< MatrixF > mLastValidShape;

   Point3F mSavedPivotPos;

   bool mCtrlDown;
   bool mSavedUndo;
   bool mHasGeometry;
   bool mDragging;
   bool mMouseDown;
   bool mHasCopied;
   RayInfo mLastRayInfo;

   Gui3DMouseEvent mMouseDownEvent;

   Point3F mGizmoMatOffset;

   Point3F mPivotPos;
   bool mUsingPivot;
   bool mSettingPivot;

   UndoAction *mLastUndo;
   UndoManager *mUndoManager;

   ConvexEditorTool *mActiveTool;
   ConvexEditorCreateTool *mCreateTool;   
};

class GuiConvexEditorUndoAction : public UndoAction
{
   friend class GuiConvexEditorCtrl;
public:

   GuiConvexEditorUndoAction( const UTF8* actionName ) : UndoAction( actionName )
   {
   }

   GuiConvexEditorCtrl *mEditor;         
   
   SimObjectId mObjId;

   Vector< MatrixF > mSavedSurfaces;
   MatrixF mSavedObjToWorld;
   Point3F mSavedScale;   

   virtual void undo();
   virtual void redo() { undo(); }
};

class ConvexEditorTool
{
public:

   enum EventResult
   {
      NotHandled = 0,
      Handled = 1,
      Done = 2,
      Failed = 3
   };

   ConvexEditorTool( GuiConvexEditorCtrl *editor ) 
      : mEditor( editor ), mDone( false ) {}
   virtual ~ConvexEditorTool() {}

   virtual void onActivated( ConvexEditorTool *prevTool ) {}
   virtual void onDeactivated( ConvexEditorTool *newTool ) {}

   virtual EventResult onKeyDown( const GuiEvent &event ) { return NotHandled; }
   virtual EventResult on3DMouseDown( const Gui3DMouseEvent &event ) { return NotHandled; }
   virtual EventResult on3DMouseUp( const Gui3DMouseEvent &event ) { return NotHandled; }
   virtual EventResult on3DMouseMove( const Gui3DMouseEvent &event ) { return NotHandled; }
   virtual EventResult on3DMouseDragged( const Gui3DMouseEvent &event ) { return NotHandled; }
   virtual EventResult on3DMouseEnter( const Gui3DMouseEvent &event ) { return NotHandled; }
   virtual EventResult on3DMouseLeave( const Gui3DMouseEvent &event ) { return NotHandled; }
   virtual EventResult on3DRightMouseDown( const Gui3DMouseEvent &event ) { return NotHandled; }
   virtual EventResult on3DRightMouseUp( const Gui3DMouseEvent &event ) { return NotHandled; }
   
   virtual void renderScene(const RectI & updateRect) {}
   virtual void render2D() {} 

   bool isDone() { return mDone; }

public:
	GuiConvexEditorCtrl *mEditor;
protected:
   bool mDone;
};

class ConvexEditorCreateTool : public ConvexEditorTool
{
   typedef ConvexEditorTool Parent;
public:
   ConvexEditorCreateTool( GuiConvexEditorCtrl *editor );
   virtual ~ConvexEditorCreateTool() {}

   virtual void onActivated( ConvexEditorTool *prevTool );
   virtual void onDeactivated( ConvexEditorTool *newTool );

   virtual EventResult on3DMouseDown( const Gui3DMouseEvent &event );
   virtual EventResult on3DMouseUp( const Gui3DMouseEvent &event );
   virtual EventResult on3DMouseMove( const Gui3DMouseEvent &event );
   virtual EventResult on3DMouseDragged( const Gui3DMouseEvent &event );

   virtual void renderScene(const RectI & updateRect);   

   ConvexShape* extrudeShapeFromFace( ConvexShape *shape, S32 face );

protected:
	
   S32 mStage;   
   ConvexShape *mNewConvex;
   PlaneF mCreatePlane;
   
   MatrixF mTransform;
   Point3F mStart;
   Point3F mEnd;
   Point3F mPlaneSizes;
};

#endif // _GUICONVEXSHAPEEDITORCTRL_H_



