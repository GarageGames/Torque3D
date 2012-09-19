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

#ifndef _GIZMO_H_
#define _GIZMO_H_

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

#ifndef _MMATRIX_H_
#include "math/mMatrix.h"
#endif

#ifndef _COLOR_H_
#include "core/color.h"
#endif

#ifndef _GUITYPES_H_
#include "gui/core/guiTypes.h"
#endif

#ifndef _MATHUTILS_H_
#include "math/mathUtils.h"
#endif

#ifndef _DYNAMIC_CONSOLETYPES_H_
#include "console/dynamicTypes.h"
#endif


enum GizmoMode
{
   NoneMode = 0,
   MoveMode,    // 1
   RotateMode,  // 2
   ScaleMode,   // 3
   ModeEnumCount 
};

enum GizmoAlignment
{
   World = 0,
   Object,
   AlignEnumCount
};

DefineEnumType( GizmoMode );
DefineEnumType( GizmoAlignment );


//
class GizmoProfile : public SimObject
{
   typedef SimObject Parent;

public:

   GizmoProfile();
   virtual ~GizmoProfile() {}

   DECLARE_CONOBJECT( GizmoProfile );

   virtual bool onAdd();

   static void initPersistFields();
   static void consoleInit();

   /// Set flags to default values.
   void restoreDefaultState();

   // Data Fields

   GizmoMode mode;
   GizmoAlignment alignment;

   F32 rotateScalar;
   F32 scaleScalar;
   U32 screenLen;
   ColorI axisColors[3];
   ColorI activeColor;   
   ColorI inActiveColor;
   ColorI centroidColor;
   ColorI centroidHighlightColor;
   Resource<GFont> font;

   bool snapToGrid;
   F32 scaleSnap;
   bool allowSnapScale;
   F32 rotationSnap;
   bool allowSnapRotations;

   bool renderWhenUsed;
   bool renderInfoText;

   Point3F gridSize;
   bool renderPlane;
   bool renderPlaneHashes;
   ColorI gridColor;
   F32 planeDim;   
   bool renderSolid;

   /// Whether to render a transparent grid overlay when using the move gizmo.
   bool renderMoveGrid;

   enum Flags {
      CanRotate         = 1 << 0, // 0
      CanRotateX        = 1 << 1,
      CanRotateY        = 1 << 2,
      CanRotateZ        = 1 << 3,
      CanRotateScreen   = 1 << 4,
      CanRotateUniform  = 1 << 5,
      CanScale          = 1 << 6,
      CanScaleX         = 1 << 7,
      CanScaleY         = 1 << 8,
      CanScaleZ         = 1 << 9,
      CanScaleUniform   = 1 << 10,
      CanTranslate      = 1 << 11,
      CanTranslateX     = 1 << 12, 
      CanTranslateY     = 1 << 13, 
      CanTranslateZ     = 1 << 14,
      CanTranslateUniform = 1 << 15,
      PlanarHandlesOn   = 1 << 16
   };

   S32 flags;

   bool hideDisabledAxes;

   bool allAxesScaleUniform;
};


// This class contains code for rendering and manipulating a 3D gizmo, it
// is usually used as a helper within a TSEdit-derived control.
//
// The Gizmo has a MatrixF transform and Point3F scale on which it will
// operate by passing it Gui3DMouseEvent(s).
//
// The idea is to set the Gizmo transform/scale to that of another 3D object 
// which is being manipulated, pass mouse events into the Gizmo, read the
// new transform/scale out, and set it to onto the object.
// And of course the Gizmo can be rendered.
//
// Gizmo derives from SimObject only because this allows its properties
// to be initialized directly from script via fields.

class Gizmo : public SimObject
{
   typedef SimObject Parent;

   friend class WorldEditor;

public:   

   enum Selection {
      None     = -1,
      Axis_X   = 0,
      Axis_Y   = 1,
      Axis_Z   = 2,
      Plane_XY = 3,  // Normal = Axis_Z
      Plane_XZ = 4,  // Normal = Axis_Y
      Plane_YZ = 5,  // Normal = Axis_X
      Centroid = 6,
      Custom1  = 7,  // screen-aligned rotation
      Custom2  = 8
   };  

   Gizmo();
   ~Gizmo();

   DECLARE_CONOBJECT( Gizmo );

   // SimObject
   bool onAdd();
   void onRemove();
   static void initPersistFields();

   // Mutators
   void set( const MatrixF &objMat, const Point3F &worldPos, const Point3F &objScale ); 
   void setProfile( GizmoProfile *profile ) 
   { 
      AssertFatal( profile != NULL, "NULL passed to Gizmo::setProfile - Gizmo must always have a profile!" );
      mProfile = profile; 
   }

   // Accessors
   
   GizmoProfile*  getProfile()               { return mProfile; }

   GizmoMode      getMode() const            { return mCurrentMode; }

   GizmoAlignment getAlignment() const       { return mCurrentAlignment; }

   /// Returns current object to world transform of the object being manipulated.
   const MatrixF& getTransform() const       { return mCurrentTransform; }
   
   Point3F        getPosition() const        { return mCurrentTransform.getPosition(); }   
   
   const Point3F& getScale() const           { return mScale; }
   
   
   // Returns change in position in last call to on3DMouseDragged.
   const Point3F& getOffset() const          { return mDeltaPos; }
   
   // Returns change is position since on3DMouseDown.
   const Point3F& getTotalOffset() const     { return mDeltaTotalPos; }
   
   const Point3F& getDeltaScale() const      { return mDeltaScale; }
   
   const Point3F& getDeltaTotalScale() const { return mDeltaTotalScale; }
   
   const Point3F& getDeltaRot() const        { return mDeltaRot; }

   const Point3F& getDeltaTotalRot() const   { return mDeltaTotalRot; }

   /// Set whether to render the grid plane.
   void setGridPlaneEnabled( bool value ) { mGridPlaneEnabled = value; }

   /// Set whether to a transparent grid overlay when using the move gizmo.
   void setMoveGridEnabled( bool value ) { mMoveGridEnabled = value; }

   /// Set the size of the move grid along one dimension.  The total size of the
   /// move grid is @a value * @a value.
   void setMoveGridSize( F32 value ) { mMoveGridSize = value; }

   /// Set the spacing between grid lines on the move grid.
   void setMoveGridSpacing( F32 value ) { mMoveGridSpacing = value; }

   // Gizmo Interface methods...

   // Set the current highlight mode on the gizmo's centroid handle
   void setCentroidHandleHighlight( bool state ) { mHighlightCentroidHandle = state; }

   // Must be called before on3DMouseDragged to save state
   void on3DMouseDown( const Gui3DMouseEvent &event );

   // So Gizmo knows the current mouse button state.
   void on3DMouseUp( const Gui3DMouseEvent &event );
   
   // Test Gizmo for collisions and set the Gizmo Selection (the part under the cursor)
   void on3DMouseMove( const Gui3DMouseEvent &event );
   
   // Make changes to the Gizmo transform/scale (depending on mode)
   void on3DMouseDragged( const Gui3DMouseEvent &event );

   // Returns an enum describing the part of the Gizmo that is Selected
   // ( under the cursor ). This should be called AFTER calling onMouseMove
   // or collideAxisGizmo
   //
   // -1 None
   // 0  Axis_X
   // 1  Axis_Y
   // 2  Axis_Z
   // 3  Plane_XY
   // 4  Plane_XZ
   // 5  Plane_YZ
   Selection getSelection();
   void setSelection( Selection sel ) { mSelectionIdx = sel; }

   // Returns the object space vector corresponding to a Selection.
   Point3F selectionToAxisVector( Selection axis ); 

   // These provide the user an easy way to check if the Gizmo's transform
   // or scale have changed by calling markClean prior to calling
   // on3DMouseDragged, and calling isDirty after.   
   bool isDirty() { return mDirty; }
   void markClean() { mDirty = false; } 

   // Renders the 3D Gizmo in the scene, GFX must be setup for proper
   // 3D rendering before calling this!
   // Calling this will change the GFXStateBlock!
   void renderGizmo( const MatrixF &cameraTransform, F32 camerFOV = 1.5f );

   // Renders text associated with the Gizmo, GFX must be setup for proper
   // 2D rendering before calling this!
   // Calling this will change the GFXStateBlock!
   void renderText( const RectI &viewPort, const MatrixF &modelView, const MatrixF &projection );

   // Returns true if the mouse event collides with any part of the Gizmo
   // and sets the Gizmo's current Selection.
   // You can call this or on3DMouseMove, they are identical   
   bool collideAxisGizmo( const Gui3DMouseEvent & event );

protected:

   void _calcAxisInfo();
   void _setStateBlock();
   void _renderPrimaryAxis();
   void _renderAxisArrows();
   void _renderAxisBoxes();
   void _renderAxisCircles();
   void _renderAxisText();   
   void _renderPlane();
   Point3F _snapPoint( const Point3F &pnt ) const;
   F32 _snapFloat( const F32 &val, const F32 &snap ) const;
   GizmoAlignment _filteredAlignment();
   void _updateState( bool collideGizmo = true );
   void _updateEnabledAxices();

   F32 _getProjectionLength( F32 dist ) const
   {
      if( GFX->isFrustumOrtho() )
         return mLastCameraFOV * dist * 0.002f;
      else
      {
         Point3F dir = mOrigin - mCameraPos;
         return ( dist * dir.len() ) / mLastWorldToScreenScale.y;
      }
   }

protected:

   GizmoProfile *mProfile;

   MatrixF mObjectMat;
   MatrixF mObjectMatInv;
   MatrixF mTransform;
   MatrixF mCurrentTransform;
   MatrixF mSavedTransform;
   
   GizmoAlignment mCurrentAlignment;
   GizmoMode mCurrentMode;

   MatrixF mCameraMat;
   Point3F mCameraPos;

   Point3F mScale;
   Point3F mSavedScale;
   Point3F mDeltaScale;
   Point3F mDeltaTotalScale;
   Point3F mLastScale;
	Point3F mScaleInfluence;
 
   EulerF mRot;
   EulerF mSavedRot;
   EulerF mDeltaRot;
   EulerF mDeltaTotalRot;
   F32 mDeltaAngle;
   F32 mLastAngle;
   Point2I mMouseDownPos;
   Point3F mMouseDownProjPnt;
   Point3F mDeltaPos;   
   Point3F mDeltaTotalPos;
   Point3F mProjPnt;
   Point3F mOrigin;
   Point3F mProjAxisVector[3];
   F32 mProjLen;
   S32 mSelectionIdx;
   bool mDirty;
   Gui3DMouseEvent mLastMouseEvent;
   GFXStateBlockRef mStateBlock;
   GFXStateBlockRef mSolidStateBlock;

   PlaneF mMouseCollidePlane;
   MathUtils::Line mMouseCollideLine;

   bool mMouseDown;

   F32 mSign;
   
   /// If false, don't render the grid plane even if it is enabled in the profile.
   bool mGridPlaneEnabled;

   /// If false, don't render a transparent grid overlay when using the move gizmo.
   bool mMoveGridEnabled;

   /// Size of the move grid along one dimension.
   F32 mMoveGridSize;

   /// Spacing between grid lines on the move grid.
   U32 mMoveGridSpacing;

   bool mAxisEnabled[3];
   bool mUniformHandleEnabled;
   bool mScreenRotateHandleEnabled;
      
   // Used to override rendering of handles.
   bool mHighlightCentroidHandle;   
   bool mHighlightAll;

   // Initialized in renderGizmo and saved for later use when projecting
   // to screen space for selection testing.
   MatrixF mLastWorldMat;
   MatrixF mLastProjMat;
   RectI mLastViewport;
   Point2F mLastWorldToScreenScale;
   F32 mLastCameraFOV;

   // Screenspace cursor collision information used in rotation mode.
   Point3F mElipseCursorCollidePntSS;
   Point3F mElipseCursorCollideVecSS;

   /// A large hard coded distance used to test 
   /// gizmo axis selection.
   static F32 smProjectDistance;
};

#endif // _GIZMO_H_