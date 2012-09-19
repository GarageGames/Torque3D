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

#ifndef _WORLDEDITOR_H_
#define _WORLDEDITOR_H_

#ifndef _EDITTSCTRL_H_
#include "gui/worldEditor/editTSCtrl.h"
#endif

#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif

#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif

#ifndef _TSIGNAL_H_
#include "core/util/tSignal.h"
#endif

#ifndef _CONSOLE_SIMOBJECTMEMENTO_H_
#include "console/simObjectMemento.h"
#endif

#ifndef _UNDO_H_
#include "util/undo.h"
#endif

#ifndef _SIMPATH_H_
#include "scene/simPath.h"
#endif

#ifndef _DYNAMIC_CONSOLETYPES_H_
#include "console/dynamicTypes.h"
#endif


class SceneObject;
class WorldEditorSelection;


///
class WorldEditor : public EditTSCtrl
{
      typedef EditTSCtrl Parent;

	public:

      typedef WorldEditorSelection Selection;

      struct Triangle
      {
         Point3F p0;
         Point3F p1;
         Point3F p2;
      };

      void ignoreObjClass(U32 argc, const char** argv);
      void clearIgnoreList();

      static bool setObjectsUseBoxCenter( void *object, const char *index, const char *data ) { static_cast<WorldEditor*>(object)->setObjectsUseBoxCenter( dAtob( data ) ); return false; };
      void setObjectsUseBoxCenter(bool state);
      bool getObjectsUseBoxCenter() { return mObjectsUseBoxCenter; }

      void clearSelection();
      void selectObject(SimObject *obj);
      void selectObject(const char* obj);
      void unselectObject(SimObject *obj);      
      void unselectObject(const char* obj);
      
      S32 getSelectionSize();
      S32 getSelectObject(S32 index);	
      const Point3F& getSelectionCentroid();
      const char* getSelectionCentroidText();
      const Box3F& getSelectionBounds();
      Point3F getSelectionExtent();
      F32 getSelectionRadius();
      
      void dropCurrentSelection( bool skipUndo );
      void copyCurrentSelection();	
      void cutCurrentSelection();	
      bool canPasteSelection();

      bool alignByBounds(S32 boundsAxis);
      bool alignByAxis(S32 axis);

      void transformSelection(bool position, Point3F& p, bool relativePos, bool rotate, EulerF& r, bool relativeRot, bool rotLocal, S32 scaleType, Point3F& s, bool sRelative, bool sLocal);

      void resetSelectedRotation();
      void resetSelectedScale();

      void addUndoState();
      void redirectConsole(S32 objID);

      void colladaExportSelection( const String &path );

      void makeSelectionPrefab( const char *filename );
      void explodeSelectedPrefab();

      //
      static SceneObject* getClientObj(SceneObject *);
      static void markAsSelected( SimObject* object, bool state );
      static void setClientObjInfo(SceneObject *, const MatrixF &, const VectorF &);
      static void updateClientTransforms(Selection* );

   protected:

      class WorldEditorUndoAction : public UndoAction
      {
      public:

         WorldEditorUndoAction( const UTF8* actionName ) : UndoAction( actionName )
         {
         }

         WorldEditor *mWorldEditor;

         struct Entry
         {
            MatrixF     mMatrix;
            VectorF     mScale;

            // validation
            U32         mObjId;
            U32         mObjNumber;
         };

         Vector<Entry>  mEntries;

         virtual void undo();
         virtual void redo() { undo(); }
      };

      void submitUndo( Selection* sel, const UTF8* label="World Editor Action" );

   public:

      /// The objects currently in the copy buffer.
      Vector<SimObjectMemento> mCopyBuffer;

      bool cutSelection(Selection* sel);
      bool copySelection(Selection*  sel);
      bool pasteSelection(bool dropSel=true);
      void dropSelection(Selection*  sel);
      void dropBelowSelection(Selection*  sel, const Point3F & centroid, bool useBottomBounds=false);

      void terrainSnapSelection(Selection* sel, U8 modifier, Point3F gizmoPos, bool forceStick=false);
      void softSnapSelection(Selection* sel, U8 modifier, Point3F gizmoPos);
      
      Selection* getActiveSelectionSet() const;
      void makeActiveSelectionSet( Selection* sel );

      void hideObject(SceneObject* obj, bool hide);

      // work off of mSelected
      void hideSelection(bool hide);
      void lockSelection(bool lock);

   public:
      bool objClassIgnored(const SimObject * obj);
      void renderObjectBox(SceneObject * obj, const ColorI & col);
      
   private:
      SceneObject * getControlObject();
      bool collide(const Gui3DMouseEvent & event, SceneObject **hitObj );

      // gfx methods
      //void renderObjectBox(SceneObject * obj, const ColorI & col);
      void renderObjectFace(SceneObject * obj, const VectorF & normal, const ColorI & col);
      void renderSelectionWorldBox(Selection*  sel);

      void renderPlane(const Point3F & origin);
      void renderMousePopupInfo();
      void renderScreenObj( SceneObject * obj, const Point3F& sPos, const Point3F& wPos );

      void renderPaths(SimObject *obj);
      void renderSplinePath(SimPath::Path *path);

      // axis gizmo
      bool        mUsingAxisGizmo;

      GFXStateBlockRef mRenderSelectionBoxSB;
      GFXStateBlockRef mRenderObjectBoxSB;
      GFXStateBlockRef mRenderObjectFaceSB;
      GFXStateBlockRef mSplineSB;

      //
      bool                       mIsDirty;

      //
      bool                       mMouseDown;
      SimObjectPtr< Selection >  mSelected;

      SimObjectPtr< Selection >  mDragSelected;
      bool                       mDragSelect;
      RectI                      mDragRect;
      Point2I                    mDragStart;

      // modes for when dragging a selection
      enum {
         Move = 0,
         Rotate,
         Scale
      };

      //
      //U32                        mCurrentMode;
      //U32                        mDefaultMode;

      S32                        mRedirectID;

      /// @name Object Icons
      /// @{

      struct IconObject
      {         
         SceneObject *object;
         F32 dist;         
         RectI rect;
         U32 alpha;
      };
      
      Vector< IconObject > mIcons;
      
      /// If true, icons fade out with distance to mouse cursor.
      bool mFadeIcons;
      
      /// Distance at which to start fading out icons.
      F32 mFadeIconsDist;
      
      /// @}

      SimObjectPtr<SceneObject>  mHitObject;
      SimObjectPtr<SceneObject>  mPossibleHitObject;
      bool                       mMouseDragged;
      Gui3DMouseEvent            mLastMouseEvent;
      Gui3DMouseEvent            mLastMouseDownEvent;

      //
      class ClassInfo
      {
         public:
            ~ClassInfo();

            struct Entry
            {
               StringTableEntry  mName;
               bool              mIgnoreCollision;
               GFXTexHandle      mDefaultHandle;
               GFXTexHandle      mSelectHandle;
               GFXTexHandle      mLockedHandle;
            };

            Vector<Entry*>       mEntries;
      };


      ClassInfo            mClassInfo;
      ClassInfo::Entry     mDefaultClassEntry;

      ClassInfo::Entry * getClassEntry(StringTableEntry name);
      ClassInfo::Entry * getClassEntry(const SimObject * obj);
      bool addClassEntry(ClassInfo::Entry * entry);

   // persist field data
   public:

      enum DropType
      {
         DropAtOrigin = 0,
         DropAtCamera,
         DropAtCameraWithRot,
         DropBelowCamera,
         DropAtScreenCenter,
         DropAtCentroid,
         DropToTerrain,
         DropBelowSelection
      };

      // Snapping alignment mode
      enum AlignmentType
      {
         AlignNone = 0,
         AlignPosX,
         AlignPosY,
         AlignPosZ,
         AlignNegX,
         AlignNegY,
         AlignNegZ
      };

      /// A large hard coded distance used to test 
      /// object and icon selection.
      static F32 smProjectDistance;

      S32               mDropType;
      bool              mBoundingBoxCollision;
      bool              mObjectMeshCollision;
      bool              mRenderPopupBackground;
      ColorI            mPopupBackgroundColor;
      ColorI            mPopupTextColor;
      StringTableEntry  mSelectHandle;
      StringTableEntry  mDefaultHandle;
      StringTableEntry  mLockedHandle;
      ColorI            mObjectTextColor;
      bool              mObjectsUseBoxCenter;
      ColorI            mObjSelectColor;
      ColorI            mObjMultiSelectColor;
      ColorI            mObjMouseOverSelectColor;
      ColorI            mObjMouseOverColor;
      bool              mShowMousePopupInfo;
      ColorI            mDragRectColor;
      bool              mRenderObjText;
      bool              mRenderObjHandle;
      StringTableEntry  mObjTextFormat;
      ColorI            mFaceSelectColor;
      bool              mRenderSelectionBox;
      ColorI            mSelectionBoxColor;
      bool              mSelectionLocked;
      bool              mPerformedDragCopy;
      bool              mDragGridSnapToggle;       ///< Grid snap setting temporarily toggled during drag.
      bool              mToggleIgnoreList;
      bool              mNoMouseDrag;
      bool              mDropAtBounds;
      F32               mDropBelowCameraOffset;
      F32               mDropAtScreenCenterScalar;
      F32               mDropAtScreenCenterMax;

      bool              mGridSnap;
      bool              mStickToGround;
      bool              mStuckToGround;            ///< Selection is stuck to the ground
      AlignmentType     mTerrainSnapAlignment;     ///< How does the stickied object align to the terrain

      bool              mSoftSnap;                 ///< Allow soft snapping all of the time
      bool              mSoftSnapActivated;        ///< Soft snap has been activated by the user and allowed by the current rules
      bool              mSoftSnapIsStuck;          ///< Are we snapping?
      AlignmentType     mSoftSnapAlignment;        ///< How does the snapped object align to the snapped surface
      bool              mSoftSnapRender;           ///< Render the soft snapping bounds
      bool              mSoftSnapRenderTriangle;   ///< Render the soft snapped triangle
      Triangle          mSoftSnapTriangle;         ///< The triangle we are snapping to
      bool              mSoftSnapSizeByBounds;     ///< Use the selection bounds for the size
      F32               mSoftSnapSize;             ///< If not the selection bounds, use this size
      Box3F             mSoftSnapBounds;           ///< The actual bounds used for the soft snap
      Box3F             mSoftSnapPreBounds;        ///< The bounds prior to any soft snapping (used when rendering the snap bounds)
      F32               mSoftSnapBackfaceTolerance;   ///< Fraction of mSoftSnapSize for backface polys to have an influence

      bool              mSoftSnapDebugRender;      ///< Activates debug rendering
      Point3F           mSoftSnapDebugPoint;       ///< The point we're attempting to snap to
      Triangle          mSoftSnapDebugSnapTri;     ///< The triangle we are snapping to
      Vector<Triangle>  mSoftSnapDebugTriangles;   ///< The triangles that are considered for snapping
      
   protected:

      S32 mCurrentCursor;
      void setCursor(U32 cursor);
      void get3DCursor(GuiCursor *&cursor, bool &visible, const Gui3DMouseEvent &event);

   public:

      WorldEditor();
      ~WorldEditor();

      void setDirty() { mIsDirty = true; }

      // SimObject
      virtual bool onAdd();
      virtual void onEditorEnable();

      // EditTSCtrl
      void on3DMouseMove(const Gui3DMouseEvent & event);
      void on3DMouseDown(const Gui3DMouseEvent & event);
      void on3DMouseUp(const Gui3DMouseEvent & event);
      void on3DMouseDragged(const Gui3DMouseEvent & event);
      void on3DMouseEnter(const Gui3DMouseEvent & event);
      void on3DMouseLeave(const Gui3DMouseEvent & event);
      void on3DRightMouseDown(const Gui3DMouseEvent & event);
      void on3DRightMouseUp(const Gui3DMouseEvent & event);

      void updateGuiInfo();

      //
      void renderScene(const RectI & updateRect);

      static void initPersistFields();

      DECLARE_CONOBJECT(WorldEditor);

	  static Signal<void(WorldEditor*)> smRenderSceneSignal;
};

typedef WorldEditor::DropType WorldEditorDropType;
typedef WorldEditor::AlignmentType WorldEditorAlignmentType;

DefineEnumType( WorldEditorDropType );
DefineEnumType( WorldEditorAlignmentType );

#endif // _WORLDEDITOR_H_

