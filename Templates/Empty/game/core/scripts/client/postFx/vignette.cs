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

$VignettePostEffect::VMax = 0.6;
$VignettePostEffect::VMin = 0.2;

singleton ShaderData( VignetteShader )
{   
   DXVertexShaderFile 	= "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/vignette/VignetteP.hlsl";
   
   OGLVertexShaderFile  = "shaders/common/postFx/gl//postFxV.glsl";
   OGLPixelShaderFile   = "shaders/common/postFx/vignette/gl/VignetteP.glsl";
   
   samplerNames[0] = "$backBuffer";
   
   pixVersion = 2.0;
};

singleton PostEffect( VignettePostEffect )  
{  
   isEnabled         = false;
   allowReflectPass  = false;
   renderTime        = "PFXAfterBin";
   renderBin         = "GlowBin";
   shader            = VignetteShader;
   stateBlock        = PFX_DefaultStateBlock;
   texture[0]        = "$backBuffer";
   renderPriority    = 10;
};

function VignettePostEffect::setShaderConsts(%this)
{
   %this.setShaderConst("$Vmax", $VignettePostEffect::VMax);
   %this.setShaderConst("$Vmin", $VignettePostEffect::VMin);
}