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

#ifndef _MPOLYHEDRON_H_
#define _MPOLYHEDRON_H_

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif

#ifndef _MPLANE_H_
#include "math/mPlane.h"
#endif

#ifndef _MPLANESET_H_
#include "math/mPlaneSet.h"
#endif

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

#ifndef _TUNMANAGEDVECTOR_H_
#include "core/util/tUnmanagedVector.h"
#endif

#ifndef _TFIXEDSIZEVECTOR_H_
#include "core/util/tFixedSizeVector.h"
#endif

#ifndef _MCONSTANTS_H_
#include "math/mConstants.h"
#endif


/// @file
/// Templated polyhedron code to allow all code to use a central definition of polyhedrons and
/// their functionality (intersection, clipping, etc.) yet still maintain full control over how
/// to create and store their data.



struct PolyhedronUnmanagedVectorData;
template< typename Base > struct PolyhedronImpl;


/// The polyhedron type to which all other polyhedron types should be convertible.
typedef PolyhedronImpl< PolyhedronUnmanagedVectorData > AnyPolyhedron;



/// Base class for helping to abstract over how a polyhedron
/// stores and updates its data.
///
/// The PolyhedronData class hierarchy is designed to give users of PolyhedronImpl
/// maximum freedom in modifying those behaviors.  This leads to some duplicating
/// in the various PolyhedronData classes but it ultimately provides greater control.
///
/// All accesses to the data go through accessors on the base classes.  This gives
/// the base class the freedom to implement lazy updates, for example.
///
/// A given base implementation is also free to store additional data or derive extended
/// classes from the base classes expected for points (Point3F), edges (Edge), and planes
/// (PlaneF).  If a class does that, it loses the ability to trivially convert to
/// AnyPolyhedron, though.
struct PolyhedronData
{
      /// Winged edge.
      ///
      /// @note Must be oriented clockwise for face[0]!  This is important!
      struct Edge
      {
         /// Index into plane vector for the two planes that go through this
         /// edge.
         U32 face[ 2 ];

         /// Index into point vector for the beginning and end point of the edge.
         /// @note The vector "vertex[ 1 ] - vertex[ 0 ]" must be oriented such that
         ///   it defines a *clockwise* orientation for face[ 0 ].  This is important!
         U32 vertex[ 2 ];

         Edge() {}
         Edge( U32 face1, U32 face2, U32 vertex1, U32 vertex2 )
         {
            face[ 0 ] = face1;
            face[ 1 ] = face2;
            vertex[ 0 ] = vertex1;
            vertex[ 1 ] = vertex2;
         }
      };

      typedef Edge EdgeType;
      typedef PlaneF PlaneType;
      typedef Point3F PointType;

      template< typename Polyhedron >
      static void buildBoxData( Polyhedron& poly, const MatrixF& mat, const Box3F& box, bool invertNormals = false );
};

/// Polyhedron data stored in Vectors.
struct PolyhedronVectorData : public PolyhedronData
{
      typedef Vector< PlaneF > PlaneListType;
      typedef Vector< Point3F > PointListType;
      typedef Vector< Edge > EdgeListType;

      /// List of planes.  Note that by default, the normals facing *inwards*.
      PlaneListType planeList;

      /// List of vertices.
      PointListType pointList;

      /// List of edges.
      EdgeListType edgeList;

      PolyhedronVectorData()
      {
         VECTOR_SET_ASSOCIATION( pointList );
         VECTOR_SET_ASSOCIATION( planeList );
         VECTOR_SET_ASSOCIATION( edgeList );
      }

      /// @name Accessors
      /// @{

      /// Return the number of planes that make up this polyhedron.
      U32 getNumPlanes() const { return planeList.size(); }

      /// Return the planes that make up the polyhedron.
      /// @note The normals of these planes are facing *inwards*.
      PlaneF* getPlanes() const { return planeList.address(); }

      /// Return the number of points that this polyhedron has.
      U32 getNumPoints() const { return pointList.size(); }

      /// 
      Point3F* getPoints() const { return pointList.address(); }

      /// Return the number of edges that this polyhedron has.
      U32 getNumEdges() const { return edgeList.size(); }

      ///
      Edge* getEdges() const { return edgeList.address(); }

      /// @}

      /// Conversion to the common polyhedron type.
      operator AnyPolyhedron() const;

      void buildBox( const MatrixF& mat, const Box3F& box, bool invertNormals = false )
      {
         pointList.setSize( 8 );
         planeList.setSize( 6 );
         edgeList.setSize( 12 );

         buildBoxData( *this, mat, box, invertNormals );
      }

      /// Build a polyhedron from the given set of planes.
      void buildFromPlanes( const PlaneSetF& planes );
};

/// Polyhedron data stored as raw points with memory
/// being managed externally.
struct PolyhedronUnmanagedVectorData : public PolyhedronData
{
      typedef UnmanagedVector< PlaneF > PlaneListType;
      typedef UnmanagedVector< Point3F > PointListType;
      typedef UnmanagedVector< Edge > EdgeListType;

   protected:

      /// List of planes.  Note that by default, the normals facing *inwards*.
      PlaneListType planeList;

      /// List of vertices.
      PointListType pointList;

      /// List of edges.
      EdgeListType edgeList;

   public:

      /// @name Accessors
      /// @{

      /// Return the number of planes that make up this polyhedron.
      U32 getNumPlanes() const { return planeList.size(); }

      /// Return the planes that make up the polyhedron.
      /// @note The normals of these planes are facing *inwards*.
      const PlaneF* getPlanes() const { return planeList.address(); }
      PlaneF* getPlanes() { return planeList.address(); }

      /// Return the number of points that this polyhedron has.
      U32 getNumPoints() const { return pointList.size(); }

      /// 
      const Point3F* getPoints() const { return pointList.address(); }
      Point3F* getPoints() { return pointList.address(); }

      /// Return the number of edges that this polyhedron has.
      U32 getNumEdges() const { return edgeList.size(); }

      ///
      const Edge* getEdges() const { return edgeList.address(); }
      Edge* getEdges() { return edgeList.address(); }

      /// @}
};

/// Polyhedron data stored in fixed size arrays.
template< int NUM_PLANES, int NUM_POINTS, int NUM_EDGES >
struct PolyhedronFixedVectorData : public PolyhedronData
{
      typedef FixedSizeVector< PlaneF, NUM_PLANES > PlaneListType;
      typedef FixedSizeVector< Point3F, NUM_POINTS > PointListType;
      typedef FixedSizeVector< Edge, NUM_EDGES > EdgeListType;

   protected:

      /// List of planes.  Note that by default, the normals facing *inwards*.
      PlaneListType planeList;

      /// List of vertices.
      PointListType pointList;

      /// List of edges.
      EdgeListType edgeList;

   public:


      /// @name Accessors
      /// @{

      /// Return the number of planes that make up this polyhedron.
      U32 getNumPlanes() const { return planeList.size(); }

      /// Return the planes that make up the polyhedron.
      /// @note The normals of these planes are facing *inwards*.
      PlaneF* getPlanes() const { return planeList.address(); }

      /// Return the number of points that this polyhedron has.
      U32 getNumPoints() const { return pointList.size(); }

      /// 
      Point3F* getPoints() const { return pointList.address(); }

      /// Return the number of edges that this polyhedron has.
      U32 getNumEdges() const { return edgeList.size(); }

      ///
      Edge* getEdges() const { return edgeList.address(); }

      /// @}

      /// Conversion to the common polyhedron type.
      operator AnyPolyhedron() const;
};


/// A polyhedron.
///
/// Polyhedrons are stored as both sets of planes as well sets of edges and vertices (basically
/// a winged-edge format).
///
/// Polyhedrons must be convex.
///
/// @note The default orientation for the plane normals of a polyhedron is *inwards*.
template< typename Base = PolyhedronVectorData >
struct PolyhedronImpl : public Base
{
      typedef typename Base::Edge Edge;

      typedef typename Base::PlaneListType PlaneListType;
      typedef typename Base::PointListType PointListType;
      typedef typename Base::EdgeListType EdgeListType;

      /// Construct an empty polyhedron.
      PolyhedronImpl() {}

      /// Construct a polyhedron described by the given planes and edges.
      PolyhedronImpl( PlaneListType planes, PointListType points, EdgeListType edges )
      {
         this->planeList = planes;
         this->pointList = points;
         this->edgeList = edges;
      }

      /// Return the AABB around the polyhedron.
      Box3F getBounds() const
      {
         return Box3F::aroundPoints( this->getPoints(), this->getNumPoints() );
      }

      /// Return the median point of all points defined on the polyhedron.
      Point3F getCenterPoint() const;

      /// @name Transform
      /// @{

      /// Transform the polyhedron using the given transform matrix and scale.
      void transform( const MatrixF& matrix, const Point3F& scale = Point3F::One );

      /// @}

      /// @name Containment
      /// @{

      /// @see PlaneSet::isContained(const Point3F&,F32)
      bool isContained( const Point3F& point, F32 epsilon = 0.f ) const
      {
         return PlaneSetF( this->getPlanes(), this->getNumPlanes() ).isContained( point, epsilon );
      }

      /// @see PlaneSet::isContained(const Point3F*,U32)
      bool isContained( const Point3F* points, U32 numPoints ) const
      {
         return PlaneSetF( this->getPlanes(), this->getNumPlanes() ).isContained( points, numPoints );
      }

      /// @see PlaneSet::isContained(const Box3F&)
      bool isContained( const Box3F& aabb ) const
      {
         return PlaneSetF( this->getPlanes(), this->getNumPlanes() ).isContained( aabb );
      }

      /// @see PlaneSet::isContained(const SphereF&)
      bool isContained( const SphereF& sphere ) const
      {
         return PlaneSetF( this->getPlanes(), this->getNumPlanes() ).isContained( sphere );
      }

      /// @see PlaneSet::isContained(const OrientedBox3F&)
      bool isContained( const OrientedBox3F& obb ) const
      {
         return PlaneSetF( this->getPlanes(), this->getNumPlanes() ).isContained( obb );
      }

      /// @}

      /// @name Intersection
      /// All of these intersection methods are approximate in that they can produce
      /// false positives on GeometryIntersecting.  For precise testing, use Intersector.
      /// @{

      /// @see PlaneSet::testPotentialIntersection(const Box3F&)
      OverlapTestResult testPotentialIntersection( const Box3F& aabb ) const
      {
         return PlaneSetF( this->getPlanes(), this->getNumPlanes() ).testPotentialIntersection( aabb );
      }

      /// @see PlaneSet::testPotentialIntersection(const SphereF&)
      OverlapTestResult testPotentialIntersection( const SphereF& sphere ) const
      {
         return PlaneSetF( this->getPlanes(), this->getNumPlanes() ).testPotentialIntersection( sphere );
      }

      /// @see PlaneSet::testPotentialIntersection(const OrientedBox3F&)
      OverlapTestResult testPotentialIntersection( const OrientedBox3F& obb ) const
      {
         return PlaneSetF( this->getPlanes(), this->getNumPlanes() ).testPotentialIntersection( obb );
      }

      /// @see PlaneSet::testPlanes
      U32 testPlanes( const Box3F& bounds, U32 planeMask = 0xFFFFFFFF, F32 expand = 0.0f ) const
      {
         return PlaneSetF( this->getPlanes(), this->getNumPlanes() ).testPlanes( bounds, planeMask, expand );
      }

      /// @}

      /// @name Clipping
      /// Functionality to clip other geometries against the polyhedron.
      /// @{

      /// @see PlaneSet::clipSegment
      bool clipSegment( Point3F& pnt0, Point3F& pnt1 ) const
      {
         return PlaneSetF( this->getPlanes(), this->getNumPlanes() ).clipSegment( pnt0, pnt1 );
      }

      /// @see PlaneSet::clipPolygon
      U32 clipPolygon( const Point3F* inVertices, U32 inNumVertices, Point3F* outVertices, U32 maxOutVertices ) const
      {
         return PlaneSetF( this->getPlanes(), this->getNumPlanes() ).clipPolygon( inVertices, inNumVertices, outVertices, maxOutVertices );
      }

      /// @}

      /// @name Construction
      /// Operations for constructing solids and polygons through boolean operations involving
      /// the polyhedron.
      /// @{

      /// Build the intersection of this polyhedron with the given plane.  The result is a
      /// polygon.
      ///
      /// @param plane Plane to intersect the polyhedron with.
      /// @param outPoints (out) Array where the resulting polygon points are stored.  A safe size is to
      ///   just allocate as many points as there are edges in the polyhedron.  If you know the maximum
      ///   number of vertices that can result from the intersection (for example, 4 for a box), then
      ///   it is ok to only allocate that much.
      /// @param maxOutPoints Number of points that can be stored in @a outPoints.  If insufficient, the
      ///   return value will be 0.
      ///
      /// @return The number of points written to @a outPoints.  If there is no intersection between the
      ///   given plane and the polyhedron, this will be zero.
      ///
      /// @note The resulting points will be ordered to form a proper polygon but there is no guarantee
      ///   on which direction the ordering is in compared to the plane.
      U32 constructIntersection( const PlaneF& plane, Point3F* outPoints, U32 maxOutPoints ) const;

      /// @}

      /// @name Extraction
      /// @{
      
      /// Extract the polygon for the given plane.
      ///
      /// The resulting indices will be CW ordered if the plane normals on the polyhedron are facing
      /// inwards and CCW ordered otherwise.
      ///
      /// @param plane Index of the plane on the polyhedron.
      /// @param outIndices Array where the resulting vertex indices will be stored.  Must have
      ///   enough room.  If you don't know the exact size that you need, just allocate one index
      ///   for any point in the mesh.
      /// @param maxOutIndices The number of indices that can be stored in @a outIndices.  If insufficient,
      ///   the return value will be 0.
      ///
      /// @return Number of indices written to @a outIndices.
      ///
      /// @note This method relies on correct CW ordering of edges with respect to face[0].
      template< typename IndexType >
      U32 extractFace( U32 plane, IndexType* outIndices, U32 maxOutIndices ) const;

      /// @}

   protected:

      template< typename P >
      OverlapTestResult _testOverlap( const P& bounds ) const;
};

/// Default polyhedron type.
typedef PolyhedronImpl<> Polyhedron;


//-----------------------------------------------------------------------------

inline PolyhedronVectorData::operator AnyPolyhedron() const
{
   return AnyPolyhedron(
      AnyPolyhedron::PlaneListType( getPlanes(), getNumPlanes() ),
      AnyPolyhedron::PointListType( getPoints(), getNumPoints() ),
      AnyPolyhedron::EdgeListType( getEdges(), getNumEdges() )
   );
}

//-----------------------------------------------------------------------------

template< int NUM_PLANES, int NUM_POINTS, int NUM_EDGES >
inline PolyhedronFixedVectorData< NUM_PLANES, NUM_POINTS, NUM_EDGES >::operator AnyPolyhedron() const
{
   return AnyPolyhedron(
      AnyPolyhedron::PlaneListType( getPlanes(), getNumPlanes() ),
      AnyPolyhedron::PointListType( getPoints(), getNumPoints() ),
      AnyPolyhedron::EdgeListType( getEdges(), getNumEdges() )
   );
}

#endif // !_MPOLYHEDRON_H_
