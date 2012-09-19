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

#ifndef _FOREST_EDITOR_BRUSHELEMENT_H_
#define _FOREST_EDITOR_BRUSHELEMENT_H_

//#ifndef _SIMOBJECT_H_
//#include "console/simObject.h"
//#endif
#ifndef _SIMSET_H_
#include "console/simSet.h"
#endif


//-------------------------------------------------------------------------
// ForestBrushElement
//-------------------------------------------------------------------------

class ForestItemData;

class ForestBrushElement : public SimObject
{
   typedef SimObject Parent;

public:

   ForestBrushElement();

   DECLARE_CONOBJECT( ForestBrushElement );
   
   static void initPersistFields();

public:

   /// The type of ForestItem this element holds placement parameters for.
   ForestItemData *mData;

   /// The probability that this element will be 
   /// created during an editor brush stroke.
   F32 mProbability;

   /// The max rotation in degrees that items will be placed.
   F32  mRotationRange;
   
   /// The minimum random size for each item.
   F32 mScaleMin;

   /// The maximum random size of each item.
   F32 mScaleMax;

   /// An exponent used to bias between the minimum
   /// and maximum random sizes.
   F32 mScaleExponent;

   /// Min variation in the sink radius.
   F32 mSinkMin;

   /// Max variation in the sink radius.
   F32 mSinkMax;

   /// This is the radius used to calculate how much to
   /// sink the trunk at its base and is used to sink 
   /// the tree into the ground when its on a slope.
   F32 mSinkRadius;

   /// The min surface slope in degrees this item will be placed on.
   F32 mSlopeMin;

   /// The max surface slope in degrees this item will be placed on.
   F32 mSlopeMax;   

   /// The min world space elevation this item will be placed.
   F32 mElevationMin;

   /// The max world space elevation this item will be placed.
   F32 mElevationMax;
};

//-------------------------------------------------------------------------
// ForestBrush
//-------------------------------------------------------------------------

class ForestBrush : public SimGroup
{
   typedef SimGroup Parent;

public:

   ForestBrush();

   DECLARE_CONOBJECT( ForestBrush );

   virtual bool onAdd();

   virtual void addObject(SimObject*);

   static SimGroup* getGroup();

   bool containsItemData( const ForestItemData *inData );	
protected:

   static SimObjectPtr<SimGroup> smGroup;
};


#endif // _FOREST_EDITOR_BRUSHELEMENT_H_