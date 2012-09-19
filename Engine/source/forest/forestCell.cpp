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
#include "forest/forestCell.h"

#include "forest/forest.h"
#include "forest/forestCellBatch.h"
#include "forest/forestCollision.h"
#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/physicsBody.h"
#include "T3D/physics/physicsCollision.h"
#include "collision/concretePolyList.h"

#include "gfx/gfxDrawUtil.h"
#include "math/util/frustum.h"


ForestCell::ForestCell( const RectF &rect ) :
   mRect( rect ),
   mBounds( Box3F::Invalid ),
   mLargestItem( ForestItem::Invalid ),
   mIsDirty( false ),
   mIsInteriorOnly( false )
{
   dMemset( mSubCells, 0, sizeof( mSubCells ) );
   dMemset( mPhysicsRep, 0, sizeof( mPhysicsRep ) );
}

ForestCell::~ForestCell()
{
   mItems.clear();

   for ( U32 i=0; i < 4; i++ )
      SAFE_DELETE( mSubCells[i] );

   freeBatches();

   SAFE_DELETE( mPhysicsRep[0] );
   SAFE_DELETE( mPhysicsRep[1] );
}

void ForestCell::freeBatches()
{
   for ( U32 i=0; i < mBatches.size(); i++ )
      SAFE_DELETE( mBatches[i] );

   mBatches.clear();
}

void ForestCell::buildBatches()
{
   // Gather items for batches.
   Vector<ForestItem> items;
   getItems( &items );

   // Ask the item to batch itself.
   Vector<ForestItem>::const_iterator item = items.begin();
   bool batched = false;
   for ( ; item != items.end(); item++ )
   {
      // Loop thru the batches till someone 
      // takes this guy off our hands.
      batched = false;
      for ( S32 i=0; i < mBatches.size(); i++ )
      {
         if ( mBatches[i]->add( *item ) )
         {
            batched = true;
            break;
         }
      }

      if ( batched )
         continue;
      
      // Gotta create a new batch.
      ForestCellBatch *batch = item->getData()->allocateBatch();
      if ( batch )
      {
         batch->add( *item );
         mBatches.push_back( batch );
      }
   }
}

S32 ForestCell::renderBatches( SceneRenderState *state, Frustum *culler )
{
   PROFILE_SCOPE( ForestCell_renderBatches );

   if ( !hasBatches() )
      return 0;

   S32 renderedItems = 0;

   for ( S32 i=0; i < mBatches.size(); i++ )
   {
      // Is this batch entirely culled?
      if ( culler && culler->isCulled( mBatches[i]->getWorldBox() ) )
         continue;

      mBatches[i]->render( state );
      renderedItems += mBatches[i]->getItemCount();
   }

   return renderedItems;
}

S32 ForestCell::render( TSRenderState *rdata, const Frustum *culler )
{
   PROFILE_SCOPE( ForestCell_render );

   AssertFatal( isLeaf(), "ForestCell::render() - This shouldn't be called on non-leaf cells!" );
       
   U32 itemsRendered = 0;

   // TODO: Items are generated in order of type,
   // so we can maybe save some overhead by preparing
   // the item for rendering once.

   Vector<ForestItem>::iterator item = mItems.begin();
   for ( ; item != mItems.end(); item++ )
   {
      // Do we need to cull individual items?
      if ( culler && culler->isCulled( item->getWorldBox() ) )
         continue;

      if ( item->getData()->render( rdata, *item ) )
         ++itemsRendered;
   }

   return itemsRendered;
}

void ForestCell::_updateBounds()
{   
   mIsDirty = false;
   mBounds = Box3F::Invalid;
   mLargestItem = ForestItem::Invalid;

   F32 radius;

   if ( isBranch() )
   {
      for ( U32 i=0; i < 4; i++ )
      {
         mBounds.intersect( mSubCells[i]->getBounds() );

         radius = mSubCells[i]->mLargestItem.getRadius();
         if ( radius > mLargestItem.getRadius() )
            mLargestItem = mSubCells[i]->mLargestItem;
      }

      return;
   }

   // Loop thru all the items in this cell.
   Vector<ForestItem>::const_iterator item = mItems.begin();
   for ( ; item != mItems.end(); item++ )
   {
      mBounds.intersect( (*item).getWorldBox() );

      radius = (*item).getRadius();
      if ( radius > mLargestItem.getRadius() )
         mLargestItem = (*item);
   }
}

void ForestCell::_updateZoning( const SceneZoneSpaceManager *zoneManager )
{
   PROFILE_SCOPE( ForestCell_UpdateZoning );

   mZoneOverlap.setSize( zoneManager->getNumZones() );
   mZoneOverlap.clear();
   mIsInteriorOnly = true;

   if ( isLeaf() )
   {
      // Skip empty cells... they don't have valid bounds.
      if ( mItems.empty() )
         return;

      Vector<U32> zones;
      zoneManager->findZones( getBounds(), zones );

      for ( U32 i=0; i < zones.size(); i++ )
      {
         // Set overlap bit for zone except it's the outdoor zone.
         if( zones[ i ] != SceneZoneSpaceManager::RootZoneId )
            mZoneOverlap.set( zones[i] );
         else
            mIsInteriorOnly = false;
      }

      return;
   }

   for ( U32 i = 0; i < 4; i++ )
   {
      ForestCell *cell = mSubCells[i];
      cell->_updateZoning( zoneManager );
      mZoneOverlap.combineOR( cell->getZoneOverlap() );
      mIsInteriorOnly &= cell->mIsInteriorOnly;
   }
}

bool ForestCell::findIndexByKey( ForestItemKey key, U32 *outIndex ) const
{
   // Do a simple binary search.

   U32   i = 0,
         lo = 0,
         hi = mItems.size();
   
   const ForestItem *items = mItems.address();

   while ( lo < hi ) 
   {
      i = (lo + hi) / 2;

      if ( key < items[i].getKey() )
         hi = i;
      else if ( key > items[i].getKey() )
         lo = i + 1;
      else
      {
         *outIndex = i;
         return true;
      }
   }

   *outIndex = lo;
   return false;
}

const ForestItem& ForestCell::insertItem( ForestItemKey key,
                                          ForestItemData *data,
                                          const MatrixF &xfm,
                                          F32 scale )
{
   AssertFatal( key != 0, "ForestCell::insertItem() - Got null key!" );
   AssertFatal( data != NULL, "ForestCell::insertItem() - Got null datablock!" );

   // Make sure we update the bounds later.
   mIsDirty = true;

   // PhysicsBody is now invalid and must be rebuilt later.
   SAFE_DELETE( mPhysicsRep[0] );
   SAFE_DELETE( mPhysicsRep[1] );

   // Destroy batches so we recreate it on
   // the next next render.
   freeBatches();

   // Ok... do we need to split this cell?
   if ( isLeaf() && mItems.size() > MaxItems )
   {
      // Add the children.
      for ( U32 i=0; i < 4; i++ )
         mSubCells[i] = new ForestCell( _makeChildRect( i ) );

      // Now push all our current children down.
      Vector<ForestItem>::iterator item = mItems.begin();
      for ( ; item != mItems.end(); item++ )
      {
         U32 index = _getSubCell( item->getPosition().x, item->getPosition().y );

         mSubCells[index]->insertItem( item->getKey(), 
                                       item->getData(), 
                                       item->getTransform(), 
                                       item->getScale() );
      }

      // Clean up.
      mItems.clear();
      mItems.compact();
   }

   // Do we have children?
   if ( isBranch() )
   {
      // Ok... kick this item down then.
      U32 index = _getSubCell( xfm.getPosition().x, xfm.getPosition().y );
      const ForestItem &result = mSubCells[index]->insertItem( key, data, xfm, scale );

      AssertFatal( index == _getSubCell( result.getPosition().x, result.getPosition().y ), "ForestCell::insertItem() - binning is hosed." );

      return result;
   }

   // Do the datablock preload here to insure it happens
   // before an item is used in the scene.
   data->preload();

   // First see if we can find it.  This is nifty so
   // I'll explain it a bit more.
   // 
   // The find function does a binary search thru the
   // sorted item list.
   //
   // If found the index is the position of the item.
   // 
   // If not found the index is the correct insertion 
   // position for adding the new item.
   //
   // So not only do we have a fast find which is worst
   // case O(log n)... but we also have the proper insert
   // position to maintain a sorted item list.
   //
   U32 index;
   bool found = findIndexByKey( key, &index );
   
   // If we didn't find one then insert it.
   if ( !found )
      mItems.insert( index );

   // Update the item settings.
   ForestItem &item = mItems[ index ];   
   item.setData( data );
   item.setTransform( xfm, scale );

   if ( !found )
      item.setKey( key );

   return item;
}

bool ForestCell::removeItem( ForestItemKey key, const Point3F &keyPos, bool deleteIfEmpty )
{
   PROFILE_SCOPE( ForestCell_removeItem );

   AssertFatal( key != 0, "ForestCell::removeItem() - Got null key!" );

   // If this cell has no items then check the children.
   if ( mItems.empty() )
   {
      // Let the child deal with it.
      U32 index = _getSubCell( keyPos.x, keyPos.y );
      if ( !mSubCells[index]->removeItem( key, keyPos, deleteIfEmpty ) )
      {
         // For debugging lets make sure we didn't pick the wrong subCell...
         
         return false;
      }

      // If requested by the caller delete our empty subcells.      
      // Note that by deleting SubCell[0] we have become a leaf with no items
      // and will return true to our parent's isEmpty test.
      if ( deleteIfEmpty &&
           mSubCells[0]->isEmpty() && 
           mSubCells[1]->isEmpty() && 
           mSubCells[2]->isEmpty() &&
           mSubCells[3]->isEmpty() )
      {
         SAFE_DELETE( mSubCells[0] );
         SAFE_DELETE( mSubCells[1] );
         SAFE_DELETE( mSubCells[2] );
         SAFE_DELETE( mSubCells[3] );
      }
   }
   else
   {
      // First see if we can find it.
      U32 index;
      if ( !findIndexByKey( key, &index ) )
         return false;

      // Erase it.
      mItems.erase( index );
   }

   // Do a full bounds update on the next request.
   mIsDirty = true;

   // PhysicsBody is now invalid and must be rebuilt later.
   SAFE_DELETE( mPhysicsRep[0] );
   SAFE_DELETE( mPhysicsRep[1] );

   // Destroy batches so we recreate it on
   // the next next render.
   freeBatches();

   return true;
}

void ForestCell::getItems( Vector<ForestItem> *outItems ) const
{
   Vector<const ForestCell*> stack;
   stack.push_back( this );

   // Now loop till we run out of cells.
   while ( !stack.empty() )
   {
      // Pop off the next cell.
      const ForestCell *cell = stack.last();
      stack.pop_back();

      // Recurse thru non-leaf cells.
      if ( !cell->isLeaf() )
      {
         stack.merge( cell->mSubCells, 4 );
         continue;
      }

      // Get the items.
      outItems->merge( cell->getItems() );
   }
}

void ForestCell::buildPhysicsRep( Forest *forest )
{   
   AssertFatal( isLeaf(), "ForestCell::buildPhysicsRep() - This shouldn't be called on non-leaf cells!" );

   bool isServer = forest->isServerObject();

   // Already has a PhysicsBody, if it needed to be rebuilt it would
   // already be null.
   if ( mPhysicsRep[ isServer ] )
      return;   

   if ( !PHYSICSMGR )
      return;

   PhysicsCollision *colShape = NULL;

   // If we can steal the collision shape from the server-side cell   
   // then do so as it saves us alot of cpu time and memory.
   if ( mPhysicsRep[ 1 ] )
   {      
      colShape = mPhysicsRep[ 1 ]->getColShape();
   }
   else
   {
      // We must pass a sphere to buildPolyList but it is not used.
      const static SphereF dummySphere( Point3F::Zero, 0 );       

      // Step thru them and build collision data.
      ForestItemVector::iterator itemItr = mItems.begin();
      ConcretePolyList polyList;
      for ( ; itemItr != mItems.end(); itemItr++ )
      {
         const ForestItem &item = *itemItr;
         const ForestItemData *itemData = item.getData();
         
         // If not collidable don't need to build anything.
         if ( !itemData->mCollidable )
            continue;

         // TODO: When we add breakable tree support this is where
         // we would need to store their collision data seperately.

         item.buildPolyList( &polyList, item.getWorldBox(), dummySphere );

         // TODO: Need to support multiple collision shapes
         // for really big forests at some point in the future.
      }

      if ( !polyList.isEmpty() )
      {
         colShape = PHYSICSMGR->createCollision();
         if ( !colShape->addTriangleMesh( polyList.mVertexList.address(),
                                          polyList.mVertexList.size(),
                                          polyList.mIndexList.address(),
                                          polyList.mIndexList.size() / 3,
                                          MatrixF::Identity ) )
         {
            SAFE_DELETE( colShape );
         }
      }
   }

   // We might not have any trees.
   if ( !colShape )
      return;

   PhysicsWorld *world = PHYSICSMGR->getWorld( isServer ? "server" : "client" );
   mPhysicsRep[ isServer ] = PHYSICSMGR->createBody();
   mPhysicsRep[ isServer ]->init( colShape, 0, 0, forest, world );
}

void ForestCell::clearPhysicsRep( Forest *forest )
{
   bool isServer = forest->isServerObject();

   SAFE_DELETE( mPhysicsRep[ isServer ] );
}