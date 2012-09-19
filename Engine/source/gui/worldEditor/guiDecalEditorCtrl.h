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

#ifndef _GUIDECALEDITORCTRL_H_
#define _GUIDECALEDITORCTRL_H_

#ifndef _EDITTSCTRL_H_
#include "gui/worldEditor/editTSCtrl.h"
#endif
#ifndef _UNDO_H_
#include "util/undo.h"
#endif
#ifndef _DECALINSTANCE_H_
#include "T3D/decal/decalInstance.h"
#endif

class GameBase;
class Gizmo;
struct RayInfo;
class DecalInstance;
class DecalData;

class GuiDecalEditorCtrl : public EditTSCtrl
{
   typedef EditTSCtrl Parent;

   public:

      GuiDecalEditorCtrl();
      ~GuiDecalEditorCtrl();

      DECLARE_CONOBJECT(GuiDecalEditorCtrl);

      // SimObject
      bool onAdd();
      static void initPersistFields();      
      static void consoleInit();
      void onEditorDisable();

      // GuiControl
      virtual bool onWake();
      virtual void onSleep();
      virtual void onRender(Point2I offset, const RectI &updateRect);

      // EditTSCtrl      
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

      /// Find clicked point on "static collision" objects.
      bool getRayInfo( const Gui3DMouseEvent &event, RayInfo *rInfo );
      
      void selectDecal( DecalInstance *inst );
      void deleteSelectedDecal();
		void deleteDecalDatablock( String lookupName );
		void retargetDecalDatablock( String dbFrom, String dbTo );
		void setMode( String mode, bool sourceShortcut );
		
		void forceRedraw( DecalInstance * decalInstance );
		void setGizmoFocus( DecalInstance * decalInstance );

	public:

		String mMode;
		DecalInstance *mSELDecal;
		DecalInstance *mHLDecal;

      static bool smRenderDecalPixelSize;

   protected:
     
      bool mPerformedDragCopy;

      DecalData *mCurrentDecalData;

      Vector<Point3F> mSELEdgeVerts;
      Vector<Point3F> mHLEdgeVerts;

      void _renderDecalEdge( const Vector<Point3F> &verts, const ColorI &color );
};

//Decal Instance Create Undo Actions
class DICreateUndoAction : public UndoAction
{
   typedef UndoAction Parent;

public:
	GuiDecalEditorCtrl *mEditor;	

protected:

      /// The captured object state.
      DecalInstance mDecalInstance;
		S32 mDatablockId;

public:

   DECLARE_CONOBJECT( DICreateUndoAction );
   static void initPersistFields();
   
   DICreateUndoAction( const UTF8* actionName = "Create Decal " );
   virtual ~DICreateUndoAction();

   void addDecal( DecalInstance decal );

   // UndoAction
   virtual void undo();
   virtual void redo();
};

//Decal Instance Delete Undo Actions
class DIDeleteUndoAction : public UndoAction
{
   typedef UndoAction Parent;

public:
	GuiDecalEditorCtrl *mEditor;	

protected:

   /// The captured object state.
   DecalInstance mDecalInstance;
	S32 mDatablockId;

public:

   DECLARE_CONOBJECT( DIDeleteUndoAction );
   static void initPersistFields();
   
   DIDeleteUndoAction( const UTF8* actionName = "Delete Decal" );
   virtual ~DIDeleteUndoAction();

   ///
   void deleteDecal( DecalInstance decal );

   // UndoAction
   virtual void undo();
   virtual void redo();
};

//Decal Datablock Delete Undo Actions
class DBDeleteUndoAction : public UndoAction
{
   typedef UndoAction Parent;

public:
	GuiDecalEditorCtrl *mEditor;	
	S32 mDatablockId;

protected:
	
	// The captured decalInstance states
	Vector<DecalInstance> mDecalInstanceVec;
	
public:

   DECLARE_CONOBJECT( DBDeleteUndoAction );
   static void initPersistFields();
   
   DBDeleteUndoAction( const UTF8* actionName = "Delete Decal Datablock" );
   virtual ~DBDeleteUndoAction();

   void deleteDecal( DecalInstance decal );

   // UndoAction
   virtual void undo();
   virtual void redo();
};

//Decal Datablock Retarget Undo Actions
class DBRetargetUndoAction : public UndoAction
{
   typedef UndoAction Parent;

public:
	GuiDecalEditorCtrl *mEditor;	
	S32 mDBFromId;
	S32 mDBToId;

protected:
	
	// The captured decalInstance states
	Vector<DecalInstance*> mDecalInstanceVec;
	
public:

   DECLARE_CONOBJECT( DBRetargetUndoAction );
   static void initPersistFields();
   
   DBRetargetUndoAction( const UTF8* actionName = "Retarget Decal Datablock" );
   virtual ~DBRetargetUndoAction();

   void retargetDecal( DecalInstance* decal );

   // UndoAction
   virtual void undo();
   virtual void redo();
};

#endif // _GUIDECALEDITORCTRL_H_



