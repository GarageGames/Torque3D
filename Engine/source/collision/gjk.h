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

#ifndef _GJK_H_
#define _GJK_H_

#ifndef _MMATH_H_
#include "math/mMath.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _CONVEX_H_
#include "collision/convex.h"
#endif

class Convex;
struct CollisionStateList;
struct Collision;

//----------------------------------------------------------------------------

struct GjkCollisionState: public CollisionState
{
   /// @name Temporary values
   /// @{
   Point3F p[4];     ///< support points of object A in local coordinates
   Point3F q[4];     ///< support points of object B in local coordinates
   VectorF y[4];     ///< support points of A - B in world coordinates

   S32 bits;         ///< identifies current simplex
   S32 all_bits;     ///< all_bits = bits | last_bit
   F32 det[16][4];   ///< cached sub-determinants
   F32 dp[4][4];     ///< cached dot products

   S32 last;         ///< identifies last found support point
   S32 last_bit;     ///< last_bit = 1<<last
   /// @}

   ///
   void compute_det();
   bool valid(int s);
   void compute_vector(int bits, VectorF& v);
   bool closest(VectorF& v);
   bool degenerate(const VectorF& w);
   void nextBit();
   void swap();
   void reset(const MatrixF& a2w, const MatrixF& b2w);

   GjkCollisionState();
   ~GjkCollisionState();

   void set(Convex* a,Convex* b,const MatrixF& a2w, const MatrixF& b2w);

   void getCollisionInfo(const MatrixF& mat, Collision* info);
   void getClosestPoints(Point3F& p1, Point3F& p2);
   bool intersect(const MatrixF& a2w, const MatrixF& b2w);
   F32 distance(const MatrixF& a2w, const MatrixF& b2w, const F32 dontCareDist,
                       const MatrixF* w2a = NULL, const MatrixF* _w2b = NULL);
};


#endif
