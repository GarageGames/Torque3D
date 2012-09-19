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

#ifndef _MPLANE_H_
#define _MPLANE_H_

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif

#ifndef _MBOX_H_
#include "math/mBox.h"
#endif

#ifndef _MORIENTEDBOX_H_
#include "math/mOrientedBox.h"
#endif

#ifndef _MSPHERE_H_
#include "math/mSphere.h"
#endif


/// A 3D plane defined by a normal and a distance along the normal.
///
/// @note The distance is stored negative.
class PlaneF : public Point3F
{
   public:

      /// *NEGATIVE* distance along the xyz normal.
      F32 d;

      /// Return the plane's normal.
      const Point3F& getNormal() const { return *this; }

      /// Return the plane's position.
      Point3F getPosition() const { return Point3F( x, y, z ) * -d; }

      bool isHorizontal() const;
      bool isVertical() const;

      bool isParallelTo( const PlaneF& plane, F32 epsilon = POINT_EPSILON ) const;

      bool isPerpendicularTo( const PlaneF& plane, F32 epsilon = POINT_EPSILON ) const;

      /// @name Initialization
      /// @{

      PlaneF() {}
      PlaneF( const Point3F& p, const Point3F& n );
      /// NOTE: d is the NEGATIVE distance along the xyz normal.
      PlaneF( F32 _x, F32 _y, F32 _z, F32 _d);
      PlaneF( const Point3F& j, const Point3F& k, const Point3F& l );

      void set( F32 _x, F32 _y, F32 _z );
      /// NOTE: d is the NEGATIVE distance along the xyz normal.
      void set( F32 _x, F32 _y, F32 _z, F32 _d );
      void set( const Point3F& p, const Point3F& n);
      void set( const Point3F& k, const Point3F& j, const Point3F& l );
      void setPoint(const Point3F &p); // assumes the x,y,z fields are already set
                                           // creates an un-normalized plane

      void setXY(F32 zz);
      void setYZ(F32 xx);
      void setXZ(F32 yy);
      void setXY(const Point3F& P, F32 dir);
      void setYZ(const Point3F& P, F32 dir);
      void setXZ(const Point3F& P, F32 dir);

      /// @}

      void shiftX(F32 xx);
      void shiftY(F32 yy);
      void shiftZ(F32 zz);
      void invert();
      void neg();

      /// Return the distance of the given point from the plane.
      F32 distToPlane( const Point3F& cp ) const;

      /// Project the given point onto the plane.
      Point3F project(const Point3F &pt) const;

      /// @name Intersection
      /// @{

      enum Side
      {
         Front = 1,
         On    = 0,
         Back  = -1
      };

      /// Compute on which side of the plane the given point lies.
      Side whichSide( const Point3F& cp ) const;

      /// Compute which side the given sphere lies on.
      Side whichSide( const SphereF& sphere ) const;

      /// Compute which side the given AABB lies on.
      Side whichSide( const Box3F& aabb ) const;

      /// Compute which side the given OBB lies on.
      Side whichSide( const OrientedBox3F& obb ) const;

      /// Compute which side the given box lies on.
      ///
      /// @param center Center point.
      /// @param axisx X-Axis vector.  Length is box half-extent along X.
      /// @param axisy Y-Axis vector.  Length is box half-extent along Y.
      /// @param axisz Z-Axis vector.  Length is box half-extent along Z.
      ///
      /// @return Side that the given box is on.
      Side whichSideBox( const Point3F& center,
                         const Point3F& axisx,
                         const Point3F& axisy,
                         const Point3F& axisz ) const;

      /// @}

      /// @name Intersection
      /// @{

      /// Compute the distance at which the ray traveling from @a start in the direction
      /// of @end intersects the plane
      /// @param start Starting point of the ray.
      /// @param end Point in the direction of which the ray travels from @a start.
      /// @return The distance (as a fraction/multiple of the length of the vector between
      ///   @a start and @a end) at which the ray intersects the plane or PARALLEL_PLANE if the
      ///   ray is parallel to the plane.
      F32 intersect( const Point3F &start, const Point3F &end ) const;

      /// Compute the intersection of the ray that emanates at @a start in the direction
      /// @dir.
      /// @param start Point where the ray emanates.
      /// @param dir Direction in which the ray travels.  Must be normalized.
      /// @param outHit Resulting intersection point.  Only set if there indeed is a hit.
      /// @return True if the ray intersects the plane or false if not.
      bool intersect( const Point3F &start, const Point3F &dir, Point3F *outHit ) const;

      /// Compute the intersection between two planes.
      ///
      /// @param plane Plane to intersect with.
      /// @param outLineOrigin Used to store the origin of the resulting intersection line.
      /// @param outLineDirection Used to store the direction of the resulting intersection line.
      ///
      /// @return True if there is an intersection or false if the two planes are coplanar.
      bool intersect( const PlaneF& plane, Point3F& outLineOrigin, Point3F& outLineDirection ) const;

      /// @}

      /// @name Clipping
      /// @{

      /// Clip a convex polygon by the plane.
      ///
      /// The resulting polygon will be the input polygon minus the part on the negative side
      /// of the plane.  The input polygon must be convex and @a inVertices must be in CCW or CW order.
      ///
      /// @param inVertices Array holding the vertices of the input polygon.
      /// @param inNumVertices Number of vertices in @a inVertices.
      /// @param outVertices Array to hold the vertices of the clipped polygon.  Must have space for one additional
      ///   vertex in case the polygon is split by the plane such that an additional vertex appears.  Must not
      ///   be the same as @a inVertices.
      /// @return Number of vertices in the clipped polygon, i.e. number of vertices in @a outVertices.
      ///
      /// @note Be aware that if the polygon fully lies on the negative side of the plane,
      ///   the resulting @a outNumVertices will be zero, i.e. no polygon will result from the clip.
      U32 clipPolygon( const Point3F* inVertices, U32 inNumVertices, Point3F* outVertices ) const;

      /// Clip a line segment by the plane.
      ///
      /// @param start Start point of the line segment.
      /// @param end End point of the line segment.
      /// @param outNewEnd New end point if there is an intersection with the plane.
      ///
      /// @return True
      bool clipSegment( const Point3F& start, const Point3F& end, Point3F& outNewEnd ) const;

      /// @}
};

#define PARALLEL_PLANE  1e20f

#define PlaneSwitchCode(s, e) (s * 3 + e)


//---------------------------------------------------------------------------

inline PlaneF::PlaneF( F32 _x, F32 _y, F32 _z, F32 _d )
{
   set( _x, _y, _z, _d );
}

//-----------------------------------------------------------------------------

inline PlaneF::PlaneF( const Point3F& p, const Point3F& n )
{
   set(p,n);
}

//-----------------------------------------------------------------------------

inline PlaneF::PlaneF( const Point3F& j, const Point3F& k, const Point3F& l )
{
   set(j,k,l);
}

//-----------------------------------------------------------------------------

inline void PlaneF::setXY( F32 zz )
{
   x = y = 0.0f; z = 1.0f; d = -zz;
}

//-----------------------------------------------------------------------------

inline void PlaneF::setYZ( F32 xx )
{
   x = 1.0f; z = y = 0.0f; d = -xx;
}

//-----------------------------------------------------------------------------

inline void PlaneF::setXZ( F32 yy )
{
   x = z = 0.0f; y = 1.0f; d = -yy;
}

//-----------------------------------------------------------------------------

inline void PlaneF::setXY(const Point3F& point, F32 dir)       // Normal = (0, 0, -1|1)
{
   x = y = 0.0f;
   d = -((z = dir) * point.z);
}

//-----------------------------------------------------------------------------

inline void PlaneF::setYZ(const Point3F& point, F32 dir)       // Normal = (-1|1, 0, 0)
{
   z = y = 0.0f;
   d = -((x = dir) * point.x);
}

//-----------------------------------------------------------------------------

inline void PlaneF::setXZ(const Point3F& point, F32 dir)       // Normal = (0, -1|1, 0)
{
   x = z = 0.0f;
   d = -((y = dir) * point.y);
}

//-----------------------------------------------------------------------------

inline void PlaneF::shiftX( F32 xx )
{
   d -= xx * x;
}

//-----------------------------------------------------------------------------

inline void PlaneF::shiftY( F32 yy )
{
   d -= yy * y;
}

//-----------------------------------------------------------------------------

inline void PlaneF::shiftZ( F32 zz )
{
   d -= zz * z;
}

//-----------------------------------------------------------------------------

inline bool PlaneF::isHorizontal() const
{
   return (x == 0.0f && y == 0.0f) ? true : false;
}

//-----------------------------------------------------------------------------

inline bool PlaneF::isVertical() const
{
    return ((x != 0.0f || y != 0.0f) && z == 0.0f) ? true : false;
}

//-----------------------------------------------------------------------------

inline Point3F PlaneF::project(const Point3F &pt) const
{
   F32 dist = distToPlane(pt);
   return Point3F(pt.x - x * dist, pt.y - y * dist, pt.z - z * dist);
}

//-----------------------------------------------------------------------------

inline F32 PlaneF::distToPlane( const Point3F& cp ) const
{
   // return mDot(*this,cp) + d;
   return (x * cp.x + y * cp.y + z * cp.z) + d;
}

//-----------------------------------------------------------------------------

inline PlaneF::Side PlaneF::whichSide(const Point3F& cp) const
{
   F32 dist = distToPlane(cp);
   if (dist >= 0.005f)                 // if (mFabs(dist) < 0.005f)
      return Front;                    //    return On;
   else if (dist <= -0.005f)           // else if (dist > 0.0f)
      return Back;                     //    return Front;
   else                                // else
      return On;                       //    return Back;
}

//-----------------------------------------------------------------------------

inline PlaneF::Side PlaneF::whichSide( const SphereF& sphere ) const
{
   const F32 dist = distToPlane( sphere.center );
   if( dist > sphere.radius )
      return Front;
   else if( dist < - sphere.radius )
      return Back;
   else
      return On;
}

//-----------------------------------------------------------------------------

inline PlaneF::Side PlaneF::whichSide( const Box3F& aabb ) const
{
   // See Graphics Gems IV, 1.7 and "A Faster Overlap Test for a Plane and a Bounding Box"
   // (http://replay.waybackmachine.org/19981203032829/http://www.cs.unc.edu/~hoff/research/vfculler/boxplane.html)
   // for details.

   Point3F pVertex;
   Point3F nVertex;

   pVertex.x = ( x > 0.0f ) ? aabb.maxExtents.x : aabb.minExtents.x;
   pVertex.y = ( y > 0.0f ) ? aabb.maxExtents.y : aabb.minExtents.y;
   pVertex.z = ( z > 0.0f ) ? aabb.maxExtents.z : aabb.minExtents.z;

   if( whichSide( pVertex ) == Back )
      return Back;

   nVertex.x = ( x > 0.0f ) ? aabb.minExtents.x : aabb.maxExtents.x;
   nVertex.y = ( y > 0.0f ) ? aabb.minExtents.y : aabb.maxExtents.y;
   nVertex.z = ( z > 0.0f ) ? aabb.minExtents.z : aabb.maxExtents.z;

   if( whichSide( nVertex ) == Front )
      return Front;

   return On;
}

//-----------------------------------------------------------------------------

inline PlaneF::Side PlaneF::whichSide( const OrientedBox3F& obb ) const
{
   // Project the box onto the line defined by the plane center and normal.
   // See "3D Game Engine Design" chapter 4.3.2.

   const F32 r = obb.getHalfExtents().x * mFabs( mDot( obb.getAxis( 0 ), *this ) ) +
                 obb.getHalfExtents().y * mFabs( mDot( obb.getAxis( 1 ), *this ) ) +
                 obb.getHalfExtents().z * mFabs( mDot( obb.getAxis( 2 ), *this ) );

   const F32 dist = distToPlane( obb.getCenter() );
   if( dist > r )
      return Front;
   else if( dist < - r )
      return Back;
   else
      return On;
}

//-----------------------------------------------------------------------------

inline PlaneF::Side PlaneF::whichSideBox(const Point3F& center,
                                         const Point3F& axisx,
                                         const Point3F& axisy,
                                         const Point3F& axisz) const
{
   F32 baseDist = distToPlane(center);

   F32 compDist = mFabs(mDot(axisx, *this)) +
      mFabs(mDot(axisy, *this)) +
      mFabs(mDot(axisz, *this));

   if (baseDist >= compDist)
      return Front;
   else if (baseDist <= -compDist)
      return Back;
   else
      return On;
}

inline void PlaneF::set( F32 _x, F32 _y, F32 _z )
{
    Point3F::set(_x,_y,_z);
}

inline void PlaneF::set( F32 _x, F32 _y, F32 _z, F32 _d )
{
    Point3F::set(_x,_y,_z);
    d = _d;
}

//---------------------------------------------------------------------------
/// Calculate the coefficients of the plane passing through
/// a point with the given normal.
inline void PlaneF::setPoint(const Point3F &p)
{
   d = -(p.x * x + p.y * y + p.z * z);
}

inline void PlaneF::set( const Point3F& p, const Point3F& n )
{
   x = n.x; y = n.y; z = n.z;
   normalize();

   // Calculate the last plane coefficient.

   d = -(p.x * x + p.y * y + p.z * z);
}

//---------------------------------------------------------------------------
// Calculate the coefficients of the plane passing through
// three points.  Basically it calculates the normal to the three
// points then calculates a plane through the middle point with that
// normal.
inline void PlaneF::set( const Point3F& k, const Point3F& j, const Point3F& l )
{
   // Point3F kj, lj, pv;
   // kj = k - j;
   // lj = l - j;
   // mCross( kj, lj, &pv );
   // set(j, pv);

   // Inline for speed...
   const F32 ax = k.x-j.x;
   const F32 ay = k.y-j.y;
   const F32 az = k.z-j.z;
   const F32 bx = l.x-j.x;
   const F32 by = l.y-j.y;
   const F32 bz = l.z-j.z;
   x = ay*bz - az*by;
   y = az*bx - ax*bz;
   z = ax*by - ay*bx;
      
   m_point3F_normalize( (F32 *)(&x) );
   d = -(j.x * x + j.y * y + j.z * z);
}

inline void PlaneF::invert()
{
   x = -x;
   y = -y;
   z = -z;
   d = -d;
}

inline void PlaneF::neg()
{
   invert();
}

inline F32 PlaneF::intersect(const Point3F &p1, const Point3F &p2) const
{
   const F32 den = mDot(p2 - p1, *this);
   if( mIsZero( den ) )
      return PARALLEL_PLANE;
   return -distToPlane(p1) / den;
}

inline bool PlaneF::intersect( const Point3F &start, const Point3F &dir, Point3F *outHit ) const
{
   const F32 den = mDot( dir, *this );
   if ( mIsZero( den ) )
      return false;

   F32 dist = -distToPlane( start ) / den;
   *outHit = start + dir * dist;
   return true;
}

class PlaneD: public Point3D
{
public:
   /// NOTE: d is the NEGATIVE distance along the xyz normal.
   F64 d;

   PlaneD();
   PlaneD( const PlaneF& copy);
   PlaneD( const Point3D& p, const Point3D& n );
   /// NOTE: d is the NEGATIVE distance along the xyz normal.
   PlaneD( F64 _x, F64 _y, F64 _z, F64 _d);
   PlaneD( const Point3D& j, const Point3D& k, const Point3D& l );

   // Methods
   //using Point3D::set;
    void set(const F64 _x, const F64 _y, const F64 _z);

   void     set( const Point3D& p, const Point3D& n);
   void     set( const Point3D& k, const Point3D& j, const Point3D& l );
   void     setPoint(const Point3D &p); // assumes the x,y,z fields are already set
                                        // creates an un-normalized plane

   void     setXY(F64 zz);
   void     setYZ(F64 xx);
   void     setXZ(F64 yy);
   void     setXY(const Point3D& P, F64 dir);
   void     setYZ(const Point3D& P, F64 dir);
   void     setXZ(const Point3D& P, F64 dir);
   void     shiftX(F64 xx);
   void     shiftY(F64 yy);
   void     shiftZ(F64 zz);
   void     invert();
   void     neg();
   Point3D  project(const Point3D &pt) const; // projects the point onto the plane.

   F64      distToPlane( const Point3D& cp ) const;

   enum Side {
      Front = 1,
      On    = 0,
      Back  = -1
   };

   Side whichSide(const Point3D& cp) const;
   F64  intersect(const Point3D &start, const Point3D &end) const;
   //DLLAPI bool     split( const Poly3F& poly, Poly3F* front, Poly3F* back );

   bool     isHorizontal() const;
   bool     isVertical() const;

   Side whichSideBox(const Point3D& center,
                     const Point3D& axisx,
                     const Point3D& axisy,
                     const Point3D& axisz) const;
};
//#define PARALLEL_PLANE  1e20f

//#define PlaneSwitchCode(s, e) (s * 3 + e)


//---------------------------------------------------------------------------

inline PlaneD::PlaneD()
{
}

inline PlaneD::
   PlaneD( F64 _x, F64 _y, F64 _z, F64 _d )
{
   x = _x; y = _y; z = _z; d = _d;
}

inline PlaneD::PlaneD( const PlaneF& copy)
{
   x = copy.x; y = copy.y; z = copy.z; d = copy.d;
}

inline PlaneD::PlaneD( const Point3D& p, const Point3D& n )
{
   set(p,n);
}

inline PlaneD::PlaneD( const Point3D& j, const Point3D& k, const Point3D& l )
{
   set(j,k,l);
}

inline void PlaneD::setXY( F64 zz )
{
   x = y = 0; z = 1; d = -zz;
}

inline void PlaneD::setYZ( F64 xx )
{
   x = 1; z = y = 0; d = -xx;
}

inline void PlaneD::setXZ( F64 yy )
{
   x = z = 0; y = 1; d = -yy;
}

inline void PlaneD::setXY(const Point3D& point, F64 dir)       // Normal = (0, 0, -1|1)
{
   x = y = 0; 
   d = -((z = dir) * point.z);
}

inline void PlaneD::setYZ(const Point3D& point, F64 dir)       // Normal = (-1|1, 0, 0)
{
   z = y = 0; 
   d = -((x = dir) * point.x);
}

inline void PlaneD::setXZ(const Point3D& point, F64 dir)       // Normal = (0, -1|1, 0)
{
   x = z = 0; 
   d = -((y = dir) * point.y);
}

inline void PlaneD::shiftX( F64 xx )
{
   d -= xx * x;
}

inline void PlaneD::shiftY( F64 yy )
{
   d -= yy * y;
}

inline void PlaneD::shiftZ( F64 zz )
{
   d -= zz * z;
}

inline bool PlaneD::isHorizontal() const
{
   return (x == 0 && y == 0) ? true : false;
}

inline bool PlaneD::isVertical() const
{
    return ((x != 0 || y != 0) && z == 0) ? true : false;
}

inline Point3D PlaneD::project(const Point3D &pt) const
{
   F64 dist = distToPlane(pt);
   return Point3D(pt.x - x * dist, pt.y - y * dist, pt.z - z * dist);
}

inline F64 PlaneD::distToPlane( const Point3D& cp ) const
{
   // return mDot(*this,cp) + d;
   return (x * cp.x + y * cp.y + z * cp.z) + d;
}

inline PlaneD::Side PlaneD::whichSide(const Point3D& cp) const
{
   F64 dist = distToPlane(cp);
   if (dist >= 0.005f)                 // if (mFabs(dist) < 0.005f)
      return Front;                    //    return On;
   else if (dist <= -0.005f)           // else if (dist > 0.0f)
      return Back;                     //    return Front;
   else                                // else
      return On;                       //    return Back;
}

inline void PlaneD::set(const F64 _x, const F64 _y, const F64 _z)
{
    Point3D::set(_x,_y,_z); 
}

//---------------------------------------------------------------------------
// Calculate the coefficients of the plane passing through 
// a point with the given normal.

////inline void PlaneD::set( const Point3D& p, const Point3D& n )
inline void PlaneD::setPoint(const Point3D &p)
{
   d = -(p.x * x + p.y * y + p.z * z);
}

inline void PlaneD::set( const Point3D& p, const Point3D& n )
{
   x = n.x; y = n.y; z = n.z;
   normalize();

   // Calculate the last plane coefficient.

   d = -(p.x * x + p.y * y + p.z * z);
}

//---------------------------------------------------------------------------
// Calculate the coefficients of the plane passing through 
// three points.  Basically it calculates the normal to the three
// points then calculates a plane through the middle point with that
// normal.

inline void PlaneD::set( const Point3D& k, const Point3D& j, const Point3D& l )
{
//   Point3D kj,lj,pv;
//   kj = k;
//   kj -= j;
//   lj = l;
//   lj -= j;
//   mCross( kj, lj, &pv );
//   set(j,pv);

// Above ends up making function calls up the %*#...
// This is called often enough to be a little more direct
// about it (sqrt should become intrinsic in the future)...
   F64 ax = k.x-j.x;
   F64 ay = k.y-j.y;
   F64 az = k.z-j.z;
   F64 bx = l.x-j.x;
   F64 by = l.y-j.y;
   F64 bz = l.z-j.z;
   x = ay*bz - az*by;
   y = az*bx - ax*bz;
   z = ax*by - ay*bx;
   F64 squared = x*x + y*y + z*z;
   AssertFatal(squared != 0.0, "Error, no plane possible!");

   // In non-debug mode
   if (squared != 0) 
   {
      F64 invSqrt = 1.0 / (F64)mSqrt(x*x + y*y + z*z);
      x *= invSqrt;
      y *= invSqrt;
      z *= invSqrt;
      d = -(j.x * x + j.y * y + j.z * z);
   }
   else 
   {
      x = 0;
      y = 0;
      z = 1;
      d = -(j.x * x + j.y * y + j.z * z);
   }
}

inline void PlaneD::invert()
{
   x = -x;
   y = -y;
   z = -z;
   d = -d;
}

inline void PlaneD::neg()
{
   invert();
}

inline F64 PlaneD::intersect(const Point3D &p1, const Point3D &p2) const
{
   F64 den = mDot(p2 - p1, *this);
   if(den == 0)
      return PARALLEL_PLANE;
   return -distToPlane(p1) / den;
}

inline PlaneD::Side PlaneD::whichSideBox(const Point3D& center,
                                         const Point3D& axisx,
                                         const Point3D& axisy,
                                         const Point3D& axisz) const
{
   F64 baseDist = distToPlane(center);

   F64 compDist = mFabs(mDot(axisx, *this)) +
                  mFabs(mDot(axisy, *this)) +
                  mFabs(mDot(axisz, *this));

   // Intersects
   if (baseDist >= compDist)
      return Front;
   else if (baseDist <= -compDist)
      return Back;
   else
      return On;

//   if (compDist > mFabs(baseDist))
//      return On;
//   else
//      return baseDist < 0.0 ? Back : Front;
}

#endif // _MPLANE_H_
