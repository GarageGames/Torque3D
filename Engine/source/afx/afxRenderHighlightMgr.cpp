
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
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
//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// The afxRenderHighlightMgr class is adapted from the resource,
// "Silhoute selection via postFX for Torque3D" posted by Konrad Kiss.
// http://www.garagegames.com/community/resources/view/17821
// Supporting code mods in other areas of the engine are marked as
// "(selection-highlight)".
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "platform/platform.h"
#include "afxRenderHighlightMgr.h"

#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "materials/sceneData.h"
#include "materials/matInstance.h"
//#include "materials/materialFeatureTypes.h"
#include "materials/processedMaterial.h"
#include "postFx/postEffect.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDebugEvent.h"
#include "math/util/matrixSet.h"

IMPLEMENT_CONOBJECT( afxRenderHighlightMgr );

afxRenderHighlightMgr::afxRenderHighlightMgr()
   : RenderTexTargetBinManager(  RenderPassManager::RIT_Mesh, 
                                 1.0f, 
                                 1.0f,
                                 GFXFormatR8G8B8A8,
                                 Point2I( 512, 512 ) )
{
   mNamedTarget.registerWithName( "highlight" );
   mTargetSizeType = WindowSize;
}

afxRenderHighlightMgr::~afxRenderHighlightMgr()
{
}

PostEffect* afxRenderHighlightMgr::getSelectionEffect()
{
   if ( !mSelectionEffect )
      mSelectionEffect = dynamic_cast<PostEffect*>( Sim::findObject( "afxHighlightPostFX" ) );
   
   return mSelectionEffect;
}

bool afxRenderHighlightMgr::isSelectionEnabled()
{
   return getSelectionEffect() && getSelectionEffect()->isEnabled();
}

void afxRenderHighlightMgr::addElement( RenderInst *inst )
{
   // Skip out if we don't have the selection post 
   // effect enabled at this time.
   if ( !isSelectionEnabled() )  
      return;

   // Skip it if we don't have a selection material.
   BaseMatInstance *matInst = getMaterial( inst );
   if ( !matInst || !matInst->needsSelectionHighlighting() )   
      return;

   internalAddElement(inst);
}

void afxRenderHighlightMgr::render( SceneRenderState *state )  
{
   PROFILE_SCOPE( RenderSelectionMgr_Render );
   
   if ( !isSelectionEnabled() )
      return;

   const U32 binSize = mElementList.size();

   // If this is a non-diffuse pass or we have no objects to
   // render then tell the effect to skip rendering.
   if ( !state->isDiffusePass() || binSize == 0 )
   {
      getSelectionEffect()->setSkip( true );
      return;
   }

   GFXDEBUGEVENT_SCOPE( RenderSelectionMgr_Render, ColorI::GREEN );

   GFXTransformSaver saver;

   // Tell the superclass we're about to render, preserve contents
   const bool isRenderingToTarget = _onPreRender( state, true );

   // Clear all the buffers to black.
   //GFX->clear( GFXClearTarget, ColorI::BLACK, 1.0f, 0);
   GFX->clear( GFXClearTarget, ColorI::ZERO, 1.0f, 0);

   // Restore transforms
   MatrixSet &matrixSet = getRenderPass()->getMatrixSet();
   matrixSet.restoreSceneViewProjection();

   // init loop data
   SceneData sgData;
   sgData.init( state, SceneData::HighlightBin );

   for( U32 j=0; j<binSize; )
   {
      MeshRenderInst *ri = static_cast<MeshRenderInst*>(mElementList[j].inst);

      setupSGData( ri, sgData );

      BaseMatInstance *mat = ri->matInst;

      U32 matListEnd = j;

      while( mat && mat->setupPass( state, sgData ) )
      {
         U32 a;
         for( a=j; a<binSize; a++ )
         {
            MeshRenderInst *passRI = static_cast<MeshRenderInst*>(mElementList[a].inst);

            if ( newPassNeeded( ri, passRI ) )
               break;

            matrixSet.setWorld(*passRI->objectToWorld);
            matrixSet.setView(*passRI->worldToCamera);
            matrixSet.setProjection(*passRI->projection);
            mat->setTransforms(matrixSet, state);

            mat->setSceneInfo(state, sgData);
            mat->setBuffers(passRI->vertBuff, passRI->primBuff);

            if ( passRI->prim )
               GFX->drawPrimitive( *passRI->prim );
            else
               GFX->drawPrimitive( passRI->primBuffIndex );
         }
         matListEnd = a;
         setupSGData( ri, sgData );
      }

      // force increment if none happened, otherwise go to end of batch
      j = ( j == matListEnd ) ? j+1 : matListEnd;
   }

   // Finish up.
   if ( isRenderingToTarget )
      _onPostRender();

   // Make sure the effect is gonna render.
   getSelectionEffect()->setSkip( false );
}