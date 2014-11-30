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

#ifndef _GFXSTATEBLOCK_H_
#define _GFXSTATEBLOCK_H_

#ifndef _REFBASE_H_
#include "core/util/refBase.h"
#endif
#ifndef _GFXENUMS_H_
#include "gfx/gfxEnums.h"
#endif
#ifndef _GFXRESOURCE_H_
#include "gfx/gfxResource.h"
#endif
#ifndef _COLOR_H_
#include "core/color.h"
#endif


struct GFXSamplerStateDesc
{
   GFXTextureAddressMode addressModeU;
   GFXTextureAddressMode addressModeV;
   GFXTextureAddressMode addressModeW;

   GFXTextureFilterType magFilter;
   GFXTextureFilterType minFilter;
   GFXTextureFilterType mipFilter;

   /// The maximum anisotropy used when one of the filter types
   /// is set to anisotropic.
   ///
   /// Defaults to 1.
   ///
   /// @see GFXTextureFilterType
   U32 maxAnisotropy;

   /// Used to offset the mipmap selection by whole or 
   /// fractional amounts either postively or negatively.
   ///
   /// Defaults to zero.
   F32 mipLODBias;

   GFXTextureOp textureColorOp;

   GFXTextureOp alphaOp;
   GFXTextureArgument alphaArg1;
   GFXTextureArgument alphaArg2;
   GFXTextureArgument alphaArg3;

   GFXTextureArgument colorArg1;
   GFXTextureArgument colorArg2;
   GFXTextureArgument colorArg3;

   GFXTextureArgument resultArg;

   GFXTextureTransformFlags textureTransform;

   GFXSamplerStateDesc();

   /// Returns an modulate, wrap, and linear sampled state.
   static GFXSamplerStateDesc getWrapLinear();

   /// Returns an modulate, wrap, and point sampled state.
   static GFXSamplerStateDesc getWrapPoint();

   /// Returns an modulate, clamp, and linear sampled state.
   static GFXSamplerStateDesc getClampLinear();

   /// Returns an modulate, clamp, and point sampled state.
   static GFXSamplerStateDesc getClampPoint();

   bool operator==(const GFXSamplerStateDesc &b) const
   {
      return !dMemcmp(this, &b, sizeof(GFXSamplerStateDesc));
   }
};

/// GFXStateBlockDesc defines a render state, which is then used to create a GFXStateBlock instance.  
struct GFXStateBlockDesc
{   
   // Blending   
   bool blendDefined;
   bool blendEnable;
   GFXBlend blendSrc;
   GFXBlend blendDest;
   GFXBlendOp blendOp;

   /// @name Separate Alpha Blending
   /// @{
   bool separateAlphaBlendDefined;
   bool separateAlphaBlendEnable;
   GFXBlend separateAlphaBlendSrc;
   GFXBlend separateAlphaBlendDest;
   GFXBlendOp separateAlphaBlendOp;
   /// @}

   // Alpha test
   bool alphaDefined;
   bool alphaTestEnable;   
   S32 alphaTestRef;
   GFXCmpFunc alphaTestFunc;

   // Color Writes
   bool colorWriteDefined;
   bool colorWriteRed;
   bool colorWriteBlue;
   bool colorWriteGreen;
   bool colorWriteAlpha;

   // Rasterizer
   bool cullDefined;
   GFXCullMode cullMode;

   // Depth
   bool zDefined;
   bool zEnable;
   bool zWriteEnable;
   GFXCmpFunc zFunc;
   F32 zBias;
   F32 zSlopeBias;

   // Stencil
   bool stencilDefined;
   bool stencilEnable;
   GFXStencilOp stencilFailOp;
   GFXStencilOp stencilZFailOp;
   GFXStencilOp stencilPassOp;
   GFXCmpFunc  stencilFunc;
   U32 stencilRef;
   U32 stencilMask;
   U32 stencilWriteMask;

   // FF lighting
   bool ffLighting;

   bool vertexColorEnable;

   GFXFillMode fillMode;

   // Sampler states
   bool samplersDefined;
   GFXSamplerStateDesc samplers[TEXTURE_STAGE_COUNT];
   ColorI textureFactor;

   GFXStateBlockDesc();

   /// Returns the hash value of this state description
   U32 getHashValue() const;

   /// Adds data from desc to this description, uses *defined parameters in desc to figure out
   /// what blocks of state to actually copy from desc.
   void addDesc( const GFXStateBlockDesc& desc );

   /// Returns a string that describes the options set (used by GFXStateBlock::describeSelf)
   const String describeSelf() const;

   /// Utility functions to make setting up stateblock descriptions less wordy.
   void setCullMode( GFXCullMode m ); 

   /// Helpers for setting the fill modes.
   void setFillModePoint() { fillMode = GFXFillPoint; }
   void setFillModeWireframe() { fillMode = GFXFillWireframe; }
   void setFillModeSolid() { fillMode = GFXFillSolid; }

   void setZReadWrite( bool read, bool write = true ); 

   void setAlphaTest(   bool enable, 
                        GFXCmpFunc func = GFXCmpGreaterEqual, 
                        S32 alphaRef = 0 );

   void setBlend( bool enable, 
                  GFXBlend src = GFXBlendSrcAlpha, 
                  GFXBlend dest = GFXBlendInvSrcAlpha,
                  GFXBlendOp op = GFXBlendOpAdd );

   void setSeparateAlphaBlend(   bool enable, 
                                 GFXBlend src = GFXBlendOne, 
                                 GFXBlend dest = GFXBlendZero,
                                 GFXBlendOp op = GFXBlendOpAdd );


   ///
   void setColorWrites( bool red, bool green, bool blue, bool alpha );
};

class GFXStateBlock : public StrongRefBase, public GFXResource
{
public:
   virtual ~GFXStateBlock() { }

   /// Returns the hash value of the desc that created this block
   virtual U32 getHashValue() const = 0;

   /// Returns a GFXStateBlockDesc that this block represents
   virtual const GFXStateBlockDesc& getDesc() const = 0;

   /// Default implementation for GFXResource::describeSelf   
   virtual const String describeSelf() const;
};

typedef StrongRefPtr<GFXStateBlock> GFXStateBlockRef;

#endif 
