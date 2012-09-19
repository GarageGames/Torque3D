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

#ifndef _FORESTDATAFILE_H_
#define _FORESTDATAFILE_H_

#ifndef _FORESTITEM_H_
#include "forest/forestItem.h"
#endif
#ifndef _TDICTIONARY_H_
#include "core/util/tDictionary.h"
#endif

class ForestCell;
class Forest;
class Frustum;


/// This is the data file for Forests.
class ForestData
{
   protected:

      enum { FILE_VERSION = 1 };

      /// Set the bucket dimensions to 2km x 2km.
      static const U32 BUCKET_DIM = 2000;

      /// Set to true if the file is dirty and
      /// needs to be saved before being destroyed.
      bool mIsDirty;

      ///
      typedef HashTable<Point2I,ForestCell*> BucketTable;

      /// The top level cell buckets which allows us
      /// to have virtually unbounded range.
      BucketTable mBuckets;

      /// The next free item id.
      static U32 smNextItemId;

      /// Converts a ForestItem's Point3F 'KeyPosition' to a Point2I
      /// key we index into BucketTable with.
      static Point2I _getBucketKey( const Point3F &pos );

      /// Finds the bucket with the given Point2I key or returns NULL.
      ForestCell* _findBucket( const Point2I &key ) const;

      /// Finds the best top level bucket for the ForestItem 'key' position
      /// or returns NULL.
      ForestCell* _findBucket( const Point3F &pos ) const;      

      /// Find the best top level bucket for the given position
      /// or returns a new one.
      ForestCell* _findOrCreateBucket( const Point3F &pos );

      void _onItemReload();
      

   public:

      ForestData();
      virtual ~ForestData();

      bool isDirty() const { return mIsDirty; }

      /// Deletes all the data and resets the 
      /// file to an empty state.
      void clear();

      /// Helper for debugging cell generation.
      void regenCells();

      ///
      bool read( Stream &stream );

      ///
      bool write( const char *path );

      const ForestItem& addItem( ForestItemData *data,
                                 const Point3F &position,
                                 F32 rotation,
                                 F32 scale );

      const ForestItem& addItem( ForestItemKey key,
                                 ForestItemData *data,
                                 const MatrixF &xfm,
                                 F32 scale );

      const ForestItem& updateItem( ForestItemKey key,
                                    const Point3F &keyPosition,
                                    ForestItemData *newData,
                                    const MatrixF &newXfm,
                                    F32 newscale );
     
      const ForestItem& updateItem( ForestItem &item );

      bool removeItem( ForestItemKey key, const Point3F &keyPosition );

      /// Performs a search using the position to limit tested cells.
      const ForestItem& findItem( ForestItemKey key, const Point3F &keyPosition ) const;

      /// Does an exhaustive search thru all cells looking for 
      /// the item.  This method is slow and should be avoided.
      const ForestItem& findItem( ForestItemKey key ) const;

      /// Fills a vector with a copy of all the items in the data set.
      ///
      /// @param outItems The output vector of items.
      /// @return The count of items found.
      ///
      U32 getItems( Vector<ForestItem> *outItems ) const;

      /// Fills a vector with a copy of all items in the Frustum.
      /// Note that this IS expensive and this is not how Forest internally
      /// collects items for rendering. This is here for ForestSelectionTool.
      ///
      /// @param The Frustum to cull with.
      /// @param outItems The output vector of items.
      /// @return The count of items found.
      ///
      U32 getItems( const Frustum &culler, Vector<ForestItem> *outItems ) const;

      /// Returns a copy of all the items that intersect the box.  If
      /// the output vector is NULL then it will early out on the first
      /// found item returning 1.
      ///
      /// @param box The search box.
      /// @param outItems The output vector of items or NULL.
      /// @return The count of items found.
      ///
      U32 getItems( const Box3F &box, Vector<ForestItem> *outItems ) const;

      /// Returns a copy of all the items that intersect the sphere.  If
      /// the output vector is NULL then it will early out on the first
      /// found item returning 1.
      ///
      /// @param point The center of the search sphere.
      /// @param radius The radius of the search sphere.
      /// @param outItems The output vector of items or NULL.
      /// @return The count of items found.
      ///
      U32 getItems( const Point3F &point, F32 radius, Vector<ForestItem> *outItems ) const;

      /// Returns a copy of all the items that intersect the 2D circle ignoring
      /// the z component.  If the output vector is NULL then it will early out
      /// on the first found item returning 1.
      ///
      /// @param point The center point of the search circle.
      /// @param radius The radius of the search circle.      
      /// @param outItems The output vector of items or NULL.
      /// @return The count of items found.
      ///
      U32 getItems( const Point2F &point, F32 radius, Vector<ForestItem> *outItems ) const;

      /// Returns a copy of all the items that share the input item datablock.
      ///
      /// @param data The datablock to search for.
      /// @param outItems The output vector of items.
      /// @return The count of items found.
      ///
      U32 getItems( const ForestItemData *data, Vector<ForestItem> *outItems ) const;

      /// Returns all the top level cells which intersect the frustum.
      void getCells( const Frustum &frustum, Vector<ForestCell*> *outCells ) const;

      /// Returns all top level cells.
      void getCells( Vector<ForestCell*> *outCells ) const;
      
      /// Gathers all the datablocks used and returns the count.
      U32 getDatablocks( Vector<ForestItemData*> *outVector ) const;

      ///
      bool castRay( const Point3F &start, const Point3F &end, RayInfo *outInfo, bool rendered ) const;

      ///
      void clearPhysicsRep( Forest *forest );
      void buildPhysicsRep( Forest *forest );
};

inline Point2I ForestData::_getBucketKey( const Point3F &pos )
{   
   return Point2I ( (S32)mFloor(pos.x / BUCKET_DIM) * BUCKET_DIM, 
                    (S32)mFloor(pos.y / BUCKET_DIM) * BUCKET_DIM ); 
}

inline ForestCell* ForestData::_findBucket( const Point3F &pos ) const
{
   return _findBucket( _getBucketKey( pos ) );
}

#endif // _FORESTDATAFILE_H_
