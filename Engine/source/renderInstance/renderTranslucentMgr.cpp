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
#include "renderInstance/renderTranslucentMgr.h"

#include "materials/sceneData.h"
#include "scene/sceneManager.h"
#include "scene/sceneObject.h"
#include "scene/sceneRenderState.h"
#include "materials/matInstance.h"
#include "gfx/gfxPrimitiveBuffer.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDebugEvent.h"
#include "renderInstance/renderParticleMgr.h"
#include "math/util/matrixSet.h"

#define HIGH_NUM ((U32(-1)/2) - 1)

IMPLEMENT_CONOBJECT(RenderTranslucentMgr);

ConsoleDocClass( RenderTranslucentMgr, 
   "@brief A render bin for rendering translucent meshes.\n\n"
   "This bin is used to render translucent render mesh instances and render object "
   "instances. It is generally ordered late in the RenderPassManager after all opaque "
   "geometry bins.\n\n"
   "@ingroup RenderBin\n" );


RenderTranslucentMgr::RenderTranslucentMgr()
   :  RenderBinManager( RenderPassManager::RIT_Translucent, 1.0f, 1.0f ), mParticleRenderMgr(NULL)
{
   notifyType( RenderPassManager::RIT_ObjectTranslucent );
   notifyType( RenderPassManager::RIT_Particle );
   notifyType( RenderPassManager::RIT_VolumetricFog);
}

RenderTranslucentMgr::~RenderTranslucentMgr()
{
}

void RenderTranslucentMgr::setupSGData(MeshRenderInst *ri, SceneData &data )
{
   Parent::setupSGData( ri, data );

   // We do not support these in the translucent bin.
   data.backBuffTex = NULL;
   data.cubemap = NULL;   
   data.lightmap = NULL;
}

void RenderTranslucentMgr::addElement( RenderInst *inst )
{
   // Right off the bat if its not translucent skip it.
   if ( !inst->translucentSort )
      return;

   // What type of instance is this.
   const bool isMeshInst = inst->type == RenderPassManager::RIT_Translucent;

   // Get its material if its a mesh.
   BaseMatInstance* matInst = NULL;
   if ( isMeshInst )
      matInst = static_cast<MeshRenderInst*>( inst )->matInst;

   // If the material isn't translucent the skip it.
   if ( matInst && !matInst->getMaterial()->isTranslucent() )
      return;

   // We made it this far, add the instance.
   mElementList.increment();
   MainSortElem& elem = mElementList.last();
   elem.inst = inst;

   // Override the instances default key to be the sort distance. All 
   // the pointer dereferencing is in there to prevent us from losing
   // information when converting to a U32.
   elem.key = *((U32*)&inst->sortDistSq);

   AssertFatal( inst->defaultKey != 0, "RenderTranslucentMgr::addElement() - Got null sort key... did you forget to set it?" );

   // Then use the instances primary key as our secondary key
   elem.key2 = inst->defaultKey;
}

GFXStateBlockRef RenderTranslucentMgr::_getStateBlock( U8 transFlag )
{
   if ( mStateBlocks[transFlag].isValid() )
      return mStateBlocks[transFlag];

   GFXStateBlockDesc d;

   d.cullDefined = true;
   d.cullMode = GFXCullNone;
   d.blendDefined = true;
   d.blendEnable = true;
   d.blendSrc = (GFXBlend)((transFlag >> 4) & 0x0f); 
   d.blendDest = (GFXBlend)(transFlag & 0x0f);
   d.alphaDefined = true;
   
   // See http://www.garagegames.com/mg/forums/result.thread.php?qt=81397
   d.alphaTestEnable = (d.blendSrc == GFXBlendSrcAlpha && (d.blendDest == GFXBlendInvSrcAlpha || d.blendDest == GFXBlendOne));
   d.alphaTestRef = 1;
   d.alphaTestFunc = GFXCmpGreaterEqual;
   d.zDefined = true;
   d.zWriteEnable = false;
   d.samplersDefined = true;
   d.samplers[0] = GFXSamplerStateDesc::getClampLinear();
   d.samplers[0].alphaOp = GFXTOPModulate;
   d.samplers[0].alphaArg1 = GFXTATexture;
   d.samplers[0].alphaArg2 = GFXTADiffuse;

   mStateBlocks[transFlag] = GFX->createStateBlock(d);
   return mStateBlocks[transFlag];
}

void RenderTranslucentMgr::render( SceneRenderState *state )
{
   PROFILE_SCOPE(RenderTranslucentMgr_render);   

   // Early out if nothing to draw.
   if(!mElementList.size())
      return;

   GFXDEBUGEVENT_SCOPE(RenderTranslucentMgr_Render, ColorI::BLUE);

   // Find the particle render manager (if we don't have it)
   if(mParticleRenderMgr == NULL)
   {
      RenderPassManager *rpm = state->getRenderPass();
      for( U32 i = 0; i < rpm->getManagerCount(); i++ )
      {
         RenderBinManager *bin = rpm->getManager(i);
         if( bin->getRenderInstType() == RenderParticleMgr::RIT_Particles )
         {
            mParticleRenderMgr = reinterpret_cast<RenderParticleMgr *>(bin);
            break;
         }
      }
   }

   GFXTransformSaver saver;   

   SceneData sgData;
   sgData.init( state );

   GFXVertexBuffer * lastVB = NULL;
   GFXPrimitiveBuffer * lastPB = NULL;

   // Restore transforms
   MatrixSet &matrixSet = getRenderPass()->getMatrixSet();
   matrixSet.restoreSceneViewProjection();

   U32 binSize = mElementList.size();
   for( U32 j=0; j<binSize; )
   {
      RenderInst *baseRI = mElementList[j].inst;
      
      U32 matListEnd = j;

      // render these separately...
      if ( baseRI->type == RenderPassManager::RIT_ObjectTranslucent )
      {
         ObjectRenderInst* objRI = static_cast<ObjectRenderInst*>(baseRI);
         objRI->renderDelegate( objRI, state, NULL );         

         lastVB = NULL;
         lastPB = NULL;
         j++;
         continue;
      }
      else if (baseRI->type == RenderPassManager::RIT_VolumetricFog)
	   {
	      ObjectRenderInst* objRI = static_cast<ObjectRenderInst*>(baseRI);
	      objRI->renderDelegate(objRI, state, NULL);
	      lastVB = NULL;
	      lastPB = NULL;
	      j++;
	      continue;
	   }
      else if ( baseRI->type == RenderPassManager::RIT_Particle )
      {
         ParticleRenderInst *ri = static_cast<ParticleRenderInst*>(baseRI);

         // Tell Particle RM to draw the system. (This allows the particle render manager
         // to manage drawing offscreen particle systems, and allows the systems
         // to be composited back into the scene with proper translucent
         // sorting order)
         mParticleRenderMgr->renderInstance(ri, state);

         lastVB = NULL;    // no longer valid, null it
         lastPB = NULL;    // no longer valid, null it

         j++;
         continue;
      }
      else if ( baseRI->type == RenderPassManager::RIT_Translucent )
      {
         MeshRenderInst* ri = static_cast<MeshRenderInst*>(baseRI);
         BaseMatInstance *mat = ri->matInst;

         setupSGData( ri, sgData );

         while( mat->setupPass( state, sgData ) )
         {
            U32 a;
            for( a=j; a<binSize; a++ )
            {
               RenderInst* nextRI = mElementList[a].inst;
               if ( nextRI->type != RenderPassManager::RIT_Translucent )
                  break;

               MeshRenderInst *passRI = static_cast<MeshRenderInst*>(nextRI);

               // Check to see if we need to break this batch.
               if ( newPassNeeded( ri, passRI ) )
                  break;

               // Z sorting and stuff is still not working in this mgr...
               setupSGData( passRI, sgData );
               mat->setSceneInfo(state, sgData);
               matrixSet.setWorld(*passRI->objectToWorld);
               matrixSet.setView(*passRI->worldToCamera);
               matrixSet.setProjection(*passRI->projection);
               mat->setTransforms(matrixSet, state);

               // Setup HW skinning transforms if applicable
               if (mat->usesHardwareSkinning())
               {
                  mat->setNodeTransforms(passRI->mNodeTransforms, passRI->mNodeTransformCount);
               }

               // If we're instanced then don't render yet.
               if ( mat->isInstanced() )
               {
                  // Let the material increment the instance buffer, but
                  // break the batch if it runs out of room for more.
                  if ( !mat->stepInstance() )
                  {
                     a++;
                     break;
                  }

                  continue;
               }

               // Setup the vertex and index buffers.
               mat->setBuffers( passRI->vertBuff, passRI->primBuff );

               // Render this sucker.
               if ( passRI->prim )   
                  GFX->drawPrimitive( *passRI->prim );   
               else  
                  GFX->drawPrimitive( passRI->primBuffIndex );              
            }

            // Draw the instanced batch.
            if ( mat->isInstanced() )
            {
               // Sets the buffers including the instancing stream.
               mat->setBuffers( ri->vertBuff, ri->primBuff );

               // Render the instanced stream.
               if ( ri->prim )
                  GFX->drawPrimitive( *ri->prim );
               else
                  GFX->drawPrimitive( ri->primBuffIndex );
            }

            matListEnd = a;
         }

         // force increment if none happened, otherwise go to end of batch
         j = ( j == matListEnd ) ? j+1 : matListEnd;
      }
   }
}