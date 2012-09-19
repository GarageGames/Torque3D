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

#include "forest/forest.h"
#include "forest/forestCell.h"
#include "forest/forestDataFile.h"

#include "gfx/gfxTransformSaver.h"
#include "renderInstance/renderPassManager.h"
#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "lighting/lightManager.h"
#include "ts/tsMesh.h"
#include "ts/tsRenderState.h"
#include "ts/tsShapeInstance.h"

#include "gfx/primBuilder.h"
#include "gfx/gfxDrawUtil.h"
#include "math/mathUtils.h"


U32   Forest::smTotalCells = 0;
U32   Forest::smCellsRendered = 0;
U32   Forest::smCellItemsRendered = 0;
U32   Forest::smCellsBatched = 0;
U32   Forest::smCellItemsBatched = 0;
F32   Forest::smAverageItemsPerCell = 0.0f;

void Forest::_clearStats(bool beginFrame)
{
   // Reset the rendering stats!
   if (beginFrame)
   {
      smTotalCells = 0;
      smCellsRendered = 0;
      smCellItemsRendered = 0;
      smCellsBatched = 0;
      smCellItemsBatched = 0;
      smAverageItemsPerCell = 0.0f;
   }
}

void Forest::prepRenderImage( SceneRenderState *state )
{
   PROFILE_SCOPE(Forest_RenderCells);

   // TODO: Fix stats.
   /*
   ForestCellVector &theCells = mData->getCells();
   smTotalCells += theCells.size();

   // Don't render if we don't have a grid!
   if ( theCells.empty() )
      return false;
   */

   // Prepare to render.
   GFXTransformSaver saver;

   // Figure out the grid range in the viewing area.
   const bool isReflectPass = state->isReflectPass();

   const F32 cullScale = isReflectPass ? mReflectionLodScalar : 1.0f;

   // If we need to update our cached 
   // zone state then do it now.
   if ( mZoningDirty )
   {
      mZoningDirty = false;

      Vector<ForestCell*> cells;
      mData->getCells(  &cells );
      for ( U32 i=0; i < cells.size(); i++ )
         cells[i]->_updateZoning( getSceneManager()->getZoneManager() );
   }

   // TODO: Move these into the TSForestItemData as something we
   // setup once and don't do per-instance.
   
   // Set up the TS render state.
   TSRenderState rdata;
   rdata.setSceneState( state );

   // Use origin sort on all forest elements as
   // its alot cheaper than the bounds sort.
   rdata.setOriginSort( true );

   // We may have some forward lit materials in
   // the forest, so pass down a LightQuery for it.
   LightQuery lightQuery;
   rdata.setLightQuery( &lightQuery );
   Frustum culler = state->getFrustum();

   // Adjust the far distance if the cull scale has changed.
   if ( !mIsEqual( cullScale, 1.0f ) )
   {
      const F32 visFarDist = culler.getFarDist() * cullScale;
      culler.setFarDist( visFarDist );
   }

   Box3F worldBox;

   // Used for debug drawing.
   GFXDrawUtil* drawer = GFX->getDrawUtil();
   drawer->clearBitmapModulation();

   // Go thru the visible cells.
   const Box3F &cullerBounds = culler.getBounds();
   const Point3F &camPos = state->getDiffuseCameraPosition();
   
   U32 clipMask;
   smAverageItemsPerCell = 0.0f;
   U32 cellsProcessed = 0;
   ForestCell *cell;
   
   // First get all the top level cells which 
   // intersect the frustum.
   Vector<ForestCell*> cellStack;
   mData->getCells( culler, &cellStack );

   // Get the culling zone state.
   const BitVector &zoneState = state->getCullingState().getZoneVisibilityFlags();

   // Now loop till we run out of cells.
   while ( !cellStack.empty() )
   {
      // Pop off the next cell.
      cell = cellStack.last();
      cellStack.pop_back();

      const Box3F &cellBounds = cell->getBounds();

      // If the cell is empty or its bounds is outside the frustum
      // bounds then we have nothing nothing more to do.
      if ( cell->isEmpty() || !cullerBounds.isOverlapped( cellBounds ) )
         continue;

      // Can we cull this cell entirely?
      clipMask = culler.testPlanes( cellBounds, Frustum::PlaneMaskAll );
      if ( clipMask == -1 )
         continue;

      // Test cell visibility for interior zones.      
      const bool visibleInside = !cell->getZoneOverlap().empty() ? zoneState.testAny( cell->getZoneOverlap() ) : false;

      // Test cell visibility for outdoor zone, but only
      // if we need to.
      bool visibleOutside = false;
      if( !cell->mIsInteriorOnly && !visibleInside )
      {         
         U32 outdoorZone = SceneZoneSpaceManager::RootZoneId;
         visibleOutside = !state->getCullingState().isCulled( cellBounds, &outdoorZone, 1 );
      }

      // Skip cell if neither visible indoors nor outdoors.
      if( !visibleInside && !visibleOutside )
         continue;

      // Update the stats.
      smAverageItemsPerCell += cell->getItems().size();
      ++cellsProcessed;
      //if ( cell->isLeaf() )
         //++leafCellsProcessed;

      // Get the distance from the camera to the cell bounds.
      F32 dist = cellBounds.getDistanceToPoint( camPos );

      // If the largest item in the cell can be billboarded
      // at the cell distance to the camera... then the whole
      // cell can be billboarded.
      //
      if (  smForceImposters || 
            ( dist > 0.0f && cell->getLargestItem().canBillboard( state, dist ) ) )
      {
         // If imposters are disabled then skip out.
         if ( smDisableImposters )
            continue;

         PROFILE_SCOPE(Forest_RenderBatches);

         // Keep track of how many cells were batched.
         ++smCellsBatched;

         // Ok... everything in this cell should be batched.  First
         // create the batches if we don't have any.
         if ( !cell->hasBatches() )
            cell->buildBatches();

         //if ( drawCells )
            //mCellRenderFlag[ cellIter - theCells.begin() ] = 1;

         // TODO: Light queries for batches?

         // Now render the batches... we pass the culler if the
         // cell wasn't fully visible so that each batch can be culled.
         smCellItemsBatched += cell->renderBatches( state, clipMask != 0 ? &culler : NULL );
         continue;
      }

      // If this isn't a leaf then recurse.
      if ( !cell->isLeaf() )
      {
         cell->getChildren( &cellStack );
         continue;
      }

      // This cell has mixed billboards and mesh based items.
      ++smCellsRendered;

      PROFILE_SCOPE(Forest_RenderItems);

      //if ( drawCells )
         //mCellRenderFlag[ cellIter - theCells.begin() ] = 2;

      // Use the cell bounds as the light query volume.
      //
      // This means all forward lit items in this cell will 
      // get the same lights, but it performs much better.
      lightQuery.init( cellBounds );

      // This cell is visible... have it render its items.
      smCellItemsRendered += cell->render( &rdata, clipMask != 0 ? &culler : NULL );
   }

   // Keep track of the average items per cell.
   if ( cellsProcessed > 0 )
      smAverageItemsPerCell /= (F32)cellsProcessed;

   // Got debug drawing to do?
   if ( smDrawCells && state->isDiffusePass() )
   {
      ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
      ri->renderDelegate.bind( this, &Forest::_renderCellBounds );
      ri->type = RenderPassManager::RIT_Editor;
      state->getRenderPass()->addInst( ri );
   }
}


void Forest::_renderCellBounds( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat )
{
   PROFILE_SCOPE( Forest_RenderCellBounds );

   if ( overrideMat )
      return;

   GFXTransformSaver saver;

   MatrixF projBias(true);
   const Frustum frustum = GFX->getFrustum();
   MathUtils::getZBiasProjectionMatrix( 0.001f, frustum, &projBias );
   GFX->setProjectionMatrix( projBias );

   VectorF extents;
   Point3F pos;

   // Get top level cells
   Vector<ForestCell*> cellStack;
   mData->getCells( &cellStack );

   // Holds child cells we need to render as we encounter them.
   Vector<ForestCell*> frontier;   
   
   GFXDrawUtil *drawer = GFX->getDrawUtil();

   GFXStateBlockDesc desc;
   desc.setZReadWrite( true, false );
   desc.setBlend( true );
   desc.setFillModeWireframe();

   while ( !cellStack.empty() )
   {
      while ( !cellStack.empty() )
      {      
         const ForestCell *cell = cellStack.last();
         cellStack.pop_back();
         
         Box3F box = cell->getBounds();

         drawer->drawCube( desc, box, ColorI( 0, 255, 0 ) );

         RectF rect = cell->getRect();

         box.minExtents.set( rect.point.x, rect.point.y, box.minExtents.z );
         box.maxExtents.set( rect.point.x + rect.extent.x, rect.point.y + rect.extent.y, box.minExtents.z );         

         drawer->drawCube( desc, box, ColorI::RED );

         // If this cell has children, add them to the frontier.
         if ( !cell->isLeaf() )      
            cell->getChildren( &frontier );      
      }

      // Now the frontier becomes the cellStack and we empty the frontier.
      cellStack = frontier;
      frontier.clear();
   }
}
