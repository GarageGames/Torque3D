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

#include "gfx/gfxDevice.h"

#if defined(TORQUE_OS_XENON)
#  include <xtl.h>
#else
#  include <d3d9.h>
#endif

#include "gfx/D3D9/gfxD3D9StateBlock.h"
#include "gfx/D3D9/gfxD3D9EnumTranslate.h"

GFXD3D9StateBlock::GFXD3D9StateBlock(const GFXStateBlockDesc& desc, LPDIRECT3DDEVICE9 d3dDevice)
{
   AssertFatal(d3dDevice, "Invalid mD3DDevice!");

   mDesc = desc;
   mCachedHashValue = desc.getHashValue();
   mD3DDevice = d3dDevice;

   // Color writes
   mColorMask = 0; 
   mColorMask |= ( mDesc.colorWriteRed   ? GFXCOLORWRITEENABLE_RED   : 0 );
   mColorMask |= ( mDesc.colorWriteGreen ? GFXCOLORWRITEENABLE_GREEN : 0 );
   mColorMask |= ( mDesc.colorWriteBlue  ? GFXCOLORWRITEENABLE_BLUE  : 0 );
   mColorMask |= ( mDesc.colorWriteAlpha ? GFXCOLORWRITEENABLE_ALPHA : 0 );

   // Z*bias
   mZBias = *((U32*)&mDesc.zBias);
   mZSlopeBias = *((U32*)&mDesc.zSlopeBias);
}

GFXD3D9StateBlock::~GFXD3D9StateBlock()
{
   
}

/// Returns the hash value of the desc that created this block
U32 GFXD3D9StateBlock::getHashValue() const
{
   return mCachedHashValue;
}

/// Returns a GFXStateBlockDesc that this block represents
const GFXStateBlockDesc& GFXD3D9StateBlock::getDesc() const
{
   return mDesc;      
}

/// Called by D3D9 device to active this state block.
/// @param oldState  The current state, used to make sure we don't set redundant states on the device.  Pass NULL to reset all states.
void GFXD3D9StateBlock::activate(GFXD3D9StateBlock* oldState)
{
   PROFILE_SCOPE( GFXD3D9StateBlock_Activate );

   // Little macro to save some typing, SD = state diff, checks for null source state block, then
   // checks to see if the states differ 
#if defined(TORQUE_OS_XENON)
   #define SD(x, y)  if (!oldState || oldState->mDesc.x != mDesc.x) \
                     mD3DDevice->SetRenderState_Inline(y, mDesc.x)

      // Same as above, but allows you to set the data
   #define SDD(x, y, z) if (!oldState || oldState->mDesc.x != mDesc.x) \
                        mD3DDevice->SetRenderState_Inline(y, z)
#else
   #define SD(x, y)  if (!oldState || oldState->mDesc.x != mDesc.x) \
                     mD3DDevice->SetRenderState(y, mDesc.x)

   // Same as above, but allows you to set the data
   #define SDD(x, y, z) if (!oldState || oldState->mDesc.x != mDesc.x) \
                        mD3DDevice->SetRenderState(y, z)
#endif

   // Blending
   SD(blendEnable, D3DRS_ALPHABLENDENABLE);
   SDD(blendSrc, D3DRS_SRCBLEND, GFXD3D9Blend[mDesc.blendSrc]);
   SDD(blendDest, D3DRS_DESTBLEND, GFXD3D9Blend[mDesc.blendDest]);
   SDD(blendOp, D3DRS_BLENDOP, GFXD3D9BlendOp[mDesc.blendOp]);

   // Separate alpha blending
   SD(separateAlphaBlendEnable, D3DRS_SEPARATEALPHABLENDENABLE);
   SDD(separateAlphaBlendSrc, D3DRS_SRCBLENDALPHA, GFXD3D9Blend[mDesc.separateAlphaBlendSrc]);
   SDD(separateAlphaBlendDest, D3DRS_DESTBLENDALPHA, GFXD3D9Blend[mDesc.separateAlphaBlendDest]);
   SDD(separateAlphaBlendOp, D3DRS_BLENDOPALPHA, GFXD3D9BlendOp[mDesc.separateAlphaBlendOp]);

   // Alpha test
   SD(alphaTestEnable, D3DRS_ALPHATESTENABLE);
   SDD(alphaTestFunc, D3DRS_ALPHAFUNC, GFXD3D9CmpFunc[mDesc.alphaTestFunc]);
   SD(alphaTestRef, D3DRS_ALPHAREF);

   // Color writes
   if ((oldState == NULL) || (mColorMask != oldState->mColorMask))
      mD3DDevice->SetRenderState(D3DRS_COLORWRITEENABLE, mColorMask);

   // Culling
   SDD(cullMode, D3DRS_CULLMODE, GFXD3D9CullMode[mDesc.cullMode]);

   // Depth
   SD(zEnable, D3DRS_ZENABLE);   
   SD(zWriteEnable, D3DRS_ZWRITEENABLE);
   SDD(zFunc, D3DRS_ZFUNC, GFXD3D9CmpFunc[mDesc.zFunc]);   
   if ((!oldState) || (mZBias != oldState->mZBias))
      mD3DDevice->SetRenderState(D3DRS_DEPTHBIAS, mZBias);
   if ((!oldState) || (mZSlopeBias != oldState->mZSlopeBias))
      mD3DDevice->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, mZSlopeBias);

   // Stencil
   SD(stencilEnable, D3DRS_STENCILENABLE);
   SDD(stencilFailOp, D3DRS_STENCILFAIL, GFXD3D9StencilOp[mDesc.stencilFailOp]);
   SDD(stencilZFailOp, D3DRS_STENCILZFAIL, GFXD3D9StencilOp[mDesc.stencilZFailOp]);
   SDD(stencilPassOp, D3DRS_STENCILPASS, GFXD3D9StencilOp[mDesc.stencilPassOp]);
   SDD(stencilFunc, D3DRS_STENCILFUNC, GFXD3D9CmpFunc[mDesc.stencilFunc]);
   SD(stencilRef, D3DRS_STENCILREF);
   SD(stencilMask, D3DRS_STENCILMASK);
   SD(stencilWriteMask, D3DRS_STENCILWRITEMASK);
   SDD(fillMode, D3DRS_FILLMODE, GFXD3D9FillMode[mDesc.fillMode]);
#if !defined(TORQUE_OS_XENON)
   SD(ffLighting, D3DRS_LIGHTING);
   SD(vertexColorEnable, D3DRS_COLORVERTEX);

   static DWORD swzTemp;
   getOwningDevice()->getDeviceSwizzle32()->ToBuffer( &swzTemp, &mDesc.textureFactor, sizeof(ColorI) );
   SDD(textureFactor, D3DRS_TEXTUREFACTOR, swzTemp);
#endif
#undef SD
#undef SDD


   // NOTE: Samplers and Stages are different things.
   //
   // The Stages were for fixed function blending.  When using shaders
   // calling SetTextureStageState() is a complete waste of time.  In
   // fact if this function rises to the top of profiles we should
   // refactor stateblocks to seperate the two.
   //
   // Samplers are used by both fixed function and shaders, but the
   // number of samplers is limited by shader model.
#if !defined(TORQUE_OS_XENON)

   #define TSS(x, y, z) if (!oldState || oldState->mDesc.samplers[i].x != mDesc.samplers[i].x) \
                        mD3DDevice->SetTextureStageState(i, y, z)
   for ( U32 i = 0; i < 8; i++ )
   {   
      TSS(textureColorOp, D3DTSS_COLOROP, GFXD3D9TextureOp[mDesc.samplers[i].textureColorOp]);
      TSS(colorArg1, D3DTSS_COLORARG1, mDesc.samplers[i].colorArg1);
      TSS(colorArg2, D3DTSS_COLORARG2, mDesc.samplers[i].colorArg2);
      TSS(colorArg3, D3DTSS_COLORARG0, mDesc.samplers[i].colorArg3);
      TSS(alphaOp, D3DTSS_ALPHAOP, GFXD3D9TextureOp[mDesc.samplers[i].alphaOp]);
      TSS(alphaArg1, D3DTSS_ALPHAARG1, mDesc.samplers[i].alphaArg1);
      TSS(alphaArg2, D3DTSS_ALPHAARG2, mDesc.samplers[i].alphaArg2);
      TSS(alphaArg3, D3DTSS_ALPHAARG0, mDesc.samplers[i].alphaArg3);
      TSS(textureTransform, D3DTSS_TEXTURETRANSFORMFLAGS, mDesc.samplers[i].textureTransform);
      TSS(resultArg, D3DTSS_RESULTARG, mDesc.samplers[i].resultArg);
   }
   #undef TSS
#endif

#if defined(TORQUE_OS_XENON)
   #define SS(x, y, z)  if (!oldState || oldState->mDesc.samplers[i].x != mDesc.samplers[i].x) \
                        mD3DDevice->SetSamplerState_Inline(i, y, z)
#else
   #define SS(x, y, z)  if (!oldState || oldState->mDesc.samplers[i].x != mDesc.samplers[i].x) \
                        mD3DDevice->SetSamplerState(i, y, z)
#endif
   for ( U32 i = 0; i < getOwningDevice()->getNumSamplers(); i++ )
   {      
      SS(minFilter, D3DSAMP_MINFILTER, GFXD3D9TextureFilter[mDesc.samplers[i].minFilter]);
      SS(magFilter, D3DSAMP_MAGFILTER, GFXD3D9TextureFilter[mDesc.samplers[i].magFilter]);
      SS(mipFilter, D3DSAMP_MIPFILTER, GFXD3D9TextureFilter[mDesc.samplers[i].mipFilter]);

      F32 bias = mDesc.samplers[i].mipLODBias;
      DWORD dwBias = *( (LPDWORD)(&bias) );
      SS(mipLODBias, D3DSAMP_MIPMAPLODBIAS, dwBias);

      SS(maxAnisotropy, D3DSAMP_MAXANISOTROPY, mDesc.samplers[i].maxAnisotropy);

      SS(addressModeU, D3DSAMP_ADDRESSU, GFXD3D9TextureAddress[mDesc.samplers[i].addressModeU]);
      SS(addressModeV, D3DSAMP_ADDRESSV, GFXD3D9TextureAddress[mDesc.samplers[i].addressModeV]);
      SS(addressModeW, D3DSAMP_ADDRESSW, GFXD3D9TextureAddress[mDesc.samplers[i].addressModeW]);
   }
   #undef SS
}
