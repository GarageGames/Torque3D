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

#ifndef _FORESTCOLLISION_H_
#define _FORESTCOLLISION_H_

#ifndef _CONVEX_H_
#include "collision/convex.h"
#endif
#ifndef _COLLISION_H_
#include "collision/collision.h"
#endif


class Forest;
class ForestItem;
class TSForestItemData;


class ForestConvex : public Convex
{
   typedef Convex Parent;
   friend class Forest;

public:

   ForestConvex() 
   { 
      mType = ForestConvexType; 
      mTransform.identity(); 
   }

   ForestConvex( const ForestConvex &cv ) 
   {
      mType          = ForestConvexType;
      mObject        = cv.mObject;
      mForestItemKey = cv.mForestItemKey;
      mTransform     = cv.mTransform;
      mData          = cv.mData;
      mScale         = cv.mScale;
      hullId         = cv.hullId;
      box            = box;
   }

   void           calculateTransform( const MatrixF &worldXfrm );
   const MatrixF& getTransform() const { return mTransform; }
   Box3F          getBoundingBox() const;
   Box3F          getBoundingBox( const MatrixF &mat, const Point3F &scale) const;
   Point3F        support( const VectorF &v ) const;
   void           getFeatures( const MatrixF &mat, const VectorF &n, ConvexFeature *cf );
   void           getPolyList( AbstractPolyList *list);

public:

   U32       hullId;
   Box3F     box;

protected:

   // JCF: ForestItemKey is a U32, didn't want to include forest.h here
   // fix me if ForestItemKey is changed to a class.
   U32 mForestItemKey;
   MatrixF mTransform;
   TSForestItemData *mData;
   F32 mScale;
};


#endif // _FORESTCOLLISION_H_