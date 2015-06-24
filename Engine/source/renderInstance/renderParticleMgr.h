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

#ifndef _RENDER_PARTICLE_MGR_H_
#define _RENDER_PARTICLE_MGR_H_

#ifndef _TEXTARGETBIN_MGR_H_
#include "renderInstance/renderTexTargetBinManager.h"
#endif

#ifndef _GFXPRIMITIVEBUFFER_H_
#include "gfx/gfxPrimitiveBuffer.h"
#endif

GFXDeclareVertexFormat( CompositeQuadVert )
{
   GFXVertexColor uvCoord;
};

class RenderParticleMgr : public RenderTexTargetBinManager
{
   typedef RenderTexTargetBinManager Parent;
   friend class RenderTranslucentMgr;

public:
   // Generic PrePass Render Instance Type
   static const RenderInstType RIT_Particles;

   RenderParticleMgr();
   RenderParticleMgr( F32 renderOrder, F32 processAddOrder );
   virtual ~RenderParticleMgr();

   // RenderBinManager
   void render(SceneRenderState * state);
   void sort();
   void clear();
   void addElement( RenderInst *inst );

   // ConsoleObject
   DECLARE_CONOBJECT(RenderParticleMgr);

   const static U8 HighResStencilRef = 0x80;
   const static U8 ParticleSystemStencilMask = 0x80; // We are using the top bit
   const static U32 OffscreenPoolSize = 5;

   virtual void setTargetChainLength(const U32 chainLength);

protected:

   // Override
   virtual bool _handleGFXEvent(GFXDevice::GFXDeviceEventType event);

   bool _initShader();
   void _initGFXResources();
   void _onLMActivate( const char*, bool activate );


   // Not only a helper method, but a method for the RenderTranslucentMgr to
   // request a particle system draw
   void renderInstance(ParticleRenderInst *ri, SceneRenderState *state);
public:
   void renderParticle(ParticleRenderInst *ri, SceneRenderState *state);
protected:
   bool mOffscreenRenderEnabled;

   /// The prepass render target used for the
   /// soft particle shader effect.
   NamedTexTargetRef mPrepassTarget;

   /// The shader used for particle rendering.
   GFXShaderRef mParticleShader;

   GFXShaderRef mParticleCompositeShader;
   NamedTexTargetRef mEdgeTarget;

   struct OffscreenSystemEntry
   {
      S32 targetChainIdx;
      MatrixF clipMatrix;
      RectF screenRect; 
      bool drawnThisFrame;
      Vector<ParticleRenderInst *> pInstances;
   };
   Vector<OffscreenSystemEntry> mOffscreenSystems;

   struct ShaderConsts
   {
      GFXShaderConstBufferRef mShaderConsts;
      GFXShaderConstHandle *mModelViewProjSC;
      GFXShaderConstHandle *mFSModelViewProjSC;
      GFXShaderConstHandle *mOneOverFarSC;
      GFXShaderConstHandle *mOneOverSoftnessSC;
      GFXShaderConstHandle *mPrePassTargetParamsSC;
      GFXShaderConstHandle *mAlphaFactorSC;
      GFXShaderConstHandle *mAlphaScaleSC;
      GFXShaderConstHandle *mSamplerDiffuse;
      GFXShaderConstHandle *mSamplerPrePassTex;
      GFXShaderConstHandle *mSamplerParaboloidLightMap;

   } mParticleShaderConsts;

   struct CompositeShaderConsts
   {
      GFXShaderConstBufferRef mShaderConsts;
      GFXShaderConstHandle *mSystemDepth;
      GFXShaderConstHandle *mScreenRect;
      GFXShaderConstHandle *mSamplerColorSource;
      GFXShaderConstHandle *mSamplerEdgeSource;
      GFXShaderConstHandle *mEdgeTargetParamsSC;
      GFXShaderConstHandle *mOffscreenTargetParamsSC;
   } mParticleCompositeShaderConsts;

   GFXVertexBufferHandle<CompositeQuadVert> mScreenQuadVertBuff;
   GFXPrimitiveBufferHandle mScreenQuadPrimBuff;

   GFXStateBlockRef mStencilClearSB;
   GFXStateBlockRef mHighResBlocks[ParticleRenderInst::BlendStyle_COUNT];
   GFXStateBlockRef mOffscreenBlocks[ParticleRenderInst::BlendStyle_COUNT];
   GFXStateBlockRef mBackbufferBlocks[ParticleRenderInst::BlendStyle_COUNT];
   GFXStateBlockRef mMixedResBlocks[ParticleRenderInst::BlendStyle_COUNT];

public:
   GFXStateBlockRef _getHighResStateBlock(ParticleRenderInst *ri);
   GFXStateBlockRef _getMixedResStateBlock(ParticleRenderInst *ri);
   GFXStateBlockRef _getOffscreenStateBlock(ParticleRenderInst *ri);
   GFXStateBlockRef _getCompositeStateBlock(ParticleRenderInst *ri);
   ShaderConsts &_getShaderConsts() { return mParticleShaderConsts; };
};


#endif // _RENDER_TRANSLUCENT_MGR_H_
