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
#include "gfx/gfxStateBlock.h"
#include "core/crc.h"
#include "gfx/gfxDevice.h"
#include "core/strings/stringFunctions.h"
#include "gfx/gfxStringEnumTranslate.h"

///
/// GFXStateBlock
///
const String GFXStateBlock::describeSelf() const
{
   return String::ToString("hashvalue: 0x%x", getDesc().getHashValue());
}

/// 
/// GFXStateBlockDesc
///
GFXStateBlockDesc::GFXStateBlockDesc()
{
   // Alpha blending
   blendDefined = false;
   blendEnable = false;
   blendSrc = GFXBlendOne;
   blendDest = GFXBlendZero;
   blendOp = GFXBlendOpAdd;

   // Separate alpha blending
   separateAlphaBlendDefined = false;
   separateAlphaBlendEnable = false;
   separateAlphaBlendSrc = GFXBlendOne;
   separateAlphaBlendDest = GFXBlendZero;
   separateAlphaBlendOp = GFXBlendOpAdd;

   // Alpha test
   alphaDefined = false;
   alphaTestEnable = false;   
   alphaTestRef = 0;
   alphaTestFunc = GFXCmpGreaterEqual;

   // Color Writes
   colorWriteDefined = false;
   colorWriteRed = true;
   colorWriteBlue = true;
   colorWriteGreen = true;
   colorWriteAlpha = true;

   // Rasterizer
   cullDefined = false;
   cullMode = GFXCullCCW;

   // Depth
   zDefined = false;
   zEnable = true;
   zWriteEnable = true;
   zFunc = GFXCmpLessEqual;
   zBias = 0;
   zSlopeBias = 0;

   // Stencil
   stencilDefined = false;
   stencilEnable = false;
   stencilFailOp = GFXStencilOpKeep;
   stencilZFailOp = GFXStencilOpKeep;
   stencilPassOp = GFXStencilOpKeep;
   stencilFunc = GFXCmpNever;
   stencilRef = 0;
   stencilMask = 0xFFFFFFFF;
   stencilWriteMask = 0xFFFFFFFF;

   // FF lighting
   ffLighting = false;

   vertexColorEnable = false;

   fillMode = GFXFillSolid;

   samplersDefined = false;
   textureFactor.set( 255, 255, 255, 255 );
}

// This method just needs to return a unique value based on its contents.
U32 GFXStateBlockDesc::getHashValue() const
{   
   return CRC::calculateCRC(this, sizeof(GFXStateBlockDesc));
}

/// Adds data from desc to this description, uses *defined parameters in desc to figure out
/// what blocks of state to actually copy from desc.
void GFXStateBlockDesc::addDesc(const GFXStateBlockDesc& desc)
{
   // Alpha blending
   if (desc.blendDefined)
   {
      blendDefined = true;
      blendEnable = desc.blendEnable;
      blendSrc = desc.blendSrc;
      blendDest = desc.blendDest;
      blendOp = desc.blendOp;
   }

   // Separate alpha blending
   if ( desc.separateAlphaBlendDefined )
   {
      separateAlphaBlendDefined = true;
      separateAlphaBlendEnable = desc.separateAlphaBlendEnable;
      separateAlphaBlendSrc = desc.separateAlphaBlendSrc;
      separateAlphaBlendDest = desc.separateAlphaBlendDest;
      separateAlphaBlendOp = desc.separateAlphaBlendOp;
   }

   // Alpha test
   if (desc.alphaDefined)   
   {
      alphaDefined = true;
      alphaTestEnable = desc.alphaTestEnable;
      alphaTestRef = desc.alphaTestRef;
      alphaTestFunc = desc.alphaTestFunc;
   }

   // Color Writes
   if (desc.colorWriteDefined)   
   {
      colorWriteDefined = true;
      colorWriteRed = desc.colorWriteRed;
      colorWriteBlue = desc.colorWriteBlue;
      colorWriteGreen = desc.colorWriteGreen;
      colorWriteAlpha = desc.colorWriteAlpha;
   }

   // Rasterizer
   if (desc.cullDefined)
   {   
      cullDefined = true;
      cullMode = desc.cullMode;
   }

   // Depth
   if (desc.zDefined)
   {   
      zDefined = true;
      zEnable = desc.zEnable;
      zWriteEnable = desc.zWriteEnable;
      zFunc = desc.zFunc;
      zBias = desc.zBias;
      zSlopeBias = desc.zSlopeBias;
   }

   // Stencil
   if (desc.stencilDefined)
   {   
      stencilDefined = true;
      stencilEnable = desc.stencilEnable;
      stencilFailOp = desc.stencilFailOp;
      stencilZFailOp = desc.stencilZFailOp;
      stencilPassOp = desc.stencilPassOp;
      stencilFunc = desc.stencilFunc;
      stencilRef = desc.stencilRef;
      stencilMask = desc.stencilMask;
      stencilWriteMask = desc.stencilWriteMask;
   }

   if (desc.samplersDefined)
   {
      samplersDefined = true;
      for (U32 i = 0; i < TEXTURE_STAGE_COUNT; i++)
      {
         samplers[i] = desc.samplers[i];
      }
      textureFactor = desc.textureFactor;
   }

   vertexColorEnable = desc.vertexColorEnable;
   fillMode = desc.fillMode;
}

/// Returns a string that describes the options set (used by GFXStateBlock::describeSelf)
const String GFXStateBlockDesc::describeSelf() const
{
   GFXStringEnumTranslate::init();

   String ret;
   ret = String::ToString("  AlphaBlend: %d, BlendSrc: %s, BlendDest: %s, BlendOp: %s\n", 
      blendEnable, GFXStringBlend[blendSrc], GFXStringBlend[blendDest], GFXStringBlendOp[blendOp]);
   ret += String::ToString("  SeparateAlphaBlend: %d, SeparateAlphaBlendSrc: %s, SeparateAlphaBlendDest: %s, SeparateAlphaBlendOp: %s\n", 
      separateAlphaBlendEnable, GFXStringBlend[separateAlphaBlendSrc], GFXStringBlend[separateAlphaBlendDest], GFXStringBlendOp[separateAlphaBlendOp]);
   ret += String::ToString("  AlphaTest: %d, AlphaTestFunc: %s, AlphaTestRef: %d\n",
      alphaTestEnable, GFXStringCmpFunc[alphaTestFunc], alphaTestRef);
   ret += String::ToString("  ColorWrites: r: %d g: %d b: %d a: %d", 
      colorWriteRed, colorWriteGreen, colorWriteBlue, colorWriteAlpha);
   ret += String::ToString("  CullMode: %s\n", GFXStringCullMode[cullMode]);
   ret += String::ToString("  ZEnable: %d, ZWriteEnable: %d, ZFunc: %s, ZBias: %f, ZSlopeBias: %f\n", 
      zEnable, zWriteEnable, GFXStringCmpFunc[zFunc], zBias, zSlopeBias);
   ret += String::ToString("  Stencil: %d, StencilFailOp: %s, StencilZFailOp: %s, StencilPassOp: %s, \n  stencilFunc: %s, stencilRef: %d, stencilMask: 0x%x, stencilWriteMask: 0x%x\n",
      stencilEnable, GFXStringCmpFunc[stencilFailOp], GFXStringCmpFunc[stencilZFailOp], GFXStringCmpFunc[stencilPassOp], 
      GFXStringCmpFunc[stencilFunc], stencilRef, stencilMask, stencilWriteMask);
   ret += String::ToString("  FF Lighting: %d, VertexColors: %d, fillMode: %s",
      ffLighting, vertexColorEnable, GFXStringFillMode[fillMode]);

   return ret;
}

// 
// Utility functions
//

void GFXStateBlockDesc::setCullMode( GFXCullMode m ) 
{ 
   cullDefined = true; 
   cullMode = m; 
}

void GFXStateBlockDesc::setZReadWrite( bool read, bool write )
{ 
   zDefined = true; 
   zEnable = read; 
   zWriteEnable = write;
}

void GFXStateBlockDesc::setAlphaTest( bool enable, GFXCmpFunc func, S32 alphaRef ) 
{ 
   alphaDefined = true; 
   alphaTestEnable = enable; 
   alphaTestFunc = func; 
   alphaTestRef = alphaRef; 
}

void GFXStateBlockDesc::setBlend( bool enable, GFXBlend src, GFXBlend dest, GFXBlendOp op ) 
{ 
   blendDefined = true; 
   blendEnable = enable; 
   blendSrc = src; 
   blendDest = dest; 
   blendOp = op;
}

void GFXStateBlockDesc::setSeparateAlphaBlend( bool enable, GFXBlend src, GFXBlend dest, GFXBlendOp op ) 
{ 
   separateAlphaBlendDefined = true; 
   separateAlphaBlendEnable = enable; 
   separateAlphaBlendSrc = src; 
   separateAlphaBlendDest = dest; 
   separateAlphaBlendOp = op;
}

void GFXStateBlockDesc::setColorWrites( bool red, bool green, bool blue, bool alpha )
{
   colorWriteDefined = true;
   colorWriteRed = red;
   colorWriteGreen = green;
   colorWriteBlue = blue;
   colorWriteAlpha = alpha;
}

GFXSamplerStateDesc::GFXSamplerStateDesc()
{
   textureColorOp = GFXTOPDisable;
   addressModeU = GFXAddressWrap;
   addressModeV = GFXAddressWrap;
   addressModeW = GFXAddressWrap;
   magFilter = GFXTextureFilterLinear;
   minFilter = GFXTextureFilterLinear;
   mipFilter = GFXTextureFilterLinear;
   maxAnisotropy = 1;
   alphaArg1 = GFXTATexture;
   alphaArg2 = GFXTADiffuse;
   alphaArg3 = GFXTACurrent;
   colorArg1 = GFXTACurrent;
   colorArg2 = GFXTATexture;
   colorArg3 = GFXTACurrent;
   alphaOp = GFXTOPModulate;
   textureTransform = GFXTTFFDisable;
   resultArg = GFXTACurrent;
   mipLODBias = 0.0f;
}

GFXSamplerStateDesc GFXSamplerStateDesc::getWrapLinear()
{
   // Linear with wrapping is already the default
   GFXSamplerStateDesc ssd;
   ssd.textureColorOp = GFXTOPModulate;
   return ssd;
}

GFXSamplerStateDesc GFXSamplerStateDesc::getWrapPoint()
{
   GFXSamplerStateDesc ssd;
   ssd.textureColorOp = GFXTOPModulate;
   ssd.magFilter = GFXTextureFilterPoint;
   ssd.minFilter = GFXTextureFilterPoint;
   ssd.mipFilter = GFXTextureFilterPoint;
   return ssd;
}

GFXSamplerStateDesc GFXSamplerStateDesc::getClampLinear()
{
   GFXSamplerStateDesc ssd;
   ssd.textureColorOp = GFXTOPModulate;
   ssd.addressModeU = GFXAddressClamp;
   ssd.addressModeV = GFXAddressClamp;
   ssd.addressModeW = GFXAddressClamp;
   return ssd;
}

GFXSamplerStateDesc GFXSamplerStateDesc::getClampPoint()
{
   GFXSamplerStateDesc ssd;
   ssd.textureColorOp = GFXTOPModulate;
   ssd.addressModeU = GFXAddressClamp;
   ssd.addressModeV = GFXAddressClamp;
   ssd.addressModeW = GFXAddressClamp;
   ssd.magFilter = GFXTextureFilterPoint;
   ssd.minFilter = GFXTextureFilterPoint;
   ssd.mipFilter = GFXTextureFilterPoint;
   return ssd;
}