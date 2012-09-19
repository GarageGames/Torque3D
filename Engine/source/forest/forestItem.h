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

#ifndef _FORESTITEM_H_
#define _FORESTITEM_H_

#ifndef _MMATH_H_
#include "math/mMath.h"
#endif
#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _DYNAMIC_CONSOLETYPES_H_
#include "console/dynamicTypes.h"
#endif


class ForestItem;
class ForestCellBatch;
class ForestConvex;
class ForestWindAccumulator;

class Box3F;
class Point3F;
class TSRenderState;
class SceneRenderState;
struct RayInfo;
class AbstractPolyList;


class ForestItemData : public SimDataBlock
{
protected:

   typedef SimDataBlock Parent;

   static SimSet* smSet;

   bool mNeedPreload;

   virtual void _preload() {}

public:
   
   /// Shape file for this item type.
   StringTableEntry mShapeFile;   

   /// This is the radius used during placement to ensure
   /// the element isn't crowded up against other trees.
   F32 mRadius;   

   /// Overall scale to the effect of wind.
   F32 mWindScale;

   /// This is used to control the overall bend amount of the tree by wind and impacts.
   F32 mTrunkBendScale;

   /// Amplitude of the effect on larger branches.
   F32 mWindBranchAmp;

   /// Amplitude of the winds effect on leafs/fronds.
   F32 mWindDetailAmp;

   /// Frequency (speed) of the effect on leafs/fronds.
   F32 mWindDetailFreq;

   /// Mass used in calculating spring forces.
   F32 mMass;

   // The rigidity of the tree's trunk.
   F32 mRigidity;

   // The tightness coefficient of the spring for this tree type's ForestWindAccumulator.
   F32 mTightnessCoefficient;

   // The damping coefficient.
   F32 mDampingCoefficient;

   /// Can other objects or spacial queries hit items of this type.
   bool mCollidable;


   static SimSet* getSet();
   static ForestItemData* find( const char *name );

   ForestItemData();
   virtual ~ForestItemData() {}

   DECLARE_CONOBJECT( ForestItemData );
   
   static void  consoleInit();
   static void  initPersistFields();   

   virtual void onNameChange(const char *name);
   virtual bool onAdd();
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);

   /// Called from Forest the first time a datablock is used
   /// in order to lazy load content.
   void preload() 
   { 
      if ( !mNeedPreload ) 
         return;

      _preload();
      mNeedPreload = false;
   }

   virtual const Box3F& getObjBox() const { return Box3F::Invalid; }

   virtual bool render( TSRenderState *rdata, const ForestItem &item ) const { return false; }

   virtual bool canBillboard( const SceneRenderState *state, const ForestItem &item, F32 distToCamera ) const { return false; }

   virtual ForestCellBatch* allocateBatch() const { return NULL; }

   typedef Signal<void(void)> ReloadSignal;

   static ReloadSignal& getReloadSignal() 
   { 
      static ReloadSignal theSignal;
      return theSignal;
   }
};

typedef Vector<ForestItemData*> ForestItemDataVector;




typedef U32 ForestItemKey;


class ForestItem
{
protected:

   ForestItemData *mDataBlock;
   
   // The unique identifier used when querying forest items.
   ForestItemKey mKey;

   MatrixF mTransform;

   F32 mScale;

   F32 mRadius;

   Box3F mWorldBox;

   // JCFHACK: change this to an abstract physics-rep.
   //NxActor *mActor;

   /// If we're currently being effected by one or
   /// more wind emitters then we hold the results
   /// in this class.
   //ForestWindAccumulator *mWind;

public:

   // Constructs an invalid item.
   ForestItem();
  
   // Note: We keep this non-virtual to save vtable space.
   ~ForestItem();

   static const ForestItem Invalid;

   // Comparison operators with other ForestItems.
   // Note that this compares only the ForestItemKey, we are not validating
   // that any other data like the position actually matches.
   bool operator==(const ForestItem&) const;
   bool operator!=(const ForestItem&) const;

   /// Returns true if this is a valid item.
   bool isValid() const { return mKey != 0; }

   /// Invalidates the item.
   void makeInvalid() { mKey = 0; }

   const ForestItemKey& getKey() const { return mKey; };

   void setKey( const ForestItemKey &key ) { mKey = key; }

   Point3F getPosition() const { return mTransform.getPosition(); }

   const MatrixF& getTransform() const { return mTransform; }

   F32 getScale() const { return mScale; }

   F32 getRadius() const { return mRadius; }

   Point3F getCenterPoint() const { return mWorldBox.getCenter(); }
  
   void setTransform( const MatrixF &xfm, F32 scale );

   F32 getSqDistanceToPoint( const Point2F &point ) const;

   void setData( ForestItemData *data );

   const Box3F& getObjBox() const { return mDataBlock->getObjBox(); }

   const Box3F& getWorldBox() const { return mWorldBox; }

   Point3F getSize() const 
   { 
      if ( !mDataBlock )
         return Point3F::One;

      Box3F size = mDataBlock->getObjBox();
      size.scale( mScale );
      return size.getExtents();
   }

   ForestItemData* getData() const { return mDataBlock; };

   inline bool canBillboard( const SceneRenderState *state, F32 distToCamera ) const
   {
      return mDataBlock && mDataBlock->canBillboard( state, *this, distToCamera );
   }

   /// Collision
   /// @{
   
      bool castRay( const Point3F &start, const Point3F &end, RayInfo *outInfo, bool rendered ) const;         

      bool buildPolyList( AbstractPolyList *polyList, const Box3F &box, const SphereF &sphere ) const;

      //ForestConvex* buildConvex( const Box3F &box, Convex *convex ) const;

   /// @}
   
};

typedef Vector<ForestItem> ForestItemVector;


inline F32 ForestItem::getSqDistanceToPoint( const Point2F &point ) const
{
   return ( getPosition().asPoint2F() - point ).lenSquared();
}

inline bool ForestItem::operator==(const ForestItem& _test) const
{
   return mKey == _test.mKey;
}

inline bool ForestItem::operator!=(const ForestItem& _test) const
{
   return mKey != _test.mKey;
}


#endif // _FORESTITEM_H_
