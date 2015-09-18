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

#ifndef _PX3CASTS_H_
#define _PX3CASTS_H_

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif
#ifndef _MMATRIX_H_
#include "math/mMatrix.h"
#endif
#ifndef _MBOX_H_
#include "math/mBox.h"
#endif
#ifndef _MQUAT_H_
#include "math/mQuat.h"
#endif
#ifndef _MTRANSFORM_H_
#include "math/mTransform.h"
#endif


template <class T, class F> inline T px3Cast( const F &from );

//-------------------------------------------------------------------------

template<>
inline Point3F px3Cast( const physx::PxVec3 &vec )
{
   return Point3F( vec.x, vec.y, vec.z );
}

template<>
inline physx::PxVec3 px3Cast( const Point3F &point )
{
   return physx::PxVec3( point.x, point.y, point.z );
}
//-------------------------------------------------------------------------
template<>
inline QuatF px3Cast( const physx::PxQuat &quat )
{
   /// The Torque quat has the opposite winding order.
   return QuatF( -quat.x, -quat.y, -quat.z, quat.w );
}

template<>
inline physx::PxQuat px3Cast( const QuatF &quat )
{
   /// The Torque quat has the opposite winding order.
   physx::PxQuat result( -quat.x, -quat.y, -quat.z, quat.w );
   return result;
}
//-------------------------------------------------------------------------

template<>
inline physx::PxExtendedVec3 px3Cast( const Point3F &point )
{
   return physx::PxExtendedVec3( point.x, point.y, point.z );
}

template<>
inline Point3F px3Cast( const physx::PxExtendedVec3 &xvec )
{
   return Point3F( xvec.x, xvec.y, xvec.z );
}

//-------------------------------------------------------------------------

template<>
inline physx::PxBounds3 px3Cast( const Box3F &box )
{
   physx::PxBounds3 bounds(px3Cast<physx::PxVec3>(box.minExtents),
							px3Cast<physx::PxVec3>(box.maxExtents));
   return bounds;
}

template<>
inline Box3F px3Cast( const physx::PxBounds3 &bounds )
{
   return Box3F(  bounds.minimum.x, 
                  bounds.minimum.y,
                  bounds.minimum.z,
                  bounds.maximum.x,
                  bounds.maximum.y,
                  bounds.maximum.z );
}

//-------------------------------------------------------------------------

template<>
inline physx::PxTransform px3Cast( const MatrixF &xfm )
{
   physx::PxTransform out;
   QuatF q;
   q.set(xfm);
   out.q = px3Cast<physx::PxQuat>(q);
   out.p = px3Cast<physx::PxVec3>(xfm.getPosition());
   return out;
}

template<>
inline TransformF px3Cast(const physx::PxTransform &xfm)
{
	TransformF out(px3Cast<Point3F>(xfm.p),AngAxisF(px3Cast<QuatF>(xfm.q)));
	return out;
}

template<>
inline MatrixF px3Cast( const physx::PxTransform &xfm )
{
   MatrixF out;
   TransformF t = px3Cast<TransformF>(xfm);
   out = t.getMatrix();
   return out;
}

#endif //_PX3CASTS_H_
