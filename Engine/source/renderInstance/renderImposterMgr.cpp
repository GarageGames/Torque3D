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
#include "renderInstance/renderImposterMgr.h"

#include "scene/sceneManager.h"
#include "T3D/gameBase/gameConnection.h"
#include "materials/shaderData.h"
#include "lighting/lightManager.h"
#include "lighting/lightInfo.h"
#include "scene/sceneRenderState.h"
#include "gfx/gfxDebugEvent.h"
#include "renderInstance/renderPrePassMgr.h"
#include "gfx/gfxTransformSaver.h"
#include "console/consoleTypes.h"
#include "gfx/util/screenspace.h"
#include "math/util/matrixSet.h"
#include "materials/materialManager.h"
#include "materials/materialFeatureTypes.h"

/*
GFXImplementVertexFormat( ImposterCorner )
{
   addElement( "ImposterCorner", GFXDeclType_Float, 4 );
};
*/

const RenderInstType RenderImposterMgr::RIT_Imposter( "Imposter" );
const RenderInstType RenderImposterMgr::RIT_ImposterBatch( "ImposterBatch" );


U32 RenderImposterMgr::smRendered = 0.0f;
U32 RenderImposterMgr::smBatches = 0.0f;
U32 RenderImposterMgr::smDrawCalls = 0.0f;
U32 RenderImposterMgr::smPolyCount = 0.0f;
U32 RenderImposterMgr::smRTChanges = 0.0f;


IMPLEMENT_CONOBJECT(RenderImposterMgr);

ConsoleDocClass( RenderImposterMgr, 
   "@brief A render bin for batch rendering imposters.\n\n"
   "This render bin gathers imposter render instances and renders them in large "
   "batches.\n\n"
   "You can type 'metrics( imposter )' in the console to see rendering statistics.\n\n"
   "@ingroup RenderBin\n" );


RenderImposterMgr::RenderImposterMgr( F32 renderOrder, F32 processAddOrder )
   :  RenderBinManager( RIT_Imposter, renderOrder, processAddOrder )
{
   notifyType( RIT_ImposterBatch );
   RenderPrePassMgr::getRenderSignal().notify( this, &RenderImposterMgr::_renderPrePass );
}

void RenderImposterMgr::initPersistFields()
{
   GFXDevice::getDeviceEventSignal().notify( &RenderImposterMgr::_clearStats );

   Con::addVariable( "$ImposterStats::rendered", TypeS32, &smRendered, "@internal" );
   Con::addVariable( "$ImposterStats::batches", TypeS32, &smBatches, "@internal" );
   Con::addVariable( "$ImposterStats::drawCalls", TypeS32, &smDrawCalls, "@internal" );
   Con::addVariable( "$ImposterStats::polyCount", TypeS32, &smPolyCount, "@internal" );
   Con::addVariable( "$ImposterStats::rtChanges", TypeS32, &smRTChanges, "@internal" );

   Parent::initPersistFields();
}

RenderImposterMgr::~RenderImposterMgr()
{
   RenderPrePassMgr::getRenderSignal().remove( this, &RenderImposterMgr::_renderPrePass );

   mIB = NULL;
}

void RenderImposterMgr::render( SceneRenderState *state )
{
   PROFILE_SCOPE( RenderImposterMgr_Render );

   if ( !mElementList.size() )
      return;

   GFXDEBUGEVENT_SCOPE( RenderImposterMgr_Render, ColorI::RED );

   _innerRender( state, NULL );
}

bool RenderImposterMgr::_clearStats( GFXDevice::GFXDeviceEventType type )
{
   if ( type == GFXDevice::deStartOfFrame )
   {
      smRendered = 0.0f;
      smBatches = 0.0f;
      smDrawCalls = 0.0f;
      smPolyCount = 0.0f;
      smRTChanges = 0.0f;
   }

   return true;
}

void RenderImposterMgr::_renderPrePass( const SceneRenderState *state, RenderPrePassMgr *prePassBin, bool startPrePass )
{
   PROFILE_SCOPE( RenderImposterMgr_RenderPrePass );

   if ( !mElementList.size() || !startPrePass )
      return;

   GFXDEBUGEVENT_SCOPE( RenderImposterMgr_RenderPrePass, ColorI::RED );

   _innerRender( state, prePassBin );
}

void RenderImposterMgr::_innerRender( const SceneRenderState *state, RenderPrePassMgr *prePassBin )
{
   PROFILE_SCOPE( RenderImposterMgr_InnerRender );

   // Capture the GFX stats for this render.
   GFXDeviceStatistics stats;
   stats.start( GFX->getDeviceStatistics() );

   GFXTransformSaver saver;

   // Restore transforms
   MatrixSet &matrixSet = getRenderPass()->getMatrixSet();
   matrixSet.restoreSceneViewProjection();
   matrixSet.setWorld( MatrixF::Identity );

   // Setup the large static index buffer for rendering the imposters.
   if ( !mIB.isValid() )
   {     
      // Setup a static index buffer for rendering.
      mIB.set( GFX, smImposterBatchSize * 6, 0, GFXBufferTypeStatic );
      U16 *idxBuff;
      mIB.lock(&idxBuff, NULL, NULL, NULL);
      for ( U32 i=0; i < smImposterBatchSize; i++ )
      {
         //
         // The vertex pattern in the VB for each 
         // imposter is as follows...
         //
         //     0----1
         //     |\   |
         //     | \  |
         //     |  \ |
         //     |   \|
         //     3----2
         //
         // We setup the index order below to ensure
         // sequental, cache friendly, access.
         //
         U32 offset = i * 4;
         idxBuff[i*6+0] = 0 + offset;
         idxBuff[i*6+1] = 1 + offset;
         idxBuff[i*6+2] = 2 + offset;
         idxBuff[i*6+3] = 2 + offset;
         idxBuff[i*6+4] = 3 + offset;
         idxBuff[i*6+5] = 0 + offset;
      }
      mIB.unlock();
   }

   /*
   if ( !mCornerVB.isValid() )
   {
      // Setup a static vertex buffer for the corner index for each imposter state.
      mCornerVB.set( GFX, smImposterBatchSize * 4, GFXBufferTypeStatic );
      ImposterCorner *corner = mCornerVB.lock( 0 );
      for ( U32 i=0; i < smImposterBatchSize; i++ )
      {
         corner->corner = 0; corner++;
         corner->corner = 1; corner++;
         corner->corner = 2; corner++;
         corner->corner = 3; corner++;
      }
      mCornerVB.unlock();
   }
   */

   // Set the buffers here once.
   GFX->setPrimitiveBuffer( mIB );

   // Batch up the imposters into the buffer.  These
   // are already sorted by texture, to minimize switches
   // so just batch them up and render as they come.

   ImposterState* statePtr = NULL;
   U32 stateCount;
   ImposterBaseRenderInst *ri;
   ImposterRenderInst *imposter;
   ImposterBatchRenderInst *batch;
   const U32 binSize = mElementList.size();
   BaseMatInstance *setupMat, *currMat;
   GFXVertexBufferHandle<ImposterState> vb;

   // TODO: We could maybe do better with lights when forward
   // rendering the imposters.  Just pass a light list with it
   // and do some simple tests to break the batch when the light
   // list changes.

   SceneData sgData;
   sgData.init( state, prePassBin ? SceneData::PrePassBin : SceneData::RegularBin );
   sgData.lights[0] = LIGHTMGR->getDefaultLight();

   // TODO: I should rework this loop to generate the VB first then
   // do all the material passes... should be easier to read and faster.
   //
   // Also consider making this store two element lists... one for
   // batches and one for individual imposters.
   //
      
   for ( U32 i=0; i < binSize; )
   {
      currMat = static_cast<ImposterBaseRenderInst*>( mElementList[i].inst )->mat;      
      setupMat = prePassBin ? prePassBin->getPrePassMaterial( currMat ) : currMat;

      // TODO: Fix MatInstance to take a const SceneRenderState!
      while ( setupMat->setupPass( (SceneRenderState*)state, sgData ) )
      {
         setupMat->setSceneInfo( (SceneRenderState*)state, sgData );
         setupMat->setTransforms( matrixSet, (SceneRenderState*)state );

         for ( ; i < binSize; )
         {
            ri = static_cast<ImposterBaseRenderInst*>( mElementList[i].inst );
            
            // NOTE: Its safe to compare matinstances here instead of
            // the state hint because imposters all share the same 
            // material instances.... if this changes revise.            
            if ( ri->mat != currMat )
               break;

            // Ok if this is a batch then we can just fire off the draw now.
            if ( ri->type == RIT_ImposterBatch )
            {
               batch = static_cast<ImposterBatchRenderInst*>( ri );

               GFX->setVertexBuffer( batch->vertBuff->getPointer() );
               GFX->drawPrimitive( GFXTriangleList, 0, batch->vertBuff->getPointer()->mNumVerts / 3 );
               
               i++;
               continue;
            }

            // This wasn't a batch so build up all the single imposters into
            // a dynamic batch and render it.

            statePtr = mBuffer;
            stateCount = 0;

            // Loop for each individual imposter.
            for ( ; i < binSize; i++ )
            {
               if ( mElementList[i].inst->type == RIT_ImposterBatch )
                  break;

               imposter = static_cast<ImposterRenderInst*>( mElementList[i].inst );

               // Stop the loop if the material changed.
               if ( imposter->mat != currMat )
                  break;

               ++smRendered;

               // If we're out of vb space then draw what we got.
               if ( stateCount + 1 >= smImposterBatchSize )
               {
                  smBatches++;
               
                  vb.set( GFX, stateCount*4, GFXBufferTypeVolatile );
                  dMemcpy( vb.lock(), mBuffer, stateCount * 4 * sizeof( ImposterState ) );
                  vb.unlock();
               
                  //GFX->setVertexBuffer( mCornerVB, 0, stateCount * 4 );
                  GFX->setVertexBuffer( vb );
                  ///GFX->setVertexFormat( &mImposterVertDecl );

                  GFX->drawIndexedPrimitive( GFXTriangleList, 0, 0, stateCount * 4, 0, stateCount * 2 );
                  statePtr = mBuffer;
                  stateCount = 0;
               }

               // Setup the imposter state.
               *statePtr = imposter->state;
               statePtr->corner = 0;
               statePtr++;

               *statePtr = imposter->state;
               statePtr->corner = 1;
               statePtr++;

               *statePtr = imposter->state;
               statePtr->corner = 2;
               statePtr++;

               *statePtr = imposter->state;
               statePtr->corner = 3;
               statePtr++;

               stateCount++;

            } // for ( ; i < binSize; i++ )

            // Any remainder to dump?
            if ( stateCount > 0 )
            {
               smBatches++;

               vb.set( GFX, stateCount*4, GFXBufferTypeVolatile );
               dMemcpy( vb.lock(), mBuffer, stateCount * 4 * sizeof( ImposterState ) );
               vb.unlock();
               
               //GFX->setVertexBuffer( mCornerVB, 0, stateCount * 4 );
               GFX->setVertexBuffer( vb );
               ///GFX->setVertexFormat( &mImposterVertDecl );

               GFX->drawIndexedPrimitive( GFXTriangleList, 0, 0, stateCount * 4, 0, stateCount * 2 );
           }

         } // for( U32 i=0; i < binSize; )

      } // while ( currMat->setupPass( (SceneRenderState*)state, sgData ) )

   } // for ( U32 i=0; i < binSize; )

   // Capture the GFX stats for this render.
   stats.end( GFX->getDeviceStatistics() );
   smDrawCalls += stats.mDrawCalls;
   smPolyCount += stats.mPolyCount;
   smRTChanges += stats.mRenderTargetChanges;
}
