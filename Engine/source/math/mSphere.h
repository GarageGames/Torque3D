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

#ifndef _MSPHERE_H_
#define _MSPHERE_H_

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif


class SphereF
{
public:
   Point3F center;
   F32     radius;

public:
   SphereF() { }
   SphereF( const Point3F& in_rPosition, const F32 in_rRadius )
    : center(in_rPosition),
      radius(in_rRadius)
   {
      if ( radius < 0.0f )
         radius = 0.0f;
   }

   bool isContained( const Point3F& in_rContain ) const;
   bool isContained( const SphereF& in_rContain ) const;
   bool isIntersecting( const SphereF& in_rIntersect ) const;
   bool intersectsRay( const Point3F &start, const Point3F &end ) const;

   F32 distanceTo( const Point3F &pt ) const;
   F32 squareDistanceTo( const Point3F &pt ) const;
};

//-------------------------------------- INLINES
//
inline bool SphereF::isContained( const Point3F& in_rContain ) const
{
   F32 distSq = (center - in_rContain).lenSquared();

   return (distSq <= (radius * radius));
}

inline bool SphereF::isContained( const SphereF& in_rContain ) const
{
   if (radius < in_rContain.radius)
      return false;

   // Since our radius is guaranteed to be >= other's, we
   //  can dodge the sqrt() here.
   //
   F32 dist = (in_rContain.center - center).lenSquared();

   return (dist <= ((radius - in_rContain.radius) *
                    (radius - in_rContain.radius)));
}

inline bool SphereF::isIntersecting( const SphereF& in_rIntersect ) const
{
   F32 distSq = (in_rIntersect.center - center).lenSquared();

   return (distSq <= ((in_rIntersect.radius + radius) *
                      (in_rIntersect.radius + radius)));
}

inline F32 SphereF::distanceTo( const Point3F &toPt ) const
{
   return (center - toPt).len() - radius;
}

#endif //_SPHERE_H_
