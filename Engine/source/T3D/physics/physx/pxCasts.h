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

#ifndef _PHYSX_CASTS_H_
#define _PHYSX_CASTS_H_

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif
#ifndef _MBOX_H_
#include "math/mBox.h"
#endif
#ifndef _MQUAT_H_
#include "math/mQuat.h"
#endif


template <class T, class F> inline T pxCast( const F &from );

//-------------------------------------------------------------------------

template<>
inline Point3F pxCast( const NxVec3 &vec )
{
   return Point3F( vec.x, vec.y, vec.z );
}

template<>
inline NxVec3 pxCast( const Point3F &point )
{
   return NxVec3( point.x, point.y, point.z );
}

//-------------------------------------------------------------------------

template<>
inline QuatF pxCast( const NxQuat &quat )
{
   /// The Torque quat has the opposite winding order.
   return QuatF( -quat.x, -quat.y, -quat.z, quat.w );
}

template<>
inline NxQuat pxCast( const QuatF &quat )
{
   /// The Torque quat has the opposite winding order.
   NxQuat result;
   result.setWXYZ( quat.w, -quat.x, -quat.y, -quat.z );
   return result;
}

//-------------------------------------------------------------------------

template<>
inline NxBounds3 pxCast( const Box3F &box )
{
   NxBounds3 bounds;
   bounds.set( box.minExtents.x, 
               box.minExtents.y,
               box.minExtents.z,
               box.maxExtents.x,
               box.maxExtents.y,
               box.maxExtents.z );
   return bounds;
}

template<>
inline Box3F pxCast( const NxBounds3 &bounds )
{
   return Box3F(  bounds.min.x, 
                  bounds.min.y,
                  bounds.min.z,
                  bounds.max.x,
                  bounds.max.y,
                  bounds.max.z );
}

//-------------------------------------------------------------------------

template<>
inline NxVec3 pxCast( const NxExtendedVec3 &xvec )
{
   return NxVec3( xvec.x, xvec.y, xvec.z );
}

template<>
inline NxExtendedVec3 pxCast( const NxVec3 &vec )
{
   return NxExtendedVec3( vec.x, vec.y, vec.z );
}

//-------------------------------------------------------------------------

template<>
inline NxExtendedVec3 pxCast( const Point3F &point )
{
   return NxExtendedVec3( point.x, point.y, point.z );
}

template<>
inline Point3F pxCast( const NxExtendedVec3 &xvec )
{
   return Point3F( xvec.x, xvec.y, xvec.z );
}

//-------------------------------------------------------------------------

template<>
inline NxBox pxCast( const NxExtendedBounds3 &exBounds )
{
   NxExtendedVec3 center;
   exBounds.getCenter( center );
   NxVec3 extents;
   exBounds.getExtents( extents );

   NxBox box;
   box.center.set( center.x, center.y, center.z );
   box.extents = extents;
   box.rot.id();

   return box;
}

template<>
inline NxExtendedBounds3 pxCast( const NxBox &box )
{
   AssertFatal( false, "Casting a NxBox to NxExtendedBounds3 is impossible without losing rotation data!" );
   return NxExtendedBounds3();
}

#endif // _PHYSX_CASTS_H_