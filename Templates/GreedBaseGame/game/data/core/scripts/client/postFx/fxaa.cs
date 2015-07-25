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

// An implementation of "NVIDIA FXAA 3.11" by TIMOTHY LOTTES
//
// http://timothylottes.blogspot.com/
//
// The shader is tuned for the defaul quality and good performance.
// See shaders\common\postFx\fxaa\fxaaP.hlsl to tweak the internal
// quality and performance settings.

singleton GFXStateBlockData( FXAA_StateBlock : PFX_DefaultStateBlock )
{   
   samplersDefined = true;   
   samplerStates[0] = SamplerClampLinear;
};

singleton ShaderData( FXAA_ShaderData )
{   
   DXVertexShaderFile 	= "shaders/common/postFx/fxaa/fxaaV.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/fxaa/fxaaP.hlsl";
   
   OGLVertexShaderFile  = "shaders/common/postFx/fxaa/gl/fxaaV.glsl";
   OGLPixelShaderFile   = "shaders/common/postFx/fxaa/gl/fxaaP.glsl";
   
   samplerNames[0] = "$colorTex";

   pixVersion = 3.0;
};

singleton PostEffect( FXAA_PostEffect )
{
   isEnabled = false;
   
   allowReflectPass = false;
   renderTime = "PFXAfterDiffuse";

   texture[0] = "$backBuffer";      

   target = "$backBuffer";

   stateBlock = FXAA_StateBlock;
   shader = FXAA_ShaderData;
};

