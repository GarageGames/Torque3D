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
#include "forest/ts/tsForestCellBatch.h"

#include "forest/ts/tsForestItemData.h"
#include "scene/sceneManager.h"
#include "ts/tsLastDetail.h"


TSForestCellBatch::TSForestCellBatch( TSLastDetail *detail )
   :  mDetail( detail )
{
}

TSForestCellBatch::~TSForestCellBatch()
{
}

bool TSForestCellBatch::_prepBatch( const ForestItem &item )
{
   // Make sure it's our item type!
   TSForestItemData* data = dynamic_cast<TSForestItemData*>( item.getData() );
   if ( !data )
      return false;

   // TODO: Eventually we should atlas multiple details into
   // a single combined texture map.  Till then we have to
   // do one batch per-detail type.

   // If the detail type doesn't match then 
   // we need to start a new batch.
   if ( data->getLastDetail() != mDetail )
      return false;

   return true;
}

void TSForestCellBatch::_rebuildBatch()
{
   // Clean up first.
   mVB = NULL;
   if ( mItems.empty() )
      return;

   // How big do we need to make this?
   U32 verts = mItems.size() * 6;
   mVB.set( GFX, verts, GFXBufferTypeStatic );
   if ( !mVB.isValid() )
   {
      // If we failed it is probably because we requested
      // a size bigger than a VB can be.  Warn the user.
      AssertWarn( false, "TSForestCellBatch::_rebuildBatch: Batch too big... try reducing the forest cell size!" );
      return;
   }

   // Fill this puppy!
   ImposterState *vertPtr = mVB.lock();
   Vector<ForestItem>::const_iterator item = mItems.begin();

   const F32 radius = mDetail->getRadius();
   ImposterState state;

   for ( ; item != mItems.end(); item++ )
   {
      item->getWorldBox().getCenter( &state.center );
      state.halfSize = radius * item->getScale();
      state.alpha = 1.0f;
      item->getTransform().getColumn( 2, &state.upVec );
      item->getTransform().getColumn( 0, &state.rightVec );

      *vertPtr = state;
      vertPtr->corner = 0;
      ++vertPtr;

      *vertPtr = state;
      vertPtr->corner = 1;
      ++vertPtr;

      *vertPtr = state;
      vertPtr->corner = 2;
      ++vertPtr;

      *vertPtr = state;
      vertPtr->corner = 2;
      ++vertPtr;

      *vertPtr = state;
      vertPtr->corner = 3;
      ++vertPtr;

      *vertPtr = state;
      vertPtr->corner = 0;
      ++vertPtr;
   }

   mVB.unlock();
}

void TSForestCellBatch::_render( const SceneRenderState *state )
{
   if (  !mVB.isValid() || 
         ( state->isShadowPass() && !TSLastDetail::smCanShadow ) )
      return;

   // Make sure we have a material to render with.
   BaseMatInstance *mat = state->getOverrideMaterial( mDetail->getMatInstance() );
   if ( mat == NULL )
      return;

   // We don't really render here... we submit it to
   // the render manager which collects all the batches
   // in the scene, sorts them by texture, sets up the
   // shader, and then renders them all at once.

   ImposterBatchRenderInst *inst = state->getRenderPass()->allocInst<ImposterBatchRenderInst>();
   inst->mat = mat;
   inst->vertBuff = &mVB;

   // We sort by the imposter type first so that RIT_Imposter and 
   // RIT_ImposterBatches do not get mixed together.
   //
   // We then sort by material.
   //
   inst->defaultKey = 0;
   inst->defaultKey2 = mat->getStateHint();

   state->getRenderPass()->addInst( inst );
}
