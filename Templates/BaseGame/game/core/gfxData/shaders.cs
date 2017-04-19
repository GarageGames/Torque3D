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
   DXVertexShaderFile     = $Core::CommonShaderPath @ "/particlesV.hlsl";
   DXPixelShaderFile      = $Core::CommonShaderPath @ "/particlesP.hlsl";   
   
   OGLVertexShaderFile     = $Core::CommonShaderPath @ "/gl/particlesV.glsl";
   OGLPixelShaderFile      = $Core::CommonShaderPath @ "/gl/particlesP.glsl";
   
   samplerNames[0] = "$diffuseMap";
   samplerNames[1] = "$deferredTex";
   samplerNames[2] = "$paraboloidLightMap";
   
   pixVersion = 2.0;
};

singleton ShaderData( OffscreenParticleCompositeShaderData )
{
   DXVertexShaderFile     = $Core::CommonShaderPath @ "/particleCompositeV.hlsl";
   DXPixelShaderFile      = $Core::CommonShaderPath @ "/particleCompositeP.hlsl";
   
   OGLVertexShaderFile     = $Core::CommonShaderPath @ "/gl/particleCompositeV.glsl";
   OGLPixelShaderFile      = $Core::CommonShaderPath @ "/gl/particleCompositeP.glsl";
   
   samplerNames[0] = "$colorSource";
   samplerNames[1] = "$edgeSource";
   
   pixVersion = 2.0;
};

//-----------------------------------------------------------------------------
// Planar Reflection
//-----------------------------------------------------------------------------
new ShaderData( ReflectBump )
{
   DXVertexShaderFile 	= $Core::CommonShaderPath @ "/planarReflectBumpV.hlsl";
   DXPixelShaderFile 	= $Core::CommonShaderPath @ "/planarReflectBumpP.hlsl";
   
   OGLVertexShaderFile  = $Core::CommonShaderPath @ "/gl/planarReflectBumpV.glsl";
   OGLPixelShaderFile   = $Core::CommonShaderPath @ "/gl/planarReflectBumpP.glsl";
              
   samplerNames[0] = "$diffuseMap";
   samplerNames[1] = "$refractMap";
   samplerNames[2] = "$bumpMap";
   
   pixVersion = 2.0;
};

new ShaderData( Reflect )
{
   DXVertexShaderFile 	= $Core::CommonShaderPath @ "/planarReflectV.hlsl";
   DXPixelShaderFile 	= $Core::CommonShaderPath @ "/planarReflectP.hlsl";
   
   OGLVertexShaderFile  = $Core::CommonShaderPath @ "/gl/planarReflectV.glsl";
   OGLPixelShaderFile   = $Core::CommonShaderPath @ "/gl/planarReflectP.glsl";
   
   samplerNames[0] = "$diffuseMap";
   samplerNames[1] = "$refractMap";
   
   pixVersion = 1.4;
};

//-----------------------------------------------------------------------------
// fxFoliageReplicator
//-----------------------------------------------------------------------------
new ShaderData( fxFoliageReplicatorShader )
{
   DXVertexShaderFile 	= $Core::CommonShaderPath @ "/fxFoliageReplicatorV.hlsl";
   DXPixelShaderFile 	= $Core::CommonShaderPath @ "/fxFoliageReplicatorP.hlsl";
   
   OGLVertexShaderFile  = $Core::CommonShaderPath @ "/gl/fxFoliageReplicatorV.glsl";
   OGLPixelShaderFile   = $Core::CommonShaderPath @ "/gl/fxFoliageReplicatorP.glsl";

   samplerNames[0] = "$diffuseMap";
   samplerNames[1] = "$alphaMap";
   
   pixVersion = 1.4;
};

singleton ShaderData( VolumetricFogDeferredShader )
{
   DXVertexShaderFile = $Core::CommonShaderPath @ "/VolumetricFog/VFogPreV.hlsl";
   DXPixelShaderFile = $Core::CommonShaderPath @ "/VolumetricFog/VFogPreP.hlsl";
	
   OGLVertexShaderFile  = $Core::CommonShaderPath @ "/VolumetricFog/gl/VFogPreV.glsl";
   OGLPixelShaderFile   = $Core::CommonShaderPath @ "/VolumetricFog/gl/VFogPreP.glsl";
   
   pixVersion = 3.0;
};
singleton ShaderData( VolumetricFogShader )
{
   DXVertexShaderFile = $Core::CommonShaderPath @ "/VolumetricFog/VFogV.hlsl";
   DXPixelShaderFile = $Core::CommonShaderPath @ "/VolumetricFog/VFogP.hlsl";
	
   OGLVertexShaderFile  = $Core::CommonShaderPath @ "/VolumetricFog/gl/VFogV.glsl";
   OGLPixelShaderFile   = $Core::CommonShaderPath @ "/VolumetricFog/gl/VFogP.glsl";	
	
   samplerNames[0] = "$deferredTex";
   samplerNames[1] = "$depthBuffer";
   samplerNames[2] = "$frontBuffer";
   samplerNames[3] = "$density";
   
   pixVersion = 3.0;
};
singleton ShaderData( VolumetricFogReflectionShader )
{
   DXVertexShaderFile = $Core::CommonShaderPath @ "/VolumetricFog/VFogPreV.hlsl";
   DXPixelShaderFile = $Core::CommonShaderPath @ "/VolumetricFog/VFogRefl.hlsl";
	
   OGLVertexShaderFile  = $Core::CommonShaderPath @ "/VolumetricFog/gl/VFogPreV.glsl";
   OGLPixelShaderFile   = $Core::CommonShaderPath @ "/VolumetricFog/gl/VFogRefl.glsl";

   pixVersion = 3.0;
};
singleton ShaderData( CubemapSaveShader )
{
   DXVertexShaderFile = "shaders/common/cubemapSaveV.hlsl";
   DXPixelShaderFile = "shaders/common/cubemapSaveP.hlsl";
	
   OGLVertexShaderFile  = "shaders/common/gl/cubemapSaveV.glsl";
   OGLPixelShaderFile   = "shaders/common/gl/cubemapSaveP.glsl";
   
   samplerNames[0] = "$cubemapTex";
	
   pixVersion = 3.0;
};