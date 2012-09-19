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
#include "lighting/shadowMap/pssmLightShadowMap.h"

#include "lighting/common/lightMapParams.h"
#include "console/console.h"
#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "lighting/lightManager.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/util/gfxFrustumSaver.h"
#include "renderInstance/renderPassManager.h"
#include "gui/controls/guiBitmapCtrl.h"
#include "lighting/shadowMap/shadowMapManager.h"
#include "materials/shaderData.h"
#include "ts/tsShapeInstance.h"
#include "console/consoleTypes.h"


AFTER_MODULE_INIT( Sim )
{
   Con::addVariable( "$pref::PSSM::detailAdjustScale", 
      TypeF32, &PSSMLightShadowMap::smDetailAdjustScale,
      "@brief Scales the model LOD when rendering into the PSSM shadow.\n"
      "Use this to reduce the draw calls when rendering the shadow by having "
      "meshes LOD out nearer to the camera than normal.\n"
      "@see $pref::TS::detailAdjust\n"
      "@ingroup AdvancedLighting" );

   Con::addVariable( "$pref::PSSM::smallestVisiblePixelSize", 
      TypeF32, &PSSMLightShadowMap::smSmallestVisiblePixelSize,
      "@brief The smallest pixel size an object can be and still be rendered into the PSSM shadow.\n"
      "Use this to force culling of small objects which contribute little to the final shadow.\n"
      "@see $pref::TS::smallestVisiblePixelSize\n"
      "@ingroup AdvancedLighting" );
}

F32 PSSMLightShadowMap::smDetailAdjustScale = 0.85f;
F32 PSSMLightShadowMap::smSmallestVisiblePixelSize = 25.0f;


PSSMLightShadowMap::PSSMLightShadowMap( LightInfo *light )
   :  LightShadowMap( light ),
      mNumSplits( 0 )
{  
   mIsViewDependent = true;
}

void PSSMLightShadowMap::_setNumSplits( U32 numSplits, U32 texSize )
{
   AssertFatal( numSplits > 0 && numSplits <= MAX_SPLITS, 
      "PSSMLightShadowMap::_setNumSplits() - Splits must be between 1 and 4!" );

   releaseTextures();
  
   mNumSplits = numSplits;
   mTexSize = texSize;
   F32 texWidth, texHeight;

   // If the split count is less than 4 then do a
   // 1xN layout of shadow maps...
   if ( mNumSplits < 4 )
   {
      texHeight = texSize;
      texWidth = texSize * mNumSplits;

      for ( U32 i = 0; i < 4; i++ )
      {
         mViewports[i].extent.set(texSize, texSize);
         mViewports[i].point.set(texSize*i, 0);
      }
   } 
   else 
   {
      // ... with 4 splits do a 2x2.
      texWidth = texHeight = texSize * 2;

      for ( U32 i = 0; i < 4; i++ )
      {
         F32 xOff = (i == 1 || i == 3) ? 0.5f : 0.0f;
         F32 yOff = (i > 1) ? 0.5f : 0.0f;
         mViewports[i].extent.set( texSize, texSize );
         mViewports[i].point.set( xOff * texWidth, yOff * texHeight );
      }
   }

   mShadowMapTex.set(   texWidth, texHeight, 
                        ShadowMapFormat, &ShadowMapProfile, 
                        "PSSMLightShadowMap" );
}

void PSSMLightShadowMap::_calcSplitPos(const Frustum& currFrustum)
{
   const F32 nearDist = 0.01f; // TODO: Should this be adjustable or different?
   const F32 farDist = currFrustum.getFarDist();

   for ( U32 i = 1; i < mNumSplits; i++ )
   {
      F32 step = (F32) i / (F32) mNumSplits;
      F32 logSplit = nearDist * mPow(farDist / nearDist, step);
      F32 linearSplit = nearDist + (farDist - nearDist) * step;
      mSplitDist[i] = mLerp( linearSplit, logSplit, mClampF( mLogWeight, 0.0f, 1.0f ) );
   }

   mSplitDist[0] = nearDist;
   mSplitDist[mNumSplits] = farDist;
}

Box3F PSSMLightShadowMap::_calcClipSpaceAABB(const Frustum& f, const MatrixF& transform, F32 farDist)
{
   // Calculate frustum center
   Point3F center(0,0,0);
   for (U32 i = 0; i < 8; i++)   
   {
      const Point3F& pt = f.getPoints()[i];      
      center += pt;
   }
   center /= 8;

   // Calculate frustum bounding sphere radius
   F32 radius = 0.0f;
   for (U32 i = 0; i < 8; i++)      
      radius = getMax(radius, (f.getPoints()[i] - center).lenSquared());
   radius = mFloor( mSqrt(radius) );
      
   // Now build box for sphere
   Box3F result;
   Point3F radiusBox(radius, radius, radius);
   result.minExtents = center - radiusBox;
   result.maxExtents = center + radiusBox;

   // Transform to light projection space
   transform.mul(result);
   
   return result;   
}

// This "rounds" the projection matrix to remove subtexel movement during shadow map
// rasterization.  This is here to reduce shadow shimmering.
void PSSMLightShadowMap::_roundProjection(const MatrixF& lightMat, const MatrixF& cropMatrix, Point3F &offset, U32 splitNum)
{
   // Round to the nearest shadowmap texel, this helps reduce shimmering
   MatrixF currentProj = GFX->getProjectionMatrix();
   currentProj = cropMatrix * currentProj * lightMat;

   // Project origin to screen.
   Point4F originShadow4F(0,0,0,1);
   currentProj.mul(originShadow4F);
   Point2F originShadow(originShadow4F.x / originShadow4F.w, originShadow4F.y / originShadow4F.w);   

   // Convert to texture space (0..shadowMapSize)
   F32 t = mNumSplits < 4 ? mShadowMapTex->getWidth() / mNumSplits : mShadowMapTex->getWidth() / 2;
   Point2F texelsToTexture(t / 2.0f, mShadowMapTex->getHeight() / 2.0f);
   if (mNumSplits >= 4) texelsToTexture.y *= 0.5f;
   originShadow.convolve(texelsToTexture);

   // Clamp to texel boundary
   Point2F originRounded;
   originRounded.x = mFloor(originShadow.x + 0.5f);
   originRounded.y = mFloor(originShadow.y + 0.5f);

   // Subtract origin to get an offset to recenter everything on texel boundaries
   originRounded -= originShadow;

   // Convert back to texels (0..1) and offset
   originRounded.convolveInverse(texelsToTexture);
   offset.x += originRounded.x;
   offset.y += originRounded.y;
}

void PSSMLightShadowMap::_render(   RenderPassManager* renderPass,
                                    const SceneRenderState *diffuseState )
{
   PROFILE_SCOPE(PSSMLightShadowMap_render);

   const ShadowMapParams *params = mLight->getExtended<ShadowMapParams>();
   const LightMapParams *lmParams = mLight->getExtended<LightMapParams>();
   const bool bUseLightmappedGeometry = lmParams ? !lmParams->representedInLightmap || lmParams->includeLightmappedGeometryInShadow : true;

   const U32 texSize = getBestTexSize( params->numSplits < 4 ? params->numSplits : 2 );

   if (  mShadowMapTex.isNull() || 
         mNumSplits != params->numSplits || 
         mTexSize != texSize )
      _setNumSplits( params->numSplits, texSize );
   mLogWeight = params->logWeight;

   Frustum fullFrustum( diffuseState->getFrustum() );
   fullFrustum.cropNearFar(fullFrustum.getNearDist(), params->shadowDistance);

   GFXFrustumSaver frustSaver;
   GFXTransformSaver saver;

   // Set our render target
   GFX->pushActiveRenderTarget();
   mTarget->attachTexture( GFXTextureTarget::Color0, mShadowMapTex );
   mTarget->attachTexture( GFXTextureTarget::DepthStencil, 
      _getDepthTarget( mShadowMapTex->getWidth(), mShadowMapTex->getHeight() ) );
   GFX->setActiveRenderTarget( mTarget );
   GFX->clear( GFXClearStencil | GFXClearZBuffer | GFXClearTarget, ColorI(255,255,255), 1.0f, 0 );

   // Calculate our standard light matrices
   MatrixF lightMatrix;
   calcLightMatrices( lightMatrix, diffuseState->getFrustum() );
   lightMatrix.inverse();
   MatrixF lightViewProj = GFX->getProjectionMatrix() * lightMatrix;

   // TODO: This is just retrieving the near and far calculated
   // in calcLightMatrices... we should make that clear.
   F32 pnear, pfar;
   GFX->getFrustum( NULL, NULL, NULL, NULL, &pnear, &pfar, NULL );

   // Set our view up
   GFX->setWorldMatrix(lightMatrix);
   MatrixF toLightSpace = lightMatrix; // * invCurrentView;

   _calcSplitPos(fullFrustum);
   
   mWorldToLightProj = GFX->getProjectionMatrix() * toLightSpace;

   // Apply the PSSM 
   const F32 savedSmallestVisible = TSShapeInstance::smSmallestVisiblePixelSize;
   const F32 savedDetailAdjust = TSShapeInstance::smDetailAdjust;
   TSShapeInstance::smDetailAdjust *= smDetailAdjustScale;
   TSShapeInstance::smSmallestVisiblePixelSize = smSmallestVisiblePixelSize;

   for (U32 i = 0; i < mNumSplits; i++)
   {
      GFXTransformSaver saver;

      // Calculate a sub-frustum
      Frustum subFrustum(fullFrustum);
      subFrustum.cropNearFar(mSplitDist[i], mSplitDist[i+1]);

      // Calculate our AABB in the light's clip space.
      Box3F clipAABB = _calcClipSpaceAABB(subFrustum, lightViewProj, fullFrustum.getFarDist());
 
      // Calculate our crop matrix
      Point3F scale(2.0f / (clipAABB.maxExtents.x - clipAABB.minExtents.x),
         2.0f / (clipAABB.maxExtents.y - clipAABB.minExtents.y),
         1.0f);

      // TODO: This seems to produce less "pops" of the
      // shadow resolution as the camera spins around and
      // it should produce pixels that are closer to being
      // square.
      //
      // Still is it the right thing to do?
      //
      scale.y = scale.x = ( getMin( scale.x, scale.y ) ); 
      //scale.x = mFloor(scale.x); 
      //scale.y = mFloor(scale.y); 

      Point3F offset(   -0.5f * (clipAABB.maxExtents.x + clipAABB.minExtents.x) * scale.x,
                        -0.5f * (clipAABB.maxExtents.y + clipAABB.minExtents.y) * scale.y,
                        0.0f );

      MatrixF cropMatrix(true);
      cropMatrix.scale(scale);
      cropMatrix.setPosition(offset);

      _roundProjection(lightMatrix, cropMatrix, offset, i);

      cropMatrix.setPosition(offset);      

      // Save scale/offset for shader computations
      mScaleProj[i].set(scale);
      mOffsetProj[i].set(offset);

      // Adjust the far plane to the max z we got (maybe add a little to deal with split overlap)
      bool isOrtho;
      {
         F32 left, right, bottom, top, nearDist, farDist;
         GFX->getFrustum(&left, &right, &bottom, &top, &nearDist, &farDist,&isOrtho);
         // BTRTODO: Fix me!
         farDist = clipAABB.maxExtents.z;
         if (!isOrtho)
            GFX->setFrustum(left, right, bottom, top, nearDist, farDist);
         else
         {
            // Calculate a new far plane, add a fudge factor to avoid bringing
            // the far plane in too close.
            F32 newFar = pfar * clipAABB.maxExtents.z + 1.0f;
            mFarPlaneScalePSSM[i] = (pfar - pnear) / (newFar - pnear);
            GFX->setOrtho(left, right, bottom, top, pnear, newFar, true);
         }
      }

      // Crop matrix multiply needs to be post-projection.
      MatrixF alightProj = GFX->getProjectionMatrix();
      alightProj = cropMatrix * alightProj;

      // Set our new projection
      GFX->setProjectionMatrix(alightProj);

      // Render into the quad of the shadow map we are using.
      GFX->setViewport(mViewports[i]);

      SceneManager* sceneManager = diffuseState->getSceneManager();

      // The frustum is currently the  full size and has not had
      // cropping applied.
      //
      // We make that adjustment here.

      const Frustum& uncroppedFrustum = GFX->getFrustum();
      Frustum croppedFrustum;
      scale *= 0.5f;
      croppedFrustum.set(
         isOrtho,
         uncroppedFrustum.getNearLeft() / scale.x,
         uncroppedFrustum.getNearRight() / scale.x,
         uncroppedFrustum.getNearTop() / scale.y,
         uncroppedFrustum.getNearBottom() / scale.y,
         uncroppedFrustum.getNearDist(),
         uncroppedFrustum.getFarDist(),
         uncroppedFrustum.getTransform()
      );

      MatrixF camera = GFX->getWorldMatrix();
      camera.inverse();
      croppedFrustum.setTransform( camera );

      // Setup the scene state and use the diffuse state
      // camera position and screen metrics values so that
      // lod is done the same as in the diffuse pass.

      SceneRenderState shadowRenderState
      (
         sceneManager,
         SPT_Shadow,
         SceneCameraState( diffuseState->getViewport(), croppedFrustum,
                           GFX->getWorldMatrix(), GFX->getProjectionMatrix() ),
         renderPass
      );

      shadowRenderState.getMaterialDelegate().bind( this, &LightShadowMap::getShadowMaterial );
      shadowRenderState.renderNonLightmappedMeshes( true );
      shadowRenderState.renderLightmappedMeshes( bUseLightmappedGeometry );

      shadowRenderState.setDiffuseCameraTransform( diffuseState->getCameraTransform() );
      shadowRenderState.setWorldToScreenScale( diffuseState->getWorldToScreenScale() );

      U32 objectMask = SHADOW_TYPEMASK;
      if ( i == mNumSplits-1 && params->lastSplitTerrainOnly )
         objectMask = TerrainObjectType;

      sceneManager->renderSceneNoLights( &shadowRenderState, objectMask );

      _debugRender( &shadowRenderState );
   }

   // Restore the original TS lod settings.
   TSShapeInstance::smSmallestVisiblePixelSize = savedSmallestVisible;
   TSShapeInstance::smDetailAdjust = savedDetailAdjust;

   // Release our render target
   mTarget->resolve();
   GFX->popActiveRenderTarget();
}

void PSSMLightShadowMap::setShaderParameters(GFXShaderConstBuffer* params, LightingShaderConstants* lsc)
{
   PROFILE_SCOPE( PSSMLightShadowMap_setShaderParameters );

   if ( lsc->mTapRotationTexSC->isValid() )
      GFX->setTexture( lsc->mTapRotationTexSC->getSamplerRegister(), 
                        SHADOWMGR->getTapRotationTex() );

   const ShadowMapParams *p = mLight->getExtended<ShadowMapParams>();

   Point4F  sx(Point4F::Zero), 
            sy(Point4F::Zero),
            ox(Point4F::Zero), 
            oy(Point4F::Zero), 
            aXOff(Point4F::Zero), 
            aYOff(Point4F::Zero);

   for (U32 i = 0; i < mNumSplits; i++)
   {
      sx[i] = mScaleProj[i].x;
      sy[i] = mScaleProj[i].y;
      ox[i] = mOffsetProj[i].x;
      oy[i] = mOffsetProj[i].y;
   }

   Point2F shadowMapAtlas;
   if (mNumSplits < 4)
   {
      shadowMapAtlas.x = 1.0f / (F32)mNumSplits;
      shadowMapAtlas.y = 1.0f;
   
      // 1xmNumSplits
      for (U32 i = 0; i < mNumSplits; i++)
         aXOff[i] = (F32)i * shadowMapAtlas.x;
   }
   else
   {
      shadowMapAtlas.set(0.5f, 0.5f);
  
      // 2x2
      for (U32 i = 0; i < mNumSplits; i++)
      {
         if (i == 1 || i == 3)
            aXOff[i] = 0.5f;
         if (i > 1)
            aYOff[i] = 0.5f;
      }
   }

   params->setSafe(lsc->mScaleXSC, sx);
   params->setSafe(lsc->mScaleYSC, sy);
   params->setSafe(lsc->mOffsetXSC, ox);
   params->setSafe(lsc->mOffsetYSC, oy);
   params->setSafe(lsc->mAtlasXOffsetSC, aXOff);
   params->setSafe(lsc->mAtlasYOffsetSC, aYOff);
   params->setSafe(lsc->mAtlasScaleSC, shadowMapAtlas);

   Point4F lightParams( mLight->getRange().x, p->overDarkFactor.x, 0.0f, 0.0f );
   params->setSafe( lsc->mLightParamsSC, lightParams );
      
   params->setSafe( lsc->mFarPlaneScalePSSM, mFarPlaneScalePSSM);

   Point2F fadeStartLength(p->fadeStartDist, 0.0f);
   if (fadeStartLength.x == 0.0f)
   {
      // By default, lets fade the last half of the last split.
      fadeStartLength.x = (mSplitDist[mNumSplits-1] + mSplitDist[mNumSplits]) / 2.0f;
   }
   fadeStartLength.y = 1.0f / (mSplitDist[mNumSplits] - fadeStartLength.x);
   params->setSafe( lsc->mFadeStartLength, fadeStartLength);
   
   params->setSafe( lsc->mOverDarkFactorPSSM, p->overDarkFactor);

   // The softness is a factor of the texel size.
   params->setSafe( lsc->mShadowSoftnessConst, p->shadowSoftness * ( 1.0f / mTexSize ) );
}
