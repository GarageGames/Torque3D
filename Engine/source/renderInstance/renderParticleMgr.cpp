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
#include "renderInstance/renderParticleMgr.h"
#include "renderInstance/renderPrePassMgr.h"
#include "scene/sceneManager.h"
#include "scene/sceneObject.h"
#include "scene/sceneRenderState.h"
#include "gfx/gfxPrimitiveBuffer.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDebugEvent.h"
#include "materials/shaderData.h"
#include "materials/sceneData.h"
#include "materials/matInstance.h"
#include "gfx/util/screenspace.h"
#include "gfx/gfxDrawUtil.h"
#include "collision/clippedPolyList.h"

static const Point4F cubePoints[9] = 
{
   Point4F(-0.5, -0.5, -0.5, 1.0f), Point4F(-0.5, -0.5,  0.5, 1.0f), Point4F(-0.5,  0.5, -0.5, 1.0f), Point4F(-0.5,  0.5,  0.5, 1.0f),
   Point4F( 0.5, -0.5, -0.5, 1.0f), Point4F( 0.5, -0.5,  0.5, 1.0f), Point4F( 0.5,  0.5, -0.5, 1.0f), Point4F( 0.5,  0.5,  0.5, 1.0f)
};

GFXImplementVertexFormat( CompositeQuadVert )
{
   addElement( "COLOR", GFXDeclType_Color );
}

IMPLEMENT_CONOBJECT(RenderParticleMgr);


ConsoleDocClass( RenderParticleMgr, 
   "@brief A render bin which renders particle geometry.\n\n"
   "This render bin gathers particle render instances, sorts, and renders them. "
   "It is currently used by ParticleEmitter and LightFlareData.\n\n"
   "@ingroup RenderBin\n" );


const RenderInstType RenderParticleMgr::RIT_Particles("ParticleSystem");

// TODO: Replace these once they are supported via options
const bool RenderToParticleTarget = true;
const bool RenderToSingleTarget = true;

RenderParticleMgr::RenderParticleMgr()
:  Parent(  RenderParticleMgr::RIT_Particles, 
            1.0f, 
            1.0f, 
            GFXFormatR8G8B8A8, 
            Point2I( Parent::DefaultTargetSize, Parent::DefaultTargetSize), 
            RenderToParticleTarget ? Parent::DefaultTargetChainLength : 0 ),
            mParticleShader( NULL )
{
   // Render particles at 1/4 resolution
   mTargetSizeType = WindowSizeScaled;
   mTargetScale.set(0.25f, 0.25f);

   // We use the target chain like a texture pool, not like a swap chain
   if(!RenderToSingleTarget)
      setTargetChainLength(5);
   else
      mOffscreenSystems.setSize(1);

   LightManager::smActivateSignal.notify( this, &RenderParticleMgr::_onLMActivate );
}

RenderParticleMgr::~RenderParticleMgr()
{
   LightManager::smActivateSignal.remove( this, &RenderParticleMgr::_onLMActivate );
}

void RenderParticleMgr::setTargetChainLength( const U32 chainLength )
{
   Parent::setTargetChainLength(chainLength);

   if(!RenderToSingleTarget)
      mOffscreenSystems.setSize(chainLength);
}

void RenderParticleMgr::addElement( RenderInst *inst )
{
   ParticleRenderInst *pri = reinterpret_cast<ParticleRenderInst *>(inst);

   // If this system isn't waiting for an offscreen draw, skip it
   if( pri->systemState != PSS_AwaitingOffscreenDraw )
      return;

   // If offscreen rendering isn't enabled, set to high-res, and skip
   if(!mOffscreenRenderEnabled)
   {
      pri->systemState = PSS_AwaitingHighResDraw;
      return;
   }

   // Assign a target index
   RectF screenRect;
   S32 chainIndex = -1;
   if(RenderToSingleTarget)
   {
      pri->targetIndex = 0;
      screenRect.point.set(-1.0f, -1.0f);
      screenRect.extent.set(2.0f, 2.0f);

      mElementList.setSize(1);
   }
   else
   {

      // If we can't fit this into the offscreen systems, skip it, it will render
      // high resolution
      //
      // TODO: Improve this once we are grouping systems
      if( mTargetChainIdx == OffscreenPoolSize )
         return;

      // Transform bounding box into screen space
      const static PlaneF planes[] = {
         PlaneF(Point3F( 1.0f,  0.0f,  0.0f), Point3F(-1.0f,  0.0f,  0.0f)),
         PlaneF(Point3F(-1.0f,  0.0f,  0.0f), Point3F( 1.0f,  0.0f,  0.0f)),

         PlaneF(Point3F( 0.0f,  1.0f,  0.0f), Point3F( 0.0f, -1.0f,  0.0f)),
         PlaneF(Point3F( 0.0f, -1.0f,  0.0f), Point3F( 0.0f,  1.0f,  0.0f)),

         PlaneF(Point3F( 0.0f,  0.0f,  0.0f), Point3F( 0.0f,  0.0f,  1.0f)),
         PlaneF(Point3F( 0.0f,  0.0f,  1.0f), Point3F( 0.0f,  0.0f, -1.0f)),
      };
      const static dsize_t numPlanes = sizeof(planes) / sizeof(PlaneF);

      // Set up a clipper
      ClippedPolyList screenClipper;
      screenClipper.setBaseTransform(MatrixF::Identity);
      screenClipper.setTransform(&MatrixF::Identity, Point3F::One);
      TORQUE_UNUSED(numPlanes);

      Point4F tempPt(0.0f, 0.0f, 0.0f, 1.0f);
      pri->bbModelViewProj->mul(tempPt);
      tempPt = tempPt / tempPt.w;

      for(int i = 0; i < 1; i++)
      {
         screenClipper.mPlaneList.push_back(planes[i]);
         screenClipper.mPlaneList.last() += tempPt.asPoint3F();
      }

      Box3F screenSpaceBoundingBox;
      screenSpaceBoundingBox.minExtents = Point3F::Zero;
      screenSpaceBoundingBox.maxExtents = Point3F::Zero;

      for(int i = 0; i < 8; i++)
      {
         tempPt = cubePoints[i];
         pri->bbModelViewProj->mul(tempPt);
         tempPt = tempPt / tempPt.w;

         screenSpaceBoundingBox.maxExtents.setMax(tempPt.asPoint3F());
         screenSpaceBoundingBox.minExtents.setMin(tempPt.asPoint3F());
      }

      screenClipper.addBox(screenSpaceBoundingBox);

      screenClipper.cullUnusedVerts();
      //screenClipper.triangulate();

      // Empty vertex list? Skip!
      if(screenClipper.mVertexList.empty())
      {
         pri->systemState = PSS_AwaitingHighResDraw;
         return;
      }

      Point2F minExtents(0.0f, 0.0f), maxExtents(0.0f, 0.0f);
      for(ClippedPolyList::VertexList::const_iterator itr = screenClipper.mVertexList.begin();
         itr != screenClipper.mVertexList.end(); itr++)
      {
         minExtents.x = getMin(minExtents.x, (*itr).point.x);
         minExtents.y = getMin(minExtents.y, (*itr).point.y);
         maxExtents.x = getMax(maxExtents.x, (*itr).point.x);
         maxExtents.y = getMax(maxExtents.y, (*itr).point.y);
      }
      screenRect.set( minExtents, maxExtents - minExtents );

      // Check the size of the system on screen. If it is small, it won't
      // be eating fillrate anyway, so just draw it high-resolution.
      // The value it checks against is one I found from experimentation, 
      // not anything really meaningful.
      if( screenRect.extent.x < 0.35f || screenRect.extent.y < 0.35f )
      {
         pri->systemState = PSS_AwaitingHighResDraw;
         return;
      }

      pri->targetIndex = mTargetChainIdx;
      chainIndex = mTargetChainIdx;
      mTargetChainIdx++;

      // TODO: Rewrite this...
      mElementList.increment();
   }

   // Set up system entry
   OffscreenSystemEntry &systemEntry = mOffscreenSystems[pri->targetIndex];

   systemEntry.screenRect = screenRect;
   systemEntry.targetChainIdx = chainIndex;
   systemEntry.pInstances.push_back(pri);

   // TODO: Rewrite this block
   // Assign proper values to sort element
   MainSortElem& elem = mElementList.last();
   elem.inst = reinterpret_cast<RenderInst *>(&systemEntry);
   elem.key = *((U32*)&inst->sortDistSq);
   elem.key2 = inst->defaultKey;

   // TODO: [re]move this block
   systemEntry.clipMatrix.identity();
   if(!RenderToSingleTarget)
   {
      // Construct crop matrix
      Point3F scale( getMax(2.0f / systemEntry.screenRect.extent.x, 1.0f), 
         getMax(2.0f / systemEntry.screenRect.extent.y, 1.0f),
         1.0f);

      Point3F offset((systemEntry.screenRect.point.x + systemEntry.screenRect.extent.x * 0.5f) * scale.x,
         (systemEntry.screenRect.point.y + systemEntry.screenRect.extent.y * 0.5f) * scale.y,
         0.0f);

      //systemEntry.clipMatrix.scale(scale);
      //systemEntry.clipMatrix.setPosition(-offset);
   }

   // The translucent mgr will also pick up particles, and will call this class
   // to composiste the particle systems back into the main scene
}

void RenderParticleMgr::sort()
{
   Parent::sort();
}

void RenderParticleMgr::clear()
{
   Parent::clear();

   // Reset pool index
   if(!RenderToSingleTarget)
      mTargetChainIdx = 0;

   for(Vector<OffscreenSystemEntry>::iterator itr = mOffscreenSystems.begin();
      itr != mOffscreenSystems.end(); itr++)
   {
      (*itr).drawnThisFrame = false;
      (*itr).pInstances.clear();
   }
}

void RenderParticleMgr::render( SceneRenderState *state )
{
   PROFILE_SCOPE(RenderParticleMgr_render);

   // Early out if nothing to draw
   if( !mElementList.size() || 
      (!mParticleShader && !_initShader()) )
      return;

   GFXDEBUGEVENT_SCOPE(RenderParticleMgr_Render, ColorI::RED);

   GFXTransformSaver saver;

   // Iterate render instances
   for( Vector<MainSortElem>::const_iterator itr = mElementList.begin();
      itr != mElementList.end(); itr++ )
   {
      OffscreenSystemEntry &systemEntry = *reinterpret_cast<OffscreenSystemEntry *>(itr->inst);

      // Setup target
      // NOTE: If you are using this on the Xbox360 with Basic Lighting,
      // you are going to have to mess with either the render order, or 
      // you are going to have to make this a 'preserve' draw
      if(!RenderToSingleTarget)
         mTargetChainIdx = systemEntry.targetChainIdx;
      _onPreRender(state);

      // Clear offscreen target
      GFX->clear(GFXClearTarget, ColorI::ZERO, 1.0f, 0);

      // Draw offscreen systems
      for( Vector<ParticleRenderInst *>::const_iterator itr2 = systemEntry.pInstances.begin();
         itr2 != systemEntry.pInstances.end(); itr2++ )
      {
         ParticleRenderInst *ri = *itr2;

         // Sanity check
         if(ri->systemState == PSS_AwaitingOffscreenDraw)
         {
            // If this is not a diffuse path, flag the system appropriately, and skip
            // the offscreen processing.
            if( !state->isDiffusePass() )
            {
               if(state->isReflectPass())
                  ri->systemState = PSS_AwaitingHighResDraw;
               else
                  ri->systemState = PSS_DrawComplete;
               continue;
            }

            // Draw system offscreen
            renderInstance(ri, state);
         }
      }

      // Cleanup
      _onPostRender();
   }
}

void RenderParticleMgr::_initGFXResources()
{
   // Screen quad
   U16 *prims = NULL;
   mScreenQuadPrimBuff.set(GFX, 4, 2, GFXBufferTypeStatic);
   mScreenQuadPrimBuff.lock(&prims);
   (*prims++) = 0;
   (*prims++) = 1;
   (*prims++) = 2;
   (*prims++) = 3;
   mScreenQuadPrimBuff.unlock();

   mScreenQuadVertBuff.set(GFX, 4, GFXBufferTypeStatic);
   CompositeQuadVert *verts = mScreenQuadVertBuff.lock();
   (*verts++).uvCoord.set(0, 0, 0, 0);
   (*verts++).uvCoord.set(0, 255, 0, 0);
   (*verts++).uvCoord.set(255, 0, 0, 0);
   (*verts++).uvCoord.set(255, 255, 0, 0);
   mScreenQuadVertBuff.unlock();

   // Stencil setup state block
   GFXStateBlockDesc d;

   d.setCullMode(GFXCullNone);
   d.setColorWrites(false, false, false, false);
   d.setBlend(false);
   d.setZReadWrite(false, false);

   d.stencilDefined = true;
   d.stencilEnable = true;
   d.stencilMask = RenderParticleMgr::ParticleSystemStencilMask;
   d.stencilWriteMask = RenderParticleMgr::ParticleSystemStencilMask;
   d.stencilFunc = GFXCmpAlways;
   d.stencilPassOp = GFXStencilOpZero;

   mStencilClearSB = GFX->createStateBlock(d);
}

void RenderParticleMgr::renderInstance(ParticleRenderInst *ri, SceneRenderState *state)
{
   // Draw system path, or draw composite path
   if(ri->systemState == PSS_DrawComplete)
      return;

   if(ri->systemState != PSS_AwaitingCompositeDraw)
   {
      // Set proper stateblock, and update state
      if(ri->systemState == PSS_AwaitingOffscreenDraw)
      {
         GFX->setStateBlock( _getOffscreenStateBlock(ri) );
         ri->systemState = PSS_AwaitingCompositeDraw;
         mParticleShaderConsts.mShaderConsts->setSafe( mParticleShaderConsts.mModelViewProjSC, 
           *ri->modelViewProj * mOffscreenSystems[ri->targetIndex].clipMatrix );
      }
      else
      {
         if(ri->systemState == PSS_AwaitingMixedResDraw)
            GFX->setStateBlock( _getMixedResStateBlock( ri ) );
         else
            GFX->setStateBlock( _getHighResStateBlock( ri ) );
         ri->systemState = PSS_DrawComplete;
         mParticleShaderConsts.mShaderConsts->setSafe( mParticleShaderConsts.mModelViewProjSC, *ri->modelViewProj );
      }

      // We want to turn everything into variation on a pre-multiplied alpha blend
      F32 alphaFactor = 0.0f, alphaScale = 1.0f;
      switch(ri->blendStyle)
      {
         // SrcAlpha, InvSrcAlpha
      case ParticleRenderInst::BlendNormal:
         alphaFactor = 1.0f;
         break;

         // SrcAlpha, One
      case ParticleRenderInst::BlendAdditive:
         alphaFactor = 1.0f;
         alphaScale = 0.0f;
         break;

         // SrcColor, One
      case ParticleRenderInst::BlendGreyscale:
         alphaFactor = -1.0f;
         alphaScale = 0.0f;
         break;
      }
      mParticleShaderConsts.mShaderConsts->setSafe( mParticleShaderConsts.mAlphaFactorSC, alphaFactor );
      mParticleShaderConsts.mShaderConsts->setSafe( mParticleShaderConsts.mAlphaScaleSC, alphaScale );

      mParticleShaderConsts.mShaderConsts->setSafe( mParticleShaderConsts.mFSModelViewProjSC, *ri->modelViewProj  );
      mParticleShaderConsts.mShaderConsts->setSafe( mParticleShaderConsts.mOneOverFarSC, 1.0f / state->getFarPlane() );     

      if ( mParticleShaderConsts.mOneOverSoftnessSC->isValid() )
      {
         F32 oneOverSoftness = 1.0f;
         if ( ri->softnessDistance > 0.0f )
            oneOverSoftness = 1.0f / ( ri->softnessDistance / state->getFarPlane() );
         mParticleShaderConsts.mShaderConsts->set( mParticleShaderConsts.mOneOverSoftnessSC, oneOverSoftness );
      }

      GFX->setShader( mParticleShader );
      GFX->setShaderConstBuffer( mParticleShaderConsts.mShaderConsts );

      GFX->setTexture( 0, ri->diffuseTex );

      // Set up the prepass texture.
      if ( mParticleShaderConsts.mPrePassTargetParamsSC->isValid() )
      {
         GFXTextureObject *texObject = mPrepassTarget ? mPrepassTarget->getTexture(0) : NULL;
         GFX->setTexture( 1, texObject );

         Point4F rtParams( 0.0f, 0.0f, 1.0f, 1.0f );
         if ( texObject )
            ScreenSpace::RenderTargetParameters(texObject->getSize(), mPrepassTarget->getViewport(), rtParams);

         mParticleShaderConsts.mShaderConsts->set( mParticleShaderConsts.mPrePassTargetParamsSC, rtParams );
      }

      GFX->setPrimitiveBuffer( *ri->primBuff );
      GFX->setVertexBuffer( *ri->vertBuff );

      GFX->drawIndexedPrimitive( GFXTriangleList, 0, 0, ri->count * 4, 0, ri->count * 2 );
   }
   else if(ri->systemState == PSS_AwaitingCompositeDraw)
   {
      OffscreenSystemEntry &systemEntry = mOffscreenSystems[ri->targetIndex];

      // If this system has already been composited this frame, skip it
      if(systemEntry.drawnThisFrame)
         return;

      // Non-target render, composite the particle system back into the scene
      GFX->setVertexBuffer(mScreenQuadVertBuff);
      GFX->setPrimitiveBuffer(mScreenQuadPrimBuff);


      // Set up shader constants
      mParticleCompositeShaderConsts.mShaderConsts->setSafe( mParticleCompositeShaderConsts.mScreenRect, *((Point4F *)&systemEntry.screenRect) );

      // Set offscreen texture
      Point4F rtParams;
      GFXTextureObject *particleSource = mNamedTarget.getTexture();
      GFX->setTexture( 0, particleSource );
      if(particleSource)
      {
         ScreenSpace::RenderTargetParameters(particleSource->getSize(), mNamedTarget.getViewport(), rtParams);
         mParticleCompositeShaderConsts.mShaderConsts->setSafe( mParticleCompositeShaderConsts.mOffscreenTargetParamsSC, rtParams );
      }

      // And edges
      GFXTextureObject *texObject = mEdgeTarget ? mEdgeTarget->getTexture() : NULL;
      GFX->setTexture( 1, texObject );
      if(texObject)
      {
         ScreenSpace::RenderTargetParameters(texObject->getSize(), mEdgeTarget->getViewport(), rtParams);
         mParticleCompositeShaderConsts.mShaderConsts->setSafe( mParticleCompositeShaderConsts.mEdgeTargetParamsSC, rtParams );
      }

      // Set shader and constant buffer
      GFX->setShader( mParticleCompositeShader );
      GFX->setShaderConstBuffer( mParticleCompositeShaderConsts.mShaderConsts );

      // Draw to stencil buffer only to clear the stencil values
      GFX->setStateBlock( mStencilClearSB );
      GFX->drawIndexedPrimitive( GFXTriangleStrip, 0, 0, 4, 0, 2 );

      // composite particle system back into the scene
      GFX->setStateBlock( _getCompositeStateBlock(ri) );
      GFX->drawIndexedPrimitive( GFXTriangleStrip, 0, 0, 4, 0, 2 );

      // Re-draw the particle systems in high-res, but only to the stenciled
      // areas which were enabled via the edge buffer
      for( Vector<ParticleRenderInst *>::const_iterator itr = systemEntry.pInstances.begin();
         itr != systemEntry.pInstances.end(); itr++ )
      {
         ParticleRenderInst *pri = *itr;
         if(pri->systemState == PSS_AwaitingCompositeDraw)
         {
            pri->systemState = PSS_AwaitingMixedResDraw;
            renderInstance(pri, state);
         }
      }

      // Mark this system as having been composited this frame
      systemEntry.drawnThisFrame = true;
   }
}

bool RenderParticleMgr::_initShader()
{
   ShaderData *shaderData = NULL;
   bool ret = true;

   // Need depth from pre-pass, so get the macros
   Vector<GFXShaderMacro> macros;
   if ( mPrepassTarget )
      mPrepassTarget->getShaderMacros( &macros );

   // Create particle shader
   if ( !Sim::findObject( "ParticlesShaderData", shaderData ) || !shaderData )
      Con::warnf( "RenderParticleMgr::_initShader - failed to locate shader ParticlesShaderData!" );
   if( shaderData )
      mParticleShader = shaderData->getShader( macros );
   ret &= (mParticleShader != NULL);

   if ( mParticleShader )
   {
      mParticleShaderConsts.mShaderConsts = mParticleShader->allocConstBuffer();
      mParticleShaderConsts.mModelViewProjSC = mParticleShader->getShaderConstHandle( "$modelViewProj" );
      mParticleShaderConsts.mOneOverFarSC = mParticleShader->getShaderConstHandle( "$oneOverFar" );
      mParticleShaderConsts.mOneOverSoftnessSC = mParticleShader->getShaderConstHandle( "$oneOverSoftness" );
      mParticleShaderConsts.mAlphaFactorSC = mParticleShader->getShaderConstHandle( "$alphaFactor" );
      mParticleShaderConsts.mAlphaScaleSC = mParticleShader->getShaderConstHandle( "$alphaScale" );
      mParticleShaderConsts.mFSModelViewProjSC = mParticleShader->getShaderConstHandle( "$fsModelViewProj" );
      mParticleShaderConsts.mPrePassTargetParamsSC = mParticleShader->getShaderConstHandle( "$prePassTargetParams" );
   }

   shaderData = NULL;

   // Create off screen particle composite shader
   if ( !Sim::findObject( "OffscreenParticleCompositeShaderData", shaderData ) || !shaderData )
      Con::warnf( "RenderParticleMgr::_initShader - failed to locate shader OffscreenParticleCompositeShaderData!" );
   if( shaderData )
      mParticleCompositeShader = shaderData->getShader( macros );
   ret &= (mParticleCompositeShader != NULL);

   if ( mParticleCompositeShader )
   {
      mParticleCompositeShaderConsts.mShaderConsts = mParticleCompositeShader->allocConstBuffer();
      mParticleCompositeShaderConsts.mScreenRect = mParticleCompositeShader->getShaderConstHandle( "$screenRect" );
      mParticleCompositeShaderConsts.mEdgeTargetParamsSC = mParticleCompositeShader->getShaderConstHandle( "$edgeTargetParams" );
      mParticleCompositeShaderConsts.mOffscreenTargetParamsSC = mParticleCompositeShader->getShaderConstHandle( "$offscreenTargetParams" );
   }

   return ret;
}

void RenderParticleMgr::_onLMActivate( const char*, bool activate )
{
   RenderPassManager *rpm = getRenderPass();
   if ( !rpm )
      return;

   // Hunt for the pre-pass manager/target
   RenderPrePassMgr *prePassBin = NULL;
   for( U32 i = 0; i < rpm->getManagerCount(); i++ )
   {
      RenderBinManager *bin = rpm->getManager(i);
      if( bin->getRenderInstType() == RenderPrePassMgr::RIT_PrePass )
      {
         prePassBin = (RenderPrePassMgr*)bin;
         break;
      }
   }

   // If we found the prepass bin, set this bin to render very shortly afterwards
   // and re-add this render-manager. If there is no pre-pass bin, or it doesn't
   // have a depth-texture, we can't render offscreen.
   mOffscreenRenderEnabled = prePassBin && (prePassBin->getTargetChainLength() > 0);
   if(mOffscreenRenderEnabled)
   {
      rpm->removeManager(this);
      setRenderOrder( prePassBin->getRenderOrder() + 0.011f );
      rpm->addManager(this);
   }

   // Find the targets we use
   mPrepassTarget = NamedTexTarget::find( "prepass" );
   mEdgeTarget = NamedTexTarget::find( "edge" );

   // Setup the shader
   if ( activate ) 
      _initShader();

   if ( mScreenQuadVertBuff.isNull() )
      _initGFXResources();
}

GFXStateBlockRef RenderParticleMgr::_getOffscreenStateBlock(ParticleRenderInst *ri)
{
   const U8 blendStyle = ri->blendStyle;
   if ( mOffscreenBlocks[blendStyle].isValid() )
      return mOffscreenBlocks[blendStyle];

   GFXStateBlockDesc d;

   d.setCullMode(GFXCullNone);
   d.setZReadWrite(false, false); // No zreads or writes, all z-testing is done in the pixel shader

   // Draw everything either subtractive, or using a variation on premultiplied
   // alpha
   if(blendStyle == ParticleRenderInst::BlendSubtractive)
      d.setBlend(true, GFXBlendZero, GFXBlendInvSrcColor);
   else
      d.setBlend(true, GFXBlendOne, GFXBlendInvSrcAlpha);

   // Offscreen target, we need to add alpha.
   d.separateAlphaBlendDefined = true;
   d.separateAlphaBlendEnable = true;
   d.separateAlphaBlendSrc = GFXBlendOne;
   d.separateAlphaBlendDest = GFXBlendInvSrcAlpha;

   d.samplersDefined = true;

   // Diffuse texture sampler
   d.samplers[0] = GFXSamplerStateDesc::getClampLinear();
   d.samplers[0].alphaOp = GFXTOPModulate;
   d.samplers[0].alphaArg1 = GFXTATexture;
   d.samplers[0].alphaArg2 = GFXTADiffuse;

   // Prepass sampler
   d.samplers[1] = GFXSamplerStateDesc::getClampPoint();

   mOffscreenBlocks[blendStyle] = GFX->createStateBlock(d);
   return mOffscreenBlocks[blendStyle];
}

GFXStateBlockRef RenderParticleMgr::_getHighResStateBlock(ParticleRenderInst *ri)
{
   const U8 blendStyle = ri->blendStyle;
   if ( mHighResBlocks[blendStyle].isValid() )
      return mHighResBlocks[blendStyle];

   GFXStateBlockDesc d;

   d.setZReadWrite(true, false);
   d.setCullMode(GFXCullNone);

   // Draw everything either subtractive, or using a variation on premultiplied
   // alpha
   if(blendStyle == ParticleRenderInst::BlendSubtractive)
      d.setBlend(true, GFXBlendZero, GFXBlendInvSrcColor);
   else
      d.setBlend(true, GFXBlendOne, GFXBlendInvSrcAlpha);

   d.samplersDefined = true;

   // Diffuse texture sampler
   d.samplers[0] = GFXSamplerStateDesc::getClampLinear();
   d.samplers[0].alphaOp = GFXTOPModulate;
   d.samplers[0].alphaArg1 = GFXTATexture;
   d.samplers[0].alphaArg2 = GFXTADiffuse;

   // Prepass sampler
   d.samplers[1] = GFXSamplerStateDesc::getClampPoint();

   mHighResBlocks[blendStyle] = GFX->createStateBlock(d);
   return mHighResBlocks[blendStyle];
}

GFXStateBlockRef RenderParticleMgr::_getMixedResStateBlock(ParticleRenderInst *ri)
{
   const U8 blendStyle = ri->blendStyle;
   if ( mHighResBlocks[blendStyle].isValid() )
      return mHighResBlocks[blendStyle];

   GFXStateBlockDesc d;

   d.setZReadWrite(true, false);
   d.setCullMode(GFXCullNone);

   /*
   // Old blend styles...
   switch (blendStyle)
   {
   case ParticleRenderInst::BlendNormal:
      d.blendSrc = GFXBlendSrcAlpha;
      d.blendDest = GFXBlendInvSrcAlpha;
      break;

   case ParticleRenderInst::BlendSubtractive:
      d.blendSrc = GFXBlendZero;
      d.blendDest = GFXBlendInvSrcColor;
      break;

   case ParticleRenderInst::BlendPremultAlpha:
      d.blendSrc = GFXBlendOne;
      d.blendDest = GFXBlendInvSrcAlpha;
      break;

      // Default to additive blend mode
   case ParticleRenderInst::BlendAdditive:
   case ParticleRenderInst::BlendUndefined:
   default:
      d.blendSrc = GFXBlendSrcAlpha;
      d.blendDest = GFXBlendOne;
      break;
   }
   */

   // Draw everything either subtractive, or using a variation on premultiplied
   // alpha
   if(blendStyle == ParticleRenderInst::BlendSubtractive)
      d.setBlend(true, GFXBlendZero, GFXBlendInvSrcColor);
   else
      d.setBlend(true, GFXBlendOne, GFXBlendInvSrcAlpha);

   // Draw to anything but the stencil ref value (the edges)
   d.stencilDefined = true;
   d.stencilEnable = true;
   d.stencilRef = RenderParticleMgr::HighResStencilRef;
   d.stencilMask = RenderParticleMgr::ParticleSystemStencilMask;
   d.stencilPassOp = GFXStencilOpKeep;
   d.stencilFailOp = GFXStencilOpKeep;
   d.stencilZFailOp = GFXStencilOpKeep;
   d.stencilFunc = GFXCmpNotEqual;

   d.samplersDefined = true;

   // Diffuse texture sampler
   d.samplers[0] = GFXSamplerStateDesc::getClampLinear();
   d.samplers[0].alphaOp = GFXTOPModulate;
   d.samplers[0].alphaArg1 = GFXTATexture;
   d.samplers[0].alphaArg2 = GFXTADiffuse;

   // Prepass sampler
   d.samplers[1] = GFXSamplerStateDesc::getClampPoint();

   mHighResBlocks[blendStyle] = GFX->createStateBlock(d);
   return mHighResBlocks[blendStyle];
}

GFXStateBlockRef RenderParticleMgr::_getCompositeStateBlock(ParticleRenderInst *ri)
{
   const U8 blendStyle = ri->blendStyle;
   if ( mBackbufferBlocks[blendStyle].isValid() )
      return mBackbufferBlocks[blendStyle];

   GFXStateBlockDesc d;

   // This is a billboard
   d.setCullMode(GFXCullNone);
   d.setZReadWrite(false, false);

   // When we re-composite the particles, it is always either a pre-mult alpha
   // blend, or a subtractive blend!
   if(blendStyle == ParticleRenderInst::BlendSubtractive)
      d.setBlend(true, GFXBlendZero, GFXBlendInvSrcColor);
   else
      d.setBlend(true, GFXBlendOne, GFXBlendInvSrcAlpha);

   // All areas which are not along the edges of geometry where the particle system
   // is being drawn get assigned a stencil ref value as the system is composited
   // back into the scene. The high-res stateblock uses this value as a mask, and
   // draws only in areas which are NOT this ref value. This causes high resolution
   // draws to ONLY the edge areas.
   d.stencilDefined = true;
   d.stencilEnable = true;
   d.stencilRef = RenderParticleMgr::HighResStencilRef; 
   d.stencilWriteMask = RenderParticleMgr::ParticleSystemStencilMask;
   d.stencilMask = RenderParticleMgr::ParticleSystemStencilMask;
   d.stencilPassOp = GFXStencilOpReplace;
   d.stencilFunc = GFXCmpGreater;

   // Diffuse texture sampler and 
   d.samplersDefined = true;
   d.samplers[0] = GFXSamplerStateDesc::getClampLinear();
   d.samplers[1] = GFXSamplerStateDesc::getClampLinear();

   mBackbufferBlocks[blendStyle] = GFX->createStateBlock(d);
   return mBackbufferBlocks[blendStyle];
}

bool RenderParticleMgr::_handleGFXEvent( GFXDevice::GFXDeviceEventType event )
{
   if(RenderToSingleTarget)
      return Parent::_handleGFXEvent( event );

   // Do nothing. This render manager uses its target chain as a pool of targets.
   return true;
}