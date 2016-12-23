//-----------------------------------------------------------------------------
// Copyright (c) 2015 GarageGames, LLC
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
#include "gfx/D3D11/gfxD3D11StateBlock.h"
#include "gfx/D3D11/gfxD3D11EnumTranslate.h"
#include "core/crc.h"

namespace DictHash
{
   inline U32 hash(const GFXSamplerStateDesc &data)
   {
      return CRC::calculateCRC(&data, sizeof(GFXSamplerStateDesc));;
   }
}

GFXD3D11StateBlock::GFXD3D11StateBlock(const GFXStateBlockDesc& desc)
{
   AssertFatal(D3D11DEVICE, "Invalid D3DDevice!");
   PROFILE_SCOPE(GFXD3D11StateBlock_CreateStateBlock);

   mDesc = desc;
   mCachedHashValue = desc.getHashValue();

   // Color writes
   mColorMask = 0;
   mColorMask |= (mDesc.colorWriteRed ? D3D11_COLOR_WRITE_ENABLE_RED : 0);
   mColorMask |= (mDesc.colorWriteGreen ? D3D11_COLOR_WRITE_ENABLE_GREEN : 0);
   mColorMask |= (mDesc.colorWriteBlue ? D3D11_COLOR_WRITE_ENABLE_BLUE : 0);
   mColorMask |= (mDesc.colorWriteAlpha ? D3D11_COLOR_WRITE_ENABLE_ALPHA : 0);

   mBlendState = NULL;
   for (U32 i = 0; i < 16; i++)
   {
      mSamplerStates[i] = NULL;
   }

   mDepthStencilState = NULL;
   mRasterizerState = NULL;

   ZeroMemory(&mBlendDesc, sizeof(D3D11_BLEND_DESC));
   mBlendDesc.AlphaToCoverageEnable = false;
   mBlendDesc.IndependentBlendEnable = mDesc.separateAlphaBlendEnable;

   mBlendDesc.RenderTarget[0].BlendEnable = mDesc.blendEnable;
   mBlendDesc.RenderTarget[0].BlendOp = GFXD3D11BlendOp[mDesc.blendOp];
   mBlendDesc.RenderTarget[0].BlendOpAlpha = GFXD3D11BlendOp[mDesc.separateAlphaBlendOp];
   mBlendDesc.RenderTarget[0].DestBlend = GFXD3D11Blend[mDesc.blendDest];
   mBlendDesc.RenderTarget[0].DestBlendAlpha = GFXD3D11Blend[mDesc.separateAlphaBlendDest];
   mBlendDesc.RenderTarget[0].SrcBlend = GFXD3D11Blend[mDesc.blendSrc];
   mBlendDesc.RenderTarget[0].SrcBlendAlpha = GFXD3D11Blend[mDesc.separateAlphaBlendSrc];
   mBlendDesc.RenderTarget[0].RenderTargetWriteMask = mColorMask;

   HRESULT hr = D3D11DEVICE->CreateBlendState(&mBlendDesc, &mBlendState);

   if (FAILED(hr))
   {
      AssertFatal(false, "GFXD3D11StateBlock::GFXD3D11StateBlock - CreateBlendState call failure.");
   }

   ZeroMemory(&mDepthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
   mDepthStencilDesc.DepthWriteMask = mDesc.zWriteEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
   mDepthStencilDesc.DepthEnable = mDesc.zEnable;
   mDepthStencilDesc.DepthFunc = GFXD3D11CmpFunc[mDesc.zFunc];
   mDepthStencilDesc.StencilWriteMask = mDesc.stencilWriteMask;
   mDepthStencilDesc.StencilReadMask = mDesc.stencilMask;
   mDepthStencilDesc.StencilEnable = mDesc.stencilEnable;

   mDepthStencilDesc.FrontFace.StencilFunc = GFXD3D11CmpFunc[mDesc.stencilFunc];
   mDepthStencilDesc.FrontFace.StencilFailOp = GFXD3D11StencilOp[mDesc.stencilFailOp];
   mDepthStencilDesc.FrontFace.StencilDepthFailOp = GFXD3D11StencilOp[mDesc.stencilZFailOp];
   mDepthStencilDesc.FrontFace.StencilPassOp = GFXD3D11StencilOp[mDesc.stencilPassOp];

   if (mDesc.stencilEnable)
      mDepthStencilDesc.BackFace = mDepthStencilDesc.FrontFace;
   else
   {
      mDepthStencilDesc.BackFace.StencilFunc = GFXD3D11CmpFunc[GFXCmpAlways];
      mDepthStencilDesc.BackFace.StencilFailOp = GFXD3D11StencilOp[GFXStencilOpKeep];
      mDepthStencilDesc.BackFace.StencilDepthFailOp = GFXD3D11StencilOp[GFXStencilOpKeep];
      mDepthStencilDesc.BackFace.StencilPassOp = GFXD3D11StencilOp[GFXStencilOpKeep];
   }

   hr = D3D11DEVICE->CreateDepthStencilState(&mDepthStencilDesc, &mDepthStencilState);

   if (FAILED(hr))
   {
      AssertFatal(false, "GFXD3D11StateBlock::GFXD3D11StateBlock - CreateDepthStencilState call failure.");
   }

   ZeroMemory(&mRasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
   mRasterizerDesc.CullMode = GFXD3D11CullMode[mDesc.cullMode];
   mRasterizerDesc.FillMode = GFXD3D11FillMode[mDesc.fillMode];
   mRasterizerDesc.DepthBias = mDesc.zBias;
   mRasterizerDesc.SlopeScaledDepthBias = mDesc.zSlopeBias;
   mRasterizerDesc.AntialiasedLineEnable = FALSE;
   mRasterizerDesc.MultisampleEnable = FALSE;
   mRasterizerDesc.ScissorEnable = FALSE;
   mRasterizerDesc.FrontCounterClockwise = FALSE;
   mRasterizerDesc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;

   if (mDesc.zEnable)
      mRasterizerDesc.DepthClipEnable = true;
   else
      mRasterizerDesc.DepthClipEnable = false;

   hr = D3D11DEVICE->CreateRasterizerState(&mRasterizerDesc, &mRasterizerState);

   if (FAILED(hr))
   {
      AssertFatal(false, "GFXD3D11StateBlock::GFXD3D11StateBlock - CreateDepthStencilState call failure.");
   }

   ZeroMemory(&mSamplerDesc, sizeof(D3D11_SAMPLER_DESC) * 16);

   GFXD3D11Device::SamplerMap &dx11SamplerMap = D3D11->getSamplersMap();

   for (U32 i = 0; i < GFX->getNumSamplers(); i++)
   {
      GFXSamplerStateDesc &gfxSamplerState = mDesc.samplers[i];
      U32 hash = DictHash::hash(gfxSamplerState);
      GFXD3D11Device::SamplerMap::Iterator itr = dx11SamplerMap.find(hash);

      if (itr == dx11SamplerMap.end())
      {
         mSamplerDesc[i].AddressU = GFXD3D11TextureAddress[gfxSamplerState.addressModeU];
         mSamplerDesc[i].AddressV = GFXD3D11TextureAddress[gfxSamplerState.addressModeV];
         mSamplerDesc[i].AddressW = GFXD3D11TextureAddress[gfxSamplerState.addressModeW];
         mSamplerDesc[i].MaxAnisotropy = gfxSamplerState.maxAnisotropy;

         mSamplerDesc[i].MipLODBias = gfxSamplerState.mipLODBias;
         mSamplerDesc[i].MinLOD = 0;
         mSamplerDesc[i].MaxLOD = FLT_MAX;

         if (gfxSamplerState.magFilter == GFXTextureFilterPoint && gfxSamplerState.minFilter == GFXTextureFilterPoint && gfxSamplerState.mipFilter == GFXTextureFilterPoint)
            mSamplerDesc[i].Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
         else if (gfxSamplerState.magFilter == GFXTextureFilterPoint && gfxSamplerState.minFilter == GFXTextureFilterPoint && gfxSamplerState.mipFilter == GFXTextureFilterLinear)
            mSamplerDesc[i].Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
         else if (gfxSamplerState.magFilter == GFXTextureFilterLinear && gfxSamplerState.minFilter == GFXTextureFilterPoint && gfxSamplerState.mipFilter == GFXTextureFilterPoint)
            mSamplerDesc[i].Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
         else if (gfxSamplerState.magFilter == GFXTextureFilterLinear && gfxSamplerState.minFilter == GFXTextureFilterPoint && gfxSamplerState.mipFilter == GFXTextureFilterLinear)
            mSamplerDesc[i].Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
         else if (gfxSamplerState.magFilter == GFXTextureFilterPoint && gfxSamplerState.minFilter == GFXTextureFilterLinear && gfxSamplerState.mipFilter == GFXTextureFilterPoint)
            mSamplerDesc[i].Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
         else if (gfxSamplerState.magFilter == GFXTextureFilterPoint && gfxSamplerState.minFilter == GFXTextureFilterLinear && gfxSamplerState.mipFilter == GFXTextureFilterLinear)
            mSamplerDesc[i].Filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
         else if (gfxSamplerState.magFilter == GFXTextureFilterLinear && gfxSamplerState.minFilter == GFXTextureFilterLinear && gfxSamplerState.mipFilter == GFXTextureFilterPoint)
            mSamplerDesc[i].Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
         else if (gfxSamplerState.magFilter == GFXTextureFilterLinear && gfxSamplerState.minFilter == GFXTextureFilterLinear && mDesc.samplers[i].mipFilter == GFXTextureFilterLinear)
            mSamplerDesc[i].Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
         else
            mSamplerDesc[i].Filter = D3D11_FILTER_ANISOTROPIC;

         mSamplerDesc[i].BorderColor[0] = 1.0f;
         mSamplerDesc[i].BorderColor[1] = 1.0f;
         mSamplerDesc[i].BorderColor[2] = 1.0f;
         mSamplerDesc[i].BorderColor[3] = 1.0f;

         hr = D3D11DEVICE->CreateSamplerState(&mSamplerDesc[i], &mSamplerStates[i]);
         if (FAILED(hr))
         {
            AssertFatal(false, "GFXD3D11StateBlock::GFXD3D11StateBlock - CreateSamplerState call failure.");
         }

         // add sampler state to the map
         dx11SamplerMap.insert(hash, mSamplerStates[i]);
      }
      else
      {
         mSamplerStates[i] = itr->value;
      }
   }
}

GFXD3D11StateBlock::~GFXD3D11StateBlock()
{
   SAFE_RELEASE(mBlendState);
   SAFE_RELEASE(mRasterizerState);
   SAFE_RELEASE(mDepthStencilState);

   for (U32 i = 0; i < 16; ++i)
   {
      mSamplerStates[i] = NULL;
   }
}

/// Returns the hash value of the desc that created this block
U32 GFXD3D11StateBlock::getHashValue() const
{
   return mCachedHashValue;
}

/// Returns a GFXStateBlockDesc that this block represents
const GFXStateBlockDesc& GFXD3D11StateBlock::getDesc() const
{
   return mDesc;
}

/// Called by D3D11 device to active this state block.
/// @param oldState  The current state, used to make sure we don't set redundant states on the device.  Pass NULL to reset all states.
void GFXD3D11StateBlock::activate(GFXD3D11StateBlock* oldState)
{
   PROFILE_SCOPE(GFXD3D11StateBlock_Activate);

   if (!oldState || (mBlendState != oldState->mBlendState))
   {
      F32 blendFactor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
      D3D11DEVICECONTEXT->OMSetBlendState(mBlendState, blendFactor, 0xFFFFFFFF);
   }

   if (!oldState || (mDepthStencilState != oldState->mDepthStencilState))
      D3D11DEVICECONTEXT->OMSetDepthStencilState(mDepthStencilState, mDesc.stencilRef);

   if (!oldState || (mRasterizerState != oldState->mRasterizerState))
      D3D11DEVICECONTEXT->RSSetState(mRasterizerState);

   U32 numSamplersChanged = 0;
   U32 numSamplers = GFX->getNumSamplers();
   U32 samplerUpdateStartSlot = 0;

   //figure out which range of samplers changed.
   for (U32 samplerSlot = 0; samplerSlot < numSamplers; samplerSlot++)
   {

      if (oldState && (oldState->mSamplerStates[samplerSlot] == mSamplerStates[samplerSlot]))
      {
         //only change the update start slot when there hasn't been any samplers changed so far
         if (numSamplersChanged == 0) {
            samplerUpdateStartSlot++;
         }
         
         continue;
      }

      numSamplersChanged = (samplerSlot - samplerUpdateStartSlot) + 1;
   }

   //TODO samplers for vertex shader
   // Set all the samplers with one call
   //D3D11DEVICECONTEXT->VSSetSamplers(0, numSamplers, &mSamplerStates[0]);

   if (numSamplersChanged > 0)
      D3D11DEVICECONTEXT->PSSetSamplers(samplerUpdateStartSlot, numSamplersChanged, &mSamplerStates[samplerUpdateStartSlot]);
}





