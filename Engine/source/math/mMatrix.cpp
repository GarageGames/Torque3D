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

#include "core/strings/stringFunctions.h"
#include "core/frameAllocator.h"

#include "math/mMatrix.h"
#include "console/console.h"


const MatrixF MatrixF::Identity( true );

// idx(i,j) is index to element in column i, row j

void MatrixF::transposeTo(F32 *matrix) const
{
   matrix[idx(0,0)] = m[idx(0,0)];
   matrix[idx(0,1)] = m[idx(1,0)];
   matrix[idx(0,2)] = m[idx(2,0)];
   matrix[idx(0,3)] = m[idx(3,0)];
   matrix[idx(1,0)] = m[idx(0,1)];
   matrix[idx(1,1)] = m[idx(1,1)];
   matrix[idx(1,2)] = m[idx(2,1)];
   matrix[idx(1,3)] = m[idx(3,1)];
   matrix[idx(2,0)] = m[idx(0,2)];
   matrix[idx(2,1)] = m[idx(1,2)];
   matrix[idx(2,2)] = m[idx(2,2)];
   matrix[idx(2,3)] = m[idx(3,2)];
   matrix[idx(3,0)] = m[idx(0,3)];
   matrix[idx(3,1)] = m[idx(1,3)];
   matrix[idx(3,2)] = m[idx(2,3)];
   matrix[idx(3,3)] = m[idx(3,3)];
}

bool MatrixF::isAffine() const
{
   // An affine transform is defined by the following structure
   //
   // [ X X X P ]
   // [ X X X P ]
   // [ X X X P ]
   // [ 0 0 0 1 ]
   //
   //  Where X is an orthonormal 3x3 submatrix and P is an arbitrary translation
   //  We'll check in the following order:
   //   1: [3][3] must be 1
   //   2: Shear portion must be zero
   //   3: Dot products of rows and columns must be zero
   //   4: Length of rows and columns must be 1
   //
   if (m[idx(3,3)] != 1.0f)
      return false;

   if (m[idx(0,3)] != 0.0f ||
       m[idx(1,3)] != 0.0f ||
       m[idx(2,3)] != 0.0f)
      return false;

   Point3F one, two, three;
   getColumn(0, &one);
   getColumn(1, &two);
   getColumn(2, &three);
   if (mDot(one, two)   > 0.0001f ||
       mDot(one, three) > 0.0001f ||
       mDot(two, three) > 0.0001f)
      return false;

   if (mFabs(1.0f - one.lenSquared()) > 0.0001f ||
       mFabs(1.0f - two.lenSquared()) > 0.0001f ||
       mFabs(1.0f - three.lenSquared()) > 0.0001f)
      return false;

   getRow(0, &one);
   getRow(1, &two);
   getRow(2, &three);
   if (mDot(one, two)   > 0.0001f ||
       mDot(one, three) > 0.0001f ||
       mDot(two, three) > 0.0001f)
      return false;

   if (mFabs(1.0f - one.lenSquared()) > 0.0001f ||
       mFabs(1.0f - two.lenSquared()) > 0.0001f ||
       mFabs(1.0f - three.lenSquared()) > 0.0001f)
      return false;

   // We're ok.
   return true;
}

// Perform inverse on full 4x4 matrix.  Used in special cases only, so not at all optimized.
bool MatrixF::fullInverse()
{
   Point4F a,b,c,d;
   getRow(0,&a);
   getRow(1,&b);
   getRow(2,&c);
   getRow(3,&d);

   // det = a0*b1*c2*d3 - a0*b1*c3*d2 - a0*c1*b2*d3 + a0*c1*b3*d2 + a0*d1*b2*c3 - a0*d1*b3*c2 -
   //       b0*a1*c2*d3 + b0*a1*c3*d2 + b0*c1*a2*d3 - b0*c1*a3*d2 - b0*d1*a2*c3 + b0*d1*a3*c2 +
   //       c0*a1*b2*d3 - c0*a1*b3*d2 - c0*b1*a2*d3 + c0*b1*a3*d2 + c0*d1*a2*b3 - c0*d1*a3*b2 -
   //       d0*a1*b2*c3 + d0*a1*b3*c2 + d0*b1*a2*c3 - d0*b1*a3*c2 - d0*c1*a2*b3 + d0*c1*a3*b2
   F32 det = a.x*b.y*c.z*d.w - a.x*b.y*c.w*d.z - a.x*c.y*b.z*d.w + a.x*c.y*b.w*d.z + a.x*d.y*b.z*c.w - a.x*d.y*b.w*c.z
           - b.x*a.y*c.z*d.w + b.x*a.y*c.w*d.z + b.x*c.y*a.z*d.w - b.x*c.y*a.w*d.z - b.x*d.y*a.z*c.w + b.x*d.y*a.w*c.z
           + c.x*a.y*b.z*d.w - c.x*a.y*b.w*d.z - c.x*b.y*a.z*d.w + c.x*b.y*a.w*d.z + c.x*d.y*a.z*b.w - c.x*d.y*a.w*b.z
           - d.x*a.y*b.z*c.w + d.x*a.y*b.w*c.z + d.x*b.y*a.z*c.w - d.x*b.y*a.w*c.z - d.x*c.y*a.z*b.w + d.x*c.y*a.w*b.z;

   if (mFabs(det)<0.00001f)
      return false;

   Point4F aa,bb,cc,dd;
   aa.x =  b.y*c.z*d.w - b.y*c.w*d.z - c.y*b.z*d.w + c.y*b.w*d.z + d.y*b.z*c.w - d.y*b.w*c.z;
   aa.y = -a.y*c.z*d.w + a.y*c.w*d.z + c.y*a.z*d.w - c.y*a.w*d.z - d.y*a.z*c.w + d.y*a.w*c.z;
   aa.z =  a.y*b.z*d.w - a.y*b.w*d.z - b.y*a.z*d.w + b.y*a.w*d.z + d.y*a.z*b.w - d.y*a.w*b.z;
   aa.w = -a.y*b.z*c.w + a.y*b.w*c.z + b.y*a.z*c.w - b.y*a.w*c.z - c.y*a.z*b.w + c.y*a.w*b.z;

   bb.x = -b.x*c.z*d.w + b.x*c.w*d.z + c.x*b.z*d.w - c.x*b.w*d.z - d.x*b.z*c.w + d.x*b.w*c.z;
   bb.y =  a.x*c.z*d.w - a.x*c.w*d.z - c.x*a.z*d.w + c.x*a.w*d.z + d.x*a.z*c.w - d.x*a.w*c.z;
   bb.z = -a.x*b.z*d.w + a.x*b.w*d.z + b.x*a.z*d.w - b.x*a.w*d.z - d.x*a.z*b.w + d.x*a.w*b.z;
   bb.w =  a.x*b.z*c.w - a.x*b.w*c.z - b.x*a.z*c.w + b.x*a.w*c.z + c.x*a.z*b.w - c.x*a.w*b.z;

   cc.x =  b.x*c.y*d.w - b.x*c.w*d.y - c.x*b.y*d.w + c.x*b.w*d.y + d.x*b.y*c.w - d.x*b.w*c.y;
   cc.y = -a.x*c.y*d.w + a.x*c.w*d.y + c.x*a.y*d.w - c.x*a.w*d.y - d.x*a.y*c.w + d.x*a.w*c.y;
   cc.z =  a.x*b.y*d.w - a.x*b.w*d.y - b.x*a.y*d.w + b.x*a.w*d.y + d.x*a.y*b.w - d.x*a.w*b.y;
   cc.w = -a.x*b.y*c.w + a.x*b.w*c.y + b.x*a.y*c.w - b.x*a.w*c.y - c.x*a.y*b.w + c.x*a.w*b.y;

   dd.x = -b.x*c.y*d.z + b.x*c.z*d.y + c.x*b.y*d.z - c.x*b.z*d.y - d.x*b.y*c.z + d.x*b.z*c.y;
   dd.y =  a.x*c.y*d.z - a.x*c.z*d.y - c.x*a.y*d.z + c.x*a.z*d.y + d.x*a.y*c.z - d.x*a.z*c.y;
   dd.z = -a.x*b.y*d.z + a.x*b.z*d.y + b.x*a.y*d.z - b.x*a.z*d.y - d.x*a.y*b.z + d.x*a.z*b.y;
   dd.w =  a.x*b.y*c.z - a.x*b.z*c.y - b.x*a.y*c.z + b.x*a.z*c.y + c.x*a.y*b.z - c.x*a.z*b.y;

   setRow(0,aa);
   setRow(1,bb);
   setRow(2,cc);
   setRow(3,dd);

   mul(1.0f/det);

   return true;
}

EulerF MatrixF::toEuler() const
{
   const F32 * mat = m;

   EulerF r;
   r.x = mAsin(mat[MatrixF::idx(2,1)]);

   if(mCos(r.x) != 0.f)
   {
      r.y = mAtan2(-mat[MatrixF::idx(2,0)], mat[MatrixF::idx(2,2)]);
      r.z = mAtan2(-mat[MatrixF::idx(0,1)], mat[MatrixF::idx(1,1)]);
   }
   else
   {
      r.y = 0.f;
      r.z = mAtan2(mat[MatrixF::idx(1,0)], mat[MatrixF::idx(0,0)]);
   }

   return r;
}

void MatrixF::dumpMatrix(const char *caption /* =NULL */) const
{
   U32 size = dStrlen(caption);
   FrameTemp<char> spacer(size+1);
   char *spacerRef = spacer;

   dMemset(spacerRef, ' ', size);
   spacerRef[size] = 0;

   Con::printf("%s = | %-8.4f %-8.4f %-8.4f %-8.4f |", caption,    m[idx(0,0)], m[idx(0, 1)], m[idx(0, 2)], m[idx(0, 3)]);
   Con::printf("%s   | %-8.4f %-8.4f %-8.4f %-8.4f |", spacerRef,  m[idx(1,0)], m[idx(1, 1)], m[idx(1, 2)], m[idx(1, 3)]);
   Con::printf("%s   | %-8.4f %-8.4f %-8.4f %-8.4f |", spacerRef,  m[idx(2,0)], m[idx(2, 1)], m[idx(2, 2)], m[idx(2, 3)]);
   Con::printf("%s   | %-8.4f %-8.4f %-8.4f %-8.4f |", spacerRef,  m[idx(3,0)], m[idx(3, 1)], m[idx(3, 2)], m[idx(3, 3)]);
}