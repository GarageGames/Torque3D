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

// Notice:
// Some of this code originates from an article in AI Game Programming Wisdom 
// by Dave Mark.

#ifndef _TRESPONSECURVE_H_
#define _TRESPONSECURVE_H_

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

#ifndef _MMATHFN_H_
#include "math/mMathFn.h"
#endif

#ifndef _SIMOBJECT_H_
#include "console/simObject.h"
#endif


// -------------------------------
// Represents a sigmoid function
// Note: not used by ResponseCurve
// -------------------------------

class Sigmoid
{
public:
   Sigmoid( F32 s, F32 m) { _s = s; _m = m; }
   ~Sigmoid() {};
   
   inline F32 get( F32 x )
   {
      F32 pow = -2.0f * ( ( x - _m ) / _s );
      F32 y = 1.0f / mPow( 1 + M_CONST_E_F, pow );
      return y;
   }

   F32 _s;
   F32 _m;
};

// -----------------------------------------------------------------------------
// Represents a response curve, can query values
// -----------------------------------------------------------------------------

template< class T >
class ResponseCurve
{
public:

   struct Sample
   {
      Sample() {}
      Sample( F32 f, const T &val ) : mF(f), mVal(val) {}

      F32 mF;
      T mVal;
   };

   typedef Vector< Sample > SampleList;
   SampleList mSamples;

   const SampleList& getSamples() { return mSamples; }

   ResponseCurve() {}
   ResponseCurve( U32 numSamples ) { mSamples.reserve(numSamples); }

   void clear() { mSamples.clear(); }
   void addPoint( F32 f, const T &val );
   //void addPoints( U32 count, F32 f[], const T &val[] );
   T getVal( F32 f ) const;
   S32 setPoint( S32 idx, F32 f, const T &val ); 
   void removePoint( S32 idx );
   S32 getSampleCount() const { return mSamples.size(); }
};

//-----------------------------------------------------------------------------
// Adds a new value to the Response Curve, at the position f
//-----------------------------------------------------------------------------
template< class T > 
inline void ResponseCurve<T>::addPoint( F32 f, const T &val )
{
   typename SampleList::iterator iter = mSamples.begin();
   for ( ; iter != mSamples.end(); iter++ )
   {
      if ( iter->mF == f )
      {
         Con::warnf( "Warn: ResponseCurve<T>::AddPoint, Duplicate values are not allowed." );
         return;
      }

   if ( iter->mF > f )
      break;
   }

   mSamples.insert( iter, Sample( f, val ) );
}

//-----------------------------------------------------------------------------
// Finds the right value at position f, interpolating between the previous
// and the following values
//-----------------------------------------------------------------------------
template< class T > 
inline T ResponseCurve<T>::getVal( F32 f ) const
{
   T retVal;

   if ( mSamples.empty() )
   {
      retVal = T();
   }
   else
   {
      U32 nSamples = mSamples.size();
      if ( nSamples == 1 || f <= mSamples[0].mF )
      {
         retVal = mSamples[0].mVal;
      }
      else if ( f >= mSamples[nSamples-1].mF )
      {
         retVal = mSamples[nSamples-1].mVal;
      }
      else
      {
         U32 i = 1;
         while ( i < (nSamples-1) && mSamples[i].mF < f )
            ++i;

         // Interpolate between m_Samples[i-1] and m_Samples[i]
         F32 fSampleMin = mSamples[i-1].mF;
         F32 fSampleMax = mSamples[i].mF;
         AssertWarn(fSampleMin != fSampleMax, "fSampleMin should not equal fSampleMax" );

         F32 t = (f - fSampleMin) / (fSampleMax - fSampleMin);
         retVal = mSamples[i-1].mVal + ( mSamples[i].mVal - mSamples[i-1].mVal) * t;
      }
   }

   return retVal;
}

template< class T > 
inline S32 ResponseCurve< T >::setPoint( S32 idx, F32 f, const T &val )
{   
   mSamples.erase( idx );
   
   typename SampleList::iterator iter = mSamples.begin();
   for ( ; iter != mSamples.end(); iter++ )
   {
      if ( iter->mF == f )
      {
         Con::warnf( "Warn: ResponseCurve<T>::AddPoint, Duplicate values are not allowed." );
         return -1;
      }

   if ( iter->mF > f )
      break;
   }

   mSamples.insert( iter, Sample( f, val ) );

   return (S32)( iter - mSamples.begin() );
}


class FloatCurve : public ResponseCurve<F32>
{
public:
   FloatCurve() {}   
};


// -----------------------------------------------
// A ResponseCurve<F32> wrapped as a SimObject
// -----------------------------------------------

class SimResponseCurve : public SimObject
{
   typedef SimObject Parent;

public:

   SimResponseCurve();
   //~SimResponseCurve();

   DECLARE_CONOBJECT( SimResponseCurve );

   virtual bool onAdd();
   virtual void onRemove();

   void addPoint( F32 value, F32 time );
   F32 getValue( F32 time );
   void clear();

   ResponseCurve<F32> mCurve;
};


// A networked-datablock version of ResponseCurve
/*
class ResponseCurveData : public SimDataBlock
{
   typedef SimDataBlock Parent;

public:

   ResponseCurveData();
   //~ResponseCurveData();

   DECLARE_CONOBJECT( ResponseCurveData );

   virtual bool onAdd();
   virtual void onRemove();


   void addPoint( F32 value, F32 time );
   F32 getValue( F32 time );
   void clear();

   ResponseCurve<F32> mCurve;
};
*/

#endif
