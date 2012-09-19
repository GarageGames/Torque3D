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



//-----------------------------------------------------------------------------
// Water
//-----------------------------------------------------------------------------

singleton ShaderData( WaterShader )
{
   DXVertexShaderFile 	= "shaders/common/water/waterV.hlsl";
   DXPixelShaderFile 	= "shaders/common/water/waterP.hlsl";
   
   OGLVertexShaderFile = "shaders/common/water/gl/waterV.glsl";
   OGLPixelShaderFile = "shaders/common/water/gl/waterP.glsl";
   
   pixVersion = 3.0;
};

new GFXSamplerStateData(WaterSampler)
{
   textureColorOp = GFXTOPModulate;
   addressModeU = GFXAddressWrap;
   addressModeV = GFXAddressWrap;
   addressModeW = GFXAddressWrap;
   magFilter = GFXTextureFilterLinear;
   minFilter = GFXTextureFilterAnisotropic;
   mipFilter = GFXTextureFilterLinear;
   maxAnisotropy = 4;
};

singleton GFXStateBlockData( WaterStateBlock )
{
   samplersDefined = true;
   samplerStates[0] = WaterSampler;  // noise
   samplerStates[1] = SamplerClampPoint;  // #prepass
   samplerStates[2] = SamplerClampLinear; // $reflectbuff
   samplerStates[3] = SamplerClampPoint;  // $backbuff
   samplerStates[4] = SamplerWrapLinear;  // $cubemap   
   samplerStates[5] = SamplerWrapLinear;  // foam     
   samplerStates[6] = SamplerClampLinear; // depthMap ( color gradient ) 
   cullDefined = true;
   cullMode = "GFXCullCCW";
};

singleton GFXStateBlockData( UnderWaterStateBlock : WaterStateBlock )
{
   cullMode = "GFXCullCW";
};

singleton CustomMaterial( WaterMat )
{   
   sampler["prepassTex"] = "#prepass";
   sampler["reflectMap"] = "$reflectbuff";
   sampler["refractBuff"] = "$backbuff";
   
   shader = WaterShader;
   stateBlock = WaterStateBlock;
   version = 3.0;
   
   useAnisotropic[0] = true;
};

//-----------------------------------------------------------------------------
// Underwater
//-----------------------------------------------------------------------------

singleton ShaderData( UnderWaterShader )
{
   DXVertexShaderFile 	= "shaders/common/water/waterV.hlsl";
   DXPixelShaderFile 	= "shaders/common/water/waterP.hlsl";   
   
   OGLVertexShaderFile 	= "shaders/common/water/gl/waterV.glsl";
   OGLPixelShaderFile 	= "shaders/common/water/gl/waterP.glsl"; 
   
   defines = "UNDERWATER";   
   pixVersion = 3.0;
};

singleton CustomMaterial( UnderwaterMat )
{  
   // These samplers are set in code not here.
   // This is to allow different WaterObject instances
   // to use this same material but override these textures
   // per instance.   
   //sampler["bumpMap"] = "core/art/water/noise02";
   //sampler["foamMap"] = "core/art/water/foam";

   sampler["prepassTex"] = "#prepass";
   sampler["refractBuff"] = "$backbuff";   
   
   shader = UnderWaterShader;
   stateBlock = UnderWaterStateBlock;
   specular = "0.75 0.75 0.75 1.0";
   specularPower = 48.0;
   version = 3.0;
};

//-----------------------------------------------------------------------------
// Basic Water
//-----------------------------------------------------------------------------

singleton ShaderData( WaterBasicShader )
{
   DXVertexShaderFile 	= "shaders/common/water/waterBasicV.hlsl";
   DXPixelShaderFile 	= "shaders/common/water/waterBasicP.hlsl";
   
   OGLVertexShaderFile 	= "shaders/common/water/gl/waterBasicV.glsl";
   OGLPixelShaderFile 	= "shaders/common/water/gl/waterBasicP.glsl"; 
   
   pixVersion = 2.0;
};

singleton GFXStateBlockData( WaterBasicStateBlock )
{
   samplersDefined = true;
   samplerStates[0] = WaterSampler;  // noise
   samplerStates[2] = SamplerClampLinear;  // $reflectbuff
   samplerStates[3] = SamplerClampPoint;  // $backbuff
   samplerStates[4] = SamplerWrapLinear;  // $cubemap
   cullDefined = true;
   cullMode = "GFXCullCCW";
};

singleton GFXStateBlockData( UnderWaterBasicStateBlock : WaterBasicStateBlock )
{
   cullMode = "GFXCullCW";
};

singleton CustomMaterial( WaterBasicMat )
{
   // These samplers are set in code not here.
   // This is to allow different WaterObject instances
   // to use this same material but override these textures
   // per instance.     
   //sampler["bumpMap"] = "core/art/water/noise02";
   //sampler["skyMap"] = "$cubemap";   
   
   //sampler["prepassTex"] = "#prepass";
   sampler["reflectMap"] = "$reflectbuff";
   sampler["refractBuff"] = "$backbuff";
    
   cubemap = NewLevelSkyCubemap;
   shader = WaterBasicShader;
   stateBlock = WaterBasicStateBlock;
   version = 2.0;
};

//-----------------------------------------------------------------------------
// Basic UnderWater
//-----------------------------------------------------------------------------

singleton ShaderData( UnderWaterBasicShader )
{
   DXVertexShaderFile 	= "shaders/common/water/waterBasicV.hlsl";
   DXPixelShaderFile 	= "shaders/common/water/waterBasicP.hlsl";   
   
   OGLVertexShaderFile 	= "shaders/common/water/gl/waterBasicV.glsl";
   OGLPixelShaderFile 	= "shaders/common/water/gl/waterBasicP.glsl";
   
   defines = "UNDERWATER";   
   pixVersion = 2.0;
};

singleton CustomMaterial( UnderwaterBasicMat )
{
   // These samplers are set in code not here.
   // This is to allow different WaterObject instances
   // to use this same material but override these textures
   // per instance.  
   //sampler["bumpMap"] = "core/art/water/noise02";
   //samplers["skyMap"] = "$cubemap";  

   //sampler["prepassTex"] = "#prepass";
   sampler["refractBuff"] = "$backbuff";
   
   shader = UnderWaterBasicShader;
   stateBlock = UnderWaterBasicStateBlock;
   version = 2.0;
};