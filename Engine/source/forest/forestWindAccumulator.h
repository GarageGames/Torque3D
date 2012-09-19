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

#ifndef _FORESTWINDACCUMULATOR_H_
#define _FORESTWINDACCUMULATOR_H_

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif


struct TreePlacementInfo;
class ForestItemData;


/// This simple class holds the state of the accumulated 
/// wind effect for a single tree.
class ForestWindAccumulator
{
protected:

   struct VerletParticle
   {
      Point2F position;
      Point2F lastPosition;
   };

   F32 mCurrentStrength;
   Point2F mCurrentDir;

   Point3F mPosition;
   F32 mScale;
   ForestItemData *mDataBlock;

   VerletParticle mParticles[2];

   void _updateParticle( VerletParticle *particle, const Point2F &force, F32 timeDelta );

public:

   ForestWindAccumulator( const TreePlacementInfo &info );
   ~ForestWindAccumulator();

   void presimulate( const VectorF &windVector, U32 ticks );

   void updateWind( const VectorF &windVector, F32 timeDelta );

   void setDirection( const VectorF &dir ) { mCurrentDir.set( dir.x, dir.y ); }
   VectorF getDirection() const { return VectorF( mCurrentDir.x, mCurrentDir.y, 0 ); }

   void setStrength( F32 strength ) { mCurrentStrength = strength; }
   F32 getStrength() const { return mCurrentStrength; }

   void setPosition( const Point3F &pos ) { mPosition = pos; }
   Point3F getPosition() const { return mPosition; }

   void applyImpulse( const VectorF &impulse );
};

#endif // _FORESTWINDACCUMULATOR_H_