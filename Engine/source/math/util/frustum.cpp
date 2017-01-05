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
#include "math/util/frustum.h"

#include "math/mMathFn.h"
#include "math/mathUtils.h"
#include "math/mSphere.h"
#include "platform/profiler.h"

static const MatrixF sGFXProjRotMatrix( EulerF( (M_PI_F / 2.0f), 0.0f, 0.0f ) );


//TODO: For OBB/frustum intersections and ortho frustums, we can resort to a much quicker AABB/OBB test


// Must be CW ordered for face[0] of each edge!  Keep in mind that normals
// are pointing *inwards* and thus what appears CCW outside is CW inside.
FrustumData::EdgeListType FrustumData::smEdges
(
   PolyhedronData::Edge( PlaneNear, PlaneTop, NearTopRight, NearTopLeft ),
   PolyhedronData::Edge( PlaneNear, PlaneBottom, NearBottomLeft, NearBottomRight ),
   PolyhedronData::Edge( PlaneNear, PlaneLeft, NearTopLeft, NearBottomLeft ),
   PolyhedronData::Edge( PlaneNear, PlaneRight, NearTopRight, NearBottomRight ),
   PolyhedronData::Edge( PlaneFar, PlaneTop, FarTopLeft, FarTopRight ),
   PolyhedronData::Edge( PlaneFar, PlaneBottom, FarBottomRight, FarBottomLeft ),
   PolyhedronData::Edge( PlaneFar, PlaneLeft, FarBottomLeft, FarTopLeft ),
   PolyhedronData::Edge( PlaneFar, PlaneRight, FarTopRight, FarBottomRight ),
   PolyhedronData::Edge( PlaneTop, PlaneLeft, FarTopLeft, NearTopLeft ),
   PolyhedronData::Edge( PlaneTop, PlaneRight, NearTopRight, FarTopRight ),
   PolyhedronData::Edge( PlaneBottom, PlaneLeft, NearBottomLeft, FarBottomLeft ),
   PolyhedronData::Edge( PlaneBottom, PlaneRight, FarBottomRight, NearBottomRight )
);


//-----------------------------------------------------------------------------

Frustum::Frustum( bool isOrtho,
                  F32 nearLeft, 
                  F32 nearRight, 
                  F32 nearTop, 
                  F32 nearBottom, 
                  F32 nearDist,                  
                  F32 farDist,
                  const MatrixF &transform )
{
   mTransform = transform;
   mPosition = transform.getPosition();

   mNearLeft = nearLeft;
   mNearRight = nearRight;
   mNearTop = nearTop;
   mNearBottom = nearBottom;
   mNearDist = nearDist;
   mFarDist = farDist;
   mIsOrtho = isOrtho;

   mNumTiles = 1;
   mCurrTile.set(0,0);
   mTileOverlap.set(0.0f, 0.0f);

   mProjectionOffset.zero();
   mProjectionOffsetMatrix.identity();
}

//-----------------------------------------------------------------------------

void Frustum::set(   bool isOrtho,
                     F32 fovYInRadians, 
                     F32 aspectRatio, 
                     F32 nearDist, 
                     F32 farDist,
                     const MatrixF &transform )
{
   F32 left, right, top, bottom;
   MathUtils::makeFrustum( &left, &right, &top, &bottom, fovYInRadians, aspectRatio, nearDist );

   tile( &left, &right, &top, &bottom, mNumTiles, mCurrTile, mTileOverlap );
   set( isOrtho, left, right, top, bottom, nearDist, farDist, transform );
}

//-----------------------------------------------------------------------------

void Frustum::set(   bool isOrtho,
                     F32 nearLeft, 
                     F32 nearRight, 
                     F32 nearTop, 
                     F32 nearBottom, 
                     F32 nearDist, 
                     F32 farDist,
                     const MatrixF &transform )
{
   mTransform = transform;
   mPosition = mTransform.getPosition();

   mNearLeft = nearLeft;
   mNearRight = nearRight;
   mNearTop = nearTop;
   mNearBottom = nearBottom;
   mNearDist = nearDist;
   mFarDist = farDist;
   mIsOrtho = isOrtho;

   mDirty = true;
}

//-----------------------------------------------------------------------------

#if 0
void Frustum::set( const MatrixF &projMat, bool normalize )
{ 
   // From "Fast Extraction of Viewing Frustum Planes from the World-View-Projection Matrix"
   // by Gil Gribb and Klaus Hartmann.
   //
   // http://www2.ravensoft.com/users/ggribb/plane%20extraction.pdf

   // Right clipping plane.
   mPlanes[ PlaneRight ].set(  projMat[3] - projMat[0], 
                                             projMat[7] - projMat[4], 
                                             projMat[11] - projMat[8],
                                             projMat[15] - projMat[12] );

   // Left clipping plane.
   mPlanes[ PlaneLeft ].set(   projMat[3] + projMat[0], 
                                             projMat[7] + projMat[4], 
                                             projMat[11] + projMat[8], 
                                             projMat[15] + projMat[12] );

   // Bottom clipping plane.
   mPlanes[ PlaneBottom ].set( projMat[3] + projMat[1], 
                                             projMat[7] + projMat[5], 
                                             projMat[11] + projMat[9], 
                                             projMat[15] + projMat[13] );

   // Top clipping plane.
   mPlanes[ PlaneTop ].set(    projMat[3] - projMat[1], 
                                             projMat[7] - projMat[5], 
                                             projMat[11] - projMat[9], 
                                             projMat[15] - projMat[13] );

   // Near clipping plane
   mPlanes[ PlaneNear ].set(   projMat[3] + projMat[2], 
                                             projMat[7] + projMat[6], 
                                             projMat[11] + projMat[10],
                                             projMat[15] + projMat[14] );

   // Far clipping plane.
   mPlanes[ PlaneFar ].set(    projMat[3] - projMat[2], 
                                             projMat[7] - projMat[6], 
                                             projMat[11] - projMat[10], 
                                             projMat[15] - projMat[14] );

   if( normalize )
   {
      for( S32 i = 0; i < PlaneCount; ++ i )
         mPlanes[ i ].normalize();
   }

   /*// Create the corner points via plane intersections.
   mPlanes[ PlaneNear ].intersect( mPlanes[ PlaneTop ], mPlanes[ PlaneLeft ], &mPoints[ NearTopLeft ] );
   mPlanes[ PlaneNear ].intersect( mPlanes[ PlaneTop ], mPlanes[ PlaneRight ], &mPoints[ NearTopRight ] );
   mPlanes[ PlaneNear ].intersect( mPlanes[ PlaneBottom ], mPlanes[ PlaneLeft ], &mPoints[ NearBottomLeft ] );
   mPlanes[ PlaneNear ].intersect( mPlanes[ PlaneBottom ], mPlanes[ PlaneRight ], &mPoints[ NearBottomRight ] );
   mPlanes[ PlaneFar ].intersect( mPlanes[ PlaneTop ], mPlanes[ PlaneLeft ], &mPoints[ FarTopLeft ] );
   mPlanes[ PlaneFar ].intersect( mPlanes[ PlaneTop ], mPlanes[ PlaneRight ], &mPoints[ FarTopRight ] );
   mPlanes[ PlaneFar ].intersect( mPlanes[ PlaneBottom ], mPlanes[ PlaneLeft ], &mPoints[ FarBottomLeft ] );
   mPlanes[ PlaneFar ].intersect( mPlanes[ PlaneBottom ], mPlanes[ PlaneRight ], &mPoints[ FarBottomRight ] );
   */

   // Update the axis aligned bounding box.
   _updateBounds();
}
#endif

//-----------------------------------------------------------------------------

void Frustum::setNearDist( F32 nearDist )
{
   setNearFarDist( nearDist, mFarDist );
}

//-----------------------------------------------------------------------------

void Frustum::setFarDist( F32 farDist )
{
   setNearFarDist( mNearDist, farDist );
}

//-----------------------------------------------------------------------------

void Frustum::setNearFarDist( F32 nearDist, F32 farDist )
{
   if( mNearDist == nearDist && mFarDist == farDist )
      return;

   // Recalculate the frustum.
   MatrixF xfm( mTransform );

   const F32 CENTER_EPSILON = 0.001;
   F32 centerX = mNearLeft + (mNearRight - mNearLeft) * 0.5;
   F32 centerY = mNearBottom + (mNearTop - mNearBottom) * 0.5;
   if ((centerX > CENTER_EPSILON || centerX < -CENTER_EPSILON) || (centerY > CENTER_EPSILON || centerY < -CENTER_EPSILON) )
   {
      // Off-center projection, so re-calc use the new distances
      FovPort expectedFovPort;
      expectedFovPort.leftTan = -(mNearLeft / mNearDist);
      expectedFovPort.rightTan = (mNearRight / mNearDist);
      expectedFovPort.upTan = (mNearTop / mNearDist);
      expectedFovPort.downTan = -(mNearBottom / mNearDist);
      MathUtils::makeFovPortFrustum(this, mIsOrtho, nearDist, farDist, expectedFovPort);
   }
   else
   {
      // Projection is not off-center, use the normal code
      set(mIsOrtho, getFov(), getAspectRatio(), nearDist, farDist, xfm);
   }
}

//-----------------------------------------------------------------------------

void Frustum::cropNearFar(F32 newNearDist, F32 newFarDist)
{
   const F32 newOverOld = newNearDist / mNearDist;

   set( mIsOrtho, mNearLeft * newOverOld, mNearRight * newOverOld, mNearTop * newOverOld, mNearBottom * newOverOld, 
      newNearDist, newFarDist, mTransform);
}

//-----------------------------------------------------------------------------

bool Frustum::bakeProjectionOffset()
{
   // Nothing to bake if ortho
   if( mIsOrtho )
      return false;

   // Nothing to bake if no offset
   if( mProjectionOffset.isZero() )
      return false;

   // Near plane points in camera space
   Point3F np[4];
   np[0].set( mNearLeft, mNearDist, mNearTop );       // NearTopLeft
   np[1].set( mNearRight, mNearDist, mNearTop );      // NearTopRight
   np[2].set( mNearLeft, mNearDist, mNearBottom );    // NearBottomLeft
   np[3].set( mNearRight, mNearDist, mNearBottom );   // NearBottomRight

   // Generate the near plane
   PlaneF nearPlane( np[0], np[1], np[3] );

   // Far plane points in camera space
   const F32 farOverNear = mFarDist / mNearDist;
   Point3F fp0( mNearLeft * farOverNear, mFarDist, mNearTop * farOverNear );     // FarTopLeft
   Point3F fp1( mNearRight * farOverNear, mFarDist, mNearTop * farOverNear );    // FarTopRight
   Point3F fp2( mNearLeft * farOverNear, mFarDist, mNearBottom * farOverNear );  // FarBottomLeft
   Point3F fp3( mNearRight * farOverNear, mFarDist, mNearBottom * farOverNear ); // FarBottomRight

   // Generate the far plane
   PlaneF farPlane( fp0, fp1, fp3 );

   // The offset camera point
   Point3F offsetCamera( mProjectionOffset.x, 0.0f, mProjectionOffset.y );

   // The near plane point we'll be using for our calculations below
   U32 nIndex = 0;
   if( mProjectionOffset.x < 0.0 )
   {
      // Offset to the left so we'll need to use the near plane point on the right
      nIndex = 1;
   }
   if( mProjectionOffset.y > 0.0 )
   {
      // Offset to the top so we'll need to use the near plane point at the bottom
      nIndex += 2;
   }

   // Begin by calculating the offset point on the far plane as it goes
   // from the offset camera to the edge of the near plane.
   Point3F farPoint;
   Point3F fdir = np[nIndex] - offsetCamera;
   fdir.normalize();
   if( farPlane.intersect(offsetCamera, fdir, &farPoint) )
   {
      // Calculate the new near plane edge from the non-offset camera position
      // to the far plane point from above.
      Point3F nearPoint;
      Point3F ndir = farPoint;
      ndir.normalize();
      if( nearPlane.intersect( Point3F::Zero, ndir, &nearPoint) )
      {
         // Handle a x offset
         if( mProjectionOffset.x < 0.0 )
         {
            // The new near plane right side
            mNearRight = nearPoint.x;
         }
         else if( mProjectionOffset.x > 0.0 )
         {
            // The new near plane left side
            mNearLeft = nearPoint.x;
         }

         // Handle a y offset
         if( mProjectionOffset.y < 0.0 )
         {
            // The new near plane top side
            mNearTop = nearPoint.y;
         }
         else if( mProjectionOffset.y > 0.0 )
         {
            // The new near plane bottom side
            mNearBottom = nearPoint.y;
         }
      }
   }

   mDirty = true;

   // Indicate that we've modified the frustum
   return true;
}

//-----------------------------------------------------------------------------

void FrustumData::_update() const
{
   if( !mDirty )
      return;

   PROFILE_SCOPE( Frustum_update );

   const Point3F& cameraPos = mPosition;

   // Build the frustum points in camera space first.

   if( mIsOrtho )
   {
      mPoints[ NearTopLeft ].set( mNearLeft, mNearDist, mNearTop );
      mPoints[ NearTopRight ].set( mNearRight, mNearDist, mNearTop );
      mPoints[ NearBottomLeft ].set( mNearLeft, mNearDist, mNearBottom );
      mPoints[ NearBottomRight ].set( mNearRight, mNearDist, mNearBottom );
      mPoints[ FarTopLeft ].set( mNearLeft, mFarDist, mNearTop );
      mPoints[ FarTopRight ].set( mNearRight, mFarDist, mNearTop );
      mPoints[ FarBottomLeft ].set( mNearLeft, mFarDist, mNearBottom );
      mPoints[ FarBottomRight ].set( mNearRight, mFarDist, mNearBottom );
   }
   else
   {
      const F32 farOverNear = mFarDist / mNearDist;

      mPoints[ NearTopLeft ].set( mNearLeft, mNearDist, mNearTop );
      mPoints[ NearTopRight ].set( mNearRight, mNearDist, mNearTop );
      mPoints[ NearBottomLeft ].set( mNearLeft, mNearDist, mNearBottom );
      mPoints[ NearBottomRight ].set( mNearRight, mNearDist, mNearBottom );
      mPoints[ FarTopLeft ].set( mNearLeft * farOverNear, mFarDist, mNearTop * farOverNear );
      mPoints[ FarTopRight ].set( mNearRight * farOverNear, mFarDist, mNearTop * farOverNear );
      mPoints[ FarBottomLeft ].set( mNearLeft * farOverNear, mFarDist, mNearBottom * farOverNear );
      mPoints[ FarBottomRight ].set( mNearRight * farOverNear, mFarDist, mNearBottom * farOverNear );
   }

   // Transform the points into the desired culling space.

   for( U32 i = 0; i < mPoints.size(); ++ i )
      mTransform.mulP( mPoints[ i ] );

   // Update the axis aligned bounding box from 
   // the newly transformed points.

   mBounds = Box3F::aroundPoints( mPoints.address(), mPoints.size() );

   // Finally build the planes.

   if( mIsOrtho )
   {
      mPlanes[ PlaneLeft ].set(   mPoints[ NearBottomLeft ], 
                                  mPoints[ FarTopLeft ], 
                                  mPoints[ FarBottomLeft ] );

      mPlanes[ PlaneRight ].set(  mPoints[ NearTopRight ], 
                                  mPoints[ FarBottomRight ], 
                                  mPoints[ FarTopRight ] );

      mPlanes[ PlaneTop ].set(    mPoints[ FarTopRight ], 
                                  mPoints[ NearTopLeft ], 
                                  mPoints[ NearTopRight ] );

      mPlanes[ PlaneBottom ].set( mPoints[ NearBottomRight ], 
                                  mPoints[ FarBottomLeft ], 
                                  mPoints[ FarBottomRight ] );

      mPlanes[ PlaneNear ].set(   mPoints[ NearTopLeft ], 
                                  mPoints[ NearBottomLeft ], 
                                  mPoints[ NearTopRight ] );

      mPlanes[ PlaneFar ].set(    mPoints[ FarTopLeft ], 
                                  mPoints[ FarTopRight ], 
                                  mPoints[ FarBottomLeft ] );
   }
   else
   {
      mPlanes[ PlaneLeft ].set(   cameraPos,
                                  mPoints[ NearTopLeft ], 
                                  mPoints[ NearBottomLeft ] );

      mPlanes[ PlaneRight ].set(  cameraPos,
                                  mPoints[ NearBottomRight ], 
                                  mPoints[ NearTopRight ] );

      mPlanes[ PlaneTop ].set(    cameraPos,
                                  mPoints[ NearTopRight ], 
                                  mPoints[ NearTopLeft ] );

      mPlanes[ PlaneBottom ].set( cameraPos,
                                  mPoints[ NearBottomLeft ], 
                                  mPoints[ NearBottomRight ] );

      mPlanes[ PlaneNear ].set(   mPoints[ NearTopLeft ],
                                  mPoints[ NearBottomLeft ], 
                                  mPoints[ NearTopRight ] );

      mPlanes[ PlaneFar ].set(    mPoints[ FarTopLeft ],
                                  mPoints[ FarTopRight ], 
                                  mPoints[ FarBottomLeft ] );
   }

   // If the frustum plane orientation doesn't match mIsInverted
   // now, invert all the plane normals.
   //
   // Note that if we have a transform matrix with a negative scale,
   // then the initial planes we have computed will always be inverted.

   const bool inverted = mPlanes[ PlaneNear ].whichSide( cameraPos ) == PlaneF::Front;
   if( inverted != mIsInverted )
   {
      for( U32 i = 0; i < mPlanes.size(); ++ i )
         mPlanes[ i ].invert();
   }

   AssertFatal( mPlanes[ PlaneNear ].whichSide( cameraPos ) != PlaneF::Front,
      "Frustum::_update - Viewpoint lies on front side of near plane!" );

   // And now the center points which are mostly just used in debug rendering.

   mPlaneCenters[ PlaneLeftCenter ] = (   mPoints[ NearTopLeft ] + 
                                          mPoints[ NearBottomLeft ] + 
                                          mPoints[ FarTopLeft ] + 
                                          mPoints[ FarBottomLeft ] ) / 4.0f;

   mPlaneCenters[ PlaneRightCenter ] = (  mPoints[ NearTopRight ] + 
                                          mPoints[ NearBottomRight ] + 
                                          mPoints[ FarTopRight ] + 
                                          mPoints[ FarBottomRight ] ) / 4.0f;

   mPlaneCenters[ PlaneTopCenter ] = ( mPoints[ NearTopLeft ] + 
                                       mPoints[ NearTopRight ] + 
                                       mPoints[ FarTopLeft ] + 
                                       mPoints[ FarTopRight ] ) / 4.0f;

   mPlaneCenters[ PlaneBottomCenter ] = ( mPoints[ NearBottomLeft ] + 
                                          mPoints[ NearBottomRight ] + 
                                          mPoints[ FarBottomLeft ] + 
                                          mPoints[ FarBottomRight ] ) / 4.0f;

   mPlaneCenters[ PlaneNearCenter ] = (   mPoints[ NearTopLeft ] + 
                                          mPoints[ NearTopRight ] + 
                                          mPoints[ NearBottomLeft ] + 
                                          mPoints[ NearBottomRight ] ) / 4.0f;

   mPlaneCenters[ PlaneFarCenter ] = ( mPoints[ FarTopLeft ] + 
                                       mPoints[ FarTopRight ] + 
                                       mPoints[ FarBottomLeft ] + 
                                       mPoints[ FarBottomRight ] ) / 4.0f;

   // Done.

   mDirty = false;
}

//-----------------------------------------------------------------------------

void Frustum::invert()
{
   mIsInverted = !mIsInverted;
   _update();
}

//-----------------------------------------------------------------------------

void Frustum::setTransform( const MatrixF &mat )
{
	mTransform = mat;
   mPosition = mTransform.getPosition();
   mDirty = true;
}

//-----------------------------------------------------------------------------

void Frustum::scaleFromCenter( F32 scale )
{
   // Extract the fov and aspect ratio.
   F32 fovInRadians = mAtan2( (mNearTop - mNearBottom)*mNumTiles/2.0f, mNearDist ) * 2.0f;
   F32 aspectRatio = (mNearRight - mNearLeft)/(mNearTop - mNearBottom);

   // Now move the near and far planes out.
   F32 halfDist = ( mFarDist - mNearDist ) / 2.0f;
   mNearDist   -= halfDist * ( scale - 1.0f );
   mFarDist    += halfDist * ( scale - 1.0f );

   // Setup the new scaled frustum.
   set( mIsOrtho, fovInRadians, aspectRatio, mNearDist, mFarDist, mTransform );
}

//-----------------------------------------------------------------------------

void Frustum::mul( const MatrixF& mat )
{
   mTransform.mul( mat );
   mDirty = true;
}

//-----------------------------------------------------------------------------

void Frustum::mulL( const MatrixF& mat )
{
   MatrixF last( mTransform );
   mTransform.mul( mat, last );

   mDirty = true;
}

//-----------------------------------------------------------------------------

void Frustum::setProjectionOffset(const Point2F& offsetMat)
{
   mProjectionOffset = offsetMat;
   mProjectionOffsetMatrix.identity();
   mProjectionOffsetMatrix.setPosition(Point3F(mProjectionOffset.x, mProjectionOffset.y, 0.0f));
}

//-----------------------------------------------------------------------------

void Frustum::getProjectionMatrix( MatrixF *proj, bool gfxRotate ) const
{
   if (mIsOrtho)
   {
      MathUtils::makeOrthoProjection(proj, mNearLeft, mNearRight, mNearTop, mNearBottom, mNearDist, mFarDist, gfxRotate);
      proj->mulL(mProjectionOffsetMatrix);
   }
   else
   {
      MathUtils::makeProjection(proj, mNearLeft, mNearRight, mNearTop, mNearBottom, mNearDist, mFarDist, gfxRotate);
      proj->mulL(mProjectionOffsetMatrix);
   }
}

//-----------------------------------------------------------------------------

void Frustum::tileFrustum(U32 numTiles, const Point2I& curTile, Point2F overlap)
{
   //These will be stored to re-tile the frustum if needed
   mNumTiles = numTiles; 
   mCurrTile = curTile; 
   mTileOverlap = overlap;
   
   tile(&mNearLeft, &mNearRight, &mNearTop, &mNearBottom, mNumTiles, mCurrTile, mTileOverlap);
}

//-----------------------------------------------------------------------------

void Frustum::tile( F32 *left, F32 *right, F32 *top, F32 *bottom, U32 numTiles, const Point2I& curTile, Point2F overlap )
{
   if (numTiles == 1)
      return;

   Point2F tileSize( ( *right - *left ) / (F32)numTiles, 
                     ( *top - *bottom ) / (F32)numTiles );
   
   F32 leftOffset   = tileSize.x*overlap.x;
   F32 rightOffset  = tileSize.x*overlap.x*2;
   F32 bottomOffset = tileSize.y*overlap.y;
   F32 topOffset    = tileSize.y*overlap.y*2;

   *left += tileSize.x * curTile.x - leftOffset;
   *right = *left + tileSize.x + rightOffset;
   *bottom += tileSize.y * curTile.y - bottomOffset;
   *top = *bottom + tileSize.y + topOffset;
}
