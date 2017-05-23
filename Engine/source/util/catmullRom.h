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

#ifndef _CATMULLROM_H_
#define _CATMULLROM_H_


/// The shared base class used by the catmull rom
/// interpolation template class.
/// @see CatmullRom
class CatmullRomBase
{
protected:

   CatmullRomBase();
   virtual ~CatmullRomBase() {}

public:
   
   /// Clean out all the data.
   virtual void clear();

   /// Find length of curve between parameters t1 and t2.
   F32 arcLength( F32 t1, F32 t2 );

   /// Get the total length of the curve.
   inline F32 getLength() { return mTotalLength; }

   /// Get the closest previous control point to time t.
   U32 getPrevNode( F32 t );

   /// Returns the time at idx (rather than at a F32 time)
   F32 getTime( U32 idx );

   /// Find length of curve segment between parameters u1 and u2.
   virtual F32 segmentArcLength( U32 i, F32 u1, F32 u2 ) = 0;

protected:

   static const F32 smX[];
   static const F32 smC[];

   void _initialize( U32 count, const F32 *times = NULL );   

   /// The time to arrive at each point.
   F32 *mTimes;

   /// the length of each curve segment.
   F32* mLengths;

   /// The total length of curve.
   F32 mTotalLength;

   /// The number of points and times.
   U32 mCount;
};


/// The catmull-rom template class for performing interpolation
/// over an arbitraty type.
template<typename TYPE>
class CatmullRom : public CatmullRomBase
{
public:

   CatmullRom();
   virtual ~CatmullRom();   

   /// Initialization.
   void initialize( U32 count, const TYPE *positions, const F32 *times = NULL );

   // evaluate position
   TYPE evaluate( F32 t );

   /// Evaluate derivative at parameter t.
   TYPE velocity( F32 t );

   // Evaluate second derivative at parameter t.
   TYPE acceleration( F32 t );

   // Returns the position at idx (rather than at a F32 time)
   TYPE getPosition( U32 idx );

   // CatmullRomBase
   void clear();
   F32 segmentArcLength( U32 i, F32 u1, F32 u2 );

protected:

   /// The sample points.
   TYPE* mPositions; 

private:

   /// The copy constructors are disabled.
   CatmullRom( const CatmullRom &other );
   CatmullRom& operator=( const CatmullRom &other );
};


template<typename TYPE>
inline CatmullRom<TYPE>::CatmullRom()
   :  CatmullRomBase(),
      mPositions( NULL )
{   
}

template<typename TYPE>
inline CatmullRom<TYPE>::~CatmullRom()
{
   clear();
}

template<typename TYPE>
inline void CatmullRom<TYPE>::clear()
{
   delete [] mPositions;
   mPositions = NULL;

   CatmullRomBase::clear();
}

template<typename TYPE>
inline void CatmullRom<TYPE>::initialize( U32 count, const TYPE *positions, const F32 *times )
{
   AssertFatal( positions, "CatmullRom::initialize - Got null position!" );
   AssertFatal( count > 1, "CatmullRom::initialize - Must have more than 2 points!" );

   // Clean up any previous state.
   clear();

   // copy the points.
   mPositions = new TYPE[count];
   for ( U32 i = 0; i < count; ++i )
      mPositions[i] = positions[i];

   _initialize( count, times );
}

template<typename TYPE>
inline TYPE CatmullRom<TYPE>::evaluate( F32 t )
{
   AssertFatal( mCount >= 2, "CatmullRom::evaluate - Not initialized!" );

   // handle boundary conditions
   if ( t <= mTimes[0] )
      return mPositions[0];
   else if ( t >= mTimes[mCount-1] )
      return mPositions[mCount-1];

   // find segment and parameter
   U32 i;  // segment #
   for ( i = 0; i < mCount-1; ++i )
   {
      if ( t <= mTimes[i+1] )
      {
         break;
      }
   }

   AssertFatal( i >= 0 && i < mCount, "CatmullRom::evaluate - Got bad index!" );

   F32 t0 = mTimes[i];
   F32 t1 = mTimes[i+1];
   F32 u = (t - t0)/(t1 - t0);
      
   S32 idx0, idx1, idx2, idx3;
   idx0 = i - 1;
   idx1 = i;
   idx2 = i + 1;
   idx3 = i + 2;

   if ( idx0 < 0 )
      idx0 = 0;
   if ( idx3 >= mCount )
      idx3 = mCount - 1;
   
   TYPE A = 3.0f*mPositions[idx1]
            - mPositions[idx0]
            - 3.0f*mPositions[idx2]
            + mPositions[idx3];

   TYPE B = 2.0f*mPositions[idx0]
            - 5.0f*mPositions[idx1]
            + 4.0f*mPositions[idx2]
            - mPositions[idx3];

   TYPE C = mPositions[idx2] - mPositions[idx0];

   return mPositions[i] + (0.5f*u)*(C + u*(B + u*A));
}

template<typename TYPE>
inline TYPE CatmullRom<TYPE>::velocity( F32 t )
{
   AssertFatal( mCount >= 2, "CatmullRom::velocity - Not initialized!" );

   // handle boundary conditions
   if ( t <= mTimes[0] )
      t = 0.0f;
   else if ( t > mTimes[mCount-1] )
      t = mTimes[mCount-1];

   // find segment and parameter
   U32 i;
   for ( i = 0; i < mCount-1; ++i )
   {
      if ( t <= mTimes[i+1] )
      {
         break;
      }
   }
   F32 t0 = mTimes[i];
   F32 t1 = mTimes[i+1];
   F32 u = (t - t0)/(t1 - t0);

   S32 idx0, idx1, idx2, idx3;
   idx0 = i - 1;
   idx1 = i;
   idx2 = i + 1;
   idx3 = i + 2;

   if ( idx0 < 0 )
      idx0 = 0;
   if ( idx3 >= mCount )
      idx3 = mCount - 1;

   TYPE A = 3.0f*mPositions[idx1]
            - mPositions[idx0]
            - 3.0f*mPositions[idx2]
            + mPositions[idx3];

   TYPE B = 2.0f*mPositions[idx0]
            - 5.0f*mPositions[idx1]
            + 4.0f*mPositions[idx2]
            - mPositions[idx3];

   TYPE C = mPositions[idx2] - mPositions[idx0];

   return 0.5f*C + u*(B + 1.5f*u*A);
}

template<typename TYPE>
inline TYPE CatmullRom<TYPE>::acceleration( F32 t )
{
   AssertFatal( mCount >= 2, "CatmullRom::acceleration - Not initialized!" );

   // handle boundary conditions
   if ( t <= mTimes[0] )
      t = 0.0f;
   else if ( t > mTimes[mCount-1] )
      t = mTimes[mCount-1];

   // find segment and parameter
   U32 i;
   for ( i = 0; i < mCount-1; ++i )
   {
      if ( t <= mTimes[i+1] )
      {
         break;
      }
   }
   F32 t0 = mTimes[i];
   F32 t1 = mTimes[i+1];
   F32 u = (t - t0)/(t1 - t0);

   S32 idx0, idx1, idx2, idx3;
   idx0 = i - 1;
   idx1 = i;
   idx2 = i + 1;
   idx3 = i + 2;

   if ( idx0 < 0 )
      idx0 = 0;
   if ( idx3 >= mCount )
      idx3 = mCount - 1;

   TYPE A = 3.0f*mPositions[idx1]
            - mPositions[idx0]
            - 3.0f*mPositions[idx2]
            + mPositions[idx3];
   
   TYPE B = 2.0f*mPositions[idx0]
            - 5.0f*mPositions[idx1]
            + 4.0f*mPositions[idx2]
            - mPositions[idx3];

   TYPE C = mPositions[idx2] - mPositions[idx0];

   return B + (3.0f*u)*A;
}

template<typename TYPE>
inline TYPE CatmullRom<TYPE>::getPosition( U32 idx )
{
   AssertFatal( idx >= 0 && idx < mCount-1, "CatmullRom<>::getPosition - Got bad index!" );
   return mPositions[idx];
}

template<typename TYPE>
inline F32 CatmullRom<TYPE>::segmentArcLength( U32 i, F32 u1, F32 u2 )
{
   AssertFatal( i >= 0 && i < mCount-1, "CatmullRom<>::getPosition - Got bad index!" );

   if ( u2 <= u1 )
      return 0.0f;

   if ( u1 < 0.0f )
      u1 = 0.0f;

   if ( u2 > 1.0f )
      u2 = 1.0f;

   S32 idx0, idx1, idx2, idx3;
   idx0 = i - 1;
   idx1 = i;
   idx2 = i + 1;
   idx3 = i + 2;

   if ( idx0 < 0 )
      idx0 = 0;
   if ( idx3 >= mCount )
      idx3 = mCount - 1;

   TYPE A = 3.0f*mPositions[idx1]
   - mPositions[idx0]
   - 3.0f*mPositions[idx2]
   + mPositions[idx3];
   TYPE B = 2.0f*mPositions[idx0]
   - 5.0f*mPositions[idx1]
   + 4.0f*mPositions[idx2]
   - mPositions[idx3];
   TYPE C = mPositions[idx2] - mPositions[idx0];

   F32 sum = 0.0f;

   for ( U32 j = 0; j < 5; ++j )
   {
      F32 u = 0.5f*((u2 - u1)*smX[j] + u2 + u1);
      TYPE derivative;
      if ( i == 0 || i >= mCount-2)
         derivative = 0.5f*B + u*A;
      else
         derivative = 0.5f*C + u*(B + 1.5f*u*A);
      sum += smC[j]*derivative.len();
   }
   sum *= 0.5f*(u2-u1);

   return sum;
}

#endif // _CATMULLROM_H_
