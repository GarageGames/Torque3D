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
#include "util/catmullRom.h"

#include "math/mMathFn.h"


const F32 CatmullRomBase::smX[] =
{
   0.0000000000f, 0.5384693101f, -0.5384693101f, 0.9061798459f, -0.9061798459f 
};

const F32 CatmullRomBase::smC[] =
{
   0.5688888889f, 0.4786286705f, 0.4786286705f, 0.2369268850f, 0.2369268850f
};


CatmullRomBase::CatmullRomBase() 
: mTimes( NULL ),
  mLengths( NULL ),
  mTotalLength( 0.0f ),
  mCount( 0 )
{
} 

void CatmullRomBase::_initialize( U32 count, const F32 *times )
{
   //AssertFatal( times, "CatmullRomBase::_initialize() - Got null position!" )
   AssertFatal( count > 1, "CatmullRomBase::_initialize() - Must have more than 2 points!" )

   // set up arrays
   mTimes = new F32[count];
   mCount = count;

   // set up curve segment lengths
   mLengths = new F32[count-1];
   mTotalLength = 0.0f;
   for ( U32 i = 0; i < count-1; ++i )
   {
      mLengths[i] = segmentArcLength(i, 0.0f, 1.0f);
      mTotalLength += mLengths[i];
   }

   // copy the times if we have them.
   F32 l = 0.0f;
   for ( U32 i = 0; i < count; ++i )
   {
      if ( times )
         mTimes[i] = times[i];
      else
      {
         if ( mIsZero( mTotalLength ) )
            mTimes[i] = 0.0f;
         else
            mTimes[i] = l / mTotalLength;
         if ( i < count-1 )
            l += mLengths[i];
      }
   }
}

void CatmullRomBase::clear()
{
   delete [] mTimes;
   mTimes = NULL;

   delete [] mLengths;
   mLengths = NULL;

   mTotalLength = 0.0f;
   mCount = 0;
}

F32 CatmullRomBase::arcLength( F32 t1, F32 t2 )
{
   if ( t2 <= t1 )
      return 0.0f;

   if ( t1 < mTimes[0] )
      t1 = mTimes[0];

   if ( t2 > mTimes[mCount-1] )
      t2 = mTimes[mCount-1];

   // find segment and parameter
   U32 seg1;
   for ( seg1 = 0; seg1 < mCount-1; ++seg1 )
   {
      if ( t1 <= mTimes[seg1+1] )
      {
         break;
      }
   }
   F32 u1 = (t1 - mTimes[seg1])/(mTimes[seg1+1] - mTimes[seg1]);

   // find segment and parameter
   U32 seg2;
   for ( seg2 = 0; seg2 < mCount-1; ++seg2 )
   {
      if ( t2 <= mTimes[seg2+1] )
      {
         break;
      }
   }
   F32 u2 = (t2 - mTimes[seg2])/(mTimes[seg2+1] - mTimes[seg2]);

   F32 result;
   // both parameters lie in one segment
   if ( seg1 == seg2 )
   {
      result = segmentArcLength( seg1, u1, u2 );
   }
   // parameters cross segments
   else
   {
      result = segmentArcLength( seg1, u1, 1.0f );
      for ( U32 i = seg1+1; i < seg2; ++i )
         result += mLengths[i];
      result += segmentArcLength( seg2, 0.0f, u2 );
   }

   return result;
}

U32 CatmullRomBase::getPrevNode( F32 t )
{
   AssertFatal( mCount >= 2, "CatmullRomBase::getPrevNode - Bad point count!" );

   // handle boundary conditions
   if ( t <= mTimes[0] )
      return 0;
   else if ( t >= mTimes[mCount-1] )
      return mCount-1;

   // find segment and parameter
   U32 i;  // segment #
   for ( i = 0; i < mCount-1; ++i )
   {
      if ( t <= mTimes[i+1] )
         break;
   }

   AssertFatal( i >= 0 && i < mCount, "CatmullRomBase::getPrevNode - Got bad output index!" );

   return i;   
}

F32 CatmullRomBase::getTime( U32 idx )
{
   AssertFatal( idx >= 0 && idx < mCount, "CatmullRomBase::getTime - Got bad index!" );
   return mTimes[idx];
}