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
//  This file contains shader data necessary for various engine utility functions
//-----------------------------------------------------------------------------


singleton ShaderData( ParticlesShaderData )
{
   DXVertexShaderFile     = "shaders/common/particlesV.hlsl";
   DXPixelShaderFile      = "shaders/common/particlesP.hlsl";   
   
   OGLVertexShaderFile     = "shaders/common/gl/particlesV.glsl";
   OGLPixelShaderFile      = "shaders/common/gl/particlesP.glsl";
   
   pixVersion = 2.0;
};

singleton ShaderData( OffscreenParticleCompositeShaderData )
{
   DXVertexShaderFile     = "shaders/common/particleCompositeV.hlsl";
   DXPixelShaderFile      = "shaders/common/particleCompositeP.hlsl";
   
   OGLVertexShaderFile     = "shaders/common/gl/particleCompositeV.glsl";
   OGLPixelShaderFile      = "shaders/common/gl/particleCompositeP.glsl";
   
   pixVersion = 2.0;
};

//-----------------------------------------------------------------------------
// Planar Reflection
//-----------------------------------------------------------------------------
new ShaderData( ReflectBump )
{
   DXVertexShaderFile 	= "shaders/common/planarReflectBumpV.hlsl";
   DXPixelShaderFile 	= "shaders/common/planarReflectBumpP.hlsl";
   
   OGLVertexShaderFile 	= "shaders/common/gl/planarReflectBumpV.glsl";
   OGLPixelShaderFile 	= "shaders/common/gl/planarReflectBumpP.glsl";
              
   samplerNames[0] = "$diffuseMap";
   samplerNames[1] = "$refractMap";
   samplerNames[2] = "$bumpMap";
   
   pixVersion = 2.0;
};

new ShaderData( Reflect )
{
   DXVertexShaderFile 	= "shaders/common/planarReflectV.hlsl";
   DXPixelShaderFile 	= "shaders/common/planarReflectP.hlsl";
   
   OGLVertexShaderFile 	= "shaders/common/gl/planarReflectV.glsl";
   OGLPixelShaderFile 	= "shaders/common/gl/planarReflectP.glsl";
   
   samplerNames[0] = "$diffuseMap";
   samplerNames[1] = "$refractMap";
   
   pixVersion = 1.4;
};

//-----------------------------------------------------------------------------
// fxFoliageReplicator
//-----------------------------------------------------------------------------
new ShaderData( fxFoliageReplicatorShader )
{
   DXVertexShaderFile 	= "shaders/common/fxFoliageReplicatorV.hlsl";
   DXPixelShaderFile 	= "shaders/common/fxFoliageReplicatorP.hlsl";
   
   OGLVertexShaderFile  = "shaders/common/gl/fxFoliageReplicatorV.glsl";
   OGLPixelShaderFile   = "shaders/common/gl/fxFoliageReplicatorP.glsl";

   samplerNames[0] = "$diffuseMap";
   samplerNames[1] = "$alphaMap";
   
   pixVersion = 1.4;
};