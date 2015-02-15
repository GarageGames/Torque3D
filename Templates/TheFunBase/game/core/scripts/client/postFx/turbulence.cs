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

singleton GFXStateBlockData( PFX_TurbulenceStateBlock : PFX_DefaultStateBlock)  
{  
   zDefined = false;
   zEnable = false;  
   zWriteEnable = false;  
        
   samplersDefined = true;  
   samplerStates[0] = SamplerClampLinear;
};  
  
singleton ShaderData( PFX_TurbulenceShader )
{   
   DXVertexShaderFile 	= "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/turbulenceP.hlsl";
           
   OGLVertexShaderFile  = "shaders/common/postFx/gl/postFxV.glsl";
   OGLPixelShaderFile   = "shaders/common/postFx/gl/turbulenceP.glsl";
           
   samplerNames[0] = "$inputTex";
   pixVersion = 3.0;
};

singleton PostEffect( TurbulenceFx )  
{  
   isEnabled = false;    
   allowReflectPass = true;  
         
   renderTime = "PFXAfterBin";
   renderBin = "GlowBin";
   renderPriority = 0.5; // Render after the glows themselves
     
   shader = PFX_TurbulenceShader;  
   stateBlock=PFX_TurbulenceStateBlock;
   texture[0] = "$backBuffer";      
 };
