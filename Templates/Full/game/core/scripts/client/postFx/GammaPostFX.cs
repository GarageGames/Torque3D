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

singleton ShaderData( GammaShader )
{
   DXVertexShaderFile 	= "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/gammaP.hlsl";

   OGLVertexShaderFile  = "shaders/common/postFx/gl/postFxV.glsl";
   OGLPixelShaderFile   = "shaders/common/postFx/gl/gammaP.glsl";
   
   samplerNames[0] = "$backBuffer";
   samplerNames[1] = "$colorCorrectionTex";

   pixVersion = 2.0;   
};

singleton GFXStateBlockData( GammaStateBlock : PFX_DefaultStateBlock )
{
   samplersDefined = true;
   samplerStates[0] = SamplerClampLinear;
   samplerStates[1] = SamplerClampLinear; 
};

singleton PostEffect( GammaPostFX )
{
   isEnabled = true;
   allowReflectPass = true;
   
   renderTime = "PFXBeforeBin";
   renderBin = "EditorBin";
   renderPriority = 9999;
      
   shader = GammaShader;
   stateBlock = GammaStateBlock;
   
   texture[0] = "$backBuffer";  
   texture[1] = $HDRPostFX::colorCorrectionRamp;  
   targetFormat = getBestHDRFormat();
};

function GammaPostFX::preProcess( %this )
{
   if ( %this.texture[1] !$= $HDRPostFX::colorCorrectionRamp )
      %this.setTexture( 1, $HDRPostFX::colorCorrectionRamp );         
}

function GammaPostFX::setShaderConsts( %this )
{
   %clampedGamma  = mClamp( $pref::Video::Gamma, 2.0, 2.5);
   %this.setShaderConst( "$OneOverGamma", 1 / %clampedGamma );
   %this.setShaderConst( "$Brightness", $pref::Video::Brightness );
   %this.setShaderConst( "$Contrast", $pref::Video::Contrast );
}