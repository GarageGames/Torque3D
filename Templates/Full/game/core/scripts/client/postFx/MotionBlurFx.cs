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

singleton ShaderData( PFX_MotionBlurShader )  
{     
   DXVertexShaderFile   = "shaders/common/postFx/postFxV.hlsl";  //we use the bare-bones postFxV.hlsl
   DXPixelShaderFile    = "shaders/common/postFx/motionBlurP.hlsl";  //new pixel shader
   
   pixVersion = 3.0;  
};  

singleton PostEffect(MotionBlurFX)  
{
   isEnabled = false;

   renderTime = "PFXAfterDiffuse";  

   shader = PFX_MotionBlurShader;  
   stateBlock = PFX_DefaultStateBlock;  
   texture[0] = "$backbuffer";
   texture[1] = "#prepass";
   target = "$backBuffer";
};

function MotionBlurFX::setShaderConsts(%this)
{
   %this.setShaderConst( "$velocityMultiplier", 3000 );
}
