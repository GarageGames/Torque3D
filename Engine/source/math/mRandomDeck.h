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

#ifndef _MRANDOMDECK_H_
#define _MRANDOMDECK_H_

#ifndef _MRANDOM_H_
#include "math/mRandom.h"
#endif

template <class T>
class MRandomDeck
{
protected:

   MRandomLCG *mRandGen;

   Vector<T> mDeck;

   Vector<T> mPile;

public:

   MRandomDeck( MRandomLCG *randGen = &gRandGen );

   void addToPile( const T &item );

   void addToPile( const Vector<T> &items );

   void shuffle();

   S32 draw( T *item, bool reshuffle = true );

   void removeAll( Vector<T> *outItems );

};

template<class T>
inline MRandomDeck<T>::MRandomDeck( MRandomLCG *randGen )
   : mRandGen( randGen )
{
}

template<class T> 
inline void MRandomDeck<T>::shuffle()
{
   // Move everything to the pile.
   mPile.merge( mDeck );

   if ( mPile.empty() )
      return;
   T& last = mPile.last();
   mDeck.clear();

   // Randomly draw from the pile
   // and place them in the deck.
   while ( !mPile.empty() )
   {
      U32 i = mRandGen->randI( 0, mPile.size() - 1 );
      mDeck.push_back( mPile[i] );
      mPile.erase_fast( i );
   }

   // Make sure that the first drawn item
   // is not the same as the last drawn item.
   if ( mDeck.last() == last )
   {
      mDeck.pop_back();
      mDeck.push_front( last );
   }
}

template<class T> 
inline S32 MRandomDeck<T>::draw( T *item, bool reshuffle )
{ 
   if ( mDeck.size() == 0 )
   {
      if ( mPile.size() == 0 )
         return -1;
      
      if ( reshuffle )
         shuffle();
      else
         return -1;
   }

   *item = mDeck.last();
   mPile.push_back( *item );
   mDeck.pop_back();   

   return mDeck.size();
}

template<class T>
inline void MRandomDeck<T>::addToPile( const T &item )
{
   mPile.push_back( item );
}

template<class T>
inline void MRandomDeck<T>::addToPile( const Vector<T> &items )
{
   mPile.merge( items );
}

template<class T>
inline void MRandomDeck<T>::removeAll( Vector<T> *outItems )
{
   if ( outItems )
   {
      outItems->merge( mPile );
      outItems->merge( mDeck );
   }

   mPile.clear();
   mDeck.clear();
}

#endif //_MRANDOMDECK_H_
