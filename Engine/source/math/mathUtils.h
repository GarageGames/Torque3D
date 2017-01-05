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

#ifndef _MATHUTILS_H_
#define _MATHUTILS_H_

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif

#ifndef _MMATRIX_H_
#include "math/mMatrix.h"
#endif

#ifndef _MRECT_H_
#include "math/mRect.h"
#endif

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

#ifndef _MATHUTIL_FRUSTUM_H_
#include "math/util/frustum.h"
#endif


class Box3F;
class RectI;
class Frustum;


/// Miscellaneous math utility functions.
namespace MathUtils
{
   /// A simple helper struct to define a line.
   struct Line
   {
      Point3F origin;
      VectorF direction;
   };

   /// A ray is also a line.
   typedef Line Ray;

   /// A simple helper struct to define a line segment.
   struct LineSegment
   {
      Point3F p0;
      Point3F p1;
   };   

   /// A simple helper struct to define a clockwise 
   /// winding quad.
   struct Quad
   {
      Point3F p00;
      Point3F p01;
      Point3F p10;
      Point3F p11;
   };

   /// Used by mTriangleDistance() to pass along collision info
   struct IntersectInfo
   {
      LineSegment    segment;    // Starts at given point, ends at collision
      Point3F        bary;       // Barycentric coords for collision
   };

   /// Rotate the passed vector around the world-z axis by the passed radians.
   void vectorRotateZAxis( Point3F &vector, F32 radians ); 
   void vectorRotateZAxis( F32 radians, Point3F *vectors, U32 count ); 

   /// Generates a projection matrix with the near plane
   /// moved forward by the bias amount.  This function is a helper primarily
   /// for working around z-fighting issues.
   ///
   /// @param bias      The amount to move the near plane forward.
   /// @param frustum   The frustum to generate the new projection matrix from.
   /// @param outMat    The resulting z-biased projection matrix.  Note: It must be initialized before the call.
   /// @param rotate    Optional parameter specifying whether to rotate the projection matrix similarly to GFXDevice.
   ///
   void getZBiasProjectionMatrix( F32 bias, const Frustum &frustum, MatrixF *outMat, bool rotate = true );

   /// Creates orientation matrix from a direction vector.  Assumes ( 0 0 1 ) is up.
   MatrixF createOrientFromDir( const Point3F &direction );

   /// Creates an orthonormal basis matrix with the unit length
   /// input vector in column 2 (up vector).
   ///
   /// @param up     The non-zero unit length up vector.
   /// @param outMat The output matrix which must be initialized prior to the call.
   ///
   void getMatrixFromUpVector( const VectorF &up, MatrixF *outMat );   

   /// Creates an orthonormal basis matrix with the unit length
   /// input vector in column 1 (forward vector).
   ///
   /// @param forward   The non-zero unit length forward vector.
   /// @param outMat    The output matrix which must be initialized prior to the call.
   ///
   void getMatrixFromForwardVector( const VectorF &forward, MatrixF *outMat );   

   /// Creates random direction given angle parameters similar to the particle system.
   ///
   /// The angles are relative to the specified axis. Both phi and theta are in degrees.
   Point3F randomDir( const Point3F &axis, F32 thetaAngleMin, F32 thetaAngleMax, F32 phiAngleMin = 0.0, F32 phiAngleMax = 360.0 );

   /// Returns a random 3D point within a sphere of the specified radius
   /// centered at the origin.
   Point3F randomPointInSphere( F32 radius );
   
   /// Returns a random 2D point within a circle of the specified radius
   /// centered at the origin.
   Point2F randomPointInCircle( F32 radius );

   /// Returns yaw and pitch angles from a given vector.
   ///
   /// Angles are in RADIANS.
   ///
   /// Assumes north is (0.0, 1.0, 0.0), the degrees move upwards clockwise.
   ///
   /// The range of yaw is 0 - 2PI.  The range of pitch is -PI/2 - PI/2.
   ///
   /// <b>ASSUMES Z AXIS IS UP</b>
   void getAnglesFromVector( const VectorF &vec, F32 &yawAng, F32 &pitchAng );

   /// Returns vector from given yaw and pitch angles.
   ///
   /// Angles are in RADIANS.
   ///
   /// Assumes north is (0.0, 1.0, 0.0), the degrees move upwards clockwise.
   ///
   /// The range of yaw is 0 - 2PI.  The range of pitch is -PI/2 - PI/2.
   ///
   /// <b>ASSUMES Z AXIS IS UP</b>
   void getVectorFromAngles( VectorF &vec, F32 yawAng, F32 pitchAng );

   /// Returns the angle between two given vectors
   /// 
   /// Angles is in RADIANS
   ///
   F32 getAngleBetweenVectors(VectorF vecA, VectorF vecB);


   /// Simple reflection equation - pass in a vector and a normal to reflect off of
   inline Point3F reflect( Point3F &inVec, Point3F &norm )
   {
      return inVec - norm * ( mDot( inVec, norm ) * 2.0f );
   }

   /// Collide two capsules (sphere swept lines) against each other, reporting only if they intersect or not.
   /// Based on routine from "Real Time Collision Detection" by Christer Ericson pp 114.
   bool capsuleCapsuleOverlap(const Point3F & a1, const Point3F & b1, F32 radius1, const Point3F & a2, const Point3F & b2, F32 radius2);
   
   /// Return capsule-sphere overlap.  Returns time of first overlap, where time
   /// is viewed as a sphere of radius radA moving from point A0 to A1.
   bool capsuleSphereNearestOverlap(const Point3F & A0, const Point3F A1, F32 radA, const Point3F & B, F32 radB, F32 & t);

   /// Intersect two line segments (p1,q1) and (p2,q2), returning points on lines (c1 & c2) and line parameters (s,t).
   /// Based on routine from "Real Time Collision Detection" by Christer Ericson pp 149.
   F32 segmentSegmentNearest(const Point3F & p1, const Point3F & q1, const Point3F & p2, const Point3F & q2, F32 & s, F32 & t, Point3F & c1, Point3F & c2);

   /// Transform bounding box making sure to keep original box entirely contained.
   void transformBoundingBox(const Box3F &sbox, const MatrixF &mat, const Point3F scale, Box3F &dbox);

   bool mProjectWorldToScreen(   const Point3F &in, 
                                 Point3F *out,
                                 const RectI &view,
                                 const MatrixF &world, 
                                 const MatrixF &projection );
   bool mProjectWorldToScreen(   const Point3F &in, 
                                 Point3F *out,
                                 const RectI &view,
                                 const MatrixF &worldProjection );

   void mProjectScreenToWorld(   const Point3F &in, 
                                 Point3F *out, 
                                 const RectI &view, 
                                 const MatrixF &world, 
                                 const MatrixF &projection, 
                                 F32 far, 
                                 F32 near);

   /// Clip @a inFrustum by the given polygon.
   ///
   /// @note The input polygon is limited to 58 vertices.
   ///
   /// @param points Polygon vertices.
   /// @param numPoints Number of vertices in @a points.
   /// @param viewport Screen viewport.  Note that this corresponds to the root frustum and not necessarily to @a inFrustum.
   /// @param world World->view transform.
   /// @param projection Projection matrix.
   /// @param inFrustum Frustum to clip.
   /// @param rootFrustum Frustum corresponding to @a viewport.
   /// @param outFrustum Resulting clipped frustum.
   ///
   /// @return True if the frustum was successfully clipped and @a outFrustum is valid, false otherwise
   ///   (if, for example, the input polygon is completely outside @a inFrustum).
   bool clipFrustumByPolygon(    const Point3F* points,
                                 U32 numPoints,
                                 const RectI& viewport,
                                 const MatrixF& world,
                                 const MatrixF& projection,
                                 const Frustum& inFrustum,
                                 const Frustum& rootFrustum,
                                 Frustum& outFrustum );

   /// Returns true if the test point is within the polygon.
   /// @param verts  The array of points which forms the polygon.
   /// @param vertCount The number of points in the polygon.
   /// @param testPt The point to test.
   bool pointInPolygon( const Point2F *verts, U32 vertCount, const Point2F &testPt );

   /// Remove all edges from the given polygon that have a total length shorter
   /// than @a epsilon.
   ///
   U32 removeShortPolygonEdges( const Point3F* verts, U32 vertCount, F32 epsilon );

   /// Calculates the shortest line segment between two lines.
   /// 
   /// @param outSegment   The result where .p0 is the point on line0 and .p1 is the point on line1.
   ///
   void mShortestSegmentBetweenLines( const Line &line0, const Line &line1, LineSegment *outSegment );
   
   /// Returns the greatest common divisor of two positive integers.
   U32 greatestCommonDivisor( U32 u, U32 v );
   
   /// Returns the barycentric coordinates and time of intersection between
   /// a line segment and a triangle.
   ///
   /// @param p1 The first point of the line segment.
   /// @param p2 The second point of the line segment.
   /// @param t1 The first point of the triangle.
   /// @param t2 The second point of the triangle.
   /// @param t2 The third point of the triangle.
   /// @param outUVW The optional output barycentric coords.
   /// @param outT The optional output time of intersection.
   ///
   /// @return Returns true if a collision occurs.
   ///
   bool mLineTriangleCollide( const Point3F &p1, const Point3F &p2, 
                              const Point3F &t1, const Point3F &t2, const Point3F &t3,
                              Point3F *outUVW = NULL,
                              F32 *outT = NULL );

   /// Returns the uv coords and time of intersection between 
   /// a ray and a quad.
   ///
   /// @param quad The quad.
   /// @param ray The ray.
   /// @param outUV The optional output UV coords of the intersection.
   /// @param outT The optional output time of intersection.
   ///
   /// @return Returns true if a collision occurs.
   ///
   bool mRayQuadCollide(   const Quad &quad, 
                           const Ray &ray, 
                           Point2F *outUV = NULL,
                           F32 *outT = NULL );

   /// Returns the distance between a point and triangle 'abc'.
   F32 mTriangleDistance( const Point3F &a, const Point3F &b, const Point3F &c, const Point3F &p, IntersectInfo* info=NULL );
   
   /// Returns the normal of the passed triangle 'abc'.
   /// 
   /// If we assume counter-clockwise triangle culling, normal will point
   /// out from the 'solid' side of the triangle.
   ///
   Point3F mTriangleNormal( const Point3F &a, const Point3F &b, const Point3F &c );

   /// Returns the closest point on the segment defined by
   /// points a, b to the point p.   
   Point3F mClosestPointOnSegment(  const Point3F &a, 
                                    const Point3F &b, 
                                    const Point3F &p );
  
	/// Sort the passed verts ( Point3F ) in a clockwise or counter-clockwise winding order.
	/// Verts must be co-planar and non-collinear.
	///
	/// @param quadMat	Transform matrix from vert space to quad space.
	/// @param clockwise	Sort clockwise or counter-clockwise
	/// @param verts		Array of Point3F verts.
	/// @param vertMap	Output - Array of vert element ids sorted by winding order.
	/// @param count		Element count of the verts and vertMap arrays which must be allocated prior to this call.				
	///
	void sortQuadWindingOrder( const MatrixF &quadMat, bool clockwise, const Point3F *verts, U32 *vertMap, U32 count );

   /// Same as above except we assume that the passed verts ( Point3F ) are already
   /// transformed into 'quad space'. If this was done correctly and the points
   /// are coplanar this means their z components will all be zero.
   void sortQuadWindingOrder( bool clockwise, const Point3F *verts, U32 *vertMap, U32 count );

   ///
   /// WORK IN PROGRESS
   ///
   /// Creates an orthonormal basis matrix from one, two, or three unit length
   /// input vectors. If more than one input vector is provided they must be 
   /// mutually perpendicular.
   ///
   /// @param rvec   Optional unit length right vector.
   /// @param fvec   Optional unit length forward vector.
   /// @param uvec   Optional unit length up vector.
   /// @param pos    Optional position to initialize the matrix.
   /// @param outMat The output matrix which must be initialized prior to the call.
   ///
   void buildMatrix( const VectorF *rvec, const VectorF *fvec, const VectorF *uvec, const VectorF *pos, MatrixF *outMat );

   ///
   bool reduceFrustum( const Frustum& frustum, const RectI& viewport, const RectF& area, Frustum& outFrustum );

   /// Build the frustum near plane dimensions from the parameters.
   void makeFrustum( F32 *outLeft,
                     F32 *outRight,
                     F32 *outTop,
                     F32 *outBottom,
                     F32 fovYInRadians, 
                     F32 aspectRatio, 
                     F32 nearPlane );

   void makeFovPortFrustum( Frustum *outFrustum,
                             bool isOrtho,
                             F32 nearDist,
                             F32 farDist,
                             const FovPort &inPort,
                             const MatrixF &transform = MatrixF(1) );

   /// Build a GFX projection matrix from the frustum parameters
   /// including the optional rotation required by GFX.
   void makeProjection( MatrixF *outMatrix, 
                        F32 fovYInRadians, 
                        F32 aspectRatio, 
                        F32 nearPlane, 
                        F32 farPlane,
                        bool gfxRotate );

   /// Build a projection matrix from the frustum near plane dimensions
   /// including the optional rotation required by GFX.
   void makeProjection( MatrixF *outMatrix, 
                        F32 left, 
                        F32 right, 
                        F32 top, 
                        F32 bottom, 
                        F32 nearPlane, 
                        F32 farPlane,
                        bool gfxRotate );

   /// Build an orthographic projection matrix from the frustum near
   /// plane dimensions including the optional rotation required by GFX.
   void makeOrthoProjection(  MatrixF *outMatrix, 
                              F32 left, 
                              F32 right, 
                              F32 top, 
                              F32 bottom, 
                              F32 nearPlane, 
                              F32 farPlane,
                              bool gfxRotate );

   /// Find the intersection of the line going from @a edgeA to @a edgeB with the triangle
   /// given by @a faceA, @a faceB, and @a faceC.
   /// @param edgeA Starting point of edge.
   /// @param edgeB End point of edge.
   /// @param faceA First vertex of triangle.
   /// @param faceB Second vertex of triangle.
   /// @param faceC Third vertex of triangle.
   /// @param intersection If there is an intersection, the point of intersection on the triangle's
   ///   face is stored here.
   /// @param True if there is an intersection, false otherwise.
   bool edgeFaceIntersect( const Point3F &edgeA, const Point3F &edgeB, 
      const Point3F &faceA, const Point3F &faceB, const Point3F &faceC, const Point3F &faceD, Point3F *intersection );

   /// Find out whether the given polygon is planar.
   /// @param vertices Array of vertices of the polygon.
   /// @param numVertices Number of vertices in @a vertices.
   /// @return True if the polygon is planar, false otherwise.
   bool isPlanarPolygon( const Point3F* vertices, U32 numVertices );

   /// Find out whether the given polygon is convex.
   /// @param vertices Array of vertices of the polygon.
   /// @param numVertices Number of vertices in @a vertices.
   /// @return True if the polygon is convex, false otherwise.
   bool isConvexPolygon( const Point3F* vertices, U32 numVertices );

   /// Extrude the given polygon along the given direction.
   U32 extrudePolygonEdges( const Point3F* vertices, U32 numVertices, const Point3F& direction, PlaneF* outPlanes );

   /// Extrude the edges of the given polygon away from @a fromPoint by constructing a set of planes
   /// that each go through @a fromPoint and a pair of vertices.
   ///
   /// The resulting planes are in the same order as the vertices and have their normals facing *inwards*,
   /// i.e. the resulting volume will enclose the polygon's interior space.
   ///
   /// @param vertices Vertices of the polygon in CCW or CW order (both are acceptable).
   /// @param numVertices Number of vertices in @a vertices.
   /// @param fromPoint
   /// @param outPlanes Array in which the resulting planes are stored.  Must have room for at least as many
   ///   planes are there are edges in the polygon.
   ///
   /// @return
   ///
   /// @note The input polygon does not necessarily need to be planar but it must be convex.
   U32 extrudePolygonEdgesFromPoint( const Point3F* vertices, U32 numVertices,
                                     const Point3F& fromPoint,
                                     PlaneF* outPlanes );

   //void findFarthestPoint( const Point3F* points, U32 numPoints, const Point3F& fromPoint, )

   /// Build a convex hull from a cloud of 2D points, first and last hull point are the same.
   void mBuildHull2D(const Vector<Point2F> inPoints, Vector<Point2F> &hullPoints);

} // namespace MathUtils

#endif // _MATHUTILS_H_
