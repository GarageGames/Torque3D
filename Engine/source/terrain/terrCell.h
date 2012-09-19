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

#ifndef _TERRCELL_H_
#define _TERRCELL_H_

#ifndef _GFXVERTEXBUFFER_H_
#include "gfx/gfxVertexBuffer.h"
#endif
#ifndef _GFXPRIMITIVEBUFFER_H_
#include "gfx/gfxPrimitiveBuffer.h"
#endif
#ifndef _TDICTIONARY_H_
#include "core/util/tDictionary.h"
#endif
#ifndef _MORIENTEDBOX_H_
#include "math/mOrientedBox.h"
#endif
#ifndef _BITVECTOR_H_
#include "core/bitVector.h"
#endif

class TerrainBlock;
class TerrainCellMaterial;
class Frustum;
class SceneRenderState;
class SceneZoneSpaceManager;


/// The TerrainCell vertex format optimized to
/// 32 bytes for optimal vertex cache performance.
GFXDeclareVertexFormat( TerrVertex )
{
   /// The position.
   Point3F point;

   /// The normal.
   Point3F normal;

   /// The height for calculating the
   /// tangent vector on the GPU.
   F32 tangentZ;

   /// The empty flag state which is either 
   /// -1 or 1 so we can do the special 
   /// interpolation trick.
   F32 empty;
};


/// The TerrCell is a single quadrant of the terrain geometry quadtree.
class TerrCell
{
protected:

   /// The handle to the static vertex buffer which holds the 
   /// vertices for this cell.
   GFXVertexBufferHandle<TerrVertex> mVertexBuffer;

   /// The handle to the static primitive buffer for this cell.
   /// It is only used if this cell has any empty squares
   GFXPrimitiveBufferHandle mPrimBuffer;

   ///
   Point2I mPoint;
   
   ///
   U32 mSize;

   /// The level of this cell within the quadtree (of cells) where
   /// zero is the root and one is a direct child of the root, etc.
   U32 mLevel;

   /// Statics used in VB and PB generation.   
   static const U32 smVBStride;
   static const U32 smMinCellSize;
   static const U32 smVBSize;
   static const U32 smPBSize;
   static const U32 smTriCount;

   /// Triangle count for our own primitive buffer, if any
   U32 mTriCount;

   /// Indicates if this cell has any empty squares
   bool mHasEmpty;

   /// A list of all empty vertices for this cell
   Vector<U32> mEmptyVertexList;

   /// The terrain this cell is based on.
   TerrainBlock *mTerrain;

   /// The material used to render the cell.
   TerrainCellMaterial *mMaterial;

   /// The bounding box of this cell in 
   /// TerrainBlock object space.
   Box3F mBounds;

   /// The OBB of this cell in world space.
   OrientedBox3F mOBB;

   /// The bounding radius of this cell.
   F32 mRadius;

   /// The child cells of this one.
   TerrCell *mChildren[4];

   /// This bit flag tells us which materials effect
   /// this cell and is used for optimizing rendering.
   /// @see TerrainFile::mMaterialAlphaMap
   U64 mMaterials;

   /// Whether this cell is fully contained inside interior zones.
   bool mIsInteriorOnly;

   /// The zone overlap for this cell.
   /// @note The bit for the outdoor zone is never set.
   BitVector mZoneOverlap;

   ///
   void _updateBounds();

   /// Update #mOBB from the current terrain transform state.
   void _updateOBB();

   //
   void _init( TerrainBlock *terrain,
               const Point2I &point,
               U32 size,
               U32 level );

   // 
   void _updateVertexBuffer();

   //
   void _updatePrimitiveBuffer();

   // 
   void _updateMaterials();

   //
   bool _isVertIndexEmpty( U32 index ) const;

public:

   TerrCell();
   virtual ~TerrCell();

   static TerrCell* init( TerrainBlock *terrain );

   void getRenderPrimitive(   GFXPrimitive *prim,
                              GFXVertexBufferHandleBase *vertBuff,
                              GFXPrimitiveBufferHandle  *primBuff ) const;

   void updateGrid( const RectI &gridRect, bool opacityOnly = false );

   /// Update the world-space OBBs used for culling.
   void updateOBBs();

   ///
   void updateZoning( const SceneZoneSpaceManager *zoneManager );

   void cullCells( const SceneRenderState *state,
                   const Point3F &objLodPos,
                   Vector<TerrCell*> *outCells );

   const Box3F& getBounds() const { return mBounds; }

   /// Returns the object space sphere bounds.
   SphereF getSphereBounds() const { return SphereF( mBounds.getCenter(), mRadius ); }

   F32 getSqDistanceTo( const Point3F &pt ) const;

   F32 getDistanceTo( const Point3F &pt ) const;

   U64 getMaterials() const { return mMaterials; }

   /// Returns a bit vector of what zones overlap this cell.
   const BitVector& getZoneOverlap() const { return mZoneOverlap; }
   
   /// Forces the loading of the materials for this
   /// cell and all its child cells.
   void preloadMaterials();

   TerrainCellMaterial* getMaterial();

   /// Return true if this is a leaf cell, i.e. a cell without children.
   bool isLeaf() const { return !mChildren[ 0 ]; }

   /// Deletes the materials for this cell 
   /// and all its children.  They will be
   /// recreate on the next request.
   void deleteMaterials();

   U32 getSize() const { return mSize; }

   Point2I getPoint() const { return mPoint; }   

   /// Initializes a primitive buffer for rendering any cell.
   static void createPrimBuffer( GFXPrimitiveBufferHandle *primBuffer );

   /// Debug Rendering
   /// @{

   /// Renders the debug bounds for this cell.
   void renderBounds() const;

   /// @}
};

inline F32 TerrCell::getDistanceTo( const Point3F &pt ) const
{
   return ( mBounds.getCenter() - pt ).len() - mRadius;
}

inline bool TerrCell::_isVertIndexEmpty( U32 index ) const
{
   for ( U32 i = 0; i < mEmptyVertexList.size(); ++i )
   {
      if ( mEmptyVertexList[i] == index )
      {
         return true;
      }
   }
   return false;
}

#endif // _TERRCELL_H_
