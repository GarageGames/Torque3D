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
#include "forest/forestDataFile.h"

#include "forest/forest.h"
#include "forest/forestCell.h"
#include "T3D/physics/physicsBody.h"
#include "core/stream/fileStream.h"
#include "core/resource.h"
#include "math/mathIO.h"
#include "math/mPoint2.h"
#include "platform/profiler.h"


template<> ResourceBase::Signature Resource<ForestData>::signature()
{
   return MakeFourCC('f','k','d','f');
}

template<>
void* Resource<ForestData>::create( const Torque::Path &path )
{
   FileStream stream;
   stream.open( path.getFullPath(), Torque::FS::File::Read );
   if ( stream.getStatus() != Stream::Ok )
      return NULL;

   ForestData *file = new ForestData();
   if ( !file->read( stream ) )
   {
      delete file;
      return NULL;
   }

   return file;
}


U32 ForestData::smNextItemId = 1;

ForestData::ForestData()
   :  mIsDirty( false )
{
   ForestItemData::getReloadSignal().notify( this, &ForestData::_onItemReload );
}

ForestData::~ForestData()
{
   ForestItemData::getReloadSignal().remove( this, &ForestData::_onItemReload );
   clear();
}

void ForestData::clear()
{
   // We only have to delete the top level cells and ForestCell will
   // clean up its sub-cells in its destructor.   

   BucketTable::Iterator iter = mBuckets.begin();
   for ( ; iter != mBuckets.end(); iter++ ) delete iter->value;
   mBuckets.clear();

   mIsDirty = true;
}

bool ForestData::read( Stream &stream )
{
   // Read our identifier... so we know we're 
   // not reading in pure garbage.
   char id[4] = { 0 };
   stream.read( 4, id );
   if ( dMemcmp( id, "FKDF", 4 ) != 0 )
   {
      Con::errorf( "ForestDataFile::read() - This is not a Forest planting file!" );
      return false;
   }

   // Empty ourselves before we really begin reading.
   clear();

   // Now the version number.
   U8 version;
   stream.read( &version );
   if ( version > (U8)FILE_VERSION )
   {
      Con::errorf( "ForestDataFile::read() - This file was created with an newer version of Forest!" );
      return false;
   }

   // Read in the names of the ForestItemData datablocks
   // and recover the datablock.
   Vector<ForestItemData*> allDatablocks;
   U32 count;
   stream.read( &count );
   allDatablocks.setSize( count );
   for ( U32 i=0; i < count; i++ )
   {
      StringTableEntry name = stream.readSTString();
      ForestItemData* data = ForestItemData::find( name );
      
      // TODO: Change this to instead create a dummy forest data
      // for each so that the user can swap it with the right one.
      if ( data == NULL )
      {
         Con::warnf( "ForestData::read - ForestItemData named %s was not found.", name );
         Con::warnf( "Note this can occur if you have deleted or renamed datablocks prior to loading this forest and is not an 'error' in this scenario." );
      }
      
      allDatablocks[ i ] = data;
   }

   U8 dataIndex;
   Point3F pos;
   QuatF rot;
   F32 scale;
   ForestItemData* data;
   MatrixF xfm;

   U32 skippedItems = 0;

   // Read in the items.
   stream.read( &count );
   for ( U32 i=0; i < count; i++ )
   {
      stream.read( &dataIndex );
      mathRead( stream, &pos );
      mathRead( stream, &rot );
      stream.read( &scale );

      data = allDatablocks[ dataIndex ];
      if ( data )
      {
         rot.setMatrix( &xfm );
         xfm.setPosition( pos );

         addItem( smNextItemId++, data, xfm, scale );
      }
      else
      {
         skippedItems++;
      }
   }

   if ( skippedItems > 0 )
      Con::warnf( "ForestData::read - %i items were skipped because their datablocks were not found.", skippedItems );

   // Clear the dirty flag.
   mIsDirty = false;

   return true;
}

bool ForestData::write( const char *path )
{
   // Open the stream.
   FileStream stream;
   if ( !stream.open( path, Torque::FS::File::Write ) )
   {
      Con::errorf( "ForestDataFile::write() - Failed opening stream!" );
      return false;
   }

   // Write our identifier... so we have a better
   // idea if we're reading pure garbage.
   stream.write( 4, "FKDF" );

   // Now the version number.
   stream.write( (U8)FILE_VERSION );

   // First gather all the ForestItemData datablocks
   // used by the items in the forest.
   Vector<ForestItemData*> allDatablocks;
   getDatablocks( &allDatablocks );

   // Write out the datablock list.
   U32 count = allDatablocks.size();
   stream.write( count );
   for ( U32 i=0; i < count; i++ )
   {
      StringTableEntry localName = allDatablocks[i]->getInternalName();
      AssertFatal( localName != NULL && localName[0] != '\0', "ForestData::write - ForestItemData had no internal name set!" );
      stream.writeString( allDatablocks[i]->getInternalName() );
   }

   // Get a copy of all the items.
   Vector<ForestItem> items;
   getItems( &items );

   // Save the item count.
   stream.write( (U32)items.size() );

   // Save the items.
   Vector<ForestItem>::const_iterator iter = items.begin();
   for ( ; iter != items.end(); iter++ )
   {
      U8 dataIndex = find( allDatablocks.begin(), allDatablocks.end(), iter->getData() ) - allDatablocks.begin();

      stream.write( dataIndex );

      mathWrite( stream, iter->getPosition() );

      QuatF quat;       
      quat.set( iter->getTransform() );
      mathWrite( stream, quat );

      stream.write( iter->getScale() );
   }

   // Clear the dirty flag.
   mIsDirty = false;

   return true;
}

void ForestData::regenCells()
{
   Vector<ForestItem> items;
   getItems( &items );

   clear();

   for ( U32 i=0; i < items.size(); i++ )
   {
      const ForestItem &item = items[i];
      addItem( item.getKey(), item.getData(), item.getTransform(), item.getScale() );
   }

   mIsDirty = true;
}

ForestCell* ForestData::_findBucket( const Point2I &key ) const
{
   BucketTable::ConstIterator iter = mBuckets.find( key );

   if ( iter != mBuckets.end() )
      return iter->value;
   else
      return NULL;
}

ForestCell* ForestData::_findOrCreateBucket( const Point3F &pos )
{
   // Look it up.
   const Point2I key = _getBucketKey( pos );
   BucketTable::Iterator iter = mBuckets.find( key );

   ForestCell *bucket = NULL;
   if ( iter != mBuckets.end() )
      bucket = iter->value;
   else
   {
      bucket = new ForestCell( RectF( key.x, key.y, BUCKET_DIM, BUCKET_DIM ) );
      mBuckets.insertUnique( key, bucket );     
      mIsDirty = true;
   }

   return bucket;
}

void ForestData::_onItemReload()
{
   // Invalidate cell batches and bounds so they
   // will be regenerated next render.

   Vector<ForestCell*> stack;
   getCells( &stack );

   ForestCell *pCell;

   while ( !stack.empty() )
   {
      pCell = stack.last();
      stack.pop_back();

      if ( !pCell )
         continue;

      pCell->freeBatches();
      pCell->invalidateBounds();

      pCell->getChildren( &stack );
   }
}

const ForestItem& ForestData::addItem( ForestItemData *data,
                                       const Point3F &position,
                                       F32 rotation,
                                       F32 scale )
{
   MatrixF xfm;
   xfm.set( EulerF( 0, 0, rotation ), position );

   return addItem(   smNextItemId++,
                     data,
                     xfm, 
                     scale );
}

const ForestItem& ForestData::addItem( ForestItemKey key,
                                       ForestItemData *data,
                                       const MatrixF &xfm,
                                       F32 scale )
{
   ForestCell *bucket = _findOrCreateBucket( xfm.getPosition() );
   
   mIsDirty = true;

   return bucket->insertItem( key, data, xfm, scale );
}

const ForestItem& ForestData::updateItem( ForestItemKey key,
                                          const Point3F &keyPosition,
                                          ForestItemData *newData,
                                          const MatrixF &newXfm,
                                          F32 newScale )
{
   Point2I bucketKey = _getBucketKey( keyPosition );

   ForestCell *bucket = _findBucket( bucketKey );

   if ( !bucket || !bucket->removeItem( key, keyPosition, true ) )
      return ForestItem::Invalid;

   if ( bucket->isEmpty() )
   {
      delete bucket;
      mBuckets.erase( bucketKey );
   }

   return addItem( key, newData, newXfm, newScale );
}

const ForestItem& ForestData::updateItem( ForestItem &item )
{
   return updateItem( item.getKey(), 
                      item.getPosition(), 
                      item.getData(), 
                      item.getTransform(), 
                      item.getScale() );   
}

bool ForestData::removeItem( ForestItemKey key, const Point3F &keyPosition )
{
   Point2I bucketkey = _getBucketKey( keyPosition );

   ForestCell *bucket = _findBucket( keyPosition );

   if ( !bucket || !bucket->removeItem( key, keyPosition, true ) )
      return false;

   if ( bucket->isEmpty() )
   {
      delete bucket;
      mBuckets.erase( bucketkey );
   }      

   mIsDirty = true;

   return true;
}

const ForestItem& ForestData::findItem( ForestItemKey key, const Point3F &keyPos ) const
{
   PROFILE_SCOPE( ForestData_findItem );

   AssertFatal( key != 0, "ForestCell::findItem() - Got null key!" );

   ForestCell *cell = _findBucket( keyPos );

   while ( cell && !cell->isLeaf() )
      cell = cell->getChildAt( keyPos );

   U32 index;
   if ( cell && cell->findIndexByKey( key, &index ) )
      return cell->getItems()[ index ];

   return ForestItem::Invalid;
}

const ForestItem& ForestData::findItem( ForestItemKey key ) const
{
   PROFILE_SCOPE( ForestData_findItem_Slow );

   AssertFatal( key != 0, "ForestData::findItem() - Got null key!" );

   // Do an exhaustive search thru all the cells... this
   // is really crappy... we shouldn't do this regularly.

   Vector<const ForestCell*> stack;
   BucketTable::ConstIterator iter = mBuckets.begin();
   for ( ; iter != mBuckets.end(); iter++ )
      stack.push_back( iter->value );

   // Now loop till we run out of cells.
   while ( !stack.empty() )
   {
      // Pop off the next cell.
      const ForestCell *cell = stack.last();
      stack.pop_back();

      // Recurse thru non-leaf cells.
      if ( !cell->isLeaf() )
      {
         cell->getChildren( &stack );
         continue;
      }

      // Finally search for the item.
      U32 index;
      if ( cell->findIndexByKey( key, &index ) )
         return cell->getItems()[ index ];
   }

   return ForestItem::Invalid;
}

U32 ForestData::getItems( Vector<ForestItem> *outItems ) const
{
   AssertFatal( outItems, "ForestData::getItems() - The output vector was NULL!" );

   PROFILE_SCOPE( ForestData_getItems );

   Vector<const ForestCell*> stack;
   U32 count = 0;

   BucketTable::ConstIterator iter = mBuckets.begin();
   for ( ; iter != mBuckets.end(); iter++ )
      stack.push_back( iter->value );

   // Now loop till we run out of cells.
   while ( !stack.empty() )
   {
      // Pop off the next cell.
      const ForestCell *cell = stack.last();
      stack.pop_back();

      // Recurse thru non-leaf cells.
      if ( !cell->isLeaf() )
      {
         cell->getChildren( &stack );
         continue;
      }

      // Get the items.
      count += cell->getItems().size();
      outItems->merge( cell->getItems() );
   }

   return count;
}

U32 ForestData::getItems( const Frustum &culler, Vector<ForestItem> *outItems ) const
{
   AssertFatal( outItems, "ForestData::getItems() - The output vector was NULL!" );

   PROFILE_SCOPE( ForestData_getItems_ByFrustum );

   Vector<ForestCell*> stack;
   getCells( &stack );
   Vector<ForestItem>::const_iterator iter;
   U32 count = 0;

   // Now loop till we run out of cells.
   while ( !stack.empty() )
   {
      // Pop off the next cell.
      const ForestCell *cell = stack.last();
      stack.pop_back();

      if ( culler.isCulled( cell->getBounds() ) )
         continue;

      // Recurse thru non-leaf cells.
      if ( cell->isBranch() )
      {
         cell->getChildren( &stack );
         continue;
      }

      // Get the items.
      iter = cell->getItems().begin();
      for ( ; iter != cell->getItems().end(); iter++ )
      {
         if ( !culler.isCulled( iter->getWorldBox() ) )
         {
            outItems->merge( cell->getItems() );
            count++;
         }
      }
   }

   return count;
}

U32 ForestData::getItems( const Box3F &box, Vector<ForestItem> *outItems ) const
{
   PROFILE_SCOPE( ForestData_getItems_ByBox );

   Vector<const ForestCell*> stack;
   U32 count = 0;

   BucketTable::ConstIterator iter = mBuckets.begin();
   for ( ; iter != mBuckets.end(); iter++ )
      stack.push_back( iter->value );

   // Now loop till we run out of cells.
   while ( !stack.empty() )
   {
      // Pop off the next cell.
      const ForestCell *cell = stack.last();
      stack.pop_back();

      // If the cell is empty or doesn't overlap the box... skip it.
      if (  cell->isEmpty() || 
            !cell->getBounds().isOverlapped( box ) )
         continue;

      // Recurse thru non-leaf cells.
      if ( !cell->isLeaf() )
      {
         cell->getChildren( &stack );
         continue;
      }

      // Finally look thru the items.
      const Vector<ForestItem> &items = cell->getItems();
      Vector<ForestItem>::const_iterator item = items.begin();
      for ( ; item != items.end(); item++ )
      {
         if ( item->getWorldBox().isOverlapped( box ) )
         {
            // If we don't have an output vector then the user just
            // wanted to know if any object existed... so early out.
            if ( !outItems )
               return 1;

            ++count;
            outItems->push_back( *item );
         }
      }
   }

   return count;
}

U32 ForestData::getItems( const Point3F &point, F32 radius, Vector<ForestItem> *outItems ) const
{
   PROFILE_SCOPE( ForestData_getItems_BySphere );

   Vector<const ForestCell*> stack;
   U32 count = 0;

   BucketTable::ConstIterator iter = mBuckets.begin();
   for ( ; iter != mBuckets.end(); iter++ )
         stack.push_back( iter->value );

   const F32 radiusSq = radius * radius;

   // Now loop till we run out of cells.
   while ( !stack.empty() )
   {
      // Pop off the next cell.
      const ForestCell *cell = stack.last();
      stack.pop_back();

      // TODO: If we could know here that the cell is fully within
      // the sphere... we could do a fast gather of all its elements
      // without any further testing of it or its children.

      // If the cell is empty or doesn't overlap the sphere... skip it.
      if (  cell->isEmpty() || 
            cell->getBounds().getSqDistanceToPoint( point ) > radiusSq )
         continue;

      // Recurse thru non-leaf cells.
      if ( !cell->isLeaf() )
      {
         cell->getChildren( &stack );
         continue;
      }

      // Finally look thru the items.
      const Vector<ForestItem> &items = cell->getItems();
      Vector<ForestItem>::const_iterator item = items.begin();
      for ( ; item != items.end(); item++ )
      {
         if ( item->getWorldBox().getSqDistanceToPoint( point ) < radiusSq )
         {
            // If we don't have an output vector then the user just
            // wanted to know if any object existed... so early out.
            if ( !outItems )
               return 1;

            ++count;
            outItems->push_back( *item );
         }
      }
   }

   return count;
}


U32 ForestData::getItems( const Point2F &point, F32 radius, Vector<ForestItem> *outItems ) const
{
   PROFILE_SCOPE( ForestData_getItems_ByCircle );

   Vector<const ForestCell*> stack;
   U32 count = 0;

   BucketTable::ConstIterator iter = mBuckets.begin();
   for ( ; iter != mBuckets.end(); iter++ )
         stack.push_back( iter->value );

   const F32 radiusSq = radius * radius;

   // Now loop till we run out of cells.
   while ( !stack.empty() )
   {
      // Pop off the next cell.
      const ForestCell *cell = stack.last();
      stack.pop_back();

      // If the cell is empty or doesn't overlap the sphere... skip it.
      if (  cell->isEmpty() || 
            cell->getRect().getSqDistanceToPoint( point ) > radiusSq )
         continue;

      // Recurse thru non-leaf cells.
      if ( !cell->isLeaf() )
      {
         cell->getChildren( &stack );
         continue;
      }

      // Finally look thru the items.
      const Vector<ForestItem> &items = cell->getItems();
      Vector<ForestItem>::const_iterator item = items.begin();
      F32 compareDist;
      for ( ; item != items.end(); item++ )
      {
         compareDist = mSquared( radius + item->getData()->mRadius );
         if ( item->getSqDistanceToPoint( point ) < compareDist )
         {
            // If we don't have an output vector then the user just
            // wanted to know if any object existed... so early out.
            if ( !outItems )
               return 1;

            ++count;
            outItems->push_back( *item );
         }
      }
   }

   return count;
}

U32 ForestData::getItems( const ForestItemData *data, Vector<ForestItem> *outItems ) const
{
   AssertFatal( outItems, "ForestData::getItems() - The output vector was NULL!" );

   PROFILE_SCOPE( ForestData_getItems_ByDatablock );

   Vector<const ForestCell*> stack;
   U32 count = 0;

   BucketTable::ConstIterator iter = mBuckets.begin();
   for ( ; iter != mBuckets.end(); iter++ )
      stack.push_back( iter->value );

   // Now loop till we run out of cells.
   while ( !stack.empty() )
   {
      // Pop off the next cell.
      const ForestCell *cell = stack.last();
      stack.pop_back();

      // Recurse thru non-leaf cells.
      if ( !cell->isLeaf() )
      {
         cell->getChildren( &stack );
         continue;
      }

      // Get the items.
      const Vector<ForestItem> &items = cell->getItems();
      Vector<ForestItem>::const_iterator item = items.begin();
      for ( ; item != items.end(); item++ )
      {
         if ( item->getData() == data )
         {
            ++count;
            outItems->push_back( *item );
         }
      }
   }

   return count;
}

void ForestData::getCells( const Frustum &frustum, Vector<ForestCell*> *outCells ) const
{
   PROFILE_SCOPE( ForestData_getCells_frustum );

   BucketTable::ConstIterator iter = mBuckets.begin();
   for ( ; iter != mBuckets.end(); iter++ )
   {
      if ( !frustum.isCulled( iter->value->getBounds() ) )
         outCells->push_back( iter->value );
   }
}

void ForestData::getCells( Vector<ForestCell*> *outCells ) const
{
   PROFILE_SCOPE( ForestData_getCells_nofrustum );

   BucketTable::ConstIterator iter = mBuckets.begin();
   for ( ; iter != mBuckets.end(); iter++ )         
      outCells->push_back( iter->value );
}

U32 ForestData::getDatablocks( Vector<ForestItemData*> *outVector ) const
{
   Vector<const ForestCell*> stack;
   U32 count = 0;

   BucketTable::ConstIterator iter = mBuckets.begin();
   for ( ; iter != mBuckets.end(); iter++ )
      stack.push_back( iter->value );

   // Now loop till we run out of cells.
   while ( !stack.empty() )
   {
      // Pop off the next cell.
      const ForestCell *cell = stack.last();
      stack.pop_back();

      // Recurse thru non-leaf cells.
      if ( !cell->isLeaf() )
      {
         cell->getChildren( &stack );
         continue;
      }

      // Go thru the items.
      const Vector<ForestItem> &items = cell->getItems();
      Vector<ForestItem>::const_iterator item = items.begin();
      for ( ; item != items.end(); item++ )
      {
         ForestItemData *data = item->getData();

         if ( find( outVector->begin(), outVector->end(), data ) != outVector->end() )
            continue;

         count++;
         outVector->push_back( data );
      }
   }

   return count;
}

void ForestData::clearPhysicsRep( Forest *forest )
{
   Vector<ForestCell*> stack;

   BucketTable::Iterator iter = mBuckets.begin();
   for ( ; iter != mBuckets.end(); iter++ )
      stack.push_back( iter->value );

   // Now loop till we run out of cells.
   while ( !stack.empty() )
   {
      // Pop off the next cell.
      ForestCell *cell = stack.last();
      stack.pop_back();

      // Recurse thru non-leaf cells.
      if ( !cell->isLeaf() )
      {
         cell->getChildren( &stack );
         continue;
      }

      cell->clearPhysicsRep( forest );      
   }
}

void ForestData::buildPhysicsRep( Forest *forest )
{
   Vector<ForestCell*> stack;

   BucketTable::Iterator iter = mBuckets.begin();
   for ( ; iter != mBuckets.end(); iter++ )
      stack.push_back( iter->value );

   // Now loop till we run out of cells.
   while ( !stack.empty() )
   {
      // Pop off the next cell.
      ForestCell *cell = stack.last();
      stack.pop_back();

      // Recurse thru non-leaf cells.
      if ( !cell->isLeaf() )
      {
         cell->getChildren( &stack );
         continue;
      }

      cell->buildPhysicsRep( forest );      
   }   
}