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
#include "gfx/sim/gfxStateBlockData.h"

#include "console/consoleTypes.h"
#include "gfx/gfxStringEnumTranslate.h"


IMPLEMENT_CONOBJECT( GFXStateBlockData );

ConsoleDocClass(  GFXStateBlockData, 
                  "@brief A state block description for rendering.\n\n"
                  "This object is used with ShaderData in CustomMaterial and PostEffect to define the "
                  "render state.\n"
                  "@tsexample\n"
                  "singleton GFXStateBlockData( PFX_DOFDownSampleStateBlock )\n"
                  "{\n"
                  "   zDefined = true;\n"
                  "   zEnable = false;\n"
                  "   zWriteEnable = false;\n"
                  "\n"
                  "   samplersDefined = true;\n"
                  "   samplerStates[0] = SamplerClampLinear;\n"
                  "   samplerStates[1] = SamplerClampPoint;\n"
                  "\n"
                  "   // Copy the clamped linear sampler, but change\n"
                  "   // the u coord to wrap for this special case.\n"
                  "   samplerStates[2] = new GFXSamplerStateData( : SamplerClampLinear )\n"
                  "   {\n"
                  "      addressModeU = GFXAddressWrap;\n"
                  "   };\n"                  
                  "};\n"
                  "@endtsexample\n"
                  "@note The 'xxxxDefined' fields are used to know what groups of fields are modified "
                  "when combining multiple state blocks in material processing.  You should take care to "
                  "enable the right ones when setting values.\n"
                  "@ingroup GFX\n" );

GFXStateBlockData::GFXStateBlockData()
{
   for (U32 i = 0; i < TEXTURE_STAGE_COUNT; i++)
      mSamplerStates[i] = NULL;
}

void GFXStateBlockData::initPersistFields()
{
   addGroup( "Alpha Blending" );

      addField( "blendDefined", TypeBool, Offset(mState.blendDefined, GFXStateBlockData),
         "Set to true if the alpha blend state is not all defaults." );

      addField( "blendEnable", TypeBool, Offset(mState.blendEnable, GFXStateBlockData),
         "Enables alpha blending.  The default is false." );

      addField( "blendSrc", TypeGFXBlend, Offset(mState.blendSrc, GFXStateBlockData),
         "The source blend state.  The default is GFXBlendOne." );

      addField("blendDest", TypeGFXBlend, Offset(mState.blendDest, GFXStateBlockData),
         "The destination blend state.  The default is GFXBlendZero." );

      addField("blendOp", TypeGFXBlendOp, Offset(mState.blendOp, GFXStateBlockData),
         "The arithmetic operation applied to alpha blending.  The default is GFXBlendOpAdd." );   

   endGroup( "Alpha Blending" );


   addGroup( "Separate Alpha Blending" );

      addField( "separateAlphaBlendDefined", TypeBool, Offset(mState.separateAlphaBlendDefined, GFXStateBlockData),
         "Set to true if the seperate alpha blend state is not all defaults." );

      addField( "separateAlphaBlendEnable", TypeBool, Offset(mState.separateAlphaBlendEnable, GFXStateBlockData),
         "Enables the separate blend mode for the alpha channel.  The default is false." );

      addField( "separateAlphaBlendSrc", TypeGFXBlend, Offset(mState.separateAlphaBlendSrc, GFXStateBlockData),
         "The source blend state.  The default is GFXBlendOne." );

      addField( "separateAlphaBlendDest", TypeGFXBlend, Offset(mState.separateAlphaBlendDest, GFXStateBlockData),
         "The destination blend state.  The default is GFXBlendZero." );

      addField( "separateAlphaBlendOp", TypeGFXBlendOp, Offset(mState.separateAlphaBlendOp, GFXStateBlockData),
         "The arithmetic operation applied to separate alpha blending.  The default is GFXBlendOpAdd." );   

   endGroup( "Separate Alpha Blending" );


   addGroup( "Alpha Test" );

      addField( "alphaDefined", TypeBool, Offset(mState.alphaDefined, GFXStateBlockData),
         "Set to true if the alpha test state is not all defaults." );

      addField( "alphaTestEnable", TypeBool, Offset(mState.alphaTestEnable, GFXStateBlockData),
         "Enables per-pixel alpha testing.  The default is false." );

      addField( "alphaTestFunc", TypeGFXCmpFunc, Offset(mState.alphaTestFunc, GFXStateBlockData),
         "The test function used to accept or reject a pixel based on its alpha value.  The default is GFXCmpGreaterEqual." );

      addField( "alphaTestRef", TypeS32, Offset(mState.alphaTestRef, GFXStateBlockData),
         "The reference alpha value against which pixels are tested.  The default is zero." );

   endGroup( "Alpha Test" );


   addGroup( "Color Write" );

      addField( "colorWriteDefined", TypeBool, Offset(mState.colorWriteDefined, GFXStateBlockData),
         "Set to true if the color write state is not all defaults." );

      addField( "colorWriteRed", TypeBool, Offset(mState.colorWriteRed, GFXStateBlockData),
         "Enables red channel writes.  The default is true." );

      addField( "colorWriteBlue", TypeBool, Offset(mState.colorWriteBlue, GFXStateBlockData),
         "Enables blue channel writes.  The default is true." );

      addField( "colorWriteGreen", TypeBool, Offset(mState.colorWriteGreen, GFXStateBlockData),
         "Enables green channel writes.  The default is true." );

      addField( "colorWriteAlpha", TypeBool, Offset(mState.colorWriteAlpha, GFXStateBlockData),
         "Enables alpha channel writes.  The default is true." );

   endGroup( "Color Write" );


   addGroup( "Culling" );

      addField("cullDefined", TypeBool, Offset(mState.cullDefined, GFXStateBlockData),
         "Set to true if the culling state is not all defaults." );

      addField("cullMode", TypeGFXCullMode, Offset(mState.cullMode, GFXStateBlockData),        
         "Defines how back facing triangles are culled if at all.  The default is GFXCullCCW." );

   endGroup( "Culling" );


   addGroup( "Depth" );

      addField( "zDefined", TypeBool, Offset(mState.zDefined, GFXStateBlockData),
         "Set to true if the depth state is not all defaults." );

      addField( "zEnable", TypeBool, Offset(mState.zEnable, GFXStateBlockData),
         "Enables z-buffer reads.  The default is true." );

      addField( "zWriteEnable", TypeBool, Offset(mState.zWriteEnable, GFXStateBlockData),
         "Enables z-buffer writes.  The default is true." );

      addField( "zFunc", TypeGFXCmpFunc, Offset(mState.zFunc, GFXStateBlockData),
         "The depth comparision function which a pixel must pass to be written to the z-buffer.  The default is GFXCmpLessEqual." );

      addField( "zBias", TypeF32, Offset(mState.zBias, GFXStateBlockData),
         "A floating-point bias used when comparing depth values.  The default is zero." );

      addField( "zSlopeBias", TypeF32, Offset(mState.zSlopeBias, GFXStateBlockData),
         "An additional floating-point bias based on the maximum depth slop of the triangle being rendered.  The default is zero." );

   endGroup( "Depth" );


   addGroup( "Stencil" );

      addField( "stencilDefined", TypeBool, Offset(mState.stencilDefined, GFXStateBlockData),
         "Set to true if the stencil state is not all defaults." );

      addField( "stencilEnable", TypeBool, Offset(mState.stencilEnable, GFXStateBlockData),
         "Enables stenciling.  The default is false." );

      addField( "stencilFailOp", TypeGFXStencilOp, Offset(mState.stencilFailOp, GFXStateBlockData),
         "The stencil operation to perform if the stencil test fails.  The default is GFXStencilOpKeep." );
      
      addField( "stencilZFailOp", TypeGFXStencilOp, Offset(mState.stencilZFailOp, GFXStateBlockData),
         "The stencil operation to perform if the stencil test passes and the depth test fails.  The default is GFXStencilOpKeep." );
      
      addField( "stencilPassOp", TypeGFXStencilOp, Offset(mState.stencilPassOp, GFXStateBlockData), 
         "The stencil operation to perform if both the stencil and the depth tests pass.  The default is GFXStencilOpKeep." );
      
      addField( "stencilFunc", TypeGFXCmpFunc, Offset(mState.stencilFunc, GFXStateBlockData),
         "The comparison function to test the reference value to a stencil buffer entry.  The default is GFXCmpNever." );

      addField( "stencilRef", TypeS32, Offset(mState.stencilRef, GFXStateBlockData),
         "The reference value for the stencil test.  The default is zero." );

      addField( "stencilMask", TypeS32, Offset(mState.stencilMask, GFXStateBlockData),
         "The mask applied to the reference value and each stencil buffer entry to determine the significant bits for the stencil test. The default is 0xFFFFFFFF." );

      addField( "stencilWriteMask", TypeS32, Offset(mState.stencilWriteMask, GFXStateBlockData),
         "The write mask applied to values written into the stencil buffer. The default is 0xFFFFFFFF." );

   endGroup( "Stencil" );


   addGroup( "Fixed Function" );

      addField( "ffLighting", TypeBool, Offset(mState.ffLighting, GFXStateBlockData),
         "Enables fixed function lighting when rendering without a shader on geometry with vertex normals.  The default is false." );

      addField( "vertexColorEnable", TypeBool, Offset(mState.vertexColorEnable, GFXStateBlockData),
         "Enables fixed function vertex coloring when rendering without a shader.  The default is false." );

   endGroup( "Fixed Function" );


   addGroup( "Sampler States" );

      addField( "samplersDefined", TypeBool, Offset(mState.samplersDefined, GFXStateBlockData),
         "Set to true if the sampler states are not all defaults." );

      addField( "samplerStates", TYPEID<GFXSamplerStateData>(), Offset(mSamplerStates, GFXStateBlockData), TEXTURE_STAGE_COUNT,
         "The array of texture sampler states.\n"
         "@note Not all graphics devices support 16 samplers.  In general "
         "all systems support 4 samplers with most modern cards doing 8." );

      addField( "textureFactor", TypeColorI, Offset(mState.textureFactor, GFXStateBlockData),
         "The color used for multiple-texture blending with the GFXTATFactor texture-blending argument or "
         "the GFXTOPBlendFactorAlpha texture-blending operation.  The default is opaque white (255, 255, 255, 255)." );

   endGroup( "Sampler States" );

   Parent::initPersistFields();
}

bool GFXStateBlockData::onAdd()
{
   if (!Parent::onAdd())
      return false;

   for (U32 i = 0; i < TEXTURE_STAGE_COUNT; i++)
   {  
      if (mSamplerStates[i])
         mSamplerStates[i]->setSamplerState(mState.samplers[i]);
   }
   return true;
}


IMPLEMENT_CONOBJECT( GFXSamplerStateData );

ConsoleDocClass(  GFXSamplerStateData, 
                  "@brief A sampler state used by GFXStateBlockData.\n\n"
                  "The samplers define how a texture will be sampled when used from the shader "
                  "or fixed function device\n"
                  "@tsexample\n"
                  "singleton GFXSamplerStateData(SamplerClampLinear)\n"
                  "{\n"
                  "   textureColorOp = GFXTOPModulate;\n"
                  "   addressModeU = GFXAddressClamp;\n"
                  "   addressModeV = GFXAddressClamp;\n"
                  "   addressModeW = GFXAddressClamp;\n"
                  "   magFilter = GFXTextureFilterLinear;\n"
                  "   minFilter = GFXTextureFilterLinear;\n"
                  "   mipFilter = GFXTextureFilterLinear;\n"
                  "};\n"
                  "@endtsexample\n"
                  "There are a few predefined samplers in the core scripts which you can use with "
                  "GFXStateBlockData for the most common rendering cases:\n"
                  "  - SamplerClampLinear\n"
                  "  - SamplerClampPoint\n"
                  "  - SamplerWrapLinear\n"
                  "  - SamplerWrapPoint\n"
                  "\n"
                  "@see GFXStateBlockData\n"
                  "@ingroup GFX\n" );

void GFXSamplerStateData::initPersistFields()
{
   Parent::initPersistFields();

   addGroup( "Color Op" );

      addField("textureColorOp", TypeGFXTextureOp, Offset(mState.textureColorOp, GFXSamplerStateData),
         "The texture color blending operation.  The default value is GFXTOPDisable which disables the sampler." );

      addField("colorArg1", TYPEID< GFXTextureArgument >(), Offset(mState.colorArg1, GFXSamplerStateData),
         "The first color argument for the texture stage.  The default value is GFXTACurrent." );

      addField("colorArg2", TYPEID< GFXTextureArgument >(), Offset(mState.colorArg2, GFXSamplerStateData),
         "The second color argument for the texture stage.  The default value is GFXTATexture." );

      addField("colorArg3", TYPEID< GFXTextureArgument >(), Offset(mState.colorArg3, GFXSamplerStateData),
         "The third color argument for triadic operations (multiply, add, and linearly interpolate).  The default value is GFXTACurrent." );

   endGroup( "Color Op" );

   addGroup( "Alpha Op" );

      addField("alphaOp", TypeGFXTextureOp, Offset(mState.alphaOp, GFXSamplerStateData),
         "The texture alpha blending operation.  The default value is GFXTOPModulate." );

      addField("alphaArg1", TYPEID< GFXTextureArgument >(), Offset(mState.alphaArg1, GFXSamplerStateData),
         "The first alpha argument for the texture stage.  The default value is GFXTATexture." );

      addField("alphaArg2", TYPEID< GFXTextureArgument >(), Offset(mState.alphaArg2, GFXSamplerStateData),
         "The second alpha argument for the texture stage.  The default value is GFXTADiffuse." );

      addField("alphaArg3", TYPEID< GFXTextureArgument >(), Offset(mState.alphaArg3, GFXSamplerStateData),
         "The third alpha channel selector operand for triadic operations (multiply, add, and linearly interpolate).  The default value is GFXTACurrent." );

   endGroup( "Alpha Op" );

   addGroup( "Address Mode" );

      addField("addressModeU", TypeGFXTextureAddressMode, Offset(mState.addressModeU, GFXSamplerStateData),
         "The texture address mode for the u coordinate.  The default is GFXAddressWrap." );

      addField("addressModeV", TypeGFXTextureAddressMode, Offset(mState.addressModeV, GFXSamplerStateData),
         "The texture address mode for the v coordinate.  The default is GFXAddressWrap." );

      addField("addressModeW", TypeGFXTextureAddressMode, Offset(mState.addressModeW, GFXSamplerStateData),
         "The texture address mode for the w coordinate.  The default is GFXAddressWrap." );

   endGroup( "Address Mode" );

   addGroup( "Filter State" );

      addField("magFilter", TypeGFXTextureFilterType, Offset(mState.magFilter, GFXSamplerStateData),
         "The texture magnification filter.  The default is GFXTextureFilterLinear." );

      addField("minFilter", TypeGFXTextureFilterType, Offset(mState.minFilter, GFXSamplerStateData),
         "The texture minification filter.  The default is GFXTextureFilterLinear." );

      addField("mipFilter", TypeGFXTextureFilterType, Offset(mState.mipFilter, GFXSamplerStateData),
         "The texture mipmap filter used during minification.  The default is GFXTextureFilterLinear." );

      addField("mipLODBias", TypeF32, Offset(mState.mipLODBias, GFXSamplerStateData),
         "The mipmap level of detail bias.  The default value is zero." );

      addField("maxAnisotropy", TypeS32, Offset(mState.maxAnisotropy, GFXSamplerStateData),
         "The maximum texture anisotropy.  The default value is 1." );

   endGroup( "Filter State" );

   addField("textureTransform", TypeGFXTextureTransformFlags, Offset(mState.textureTransform, GFXSamplerStateData),
      "Sets the texture transform state.  The default is GFXTTFFDisable." );

   addField("resultArg", TypeGFXTextureArgument, Offset(mState.resultArg, GFXSamplerStateData),
      "The selection of the destination register for the result of this stage.  The default is GFXTACurrent." );
}

/// Copies the data of this object into desc
void GFXSamplerStateData::setSamplerState(GFXSamplerStateDesc& desc)
{
   desc = mState;
}
