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
#include "math/util/quadTransforms.h"


BiQuadToSqr::BiQuadToSqr(  const Point2F &p00, 
                           const Point2F &p10, 
                           const Point2F &p11, 
                           const Point2F &p01 )
   : m_kP00( p00 )
{
   m_kB = p10 - p00 ;   // width
   m_kC = p01 - p00;   // height
   m_kD = p11 + p00 - p10 - p01; // diagonal dist

   if(mFabs(m_kD.x) < POINT_EPSILON)    
      m_kD.x = 0.f;
   if(mFabs(m_kD.y) < POINT_EPSILON) 
      m_kD.y = 0.f;  

   m_fBC = mDotPerp( m_kB, m_kC );
   m_fBD = mDotPerp( m_kB, m_kD );   
   m_fCD = mDotPerp( m_kC, m_kD );
}

Point2F BiQuadToSqr::transform( const Point2F &p ) const
{
   Point2F kA = m_kP00 - p;
   
   F32 fAB = mDotPerp( kA, m_kB );
   F32 fAC = mDotPerp( kA, m_kC);

   // 0 = ac*bc+(bc^2+ac*bd-ab*cd)*s+bc*bd*s^2 = k0 + k1*s + k2*s^2
   F32 fK0 = fAC*m_fBC;
   F32 fK1 = m_fBC*m_fBC + fAC*m_fBD - fAB*m_fCD;
   F32 fK2 = m_fBC*m_fBD;

   if (mFabs(fK2) > POINT_EPSILON)
   {
      // s-equation is quadratic
      F32 fInv = 0.5f/fK2;
      F32 fDiscr = fK1*fK1 - 4.0f*fK0*fK2;
      F32 fRoot = mSqrt( mFabs(fDiscr) );

      Point2F kResult0( 0, 0 );
      kResult0.x = (-fK1 - fRoot)*fInv;
      kResult0.y = fAB/(m_fBC + m_fBD*kResult0.x);
      F32 fDeviation0 = deviation(kResult0);
      if ( fDeviation0 == 0.0f )
         return kResult0;

      Point2F kResult1( 0, 0 );
      kResult1.x = (-fK1 + fRoot)*fInv;
      kResult1.y = fAB/(m_fBC + m_fBD*kResult1.x);
      F32 fDeviation1 = deviation(kResult1);
      if ( fDeviation1 == 0.0f )
         return kResult1;

      if (fDeviation0 <= fDeviation1)
      {
         if ( fDeviation0 < POINT_EPSILON )
            return kResult0;
      }
      else
      {
         if ( fDeviation1 < POINT_EPSILON )
            return kResult1;
      }
   }
   else
   {
      // s-equation is linear
      Point2F kResult( 0, 0 );

      kResult.x = -fK0/fK1;
      kResult.y = fAB/(m_fBC + m_fBD*kResult.x);
      F32 fDeviation = deviation(kResult);
      if ( fDeviation < POINT_EPSILON )
         return kResult;
   }

   // point is outside the quadrilateral, return invalid
   return Point2F(F32_MAX,F32_MAX);
}

F32 BiQuadToSqr::deviation( const Point2F &sp )
{
   // deviation is the squared distance of the point from the unit square
   F32 fDeviation = 0.0f;
   F32 fDelta;

   if (sp.x < 0.0f)
   {
      fDeviation += sp.x*sp.x;
   }
   else if (sp.x > 1.0f)
   {
      fDelta = sp.x - 1.0f;
      fDeviation += fDelta*fDelta;
   }

   if (sp.y < 0.0f)
   {
      fDeviation += sp.y*sp.y;
   }
   else if (sp.y > 1.0f)
   {
      fDelta = sp.y - 1.0f;
      fDeviation += fDelta*fDelta;
   }

   return fDeviation;
}


BiSqrToQuad3D::BiSqrToQuad3D( const Point3F& pnt00,
                              const Point3F& pnt10, 
                              const Point3F& pnt11,
                              const Point3F& pnt01)
{
   p00 = pnt00;
   p10 = pnt10;
   p11 = pnt11;
   p01 = pnt01;
}

Point3F BiSqrToQuad3D::transform( const Point2F &p ) const
{   
   //Let p00, p10, p01, and p11 be your 3-tuples that are the quad's
   //vertices.  You can parameterize the quad as follows.

   //q(s,t) = (1-s)*((1-t)*p00 + t*p01) + s*((1-t)*p10 + t*p11)

   //for 0 <= s <= 1 and 0 <= t <= 1.  Notice that q(0,0) = p00,
   //q(1,0) = p10, q(0,1) = p01, and q(1,1) = p11, so the parameter
   //"square" whose points are (s,t) will be mapped to the quad.

   const F32 &s = p.x;
   const F32 &t = p.y;

   Point3F result = (1.0f-s)*((1.0f-t)*p00 + t*p01) + s*((1.0f-t)*p10 + t*p11);
   return result;   
}

