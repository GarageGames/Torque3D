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

#ifndef _MRECT_H_
#define _MRECT_H_

#ifndef _MPOINT2_H_
#include "math/mPoint2.h"
#endif


class RectI
{
  public:
   Point2I  point;
   Point2I  extent;

  public:
   RectI() { }
   RectI(const Point2I& in_rMin,
         const Point2I& in_rExtent);
   RectI(const S32 in_left,  const S32 in_top,
         const S32 in_width, const S32 in_height);

   void set(const Point2I& in_rMin, const Point2I& in_rExtent);
   void set(const S32 in_left,  const S32 in_top,
         const S32 in_width, const S32 in_height);

   bool intersect(const RectI& clipRect);
   bool pointInRect(const Point2I& pt) const;
   bool overlaps(RectI R) const;
   bool contains(const RectI& R) const;
   void inset(S32 x, S32 y);

   void unionRects(const RectI&);

   S32   len_x() const;
   S32   len_y() const;

   /// Returns the area of the rectangle.
   S32 area() const { return extent.x * extent.y; }

   bool operator==(const RectI&) const;
   bool operator!=(const RectI&) const;

   bool isValidRect() const { return (extent.x > 0 && extent.y > 0); }

public:

   /// A rect of zero extent.
   static const RectI Zero;

   /// A rect of 1,1 extent.
   static const RectI One;

};

class RectF
{
  public:
   Point2F  point;
   Point2F  extent;

  public:
   RectF() { }
   RectF(const Point2F& in_rMin,
         const Point2F& in_rExtent);
   RectF(const F32 in_left,  const F32 in_top,
         const F32 in_width, const F32 in_height);

   void set(const Point2F& in_rMin, const Point2F& in_rExtent);
   void set(const F32 in_left,  const F32 in_top,
      const F32 in_width, const F32 in_height);

   void inset(F32 x, F32 y);

   /// Return distance of the reference point to the rectangle.
   F32 getDistanceToPoint( const Point2F &refPt ) const;

   /// Return the squared distance of the reference point to the rectangle.
   F32 getSqDistanceToPoint( const Point2F &refPt ) const;

   bool intersect(const RectF& clipRect);
   bool pointInRect(const Point2F& pt) const;
   bool overlaps(const RectF&) const;
   bool contains(const Point2F &p) const
   {
      Point2F minkowskiP = p - point;

      // If we're beyond origin...
      if(minkowskiP.x < 0 || minkowskiP.y < 0)
         return false;

      // Or past extent...
      if(minkowskiP.x > extent.x || minkowskiP.y > extent.y)
         return false;

      // Otherwise we're ok.
      return true;
   }

   bool contains(const RectF& R) const;

   void unionRects( const RectF &rect );

   F32 len_x() const;
   F32 len_y() const;

   bool isValidRect() const { return (extent.x > 0.0f && extent.y > 0.0f); }
   inline bool intersectTriangle(const Point2F &a, const Point2F &b, const Point2F &c);

};

class RectD
{
  public:
   Point2D  point;
   Point2D  extent;

  public:
   RectD() { }
   RectD(const Point2D& in_rMin,
         const Point2D& in_rExtent);
   RectD(const F64 in_left,  const F64 in_top,
         const F64 in_width, const F64 in_height);
   void inset(F64 x, F64 y);

   bool intersect(const RectD& clipRect);
   F64 len_x() const;
   F64 len_y() const;

   bool isValidRect() const { return (extent.x > 0 && extent.y > 0); }
};

//------------------------------------------------------------------------------
//-------------------------------------- INLINES (RectI)
//
inline
RectI::RectI(const Point2I& in_rMin,
             const Point2I& in_rExtent)
 : point(in_rMin),
   extent(in_rExtent)
{
   //
}

inline
RectI::RectI(const S32 in_left,  const S32 in_top,
             const S32 in_width, const S32 in_height)
 : point(in_left,  in_top),
   extent(in_width, in_height)
{
   //
}

inline void RectI::set(const Point2I& in_rMin, const Point2I& in_rExtent)
{
   point = in_rMin;
   extent = in_rExtent;
}

inline void RectI::set(const S32 in_left,  const S32 in_top,
                      const S32 in_width, const S32 in_height)
{
   point.set(in_left,  in_top);
   extent.set(in_width, in_height);
}

inline bool RectI::intersect(const RectI& clipRect)
{
   Point2I bottomL;
   bottomL.x = getMin(point.x + extent.x - 1, clipRect.point.x + clipRect.extent.x - 1);
   bottomL.y = getMin(point.y + extent.y - 1, clipRect.point.y + clipRect.extent.y - 1);

   point.x = getMax(point.x, clipRect.point.x);
   point.y = getMax(point.y, clipRect.point.y);

   extent.x = bottomL.x - point.x + 1;
   extent.y = bottomL.y - point.y + 1;

   return isValidRect();
}

inline bool RectI::pointInRect(const Point2I &pt) const
{
   return (pt.x >= point.x && pt.x < point.x + extent.x && pt.y >= point.y && pt.y < point.y + extent.y);
}

inline bool RectI::contains(const RectI& R) const
{
   if (point.x <= R.point.x && point.y <= R.point.y)
    if (R.point.x + R.extent.x <= point.x + extent.x)
     if (R.point.y + R.extent.y <= point.y + extent.y)
      return true;
   return false;
}

inline bool RectI::overlaps(RectI R) const
{
   return R.intersect (* this);
}

inline void RectI::inset(S32 x, S32 y)
{
   point.x += x;
   point.y += y;
   extent.x -= 2 * x;
   extent.y -= 2 * y;
}

inline void RectF::inset(F32 x, F32 y)
{
   point.x += x;
   point.y += y;
   extent.x -= 2.0f * x;
   extent.y -= 2.0f * y;
}

inline void RectD::inset(F64 x, F64 y)
{
   point.x += x;
   point.y += y;
   extent.x -= 2.0 * x;
   extent.y -= 2.0 * y;
}


inline void RectI::unionRects(const RectI& u)
{
   S32 minx = point.x < u.point.x ? point.x : u.point.x;
   S32 miny = point.y < u.point.y ? point.y : u.point.y;
   S32 maxx = (point.x + extent.x) > (u.point.x + u.extent.x) ? (point.x + extent.x) : (u.point.x + u.extent.x);
   S32 maxy = (point.y + extent.y) > (u.point.y + u.extent.y) ? (point.y + extent.y) : (u.point.y + u.extent.y);

   point.x  = minx;
   point.y  = miny;
   extent.x = maxx - minx;
   extent.y = maxy - miny;
}

inline S32
RectI::len_x() const
{
   return extent.x;
}

inline S32
RectI::len_y() const
{
   return extent.y;
}

inline bool
RectI::operator==(const RectI& in_rCompare) const
{
   return (point == in_rCompare.point) && (extent == in_rCompare.extent);
}

inline bool
RectI::operator!=(const RectI& in_rCompare) const
{
   return (operator==(in_rCompare) == false);
}

//------------------------------------------------------------------------------
//-------------------------------------- INLINES (RectF)
//
inline
RectF::RectF(const Point2F& in_rMin,
             const Point2F& in_rExtent)
 : point(in_rMin),
   extent(in_rExtent)
{
   //
}

inline
RectF::RectF(const F32 in_left,  const F32 in_top,
             const F32 in_width, const F32 in_height)
 : point(in_left,  in_top),
   extent(in_width, in_height)
{
   //
}

inline F32
RectF::len_x() const
{
   return extent.x;
}

inline F32
RectF::len_y() const
{
   return extent.y;
}

inline void RectF::set(const Point2F& in_rMin, const Point2F& in_rExtent)
{
   point = in_rMin;
   extent = in_rExtent;
}

inline void RectF::set(const F32 in_left,  const F32 in_top,
                       const F32 in_width, const F32 in_height)
{
   point.set(in_left,  in_top);
   extent.set(in_width, in_height);
}

inline F32 RectF::getDistanceToPoint( const Point2F &refPt ) const
{
   return mSqrt( getSqDistanceToPoint( refPt ) );
}

inline F32 RectF::getSqDistanceToPoint( const Point2F &refPt ) const
{
   const Point2F maxPoint( point + extent );

   F32 sqDist = 0.0f;

   for ( U32 i=0; i < 2; i++ )
   {
      const F32 v = refPt[i];
      if ( v < point[i] )
         sqDist += mSquared( point[i] - v );
      else if ( v > maxPoint[i] )
         sqDist += mSquared( v - maxPoint[i] );
   }

   return sqDist;
}

inline bool RectF::intersect(const RectF& clipRect)
{
   Point2F bottomL;
   bottomL.x = getMin(point.x + extent.x, clipRect.point.x + clipRect.extent.x);
   bottomL.y = getMin(point.y + extent.y, clipRect.point.y + clipRect.extent.y);

   point.x = getMax(point.x, clipRect.point.x);
   point.y = getMax(point.y, clipRect.point.y);

   extent.x = bottomL.x - point.x;
   extent.y = bottomL.y - point.y;

   return isValidRect();
}

inline bool RectF::pointInRect(const Point2F &pt) const
{
   return (pt.x >= point.x && pt.x < point.x + extent.x && pt.y >= point.y && pt.y < point.y + extent.y);
}

inline bool RectF::overlaps(const RectF& clipRect) const
{
   RectF test = *this;
   return test.intersect(clipRect);
}

inline bool lineToLineIntersect(const Point2F & a1, const Point2F & a2, const Point2F & b1, const Point2F & b2)
{
   const F32 ua_t = (b2.x - b1.x) * (a1.y - b1.y) - (b2.y - b1.y) * (a1.x - b1.x);
   const F32 ub_t = (a2.x - a1.x) * (a1.y - b1.y) - (a2.y - a1.y) * (a1.x - b1.x);
   const F32 u_b = (b2.y - b1.y) * (a2.x - a1.x) - (b2.x - b1.x) * (a2.y - a1.y);

   if(u_b != 0)
   {
      const F32 ua = ua_t / u_b;
      const F32 ub = ub_t / u_b;

      return ( 0.0f <= ua && ua <= 1.0f && 0.0f <= ub && ub <= 1.0f );
   }
   else
   {
      return ( ua_t == 0 || ub_t == 0 );
   }
}

inline bool RectF::intersectTriangle(const Point2F &a, const Point2F &b, const Point2F &c)
{
   const Point2F topLeft     = point;
   const Point2F topRight    = Point2F( point.x + extent.x, point.y );
   const Point2F bottomLeft  = Point2F( point.x, point.y + extent.y );
   const Point2F bottomRight = point + extent;

   // 3 point plus 12 edge tests.

   // Check each triangle point to see if it's in us.
   if(contains(a) || contains(b) || contains(c))
      return true;

   // Check a-b against the rect.
   if(lineToLineIntersect(topLeft, topRight, a, b))
      return true;

   if(lineToLineIntersect(topRight, bottomRight, a, b))
      return true;

   if(lineToLineIntersect(bottomRight, bottomLeft, a, b))
      return true;

   if(lineToLineIntersect(bottomLeft, topLeft, a, b))
      return true;

   // Check b-c
   if(lineToLineIntersect(topLeft, topRight, b, c))
      return true;

   if(lineToLineIntersect(topRight, bottomRight, b, c))
      return true;

   if(lineToLineIntersect(bottomRight, bottomLeft, b, c))
      return true;

   if(lineToLineIntersect(bottomLeft, topLeft, b, c))
      return true;

   // Check c-a
   if(lineToLineIntersect(topLeft, topRight, c, a))
      return true;

   if(lineToLineIntersect(topRight, bottomRight, c, a))
      return true;

   if(lineToLineIntersect(bottomRight, bottomLeft, c, a))
      return true;

   if(lineToLineIntersect(bottomLeft, topLeft, c, a))
      return true;

   return false;
}

inline bool RectF::contains(const RectF& R) const
{
   if (point.x <= R.point.x && point.y <= R.point.y)
      if (R.point.x + R.extent.x <= point.x + extent.x)
         if (R.point.y + R.extent.y <= point.y + extent.y)
            return true;
   return false;
}

inline void RectF::unionRects( const RectF &r )
{
   F32 minx = point.x < r.point.x ? point.x : r.point.x;
   F32 miny = point.y < r.point.y ? point.y : r.point.y;
   F32 maxx = (point.x + extent.x) > (r.point.x + r.extent.x) ? (point.x + extent.x) : (r.point.x + r.extent.x);
   F32 maxy = (point.y + extent.y) > (r.point.y + r.extent.y) ? (point.y + extent.y) : (r.point.y + r.extent.y);

   point.x  = minx;
   point.y  = miny;
   extent.x = maxx - minx;
   extent.y = maxy - miny;
}

//------------------------------------------------------------------------------
//-------------------------------------- INLINES (RectD)
//
inline
RectD::RectD(const Point2D& in_rMin,
             const Point2D& in_rExtent)
 : point(in_rMin),
   extent(in_rExtent)
{
   //
}

inline
RectD::RectD(const F64 in_left,  const F64 in_top,
             const F64 in_width, const F64 in_height)
 : point(in_left,  in_top),
   extent(in_width, in_height)
{
   //
}

inline F64
RectD::len_x() const
{
   return extent.x;
}

inline F64
RectD::len_y() const
{
   return extent.y;
}

inline bool RectD::intersect(const RectD& clipRect)
{
   Point2D bottomL;
   bottomL.x = getMin(point.x + extent.x, clipRect.point.x + clipRect.extent.x);
   bottomL.y = getMin(point.y + extent.y, clipRect.point.y + clipRect.extent.y);

   point.x = getMax(point.x, clipRect.point.x);
   point.y = getMax(point.y, clipRect.point.y);

   extent.x = bottomL.x - point.x;
   extent.y = bottomL.y - point.y;

   return isValidRect();
}

#endif //_RECT_H_
