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
#include "math/mMathFn.h"

//--------------------------------------------------------------------------
#define EQN_EPSILON     (1e-8)

static inline void swap(F32 & a, F32 & b)
{
   F32 t = b;
   b = a;
   a = t;
}

static inline F32 mCbrt(F32 val)
{
   if(val < 0.f)
      return(-mPow(-val, F32(1.f/3.f)));
   else
      return(mPow(val, F32(1.f/3.f)));
}

static inline U32 mSolveLinear(F32 a, F32 b, F32 * x)
{
   if(mIsZero(a))
      return(0);

   x[0] = -b/a;
   return(1);
}

static U32 mSolveQuadratic_c(F32 a, F32 b, F32 c, F32 * x)
{
   // really linear?
   if(mIsZero(a))
      return(mSolveLinear(b, c, x));

   // get the descriminant:   (b^2 - 4ac)
   F32 desc = (b * b) - (4.f * a * c);

   // solutions:
   // desc < 0:   two imaginary solutions
   // desc > 0:   two real solutions (b +- sqrt(desc)) / 2a
   // desc = 0:   one real solution (b / 2a)
   if(mIsZero(desc))
   {
      x[0] = b / (2.f * a);
      return(1);
   }
   else if(desc > 0.f)
   {
      F32 sqrdesc = mSqrt(desc);
      F32 den = (2.f * a);
      x[0] = (-b + sqrdesc) / den;
      x[1] = (-b - sqrdesc) / den;

      if(x[1] < x[0])
         swap(x[0], x[1]);

      return(2);
   }
   else
      return(0);
}

//--------------------------------------------------------------------------
// from Graphics Gems I: pp 738-742
U32 mSolveCubic_c(F32 a, F32 b, F32 c, F32 d, F32 * x)
{
   if(mIsZero(a))
      return(mSolveQuadratic(b, c, d, x));

   // normal form: x^3 + Ax^2 + BX + C = 0
   F32 A = b / a;
   F32 B = c / a;
   F32 C = d / a;

   // substitute x = y - A/3 to eliminate quadric term and depress
   // the cubic equation to (x^3 + px + q = 0)
   F32 A2 = A * A;
   F32 A3 = A2 * A;

   F32 p = (1.f/3.f) * (((-1.f/3.f) * A2) + B);
   F32 q = (1.f/2.f) * (((2.f/27.f) * A3) - ((1.f/3.f) * A * B) + C);

   // use Cardano's fomula to solve the depressed cubic
   F32 p3 = p * p * p;
   F32 q2 = q * q;

   F32 D = q2 + p3;

   U32 num = 0;

   if(mIsZero(D))          // 1 or 2 solutions
   {
      if(mIsZero(q)) // 1 triple solution
      {
         x[0] = 0.f;
         num = 1;
      }
      else // 1 single and 1 double
      {
         F32 u = mCbrt(-q);
         x[0] = 2.f * u;
         x[1] = -u;
         num = 2;
      }
   }
   else if(D < 0.f)        // 3 solutions: casus irreducibilis
   {
      F32 phi = (1.f/3.f) * mAcos(-q / mSqrt(-p3));
      F32 t = 2.f * mSqrt(-p);

      x[0] = t * mCos(phi);
      x[1] = -t * mCos(phi + (M_PI / 3.f));
      x[2] = -t * mCos(phi - (M_PI / 3.f));
      num = 3;
   }
   else                    // 1 solution
   {
      F32 sqrtD = mSqrt(D);
      F32 u = mCbrt(sqrtD - q);
      F32 v = -mCbrt(sqrtD + q);

      x[0] = u + v;
      num = 1;
   }

   // resubstitute
   F32 sub = (1.f/3.f) * A;
   for(U32 i = 0; i < num; i++)
      x[i] -= sub;

   // sort the roots
   for(S32 j = 0; j < (num - 1); j++)
      for(S32 k = j + 1; k < num; k++)
         if(x[k] < x[j])
            swap(x[k], x[j]);

   return(num);
}

//--------------------------------------------------------------------------
// from Graphics Gems I: pp 738-742
U32 mSolveQuartic_c(F32 a, F32 b, F32 c, F32 d, F32 e, F32 * x)
{
   if(mIsZero(a))
      return(mSolveCubic(b, c, d, e, x));

   // normal form: x^4 + ax^3 + bx^2 + cx + d = 0
   F32 A = b / a;
   F32 B = c / a;
   F32 C = d / a;
   F32 D = e / a;

   // substitue x = y - A/4 to eliminate cubic term:
   // x^4 + px^2 + qx + r = 0
   F32 A2 = A * A;
   F32 A3 = A2 * A;
   F32 A4 = A2 * A2;

   F32 p = ((-3.f/8.f) * A2) + B;
   F32 q = ((1.f/8.f) * A3) - ((1.f/2.f) * A * B) + C;
   F32 r = ((-3.f/256.f) * A4) + ((1.f/16.f) * A2 * B) - ((1.f/4.f) * A * C) + D;

   U32 num = 0;
   if(mIsZero(r)) // no absolute term: y(y^3 + py + q) = 0
   {
      num = mSolveCubic(1.f, 0.f, p, q, x);
      x[num++] = 0.f;
   }
   else
   {
      // solve the resolvent cubic
      F32 q2 = q * q;

      a = 1.f;
      b = (-1.f/2.f) * p;
      c = -r;
      d = ((1.f/2.f) * r * p) - ((1.f/8.f) * q2);

      mSolveCubic(a, b, c, d, x);

      F32 z = x[0];

      // build 2 quadratic equations from the one solution
      F32 u = (z * z) - r;
      F32 v = (2.f * z) - p;

      if(mIsZero(u))
         u = 0.f;
      else if(u > 0.f)
         u = mSqrt(u);
      else
         return(0);

      if(mIsZero(v))
         v = 0.f;
      else if(v > 0.f)
         v = mSqrt(v);
      else
         return(0);

      // solve the two quadratics
      a = 1.f;
      b = v;
      c = z - u;
      num = mSolveQuadratic(a, b, c, x);

      a = 1.f;
      b = -v;
      c = z + u;
      num += mSolveQuadratic(a, b, c, x + num);
   }

   // resubstitute
   F32 sub = (1.f/4.f) * A;
   for(U32 i = 0; i < num; i++)
      x[i] -= sub;

   // sort the roots
   for(S32 j = 0; j < (num - 1); j++)
      for(S32 k = j + 1; k < num; k++)
         if(x[k] < x[j])
            swap(x[k], x[j]);

   return(num);
}

U32 (*mSolveQuadratic)( F32 a, F32 b, F32 c, F32* x ) = mSolveQuadratic_c;
U32 (*mSolveCubic)( F32 a, F32 b, F32 c, F32 d, F32* x ) = mSolveCubic_c;
U32 (*mSolveQuartic)( F32 a, F32 b, F32 c, F32 d, F32 e, F32* x ) = mSolveQuartic_c;
