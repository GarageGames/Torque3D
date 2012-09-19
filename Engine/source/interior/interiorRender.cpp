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

#include "interior/interior.h"
#include "scene/sceneRenderState.h"
#include "scene/sceneManager.h"
#include "lighting/lightManager.h"

#include "gfx/bitmap/gBitmap.h"
#include "math/mMatrix.h"
#include "math/mRect.h"
#include "core/bitVector.h"
#include "platform/profiler.h"
#include "gfx/gfxDevice.h"
#include "interior/interiorInstance.h"
#include "gfx/gfxTextureHandle.h"
#include "materials/materialList.h"
#include "materials/sceneData.h"
#include "materials/matInstance.h"
#include "materials/materialFeatureTypes.h"
#include "math/mathUtils.h"
#include "renderInstance/renderPassManager.h"
#include "core/frameAllocator.h"

extern bool sgFogActive;
extern U16* sgActivePolyList;
extern U32  sgActivePolyListSize;
extern U16* sgFogPolyList;
extern U32  sgFogPolyListSize;
extern U16* sgEnvironPolyList;
extern U32  sgEnvironPolyListSize;

Point3F sgOSCamPosition;


U32         sgRenderIndices[2048];
U32         csgNumAllowedPoints = 256;

extern "C" {
   F32   texGen0[8];
   F32   texGen1[8];
   Point2F *fogCoordinatePointer;
}


//------------------------------------------------------------------------------
// Set up render states for interor rendering
//------------------------------------------------------------------------------
void Interior::setupRenderStates()
{
   // GFX2_RENDER_MERGE
   // I suspect we don't need this anymore - MDF
   GFX->setStateBlock(mInteriorSB);
}

//------------------------------------------------------------------------------
// Setup zone visibility
//------------------------------------------------------------------------------
ZoneVisDeterminer Interior::setupZoneVis( InteriorInstance *intInst, SceneRenderState *state )
{

   U32 zoneOffset = intInst->getZoneRangeStart() != 0xFFFFFFFF ? intInst->getZoneRangeStart() : 0;

   U32 baseZone = 0xFFFFFFFF;

   if (intInst->_getNumCurrZones() == 1)
   {
      baseZone = intInst->_getCurrZone(0);
   }
   else
   {
      for (U32 i = 0; i < intInst->_getNumCurrZones(); i++)
      {
         if (state->getCullingState().getZoneState(intInst->_getCurrZone(i)).isZoneVisible())
         {
            if (baseZone == 0xFFFFFFFF) {
               baseZone = intInst->_getCurrZone(i);
               break;
            }
         }
      }
      if (baseZone == 0xFFFFFFFF)
      {
         baseZone = intInst->_getCurrZone(0);
      }
   }

   ZoneVisDeterminer zoneVis;
   zoneVis.runFromState(state, zoneOffset, baseZone);
   return zoneVis;
}

//------------------------------------------------------------------------------
// Setup scenegraph data structure for materials
//------------------------------------------------------------------------------
SceneData Interior::setupSceneGraphInfo( InteriorInstance *intInst,
                                              SceneRenderState *state )
{
   SceneData sgData;
   sgData.init( state );

   // TODO: This sucks... interiors only get sunlight?
   sgData.lights[0] = LIGHTMGR->getSpecialLight( LightManager::slSunLightType );
   sgData.objTrans = &intInst->getTransform();
   //sgData.backBuffTex = GFX->getSfxBackBuffer(); // NOTICE: SFXBB is removed and refraction is disabled!

   return sgData;
}

//------------------------------------------------------------------------------
// Render zone RenderNode
//------------------------------------------------------------------------------
void Interior::renderZoneNode( SceneRenderState *state,
                               RenderNode &node,
                               InteriorInstance *intInst,
                               SceneData &sgData,
                               MeshRenderInst *coreRi )
{
   BaseMatInstance *matInst = state->getOverrideMaterial( node.matInst );
   if ( !matInst )
      return;

   MeshRenderInst *ri = state->getRenderPass()->allocInst<MeshRenderInst>();
   *ri = *coreRi;

   ri->lights[0] = LIGHTMGR->getSpecialLight( LightManager::slSunLightType );

   // setup lightmap
   if (dStricmp(LIGHTMGR->getId(), "BLM") == 0)
   {
      if (  node.lightMapIndex != U8(-1) && 
            matInst->getFeatures().hasFeature( MFT_LightMap ) )
         ri->lightmap = gInteriorLMManager.getHandle(mLMHandle, intInst->getLMHandle(), node.lightMapIndex );
   }

   ri->matInst = matInst;
   ri->primBuffIndex = node.primInfoIndex;

   if ( matInst->getMaterial()->isTranslucent() )
   {
      ri->type = RenderPassManager::RIT_Translucent;
      ri->translucentSort = true;
      ri->sortDistSq = intInst->getRenderWorldBox().getSqDistanceToPoint( state->getCameraPosition() );
   }

   // Sort by the material then the normal map or vertex buffer!
   ri->defaultKey = matInst->getStateHint();
   ri->defaultKey2 = ri->lightmap ? (U32)ri->lightmap : (U32)ri->vertBuff;

   state->getRenderPass()->addInst( ri );
}

//------------------------------------------------------------------------------
// Render zone RenderNode
//------------------------------------------------------------------------------
void Interior::renderReflectNode( SceneRenderState *state,
                                  ReflectRenderNode &node,
                                  InteriorInstance *intInst,
                                  SceneData &sgData,
                                  MeshRenderInst *coreRi )
{
   BaseMatInstance *matInst = state->getOverrideMaterial( node.matInst );
   if ( !matInst )
      return;
   
   MeshRenderInst *ri = state->getRenderPass()->allocInst<MeshRenderInst>();
   *ri = *coreRi;

   ri->vertBuff = &mReflectVertBuff;
   ri->primBuff = &mReflectPrimBuff;

   // use sgData.backBuffer to transfer the reflect texture to the materials
   PlaneReflector *rp = &intInst->mPlaneReflectors[ node.reflectPlaneIndex ];
   ri->reflectTex = rp->reflectTex;
   ri->reflective = true;

   // setup lightmap
   if (dStricmp(LIGHTMGR->getId(), "BLM") == 0)
   {
      if (  node.lightMapIndex != U8(-1) && 
            matInst->getFeatures().hasFeature( MFT_LightMap ) )
         ri->lightmap = gInteriorLMManager.getHandle(mLMHandle, intInst->getLMHandle(), node.lightMapIndex );
   }

   ri->matInst = matInst;
   ri->primBuffIndex = node.primInfoIndex;

   // Sort by the material then the normal map or vertex buffer!
   ri->defaultKey = matInst->getStateHint();
   ri->defaultKey2 = ri->lightmap ? (U32)ri->lightmap : (U32)ri->vertBuff;

   state->getRenderPass()->addInst( ri );
}


//------------------------------------------------------------------------------
// Setup the rendering
//------------------------------------------------------------------------------
void Interior::setupRender( InteriorInstance *intInst,
                            SceneRenderState *state,
                            MeshRenderInst *coreRi, const MatrixF* worldToCamera )
{
   // Set the vertex and primitive buffers
   coreRi->vertBuff = &mVertBuff;
   coreRi->primBuff = &mPrimBuff;

   // Grab our render transform and scale it
   MatrixF objectToWorld = intInst->getRenderTransform();
   objectToWorld.scale( intInst->getScale() );

   coreRi->objectToWorld = state->getRenderPass()->allocUniqueXform(objectToWorld);
   coreRi->worldToCamera = state->getRenderPass()->allocUniqueXform(*worldToCamera); // This is handed down from SceneRenderState::renderCurrentImages()
   coreRi->projection = state->getRenderPass()->allocSharedXform(RenderPassManager::Projection);

   coreRi->type = RenderPassManager::RIT_Interior;
   
   // NOTICE: SFXBB is removed and refraction is disabled!
   //coreRi->backBuffTex = GFX->getSfxBackBuffer();
}


//------------------------------------------------------------------------------
// Render
//------------------------------------------------------------------------------
void Interior::prepBatchRender( InteriorInstance *intInst, SceneRenderState *state )
{
   // coreRi - used as basis for subsequent interior ri allocations
   MeshRenderInst *coreRi = state->getRenderPass()->allocInst<MeshRenderInst>();
   SceneData sgData;
   sgData.init( state );
   setupRender( intInst, state, coreRi, &state->getWorldViewMatrix() );
   ZoneVisDeterminer zoneVis = setupZoneVis( intInst, state );

// GFX2_RENDER_MERGE
#ifndef TORQUE_SHIPPING
   if( smRenderMode != 0 )
   {
      ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
      ri->renderDelegate.bind(intInst, &InteriorInstance::_renderObject);
      ri->type = RenderPassManager::RIT_Object;
      state->getRenderPass()->addInst( ri );
      return;
   }
#endif

   // render zones
   for( U32 i=0; i<mZones.size(); i++ )
   {
      // No need to try to render zones without any surfaces
      if (mZones[i].surfaceCount == 0)
         continue;

      if( zoneVis.isZoneVisible(i) == false && !state->isReflectPass() )
         continue;

      for( U32 j=0; j<mZoneRNList[i].renderNodeList.size(); j++ )
      {
         RenderNode &node = mZoneRNList[i].renderNodeList[j];
         renderZoneNode( state, node, intInst, sgData, coreRi );
      }
   }
   
   // render static meshes...
   for(U32 i=0; i<mStaticMeshes.size(); i++)
      mStaticMeshes[i]->render(  state, *coreRi, getLMHandle(), 
                                 intInst->getLMHandle(), intInst );

   // render reflective surfaces
   if ( !state->isReflectPass() )
   {
      renderLights(state, intInst, sgData, coreRi, zoneVis);

      for( U32 i=0; i<mZones.size(); i++ )
      {
         if( zoneVis.isZoneVisible(i) == false ) continue;

         for( U32 j=0; j<mZoneReflectRNList[i].reflectList.size(); j++ )
         {
            ReflectRenderNode &node = mZoneReflectRNList[i].reflectList[j];
            renderReflectNode( state, node, intInst, sgData, coreRi );
         }
      }
   }
}

void Interior::renderLights(  SceneRenderState* state, 
                              InteriorInstance *intInst, 
                              SceneData &sgData,
		                        MeshRenderInst *coreRi, 
                              const ZoneVisDeterminer &zonevis )
{
   // TODO!

   /*
   if (!smLightPlugin)
      return;

   if (!smLightPlugin->interiorInstInit(intInst))
      return;

	// build the render instances...
	for(U32 z=0; z<mZones.size(); z++)
	{
		if(!zonevis.isZoneVisible(z))
			continue;

		Zone &zone = mZones[z];
		S32 zoneid = zone.zoneId - 1;
		if(zoneid > -1)
			zoneid += intInst->getZoneRangeStart();// only zone managers...
		else
			zoneid = intInst->_getCurrZone(0);// if not what zone is it in...

      if (!smLightPlugin->zoneInit(zoneid))
         continue;

      static Vector<MeshRenderInst*> sRenderList;
      sRenderList.clear();

		for(U32 j=0; j<mZoneRNList[z].renderNodeList.size(); j++)
		{
			RenderNode &node = mZoneRNList[z].renderNodeList[j];

			if (!node.matInst)
				continue;

			MeshRenderInst *ri = state->getRenderPass()->allocInst<MeshRenderInst>();
			*ri = *coreRi;
			ri->type = RenderPassManager::RIT_InteriorDynamicLighting;
			ri->matInst = node.matInst;
			ri->primBuffIndex = node.primInfoIndex;

         sRenderList.push_back(ri);
		}		
      smLightPlugin->processRI(state, sRenderList);
	}
   */
}


