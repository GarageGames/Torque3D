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
#include "lighting/shadowMap/cubeLightShadowMap.h"

#include "lighting/shadowMap/shadowMapManager.h"
#include "lighting/common/lightMapParams.h"
#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDebugEvent.h"
#include "renderInstance/renderPassManager.h"
#include "materials/materialDefinition.h"
#include "gfx/util/gfxFrustumSaver.h"
#include "math/mathUtils.h"


CubeLightShadowMap::CubeLightShadowMap( LightInfo *light )
   : Parent( light )
{
}

bool CubeLightShadowMap::setTextureStage( U32 currTexFlag, LightingShaderConstants* lsc )
{
   if ( currTexFlag == Material::DynamicLight )
   {
      S32 reg = lsc->mShadowMapSC->getSamplerRegister();
   	if ( reg != -1 )
      	GFX->setCubeTexture( reg, mCubemap );

      return true;
   }

   return false;
}

void CubeLightShadowMap::setShaderParameters(   GFXShaderConstBuffer *params, 
                                                LightingShaderConstants *lsc )
{
   if ( lsc->mTapRotationTexSC->isValid() )
      GFX->setTexture( lsc->mTapRotationTexSC->getSamplerRegister(), 
                        SHADOWMGR->getTapRotationTex() );

   ShadowMapParams *p = mLight->getExtended<ShadowMapParams>();

   if ( lsc->mLightParamsSC->isValid() )
   {
      Point4F lightParams( mLight->getRange().x, 
                           p->overDarkFactor.x, 
                           0.0f, 
                           0.0f );
      params->set(lsc->mLightParamsSC, lightParams);
   }

   // The softness is a factor of the texel size.
   params->setSafe( lsc->mShadowSoftnessConst, p->shadowSoftness * ( 1.0f / mTexSize ) );
}

void CubeLightShadowMap::releaseTextures()
{
   Parent::releaseTextures();
   mCubemap = NULL;
}

void CubeLightShadowMap::_render(   RenderPassManager* renderPass,
                                    const SceneRenderState *diffuseState )
{
   PROFILE_SCOPE( CubeLightShadowMap_Render );

   const LightMapParams *lmParams = mLight->getExtended<LightMapParams>();
   const bool bUseLightmappedGeometry = lmParams ? !lmParams->representedInLightmap || lmParams->includeLightmappedGeometryInShadow : true;

   const U32 texSize = getBestTexSize();

   if (  mCubemap.isNull() || 
         mTexSize != texSize )
   {
      mTexSize = texSize;
      mCubemap = GFX->createCubemap();
      mCubemap->initDynamic( mTexSize, LightShadowMap::ShadowMapFormat );
   }

   // Setup the world to light projection which is used
   // in the shader to transform the light vector for the
   // shadow lookup.
   mWorldToLightProj = mLight->getTransform();
   mWorldToLightProj.inverse();

   // Set up frustum and visible distance
   GFXFrustumSaver fsaver;
   GFXTransformSaver saver;
   {
      F32 left, right, top, bottom;
      MathUtils::makeFrustum( &left, &right, &top, &bottom, M_HALFPI_F, 1.0f, 0.1f );
      GFX->setFrustum( left, right, bottom, top, 0.1f, mLight->getRange().x );
   }

   // Render the shadowmap!
   GFX->pushActiveRenderTarget();

   for( U32 i = 0; i < 6; i++ )
   {
      // Standard view that will be overridden below.
      VectorF vLookatPt(0.0f, 0.0f, 0.0f), vUpVec(0.0f, 0.0f, 0.0f), vRight(0.0f, 0.0f, 0.0f);

      switch( i )
      {
      case 0 : // D3DCUBEMAP_FACE_POSITIVE_X:
         vLookatPt = VectorF(1.0f, 0.0f, 0.0f);
         vUpVec    = VectorF(0.0f, 1.0f, 0.0f);
         break;
      case 1 : // D3DCUBEMAP_FACE_NEGATIVE_X:
         vLookatPt = VectorF(-1.0f, 0.0f, 0.0f);
         vUpVec    = VectorF(0.0f, 1.0f, 0.0f);
         break;
      case 2 : // D3DCUBEMAP_FACE_POSITIVE_Y:
         vLookatPt = VectorF(0.0f, 1.0f, 0.0f);
         vUpVec    = VectorF(0.0f, 0.0f,-1.0f);
         break;
      case 3 : // D3DCUBEMAP_FACE_NEGATIVE_Y:
         vLookatPt = VectorF(0.0f, -1.0f, 0.0f);
         vUpVec    = VectorF(0.0f, 0.0f, 1.0f);
         break;
      case 4 : // D3DCUBEMAP_FACE_POSITIVE_Z:
         vLookatPt = VectorF(0.0f, 0.0f, 1.0f);
         vUpVec    = VectorF(0.0f, 1.0f, 0.0f);
         break;
      case 5: // D3DCUBEMAP_FACE_NEGATIVE_Z:
         vLookatPt = VectorF(0.0f, 0.0f, -1.0f);
         vUpVec    = VectorF(0.0f, 1.0f, 0.0f);
         break;
      }

      GFXDEBUGEVENT_START( CubeLightShadowMap_Render_Face, ColorI::RED );

      // create camera matrix
      VectorF cross = mCross(vUpVec, vLookatPt);
      cross.normalizeSafe();

      MatrixF lightMatrix(true);
      lightMatrix.setColumn(0, cross);
      lightMatrix.setColumn(1, vLookatPt);
      lightMatrix.setColumn(2, vUpVec);
      lightMatrix.setPosition( mLight->getPosition() );
      lightMatrix.inverse();

      GFX->setWorldMatrix( lightMatrix );

      mTarget->attachTexture(GFXTextureTarget::Color0, mCubemap, i);
      mTarget->attachTexture(GFXTextureTarget::DepthStencil, _getDepthTarget( mTexSize, mTexSize ));
      GFX->setActiveRenderTarget(mTarget);
      GFX->clear( GFXClearTarget | GFXClearStencil | GFXClearZBuffer, ColorI(255,255,255,255), 1.0f, 0 );

      // Create scene state, prep it
      SceneManager* sceneManager = diffuseState->getSceneManager();
      
      SceneRenderState shadowRenderState
      (
         sceneManager,
         SPT_Shadow,
         SceneCameraState::fromGFXWithViewport( diffuseState->getViewport() ),
         renderPass
      );

      shadowRenderState.getMaterialDelegate().bind( this, &LightShadowMap::getShadowMaterial );
      shadowRenderState.renderNonLightmappedMeshes( true );
      shadowRenderState.renderLightmappedMeshes( bUseLightmappedGeometry );
      shadowRenderState.setDiffuseCameraTransform( diffuseState->getCameraTransform() );
      shadowRenderState.setWorldToScreenScale( diffuseState->getWorldToScreenScale() );

      sceneManager->renderSceneNoLights( &shadowRenderState, SHADOW_TYPEMASK );

      _debugRender( &shadowRenderState );

      // Resolve this face
      mTarget->resolve();

      GFXDEBUGEVENT_END();
   }
   GFX->popActiveRenderTarget();
}
