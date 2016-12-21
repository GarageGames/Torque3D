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

#ifndef _MATHUTIL_FRUSTUM_H_
#define _MATHUTIL_FRUSTUM_H_

#ifndef _MPOLYHEDRON_H_
#include "math/mPolyhedron.h"
#endif

#ifndef _MBOX_H_
#include "math/mBox.h"
#endif

#ifndef _MPLANE_H_
#include "math/mPlane.h"
#endif

#ifndef _MMATRIX_H_
#include "math/mMatrix.h"
#endif

#ifndef _MQUAT_H_
#include "math/mQuat.h"
#endif

#ifndef _MSPHERE_H_
#include "math/mSphere.h"
#endif


//TODO: Specialize intersection tests for frustums using octant tests


class OrientedBox3F;

/// Advanced fov specification for oculus
struct FovPort
{
   float upTan;
   float downTan;
   float leftTan;
   float rightTan;
};

/// Polyhedron data for use by frustums.  Uses fixed-size vectors
/// and a static vector for the edge list as that never changes
/// between frustums.
struct FrustumData : public PolyhedronData
{
      enum
      {
         EdgeCount = 12
      };

      /// Indices for the planes in a frustum.
      ///
      /// Note the planes are ordered left, right, near, 
      /// far, top, bottom for getting early rejections
      /// from the typical horizontal scene.
      enum
      {
         PlaneLeft,
         PlaneRight,
         PlaneNear,
         PlaneFar,
         PlaneTop,
         PlaneBottom,

         /// The total number of frustum planes.
         PlaneCount
      };

      /// Indices for the corner points of the frustum.
      enum CornerPoints
      {
         NearTopLeft,
         NearTopRight,
         NearBottomLeft,
         NearBottomRight,
         FarTopLeft,
         FarTopRight,
         FarBottomLeft,
         FarBottomRight,

         /// Total number of corner points.
         CornerPointCount
      };

      /// Indices for the center points of the frustum planes.
      enum PlaneCenters 
      {
         PlaneLeftCenter,
         PlaneRightCenter,
         PlaneTopCenter,
         PlaneBottomCenter,
         PlaneNearCenter,
         PlaneFarCenter,
      };

      /// Used to mask out planes for testing.
      enum : U32
      {
         PlaneMaskLeft     = ( 1 << PlaneLeft ),
         PlaneMaskRight    = ( 1 << PlaneRight ),
         PlaneMaskTop      = ( 1 << PlaneTop ),
         PlaneMaskBottom   = ( 1 << PlaneBottom ),
         PlaneMaskNear     = ( 1 << PlaneNear ),
         PlaneMaskFar      = ( 1 << PlaneFar ),

         PlaneMaskAll      = 0xFFFFFFFF,
      };

      typedef FixedSizeVector< PlaneF, PlaneCount > PlaneListType;
      typedef FixedSizeVector< Point3F, CornerPointCount > PointListType;
      typedef FixedSizeVector< Edge, EdgeCount > EdgeListType;

   protected:

      /// @name Lazily Updated Data
      /// @{

      /// When true, points, planes and bounds must be re-calculated before use.
      mutable bool mDirty;

      mutable PlaneListType mPlanes;
      mutable PointListType mPoints;

      /// The center points of the individual faces of the frustum.
      mutable Point3F mPlaneCenters[ PlaneCount ];

      /// The clipping-space axis-aligned bounding box which contains
      /// the extents of the frustum.
      mutable Box3F mBounds;

      /// @}

      /// Static edge list.  Shared by all frustum polyhedrons
      /// since they are always constructed the same way.
      static EdgeListType smEdges;

      /// Determines whether this Frustum
      /// is orthographic or perspective.
      bool mIsOrtho;

      /// Whether the frustum is inverted, i.e. whether the planes are
      /// facing outwards rather than inwards.
      bool mIsInverted;

      /// Used to transform the frustum points from camera
      /// space into the desired clipping space.
      MatrixF mTransform;

      /// Camera position extracted from tarnsform.
      Point3F mPosition;

      /// The size of the near plane used to generate
      /// the frustum points and planes.
      F32 mNearLeft;
      F32 mNearRight;
      F32 mNearTop;
      F32 mNearBottom;
      F32 mNearDist;
      F32 mFarDist;

      /// Update the point and plane data from the current frustum settings.
      void _update() const;

      FrustumData()
         : mDirty( false ),
           mIsInverted( false ) {}

   public:

      /// @name Accessors
      /// @{

      /// Return the number of planes that a frustum has.
      static U32 getNumPlanes() { return PlaneCount; }

      /// Return the planes that make up the polyhedron.
      /// @note The normals of these planes are facing *inwards*.
      const PlaneF* getPlanes() const { _update(); return mPlanes.address(); }

      /// Return the number of corner points that a frustum has.
      static U32 getNumPoints() { return CornerPointCount; }

      ///
      const Point3F* getPoints() const { _update(); return mPoints.address(); }

      /// Return the number of edges that a frustum has.
      static U32 getNumEdges() { return EdgeCount; }

      /// Return the edge definitions for a frustum.
      static const Edge* getEdges() { return smEdges.address(); }

      /// @}

      operator AnyPolyhedron() const
      {
         return AnyPolyhedron(
            AnyPolyhedron::PlaneListType( const_cast< PlaneF* >( getPlanes() ), getNumPlanes() ),
            AnyPolyhedron::PointListType( const_cast< Point3F* >( getPoints() ), getNumPoints() ),
            AnyPolyhedron::EdgeListType( const_cast< Edge* >( getEdges() ), getNumEdges() )
         );
      }
};


/// This class implements a view frustum for use in culling scene objects and
/// rendering the scene.
///
/// @warn Frustums are always non-inverted by default which means that even if
///   the frustum transform applies a negative scale, the frustum will still be
///   non-inverted.
class Frustum : public PolyhedronImpl< FrustumData >
{
   public:

      typedef PolyhedronImpl< FrustumData > Parent;

   protected:

      /// @name Tiling
      /// @{

      /// Number of subdivisions.
      U32 mNumTiles;

      /// Current rendering tile.
      Point2I mCurrTile;

      /// Tile overlap percentage.
      Point2F mTileOverlap;

      /// @}

      /// Offset used for projection matrix calculations
      Point2F mProjectionOffset;

      /// The calculated projection offset matrix
      MatrixF mProjectionOffsetMatrix;

   public:

      /// @name Constructors
      /// @{

      /// Construct a non-inverted frustum.
      ///
      /// @note If the given transform has a negative scale, the plane
      ///   normals will automatically be inverted so that the frustum
      ///   will still be non-inverted.  Use invert() to actually cause
      ///   the frustum to be inverted.
      Frustum( bool orthographic = false,
               F32 nearLeft = -1.0f, 
               F32 nearRight = 1.0f, 
               F32 nearTop = 1.0f, 
               F32 nearBottom = -1.0f, 
               F32 nearDist = 0.1f, 
               F32 farDist = 1.0f,
               const MatrixF &transform = MatrixF( true ) );

      /// @}


      /// @name Operators
      /// @{

      bool operator==( const Frustum& frustum ) const
      {
         return ( ( mNearLeft == frustum.mNearLeft ) &&
            ( mNearTop == frustum.mNearTop ) &&
            ( mNearBottom == frustum.mNearBottom ) &&
            ( mNearDist == frustum.mNearDist ) &&
            ( mFarDist == frustum.mFarDist ) &&
            ( mProjectionOffset.x == frustum.mProjectionOffset.x ) &&
            ( mProjectionOffset.y == frustum.mProjectionOffset.y ) );

      }
      bool operator!=( const Frustum& frustum ) const { return !( *this == frustum ); }

      /// @}


      /// @name Initialization
      ///
      /// Functions used to initialize the frustum.
      ///
      /// @{
      
      /// Sets the frustum from the field of view, screen aspect
      /// ratio, and the near and far distances.  You can pass an
      /// matrix to transform the frustum.
      void set(   bool isOrtho,
                  F32 fovYInRadians, 
                  F32 aspectRatio, 
                  F32 nearDist, 
                  F32 farDist,
                  const MatrixF &mat = MatrixF( true ) );

      /// Sets the frustum from the near plane dimensions and
      /// near and far distances.
      void set(   bool isOrtho,
                  F32 nearLeft, 
                  F32 nearRight, 
                  F32 nearTop, 
                  F32 nearBottom, 
                  F32 nearDist, 
                  F32 farDist,
                  const MatrixF &transform = MatrixF( true ) );

      /// Sets the frustum by extracting the planes from a projection,
      /// view-projection, or world-view-projection matrix.
      //void set( const MatrixF& projMatrix, bool normalize );

      /// Changes the near distance of the frustum.
      void setNearDist( F32 nearDist );

      /// Changes the far distance of the frustum.
      void setFarDist( F32 farDist );

      /// Changes the near and far distance of the frustum.
      void setNearFarDist( F32 nearDist, F32 farDist );

      ///
      void cropNearFar(F32 newNearDist, F32 newFarDist);

      /// Returns the far clip distance used to create 
      /// the frustum planes.
      F32 getFarDist() const { return mFarDist; }

      /// Returns the far clip distance used to create 
      /// the frustum planes.
      F32 getNearDist() const { return mNearDist; }

      /// Return the camera-space minimum X coordinate on the near plane.
      F32 getNearLeft() const { return mNearLeft; }

      /// Return the camera-space maximum X coordinate on the near plane.
      F32 getNearRight() const { return mNearRight; }

      /// Return the camera-space maximum Z coordinate on the near plane.
      F32 getNearTop() const { return mNearTop; }

      /// Return the camera-space minimum Z coordinate on the near plane.
      F32 getNearBottom() const { return mNearBottom; }

      /// Return the camera-space width of the frustum.
      F32 getWidth() const { return mFabs( mNearRight - mNearLeft ); }

      /// Return the camera-space height of the frustum.
      F32 getHeight() const { return mFabs( mNearTop - mNearBottom ); }

      ///
      F32 getFov() const
      {
         F32 nonTiledHeight = getHeight()*mNumTiles;
         return mAtan2( nonTiledHeight/2.0f, mNearDist ) * 2.0f;
      }

      ///
      F32 getAspectRatio() const { return (mNearRight - mNearLeft)/(mNearTop - mNearBottom); }

      /// @}


      /// @name Transformation
      ///
      /// These functions for transforming the frustum from
      /// one space to another.
      ///
      /// @{

      /// Sets a new transform for the frustum.
      void setTransform( const MatrixF &transform );

      /// Returns the current transform matrix for the frustum.
      const MatrixF& getTransform() const { return mTransform; }

      /// Scales up the frustum from its center point.
      void scaleFromCenter( F32 scale );

      /// Transforms the frustum by F = F * mat.
      void mul( const MatrixF &mat );

      /// Transforms the frustum by F = mat * F.
      void mulL( const MatrixF &mat );

      /// Flip the plane normals which has the result
      /// of reversing the culling results.
      void invert();

      /// Returns true if the frustum planes point outwards.
      bool isInverted() const { return mIsInverted; }

      /// Returns the origin point of the frustum.
      const Point3F& getPosition() const { return mPosition; }

      /// Returns the axis aligned bounding box of the frustum
      /// points typically used for early rejection.
      const Box3F& getBounds() const { _update(); return mBounds; }

      // Does the frustum have a projection offset?
      bool hasProjectionOffset() const { return !mProjectionOffset.isZero(); }

      /// Get the offset used when calculating the projection matrix
      const Point2F& getProjectionOffset() const { return mProjectionOffset; }

      /// Get the offset matrix used when calculating the projection matrix
      const MatrixF& getProjectionOffsetMatrix() const { return mProjectionOffsetMatrix; }

      /// Set the offset used when calculating the projection matrix
      void setProjectionOffset(const Point2F& offsetMat);

      /// Clear any offset used when calculating the projection matrix
      void clearProjectionOffset() { mProjectionOffset.zero(); mProjectionOffsetMatrix.identity(); }

      /// Enlarges the frustum to contain the planes generated by a project offset, if any.
      /// Used by scene culling to ensure that all object are contained within the asymetrical frustum.
      bool bakeProjectionOffset();

      /// Generates a projection matrix from the frustum.
      void getProjectionMatrix( MatrixF *proj, bool gfxRotate=true ) const;

      /// Will update the frustum if it is dirty
      void update() { _update(); }
      /// @}

      /// @name Culling
      /// @{

      /// Return true if the contents of the given AABB can be culled.
      bool isCulled( const Box3F& aabb ) const { return ( testPotentialIntersection( aabb ) == GeometryOutside ); }

      /// Return true if the contents of the given OBB can be culled.
      bool isCulled( const OrientedBox3F& obb ) const { return ( testPotentialIntersection( obb ) == GeometryOutside ); }

      /// Return true if the contents of the given sphere can be culled.
      bool isCulled( const SphereF& sphere ) const { return ( testPotentialIntersection( sphere ) == GeometryOutside ); }

      /// @}

      /// @name Projection Type
      /// @{

      bool isOrtho() const { return mIsOrtho; }
      
      /// @}

      /// @name Tile settings
      /// @{

      U32 getNumTiles() const { return mNumTiles; }
      const Point2I& getCurTile() const { return mCurrTile; }
      void tileFrustum(U32 numTiles, const Point2I& curTile, Point2F overlap);
      static void tile( F32 *left, F32 *right, F32 *top, F32 *bottom, U32 numTiles, const Point2I& curTile, Point2F overlap );

      /// @}
};

#endif // _MATHUTIL_FRUSTUM_H_