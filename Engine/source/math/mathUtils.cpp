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
#include "math/mathUtils.h"

#include "math/mMath.h"
#include "math/mRandom.h"
#include "math/util/frustum.h"
#include "platform/profiler.h"
#include "core/tAlgorithm.h"

namespace MathUtils
{

MRandomLCG sgRandom(0xdeadbeef); ///< Our random number generator.

//-----------------------------------------------------------------------------

bool capsuleCapsuleOverlap(const Point3F & a1, const Point3F & b1, F32 rad1, const Point3F & a2, const Point3F & b2, F32 rad2)
{
   F32 s,t;
   Point3F c1,c2;
   F32 dist = segmentSegmentNearest(a1,b1,a2,b2,s,t,c1,c2);
   return dist <= (rad1+rad2)*(rad1+rad2);
}

//-----------------------------------------------------------------------------

F32 segmentSegmentNearest(const Point3F & p1, const Point3F & q1, const Point3F & p2, const Point3F & q2, F32 & s, F32 & t, Point3F & c1, Point3F & c2)
{
   Point3F d1 = q1-p1;
   Point3F d2 = q2-p2;
   Point3F r = p1-p2;
   F32 a = mDot(d1,d1);
   F32 e = mDot(d2,d2);
   F32 f = mDot(d2,r);
   
   const F32 EPSILON = 0.001f;
   
   if (a <= EPSILON && e <= EPSILON)
   {
      s = t = 0.0f;
      c1 = p1;
      c2 = p2;
      return mDot(c1-c2,c1-c2);
   }
   
   if (a <= EPSILON)
   {
      s = 0.0f;
      t = mClampF(f/e,0.0f,1.0f);
   }
   else
   {
      F32 c = mDot(d1,r);
      if (e <= EPSILON)
      {
         t = 0.0f;
         s = mClampF(-c/a,0.0f,1.0f);
      }
      else
      {
         F32 b = mDot(d1,d2);
         F32 denom = a*e-b*b;
         if (denom != 0.0f)
            s = mClampF((b*f-c*e)/denom,0.0f,1.0f);
         else
            s = 0.0f;
         F32 tnom = b*s+f;
         if (tnom < 0.0f)
         {
            t = 0.0f;
            s = mClampF(-c/a,0.0f,1.0f);
         }
         else if (tnom>e)
         {
            t = 1.0f;
            s = mClampF((b-c)/a,0.0f,1.0f);
         }
         else
            t = tnom/e;
      }
   }
   
   c1 = p1 + d1*s;
   c2 = p2 + d2*t;
   return mDot(c1-c2,c1-c2);
}

//-----------------------------------------------------------------------------

bool capsuleSphereNearestOverlap(const Point3F & A0, const Point3F A1, F32 radA, const Point3F & B, F32 radB, F32 & t)
{
   Point3F V = A1-A0;
   Point3F A0B = A0-B;
   F32 d1 = mDot(A0B,V);
   F32 d2 = mDot(A0B,A0B);
   F32 d3 = mDot(V,V);
   F32 R2 = (radA+radB)*(radA+radB);
   if (d2<R2)
   {
      // starting in collision state
      t=0;
      return true;
   }
   if (d3<0.01f)
      // no movement, and don't start in collision state, so no collision
      return false;

   F32 b24ac = mSqrt(d1*d1-d2*d3+d3*R2);
   F32 t1 = (-d1-b24ac)/d3;
   if (t1>0 && t1<1.0f)
   {
      t=t1;
      return true;
   }
   F32 t2 = (-d1+b24ac)/d3;
   if (t2>0 && t2<1.0f)
   {
      t=t2;
      return true;
   }
   if (t1<0 && t2>0)
   {
      t=0;
      return true;
   }
   return false;   
}

//-----------------------------------------------------------------------------

void vectorRotateZAxis( Point3F &vector, F32 radians )
{
   F32 sin, cos;
   mSinCos(radians, sin, cos);
   F32 x = cos * vector.x - sin * vector.y;
   F32 y = sin * vector.x + cos * vector.y;
   vector.x = x;
   vector.y = y;     
}

void vectorRotateZAxis( F32 radians, Point3F *vectors, U32 count )
{
   F32 sin, cos;
   mSinCos(radians, sin, cos);

   F32 x, y;
   const Point3F *end = vectors + count;
   for ( ; vectors != end; vectors++ )
   {
      x = cos * vectors->x - sin * vectors->y;
      y = sin * vectors->x + cos * vectors->y;
      vectors->x = x;
      vectors->y = y;      
   }
}

//-----------------------------------------------------------------------------

void getZBiasProjectionMatrix( F32 bias, const Frustum &frustum, MatrixF *outMat, bool rotate )
{
   Frustum temp(frustum);
   temp.setNearDist(frustum.getNearDist() + bias);
   temp.getProjectionMatrix(outMat, rotate);
}

//-----------------------------------------------------------------------------

MatrixF createOrientFromDir( const Point3F &direction )
{
	Point3F j = direction;
	Point3F k(0.0f, 0.0f, 1.0f);
	Point3F i;
	
	mCross( j, k, &i );

	if( i.magnitudeSafe() == 0.0f )
	{
		i.set( 0.0f, -1.0f, 0.0f );
	}

	i.normalizeSafe();
	mCross( i, j, &k );

   MatrixF mat( true );
   mat.setColumn( 0, i );
   mat.setColumn( 1, j );
   mat.setColumn( 2, k );

	return mat;
}

//-----------------------------------------------------------------------------

void getMatrixFromUpVector( const VectorF &up, MatrixF *outMat )
{
   AssertFatal( up.isUnitLength(), "MathUtils::getMatrixFromUpVector() - Up vector was not normalized!" );
   AssertFatal( outMat, "MathUtils::getMatrixFromUpVector() - Got null output matrix!" );
   AssertFatal( outMat->isAffine(), "MathUtils::getMatrixFromUpVector() - Got uninitialized matrix!" );

   VectorF forward = mPerp( up );
   VectorF right = mCross( forward, up );
   right.normalize();
   forward = mCross( up, right );
   forward.normalize();

   outMat->setColumn( 0, right );
   outMat->setColumn( 1, forward );
   outMat->setColumn( 2, up );
}

//-----------------------------------------------------------------------------

void getMatrixFromForwardVector( const VectorF &forward, MatrixF *outMat  )
{
   AssertFatal( forward.isUnitLength(), "MathUtils::getMatrixFromForwardVector() - Forward vector was not normalized!" );
   AssertFatal( outMat, "MathUtils::getMatrixFromForwardVector() - Got null output matrix!" );
   AssertFatal( outMat->isAffine(), "MathUtils::getMatrixFromForwardVector() - Got uninitialized matrix!" );

   VectorF up = mPerp( forward );
   VectorF right = mCross( forward, up );
   right.normalize();
   up = mCross( right, forward );
   up.normalize();

   outMat->setColumn( 0, right );
   outMat->setColumn( 1, forward );
   outMat->setColumn( 2, up );
}

//-----------------------------------------------------------------------------

Point3F randomDir( const Point3F &axis, F32 thetaAngleMin, F32 thetaAngleMax,
                                        F32 phiAngleMin, F32 phiAngleMax )
{
   MatrixF orient = createOrientFromDir( axis );
   Point3F axisx;
   orient.getColumn( 0, &axisx );

   F32 theta = (thetaAngleMax - thetaAngleMin) * sgRandom.randF() + thetaAngleMin;
   F32 phi = (phiAngleMax - phiAngleMin) * sgRandom.randF() + phiAngleMin;

   // Both phi and theta are in degs.  Create axis angles out of them, and create the
   //  appropriate rotation matrix...
   AngAxisF thetaRot(axisx, theta * (M_PI_F / 180.0f));
   AngAxisF phiRot(axis,    phi   * (M_PI_F / 180.0f));

   Point3F ejectionAxis = axis;

   MatrixF temp(true);
   thetaRot.setMatrix(&temp);
   temp.mulP(ejectionAxis);
   phiRot.setMatrix(&temp);
   temp.mulP(ejectionAxis);

   return ejectionAxis;
}

//-----------------------------------------------------------------------------

Point3F randomPointInSphere( F32 radius )
{
   AssertFatal( radius > 0.0f, "MathUtils::randomPointInRadius - radius must be positive" );

   #define MAX_TRIES 20

   Point3F out;
   F32 radiusSq = radius * radius;

   for ( S32 i = 0; i < MAX_TRIES; i++ )
   {
      out.x = sgRandom.randF(-radius,radius);
      out.y = sgRandom.randF(-radius,radius);
      out.z = sgRandom.randF(-radius,radius);

      if ( out.lenSquared() < radiusSq )
         return out;
   }

   AssertFatal( false, "MathUtils::randomPointInRadius - something is wrong, should not fail this many times." );
   return Point3F::Zero;
}

//-----------------------------------------------------------------------------

Point2F randomPointInCircle( F32 radius )
{
   AssertFatal( radius > 0.0f, "MathUtils::randomPointInRadius - radius must be positive" );

   #define MAX_TRIES 20

   Point2F out;
   F32 radiusSq = radius * radius;

   for ( S32 i = 0; i < MAX_TRIES; i++ )
   {
      out.x = sgRandom.randF(-radius,radius);
      out.y = sgRandom.randF(-radius,radius);

      if ( out.lenSquared() < radiusSq )
         return out;
   }

   AssertFatal( false, "MathUtils::randomPointInRadius - something is wrong, should not fail this many times." );
   return Point2F::Zero;
}

//-----------------------------------------------------------------------------

void getAnglesFromVector( const VectorF &vec, F32 &yawAng, F32 &pitchAng )
{
   yawAng = mAtan2( vec.x, vec.y );
   if( yawAng < 0.0f )
      yawAng += M_2PI_F;

   if( mFabs(vec.x) > mFabs(vec.y) )
      pitchAng = mAtan2( mFabs(vec.z), mFabs(vec.x) );
   else
      pitchAng = mAtan2( mFabs(vec.z), mFabs(vec.y) );
   if( vec.z < 0.0f )
      pitchAng = -pitchAng;
}

//-----------------------------------------------------------------------------

void getVectorFromAngles( VectorF &vec, F32 yawAng, F32 pitchAng )
{
   VectorF  pnt( 0.0f, 1.0f, 0.0f );

   EulerF   rot( -pitchAng, 0.0f, 0.0f );
   MatrixF  mat( rot );

   rot.set( 0.0f, 0.0f, yawAng );
   MatrixF   mat2( rot );

   mat.mulV( pnt );
   mat2.mulV( pnt );

   vec = pnt;
}

//-----------------------------------------------------------------------------

void transformBoundingBox(const Box3F &sbox, const MatrixF &mat, const Point3F scale, Box3F &dbox)
{
	Point3F center;

	// set transformed center...
	sbox.getCenter(&center);
   center.convolve(scale);
	mat.mulP(center);
	dbox.minExtents = center;
	dbox.maxExtents = center;

	Point3F val;
	for(U32 ix=0; ix<2; ix++)
	{
		if(ix & 0x1)
			val.x = sbox.minExtents.x;
		else
			val.x = sbox.maxExtents.x;

		for(U32 iy=0; iy<2; iy++)
		{
			if(iy & 0x1)
				val.y = sbox.minExtents.y;
			else
				val.y = sbox.maxExtents.y;

			for(U32 iz=0; iz<2; iz++)
			{
				if(iz & 0x1)
					val.z = sbox.minExtents.z;
				else
					val.z = sbox.maxExtents.z;

				Point3F v1, v2;
            v1 = val;
            v1.convolve(scale);
				mat.mulP(v1, &v2);
				dbox.minExtents.setMin(v2);
				dbox.maxExtents.setMax(v2);
			}
		}
	}
}

//-----------------------------------------------------------------------------

bool mProjectWorldToScreen(   const Point3F &in, 
                              Point3F *out, 
                              const RectI &view, 
                              const MatrixF &world, 
                              const MatrixF &projection )
{
   MatrixF worldProjection = projection;
   worldProjection.mul(world);

   return mProjectWorldToScreen( in, out, view, worldProjection );
}

//-----------------------------------------------------------------------------

bool mProjectWorldToScreen(   const Point3F &in, 
                              Point3F *out, 
                              const RectI &view, 
                              const MatrixF &worldProjection )
{
   Point4F temp(in.x,in.y,in.z,1.0f);
   worldProjection.mul(temp);

   // Perform the perspective division.  For orthographic
   // projections, temp.w will be 1.

   temp.x /= temp.w;
   temp.y /= temp.w;
   temp.z /= temp.w;

   // Take the normalized device coordinates (NDC) and transform them
   // into device coordinates.

   out->x = (temp.x + 1.0f) / 2.0f * view.extent.x + view.point.x;
   out->y = (1.0f - temp.y) / 2.0f * view.extent.y + view.point.y;
   out->z = temp.z;

   if ( out->z < 0.0f || out->z > 1.0f || 
        out->x < (F32)view.point.x || out->x > (F32)view.point.x + (F32)view.extent.x ||
        out->y < (F32)view.point.y || out->y > (F32)view.point.y + (F32)view.extent.y )
      return false;

   return true;
}

//-----------------------------------------------------------------------------

void mProjectScreenToWorld(   const Point3F &in, 
                              Point3F *out, 
                              const RectI &view, 
                              const MatrixF &world, 
                              const MatrixF &projection, 
                              F32 zfar, 
                              F32 znear )
{
   MatrixF invWorldProjection = projection;
   invWorldProjection.mul(world);
   invWorldProjection.inverse();

   Point3F vec;
   vec.x = (in.x - view.point.x) * 2.0f / view.extent.x - 1.0f;
   vec.y = -(in.y - view.point.y) * 2.0f / view.extent.y + 1.0f;
   vec.z = (znear + in.z * (zfar - znear))/zfar;

   invWorldProjection.mulV(vec);
   vec *= 1.0f + in.z * zfar;

   invWorldProjection.getColumn(3, out);
   (*out) += vec;
}

//-----------------------------------------------------------------------------

bool pointInPolygon( const Point2F *verts, U32 vertCount, const Point2F &testPt )
{
  U32 i, j, c = 0;
  for ( i = 0, j = vertCount-1; i < vertCount; j = i++ ) 
  {
    if ( ( ( verts[i].y > testPt.y ) != ( verts[j].y > testPt.y ) ) &&
	         ( testPt.x <   ( verts[j].x - verts[i].x ) * 
                           ( testPt.y - verts[i].y ) / 
                           ( verts[j].y - verts[i].y ) + verts[i].x ) )
       c = !c;
  }
  
  return c != 0;
}  

//-----------------------------------------------------------------------------

F32 mTriangleDistance( const Point3F &A, const Point3F &B, const Point3F &C, const Point3F &P, IntersectInfo* info )
{
    Point3F diff = A - P;
    Point3F edge0 = B - A;
    Point3F edge1 = C - A;
    F32 a00 = edge0.lenSquared();
    F32 a01 = mDot( edge0, edge1 );
    F32 a11 = edge1.lenSquared();
    F32 b0 = mDot( diff, edge0 );
    F32 b1 = mDot( diff, edge1 );
    F32 c = diff.lenSquared();
    F32 det = mFabs(a00*a11-a01*a01);
    F32 s = a01*b1-a11*b0;
    F32 t = a01*b0-a00*b1;
    F32 sqrDistance;

    if (s + t <= det)
    {
        if (s < 0.0f)
        {
            if (t < 0.0f)  // region 4
            {
                if (b0 < 0.0f)
                {
                    t = 0.0f;
                    if (-b0 >= a00)
                    {
                        s = 1.0f;
                        sqrDistance = a00 + (2.0f)*b0 + c;
                    }
                    else
                    {
                        s = -b0/a00;
                        sqrDistance = b0*s + c;
                    }
                }
                else
                {
                    s = 0.0f;
                    if (b1 >= 0.0f)
                    {
                        t = 0.0f;
                        sqrDistance = c;
                    }
                    else if (-b1 >= a11)
                    {
                        t = 1.0f;
                        sqrDistance = a11 + 2.0f*b1 + c;
                    }
                    else
                    {
                        t = -b1/a11;
                        sqrDistance = b1*t + c;
                    }
                }
            }
            else  // region 3
            {
                s = 0.0f;
                if (b1 >= 0.0f)
                {
                    t = 0.0f;
                    sqrDistance = c;
                }
                else if (-b1 >= a11)
                {
                    t = 1.0f;
                    sqrDistance = a11 + 2.0f*b1 + c;
                }
                else
                {
                    t = -b1/a11;
                    sqrDistance = b1*t + c;
                }
            }
        }
        else if (t < 0.0f)  // region 5
        {
            t = 0.0f;
            if (b0 >= 0.0f)
            {
                s = 0.0f;
                sqrDistance = c;
            }
            else if (-b0 >= a00)
            {
                s = 1.0f;
                sqrDistance = a00 + 2.0f*b0 + c;
            }
            else
            {
                s = -b0/a00;
                sqrDistance = b0*s + c;
            }
        }
        else  // region 0
        {
            // minimum at interior point
            F32 invDet = 1.0f / det;
            s *= invDet;
            t *= invDet;
            sqrDistance = s * (a00*s + a01*t + 2.0f*b0) +
                t * (a01*s + a11*t + 2.0f*b1) + c;
        }
    }
    else
    {
        F32 tmp0, tmp1, numer, denom;

        if (s < 0.0f)  // region 2
        {
            tmp0 = a01 + b0;
            tmp1 = a11 + b1;
            if (tmp1 > tmp0)
            {
                numer = tmp1 - tmp0;
                denom = a00 - 2.0f*a01 + a11;
                if (numer >= denom)
                {
                    s = 1.0f;
                    t = 0.0f;
                    sqrDistance = a00 + 2.0f*b0 + c;
                }
                else
                {
                    s = numer/denom;
                    t = 1.0f - s;
                    sqrDistance = s * (a00*s + a01*t + 2.0f*b0) +
                                  t * (a01*s + a11*t + 2.0f*b1) + c;
                }
            }
            else
            {
                s = 0.0f;
                if (tmp1 <= 0.0f)
                {
                    t = 1.0f;
                    sqrDistance = a11 + 2.0f*b1 + c;
                }
                else if (b1 >= 0.0f)
                {
                    t = 0.0f;
                    sqrDistance = c;
                }
                else
                {
                    t = -b1/a11;
                    sqrDistance = b1*t + c;
                }
            }
        }
        else if (t < 0.0f)  // region 6
        {
            tmp0 = a01 + b1;
            tmp1 = a00 + b0;
            if (tmp1 > tmp0)
            {
                numer = tmp1 - tmp0;
                denom = a00 - 2.0f*a01 + a11;
                if (numer >= denom)
                {
                    t = 1.0f;
                    s = 0.0f;
                    sqrDistance = a11 + 2.0f*b1 + c;
                }
                else
                {
                    t = numer/denom;
                    s = 1.0f - t;
                    sqrDistance = s * (a00*s + a01*t + 2.0f*b0) +
                                  t * (a01*s + a11*t + 2.0f*b1) + c;
                }
            }
            else
            {
                t = 0.0f;
                if (tmp1 <= 0.0f)
                {
                    s = 1.0f;
                    sqrDistance = a00 + 2.0f*b0 + c;
                }
                else if (b0 >= 0.0f)
                {
                    s = 0.0f;
                    sqrDistance = c;
                }
                else
                {
                    s = -b0/a00;
                    sqrDistance = b0*s + c;
                }
            }
        }
        else  // region 1
        {
            numer = a11 + b1 - a01 - b0;
            if (numer <= 0.0f)
            {
                s = 0.0f;
                t = 1.0f;
                sqrDistance = a11 + 2.0f*b1 + c;
            }
            else
            {
                denom = a00 - 2.0f*a01 + a11;
                if (numer >= denom)
                {
                    s = 1.0f;
                    t = 0.0f;
                    sqrDistance = a00 + 2.0f*b0 + c;
                }
                else
                {
                    s = numer/denom;
                    t = 1.0f - s;
                    sqrDistance = s * (a00*s + a01*t + 2.0f*b0) +
                                  t * (a01*s + a11*t + 2.0f*b1) + c;
                }
            }
        }
    }

    // account for numerical round-off error
    if (sqrDistance < 0.0f)
        sqrDistance = 0.0f;

    // This also calculates the barycentric coordinates and the closest point!
    //m_kClosestPoint0 = P;
    //m_kClosestPoint1 = A + s*edge0 + t*edge1;
    //m_afTriangleBary[1] = s;
    //m_afTriangleBary[2] = t;
    //m_afTriangleBary[0] = (Real)1.0 - fS - fT;
    if(info)
    {
       info->segment.p0 = P;
       info->segment.p1 = A + s*edge0 + t*edge1;
       info->bary.x = s;
       info->bary.y = t;
       info->bary.z = 1.0f - s - t;
    }

    return sqrDistance;
}

//-----------------------------------------------------------------------------

Point3F mTriangleNormal( const Point3F &a, const Point3F &b, const Point3F &c )
{
   // Vector from b to a.
   const F32 ax = a.x-b.x;
   const F32 ay = a.y-b.y;
   const F32 az = a.z-b.z;
   // Vector from b to c.
   const F32 cx = c.x-b.x;
   const F32 cy = c.y-b.y;
   const F32 cz = c.z-b.z;

   Point3F n;

   // This is an in-line cross product.
   n.x = ay*cz - az*cy;
   n.y = az*cx - ax*cz;
   n.z = ax*cy - ay*cx;
   m_point3F_normalize( (F32*)(&n) );

   return n;
}

//-----------------------------------------------------------------------------

Point3F mClosestPointOnSegment( const Point3F &a, const Point3F &b, const Point3F &p )
{
	Point3F c = p - a;               // Vector from a to Point
   Point3F v = (b - a);
	F32 d = v.len();           // Length of the line segment
   v.normalize();	                  // Unit Vector from a to b
	F32 t = mDot( v, c );            // Intersection point Distance from a

	// Check to see if the point is on the line
	// if not then return the endpoint
	if(t < 0) return a;
	if(t > d) return b;

	// get the distance to move from point a
	v *= t;

	// move from point a to the nearest point on the segment
	return a + v;
}

//-----------------------------------------------------------------------------

void mShortestSegmentBetweenLines( const Line &line0, const Line &line1, LineSegment *outSegment )
{   
   // compute intermediate parameters
   Point3F w0 = line0.origin - line1.origin;
   F32 a = mDot( line0.direction, line0.direction );
   F32 b = mDot( line0.direction, line1.direction );
   F32 c = mDot( line1.direction, line1.direction );
   F32 d = mDot( line0.direction, w0 );
   F32 e = mDot( line1.direction, w0 );

   F32 denom = a*c - b*b;

   if ( denom > -0.001f && denom < 0.001f )
   {
      outSegment->p0 = line0.origin;
      outSegment->p1 = line1.origin + (e/c)*line1.direction;
   }
   else
   {
      outSegment->p0 = line0.origin + ((b*e - c*d)/denom)*line0.direction;
      outSegment->p1 = line1.origin + ((a*e - b*d)/denom)*line1.direction;
   }
}

//-----------------------------------------------------------------------------

U32 greatestCommonDivisor( U32 u, U32 v )
{
   // http://en.wikipedia.org/wiki/Binary_GCD_algorithm
      
   S32 shift;

   /* GCD(0,x) := x */
   if (u == 0 || v == 0)
      return u | v;

   /* Left shift := lg K, where K is the greatest power of 2
   dividing both u and v. */
   for (shift = 0; ((u | v) & 1) == 0; ++shift) {
      u >>= 1;
      v >>= 1;
   }

   while ((u & 1) == 0)
      u >>= 1;

   /* From here on, u is always odd. */
   do {
      while ((v & 1) == 0)  /* Loop X */
         v >>= 1;

      /* Now u and v are both odd, so diff(u, v) is even.
      Let u = min(u, v), v = diff(u, v)/2. */
      if (u < v) {
         v -= u;
      } else {
         U32 diff = u - v;
         u = v;
         v = diff;
      }
      v >>= 1;
   } while (v != 0);

   return u << shift;
}

//-----------------------------------------------------------------------------

bool mLineTriangleCollide( const Point3F &p1, const Point3F &p2, 
                           const Point3F &t1, const Point3F &t2, const Point3F &t3,
                           Point3F *outUVW, F32 *outT )
{
   VectorF ab = t2 - t1;
   VectorF ac = t3 - t1;
   VectorF qp = p1 - p2;

   // Compute triangle normal. Can be precalculated or cached if
   // intersecting multiple segments against the same triangle
   VectorF n = mCross( ab, ac );

   // Compute denominator d. If d <= 0, segment is parallel to or points
   // away from triangle, so exit early
   F32 d = mDot( qp, n );
   if ( d <= 0.0f ) 
      return false;

   // Compute intersection t value of pq with plane of triangle. A ray
   // intersects if 0 <= t. Segment intersects iff 0 <= t <= 1. Delay
   // dividing by d until intersection has been found to pierce triangle
   VectorF ap = p1 - t1;
   F32 t = mDot( ap, n );
   if ( t < 0.0f ) 
      return false;      
   if ( t > d ) 
      return false; // For segment; exclude this code line for a ray test

   // Compute barycentric coordinate components and test if within bounds
   VectorF e = mCross( qp, ap );
   F32 v = mDot( ac, e );
   if ( v < 0.0f || v > d )
      return false;
   F32 w = -mDot( ab, e );
   if ( w < 0.0f || v + w > d ) 
      return false;

   // Segment/ray intersects triangle. Perform delayed division and
   // compute the last barycentric coordinate component
   const F32 ood = 1.0f / d;
   
   if ( outT )
      *outT = t * ood;
      
   if ( outUVW )
   {
      v *= ood;
      w *= ood;
      outUVW->set( 1.0f - v - w, v, w );
   }

   return true;
}

//-----------------------------------------------------------------------------

bool mRayQuadCollide(   const Quad &quad, 
                        const Ray &ray, 
                        Point2F *outUV,
                        F32 *outT )
{
   static const F32 eps = F32(10e-6);

   // Rejects rays that are parallel to Q, and rays that intersect the plane of
   // Q either on the left of the line V00V01 or on the right of the line V00V10.

   //   p01-----eXX-----p11
   //   ^            . ^ |
   //   |          .     |
   //   e03     e02     eXX
   //   |    .           |
   //   |  .             |
   //   p00-----e01---->p10

   VectorF e01 = quad.p10 - quad.p00;
   VectorF e03 = quad.p01 - quad.p00;

   // If the ray is perfectly perpendicular to e03, which 
   // represents the entire planes tangent, then the 
   // result of this cross product (P) will equal e01
   // If it is parallel it will result in a vector opposite e01.

   // If the ray is heading DOWN the cross product will point to the RIGHT
   // If the ray is heading UP the cross product will point to the LEFT
   // We do not reject based on this though...
   //
   // In either case cross product will be more parallel to e01 the more
   // perpendicular the ray is to e03, and it will be more perpendicular to
   // e01 the more parallel it is to e03.
   VectorF P = mCross(ray.direction, e03);

   // det can be seen as 'the amount of vector e01 in the direction P'
   F32 det = mDot(e01, P);

   // Take a Abs of the dot because we do not care if the ray is heading up or down,
   // but if it is perfectly parallel to the quad we want to reject it.
   if ( mFabs(det) < eps ) 
      return false;

   F32 inv_det = 1.0f / det;

   VectorF T = ray.origin - quad.p00;

   // alpha can be seen as 'the amount of vector T in the direction P'
   // T is a vector up from the quads corner point 00 to the ray's origin.
   // P is the cross product of the ray and e01, which should be "roughly"
   // parallel with e03 but might be of either positive or negative magnitude.
   F32 alpha = mDot(T, P) * inv_det;
   if ( alpha < 0.0f ) 
      return false;

   // if (alpha > real(1.0)) return false; // Uncomment if VR is used.

   // The cross product of T and e01 should be roughly parallel to e03
   // and of either positive or negative magnitude.
   VectorF Q = mCross(T, e01);
   F32 beta = mDot(ray.direction, Q) * inv_det;
   if ( beta < 0.0f ) 
      return false; 

   // if (beta > real(1.0)) return false; // Uncomment if VR is used.

   if ( alpha + beta > 1.0f ) 
   //if ( false )
   {
      // Rejects rays that intersect the plane of Q either on the
      // left of the line V11V10 or on the right of the line V11V01.

      VectorF e23 = quad.p01 - quad.p11;
      VectorF e21 = quad.p10 - quad.p11;
      VectorF P_prime = mCross(ray.direction, e21);
      F32 det_prime = mDot(e23, P_prime);
      if ( mFabs(det_prime) < eps) 
         return false;
      F32 inv_det_prime = 1.0f / det_prime;
      VectorF T_prime = ray.origin - quad.p11;
      F32 alpha_prime = mDot(T_prime, P_prime) * inv_det_prime;
      if (alpha_prime < 0.0f) 
         return false;
      VectorF Q_prime = mCross(T_prime, e23);
      F32 beta_prime = mDot(ray.direction, Q_prime) * inv_det_prime;
      if (beta_prime < 0.0f) 
         return false;
   }

   // Compute the ray parameter of the intersection point, and
   // reject the ray if it does not hit Q.

   F32 t = mDot(e03, Q) * inv_det;
   if ( t < 0.0f ) 
      return false; 


   // Compute the barycentric coordinates of the fourth vertex.
   // These do not depend on the ray, and can be precomputed
   // and stored with the quadrilateral.  

   F32 alpha_11, beta_11;
   VectorF e02 = quad.p11 - quad.p00;
   VectorF n = mCross(e01, e03);

   if ( mFabs(n.x) >= mFabs(n.y) && 
      mFabs(n.x) >= mFabs(n.z) )
   {
      alpha_11 = ( e02.y * e03.z - e02.z * e03.y ) / n.x;
      beta_11  = ( e01.y * e02.z - e01.z * e02.y ) / n.x;
   }
   else if ( mFabs(n.y) >= mFabs(n.x) && 
      mFabs(n.y) >= mFabs(n.z) ) 
   {  
      alpha_11 = ((e02.z * e03.x) - (e02.x * e03.z)) / n.y;
      beta_11  = ((e01.z * e02.x) - (e01.x * e02.z)) / n.y;
   }
   else 
   {
      alpha_11 = ((e02.x * e03.y) - (e02.y * e03.x)) / n.z;
      beta_11  = ((e01.x * e02.y) - (e01.y * e02.x)) / n.z;
   }

   // Compute the bilinear coordinates of the intersection point.

   F32 u,v;

   if ( mFabs(alpha_11 - 1.0f) < eps) 
   {    
      // Q is a trapezium.
      u = alpha;
      if ( mFabs(beta_11 - 1.0f) < eps) 
         v = beta; // Q is a parallelogram.
      else 
         v = beta / ((u * (beta_11 - 1.0f)) + 1.0f); // Q is a trapezium.
   }
   else if ( mFabs(beta_11 - 1.0f) < eps) 
   {
      // Q is a trapezium.
      v = beta;
      u = alpha / ((v * (alpha_11 - 1.0f)) + 1.0f);
   }
   else 
   {
      F32 A = 1.0f - beta_11;
      F32 B = (alpha * (beta_11 - 1.0f))
         - (beta * (alpha_11 - 1.0f)) - 1.0f;
      F32 C = alpha;
      F32 D = (B * B) - (4.0f * A * C);
      F32 Q = -0.5f * (B + (B < 0.0f ? -1.0f : 1.0f) ) * mSqrt(D);
      u = Q / A;
      if ((u < 0.0f) || (u > 1.0f)) u = C / Q;
      v = beta / ((u * (beta_11 - 1.0f)) + 1.0f); 
   }

   if ( outUV )
      outUV->set( u, v );  
   if ( outT )
      *outT = t;

   return true;  
}

//-----------------------------------------------------------------------------

// Used by sortQuadWindingOrder.
struct QuadSortPoint
{
   U32 id;
   F32 theta;
};

// Used by sortQuadWindingOrder.
S32 QSORT_CALLBACK cmpAngleAscending( const void *a, const void *b )
{
   const QuadSortPoint *p0 = (const QuadSortPoint*)a;
   const QuadSortPoint *p1 = (const QuadSortPoint*)b;   

	F32 diff = p1->theta - p0->theta;

	if ( diff > 0.0f )
		return -1;
	else if ( diff < 0.0f )
		return 1;
	else
		return 0;   
}

// Used by sortQuadWindingOrder.
S32 QSORT_CALLBACK cmpAngleDescending( const void *a, const void *b )
{
	const QuadSortPoint *p0 = (const QuadSortPoint*)a;
	const QuadSortPoint *p1 = (const QuadSortPoint*)b;   

	F32 diff = p1->theta - p0->theta;

	if ( diff > 0.0f )
		return 1;
	else if ( diff < 0.0f )
		return -1;
	else
		return 0;   
}

void sortQuadWindingOrder( const MatrixF &quadMat, bool clockwise, const Point3F *verts, U32 *vertMap, U32 count )
{
   PROFILE_SCOPE( MathUtils_sortQuadWindingOrder );

   if ( count == 0 )
      return;   
   
   Point3F *quadPoints = new Point3F[count];
   
   for ( S32 i = 0; i < count; i++ )   
	{		
      quadMat.mulP( verts[i], &quadPoints[i] );
		quadPoints[i].normalizeSafe();
	}

   sortQuadWindingOrder( clockwise, quadPoints, vertMap, count );

   delete [] quadPoints;
}

void sortQuadWindingOrder( bool clockwise, const Point3F *verts, U32 *vertMap, U32 count )
{
   QuadSortPoint *sortPoints = new QuadSortPoint[count];

   for ( S32 i = 0; i < count; i++ )
   {
      QuadSortPoint &sortPnt = sortPoints[i];
      const Point3F &vec = verts[i];

      sortPnt.id = i;

      F32 theta = mAtan2( vec.y, vec.x );

      if ( vec.y < 0.0f )
         theta = M_2PI_F + theta;

      sortPnt.theta = theta;
   }

   dQsort( sortPoints, count, sizeof( QuadSortPoint ), clockwise ? cmpAngleDescending : cmpAngleAscending ); 

   for ( S32 i = 0; i < count; i++ )
      vertMap[i] = sortPoints[i].id;

   delete [] sortPoints;
}

//-----------------------------------------------------------------------------

void buildMatrix( const VectorF *rvec, const VectorF *fvec, const VectorF *uvec, const VectorF *pos, MatrixF *outMat )
{     
   /// Work in Progress

   /*
   AssertFatal( !rvec || rvec->isUnitLength(), "MathUtils::buildMatrix() - Right vector was not normalized!" );  
   AssertFatal( !fvec || fvec->isUnitLength(), "MathUtils::buildMatrix() - Forward vector was not normalized!" );  
   AssertFatal( !uvec || uvec->isUnitLength(), "MathUtils::buildMatrix() - Up vector was not normalized!" ); 

   // Note this relationship:
   //
   // Column0  Column1  Column2
   // Axis X   Axis Y   Axis Z
   // Rvec     Fvec     Uvec
   //

   enum 
   {
      RVEC = 1,
      FVEC = 1 << 1,
      UVEC = 1 << 2,
      ALL = RVEC | FVEC | UVEC
   };

   U8 mask = 0;
   U8 count = 0;
   U8 axis0, axis1;

   if ( rvec )
   {
      mask |= RVEC;
      axis0 == 0;      
      count++;
   }
   if ( fvec )
   {
      mask |= FVEC;      
      if ( count == 0 )
         axis0 = 1;
      else
         axis1 = 1;
      count++;
   }
   if ( uvec )
   {
      mask |= UVEC;      
      count++;
   }

   U8 bR = 1;
   U8 bF = 1 << 1;
   U8 bU = 1 << 2;
   U8 bRF = bR | bF;
   U8 bRU = bR | bU;
   U8 bFU = bF | bU;
   U8 bRFU = bR | bF | bU;

   

   // Cross product map.
   U8 cpdMap[3][2] = 
   {
      { 1, 2 },
      { 2, 0 },
      { 0, 1 },
   }

   if ( count == 1 )
   {
      if ( mask == bR )
      {
         
      }
      else if ( mask == bF )
      {

      }
      else if ( mask == bU )
      {

      }
   }
   else if ( count == 2 )
   {
      if ( mask == bRF )
      {

      }
      else if ( mask == bRU )
      {

      }
      else if ( mask == bFU )
      {

      }
   }
   else // bRFU
   {

   }

   if ( rvec )
   {
      outMat->setColumn( 0, *rvec );

      if ( fvec )
      {
         outMat->setColumn( 1, *fvec );

         if ( uvec )         
            outMat->setColumn( 2, *uvec );         
         else
         {
            // Set uvec from rvec/fvec
            tmp = mCross( rvec, fvec );
            tmp.normalizeSafe();
            outMat->setColumn( 2, tmp );
         }
      }
      else if ( uvec )
      {
         // Set fvec from uvec/rvec
         tmp = mCross( uvec, rvec );
         tmp.normalizeSafe();
         outMat->setColumn( 1, tmp );
      }
      else
      {
         // Set fvec and uvec from rvec
         Point3F tempFvec = mPerp( rvec );
         Point3F tempUvec = mCross( )

      }
   }
   AssertFatal( rvec->isUnitLength(), "MathUtils::buildMatrix() - Right vector was not normalized!" );
   AssertFatal( fvec->isUnitLength(), "MathUtils::buildMatrix() - Forward vector was not normalized!" );
   AssertFatal( uvec->isUnitLength(), "MathUtils::buildMatrix() - UpVector vector was not normalized!" );
   AssertFatal( outMat, "MathUtils::buildMatrix() - Got null output matrix!" );
   AssertFatal( outMat->isAffine(), "MathUtils::buildMatrix() - Got uninitialized matrix!" );
   */
}

//-----------------------------------------------------------------------------

bool reduceFrustum( const Frustum& frustum, const RectI& viewport, const RectF& area, Frustum& outFrustum )
{
   // Just to be safe, clamp the area to the viewport.

   Point2F clampedMin;
   Point2F clampedMax;

   clampedMin.x = mClampF( area.extent.x, ( F32 ) viewport.point.x, ( F32 ) viewport.point.x + viewport.extent.x );
   clampedMin.y = mClampF( area.extent.y, ( F32 ) viewport.point.y, ( F32 ) viewport.point.y + viewport.extent.y );

   clampedMax.x = mClampF( area.extent.x, ( F32 ) viewport.point.x, ( F32 ) viewport.point.x + viewport.extent.x );
   clampedMax.y = mClampF( area.extent.y, ( F32 ) viewport.point.y, ( F32 ) viewport.point.y + viewport.extent.y );

   // If we have ended up without a visible region on the screen,
   // terminate now.
   
   if(   mFloor( clampedMin.x ) == mFloor( clampedMax.x ) ||
         mFloor( clampedMin.y ) == mFloor( clampedMax.y ) )
      return false;

   // Get the extents of the frustum.

   const F32 frustumXExtent = mFabs( frustum.getNearRight() - frustum.getNearLeft() );
   const F32 frustumYExtent = mFabs( frustum.getNearTop() - frustum.getNearBottom() );

   // Now, normalize the screen-space pixel coordinates to lie within the screen-centered
   // -1 to 1 coordinate space that is used for the frustum planes.

   Point2F normalizedMin;
   Point2F normalizedMax;

   normalizedMin.x = ( ( clampedMin.x / viewport.extent.x ) * frustumXExtent ) - ( frustumXExtent / 2.f );
   normalizedMin.y = ( ( clampedMin.y / viewport.extent.y ) * frustumYExtent ) - ( frustumYExtent / 2.f );
   normalizedMax.x = ( ( clampedMax.x / viewport.extent.x ) * frustumXExtent ) - ( frustumXExtent / 2.f );
   normalizedMax.y = ( ( clampedMax.y / viewport.extent.y ) * frustumYExtent ) - ( frustumYExtent / 2.f );

   // Make sure the generated frustum metrics are somewhat sane.

   if( normalizedMax.x - normalizedMin.x < 0.001f ||
       normalizedMax.y - normalizedMin.y < 0.001f )
      return false;
   
   // Finally, create the new frustum using the original's frustum
   // information except its left/right/top/bottom planes.
   //
   // Note that screen-space coordinates go upside down on Y whereas
   // camera-space frustum coordinates go downside up on Y which is
   // why we are inverting Y here.

   outFrustum.set(
      frustum.isOrtho(),
      normalizedMin.x,
      normalizedMax.x,
      - normalizedMin.y,
      - normalizedMax.y,
      frustum.getNearDist(),
      frustum.getFarDist(),
      frustum.getTransform()
   );

   return true;
}

//-----------------------------------------------------------------------------

void makeFrustum( F32 *outLeft,
                  F32 *outRight,
                  F32 *outTop,
                  F32 *outBottom,
                  F32 fovYInRadians, 
                  F32 aspectRatio, 
                  F32 nearPlane )
{
   F32 top = nearPlane * mTan( fovYInRadians / 2.0 );
   if ( outTop ) *outTop = top;
   if ( outBottom ) *outBottom = -top;

   F32 left = top * aspectRatio;
   if ( outLeft ) *outLeft = -left;
   if ( outRight ) *outRight = left;
}

//-----------------------------------------------------------------------------

void makeProjection( MatrixF *outMatrix, 
                     F32 fovYInRadians, 
                     F32 aspectRatio, 
                     F32 nearPlane, 
                     F32 farPlane,
                     bool gfxRotate )
{
   F32 left, right, top, bottom;
   makeFrustum( &left, &right, &top, &bottom, fovYInRadians, aspectRatio, nearPlane );
   makeProjection( outMatrix, left, right, top, bottom, nearPlane, farPlane, gfxRotate );
}

//-----------------------------------------------------------------------------

void makeFovPortFrustum(
   Frustum *outFrustum,
   bool isOrtho,
   F32 nearDist,
   F32 farDist,
   const FovPort &inPort,
   const MatrixF &transform)
{
   F32 leftSize = nearDist * inPort.leftTan;
   F32 rightSize = nearDist * inPort.rightTan;
   F32 upSize = nearDist * inPort.upTan;
   F32 downSize = nearDist * inPort.downTan;

   F32 left = -leftSize;
   F32 right = rightSize;
   F32 top = upSize;
   F32 bottom = -downSize;

   outFrustum->set(isOrtho, left, right, top, bottom, nearDist, farDist, transform);
}

//-----------------------------------------------------------------------------

/// This is the special rotation matrix applied to
/// projection matricies for GFX.
///
/// It is a wart of the OGL to DX change over.
///
static const MatrixF sGFXProjRotMatrix( EulerF( (M_PI_F / 2.0f), 0.0f, 0.0f ) );

void makeProjection( MatrixF *outMatrix, 
                     F32 left, 
                     F32 right, 
                     F32 top, 
                     F32 bottom, 
                     F32 nearPlane, 
                     F32 farPlane,
                     bool gfxRotate )
{

   Point4F row;
   row.x = 2.0*nearPlane / (right-left);
   row.y = 0.0;
   row.z = 0.0;
   row.w = 0.0;
   outMatrix->setRow( 0, row );

   row.x = 0.0;
   row.y = 2.0 * nearPlane / (top-bottom);
   row.z = 0.0;
   row.w = 0.0;
   outMatrix->setRow( 1, row );

   row.x = (left+right) / (right-left);
   row.y = (top+bottom) / (top-bottom);
   row.z = farPlane / (nearPlane-farPlane);
   row.w = -1.0;
   outMatrix->setRow( 2, row );

   row.x = 0.0;
   row.y = 0.0;
   row.z = nearPlane * farPlane / (nearPlane-farPlane);
   row.w = 0.0;
   outMatrix->setRow( 3, row );

   outMatrix->transpose();

   if ( gfxRotate )
      outMatrix->mul( sGFXProjRotMatrix );
}

//-----------------------------------------------------------------------------

void makeOrthoProjection(  MatrixF *outMatrix, 
                           F32 left, 
                           F32 right, 
                           F32 top, 
                           F32 bottom, 
                           F32 nearPlane, 
                           F32 farPlane,
                           bool gfxRotate )
{
   Point4F row;
   row.x = 2.0f / (right - left);
   row.y = 0.0f;
   row.z = 0.0f;
   row.w = 0.0f;
   outMatrix->setRow( 0, row );

   row.x = 0.0f;
   row.y = 2.0f / (top - bottom);
   row.z = 0.0f;
   row.w = 0.0f;
   outMatrix->setRow( 1, row );

   row.x = 0.0f;
   row.y = 0.0f;
   row.w = 0.0f;

   // This may need be modified to work with OpenGL (d3d has 0..1 
   // projection for z, vs -1..1 in OpenGL)
   row.z = 1.0f / (nearPlane - farPlane); 

   outMatrix->setRow( 2, row );

   row.x = (left + right) / (left - right);
   row.y = (top + bottom) / (bottom - top);
   row.z = nearPlane / (nearPlane - farPlane);
   row.w = 1.0f;
   outMatrix->setRow( 3, row );

   outMatrix->transpose();

   if ( gfxRotate )
      outMatrix->mul( sGFXProjRotMatrix );
}

//-----------------------------------------------------------------------------

bool edgeFaceIntersect( const Point3F &edgeA, const Point3F &edgeB, 
                        const Point3F &faceA, const Point3F &faceB, const Point3F &faceC, const Point3F &faceD, Point3F *intersection )
{
   VectorF edgeAB = edgeB - edgeA;
   VectorF edgeAFaceA = faceA - edgeA;
   VectorF edgeAFaceB = faceB - edgeA;
   VectorF edgeAFaceC = faceC - edgeA;

   VectorF m = mCross( edgeAFaceC, edgeAB );
   F32 v = mDot( edgeAFaceA, m );
   if ( v >= 0.0f )
   {
      F32 u = -mDot( edgeAFaceB, m );
      if ( u < 0.0f )
         return false;

      VectorF tmp = mCross( edgeAFaceB, edgeAB );
      F32 w = mDot( edgeAFaceA, tmp );
      if ( w < 0.0f )
         return false;

      F32 denom = 1.0f / (u + v + w );
      u *= denom;
      v *= denom;
      w *= denom;

      (*intersection) = u * faceA + v * faceB + w * faceC;
   }
   else
   {
      VectorF edgeAFaceD = faceD - edgeA;
      F32 u = mDot( edgeAFaceD, m );
      if ( u < 0.0f )
         return false;

      VectorF tmp = mCross( edgeAFaceA, edgeAB );
      F32 w = mDot( edgeAFaceD, tmp );
      if ( w < 0.0f )
         return false;

      v = -v;

      F32 denom = 1.0f / ( u + v + w );
      u *= denom;
      v *= denom;
      w *= denom;

      (*intersection) = u * faceA + v * faceD + w * faceC;
   }

   return true;
}

//-----------------------------------------------------------------------------

bool isPlanarPolygon( const Point3F* vertices, U32 numVertices )
{
   AssertFatal( vertices != NULL, "MathUtils::isPlanarPolygon - Received NULL pointer" );
   AssertFatal( numVertices >= 3, "MathUtils::isPlanarPolygon - Must have at least three vertices" );

   // Triangles are always planar.  Letting smaller numVertices
   // slip through provides robustness for errors in release builds.
   
   if( numVertices <= 3 )
      return true;

   // Compute the normal of the first triangle in the polygon.
   
   Point3F triangle1Normal = mTriangleNormal( vertices[ 0 ], vertices[ 1 ], vertices[ 2 ] );

   // Now go through all the remaining vertices and build triangles
   // with the first two vertices.  Then the normals of all these triangles
   // must be the same (minus some variance due to floating-point inaccuracies)
   // as the normal of the first triangle.

   for( U32 i = 3; i < numVertices; ++ i )
   {
      Point3F triangle2Normal = mTriangleNormal( vertices[ 0 ], vertices[ 1 ], vertices[ i ] );
      if( !triangle1Normal.equal( triangle2Normal ) )
         return false;
   }

   return true;
}

//-----------------------------------------------------------------------------

bool isConvexPolygon( const Point3F* vertices, U32 numVertices )
{
   AssertFatal( vertices != NULL, "MathUtils::isConvexPolygon - Received NULL pointer" );
   AssertFatal( numVertices >= 3, "MathUtils::isConvexPolygon - Must have at least three vertices" );

   // Triangles are always convex.  Letting smaller numVertices
   // slip through provides robustness for errors in release builds.

   if( numVertices <= 3 )
      return true;

   U32 numPositive = 0;
   U32 numNegative = 0;

   for( U32 i = 0; i < numVertices; ++ i )
   {
      const Point3F& a = vertices[ i ];
      const Point3F& b = vertices[ ( i + 1 ) % numVertices ];
      const Point3F& c = vertices[ ( i + 2 ) % numVertices ];

      const F32 crossProductLength = mCross( b - a, c - b ).len();
      
      if( crossProductLength < 0.f )
         numNegative ++;
      else if( crossProductLength > 0.f )
         numPositive ++;

      if( numNegative && numPositive )
         return false;
   }

   return true;
}

//-----------------------------------------------------------------------------

bool clipFrustumByPolygon( const Point3F* points, U32 numPoints, const RectI& viewport, const MatrixF& world,
                           const MatrixF& projection, const Frustum& inFrustum, const Frustum& rootFrustum, Frustum& outFrustum )
{
   enum
   {
      MAX_RESULT_VERTICES = 64,
      MAX_INPUT_VERTICES = MAX_RESULT_VERTICES - Frustum::PlaneCount // Clipping against each plane may add a vertex.
   };

   AssertFatal( numPoints <= MAX_INPUT_VERTICES, "MathUtils::clipFrustumByPolygon - Too many vertices!" );
   if( numPoints > MAX_INPUT_VERTICES )
      return false;

   // First, we need to clip the polygon against inFrustum.
   //
   // Use two buffers here in interchanging roles as sources and targets
   // in clipping against the frustum planes.

   Point3F polygonBuffer1[ MAX_RESULT_VERTICES ];
   Point3F polygonBuffer2[ MAX_RESULT_VERTICES ];

   Point3F* tempPolygon = polygonBuffer1;
   Point3F* clippedPolygon = polygonBuffer2;

   dMemcpy( clippedPolygon, points, numPoints * sizeof( points[ 0 ] ) );

   U32 numClippedPolygonVertices = numPoints;
   U32 numTempPolygonVertices = 0;

   for( U32 nplane = 0; nplane < Frustum::PlaneCount; ++ nplane )
   {
      // Make the output of the last iteration the
      // input of this iteration.

      swap( tempPolygon, clippedPolygon );
      numTempPolygonVertices = numClippedPolygonVertices;

      // Clip our current remainder of the original polygon
      // against the current plane.

      const PlaneF& plane = inFrustum.getPlanes()[ nplane ];
      numClippedPolygonVertices = plane.clipPolygon( tempPolygon, numTempPolygonVertices, clippedPolygon );

      // If the polygon was completely on the backside of the plane,
      // then polygon is outside the frustum.  In this case, return false
      // to indicate we haven't clipped anything.

      if( !numClippedPolygonVertices )
         return false;
   }

   // Project the clipped polygon into screen space.

   MatrixF worldProjection = projection;
   worldProjection.mul( world ); // Premultiply world*projection so we don't have to do this over and over for each point.

   Point3F projectedPolygon[ 10 ];
   for( U32 i = 0; i < numClippedPolygonVertices; ++ i )
      mProjectWorldToScreen(
         clippedPolygon[ i ],
         &projectedPolygon[ i ],
         viewport,
         worldProjection
      );

   // Put an axis-aligned rectangle around our polygon.

   Point2F minPoint( projectedPolygon[ 0 ].x, projectedPolygon[ 0 ].y );
   Point2F maxPoint( projectedPolygon[ 0 ].x, projectedPolygon[ 0 ].y );

   for( U32 i = 1; i < numClippedPolygonVertices; ++ i )
   {
      minPoint.setMin( Point2F( projectedPolygon[ i ].x, projectedPolygon[ i ].y ) );
      maxPoint.setMax( Point2F( projectedPolygon[ i ].x, projectedPolygon[ i ].y ) );
   }

   RectF area( minPoint, maxPoint - minPoint );

   // Finally, reduce the input frustum to the given area.  Note that we
   // use rootFrustum here instead of inFrustum as the latter does not necessarily
   // represent the full viewport we are using here which thus would skew the mapping.

   return reduceFrustum( rootFrustum, viewport, area, outFrustum );
}

//-----------------------------------------------------------------------------

U32 extrudePolygonEdges( const Point3F* vertices, U32 numVertices, const Point3F& direction, PlaneF* outPlanes )
{
   U32 numPlanes = 0;
   U32 lastVertex = numVertices - 1;
   bool invert = false;

   for( U32 i = 0; i < numVertices; lastVertex = i, ++ i )
   {
      const Point3F& v1 = vertices[ i ];
      const Point3F& v2 = vertices[ lastVertex ];

      // Skip the edge if it's length is really short.

      const Point3F edgeVector = v2 - v1;
      if( edgeVector.len() < 0.05 )
         continue;

      // Compute the plane normal.  The direction and the edge vector
      // basically define the orientation of the plane so their cross
      // product is the plane normal.

      Point3F normal;
      if( !invert )
         normal = mCross( edgeVector, direction );
      else
         normal = mCross( direction, edgeVector );

      // Create a plane for the edge.

      outPlanes[ numPlanes ] = PlaneF( v1, normal );
      numPlanes ++;

      // If this is the first plane that we have created, find out whether
      // the vertex ordering is giving us the plane orientations that we want
      // (facing inside).  If not, invert vertex order from now on.

      if( i == 0 )
      {
         const PlaneF& plane = outPlanes[ numPlanes - 1 ];
         for( U32 n = i + 1; n < numVertices; ++ n )
         {
            const PlaneF::Side side = plane.whichSide( vertices[ n ] );
            if( side == PlaneF::On )
               continue;

            if( side != PlaneF::Front )
               invert = true;
            break;
         }
      }
   }

   return numPlanes;
}

//-----------------------------------------------------------------------------

U32 extrudePolygonEdgesFromPoint( const Point3F* vertices, U32 numVertices, const Point3F& fromPoint, PlaneF* outPlanes )
{
   U32 numPlanes = 0;
   U32 lastVertex = numVertices - 1;
   bool invert = false;

   for( U32 i = 0; i < numVertices; lastVertex = i, ++ i )
   {
      const Point3F& v1 = vertices[ i ];
      const Point3F& v2 = vertices[ lastVertex ];

      // Skip the edge if it's length is really short.

      const Point3F edgeVector = v2 - v1;
      if( edgeVector.len() < 0.05 )
         continue;

      // Create a plane for the edge.

      if( !invert )
         outPlanes[ numPlanes ] = PlaneF( v1, fromPoint, v2 );
      else
         outPlanes[ numPlanes ] = PlaneF( v2, fromPoint, v1 );
      
      numPlanes ++;

      // If this is the first plane that we have created, find out whether
      // the vertex ordering is giving us the plane orientations that we want
      // (facing inside).  If not, invert vertex order from now on.

      if( i == 0 )
      {
         const PlaneF& plane = outPlanes[ numPlanes - 1 ];
         for( U32 n = i + 1; n < numVertices; ++ n )
         {
            const PlaneF::Side side = plane.whichSide( vertices[ n ] );
            if( side == PlaneF::On )
               continue;

            if( side != PlaneF::Front )
               invert = true;
            break;
         }
      }
   }

   return numPlanes;
}

//-----------------------------------------------------------------------------

void mBuildHull2D(const Vector<Point2F> _inPoints, Vector<Point2F> &hullPoints)
{
   /// Andrew's monotone chain convex hull algorithm implementation

   struct Util
   {
      //compare by x and then by y   
      static int CompareLexicographic( const Point2F *a, const Point2F *b)
      {
         return a->x < b->x || (a->x == b->x && a->y < b->y);
      }
   };

   hullPoints.clear();
   hullPoints.setSize( _inPoints.size()*2 );

   // sort in points by x and then by y
   Vector<Point2F> inSortedPoints = _inPoints;
   inSortedPoints.sort( &Util::CompareLexicographic );

   Point2F* lowerHullPtr = hullPoints.address();
   U32 lowerHullIdx = 0;

   //lower part of hull
   for( int i = 0; i < inSortedPoints.size(); ++i )
   {      
      while( lowerHullIdx >= 2 && mCross( lowerHullPtr[ lowerHullIdx - 2], lowerHullPtr[lowerHullIdx - 1], inSortedPoints[i] ) <= 0 )
         --lowerHullIdx;

      lowerHullPtr[lowerHullIdx++] = inSortedPoints[i];
   }

   --lowerHullIdx; // last point are the same as first in upperHullPtr

   Point2F* upperHullPtr = hullPoints.address() + lowerHullIdx;
   U32 upperHullIdx = 0;

   //upper part of hull
   for( int i = inSortedPoints.size()-1; i >= 0; --i )
   {
      while( upperHullIdx >= 2 && mCross( upperHullPtr[ upperHullIdx - 2], upperHullPtr[upperHullIdx - 1], inSortedPoints[i] ) <= 0 )
         --upperHullIdx;

      upperHullPtr[upperHullIdx++] = inSortedPoints[i];
   }

   hullPoints.setSize( lowerHullIdx + upperHullIdx );
}

} // namespace MathUtils
