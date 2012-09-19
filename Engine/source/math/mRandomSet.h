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

#ifndef _MRANDOMSET_H_
#define _MRANDOMSET_H_

#ifndef _MRANDOM_H_
#include "math/mRandom.h"
#endif

template <class T>
class MRandomSet
{
protected:

   MRandomLCG *mRandGen;

   Vector<T> mItems;
   Vector<F32> mProbability;
   F32 mSum;

public:

   MRandomSet( MRandomLCG *randGen = &gRandGen );

   void add( const T &item, F32 probability );
   
   /// Return a random item from the set using the specified per
   /// item probability distribution.
   T get();
};

template<class T>
inline MRandomSet<T>::MRandomSet( MRandomLCG *randGen )
 : mRandGen( randGen ),
   mSum( 0.0f )
{
}

template<class T> 
inline void MRandomSet<T>::add( const T &item, F32 probability )
{
   AssertFatal( probability > 0.0f, "MRandomDeck - item probability must be positive." );

   mItems.push_back( item );
   mProbability.push_back( probability );
   mSum += probability;
}

template<class T> 
inline T MRandomSet<T>::get()
{ 
   AssertFatal( mSum > 0.0f, "MRandomDeck - no items to get." );

   F32 rand = mRandGen->randF(0.0f, mSum);

   F32 prev = -1.0f;
   F32 curr = 0.0f;

   for ( S32 i = 0; i < mItems.size(); i++ )
   {
      curr += mProbability[i];

      if ( rand > prev && rand <= curr )
         return mItems[i];
         
      prev = curr;
   }
   
   AssertFatal( false, "MRandomSet::get() has failed." );
   return NULL;
}

#endif //_MRANDOMSET_H_
