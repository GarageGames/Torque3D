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
#include "renderInstance/renderMeshMgr.h"

#include "console/consoleTypes.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxPrimitiveBuffer.h"
#include "materials/sceneData.h"
#include "materials/processedMaterial.h"
#include "materials/materialManager.h"
#include "scene/sceneRenderState.h"
#include "gfx/gfxDebugEvent.h"
#include "math/util/matrixSet.h"


IMPLEMENT_CONOBJECT(RenderMeshMgr);

ConsoleDocClass( RenderMeshMgr, 
   "@brief A render bin for mesh rendering.\n\n"
   "This is the primary render bin in Torque which does most of the "
   "work of rendering DTS shapes and arbitrary mesh geometry.  It knows "
   "how to render mesh instances using materials and supports hardware mesh "
   "instancing.\n\n"
   "@ingroup RenderBin\n" );


RenderMeshMgr::RenderMeshMgr()
: RenderBinManager(RenderPassManager::RIT_Mesh, 1.0f, 1.0f)
{
}

RenderMeshMgr::RenderMeshMgr(RenderInstType riType, F32 renderOrder, F32 processAddOrder)
   : RenderBinManager(riType, renderOrder, processAddOrder)
{
}

void RenderMeshMgr::init()
{
   GFXStateBlockDesc d;

   d.cullDefined = true;
   d.cullMode = GFXCullCCW;
   d.samplersDefined = true;
   d.samplers[0] = GFXSamplerStateDesc::getWrapLinear();

   mNormalSB = GFX->createStateBlock(d);

   d.cullMode = GFXCullCW;
   mReflectSB = GFX->createStateBlock(d);
}

void RenderMeshMgr::initPersistFields()
{
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------
// add element
//-----------------------------------------------------------------------------
void RenderMeshMgr::addElement( RenderInst *inst )
{
   // If this instance is translucent handle it in RenderTranslucentMgr
   if (inst->translucentSort)
      return;

   AssertFatal( inst->defaultKey != 0, "RenderMeshMgr::addElement() - Got null sort key... did you forget to set it?" );

   internalAddElement(inst);
}

//-----------------------------------------------------------------------------
// render
//-----------------------------------------------------------------------------
void RenderMeshMgr::render(SceneRenderState * state)
{
   PROFILE_SCOPE(RenderMeshMgr_render);

   // Early out if nothing to draw.
   if(!mElementList.size())
      return;


   GFXDEBUGEVENT_SCOPE( RenderMeshMgr_Render, ColorI::GREEN );

   // Automagically save & restore our viewport and transforms.
   GFXTransformSaver saver;

   // Restore transforms
   MatrixSet &matrixSet = getRenderPass()->getMatrixSet();
   matrixSet.restoreSceneViewProjection();

   // init loop data
   GFXTextureObject *lastLM = NULL;
   GFXCubemap *lastCubemap = NULL;
   GFXTextureObject *lastReflectTex = NULL;
   GFXTextureObject *lastMiscTex = NULL;
   GFXTextureObject *lastAccuTex = NULL;

   SceneData sgData;
   sgData.init( state );

   U32 binSize = mElementList.size();

   for( U32 j=0; j<binSize; )
   {
      MeshRenderInst *ri = static_cast<MeshRenderInst*>(mElementList[j].inst);

      setupSGData( ri, sgData );
      BaseMatInstance *mat = ri->matInst;

      // If we have an override delegate then give it a 
      // chance to swap the material with another.
      if ( mMatOverrideDelegate )
      {
         mat = mMatOverrideDelegate( mat );
         if ( !mat )
         {
            j++;
            continue;
         }
      }

      if( !mat )
         mat = MATMGR->getWarningMatInstance();

      // Check if bin is disabled in advanced lighting.
      // Allow forward rendering pass on custom materials.

      if ( ( MATMGR->getPrePassEnabled() && mBasicOnly && !mat->isCustomMaterial() ) )
      {
         j++;
         continue;
      }

      U32 matListEnd = j;
      lastMiscTex = sgData.miscTex;
      U32 a;

      while( mat && mat->setupPass(state, sgData ) )
      {
         for( a=j; a<binSize; a++ )
         {
            MeshRenderInst *passRI = static_cast<MeshRenderInst*>(mElementList[a].inst);

            // Check to see if we need to break this batch.
            if (  newPassNeeded( ri, passRI ) ||
                  lastMiscTex != passRI->miscTex )
            {
               lastLM = NULL;
               break;
            }

            matrixSet.setWorld(*passRI->objectToWorld);
            matrixSet.setView(*passRI->worldToCamera);
            matrixSet.setProjection(*passRI->projection);
            mat->setTransforms(matrixSet, state);

            setupSGData( passRI, sgData );
            mat->setSceneInfo( state, sgData );

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

            // TODO: This could proably be done in a cleaner way.
            //
            // This section of code is dangerous, it overwrites the
            // lightmap values in sgData.  This could be a problem when multiple
            // render instances use the same multi-pass material.  When
            // the first pass is done, setupPass() is called again on
            // the material, but the lightmap data has been changed in
            // sgData to the lightmaps in the last renderInstance rendered.

            // This section sets the lightmap data for the current batch.
            // For the first iteration, it sets the same lightmap data,
            // however the redundancy will be caught by GFXDevice and not
            // actually sent to the card.  This is done for simplicity given
            // the possible condition mentioned above.  Better to set always
            // than to get bogged down into special case detection.
            //-------------------------------------
            bool dirty = false;

            // set the lightmaps if different
            if( passRI->lightmap && passRI->lightmap != lastLM )
            {
               sgData.lightmap = passRI->lightmap;
               lastLM = passRI->lightmap;
               dirty = true;
            }

            // set the cubemap if different.
            if ( passRI->cubemap != lastCubemap )
            {
               sgData.cubemap = passRI->cubemap;
               lastCubemap = passRI->cubemap;
               dirty = true;
            }

            if ( passRI->reflectTex != lastReflectTex )
            {
               sgData.reflectTex = passRI->reflectTex;
               lastReflectTex = passRI->reflectTex;
               dirty = true;
            }

            // Update accumulation texture if it changed.
            // Note: accumulation texture can be NULL, and must be updated.
            if ( passRI->accuTex != lastAccuTex )
            {
               sgData.accuTex = passRI->accuTex;
               lastAccuTex = lastAccuTex;
               dirty = true;
            }

            if ( dirty )
               mat->setTextureStages( state, sgData );

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

