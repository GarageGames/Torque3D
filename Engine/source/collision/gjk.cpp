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

// The core algorithms in this file are inspired by public papers written
// by G. van den Bergen for his interference detection library,
// "SOLID 2.0"

#include "core/dataChunker.h"
#include "collision/collision.h"
#include "scene/sceneObject.h"
#include "collision/convex.h"
#include "collision/gjk.h"


//----------------------------------------------------------------------------

static F32 rel_error = 1E-5f;     // relative error in the computed distance
static F32 sTolerance = 1E-3f;    // Distance tolerance
static F32 sEpsilon2 = 1E-20f;    // Zero length vector
static U32 sIteration = 15;       // Stuck in a loop?

S32 num_iterations = 0;
S32 num_irregularities = 0;


//----------------------------------------------------------------------------

GjkCollisionState::GjkCollisionState()
{
   mA = mB = 0;
}

GjkCollisionState::~GjkCollisionState()
{
}


//----------------------------------------------------------------------------

void GjkCollisionState::swap()
{
   Convex* t = mA; mA = mB; mB = t;
   CollisionStateList* l = mLista; mLista = mListb; mListb = l;
   mDistvec.neg();
}


//----------------------------------------------------------------------------

void GjkCollisionState::compute_det()
{
   // Dot new point with current set
   for (S32 i = 0, bit = 1; i < 4; ++i, bit <<=1)
      if (mBits & bit)
		  mDP[i][mLast] = mDP[mLast][i] = mDot(mY[i], mY[mLast]);
   mDP[mLast][mLast] = mDot(mY[mLast], mY[mLast]);

   // Calulate the determinent
   mDet[mLast_bit][mLast] = 1;
   for (S32 j = 0, sj = 1; j < 4; ++j, sj <<= 1) {
      if (mBits & sj) {
         S32 s2 = sj | mLast_bit;
		 mDet[s2][j] = mDP[mLast][mLast] - mDP[mLast][j];
		 mDet[s2][mLast] = mDP[j][j] - mDP[j][mLast];
         for (S32 k = 0, sk = 1; k < j; ++k, sk <<= 1) {
            if (mBits & sk) {
              S32 s3 = sk | s2;
			  mDet[s3][k] = mDet[s2][j] * (mDP[j][j] - mDP[j][k]) +
                            mDet[s2][mLast] * (mDP[mLast][j] - mDP[mLast][k]);
			  mDet[s3][j] = mDet[sk | mLast_bit][k] * (mDP[k][k] - mDP[k][j]) +
                            mDet[sk | mLast_bit][mLast] * (mDP[mLast][k] - mDP[mLast][j]);
			  mDet[s3][mLast] = mDet[sk | sj][k] * (mDP[k][k] - mDP[k][mLast]) +
                                mDet[sk | sj][j] * (mDP[j][k] - mDP[j][mLast]);
            }
         }
      }
   }

   if (mAll_bits == 15) {
     mDet[15][0] = mDet[14][1] * (mDP[1][1] - mDP[1][0]) +
                   mDet[14][2] * (mDP[2][1] - mDP[2][0]) +
                   mDet[14][3] * (mDP[3][1] - mDP[3][0]);
	 mDet[15][1] = mDet[13][0] * (mDP[0][0] - mDP[0][1]) +
                   mDet[13][2] * (mDP[2][0] - mDP[2][1]) +
                   mDet[13][3] * (mDP[3][0] - mDP[3][1]);
	 mDet[15][2] = mDet[11][0] * (mDP[0][0] - mDP[0][2]) +
                   mDet[11][1] * (mDP[1][0] - mDP[1][2]) +
                   mDet[11][3] * (mDP[3][0] - mDP[3][2]);
	 mDet[15][3] = mDet[7][0] * (mDP[0][0] - mDP[0][3]) +
                   mDet[7][1] * (mDP[1][0] - mDP[1][3]) +
                   mDet[7][2] * (mDP[2][0] - mDP[2][3]);
   }
}


//----------------------------------------------------------------------------

inline void GjkCollisionState::compute_vector(S32 bits, VectorF& v)
{
   F32 sum = 0;
   v.set(0, 0, 0);
   for (S32 i = 0, bit = 1; i < 4; ++i, bit <<= 1) {
      if (bits & bit) {
         sum += mDet[bits][i];
         v += mY[i] * mDet[bits][i];
      }
   }
   v *= 1 / sum;
}


//----------------------------------------------------------------------------

inline bool GjkCollisionState::valid(S32 s)
{
   for (S32 i = 0, bit = 1; i < 4; ++i, bit <<= 1) {
      if (mAll_bits & bit) {
         if (s & bit) {
            if (mDet[s][i] <= 0)
               return false;
         }
         else
            if (mDet[s | bit][i] > 0)
               return false;
      }
   }
   return true;
}


//----------------------------------------------------------------------------

inline bool GjkCollisionState::closest(VectorF& v)
{
   compute_det();
   for (S32 s = mBits; s; --s) {
      if ((s & mBits) == s) {
         if (valid(s | mLast_bit)) {
			 mBits = s | mLast_bit;
            if (mBits != 15)
    	         compute_vector(mBits, v);
	         return true;
         }
      }
   }
   if (valid(mLast_bit)) {
      mBits = mLast_bit;
      v = mY[mLast];
      return true;
   }
   return false;
}


//----------------------------------------------------------------------------

inline bool GjkCollisionState::degenerate(const VectorF& w)
{
  for (S32 i = 0, bit = 1; i < 4; ++i, bit <<= 1)
   if ((mAll_bits & bit) && mY[i] == w)
      return true;
  return false;
}


//----------------------------------------------------------------------------

inline void GjkCollisionState::nextBit()
{
   mLast = 0;
   mLast_bit = 1;
   while (mBits & mLast_bit) {
      ++mLast;
	  mLast_bit <<= 1;
   }
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

void GjkCollisionState::set(Convex* aa, Convex* bb,
   const MatrixF& a2w, const MatrixF& b2w)
{
   mA = aa;
   mB = bb;

   mBits = 0;
   mAll_bits = 0;
   reset(a2w,b2w);

   // link
   mLista = CollisionStateList::alloc();
   mLista->mState = this;
   mListb = CollisionStateList::alloc();
   mListb->mState = this;
}


//----------------------------------------------------------------------------

void GjkCollisionState::reset(const MatrixF& a2w, const MatrixF& b2w)
{
   VectorF zero(0,0,0),sa,sb;
   a2w.mulP(mA->support(zero),&sa);
   b2w.mulP(mB->support(zero),&sb);
   mDistvec = sa - sb;
   mDist = mDistvec.len();
}


//----------------------------------------------------------------------------

void GjkCollisionState::getCollisionInfo(const MatrixF& mat, Collision* info)
{
   AssertFatal(false, "GjkCollisionState::getCollisionInfo() - There remain scaling problems here.");
   // This assumes that the shapes do not intersect
   Point3F pa,pb;
   if (mBits) {
      getClosestPoints(pa,pb);
      mat.mulP(pa,&info->point);
	  mB->getTransform().mulP(pb,&pa);
      info->normal = info->point - pa;
   }
   else {
      mat.mulP(mP[mLast],&info->point);
      info->normal = mDistvec;
   }
   info->normal.normalize();
   info->object = mB->getObject();
}

void GjkCollisionState::getClosestPoints(Point3F& p1, Point3F& p2)
{
   F32 sum = 0;
   p1.set(0, 0, 0);
   p2.set(0, 0, 0);
   for (S32 i = 0, bit = 1; i < 4; ++i, bit <<= 1) {
      if (mBits & bit) {
         sum += mDet[mBits][i];
         p1 += mP[i] * mDet[mBits][i];
         p2 += mQ[i] * mDet[mBits][i];
      }
   }
   F32 s = 1 / sum;
   p1 *= s;
   p2 *= s;
}


//----------------------------------------------------------------------------

bool GjkCollisionState::intersect(const MatrixF& a2w, const MatrixF& b2w)
{
   num_iterations = 0;
   MatrixF w2a,w2b;

   w2a = a2w;
   w2b = b2w;
   w2a.inverse();
   w2b.inverse();
   reset(a2w,b2w);

   mBits = 0;
   mAll_bits = 0;

   do {
      nextBit();

      VectorF va,sa;
      w2a.mulV(-mDistvec,&va);
	  mP[mLast] = mA->support(va);
      a2w.mulP(mP[mLast],&sa);

      VectorF vb,sb;
      w2b.mulV(mDistvec,&vb);
	  mQ[mLast] = mB->support(vb);
      b2w.mulP(mQ[mLast],&sb);

      VectorF w = sa - sb;
      if (mDot(mDistvec,w) > 0)
         return false;
      if (degenerate(w)) {
         ++num_irregularities;
         return false;
      }

	  mY[mLast] = w;
	  mAll_bits = mBits | mLast_bit;

      ++num_iterations;
      if (!closest(mDistvec) || num_iterations > sIteration) {
         ++num_irregularities;
         return false;
      }
   }
   while (mBits < 15 && mDistvec.lenSquared() > sEpsilon2);
   return true;
}

F32 GjkCollisionState::distance(const MatrixF& a2w, const MatrixF& b2w,
   const F32 dontCareDist, const MatrixF* _w2a, const MatrixF* _w2b)
{
   num_iterations = 0;
   MatrixF w2a,w2b;

   if (_w2a == NULL || _w2b == NULL) {
      w2a = a2w;
      w2b = b2w;
      w2a.inverse();
      w2b.inverse();
   }
   else {
      w2a = *_w2a;
      w2b = *_w2b;
   }

   reset(a2w,b2w);
   mBits = 0;
   mAll_bits = 0;
   F32 mu = 0;

   do {
      nextBit();

      VectorF va,sa;
      w2a.mulV(-mDistvec,&va);
	  mP[mLast] = mA->support(va);
      a2w.mulP(mP[mLast],&sa);

      VectorF vb,sb;
      w2b.mulV(mDistvec,&vb);
	  mQ[mLast] = mB->support(vb);
      b2w.mulP(mQ[mLast],&sb);

      VectorF w = sa - sb;
      F32 nm = mDot(mDistvec, w) / mDist;
      if (nm > mu)
         mu = nm;
      if (mu > dontCareDist)
         return mu;
      if (mFabs(mDist - mu) <= mDist * rel_error)
         return mDist;

      ++num_iterations;
      if (degenerate(w) || num_iterations > sIteration) {
         ++num_irregularities;
         return mDist;
      }

	  mY[mLast] = w;
	  mAll_bits = mBits | mLast_bit;

      if (!closest(mDistvec)) {
         ++num_irregularities;
         return mDist;
      }

	  mDist = mDistvec.len();
   }
   while (mBits < 15 && mDist > sTolerance) ;

   if (mBits == 15 && mu <= 0)
	   mDist = 0;
   return mDist;
}
