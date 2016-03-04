//-----------------------------------------------------------------------------
// Copyright (c) 2014 Daniel Buckmaster
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

#ifndef _GUINAVEDITORCTRL_H_
#define _GUINAVEDITORCTRL_H_

#ifndef _EDITTSCTRL_H_
#include "gui/worldEditor/editTSCtrl.h"
#endif
#ifndef _UNDO_H_
#include "util/undo.h"
#endif
#ifndef _GIZMO_H_
#include "gui/worldEditor/gizmo.h"
#endif

#include "navMesh.h"
#include "T3D/aiPlayer.h"

struct ObjectRenderInst;
class SceneManager;
class SceneRenderState;
class BaseMatInstance;

class GuiNavEditorCtrl : public EditTSCtrl
{
   typedef EditTSCtrl Parent;
   friend class GuiNavEditorUndoAction;

public:
   static const String mSelectMode;
   static const String mLinkMode;
   static const String mCoverMode;
   static const String mTileMode;
   static const String mTestMode;

   GuiNavEditorCtrl();
   ~GuiNavEditorCtrl();

   DECLARE_CONOBJECT(GuiNavEditorCtrl);

   /// @name SimObject
   /// @{

   bool onAdd();
   static void initPersistFields();

   /// @}

   /// @name GuiControl
   /// @{

   virtual void onSleep();
   virtual void onRender(Point2I offset, const RectI &updateRect);

   /// @}

   /// @name EditTSCtrl
   /// @{

   void get3DCursor(GuiCursor *&cursor, bool &visible, const Gui3DMouseEvent &event_);
   bool get3DCentre(Point3F &pos);
   void on3DMouseDown(const Gui3DMouseEvent & event);
   void on3DMouseUp(const Gui3DMouseEvent & event);
   void on3DMouseMove(const Gui3DMouseEvent & event);
   void on3DMouseDragged(const Gui3DMouseEvent & event);
   void on3DMouseEnter(const Gui3DMouseEvent & event);
   void on3DMouseLeave(const Gui3DMouseEvent & event);
   void updateGuiInfo();      
   void renderScene(const RectI & updateRect);

   /// @}

   /// @name GuiNavEditorCtrl
   /// @{

   bool getStaticPos(const Gui3DMouseEvent & event, Point3F &tpos);

   void setMode(String mode, bool sourceShortcut);
   String getMode() { return mMode; }

   void selectMesh(NavMesh *mesh);
   void deselect();

   S32 getMeshId();
   S32 getPlayerId();

   String mSpawnClass;
   String mSpawnDatablock;

   void deleteLink();
   void setLinkFlags(const LinkData &d);

   void buildTile();

   void spawnPlayer(const Point3F &pos);

   /// @}

protected:

   void _prepRenderImage(SceneManager* sceneGraph, const SceneRenderState* sceneState);

   void submitUndo(const UTF8 *name = "Action");

   GFXStateBlockRef mZDisableSB;
   GFXStateBlockRef mZEnableSB;

   bool mSavedDrag;
   bool mIsDirty;

   String mMode;

   /// Currently-selected NavMesh.
   SimObjectPtr<NavMesh> mMesh;

   /// @name Link mode
   /// @{

   Point3F mLinkStart;
   S32 mCurLink;
   S32 mLink;

   /// @}

   /// @name Tile mode
   /// @{

   S32 mCurTile;
   S32 mTile;

   duDebugDrawTorque dd;

   /// @}

   /// @name Test mode
   /// @{

   SimObjectPtr<AIPlayer> mPlayer;
   SimObjectPtr<AIPlayer> mCurPlayer;

   /// @}

   Gui3DMouseEvent mLastMouseEvent;

#define InvalidMousePoint Point2I(-100,-100)
   Point2I mStartDragMousePoint;
};

class GuiNavEditorUndoAction : public UndoAction
{
public:
   GuiNavEditorUndoAction(const UTF8* actionName) : UndoAction(actionName)
   {
   }

   GuiNavEditorCtrl *mNavEditor;         

   SimObjectId mObjId;
   F32 mMetersPerSegment;
   U32 mSegmentsPerBatch;

   virtual void undo();
   virtual void redo() { undo(); }
};

#endif



