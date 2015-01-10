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

singleton GFXStateBlockData( PFX_CausticsStateBlock : PFX_DefaultStateBlock )
{
   blendDefined = true;
   blendEnable = true; 
   blendSrc = GFXBlendOne;
   blendDest = GFXBlendOne;
   
   samplersDefined = true;
   samplerStates[0] = SamplerClampLinear;
   samplerStates[1] = SamplerWrapLinear;
   samplerStates[2] = SamplerWrapLinear;
};

singleton ShaderData( PFX_CausticsShader )
{   
   DXVertexShaderFile 	= "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/caustics/causticsP.hlsl";
         
   OGLVertexShaderFile  = "shaders/common/postFx/gl//postFxV.glsl";
   OGLPixelShaderFile   = "shaders/common/postFx/caustics/gl/causticsP.glsl";
      
   samplerNames[0] = "$prepassTex";
   samplerNames[1] = "$causticsTex0";
   samplerNames[2] = "$causticsTex1";
   
   pixVersion = 3.0;
};

singleton PostEffect( CausticsPFX )
{
   isEnabled = false;
   renderTime = "PFXBeforeBin";
   renderBin = "ObjTranslucentBin";      
   //renderPriority = 0.1;
      
   shader = PFX_CausticsShader;
   stateBlock = PFX_CausticsStateBlock;
   texture[0] = "#prepass";
   texture[1] = "textures/caustics_1";
   texture[2] = "textures/caustics_2";
   target = "$backBuffer";
};
