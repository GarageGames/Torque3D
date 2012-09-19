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

//------------------------------------------------------------------------------
// Fog
//------------------------------------------------------------------------------

singleton ShaderData( FogPassShader )
{   
   DXVertexShaderFile 	= "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/fogP.hlsl";
         
//   OGLVertexShaderFile  = "shaders/common/postFx/gl//postFxV.glsl";
//   OGLPixelShaderFile   = "shaders/common/postFx/gl/fogP.glsl";
            
   samplerNames[0] = "$prepassTex";
   
   pixVersion = 2.0;
};


singleton GFXStateBlockData( FogPassStateBlock : PFX_DefaultStateBlock )
{   
   blendDefined = true;
   blendEnable = true; 
   blendSrc = GFXBlendSrcAlpha;
   blendDest = GFXBlendInvSrcAlpha;
};


singleton PostEffect( FogPostFx )
{   
   // We forward render the reflection pass
   // so it does its own fogging.
   allowReflectPass = false;
      
   renderTime = "PFXBeforeBin";
   renderBin = "ObjTranslucentBin";   
   
   shader = FogPassShader;
   stateBlock = FogPassStateBlock;
   texture[0] = "#prepass";
   
   renderPriority = 5;
   
   isEnabled = true;
};


//------------------------------------------------------------------------------
// UnderwaterFog
//------------------------------------------------------------------------------

singleton ShaderData( UnderwaterFogPassShader )
{   
   DXVertexShaderFile 	= "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/underwaterFogP.hlsl";
         
//   OGLVertexShaderFile  = "shaders/common/postFx/gl/postFxV.glsl";
//   OGLPixelShaderFile   = "shaders/common/postFx/gl/fogP.glsl";
            
   samplerNames[0] = "$prepassTex";
   
   pixVersion = 2.0;      
};


singleton GFXStateBlockData( UnderwaterFogPassStateBlock : PFX_DefaultStateBlock )
{   
   samplersDefined = true;
   samplerStates[0] = SamplerClampPoint;
   samplerStates[1] = SamplerClampPoint;   
   samplerStates[2] = SamplerClampLinear;
};


singleton PostEffect( UnderwaterFogPostFx )
{
   oneFrameOnly = true;
   onThisFrame = false;
   
   // Let the fog effect render during the 
   // reflection pass.
   allowReflectPass = true;
      
   renderTime = "PFXBeforeBin";
   renderBin = "ObjTranslucentBin";   
  
   shader = UnderwaterFogPassShader;
   stateBlock = UnderwaterFogPassStateBlock;
   texture[0] = "#prepass";
   texture[1] = "$backBuffer";
   texture[2] = "#waterDepthGradMap";
   
   // Needs to happen after the FogPostFx
   renderPriority = 4;
   
   isEnabled = true;
};

