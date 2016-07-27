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

#ifndef _GFXD3D11STATEBLOCK_H_
#define _GFXD3D11STATEBLOCK_H_

#include "gfx/D3D11/gfxD3D11Device.h"
#include "gfx/gfxStateBlock.h"

class GFXD3D11StateBlock : public GFXStateBlock
{   
public:

   GFXD3D11StateBlock(const GFXStateBlockDesc& desc);
   virtual ~GFXD3D11StateBlock();

   /// Called by D3D11 device to active this state block.
   /// @param oldState  The current state, used to make sure we don't set redundant states on the device.  Pass NULL to reset all states.
   void activate(GFXD3D11StateBlock* oldState);

   // 
   // GFXStateBlock interface
   //

   /// Returns the hash value of the desc that created this block
   virtual U32 getHashValue() const;

   /// Returns a GFXStateBlockDesc that this block represents
   virtual const GFXStateBlockDesc& getDesc() const;

   //
   // GFXResource
   //
   virtual void zombify() { }
   /// When called the resource should restore all device sensitive information destroyed by zombify()
   virtual void resurrect() { }
private:

   D3D11_BLEND_DESC mBlendDesc;
   D3D11_RASTERIZER_DESC mRasterizerDesc;
   D3D11_DEPTH_STENCIL_DESC mDepthStencilDesc; 
   D3D11_SAMPLER_DESC mSamplerDesc[TEXTURE_STAGE_COUNT];

   ID3D11BlendState* mBlendState;
   ID3D11DepthStencilState* mDepthStencilState;
   ID3D11RasterizerState* mRasterizerState;
   ID3D11SamplerState* mSamplerStates[TEXTURE_STAGE_COUNT];

   GFXStateBlockDesc mDesc;
   U32 mCachedHashValue;
   // Cached D3D specific things, these are "calculated" from GFXStateBlock
   U32 mColorMask; 
};

typedef StrongRefPtr<GFXD3D11StateBlock> GFXD3D11StateBlockRef;

#endif