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

// This material should work fine for uniformly colored ribbons.

//Basic ribbon shader/////////////////////////////////////////////
 
new ShaderData( BasicRibbonShader )
{
   DXVertexShaderFile   = "shaders/common/ribbons/basicRibbonShaderV.hlsl";
   DXPixelShaderFile    = "shaders/common/ribbons/basicRibbonShaderP.hlsl";
 
   OGLVertexShaderFile   = "shaders/common/ribbons/gl/basicRibbonShaderV.glsl";
   OGLPixelShaderFile    = "shaders/common/ribbons/gl/basicRibbonShaderP.glsl";
 
   samplerNames[0] = "$ribTex";
 
   pixVersion = 2.0;
};
 
singleton CustomMaterial( BasicRibbonMat )
{
   shader = BasicRibbonShader;
   version = 2.0;
   
   emissive[0] = true;
   
   doubleSided = true;
   translucent = true;
   BlendOp = AddAlpha;
   translucentBlendOp = AddAlpha;
   
   preload = true;
};

// This material can render a texture on top of a ribbon.

//Texture ribbon shader/////////////////////////////////////////////
 
new ShaderData( TexturedRibbonShader )
{
   DXVertexShaderFile   = "shaders/common/ribbons/texRibbonShaderV.hlsl";
   DXPixelShaderFile    = "shaders/common/ribbons/texRibbonShaderP.hlsl";
   
   OGLVertexShaderFile   = "shaders/common/ribbons/gl/texRibbonShaderV.glsl";
   OGLPixelShaderFile    = "shaders/common/ribbons/gl/texRibbonShaderP.glsl";
   
   samplerNames[0] = "$ribTex";
   
   pixVersion = 2.0;
};
 
singleton CustomMaterial( TexturedRibbonMat )
{
   shader = TexturedRibbonShader;
   version = 2.0;
   
   emissive[0] = true;
   
   doubleSided = true;
   translucent = true;
   BlendOp = AddAlpha;
   translucentBlendOp = AddAlpha;

   sampler["ribTex"] = "art/ribbons/ribTex.png";
   
   preload = true;
};