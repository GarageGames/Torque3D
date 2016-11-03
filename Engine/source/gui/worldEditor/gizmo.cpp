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

#include "platform/platform.h"
#include "gui/worldEditor/gizmo.h"

#include "console/consoleTypes.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/util/gfxFrustumSaver.h"
#include "gfx/gfxDrawUtil.h"
#include "T3D/gameBase/gameConnection.h"
//#include "math/mathUtils.h"

using namespace MathUtils;


// Developer Notes:

// How to... Calculate the SelectionAxis index representing the normal 
// of a plane, given a SelectionPlane index
// normal = axisVector[2 - (planeIdx - 3 )];  

// How to... Get the two AxisVectors of a selected plane
// vec0 = mProjAxisVector[mAxisGizmoPlanarVectors[mSelectionIdx-3][0]];
// vec1 = mProjAxisVector[mAxisGizmoPlanarVectors[mSelectionIdx-3][1]]; 

ImplementEnumType(GizmoAlignment,
   "Whether the gizmo should be aligned with the world, or with the object.\n"
   "@internal\n\n")
   { World, "World", "Align the gizmo with the world.\n" },
   { Object, "Object", "Align the gizmo with the object.\n" }      
EndImplementEnumType;

ImplementEnumType( GizmoMode,
   "@internal" )
   { NoneMode, "None" },
   { MoveMode, "Move" },      
   { RotateMode, "Rotate" },
   { ScaleMode, "Scale" }
EndImplementEnumType;


//-------------------------------------------------------------------------
// Unnamed namespace for static data
//-------------------------------------------------------------------------

namespace {

   static S32 sgAxisRemap[3][3] = {
      {0, 1, 2},
      {2, 0, 1},
      {1, 2, 0},
   };

   static VectorF sgAxisVectors[3] = {
      VectorF(1.0f,0.0f,0.0f),
      VectorF(0.0f,1.0f,0.0f),
      VectorF(0.0f,0.0f,1.0f)
   };

   static U32 sgPlanarVectors[3][2] = {
      { 0, 1 }, // XY
      { 0, 2 }, // XZ
      { 1, 2 }  // YZ
   };
      
   static Point3F sgBoxPnts[] = {
      Point3F(0.0f,0.0f,0.0f),
      Point3F(0.0f,0.0f,1.0f),
      Point3F(0.0f,1.0f,0.0f),
      Point3F(0.0f,1.0f,1.0f),
      Point3F(1.0f,0.0f,0.0f),
      Point3F(1.0f,0.0f,1.0f),
      Point3F(1.0f,1.0f,0.0f),
      Point3F(1.0f,1.0f,1.0f)
   };

   static U32 sgBoxVerts[][4] = {
      {0,2,3,1},     // -x
      {7,6,4,5},     // +x
      {0,1,5,4},     // -y
      {3,2,6,7},     // +y
      {0,4,6,2},     // -z
      {3,7,5,1}      // +z
   };

   static Point3F sgBoxNormals[] = {
      Point3F(-1.0f, 0.0f, 0.0f),
      Point3F( 1.0f, 0.0f, 0.0f),
      Point3F( 0.0f,-1.0f, 0.0f),
      Point3F( 0.0f, 1.0f, 0.0f),
      Point3F( 0.0f, 0.0f,-1.0f),
      Point3F( 0.0f, 0.0f, 1.0f)
   };

   static Point3F sgConePnts[] = {
      Point3F(0.0f, 0.0f, 0.0f),
      Point3F(-1.0f, 0.0f, -0.25f),
      Point3F(-1.0f, -0.217f, -0.125f),
      Point3F(-1.0f, -0.217f, 0.125f),
      Point3F(-1.0f, 0.0f, 0.25f),
      Point3F(-1.0f, 0.217f, 0.125f),
      Point3F(-1.0f, 0.217f, -0.125f),
      Point3F(-1.0f, 0.0f, 0.0f)
   };

   static U32 sgConeVerts[][3] = {
      {0, 2, 1},
      {0, 3, 2},
      {0, 4, 3},
      {0, 5, 4},
      {0, 6, 5},
      {0, 1, 6},
      {7, 1, 6}, // Base
      {7, 6, 5},
      {7, 5, 4},
      {7, 4, 3},
      {7, 3, 2},
      {7, 2, 1}
   };

   static Point3F sgCenterBoxPnts[] = {
      Point3F(-0.5f, -0.5f, -0.5f),
      Point3F(-0.5f, -0.5f,  0.5f),
      Point3F(-0.5f,  0.5f, -0.5f),
      Point3F(-0.5f,  0.5f,  0.5f),
      Point3F( 0.5f, -0.5f, -0.5f),
      Point3F( 0.5f, -0.5f,  0.5f),
      Point3F( 0.5f,  0.5f, -0.5f),
      Point3F( 0.5f,  0.5f,  0.5f)
   };

   static Point3F sgCirclePnts[] = {
      Point3F(0.0f,  0.0f,   -0.5f),
      Point3F(0.0f,  0.354f, -0.354f),
      Point3F(0.0f,  0.5f,    0),
      Point3F(0.0f,  0.354f,  0.354f),
      Point3F(0.0f,  0.0f,    0.5f),
      Point3F(0.0f, -0.354f,  0.354f),
      Point3F(0.0f, -0.5f,    0),
      Point3F(0.0f, -0.354f, -0.354f),
      Point3F(0.0f,  0.0f,   -0.5f),
   };
}


//-------------------------------------------------------------------------
// GizmoProfile Class
//-------------------------------------------------------------------------

GizmoProfile::GizmoProfile()
{
   mode = MoveMode;
   alignment = World;
   screenLen = 100;
   renderWhenUsed = false;
   renderInfoText = true;
   renderPlane = true;
   renderPlaneHashes = true;
   renderSolid = false;
   renderMoveGrid = true;
   gridColor.set(255,255,255,20);
   planeDim = 500.0f;   

   gridSize.set(10,10,10);
   snapToGrid = false;
   allowSnapRotations = true;
   rotationSnap = 15.0f;
   allowSnapScale = true;
   scaleSnap = 0.1f;

   rotateScalar = 0.8f;
   scaleScalar = 0.8f;

   axisColors[0].set( 255, 0, 0 );   
   axisColors[1].set( 0, 255, 0 );   
   axisColors[2].set( 0, 0, 255 );      

   activeColor.set( 237, 219, 0 );
   inActiveColor.set( 170, 170, 170 );

   centroidColor.set( 255, 255, 255 );
   centroidHighlightColor.set( 255, 0, 255 );

   restoreDefaultState();
}

void GizmoProfile::restoreDefaultState()
{   
   flags = U32_MAX;  
   flags &= ~CanRotateUniform;
   allAxesScaleUniform = false;
}

IMPLEMENT_CONOBJECT( GizmoProfile );

ConsoleDocClass( GizmoProfile,
				"@brief This class contains behavior and rendering properties used "
				"by the Gizmo class\n\n"
				"Not intended for game development, for editors or internal use only.\n\n "
				"@internal");

bool GizmoProfile::onAdd()
{   
   if ( !Parent::onAdd() )
      return false;

   const char* fontCacheDirectory = Con::getVariable("$GUI::fontCacheDirectory");
   font = GFont::create( "Arial", 10, fontCacheDirectory, TGE_ANSI_CHARSET);
   if ( !font )
   {
      Con::errorf( "GizmoProfile::onAdd - failed to load font!" );
      return false;
   }

   return true;
}

void GizmoProfile::initPersistFields()
{
   addField( "alignment",           TYPEID< GizmoAlignment >(),   Offset(alignment, GizmoProfile ) );
   addField( "mode",                TYPEID< GizmoMode >(),   Offset(mode, GizmoProfile ) );

   addField( "snapToGrid",          TypeBool,   Offset(snapToGrid, GizmoProfile) );
   addField( "allowSnapRotations",  TypeBool,   Offset(allowSnapRotations, GizmoProfile) );
   addField( "rotationSnap",        TypeF32,    Offset(rotationSnap, GizmoProfile) );
   addField( "allowSnapScale",      TypeBool,   Offset(allowSnapScale, GizmoProfile) );
   addField( "scaleSnap",           TypeF32,    Offset(scaleSnap, GizmoProfile) );
   addField( "renderWhenUsed",      TypeBool,   Offset(renderWhenUsed, GizmoProfile) );
   addField( "renderInfoText",      TypeBool,   Offset(renderInfoText, GizmoProfile) );
   addField( "renderPlane",         TypeBool,   Offset(renderPlane, GizmoProfile) );
   addField( "renderPlaneHashes",   TypeBool,   Offset(renderPlaneHashes, GizmoProfile) );
   addField( "renderSolid",         TypeBool,   Offset(renderSolid, GizmoProfile) );
   addField( "renderMoveGrid",      TypeBool,   Offset( renderMoveGrid, GizmoProfile ) );
   addField( "gridColor",           TypeColorI, Offset(gridColor, GizmoProfile) );
   addField( "planeDim",            TypeF32,    Offset(planeDim, GizmoProfile) );
   addField( "gridSize",            TypePoint3F, Offset(gridSize, GizmoProfile) );
   addField( "screenLength",        TypeS32,    Offset(screenLen, GizmoProfile) );
   addField( "rotateScalar",        TypeF32,    Offset(rotateScalar, GizmoProfile) );
   addField( "scaleScalar",         TypeF32,    Offset(scaleScalar, GizmoProfile) );   
   addField( "flags",               TypeS32,    Offset(flags, GizmoProfile) );
}

void GizmoProfile::consoleInit()
{
   Parent::consoleInit();

   Con::setIntVariable( "$GizmoFlag::CanRotate", CanRotate );
   Con::setIntVariable( "$GizmoFlag::CanRotateX", CanRotateX );
   Con::setIntVariable( "$GizmoFlag::CanRotateY", CanRotateY );
   Con::setIntVariable( "$GizmoFlag::CanRotateZ", CanRotateZ );
   Con::setIntVariable( "$GizmoFlag::CanRotateScreen", CanRotateScreen );
   Con::setIntVariable( "$GizmoFlag::CanRotateUniform", CanRotateUniform );
   Con::setIntVariable( "$GizmoFlag::CanScale", CanScale );
   Con::setIntVariable( "$GizmoFlag::CanScaleX", CanScaleX );
   Con::setIntVariable( "$GizmoFlag::CanScaleY", CanScaleY );
   Con::setIntVariable( "$GizmoFlag::CanScaleZ", CanScaleZ );
   Con::setIntVariable( "$GizmoFlag::CanScaleUniform", CanScaleUniform );
   Con::setIntVariable( "$GizmoFlag::CanTranslate", CanTranslate );
   Con::setIntVariable( "$GizmoFlag::CanTranslateX", CanTranslateX );
   Con::setIntVariable( "$GizmoFlag::CanTranslateY", CanTranslateY );
   Con::setIntVariable( "$GizmoFlag::CanTranslateZ", CanTranslateZ );
   Con::setIntVariable( "$GizmoFlag::CanTranslateUniform", CanTranslateUniform );
   Con::setIntVariable( "$GizmoFlag::PlanarHandlesOn", PlanarHandlesOn );   
}

//-------------------------------------------------------------------------
// Gizmo Class
//-------------------------------------------------------------------------

F32 Gizmo::smProjectDistance = 20000.0f;

Gizmo::Gizmo()
: mProfile( NULL ),
  mSelectionIdx( -1 ),
  mCameraMat( true ),
  mTransform( true ),
  mObjectMat( true ),
  mObjectMatInv( true ),
  mCurrentTransform( true ),
  mSavedTransform( true ),
  mSavedScale( 0,0,0 ),
  mDeltaScale( 0,0,0 ),
  mDeltaRot( 0,0,0 ),
  mDeltaPos( 0,0,0 ),
  mDeltaTotalPos( 0,0,0 ),
  mDeltaTotalRot( 0,0,0 ),
  mDeltaTotalScale( 0,0,0 ),
  mCurrentAlignment( World ),
  mCurrentMode( MoveMode ),
  mDirty( false ),
  mMouseDownPos( -1,-1 ),
  mMouseDown( false ),
  mLastWorldMat( true ),
  mLastProjMat( true ),
  mLastViewport( 0, 0, 10, 10 ),
  mLastCameraFOV( 1.f ),
  mElipseCursorCollideVecSS( 1.0f, 0.0f, 0.0f ),
  mElipseCursorCollidePntSS( 0.0f, 0.0f, 0.0f ),
  mHighlightCentroidHandle( false ),
  mHighlightAll( false ),
  mGridPlaneEnabled( true ),
  mMoveGridEnabled( true ),
  mMoveGridSize( 20.f ),
  mMoveGridSpacing( 1.f )
{   
   mUniformHandleEnabled = true;   
   mAxisEnabled[0] = mAxisEnabled[1] = mAxisEnabled[2] = true;
}

Gizmo::~Gizmo()
{
}

IMPLEMENT_CONOBJECT( Gizmo );

ConsoleDocClass( Gizmo,
				"@brief This class contains code for rendering and manipulating a 3D gizmo\n\n"
				"It is usually used as a helper within a TSEdit-derived control. "
				"Not intended for game development, for editors or internal use only.\n\n "
				"@internal");

// SimObject Methods...

bool Gizmo::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   if ( !mProfile )
      return false;

   mCurrentAlignment = mProfile->alignment;
   mCurrentMode = mProfile->mode;

   return true;
}

void Gizmo::onRemove()
{
   Parent::onRemove();
}

void Gizmo::initPersistFields()
{
   Parent::initPersistFields();

   //addField( "profile",)  
}

// Gizmo Accessors and Mutators...

void Gizmo::set( const MatrixF &objMat, const Point3F &worldPos, const Point3F &objScale )
{   
   if ( mMouseDown )
      return;

   mCurrentAlignment = _filteredAlignment();   

   if ( mCurrentAlignment == World )
   {
      mTransform.identity();
      mTransform.setPosition( worldPos );
		mScale = objScale;
      mObjectMat = objMat;         
   }
   else
   {
      mTransform = objMat;
      mTransform.setPosition( worldPos );
      mScale = objScale;
      mObjectMat.identity();
   }

   mCurrentTransform = objMat;
   mObjectMat.invertTo( &mObjectMatInv );   
}

Gizmo::Selection Gizmo::getSelection()
{
   if ( mProfile->mode == NoneMode )
      return None;

   return (Selection)mSelectionIdx;
}

VectorF Gizmo::selectionToAxisVector( Selection axis )
{
   if ( axis < Axis_X || axis > Axis_Z )
      return VectorF(0,0,0);

   return sgAxisVectors[(U32)axis];
}

bool Gizmo::collideAxisGizmo( const Gui3DMouseEvent & event )
{   
   if ( mProfile->mode == NoneMode )
      return false;

   _calcAxisInfo();

   // Early out if we are in a mode that is disabled.
   if ( mProfile->mode == RotateMode && !(mProfile->flags & GizmoProfile::CanRotate ) )
      return false;
   if ( mProfile->mode == MoveMode && !(mProfile->flags & GizmoProfile::CanTranslate ) )
      return false;
   if ( mProfile->mode == ScaleMode && !(mProfile->flags & GizmoProfile::CanScale ) )
      return false;
      
   VectorF camPos;
   if( GFX->isFrustumOrtho() )
      camPos = event.pos;
   else
      camPos = mCameraPos;

   VectorF toGizmoVec;

   // get the projected size...
   
   toGizmoVec = mOrigin - mCameraPos;   
   toGizmoVec.normalizeSafe();
   
   PlaneF clipPlane( mOrigin, toGizmoVec );

   mSelectionIdx = -1;
   Point3F end = camPos + event.vec * smProjectDistance;

   if ( mProfile->mode == RotateMode )
   {
      const Point3F mousePntSS( (F32)event.mousePoint.x, (F32)event.mousePoint.y, 0.0f );
      const F32 axisCollisionThresholdSS = 10.0f;


      Point3F originSS;
      MathUtils::mProjectWorldToScreen( mOrigin, &originSS, mLastViewport, mLastWorldMat, mLastProjMat );
      originSS.z = 0.0f;

      const F32 originDistSS = mAbs( ( mousePntSS - originSS ).len() );

      // Check for camera facing axis rotation handle collision.
      if ( mScreenRotateHandleEnabled )
      {
         const F32 distSS = mAbs( ( (F32)mProfile->screenLen * 0.7f ) -  originDistSS );

         if ( distSS < axisCollisionThresholdSS )
         {
            mSelectionIdx = Custom1;

            Point3F normal = mousePntSS - originSS;
            normal.normalizeSafe();            
            Point3F tangent = mCross( -normal, Point3F(0,0,1) );
            tangent.normalizeSafe();

            mElipseCursorCollidePntSS = mousePntSS;
            mElipseCursorCollideVecSS = tangent;
            mElipseCursorCollideVecSS.z = 0.0f;
            mElipseCursorCollideVecSS.normalizeSafe();

            return true;
         }
         
      }
      
      // Check for x/y/z axis ellipse handle collision.
      // We do this as a screen-space pixel distance test between
      // the cursor position and the ellipse handle projected to the screen
      // as individual segments.
      {         
         const F32 ellipseRadiusWS = mProjLen * 0.5f;
         const U32 segments = 40;
         const F32 stepRadians = mDegToRad(360.0f) / segments;
         
         U32 x,y,z;
         F32 ang0, ang1, distSS;
         Point3F temp, pnt0, pnt1, closestPntSS;
         bool valid0, valid1;

         MatrixF worldToGizmo = mTransform;
         worldToGizmo.inverse();
         PlaneF clipPlaneGS; // Clip plane in gizmo space.
         mTransformPlane( worldToGizmo, Point3F(1,1,1), clipPlane, &clipPlaneGS );
 
         for ( U32 i = 0; i < 3; i++ )
         {                  
            if ( !mAxisEnabled[i] )
               continue;
            
            x = sgAxisRemap[i][0];
            y = sgAxisRemap[i][1];
            z = sgAxisRemap[i][2];

            for ( U32 j = 1; j <= segments; j++ )
            {
               ang0 = (j-1) * stepRadians;
               ang1 = j * stepRadians;
               
               temp.x = 0.0f;
               temp.y = mCos(ang0) * ellipseRadiusWS;
               temp.z = mSin(ang0) * ellipseRadiusWS;
               pnt0.set( temp[x], temp[y], temp[z] );

               temp.x = 0.0f;
               temp.y = mCos(ang1) * ellipseRadiusWS;
               temp.z = mSin(ang1) * ellipseRadiusWS;
               pnt1.set( temp[x], temp[y], temp[z] );

               valid0 = ( clipPlaneGS.whichSide(pnt0) == PlaneF::Back );
               valid1 = ( clipPlaneGS.whichSide(pnt1) == PlaneF::Back );

               if ( !valid0 || !valid1 )
                  continue;

               // Transform points from gizmo space to world space.

               mTransform.mulP( pnt0 );
               mTransform.mulP( pnt1 );

               // Transform points from gizmo space to screen space.

               valid0 = MathUtils::mProjectWorldToScreen( pnt0, &pnt0, mLastViewport, mLastWorldMat, mLastProjMat );
               valid1 = MathUtils::mProjectWorldToScreen( pnt1, &pnt1, mLastViewport, mLastWorldMat, mLastProjMat );

               // Get distance from the cursor.

               closestPntSS = MathUtils::mClosestPointOnSegment( Point3F( pnt0.x, pnt0.y, 0.0f ), Point3F( pnt1.x, pnt1.y, 0.0f ), mousePntSS );
               distSS = ( closestPntSS - mousePntSS ).len();

               if ( distSS < axisCollisionThresholdSS )
               {
                  mSelectionIdx = i;
                  mElipseCursorCollidePntSS = mousePntSS;
                  mElipseCursorCollideVecSS = pnt1 - pnt0;
                  mElipseCursorCollideVecSS.z = 0.0f;
                  mElipseCursorCollideVecSS.normalizeSafe();

                  return true;
               }
            }
         }
      }

      // Check for sphere surface collision               
      if ( originDistSS <= (F32)mProfile->screenLen * 0.5f )
      {
         // If this style manipulation is desired it must also be implemented in onMouseDragged.
         //mSelectionIdx = Custom2;
         //return true;
      }
   }

   // Check if we've hit the uniform scale handle...
   if ( mUniformHandleEnabled )
   {
      F32 tipScale = mProjLen * 0.1f;
      Point3F sp( tipScale, tipScale, tipScale );

      Point3F min = mOrigin - sp;
      Point3F max = mOrigin + sp;
      Box3F uhandle(min, max);
      if ( uhandle.collideLine( camPos, end ) )
      {
         mSelectionIdx = Centroid;
         return true;
      }
   }

   // Check if we've hit the planar handles...
   if ( ( mProfile->mode == MoveMode || mProfile->mode == ScaleMode ) &&
        ( mProfile->flags & GizmoProfile::PlanarHandlesOn ) )
   {
      for ( U32 i = 0; i < 3; i++ )
      {
         Point3F p1 = mProjAxisVector[sgPlanarVectors[i][0]];
         Point3F p2 = mProjAxisVector[sgPlanarVectors[i][1]];
         VectorF normal;
         mCross(p1, p2, &normal);

         if(normal.isZero())
            continue;

         PlaneF plane(mOrigin, normal);

         p1 *= mProjLen * 0.5f;
         p2 *= mProjLen * 0.5f;

         F32 scale = 0.5f;

         Point3F poly [] = {
            Point3F(mOrigin + p1 + p2 * scale),
            Point3F(mOrigin + p1 + p2),
            Point3F(mOrigin + p1 * scale + p2),
            Point3F(mOrigin + (p1 + p2) * scale)
         };

         Point3F end = camPos + event.vec * smProjectDistance;
         F32 t = plane.intersect(camPos, end);
         if ( t >= 0 && t <= 1 )
         {
            Point3F pos;
            pos.interpolate(camPos, end, t);

            // check if inside our 'poly' of this axisIdx vector...
            bool inside = true;
            for(U32 j = 0; inside && (j < 4); j++)
            {
               U32 k = (j+1) % 4;
               VectorF vec1 = poly[k] - poly[j];
               VectorF vec2 = pos - poly[k];

               if(mDot(vec1, vec2) > 0.f)
                  inside = false;
            }

            //
            if ( inside )
            {
               mSelectionIdx = i+3;
               //mAxisGizmoSelPlane = plane;
               //mAxisGizmoSelPlaneIndex = i;
               //mAxisGizmoSelPlanePoint = pos;
               //mAxisGizmoSelStart = camPos;
               return true;
            }
         }
      }
   }

   if ( mCurrentMode == RotateMode )
      return false;

   // Check if we've hit an axis...
   for ( U32 i = 0; i < 3; i++ )
   {
      if ( !mAxisEnabled[i] )
         continue;

      VectorF up, normal;
      mCross(toGizmoVec, mProjAxisVector[i], &up);
      mCross(up, mProjAxisVector[i], &normal);

      if ( normal.isZero() )
         continue;

      PlaneF plane( mOrigin, normal );

      // width of the axisIdx poly is 1/10 the run
      Point3F a = up * mProjLen / 10;
      Point3F b = mProjAxisVector[i] * mProjLen;

      Point3F poly [] = {
         Point3F(mOrigin + a),
         Point3F(mOrigin + a + b),
         Point3F(mOrigin - a + b),
         Point3F(mOrigin - a)   
      };

      F32 t = plane.intersect(camPos, end);
      if ( t >= 0 && t <= 1 )
      {
         Point3F pos;
         pos.interpolate(camPos, end, t);

         // check if inside our 'poly' of this axisIdx vector...
         bool inside = true;
         for ( U32 j = 0; inside && (j < 4); j++ )
         {
            U32 k = (j+1) % 4;
            VectorF vec1 = poly[k] - poly[j];
            VectorF vec2 = pos - poly[k];

            if ( mDot(vec1, vec2) > 0.f )
               inside = false;
         }

         //
         if(inside)
         {
            mSelectionIdx = i;

            return true;
         }
      }
   }   

   return false;
}

void Gizmo::on3DMouseDown( const Gui3DMouseEvent & event )
{
   _updateState();

   mMouseDown = true;
   
   if ( mProfile->mode == NoneMode )
      return;

   // Save the current transforms, need this for some
   // operations that occur on3DMouseDragged.

   mSavedTransform = mTransform;
   mSavedScale = mScale;
   mSavedRot = mTransform.toEuler();

   mMouseDownPos = event.mousePoint;
   mLastAngle = 0.0f;
   mLastScale = mScale;
   mLastMouseEvent = event;
   mSign = 0.0f;

   _calcAxisInfo();

   // Calculate mMouseCollideLine and mMouseDownProjPnt
   // which are used in on3DMouseDragged.

   if ( mProfile->mode == MoveMode || mProfile->mode == ScaleMode )
   {
      if ( mSelectionIdx >= Axis_X && mSelectionIdx <= Axis_Z )
      {
         MathUtils::Line clickLine;
         clickLine.origin = event.pos;
         clickLine.direction = event.vec;

         VectorF objectAxisVector = sgAxisVectors[mSelectionIdx];
         VectorF worldAxisVector = objectAxisVector;
         mTransform.mulV( worldAxisVector );

         MathUtils::Line axisLine;
         axisLine.origin = mTransform.getPosition();
         axisLine.direction = worldAxisVector;

         mMouseCollideLine = axisLine;

         LineSegment segment;
         mShortestSegmentBetweenLines( clickLine, axisLine, &segment );   

         mMouseDownProjPnt = segment.p1;
      }
      else if ( mSelectionIdx >= Plane_XY && mSelectionIdx <= Plane_YZ )
      {
         VectorF objectPlaneNormal = sgAxisVectors[2 - (mSelectionIdx - 3 )];
         VectorF worldPlaneNormal = objectPlaneNormal;
         mTransform.mulV( worldPlaneNormal );

         PlaneF plane( mTransform.getPosition(), worldPlaneNormal );

         mMouseCollidePlane = plane;

         Point3F intersectPnt;
         if ( plane.intersect( event.pos, event.vec, &intersectPnt ) )
         {
            mMouseDownProjPnt = intersectPnt;
         }

         // We also calculate the line to be used later.

         VectorF objectAxisVector(0,0,0);
         objectAxisVector += sgAxisVectors[sgPlanarVectors[mSelectionIdx-3][0]];
         objectAxisVector += sgAxisVectors[sgPlanarVectors[mSelectionIdx-3][1]];
         objectAxisVector.normalize();

         VectorF worldAxisVector = objectAxisVector;
         mTransform.mulV( worldAxisVector );

         MathUtils::Line axisLine;
         axisLine.origin = mTransform.getPosition();
         axisLine.direction = worldAxisVector;

         mMouseCollideLine = axisLine;
      }
      else if ( mSelectionIdx == Centroid )
      {
         VectorF normal;
         mCameraMat.getColumn(1,&normal);
         normal = -normal;
         
         PlaneF plane( mOrigin, normal );

         mMouseCollidePlane = plane;

         Point3F intersectPnt;
         if ( plane.intersect( event.pos, event.vec, &intersectPnt ) )
         {
            mMouseDownProjPnt = intersectPnt;
         }
      }
   }
   else if ( mProfile->mode == RotateMode )
   {
      VectorF camPos;
      if( GFX->isFrustumOrtho() )
         camPos = event.pos;
      else
         camPos = mCameraPos;

      if ( 0 <= mSelectionIdx && mSelectionIdx <= 2 )
      {       
         // Nothing to do, we already have mElipseCursorCollidePntSS
         // and mElipseCursorCollideVecSS set.         
      }
      else if ( mSelectionIdx == Custom1 )
      {
         // Nothing to do, we already have mElipseCursorCollidePntSS
         // and mElipseCursorCollideVecSS set.    
      }
      else if ( mSelectionIdx == Centroid )
      {
         // The Centroid handle for rotation mode is not implemented to do anything.
         // It can be handled by the class making use of the Gizmo.
      }
   }
}

void Gizmo::on3DMouseUp( const Gui3DMouseEvent &event )
{
   _updateState();
   mMouseDown = false;
   mDeltaTotalPos.zero();
   mDeltaTotalScale.zero();
   mDeltaTotalRot.zero();

   // Done with a drag operation, recenter our orientation to the world.
   if ( mCurrentAlignment == World )
   {
      Point3F pos = mTransform.getPosition();
      mTransform.identity();
      mTransform.setPosition( pos );
   }
}

void Gizmo::on3DMouseMove( const Gui3DMouseEvent & event )
{
   _updateState( false );

   if ( mProfile->mode == NoneMode )
      return;

   collideAxisGizmo( event );

   mLastMouseEvent = event;   
}

void Gizmo::on3DMouseDragged( const Gui3DMouseEvent & event )
{   
   _updateState( false );

   if ( !mProfile || mProfile->mode == NoneMode || mSelectionIdx == None )
      return;

   // If we got a dragged event without the mouseDown flag the drag operation
   // must have been canceled by a mode change, ignore further dragged events.
   if ( !mMouseDown )
      return;

	_calcAxisInfo();

   if ( mProfile->mode == MoveMode || mProfile->mode == ScaleMode )
   {      
      Point3F projPnt = mOrigin;

      // Project the mouse position onto the line/plane of manipulation...

      if ( mSelectionIdx >= 0 && mSelectionIdx <= 2 )
      {
         MathUtils::Line clickLine;
         clickLine.origin = event.pos;
         clickLine.direction = event.vec;
         
         LineSegment segment;
         mShortestSegmentBetweenLines( clickLine, mMouseCollideLine, &segment );            
         
         projPnt = segment.p1;

         // snap to the selected axisIdx, if required
         Point3F snapPnt = _snapPoint(projPnt);

         if ( mSelectionIdx < 3 )
         {
            projPnt[mSelectionIdx] = snapPnt[mSelectionIdx];
         }
         else
         {
            projPnt[sgPlanarVectors[mSelectionIdx-3][0]] = snapPnt[sgPlanarVectors[mSelectionIdx-3][0]];
            projPnt[sgPlanarVectors[mSelectionIdx-3][1]] = snapPnt[sgPlanarVectors[mSelectionIdx-3][1]];
         }         
      }
      else if ( 3 <= mSelectionIdx && mSelectionIdx <= 5 )
      {
         if ( mProfile->mode == MoveMode )
         {
            Point3F intersectPnt;
            if ( mMouseCollidePlane.intersect( event.pos, event.vec, &intersectPnt ) )
            {                    
               projPnt = intersectPnt;

               // snap to the selected axisIdx, if required
               Point3F snapPnt = _snapPoint(projPnt);
               projPnt[sgPlanarVectors[mSelectionIdx-3][0]] = snapPnt[sgPlanarVectors[mSelectionIdx-3][0]];
               projPnt[sgPlanarVectors[mSelectionIdx-3][1]] = snapPnt[sgPlanarVectors[mSelectionIdx-3][1]];
            }
         }
         else // ScaleMode
         {
            MathUtils::Line clickLine;
            clickLine.origin = event.pos;
            clickLine.direction = event.vec;

            LineSegment segment;
            mShortestSegmentBetweenLines( clickLine, mMouseCollideLine, &segment );            

            projPnt = segment.p1;
         }
      }
      else if ( mSelectionIdx == Centroid )
      {
         Point3F intersectPnt;
         if ( mMouseCollidePlane.intersect( event.pos, event.vec, &intersectPnt ) )
         {
            projPnt = _snapPoint( intersectPnt );
         }
      }

      // Perform the manipulation...

      if ( mProfile->mode == MoveMode )
      {
         // Clear deltas we aren't using...
         mDeltaRot.zero();
         mDeltaScale.zero();

         Point3F newPosition;
         if( mProfile->snapToGrid )
         {
            Point3F snappedMouseDownProjPnt = _snapPoint( mMouseDownProjPnt );
            mDeltaTotalPos = projPnt - snappedMouseDownProjPnt;
            newPosition = projPnt;
         }
         else
         {
            mDeltaTotalPos = projPnt - mMouseDownProjPnt;  
            newPosition = mSavedTransform.getPosition() + mDeltaTotalPos;
         }
         
         mDeltaPos = newPosition - mTransform.getPosition();
         mTransform.setPosition( newPosition );

         mCurrentTransform.setPosition( newPosition );
      }
      else // ScaleMode
      {
         // This is the world-space axis we want to scale      
         //VectorF axis = sgAxisVectors[mSelectionIdx];

         // Find its object-space components...
         //MatrixF mat = mObjectMat;
         //mat.inverse();
         //mat.mulV(axis);

         // Which needs to always be positive, this is a 'scale' transformation
         // not really a 'vector' transformation.
         //for ( U32 i = 0; i < 3; i++ )
         //   axis[i] = mFabs(axis[i]);

         //axis.normalizeSafe();

         // Clear deltas we aren't using...
         mDeltaRot.zero();
         mDeltaPos.zero();

         
         // Calculate the deltaScale...
         VectorF deltaScale(0,0,0);

         if ( 0 <= mSelectionIdx && mSelectionIdx <= 2 )
         {
            // Are we above or below the starting position relative to this axis?
            PlaneF plane( mMouseDownProjPnt, mProjAxisVector[mSelectionIdx] );
            F32 sign = ( plane.whichSide( projPnt ) == PlaneF::Front ) ? 1 : -1;
            F32 diff = ( projPnt - mMouseDownProjPnt ).len();    

            if ( mProfile->allAxesScaleUniform )
            {
               deltaScale.set(1,1,1);
               deltaScale = deltaScale * sign * diff;
            }
            else
               deltaScale[mSelectionIdx] = diff * sign;
         }
         else if ( 3 <= mSelectionIdx && mSelectionIdx <= 5 )
         {
            PlaneF plane( mMouseDownProjPnt, mMouseCollideLine.direction );
            F32 sign = ( plane.whichSide( projPnt ) == PlaneF::Front ) ? 1 : -1;
            F32 diff = ( projPnt - mMouseDownProjPnt ).len();

            if ( mProfile->allAxesScaleUniform )
            {
               deltaScale.set(1,1,1);
               deltaScale = deltaScale * sign * diff;
            }
            else
            {
               deltaScale[sgPlanarVectors[mSelectionIdx-3][0]] = diff * sign;
               deltaScale[sgPlanarVectors[mSelectionIdx-3][1]] = diff * sign;
            }
         }
         else // mSelectionIdx == 6
         {
            // Are we above or below the starting position relative to the camera?
            VectorF normal;
            mCameraMat.getColumn( 2, &normal );

            PlaneF plane( mMouseDownProjPnt, normal );

            F32 sign = ( plane.whichSide( projPnt ) == PlaneF::Front ) ? 1 : -1;
            F32 diff = ( projPnt - mMouseDownProjPnt ).len();    
            deltaScale.set(1,1,1);
            deltaScale = deltaScale * sign * diff;
         }

         // Save current scale, then set mDeltaScale
         // to the amount it changes during this call.
         mDeltaScale = mScale;

         mDeltaTotalScale = deltaScale;

         mScale = mSavedScale;
         mScale += deltaScale * mProfile->scaleScalar;

         mDeltaScale = mScale - mDeltaScale;

         mScale.setMax( Point3F( 0.01f ) );
      }

      mDirty = true;
   }
   else if ( mProfile->mode == RotateMode &&
             mSelectionIdx != Centroid )      
   {
      // Clear deltas we aren't using...
      mDeltaScale.zero();
      mDeltaPos.zero();

      bool doScreenRot = ( mSelectionIdx == Custom1 );

      U32 rotAxisIdx = ( doScreenRot ) ? 1 : mSelectionIdx;
      
      Point3F mousePntSS( event.mousePoint.x, event.mousePoint.y, 0.0f );
      
      Point3F pntSS0 = mElipseCursorCollidePntSS + mElipseCursorCollideVecSS * 10000.0f;
      Point3F pntSS1 = mElipseCursorCollidePntSS - mElipseCursorCollideVecSS * 10000.0f;

      Point3F closestPntSS = MathUtils::mClosestPointOnSegment( pntSS0, pntSS1, mousePntSS );  

      Point3F offsetDir = closestPntSS - mElipseCursorCollidePntSS;
      F32 offsetDist = offsetDir.len();
      offsetDir.normalizeSafe();                  

      F32 dot = mDot( mElipseCursorCollideVecSS, offsetDir );
      mSign = mIsZero( dot ) ? 0.0f : ( dot > 0.0f ) ? -1.0f : 1.0f;  

      // The angle that we will rotate the (saved) gizmo transform by to 
      // generate the current gizmo transform.
      F32 angle = offsetDist * mSign * mProfile->rotateScalar;
      angle *= 0.02f; // scale down to not require rotate scalar to be microscopic

      //
      if( mProfile->allowSnapRotations && event.modifier & SI_SHIFT )
         angle = mDegToRad( _snapFloat( mRadToDeg( angle ), mProfile->rotationSnap ) );

      mDeltaAngle = angle - mLastAngle;
      mLastAngle = angle;         

      if ( doScreenRot )
      {         
         // Rotate relative to the camera.
         // We rotate around the y/forward vector pointing from the camera
         // to the gizmo.
                
         // NOTE: This does NOT work

         // Calculate mDeltaAngle and mDeltaTotalRot
         //{
         //   VectorF fvec( mOrigin - mCameraPos );
         //   fvec.normalizeSafe();        

         //   AngAxisF aa( fvec, mDeltaAngle );
         //   MatrixF mat;
         //   aa.setMatrix( &mat );

         //   mDeltaRot = mat.toEuler();                

         //   aa.set( fvec, mLastAngle );            
         //   aa.setMatrix( &mat );
         //   mDeltaTotalRot = mat.toEuler();
         //}
     
         //MatrixF rotMat( mDeltaTotalRot );

         //if ( mCurrentAlignment == World )
         //{
         //   //aa.setMatrix( &rotMat );
         //   mTransform = mSavedTransform * rotMat;
         //   mTransform.setPosition( mOrigin );

         //   rotMat.inverse();
         //   mCurrentTransform = mObjectMatInv * rotMat;
         //   mCurrentTransform.inverse();
         //   mCurrentTransform.setPosition( mOrigin ); 
         //}
         //else
         //{
         //   rotMat.inverse();

         //   MatrixF m0;
         //   mSavedTransform.invertTo(&m0);
         //   
         //   mTransform = m0 * rotMat;
         //   mTransform.inverse();
         //   mTransform.setPosition( mOrigin );

         //   mCurrentTransform = mTransform;
         //}    
      }      
      else
      {        
         // Normal rotation, eg, not screen relative.

         mDeltaRot.set(0,0,0);      
         mDeltaRot[rotAxisIdx] = mDeltaAngle;            

         mDeltaTotalRot.set(0,0,0);
         mDeltaTotalRot[rotAxisIdx] = angle; 

         MatrixF rotMat( mDeltaTotalRot );

         mTransform = mSavedTransform * rotMat;      
         mTransform.setPosition( mSavedTransform.getPosition() );

         if ( mCurrentAlignment == World )
         {
            MatrixF mat0 = mCurrentTransform;

            rotMat.inverse();
            mCurrentTransform = mObjectMatInv * rotMat;
            mCurrentTransform.inverse();
            mCurrentTransform.setPosition( mOrigin );         

            MatrixF mat1 = mCurrentTransform;
            mat0.inverse();
            MatrixF mrot;
            mrot = mat0 * mat1;
            mDeltaRot = mrot.toEuler();
         }
         else
         {
            mCurrentTransform = mTransform;
         }
      }            

      mDirty = true;
   }   

   mLastMouseEvent = event;
}

//------------------------------------------------------------------------------

void Gizmo::renderGizmo(const MatrixF &cameraTransform, F32 cameraFOV )
{
   mLastWorldMat = GFX->getWorldMatrix();
   mLastProjMat = GFX->getProjectionMatrix();
   mLastViewport = GFX->getViewport();
   mLastWorldToScreenScale = GFX->getWorldToScreenScale();
   mLastCameraFOV = cameraFOV;

   // Save the Camera transform matrix, used all over...
   mCameraMat = cameraTransform;
   mCameraPos = mCameraMat.getPosition();   

   GFXFrustumSaver fsaver;

   // Change the far plane distance so that the gizmo is always visible.
   Frustum frustum = GFX->getFrustum();
   frustum.setFarDist( 100000.0f );
   GFX->setFrustum( frustum );

   _updateEnabledAxices();   

   _updateState();

   _calcAxisInfo();   

   if( mMouseDown )
   {
      if( mProfile->renderMoveGrid && mMoveGridEnabled && mCurrentMode == MoveMode )
      {
         GFXStateBlockDesc desc;

         desc.setBlend( true );
         desc.setZReadWrite( true, true );

         GFXDrawUtil::Plane plane = GFXDrawUtil::PlaneXY;
         ColorI color( 128, 128, 128, 200 );

         switch( mSelectionIdx )
         {
            case Axis_Z:
            case Plane_XY:
               plane = GFXDrawUtil::PlaneXY;
               break;

            case Axis_X:
            case Plane_YZ:
               plane = GFXDrawUtil::PlaneYZ;
               break;

            case Axis_Y:
            case Plane_XZ:
               plane = GFXDrawUtil::PlaneXZ;
               break;
         }

         GFX->getDrawUtil()->drawPlaneGrid(
            desc,
            mTransform.getPosition(),
            Point2F( mMoveGridSize, mMoveGridSize ),
            Point2F( mMoveGridSpacing, mMoveGridSpacing ),
            color,
            plane
         );
      }

      if( !mProfile->renderWhenUsed )
         return;
   }

   mHighlightAll = mProfile->allAxesScaleUniform && mSelectionIdx >= 0 && mSelectionIdx <= 3;      

   // Render plane (if set to render) behind the gizmo
   if ( mProfile->mode != NoneMode )
      _renderPlane();

   _setStateBlock(); 

   // Special case for NoneMode, 
   // we only render the primary axis with no tips.
   if ( mProfile->mode == NoneMode )
   {
      _renderPrimaryAxis();
      return;
   }

   if ( mProfile->mode == RotateMode )
   {
      PrimBuild::begin( GFXLineList, 6 );

      // Render the primary axisIdx
      for(U32 i = 0; i < 3; i++)
      {         
         PrimBuild::color( ( mHighlightAll || i == mSelectionIdx ) ? mProfile->axisColors[i] : mProfile->inActiveColor );
         PrimBuild::vertex3fv( mOrigin );
         PrimBuild::vertex3fv( mOrigin + mProjAxisVector[i] * mProjLen * 0.25f );
      }

      PrimBuild::end();

      _renderAxisCircles();
   }
   else
   {
      // Both Move and Scale modes render basis vectors as
      // large stick lines.
      _renderPrimaryAxis();

      // Render the tips based on current operation.

      GFXTransformSaver saver( true, false );
      
      GFX->multWorld(mTransform);

      if ( mProfile->mode == ScaleMode )
      {      
         _renderAxisBoxes();
      }
      else if ( mProfile->mode == MoveMode )
      {
         _renderAxisArrows();
      }

      saver.restore();
   }         

   // Render the planar handles...

   if ( mCurrentMode != RotateMode )
   {   
      Point3F midpnt[3];
      for(U32 i = 0; i < 3; i++)
         midpnt[i] = mProjAxisVector[i] * mProjLen * 0.5f;

      PrimBuild::begin( GFXLineList, 12 );

      for(U32 i = 0; i < 3; i++)
      {
         U32 axis0 = sgPlanarVectors[i][0];
         U32 axis1 = sgPlanarVectors[i][1];

         const Point3F &p0 = midpnt[axis0];
         const Point3F &p1 = midpnt[axis1];

         bool selected0 = false;
         bool selected1 = false;
         
         if ( i + 3 == mSelectionIdx )
            selected0 = selected1 = true;

         bool inactive = !mAxisEnabled[axis0] || !mAxisEnabled[axis1] || !(mProfile->flags & GizmoProfile::PlanarHandlesOn);
         
         if ( inactive )
            PrimBuild::color( mProfile->hideDisabledAxes ? ColorI::ZERO : mProfile->inActiveColor );
         else
            PrimBuild::color( selected0 ? mProfile->activeColor : mProfile->axisColors[axis0] );
          
         PrimBuild::vertex3fv( mOrigin + p0 );
         PrimBuild::vertex3fv( mOrigin + p0 + p1 );

         if ( inactive )
            PrimBuild::color( mProfile->hideDisabledAxes ? ColorI::ZERO : mProfile->inActiveColor );
         else
            PrimBuild::color( selected1 ? mProfile->activeColor : mProfile->axisColors[axis1] );

         PrimBuild::vertex3fv( mOrigin + p1 );
         PrimBuild::vertex3fv( mOrigin + p0 + p1 );
      }

      PrimBuild::end();

      // Render planar handle as solid if selected.

      ColorI planeColorSEL( mProfile->activeColor );
      planeColorSEL.alpha = 75;
      ColorI planeColorNA( 0, 0, 0, 0 );

      PrimBuild::begin( GFXTriangleList, 18 );

      for(U32 i = 0; i < 3; i++)
      {
         U32 axis0 = sgPlanarVectors[i][0];
         U32 axis1 = sgPlanarVectors[i][1];

         const Point3F &p0 = midpnt[axis0];
         const Point3F &p1 = midpnt[axis1];

         if ( i + 3 == mSelectionIdx )
            PrimBuild::color( planeColorSEL );
         else
            PrimBuild::color( planeColorNA );
         
         PrimBuild::vertex3fv( mOrigin );
         PrimBuild::vertex3fv( mOrigin + p0 );
         PrimBuild::vertex3fv( mOrigin + p0 + p1 );

         PrimBuild::vertex3fv( mOrigin );
         PrimBuild::vertex3fv( mOrigin + p0 + p1 );
         PrimBuild::vertex3fv( mOrigin + p1 );
      }

      PrimBuild::end();
   }

   // Render Centroid Handle...
   if ( mUniformHandleEnabled )   
   {      
      F32 tipScale = mProjLen * 0.075f;
      GFXTransformSaver saver;
      GFX->multWorld( mTransform );

      if ( mSelectionIdx == Centroid || mHighlightAll || mHighlightCentroidHandle )
         PrimBuild::color( mProfile->centroidHighlightColor );
      else
         PrimBuild::color( mProfile->centroidColor );

      for(U32 j = 0; j < 6; j++)
      {
         PrimBuild::begin( GFXTriangleStrip, 4 );

         PrimBuild::vertex3fv( sgCenterBoxPnts[sgBoxVerts[j][0]] * tipScale);
         PrimBuild::vertex3fv( sgCenterBoxPnts[sgBoxVerts[j][1]] * tipScale);
         PrimBuild::vertex3fv( sgCenterBoxPnts[sgBoxVerts[j][2]] * tipScale);
         PrimBuild::vertex3fv( sgCenterBoxPnts[sgBoxVerts[j][3]] * tipScale);

         PrimBuild::end();
      }
   }
}

void Gizmo::renderText( const RectI &viewPort, const MatrixF &modelView, const MatrixF &projection )
{
   if ( mProfile->mode == NoneMode )
      return;

   if ( mMouseDown && !mProfile->renderWhenUsed )
      return;

   GFXDrawUtil *drawer = GFX->getDrawUtil();

   _setStateBlock();

   char axisText[] = "xyz";

   F32 projLen = mProjLen * 1.05f;
   if ( mProfile->mode == RotateMode )
      projLen *= 0.28f;

   for ( U32 i = 0; i < 3; i++ )
   {
      if ( !mAxisEnabled[i] && mProfile->hideDisabledAxes )
         continue;

      const Point3F & centroid = mOrigin;
      Point3F pos(centroid.x + mProjAxisVector[i].x * projLen,
         centroid.y + mProjAxisVector[i].y * projLen,
         centroid.z + mProjAxisVector[i].z * projLen);

      Point3F sPos;

      if ( MathUtils::mProjectWorldToScreen( pos, &sPos, viewPort, modelView, projection ) )
      {
         ColorI textColor = ColorI(170,170,170);

         if ( mProfile->mode == RotateMode )
         {
            textColor.set(170,170,170);
            if ( i == mSelectionIdx )
               textColor = mProfile->axisColors[i];
         }
         else
         {
            if ( i == mSelectionIdx || !mAxisEnabled[i] )
               textColor = mProfile->inActiveColor;
            else
               textColor = mProfile->axisColors[i];
         }

         char buf[2];
         buf[0] = axisText[i]; buf[1] = '\0';
         drawer->setBitmapModulation(textColor);
         drawer->drawText( mProfile->font, Point2I((S32)sPos.x, (S32)sPos.y), buf );
      }
   }
}

// Gizmo Internal Methods...

void Gizmo::_calcAxisInfo()
{   
   mOrigin = mTransform.getPosition();

   for ( U32 i = 0; i < 3; i++ )
   {      
      VectorF tmp;
      mTransform.mulV(sgAxisVectors[i], &tmp);
      mProjAxisVector[i] = tmp;
      mProjAxisVector[i].normalizeSafe();
   }     

   // get the projected size...
   
   mProjLen = _getProjectionLength( mProfile->screenLen );
}

void Gizmo::_renderPrimaryAxis()
{
   // Render the primary axis(s)
   for ( U32 i = 0; i < 3; i++ )
   {
      ColorI color = mProfile->axisColors[i];

      if ( !mAxisEnabled[i] )
      {
         color = mProfile->inActiveColor;
         if ( mProfile->hideDisabledAxes )
            color.alpha = 0;
      }
      else
      {
         if ( 0 <= mSelectionIdx && mSelectionIdx <= 2 )
         {
            if ( i == mSelectionIdx )
               color = mProfile->activeColor;
         }
         else if ( 3 <= mSelectionIdx && mSelectionIdx <= 5 )
         {
            if ( i == sgPlanarVectors[mSelectionIdx-3][0] ||
               i == sgPlanarVectors[mSelectionIdx-3][1] )
               color = mProfile->activeColor;
         }
         else if ( mSelectionIdx == 6 )
            color = mProfile->activeColor;      
      }

      if ( mHighlightAll )
      {
         // Previous logic is complex so do this outside.
         // Don't change the alpha calculated previously but override
         // the color to the activeColor.
         U8 saveAlpha = color.alpha;
         color = mProfile->activeColor;
         color.alpha = saveAlpha;
      }

      PrimBuild::begin( GFXLineList, 2 );    
      PrimBuild::color( color );
      PrimBuild::vertex3fv( mOrigin );
      PrimBuild::vertex3fv( mOrigin + mProjAxisVector[i] * mProjLen );
      PrimBuild::end();
   }
}

void Gizmo::_renderAxisArrows()
{
   F32 tipScale = mProjLen * 0.25;
   S32 x, y, z;
   Point3F pnt;

   for ( U32 axisIdx = 0; axisIdx < 3; axisIdx++ )
   {
      if ( mProfile->hideDisabledAxes && !mAxisEnabled[axisIdx] )
         continue;

      PrimBuild::begin( GFXTriangleList, 12*3 );

      if ( !mAxisEnabled[axisIdx] )
         PrimBuild::color( mProfile->inActiveColor );
      else
         PrimBuild::color( mProfile->axisColors[axisIdx] );

      x = sgAxisRemap[axisIdx][0];
      y = sgAxisRemap[axisIdx][1];
      z = sgAxisRemap[axisIdx][2];      

      for ( U32 i = 0; i < sizeof(sgConeVerts) / (sizeof(U32)*3); ++i )
      {
         const Point3F& conePnt0 = sgConePnts[sgConeVerts[i][0]];
         pnt.set(conePnt0[x], conePnt0[y], conePnt0[z]);
         PrimBuild::vertex3fv(pnt * tipScale + sgAxisVectors[axisIdx] * mProjLen);

         const Point3F& conePnt1 = sgConePnts[sgConeVerts[i][1]];
         pnt.set(conePnt1[x], conePnt1[y], conePnt1[z]);
         PrimBuild::vertex3fv(pnt * tipScale + sgAxisVectors[axisIdx] * mProjLen);

         const Point3F& conePnt2 = sgConePnts[sgConeVerts[i][2]];
         pnt.set(conePnt2[x], conePnt2[y], conePnt2[z]);
         PrimBuild::vertex3fv(pnt * tipScale + sgAxisVectors[axisIdx] * mProjLen);
      }

      PrimBuild::end();
   }
}

void Gizmo::_renderAxisBoxes()
{
   if ( mProfile->hideDisabledAxes && !( mProfile->flags & GizmoProfile::CanScale ) )
      return;

   F32 tipScale = mProjLen * 0.1;
   F32 pos = mProjLen - 0.5 * tipScale;

   for( U32 axisIdx = 0; axisIdx < 3; ++axisIdx )
   {
      if ( mProfile->hideDisabledAxes && !( mProfile->flags & ( GizmoProfile::CanScaleX << axisIdx ) ) )
         continue;

      if ( mAxisEnabled[axisIdx] )
         PrimBuild::color( mProfile->axisColors[axisIdx] );
      else
         PrimBuild::color( mProfile->inActiveColor );

      for(U32 j = 0; j < 6; j++)
      {
         PrimBuild::begin( GFXTriangleStrip, 4 );

         PrimBuild::vertex3fv( sgCenterBoxPnts[sgBoxVerts[j][0]] * tipScale + sgAxisVectors[axisIdx] * pos );
         PrimBuild::vertex3fv( sgCenterBoxPnts[sgBoxVerts[j][1]] * tipScale + sgAxisVectors[axisIdx] * pos );
         PrimBuild::vertex3fv( sgCenterBoxPnts[sgBoxVerts[j][2]] * tipScale + sgAxisVectors[axisIdx] * pos );
         PrimBuild::vertex3fv( sgCenterBoxPnts[sgBoxVerts[j][3]] * tipScale + sgAxisVectors[axisIdx] * pos );

         PrimBuild::end();
      }
   }
}

void Gizmo::_renderAxisCircles()
{
   if ( mProfile->hideDisabledAxes && !( mProfile->flags & GizmoProfile::CanRotate ) )
      return;

   // Setup the WorldMatrix for rendering in camera space.
   // Honestly not sure exactly why this works but it does...
   GFX->pushWorldMatrix();
   MatrixF cameraXfm = GFX->getWorldMatrix();   
   cameraXfm.inverse();
   const Point3F cameraPos = cameraXfm.getPosition();
   cameraXfm.setPosition( mOrigin );
   GFX->multWorld(cameraXfm);

   // Render the ScreenSpace rotation circle...
   if ( !( mProfile->hideDisabledAxes && !mScreenRotateHandleEnabled ) )
   {
      F32 radius = mProjLen * 0.7f;
      U32 segments = 40;
      F32 step = mDegToRad(360.0f)/ segments;
      Point3F pnt;

      PrimBuild::color( ( mHighlightAll || mSelectionIdx == Custom1 ) ? mProfile->activeColor : mProfile->inActiveColor );
      PrimBuild::begin( GFXLineStrip, segments+1 );

      for(U32 i = 0; i <= segments; i++)
      {
         F32 angle = i * step;

         pnt.x = mCos(angle) * radius;
         pnt.y = 0.0f;
         pnt.z = mSin(angle) * radius;

         PrimBuild::vertex3fv( pnt );
      }

      PrimBuild::end();
   }

   // Render the gizmo/sphere bounding circle...
   {
      F32 radius = mProjLen * 0.5f;   
      U32 segments = 40;
      F32 step = mDegToRad(360.0f) / segments;
      Point3F pnt;
      
      // Render as solid (with transparency) when the sphere is selected
      if ( mSelectionIdx == Custom2 )
      {
         ColorI color = mProfile->inActiveColor;
         color.alpha = 100;
         PrimBuild::color( color );
         PrimBuild::begin( GFXTriangleStrip, segments+2 );
         
         PrimBuild::vertex3fv( Point3F(0,0,0) );

         for(U32 i = 0; i <= segments; i++)
         {
            F32 angle = i * step;

            pnt.x = mCos(angle) * radius;
            pnt.y = 0.0f;
            pnt.z = mSin(angle) * radius;

            PrimBuild::vertex3fv( pnt );
         }

         PrimBuild::end();
      }
      else
      {
         PrimBuild::color( mProfile->inActiveColor );
         PrimBuild::begin( GFXLineStrip, segments+1 );

         for(U32 i = 0; i <= segments; i++)
         {
            F32 angle = i * step;

            pnt.x = mCos(angle) * radius;
            pnt.y = 0.0f;
            pnt.z = mSin(angle) * radius;

            PrimBuild::vertex3fv( pnt );
         }

         PrimBuild::end();
      }
   }

   // Done rendering in camera space.
   GFX->popWorldMatrix();

   // Setup WorldMatrix for Gizmo-Space rendering.
   GFX->pushWorldMatrix();
   GFX->multWorld(mTransform);

   // Render the axis-manipulation ellipses...
   {
      F32 radius = mProjLen * 0.5f;
      U32 segments = 40;
      F32 step = mDegToRad(360.0f) / segments;
      U32 x,y,z;

      VectorF planeNormal;
      planeNormal = mOrigin - cameraPos;
      planeNormal.normalize();
      PlaneF clipPlane( mOrigin, planeNormal );

      MatrixF worldToGizmo = mTransform;
      worldToGizmo.inverse();
      mTransformPlane( worldToGizmo, Point3F(1,1,1), clipPlane, &clipPlane );

      for ( U32 axis = 0; axis < 3; axis++ )
      {
         if ( mProfile->hideDisabledAxes && !( mProfile->flags & ( GizmoProfile::CanRotateX << axis ) ) )
            continue;

         if ( mAxisEnabled[axis] || mHighlightAll )
            PrimBuild::color( (axis == mSelectionIdx) ? mProfile->activeColor : mProfile->axisColors[axis] );
         else
            PrimBuild::color( mProfile->inActiveColor );

         x = sgAxisRemap[axis][0];
         y = sgAxisRemap[axis][1];
         z = sgAxisRemap[axis][2];

         PrimBuild::begin( GFXLineList, (segments+1) * 2 );

         for ( U32 i = 1; i <= segments; i++ )
         {
            F32 ang0 = (i-1) * step;
            F32 ang1 = i * step;

            Point3F temp;

            temp.x = 0.0f;
            temp.y = mCos(ang0) * radius;
            temp.z = mSin(ang0) * radius;
            Point3F pnt0( temp[x], temp[y], temp[z] );

            temp.x = 0.0f;
            temp.y = mCos(ang1) * radius;
            temp.z = mSin(ang1) * radius;
            Point3F pnt1( temp[x], temp[y], temp[z] );
            
            bool valid0 = ( clipPlane.whichSide(pnt0) == PlaneF::Back );
            bool valid1 = ( clipPlane.whichSide(pnt1) == PlaneF::Back );

            //if ( !valid0 && !valid1 )
            //   continue;

            if ( !valid0 || !valid1 )
               continue;

            PrimBuild::vertex3fv( pnt0 );
            PrimBuild::vertex3fv( pnt1 );
         }

         PrimBuild::end();
      }
   }

   // Done rendering in Gizmo-Space.
   GFX->popWorldMatrix();

   // Render hint-arrows...
   /*
   if ( mMouseDown && mSelectionIdx != -1 )
   {
      PrimBuild::begin( GFXLineList, 4 );

      F32 hintArrowScreenLength = mProfile->screenLen * 0.5f;
      F32 hintArrowTipScreenLength = mProfile->screenLen * 0.25;

      F32 worldZDist = ( mMouseCollideLine.origin - mCameraPos ).len();
      F32 hintArrowLen = ( hintArrowScreenLength * worldZDist ) / mLastWorldToScreenScale.y;
      F32 hintArrowTipLen = ( hintArrowTipScreenLength * worldZDist ) / mLastWorldToScreenScale.y;            

      Point3F p0 = mMouseCollideLine.origin - mMouseCollideLine.direction * hintArrowLen;
      Point3F p1 = mMouseCollideLine.origin;
      Point3F p2 = mMouseCollideLine.origin + mMouseCollideLine.direction * hintArrowLen;

      // For whatever reason, the sign is actually negative if we are on the
      // positive size of the MouseCollideLine direction.
      ColorI color0 = ( mSign > 0.0f ) ? mProfile->activeColor : mProfile->inActiveColor;
      ColorI color1 = ( mSign < 0.0f ) ? mProfile->activeColor : mProfile->inActiveColor;
            
      PrimBuild::color( color0 );
      PrimBuild::vertex3fv( p1 );
      PrimBuild::vertex3fv( p0 );

      PrimBuild::color( color1 );
      PrimBuild::vertex3fv( p1 );
      PrimBuild::vertex3fv( p2 );
      PrimBuild::end();

      GFXStateBlockDesc desc;
      desc.setBlend( true );
      desc.setZReadWrite( false, false );

      GFXDrawUtil *drawer = GFX->getDrawUtil();
      drawer->drawCone( desc, p0, p0 - mMouseCollideLine.direction * hintArrowTipLen, hintArrowTipLen * 0.5f, color0 );
      drawer->drawCone( desc, p2, p2 + mMouseCollideLine.direction * hintArrowTipLen, hintArrowTipLen * 0.5f, color1 );
   }
   */
}

void Gizmo::_renderPlane()
{
   if( !mGridPlaneEnabled )
      return;
      
   Point2F size( mProfile->planeDim, mProfile->planeDim );

   GFXStateBlockDesc desc;
   desc.setBlend( true );
   desc.setZReadWrite( true, false );

   GFXTransformSaver saver;
   GFX->multWorld( mTransform );

   if ( mProfile->renderPlane )
      GFX->getDrawUtil()->drawSolidPlane( desc, Point3F::Zero, size, mProfile->gridColor );

   if ( mProfile->renderPlaneHashes )
   {
      // TODO: This wasn't specified before... so it was 
      // rendering lines that were invisible.  Maybe we need
      // a new field for grid line color?
      ColorI gridColor( mProfile->gridColor );
      gridColor.alpha *= 2;

      GFX->getDrawUtil()->drawPlaneGrid( desc, Point3F::Zero, size, Point2F( mProfile->gridSize.x, mProfile->gridSize.y ), gridColor );
   }
}


void Gizmo::_setStateBlock()
{
   if ( !mStateBlock )
   {
      GFXStateBlockDesc sb;
      sb.blendDefined = true;
      sb.blendEnable = true;
      sb.blendSrc = GFXBlendSrcAlpha;
      sb.blendDest = GFXBlendInvSrcAlpha;
      sb.zDefined = true;
      sb.zEnable = false;
      sb.cullDefined = true;
      sb.cullMode = GFXCullNone;
      mStateBlock = GFX->createStateBlock(sb);

      sb.setZReadWrite( true, false );
      mSolidStateBlock = GFX->createStateBlock(sb);
   }

   //if ( mProfile->renderSolid )
   //   GFX->setStateBlock( mSolidStateBlock );
   //else
      GFX->setStateBlock( mStateBlock );
}

Point3F Gizmo::_snapPoint( const Point3F &pnt ) const
{   
   if ( !mProfile->snapToGrid )
      return pnt;

   Point3F snap;
   snap.x = _snapFloat( pnt.x, mProfile->gridSize.x );
   snap.y = _snapFloat( pnt.y, mProfile->gridSize.y );
   snap.z = _snapFloat( pnt.z, mProfile->gridSize.z );

   return snap;
}

F32 Gizmo::_snapFloat( const F32 &val, const F32 &snap ) const
{   
   if ( snap == 0.0f )
      return val;

   F32 a = mFmod( val, snap );

   F32 temp = val;

   if ( mFabs(a) > (snap / 2) )
      val < 0.0f ? temp -= snap : temp += snap;

   return(temp - a);
}

GizmoAlignment Gizmo::_filteredAlignment()
{
   GizmoAlignment align = mProfile->alignment;

   // Special case in ScaleMode, always be in object.
   if ( mProfile->mode == ScaleMode )
      align = Object;

   return align;
}

void Gizmo::_updateState( bool collideGizmo )
{   
   if ( !mProfile )
      return;

   // Update mCurrentMode

   if ( mCurrentMode != mProfile->mode )
   {
      // Changing the mode invalidates the prior selection since the gizmo
      // has changed shape.

      mCurrentMode = mProfile->mode;
      mSelectionIdx = -1;

      // Determine the new selection unless we have been told not to.
      if ( collideGizmo )
         collideAxisGizmo( mLastMouseEvent );

      // Also cancel any current dragging operation since it would only be
      // valid if the mouse down event occurred first.

      mMouseDown = false;
   }

   // Update mCurrentAlignment

   // Changing the alignment during a drag could be really bad.
   // Haven't actually tested this though.
   if ( mMouseDown )
      return;

   GizmoAlignment desired = _filteredAlignment();
   
   if ( desired == World && 
        mCurrentAlignment == Object )
   {
      mObjectMat = mTransform;
      mTransform.identity();
      mTransform.setPosition( mObjectMat.getPosition() );
   }
   else if ( desired == Object && 
             mCurrentAlignment == World )
   {
      Point3F pos = mTransform.getPosition();
      mTransform = mObjectMat;
      mTransform.setPosition( pos );
      mObjectMat.identity();        
      mObjectMat.setPosition( pos );
   }

   mCurrentAlignment = desired;
   
   mObjectMat.invertTo( &mObjectMatInv );
}

void Gizmo::_updateEnabledAxices()
{
   if ( ( mProfile->mode == ScaleMode && mProfile->flags & GizmoProfile::CanScaleUniform ) ||
        ( mProfile->mode == MoveMode && mProfile->flags & GizmoProfile::CanTranslateUniform ) ||
        ( mProfile->mode == RotateMode && mProfile->flags & GizmoProfile::CanRotateUniform ) )
      mUniformHandleEnabled = true;
   else
      mUniformHandleEnabled = false;

   // Screen / camera relative rotation disabled until it functions properly
   //
   //if ( mProfile->mode == RotateMode && mProfile->flags & GizmoProfile::CanRotateScreen )
   //   mScreenRotateHandleEnabled = true;
   //else
      mScreenRotateHandleEnabled = false;

   // Early out if we are in a mode that is disabled.
   if ( mProfile->mode == RotateMode && !(mProfile->flags & GizmoProfile::CanRotate ) )
   {
      mAxisEnabled[0] = mAxisEnabled[1] = mAxisEnabled[2] = false;
      return;
   }
   if ( mProfile->mode == MoveMode && !(mProfile->flags & GizmoProfile::CanTranslate ) )
   {
      mAxisEnabled[0] = mAxisEnabled[1] = mAxisEnabled[2] = false;
      return;
   }
   if ( mProfile->mode == ScaleMode && !(mProfile->flags & GizmoProfile::CanScale ) )
   {
      mAxisEnabled[0] = mAxisEnabled[1] = mAxisEnabled[2] = false;
      return;
   }
   
   for ( U32 i = 0; i < 3; i++ )
   {
      mAxisEnabled[i] = false;

      // Some tricky enum math... x/y/z are sequential in the enum
      if ( ( mProfile->mode == RotateMode ) &&
           !( mProfile->flags & ( GizmoProfile::CanRotateX << i ) ) )
         continue;
      if ( ( mProfile->mode == MoveMode ) &&
           !( mProfile->flags & ( GizmoProfile::CanTranslateX << i ) ) )
         continue;
      if ( ( mProfile->mode == ScaleMode ) &&
           !( mProfile->flags & ( GizmoProfile::CanScaleX << i ) ) )
         continue;

      mAxisEnabled[i] = true;
   }   
}
