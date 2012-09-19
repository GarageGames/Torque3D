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

#ifndef _MBOX_H_
#define _MBOX_H_

#ifndef _MBOXBASE_H_
#include "math/mBoxBase.h"
#endif

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif

#ifndef _MPOINT2_H_
#include "math/mPoint2.h"
#endif


class MatrixF;
class SphereF;



/// Axis-aligned bounding box (AABB).
///
/// A helper class for working with boxes. It runs at F32 precision.
///
/// @see Box3D
class Box3F : public BoxBase
{
   public:

      Point3F minExtents; ///< Minimum extents of box
      Point3F maxExtents; ///< Maximum extents of box

      Box3F() { }

      /// Create a box from two points.
      ///
      /// Normally, this function will compensate for mismatched
      /// min/max values. If you know your values are valid, you
      /// can set in_overrideCheck to true and skip this.
      ///
      /// @param   in_rMin          Minimum extents of box.
      /// @param   in_rMax          Maximum extents of box.
      /// @param   in_overrideCheck  Pass true to skip check of extents.
      Box3F( const Point3F& in_rMin, const Point3F& in_rMax, const bool in_overrideCheck = false );

      /// Create a box from six extent values.
      ///
      /// No checking is performed as to the validity of these
      /// extents, unlike the other constructor.
      Box3F( const F32 &xMin, const F32 &yMin, const F32 &zMin, 
             const F32 &xMax, const F32 &yMax, const F32 &zMax );

      Box3F(F32 cubeSize);

      void set( const Point3F& in_rMin, const Point3F& in_rMax );

      void set( const F32 &xMin, const F32 &yMin, const F32 &zMin, 
                const F32 &xMax, const F32 &yMax, const F32 &zMax );

      /// Create box around origin given lengths
      void set( const Point3F& in_Length );   

      /// Recenter the box
      void setCenter( const Point3F& center );

      /// Check to see if a point is contained in this box.
      bool isContained( const Point3F& in_rContained ) const;
      
      /// Check if the Point2F is within the box xy extents.
      bool isContained( const Point2F &pt ) const;

      /// Check to see if another box overlaps this box.
      bool isOverlapped( const Box3F&  in_rOverlap ) const;

      /// Check if the given sphere overlaps with the box.
      bool isOverlapped( const SphereF& sphere ) const;

      /// Check to see if another box is contained in this box.
      bool isContained( const Box3F& in_rContained ) const;

      /// Returns the length of the x extent.
      F32 len_x() const { return maxExtents.x - minExtents.x; }

      /// Returns the length of the y extent.
      F32 len_y() const  { return maxExtents.y - minExtents.y; }

      /// Returns the length of the z extent.
      F32 len_z() const  { return maxExtents.z - minExtents.z; }

      /// Returns the minimum box extent.
      F32 len_min() const { return getMin( len_x(), getMin( len_y(), len_z() ) ); }

      /// Returns the maximum box extent.
      F32 len_max() const { return getMax( len_x(), getMax( len_y(), len_z() ) ); }

      /// Returns the diagonal box length.
      F32 len() const { return ( maxExtents - minExtents ).len(); }

      /// Returns the length of extent by axis index.
      ///
      /// @param axis The axis index of 0, 1, or 2.
      ///
      F32 len( S32 axis ) const { return maxExtents[axis] - minExtents[axis]; }

      /// Returns true if any of the extent axes
      /// are less than or equal to zero.
      bool isEmpty() const { return len_x() <= 0.0f || len_y() <= 0.0f || len_z() <= 0.0f; }

      /// Perform an intersection operation with another box
      /// and store the results in this box.
      void intersect( const Box3F &in_rIntersect );
      void intersect( const Point3F &in_rIntersect );

      /// Return the overlap between this box and @a otherBox.
      Box3F getOverlap( const Box3F& otherBox ) const;

      /// Return the volume of the box.
      F32 getVolume() const;

      /// Get the center of this box.
      ///
      /// This is the average of min and max.
      void getCenter( Point3F *center ) const;
      Point3F getCenter() const;

      /// Returns the max minus the min extents.
      Point3F getExtents() const { return maxExtents - minExtents; }

      /// Collide a line against the box.
      ///
      /// @param   start   Start of line.
      /// @param   end     End of line.
      /// @param   t       Value from 0.0-1.0, indicating position
      ///                  along line of collision.
      /// @param   n       Normal of collision.
      bool collideLine( const Point3F &start, const Point3F &end, F32 *t, Point3F *n ) const;

      /// Collide a line against the box.
      ///
      /// Returns true on collision.
      bool collideLine( const Point3F &start, const Point3F &end ) const;

      /// Collide an oriented box against the box.
      ///
      /// Returns true if "oriented" box collides with us.
      /// Assumes incoming box is centered at origin of source space.
      ///
      /// @param   radii   The dimension of incoming box (half x,y,z length).
      /// @param   toUs    A transform that takes incoming box into our space.
      bool collideOrientedBox( const Point3F &radii, const MatrixF &toUs ) const;

      /// Check that the min extents of the box is
      /// less than or equal to the max extents.
      bool isValidBox() const { return (minExtents.x <= maxExtents.x) &&
                                       (minExtents.y <= maxExtents.y) &&
                                       (minExtents.z <= maxExtents.z); }

      /// Return the closest point of the box, relative to the passed point.
      Point3F getClosestPoint( const Point3F &refPt ) const;

      /// Return distance of closest point on box to refPt.
      F32 getDistanceToPoint( const Point3F &refPt ) const;

      /// Return the squared distance to  closest point on box to refPt.
      F32 getSqDistanceToPoint( const Point3F &refPt ) const;

      /// Return one of the corner vertices of the box.
      Point3F computeVertex( U32 corner ) const;

      /// Get the length of the longest diagonal in the box.
      F32 getGreatestDiagonalLength() const;

      /// Return the bounding sphere that contains this AABB.
      SphereF getBoundingSphere() const;

      /// Extend the box to include point.
      /// @see Invalid
      void extend( const Point3F &p );

      /// Scale the box by a Point3F or F32
      void scale( const Point3F &amt );
      void scale( F32 amt );
      
      /// Equality operator.
      bool operator ==( const Box3F &b ) const;
      
      /// Inequality operator.
      bool operator !=( const Box3F &b ) const;

      /// Create an AABB that fits around the given point cloud, i.e.
      /// find the minimum and maximum extents of the given point set.
      static Box3F aroundPoints( const Point3F* points, U32 numPoints );

   public:

      /// An inverted bounds where the minimum point is positive
      /// and the maximum point is negative.  Should be used with
      /// extend() to construct a minimum volume box.
      /// @see extend
      static const Box3F Invalid;

      /// A box that covers the entire floating point range.
      static const Box3F Max;

      /// A null box of zero size.
      static const Box3F Zero;
};

inline Box3F::Box3F(const Point3F& in_rMin, const Point3F& in_rMax, const bool in_overrideCheck)
 : minExtents(in_rMin),
   maxExtents(in_rMax)
{
   if (in_overrideCheck == false) {
      minExtents.setMin(in_rMax);
      maxExtents.setMax(in_rMin);
   }
}

inline Box3F::Box3F( const F32 &xMin, const F32 &yMin, const F32 &zMin, 
                    const F32 &xMax, const F32 &yMax, const F32 &zMax )
   : minExtents(xMin,yMin,zMin),
     maxExtents(xMax,yMax,zMax)
{
}

inline Box3F::Box3F(F32 cubeSize)
   : minExtents(-0.5f * cubeSize, -0.5f * cubeSize, -0.5f * cubeSize),
     maxExtents(0.5f * cubeSize, 0.5f * cubeSize, 0.5f * cubeSize)
{
}

inline void Box3F::set(const Point3F& in_rMin, const Point3F& in_rMax)
{
   minExtents.set(in_rMin);
   maxExtents.set(in_rMax);
}

inline void Box3F::set( const F32 &xMin, const F32 &yMin, const F32 &zMin, 
                        const F32 &xMax, const F32 &yMax, const F32 &zMax  )
{
   minExtents.set( xMin, yMin, zMin );
   maxExtents.set( xMax, yMax, zMax );
}

inline void Box3F::set(const Point3F& in_Length)
{
   minExtents.set(-in_Length.x * 0.5f, -in_Length.y * 0.5f, -in_Length.z * 0.5f);
   maxExtents.set( in_Length.x * 0.5f,  in_Length.y * 0.5f,  in_Length.z * 0.5f);
}

inline void Box3F::setCenter(const Point3F& center)
{
   F32 halflenx = len_x() * 0.5f;
   F32 halfleny = len_y() * 0.5f;
   F32 halflenz = len_z() * 0.5f;

   minExtents.set(center.x-halflenx, center.y-halfleny, center.z-halflenz);
   maxExtents.set(center.x+halflenx, center.y+halfleny, center.z+halflenz);
}

inline bool Box3F::isContained(const Point3F& in_rContained) const
{
   return (in_rContained.x >= minExtents.x && in_rContained.x < maxExtents.x) &&
          (in_rContained.y >= minExtents.y && in_rContained.y < maxExtents.y) &&
          (in_rContained.z >= minExtents.z && in_rContained.z < maxExtents.z);
}

inline bool Box3F::isContained( const Point2F &pt ) const
{
   return ( pt.x >= minExtents.x && pt.x < maxExtents.x ) &&
          ( pt.y >= minExtents.y && pt.y < maxExtents.y );      
}

inline bool Box3F::isOverlapped(const Box3F& in_rOverlap) const
{
   if (in_rOverlap.minExtents.x > maxExtents.x ||
       in_rOverlap.minExtents.y > maxExtents.y ||
       in_rOverlap.minExtents.z > maxExtents.z)
      return false;
   if (in_rOverlap.maxExtents.x < minExtents.x ||
       in_rOverlap.maxExtents.y < minExtents.y ||
       in_rOverlap.maxExtents.z < minExtents.z)
      return false;
   return true;
}

inline bool Box3F::isContained(const Box3F& in_rContained) const
{
   return (minExtents.x <= in_rContained.minExtents.x) &&
          (minExtents.y <= in_rContained.minExtents.y) &&
          (minExtents.z <= in_rContained.minExtents.z) &&
          (maxExtents.x >= in_rContained.maxExtents.x) &&
          (maxExtents.y >= in_rContained.maxExtents.y) &&
          (maxExtents.z >= in_rContained.maxExtents.z);
}

inline void Box3F::intersect(const Box3F& in_rIntersect)
{
   minExtents.setMin(in_rIntersect.minExtents);
   maxExtents.setMax(in_rIntersect.maxExtents);
}

inline void Box3F::intersect(const Point3F& in_rIntersect)
{
   minExtents.setMin(in_rIntersect);
   maxExtents.setMax(in_rIntersect);
}

inline Box3F Box3F::getOverlap( const Box3F& otherBox ) const
{
   Box3F overlap;

   for( U32 i = 0; i < 3; ++ i )
      if( minExtents[ i ] > otherBox.maxExtents[ i ] || otherBox.minExtents[ i ] > maxExtents[ i ] )
         overlap.minExtents[ i ] = 0.f;
      else
         overlap.minExtents[ i ] = getMax( minExtents[ i ], otherBox.minExtents[ i ] );

   return overlap;
}

inline F32 Box3F::getVolume() const
{
   return ( maxExtents.x - minExtents.x ) * ( maxExtents.y - minExtents.y ) * ( maxExtents.z - minExtents.z );
}

inline void Box3F::getCenter(Point3F* center) const
{
   center->x = (minExtents.x + maxExtents.x) * 0.5f;
   center->y = (minExtents.y + maxExtents.y) * 0.5f;
   center->z = (minExtents.z + maxExtents.z) * 0.5f;
}

inline Point3F Box3F::getCenter() const
{
   Point3F center;
   center.x = (minExtents.x + maxExtents.x) * 0.5f;
   center.y = (minExtents.y + maxExtents.y) * 0.5f;
   center.z = (minExtents.z + maxExtents.z) * 0.5f;
   return center;
}

inline Point3F Box3F::getClosestPoint(const Point3F& refPt) const
{
   Point3F closest;
   if      (refPt.x <= minExtents.x) closest.x = minExtents.x;
   else if (refPt.x >  maxExtents.x) closest.x = maxExtents.x;
   else                       closest.x = refPt.x;

   if      (refPt.y <= minExtents.y) closest.y = minExtents.y;
   else if (refPt.y >  maxExtents.y) closest.y = maxExtents.y;
   else                       closest.y = refPt.y;

   if      (refPt.z <= minExtents.z) closest.z = minExtents.z;
   else if (refPt.z >  maxExtents.z) closest.z = maxExtents.z;
   else                       closest.z = refPt.z;

   return closest;
}

inline F32 Box3F::getDistanceToPoint(const Point3F& refPt) const
{
   return mSqrt( getSqDistanceToPoint( refPt ) );
}

inline F32 Box3F::getSqDistanceToPoint( const Point3F &refPt ) const
{
   F32 sqDist = 0.0f;

   for ( U32 i=0; i < 3; i++ )
   {
      const F32 v = refPt[i];
      if ( v < minExtents[i] )
         sqDist += mSquared( minExtents[i] - v );
      else if ( v > maxExtents[i] )
         sqDist += mSquared( v - maxExtents[i] );
   }

   return sqDist;
}

inline void Box3F::extend(const Point3F & p)
{
#define EXTEND_AXIS(AXIS)    \
if (p.AXIS < minExtents.AXIS)       \
   minExtents.AXIS = p.AXIS;        \
else if (p.AXIS > maxExtents.AXIS)  \
   maxExtents.AXIS = p.AXIS;

   EXTEND_AXIS(x)
   EXTEND_AXIS(y)
   EXTEND_AXIS(z)

#undef EXTEND_AXIS
}

inline void Box3F::scale( const Point3F &amt )
{
   minExtents *= amt;
   maxExtents *= amt;
}

inline void Box3F::scale( F32 amt )
{
   minExtents *= amt;
   maxExtents *= amt;
}

inline bool Box3F::operator ==( const Box3F &b ) const
{
   return minExtents.equal( b.minExtents ) && maxExtents.equal( b.maxExtents );
}

inline bool Box3F::operator !=( const Box3F &b ) const
{
   return !minExtents.equal( b.minExtents ) || !maxExtents.equal( b.maxExtents );   
}

//------------------------------------------------------------------------------
/// Clone of Box3F, using 3D types.
///
/// 3D types use F64.
///
/// @see Box3F
class Box3D
{
  public:
   Point3D minExtents;
   Point3D maxExtents;

  public:
   Box3D() { }
   Box3D(const Point3D& in_rMin, const Point3D& in_rMax, const bool in_overrideCheck = false);

   bool isContained(const Point3D& in_rContained) const;
   bool isOverlapped(const Box3D&  in_rOverlap) const;

   F64 len_x() const;
   F64 len_y() const;
   F64 len_z() const;

   void intersect(const Box3D& in_rIntersect);
   void getCenter(Point3D* center) const;

   void extend(const Point3D & p);
};

inline Box3D::Box3D(const Point3D& in_rMin, const Point3D& in_rMax, const bool in_overrideCheck)
 : minExtents(in_rMin),
   maxExtents(in_rMax)
{
   if (in_overrideCheck == false) {
      minExtents.setMin(in_rMax);
      maxExtents.setMax(in_rMin);
   }
}

inline bool Box3D::isContained(const Point3D& in_rContained) const
{
   return (in_rContained.x >= minExtents.x && in_rContained.x < maxExtents.x) &&
          (in_rContained.y >= minExtents.y && in_rContained.y < maxExtents.y) &&
          (in_rContained.z >= minExtents.z && in_rContained.z < maxExtents.z);
}

inline bool Box3D::isOverlapped(const Box3D& in_rOverlap) const
{
   if (in_rOverlap.minExtents.x > maxExtents.x ||
       in_rOverlap.minExtents.y > maxExtents.y ||
       in_rOverlap.minExtents.z > maxExtents.z)
      return false;
   if (in_rOverlap.maxExtents.x < minExtents.x ||
       in_rOverlap.maxExtents.y < minExtents.y ||
       in_rOverlap.maxExtents.z < minExtents.z)
      return false;
   return true;
}

inline F64 Box3D::len_x() const
{
   return maxExtents.x - minExtents.x;
}

inline F64 Box3D::len_y() const
{
   return maxExtents.y - minExtents.y;
}

inline F64 Box3D::len_z() const
{
   return maxExtents.z - minExtents.z;
}

inline void Box3D::intersect(const Box3D& in_rIntersect)
{
   minExtents.setMin(in_rIntersect.minExtents);
   maxExtents.setMax(in_rIntersect.maxExtents);
}

inline void Box3D::getCenter(Point3D* center) const
{
   center->x = (minExtents.x + maxExtents.x) * 0.5;
   center->y = (minExtents.y + maxExtents.y) * 0.5;
   center->z = (minExtents.z + maxExtents.z) * 0.5;
}

inline void Box3D::extend(const Point3D & p)
{
#define EXTEND_AXIS(AXIS)    \
if (p.AXIS < minExtents.AXIS)       \
   minExtents.AXIS = p.AXIS;        \
else if (p.AXIS > maxExtents.AXIS)  \
   maxExtents.AXIS = p.AXIS;

   EXTEND_AXIS(x)
   EXTEND_AXIS(y)
   EXTEND_AXIS(z)

#undef EXTEND_AXIS
}


/// Bounding Box
///
/// A helper class for working with boxes. It runs at F32 precision.
///
/// @see Box3D
class Box3I
{
public:
   Point3I minExtents; ///< Minimum extents of box
   Point3I maxExtents; ///< Maximum extents of box

public:
   Box3I() { }

   /// Create a box from two points.
   ///
   /// Normally, this function will compensate for mismatched
   /// min/max values. If you know your values are valid, you
   /// can set in_overrideCheck to true and skip this.
   ///
   /// @param   in_rMin          Minimum extents of box.
   /// @param   in_rMax          Maximum extents of box.
   /// @param   in_overrideCheck  Pass true to skip check of extents.
   Box3I(const Point3I& in_rMin, const Point3I& in_rMax, const bool in_overrideCheck = false);

   /// Create a box from six extent values.
   ///
   /// No checking is performed as to the validity of these
   /// extents, unlike the other constructor.
   Box3I(S32 xmin, S32 ymin, S32 zmin, S32 max, S32 ymax, S32 zmax);

   /// Check to see if a point is contained in this box.
   bool isContained(const Point3I& in_rContained) const;

   /// Check to see if another box overlaps this box.
   bool isOverlapped(const Box3I&  in_rOverlap) const;

   /// Check to see if another box is contained in this box.
   bool isContained(const Box3I& in_rContained) const;

   S32 len_x() const;
   S32 len_y() const;
   S32 len_z() const;

   /// Perform an intersection operation with another box
   /// and store the results in this box.
   void intersect(const Box3I& in_rIntersect);

   /// Get the center of this box.
   ///
   /// This is the average of min and max.
   void getCenter(Point3I* center) const;

   /// Check that the box is valid.
   ///
   /// Currently, this just means that min < max.
   bool isValidBox() const 
   { 
      return (minExtents.x <= maxExtents.x) &&
         (minExtents.y <= maxExtents.y) &&
         (minExtents.z <= maxExtents.z); 
   }

   void extend(const Point3I & p);
};

inline Box3I::Box3I(const Point3I& in_rMin, const Point3I& in_rMax, const bool in_overrideCheck)
: minExtents(in_rMin),
maxExtents(in_rMax)
{
   if (in_overrideCheck == false) 
   {
      minExtents.setMin(in_rMax);
      maxExtents.setMax(in_rMin);
   }
}

inline Box3I::Box3I(S32 xMin, S32 yMin, S32 zMin, S32 xMax, S32 yMax, S32 zMax)
: minExtents(xMin,yMin,zMin),
maxExtents(xMax,yMax,zMax)
{
}

inline bool Box3I::isContained(const Point3I& in_rContained) const
{
   return (in_rContained.x >= minExtents.x && in_rContained.x < maxExtents.x) &&
      (in_rContained.y >= minExtents.y && in_rContained.y < maxExtents.y) &&
      (in_rContained.z >= minExtents.z && in_rContained.z < maxExtents.z);
}

inline bool Box3I::isOverlapped(const Box3I& in_rOverlap) const
{
   if (in_rOverlap.minExtents.x > maxExtents.x ||
      in_rOverlap.minExtents.y > maxExtents.y ||
      in_rOverlap.minExtents.z > maxExtents.z)
      return false;
   if (in_rOverlap.maxExtents.x < minExtents.x ||
      in_rOverlap.maxExtents.y < minExtents.y ||
      in_rOverlap.maxExtents.z < minExtents.z)
      return false;
   return true;
}

inline bool Box3I::isContained(const Box3I& in_rContained) const
{
   return (minExtents.x <= in_rContained.minExtents.x) &&
      (minExtents.y <= in_rContained.minExtents.y) &&
      (minExtents.z <= in_rContained.minExtents.z) &&
      (maxExtents.x >= in_rContained.maxExtents.x) &&
      (maxExtents.y >= in_rContained.maxExtents.y) &&
      (maxExtents.z >= in_rContained.maxExtents.z);
}

inline S32 Box3I::len_x() const
{
   return maxExtents.x - minExtents.x;
}

inline S32 Box3I::len_y() const
{
   return maxExtents.y - minExtents.y;
}

inline S32 Box3I::len_z() const
{
   return maxExtents.z - minExtents.z;
}

inline void Box3I::intersect(const Box3I& in_rIntersect)
{
   minExtents.setMin(in_rIntersect.minExtents);
   maxExtents.setMax(in_rIntersect.maxExtents);
}

inline void Box3I::getCenter(Point3I* center) const
{
   center->x = (minExtents.x + maxExtents.x) >> 1;
   center->y = (minExtents.y + maxExtents.y) >> 1;
   center->z = (minExtents.z + maxExtents.z) >> 1;
}

inline void Box3I::extend(const Point3I & p)
{
#define EXTEND_AXIS(AXIS)    \
if (p.AXIS < minExtents.AXIS)       \
   minExtents.AXIS = p.AXIS;        \
else if (p.AXIS > maxExtents.AXIS)  \
   maxExtents.AXIS = p.AXIS;

   EXTEND_AXIS(x)
   EXTEND_AXIS(y)
   EXTEND_AXIS(z)

#undef EXTEND_AXIS
}


#endif // _DBOX_H_
