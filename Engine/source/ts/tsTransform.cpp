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

#include "ts/tsTransform.h"
#include "core/stream/stream.h"

void Quat16::identity()
{
   x = y = z = 0;
   w = MAX_VAL;
}

QuatF Quat16::getQuatF() const
{
   return QuatF ( F32(x) / F32(MAX_VAL),
                  F32(y) / F32(MAX_VAL),
                  F32(z) / F32(MAX_VAL),
                  F32(w) / F32(MAX_VAL) );
}

QuatF & Quat16::getQuatF( QuatF * q ) const
{
   q->x = F32( x ) / F32(MAX_VAL);
   q->y = F32( y ) / F32(MAX_VAL);
   q->z = F32( z ) / F32(MAX_VAL);
   q->w = F32( w ) / F32(MAX_VAL);
   return *q;
}

void Quat16::set( const QuatF & q )
{
   x = (S16)(q.x * F32(MAX_VAL));
   y = (S16)(q.y * F32(MAX_VAL));
   z = (S16)(q.z * F32(MAX_VAL));
   w = (S16)(q.w * F32(MAX_VAL));
}

S32 Quat16::operator==( const Quat16 & q ) const
{
   return( x == q.x && y == q.y && z == q.z && w == q.w );
}

void Quat16::read(Stream * s)
{
    s->read(&x);
    s->read(&y);
    s->read(&z);
    s->read(&w);
}

void Quat16::write(Stream * s)
{
    s->write(x);
    s->write(y);
    s->write(z);
    s->write(w);
}

QuatF & TSTransform::interpolate( const QuatF & q1, const QuatF & q2, F32 interp, QuatF * q )
{
   F32 Dot;
   F32 Dist2;
   F32 OneOverL;
   F32 x1,y1,z1,w1;
   F32 x2,y2,z2,w2;

   //
   // This is a linear interpolation with a fast renormalization.
   //
   x1 = q1.x;
   y1 = q1.y;
   z1 = q1.z;
   w1 = q1.w;
   x2 = q2.x;
   y2 = q2.y;
   z2 = q2.z;
   w2 = q2.w;

   // Determine if quats are further than 90 degrees
   Dot = x1*x2 + y1*y2 + z1*z2 + w1*w2;

   // If dot is negative flip one of the quaterions
   if( Dot < 0.0f )
   {
       x1 = -x1;
       y1 = -y1;
       z1 = -z1;
       w1 = -w1;
   }

   // Compute interpolated values
   x1 = x1 + interp*(x2 - x1);
   y1 = y1 + interp*(y2 - y1);
   z1 = z1 + interp*(z2 - z1);
   w1 = w1 + interp*(w2 - w1);

   // Get squared distance of new quaternion
   Dist2 = x1*x1 + y1*y1 + z1*z1 + w1*w1;

   // Use home-baked polynomial to compute 1/sqrt(Dist2)
   // since we know the range is 0.707 >= Dist2 <= 1.0
   // we'll split in half.

   if( Dist2<0.857f )
      OneOverL = (((0.699368f)*Dist2) + -1.819985f)*Dist2 + 2.126369f;    //0.0000792
   else
      OneOverL = (((0.454012f)*Dist2) + -1.403517f)*Dist2 + 1.949542f;    //0.0000373

   // Renormalize
   q->x = x1*OneOverL;
   q->y = y1*OneOverL;
   q->z = z1*OneOverL;
   q->w = w1*OneOverL;

   return *q;
}

void TSTransform::applyScale(F32 scale, MatrixF * mat)
{
   mat->scale(Point3F(scale,scale,scale));
}

void TSTransform::applyScale(const Point3F & scale, MatrixF * mat)
{
   mat->scale(scale);
}

void TSTransform::applyScale(const TSScale & scale, MatrixF * mat)
{
   MatrixF mat2;
   TSTransform::setMatrix(scale.mRotate,&mat2);
   MatrixF mat3(mat2);
   mat3.inverse();
   mat2.scale(scale.mScale);
   mat2.mul(mat3);
   mat->mul(mat2);
}
