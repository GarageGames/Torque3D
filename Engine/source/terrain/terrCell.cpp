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
#include "terrain/terrCell.h"

#include "math/util/frustum.h"
#include "terrain/terrData.h"
#include "terrain/terrCellMaterial.h"
#include "scene/sceneRenderState.h"
#include "lighting/lightManager.h"
#include "gfx/gfxDrawUtil.h"


GFXImplementVertexFormat( TerrVertex )
{
   addElement( "POSITION", GFXDeclType_Float3 );
   addElement( "NORMAL", GFXDeclType_Float3 );
   addElement( "TangentZ", GFXDeclType_Float, 0 );
   addElement( "Empty", GFXDeclType_Float, 1 );
};

const U32 TerrCell::smMinCellSize   = 64;
const U32 TerrCell::smVBStride      = TerrCell::smMinCellSize + 1;         // 129
const U32 TerrCell::smVBSize        = ( TerrCell::smVBStride * TerrCell::smVBStride ) + 
                                      ( TerrCell::smVBStride * 4 );        // 17,157
const U32 TerrCell::smPBSize        = ( TerrCell::smMinCellSize * TerrCell::smMinCellSize * 6 ) + 
                                      ( TerrCell::smMinCellSize * 4 * 6 ); // 101,376
const U32 TerrCell::smTriCount      = TerrCell::smPBSize / 3;              // 33,792


TerrCell::TerrCell()
   :  mTriCount( 0 ),
      mHasEmpty( false ),
      mMaterial( NULL ),
      mMaterials( 0 ),
      mIsInteriorOnly( false )
{
   dMemset( mChildren, 0, sizeof( mChildren ) );
}

TerrCell::~TerrCell()
{
   SAFE_DELETE( mMaterial );

   for ( U32 i=0; i < 4; i++ )
      SAFE_DELETE( mChildren[i] );
}

void TerrCell::createPrimBuffer( GFXPrimitiveBufferHandle *primBuffer )
{
   PROFILE_SCOPE( TerrCell_AllocPrimBuffer );

   primBuffer->set( GFX, smPBSize, 1, GFXBufferTypeStatic, "TerrCell" );
   
   // We don't use the primitive for normal clipmap
   // rendering, but it is used for the shadow pass.
   GFXPrimitive *prim = primBuffer->getPointer()->mPrimitiveArray;
   prim->type = GFXTriangleList;
   prim->numPrimitives = smTriCount;
   prim->numVertices = smVBSize;

   //
   // The vertex pattern for the terrain is as
   // follows...
   //
   //     0----1----2.....n
   //     |\   |   /|
   //     | \  |  / |
   //     |  \ | /  |
   //     |   \|/   |
   //     n----n----n
   //     |   /|\   |
   //     |  / | \  |
   //     | /  |  \ |
   //     |/   |   \|
   //     n----n----n
   //

   // Lock and fill it up!
   U16 *idxBuff;
   primBuffer->lock( &idxBuff );
   U32 counter = 0;
   U32 maxIndex = 0;

   for ( U32 y = 0; y < smMinCellSize; y++ )
   {
      const U32 yTess = y % 2;

      for ( U32 x = 0; x < smMinCellSize; x++ )
      {
         U32 index = ( y * smVBStride ) + x;
         
         const U32 xTess = x % 2;

         if ( ( xTess == 0 && yTess == 0 ) ||
              ( xTess != 0 && yTess != 0 ) )
         {
            idxBuff[0] = index + 0;
            idxBuff[1] = index + smVBStride;
            idxBuff[2] = index + smVBStride + 1;

            idxBuff[3] = index + 0;
            idxBuff[4] = index + smVBStride + 1;
            idxBuff[5] = index + 1;
         }
         else
         {
            idxBuff[0] = index + 1;
            idxBuff[1] = index;
            idxBuff[2] = index + smVBStride;

            idxBuff[3] = index + 1;
            idxBuff[4] = index + smVBStride;
            idxBuff[5] = index + smVBStride + 1;
         }

         idxBuff += 6;
         maxIndex = index + 1 + smVBStride;         
         counter += 6;         
      }
   }

   // Now add indices for the 'skirts'.
   // These could probably be reduced to a loop.

   // Temporaries that hold triangle indices.
   // Top/Bottom - 0,1
   U32 t0, t1, b0, b1;

   // Top edge skirt...

   // Index to the first vert of the top row.
   U32 startIndex = 0;
   // Index to the first vert of the skirt under the top row.
   U32 skirtStartIdx = smVBStride * smVBStride;
   // Step to go one vert to the right.
   U32 step = 1;

   for ( U32 i = 0; i < smMinCellSize; i++ )
   {
      t0 = startIndex + i * step;
      t1 = t0 + step;
      b0 = skirtStartIdx + i;
      b1 = skirtStartIdx + i + 1;

      idxBuff[0] = b0;
      idxBuff[1] = t0;
      idxBuff[2] = t1;

      idxBuff[3] = b1;
      idxBuff[4] = b0;
      idxBuff[5] = t1;

      idxBuff += 6;
      maxIndex = b1;
      counter += 6;
   }

   // Bottom edge skirt...

   // Index to the first vert of the bottom row.
   startIndex = smVBStride * smVBStride - smVBStride;
   // Index to the first vert of the skirt under the bottom row.
   skirtStartIdx = startIndex + smVBStride * 2;
   // Step to go one vert to the right.
   step = 1;
   
   for ( U32 i = 0; i < smMinCellSize; i++ )
   {
      t0 = startIndex + ( i * step );
      t1 = t0 + step;
      b0 = skirtStartIdx + i;
      b1 = skirtStartIdx + i + 1;

      idxBuff[0] = t1;
      idxBuff[1] = t0;
      idxBuff[2] = b0;

      idxBuff[3] = t1;
      idxBuff[4] = b0;
      idxBuff[5] = b1;

      idxBuff += 6;
      maxIndex = b1;
      counter += 6;
   }

   // Left edge skirt...

   // Index to the first vert of the left column.
   startIndex = 0;
   // Index to the first vert of the skirt under the left column.
   skirtStartIdx = smVBStride * smVBStride + smVBStride * 2;
   // Step to go one vert down.
   step = smVBStride;

   for ( U32 i = 0; i < smMinCellSize; i++ )
   {
      t0 = startIndex + ( i * step );
      t1 = t0 + step;
      b0 = skirtStartIdx + i;
      b1 = skirtStartIdx + i + 1;

      idxBuff[0] = t1;
      idxBuff[1] = t0;
      idxBuff[2] = b0;

      idxBuff[3] = t1;
      idxBuff[4] = b0;
      idxBuff[5] = b1;

      idxBuff += 6;
      maxIndex = b1;
      counter += 6;
   }

   // Right edge skirt...

   // Index to the first vert of the right column.
   startIndex = smVBStride - 1;
   // Index to the first vert of the skirt under the right column.
   skirtStartIdx = smVBStride * smVBStride + smVBStride * 3;
   // Step to go one vert down.
   step = smVBStride;

   for ( U32 i = 0; i < smMinCellSize; i++ )
   {
      t0 = startIndex + ( i * step );
      t1 = t0 + step;
      b0 = skirtStartIdx + i;
      b1 = skirtStartIdx + i + 1;

      idxBuff[0] = b0;
      idxBuff[1] = t0;
      idxBuff[2] = t1;

      idxBuff[3] = b1;
      idxBuff[4] = b0;
      idxBuff[5] = t1;

      idxBuff += 6;
      maxIndex = b1;
      counter += 6;
   }

   primBuffer->unlock();
}

TerrCell* TerrCell::init( TerrainBlock *terrain )
{
   // Just create the root cell and call the inner init.
   TerrCell *root = new TerrCell;
   root->_init(   terrain, 
                  Point2I( 0, 0 ),
                  terrain->getBlockSize(),
                  0 );

   // Set initial states of OBBs.
   root->updateOBBs();

   return root;
}


void TerrCell::_init( TerrainBlock *terrain,               
                      const Point2I &point,
                      U32 size,
                      U32 level )
{
   PROFILE_SCOPE( TerrCell_Init );

   mTerrain = terrain;
   mPoint = point;
   mSize = size;
   mLevel = level;

   // Generate a VB (and maybe a PB) for this cell, unless we are the Root cell.
   if ( level > 0 )
   {
      _updateVertexBuffer();
      _updatePrimitiveBuffer();
   }
      
   if ( mSize <= smMinCellSize )
   {
      // Update our bounds and materials... the 
      // parent will use it to update itself.
      _updateBounds();
      _updateMaterials();
      return;
   }

   // Create our children and update our 
   // bounds and materials from them.

   const U32 childSize = mSize / 2;
   const U32 childLevel = mLevel + 1;

   mChildren[0] = new TerrCell;
   mChildren[0]->_init( mTerrain,               
                        Point2I( mPoint.x, mPoint.y ),
                        childSize,
                        childLevel );
   mBounds = mChildren[0]->getBounds();
   mMaterials = mChildren[0]->getMaterials();

   mChildren[1] = new TerrCell;
   mChildren[1]->_init( mTerrain,               
                        Point2I( mPoint.x + childSize, mPoint.y ),
                        childSize,
                        childLevel );
   mBounds.intersect( mChildren[1]->getBounds() );
   mMaterials |= mChildren[1]->getMaterials();

   mChildren[2] = new TerrCell;
   mChildren[2]->_init( mTerrain,               
                        Point2I( mPoint.x, mPoint.y + childSize ),
                        childSize,
                        childLevel );
   mBounds.intersect( mChildren[2]->getBounds() );
   mMaterials |= mChildren[2]->getMaterials();

   mChildren[3] = new TerrCell;
   mChildren[3]->_init( mTerrain,               
                        Point2I( mPoint.x + childSize, mPoint.y + childSize ),
                        childSize, 
                        childLevel );
   mBounds.intersect( mChildren[3]->getBounds() );
   mMaterials |= mChildren[3]->getMaterials();

   mRadius = mBounds.len() * 0.5f;

   _updateOBB();
}

void TerrCell::updateGrid( const RectI &gridRect, bool opacityOnly )
{
   PROFILE_SCOPE( TerrCell_UpdateGrid );

   // If we have a VB... then update it.
   if ( mVertexBuffer.isValid() && !opacityOnly )            
      _updateVertexBuffer();

   // Update our PB, if any
   _updatePrimitiveBuffer();

   // If we don't have children... then we're
   // a leaf at the bottom of the cell quadtree
   // and we should just update our bounds.
   if ( !mChildren[0] )
   {
      if ( !opacityOnly )
         _updateBounds(); 

      _updateMaterials();
      return;
   }

   // Otherwise, we must call updateGrid on our children
   // and then update our bounds/materials AFTER to contain them.

   mMaterials = 0;

   for ( U32 i = 0; i < 4; i++ )
   {
      TerrCell *cell = mChildren[i];

      // The overlap test doesn't hit shared edges
      // so grow it a bit when we create it.
      const RectI cellRect( cell->mPoint.x - 1,
                            cell->mPoint.y - 1,
                            cell->mSize + 2, 
                            cell->mSize + 2 );

      // We do an overlap and containment test as it 
      // properly handles zero sized rects.
      if (  cellRect.contains( gridRect ) ||
            cellRect.overlaps( gridRect ) )
         cell->updateGrid( gridRect, opacityOnly );

      // Update the bounds from our children.
      if ( !opacityOnly )
      {
         if ( i == 0 )
            mBounds = mChildren[i]->getBounds();
         else
            mBounds.intersect( mChildren[i]->getBounds() );

         mRadius = mBounds.len() * 0.5f;
      }

      // Update the material flags.
      mMaterials |= mChildren[i]->getMaterials();
   }

   if ( mMaterial )
      mMaterial->init( mTerrain, mMaterials );
}

void TerrCell::_updateVertexBuffer()
{
   PROFILE_SCOPE( TerrCell_UpdateVertexBuffer );

   // Start off with no empty squares
   mHasEmpty = false;
   mEmptyVertexList.clear();

   mVertexBuffer.set( GFX, smVBSize, GFXBufferTypeStatic );

   const F32 squareSize = mTerrain->getSquareSize();
   const U32 blockSize = mTerrain->getBlockSize();
   const U32 stepSize = mSize / smMinCellSize;

   U32 vbcounter = 0;

   TerrVertex *vert = mVertexBuffer.lock();

   Point2I gridPt;
   Point2F point;
   F32 height;
   Point3F normal;   
   
   const TerrainFile *file = mTerrain->getFile();

   for ( U32 y = 0; y < smVBStride; y++ )
   {
      for ( U32 x = 0; x < smVBStride; x++ )
      {
         // We clamp here to keep the geometry from reading across
         // one side of the height map to the other causing walls
         // around the edges of the terrain.
         gridPt.x = mClamp( mPoint.x + x * stepSize, 0, blockSize - 1 );
         gridPt.y = mClamp( mPoint.y + y * stepSize, 0, blockSize - 1 );

         // Setup this point.
         point.x = (F32)gridPt.x * squareSize;
         point.y = (F32)gridPt.y * squareSize;
         height = fixedToFloat( file->getHeight( gridPt.x, gridPt.y ) );
         vert->point.x = point.x;
         vert->point.y = point.y;
         vert->point.z = height;

         // Get the normal.
         mTerrain->getSmoothNormal( point, &normal, true, false );
         vert->normal = normal;

         // Get the tangent z.
         vert->tangentZ = fixedToFloat( file->getHeight( gridPt.x + 1, gridPt.y ) ) - height;

         // Test the empty state for this vert.
         if ( file->isEmptyAt( gridPt.x, gridPt.y ) )
         {
            mHasEmpty = true;
            mEmptyVertexList.push_back( vbcounter );
         }

         vbcounter++;
         ++vert;
      }
   }

   // Add verts for 'skirts' around/beneath the edge verts of this cell.
   // This could probably be reduced to a loop...
   
   const F32 skirtDepth = mSize / smMinCellSize * mTerrain->getSquareSize();

   // Top edge skirt
   for ( U32 i = 0; i < smVBStride; i++ )
   {      
      gridPt.x = mClamp( mPoint.x + i * stepSize, 0, blockSize - 1 );
      gridPt.y = mClamp( mPoint.y, 0, blockSize - 1 );
      
      point.x = (F32)gridPt.x * squareSize;
      point.y = (F32)gridPt.y * squareSize;
      height = fixedToFloat( file->getHeight( gridPt.x, gridPt.y ) );
      vert->point.x = point.x;
      vert->point.y = point.y;
      vert->point.z = height - skirtDepth;

      // Get the normal.
      mTerrain->getNormal( point, &normal, true, false );
      vert->normal = normal;

      // Get the tangent.
      vert->tangentZ = height - fixedToFloat( file->getHeight( gridPt.x + 1, gridPt.y ) );

      vbcounter++;
      ++vert;      
   }

   // Bottom edge skirt
   for ( U32 i = 0; i < smVBStride; i++ )
   {      
      gridPt.x = mClamp( mPoint.x + i * stepSize, 0, blockSize - 1 );
      gridPt.y = mClamp( mPoint.y + smMinCellSize * stepSize, 0, blockSize - 1 );

      point.x = (F32)gridPt.x * squareSize;
      point.y = (F32)gridPt.y * squareSize;
      height = fixedToFloat( file->getHeight( gridPt.x, gridPt.y ) );
      vert->point.x = point.x;
      vert->point.y = point.y;
      vert->point.z = height - skirtDepth;

      // Get the normal.
      mTerrain->getNormal( point, &normal, true, false );
      vert->normal = normal;

      // Get the tangent.
      vert->tangentZ = height - fixedToFloat( file->getHeight( gridPt.x + 1, gridPt.y ) );

      vbcounter++;
      ++vert;      
   }

   // Left edge skirt
   for ( U32 i = 0; i < smVBStride; i++ )
   {      
      gridPt.x = mClamp( mPoint.x, 0, blockSize - 1 );
      gridPt.y = mClamp( mPoint.y + i * stepSize, 0, blockSize - 1 );

      point.x = (F32)gridPt.x * squareSize;
      point.y = (F32)gridPt.y * squareSize;
      height = fixedToFloat( file->getHeight( gridPt.x, gridPt.y ) );
      vert->point.x = point.x;
      vert->point.y = point.y;
      vert->point.z = height - skirtDepth;

      // Get the normal.
      mTerrain->getNormal( point, &normal, true, false );
      vert->normal = normal;

      // Get the tangent.
      vert->tangentZ = height - fixedToFloat( file->getHeight( gridPt.x + 1, gridPt.y ) );

      vbcounter++;
      ++vert;      
   }

   // Right edge skirt
   for ( U32 i = 0; i < smVBStride; i++ )
   {      
      gridPt.x = mClamp( mPoint.x + smMinCellSize * stepSize, 0, blockSize - 1 );
      gridPt.y = mClamp( mPoint.y + i * stepSize, 0, blockSize - 1 );

      point.x = (F32)gridPt.x * squareSize;
      point.y = (F32)gridPt.y * squareSize;
      height = fixedToFloat( file->getHeight( gridPt.x, gridPt.y ) );
      vert->point.x = point.x;
      vert->point.y = point.y;
      vert->point.z = height - skirtDepth;

      // Get the normal.
      mTerrain->getNormal( point, &normal, true, false );
      vert->normal = normal;

      // Get the tangent.
      vert->tangentZ = height - fixedToFloat( file->getHeight( gridPt.x + 1, gridPt.y ) );

      vbcounter++;
      ++vert;      
   }

   AssertFatal( vbcounter == smVBSize, "bad" );
   mVertexBuffer.unlock();
}

void TerrCell::_updatePrimitiveBuffer()
{
   PROFILE_SCOPE( TerrCell_UpdatePrimitiveBuffer );

   if ( !mHasEmpty )
   {
      if ( mPrimBuffer.isValid() )
      {
         // There are no more empty squares for this cell, so
         // get rid of the primitive buffer to use the standard one.
         mPrimBuffer = NULL;
      }

      return;
   }

   // Build our custom primitive buffer.  We're setting it to the maximum allowed
   // size, but should be just shy of it depending on the number of empty squares
   // in this cell.  We could calculate it, but note that it would be different
   // from mEmptyVertexList.size() as that can include vertices on the edges that
   // are really considered part of another cell's squares.  So we take a slightly
   // larger buffer over running through the calculation.
   mPrimBuffer.set( GFX, smPBSize, 1, GFXBufferTypeStatic, "TerrCell" );

   GFXPrimitive *prim = mPrimBuffer.getPointer()->mPrimitiveArray;
   prim->type = GFXTriangleList;
   prim->numVertices = smVBSize;

   mTriCount = 0;

   // Lock and fill it up!
   U16 *idxBuff;
   mPrimBuffer.lock( &idxBuff );
   U32 counter = 0;
   U32 maxIndex = 0;

   for ( U32 y = 0; y < smMinCellSize; y++ )
   {
      const U32 yTess = y % 2;

      for ( U32 x = 0; x < smMinCellSize; x++ )
      {
         U32 index = ( y * smVBStride ) + x;
         
         // Should this square be skipped?
         if ( _isVertIndexEmpty(index) )
            continue;

         const U32 xTess = x % 2;

         if ( ( xTess == 0 && yTess == 0 ) ||
              ( xTess != 0 && yTess != 0 ) )
         {
            idxBuff[0] = index + 0;
            idxBuff[1] = index + smVBStride;
            idxBuff[2] = index + smVBStride + 1;

            idxBuff[3] = index + 0;
            idxBuff[4] = index + smVBStride + 1;
            idxBuff[5] = index + 1;
         }
         else
         {
            idxBuff[0] = index + 1;
            idxBuff[1] = index;
            idxBuff[2] = index + smVBStride;

            idxBuff[3] = index + 1;
            idxBuff[4] = index + smVBStride;
            idxBuff[5] = index + smVBStride + 1;
         }

         idxBuff += 6;
         maxIndex = index + 1 + smVBStride;         
         counter += 6;         

         mTriCount += 2;
      }
   }

   // Now add indices for the 'skirts'.
   // These could probably be reduced to a loop.

   // Temporaries that hold triangle indices.
   // Top/Bottom - 0,1
   U32 t0, t1, b0, b1;

   // Top edge skirt...

   // Index to the first vert of the top row.
   U32 startIndex = 0;
   // Index to the first vert of the skirt under the top row.
   U32 skirtStartIdx = smVBStride * smVBStride;
   // Step to go one vert to the right.
   U32 step = 1;

   for ( U32 i = 0; i < smMinCellSize; i++ )
   {
      t0 = startIndex + i * step;
         
      // Should this square be skipped?
      if ( _isVertIndexEmpty(t0) )
         continue;

      t1 = t0 + step;
      b0 = skirtStartIdx + i;
      b1 = skirtStartIdx + i + 1;

      idxBuff[0] = b0;
      idxBuff[1] = t0;
      idxBuff[2] = t1;

      idxBuff[3] = b1;
      idxBuff[4] = b0;
      idxBuff[5] = t1;

      idxBuff += 6;
      maxIndex = b1;
      counter += 6;

      mTriCount += 2;
   }

   // Bottom edge skirt...

   // Index to the first vert of the bottom row.
   startIndex = smVBStride * smVBStride - smVBStride;
   // Index to the first vert of the skirt under the bottom row.
   skirtStartIdx = startIndex + smVBStride * 2;
   // Step to go one vert to the right.
   step = 1;
   
   for ( U32 i = 0; i < smMinCellSize; i++ )
   {
      t0 = startIndex + ( i * step );
         
      // Should this square be skipped?  We actually need to test
      // the vertex one row down as it defines the empty state
      // for this square.
      if ( _isVertIndexEmpty( t0 - smVBStride ) )
         continue;

      t1 = t0 + step;
      b0 = skirtStartIdx + i;
      b1 = skirtStartIdx + i + 1;

      idxBuff[0] = t1;
      idxBuff[1] = t0;
      idxBuff[2] = b0;

      idxBuff[3] = t1;
      idxBuff[4] = b0;
      idxBuff[5] = b1;

      idxBuff += 6;
      maxIndex = b1;
      counter += 6;

      mTriCount += 2;
   }

   // Left edge skirt...

   // Index to the first vert of the left column.
   startIndex = 0;
   // Index to the first vert of the skirt under the left column.
   skirtStartIdx = smVBStride * smVBStride + smVBStride * 2;
   // Step to go one vert down.
   step = smVBStride;

   for ( U32 i = 0; i < smMinCellSize; i++ )
   {
      t0 = startIndex + ( i * step );
         
      // Should this square be skipped?
      if ( _isVertIndexEmpty(t0) )
         continue;

      t1 = t0 + step;
      b0 = skirtStartIdx + i;
      b1 = skirtStartIdx + i + 1;

      idxBuff[0] = t1;
      idxBuff[1] = t0;
      idxBuff[2] = b0;

      idxBuff[3] = t1;
      idxBuff[4] = b0;
      idxBuff[5] = b1;

      idxBuff += 6;
      maxIndex = b1;
      counter += 6;

      mTriCount += 2;
   }

   // Right edge skirt...

   // Index to the first vert of the right column.
   startIndex = smVBStride - 1;
   // Index to the first vert of the skirt under the right column.
   skirtStartIdx = smVBStride * smVBStride + smVBStride * 3;
   // Step to go one vert down.
   step = smVBStride;

   for ( U32 i = 0; i < smMinCellSize; i++ )
   {
      t0 = startIndex + ( i * step );
         
      // Should this square be skipped?  We actually need to test
      // the vertex one column to the left as it defines the empty
      // state for this square.
      if ( _isVertIndexEmpty( t0 - 1 ) )
         continue;

      t1 = t0 + step;
      b0 = skirtStartIdx + i;
      b1 = skirtStartIdx + i + 1;

      idxBuff[0] = b0;
      idxBuff[1] = t0;
      idxBuff[2] = t1;

      idxBuff[3] = b1;
      idxBuff[4] = b0;
      idxBuff[5] = t1;

      idxBuff += 6;
      maxIndex = b1;
      counter += 6;

      mTriCount += 2;
   }

   mPrimBuffer.unlock();
   prim->numPrimitives = mTriCount;
}

void TerrCell::_updateMaterials()
{
   PROFILE_SCOPE( TerrCell_UpdateMaterials );

   // This should really only be called for cells of smMinCellSize,
   // in which case stepSize is always one.
   U32 stepSize = mSize / smMinCellSize;
   mMaterials = 0;
   U8 index;
   U32 x, y;

   const TerrainFile *file = mTerrain->getFile();

   // Step thru the samples in the map then.
   for ( y = 0; y < smVBStride; y++ )
   {
      for ( x = 0; x < smVBStride; x++ )
      {
         index = file->getLayerIndex(  mPoint.x + x * stepSize, 
                                       mPoint.y + y * stepSize );
         
         // Skip empty layers and anything that doesn't fit
         // the 64bit material flags.
         if ( index == U8_MAX || index > 63 )
            continue;

         mMaterials |= (U64)((U64)1<<index);
      }
   }

   if ( mMaterial )
      mMaterial->init( mTerrain, mMaterials );
}

void TerrCell::_updateBounds()
{
   PROFILE_SCOPE( TerrCell_UpdateBounds );

   const F32 squareSize = mTerrain->getSquareSize();

   // This should really only be called for cells of smMinCellSize,
   // in which case stepSize is always one.
   const U32 stepSize = mSize / smMinCellSize;

   // Prepare to expand the bounds.
   mBounds.minExtents.set( F32_MAX, F32_MAX, F32_MAX );
   mBounds.maxExtents.set( -F32_MAX, -F32_MAX, -F32_MAX );   

   Point3F vert;
   Point2F texCoord;

   const TerrainFile *file = mTerrain->getFile();

   for ( U32 y = 0; y < smVBStride; y++ )
   {
      for ( U32 x = 0; x < smVBStride; x++ )
      {
         // Setup this point.
         vert.x = (F32)( mPoint.x + x * stepSize ) * squareSize;
         vert.y = (F32)( mPoint.y + y * stepSize ) * squareSize;
         vert.z = fixedToFloat( file->getHeight(   mPoint.x + x,
                                                   mPoint.y + y ) );

         // HACK: Call it twice to deal with the inverted
         // inital bounds state... shouldn't be a perf issue.
         mBounds.extend( vert );
         mBounds.extend( vert );
      }
   }

   mRadius = mBounds.len() * 0.5;

   _updateOBB();
}

void TerrCell::_updateOBB()
{
   mOBB.set( mTerrain->getTransform(), mBounds );
}

void TerrCell::updateOBBs()
{
   _updateOBB();

   // Update children.

   if( mChildren[ 0 ] )
      for( U32 i = 0; i < 4; ++ i )
         mChildren[ i ]->updateOBBs();
}

void TerrCell::updateZoning( const SceneZoneSpaceManager *zoneManager )
{
   PROFILE_SCOPE( TerrCell_UpdateZoning );

   mZoneOverlap.setSize( zoneManager->getNumZones() );
   mZoneOverlap.clear();
   mIsInteriorOnly = true;

   if ( mChildren[0] == NULL )
   {
      Box3F worldBounds( mBounds );
      mTerrain->getTransform().mul( worldBounds );

      Vector<U32> zones;
      zoneManager->findZones( worldBounds, zones );

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
      TerrCell *cell = mChildren[i];
      cell->updateZoning( zoneManager );
      mZoneOverlap.combineOR( cell->getZoneOverlap() );
      mIsInteriorOnly &= cell->mIsInteriorOnly;
   }
}

void TerrCell::cullCells(  const SceneRenderState *state,
                           const Point3F &objLodPos,
                           Vector<TerrCell*> *outCells  )
{
   // If we have a VB and no children then just add 
   // ourselves to the results and return.
   if ( mVertexBuffer.isValid() && !mChildren[0]  )               
   {
      outCells->push_back( this );
      return;
   }

   const F32 screenError = mTerrain->getScreenError();
   const BitVector &zoneState = state->getCullingState().getZoneVisibilityFlags();

   for ( U32 i = 0; i < 4; i++ )
   {
      TerrCell *cell = mChildren[i];

      // Test cell visibility for interior zones.
      
      const bool visibleInside = !cell->getZoneOverlap().empty() ? zoneState.testAny( cell->getZoneOverlap() ) : false;

      // Test cell visibility for outdoor zone, but only
      // if we need to.

      bool visibleOutside = false;
      if( !mIsInteriorOnly && !visibleInside )
      {         
         U32 outdoorZone = SceneZoneSpaceManager::RootZoneId;
         visibleOutside = !state->getCullingState().isCulled( cell->mOBB, &outdoorZone, 1 );
      }

      // Skip cell if neither visible indoors nor outdoors.

      if( !visibleInside && !visibleOutside )
         continue;

      // Lod based on screen error...
      // If far enough, just add this child cells vb ( skipping its children ).
      F32 dist = cell->getDistanceTo( objLodPos );
      F32 errorMeters = ( cell->mSize / smMinCellSize ) * mTerrain->getSquareSize();
      U32 errorPixels = mCeil( state->projectRadius( dist, errorMeters ) );

      if ( errorPixels < screenError )
      {
         if ( cell->mVertexBuffer.isValid() )
            outCells->push_back( cell );       
      }
      else      
         cell->cullCells( state, objLodPos, outCells );
   }
}

void TerrCell::getRenderPrimitive(  GFXPrimitive *prim,
                                    GFXVertexBufferHandleBase *vertBuff,
                                    GFXPrimitiveBufferHandle  *primBuff ) const
{
	*vertBuff = mVertexBuffer;

   // Only supply a primitive buffer if we're using our own
   // due to empty squares.
   bool useStaticPrimBuffer = true;
   if ( mPrimBuffer.isValid() )
   {
      useStaticPrimBuffer = false;
      *primBuff = mPrimBuffer;
   }
      
	prim->type = GFXTriangleList;
	prim->startVertex = 0;
	prim->minIndex = 0;
	prim->startIndex = 0;
	prim->numVertices = smVBSize;

   if ( useStaticPrimBuffer )
   {
      // Use the standard primitive buffer count
      prim->numPrimitives = smTriCount;
   }
   else
   {
      // Use our triangle count that matches out primitive buffer
      prim->numPrimitives = mTriCount;
   }
}

void TerrCell::renderBounds() const
{
   ColorI color;
   color.interpolate( ColorI::RED, ColorI::GREEN, (F32)mLevel / 3.0f );

   GFXStateBlockDesc desc;
   desc.setZReadWrite( true, false );
   desc.fillMode = GFXFillWireframe;
   
   GFX->getDrawUtil()->drawCube( desc, mBounds, color );
}

void TerrCell::preloadMaterials()
{
   PROFILE_SCOPE( TerrCell_PreloadMaterials );

   // If we have a VB then we need a material.
   if ( mVertexBuffer.isValid() )
   {
      TerrainCellMaterial *material = getMaterial();
      material->getReflectMat();

      if (  GFX->getPixelShaderVersion() > 2.0f && 
            dStrcmp( LIGHTMGR->getId(), "BLM" ) != 0)
         material->getPrePassMat();
   }

   for ( U32 i = 0; i < 4; i++ )
      if ( mChildren[i] ) 
         mChildren[i]->preloadMaterials();
}

TerrainCellMaterial* TerrCell::getMaterial()
{
   if ( !mMaterial )
   {
      mMaterial = new TerrainCellMaterial;
      mMaterial->init( mTerrain, mMaterials );
   }

   return mMaterial;
}

void TerrCell::deleteMaterials()
{
   SAFE_DELETE( mMaterial );

   for ( U32 i = 0; i < 4; i++ )
      if ( mChildren[i] ) 
         mChildren[i]->deleteMaterials();
}
