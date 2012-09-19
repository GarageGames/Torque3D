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

/// 
$CAPostFx::enabled = false;

/// The lens distortion coefficient.
$CAPostFx::distCoeffecient =  -0.05;

/// The cubic distortion value.
$CAPostFx::cubeDistortionFactor =  -0.1;

/// The amount and direction of the maxium shift for
/// the red, green, and blue channels.
$CAPostFx::colorDistortionFactor = "0.005 -0.005 0.01";


singleton GFXStateBlockData( PFX_DefaultChromaticLensStateBlock )
{
   zDefined = true;
   zEnable = false;
   zWriteEnable = false;   
   samplersDefined = true;
   samplerStates[0] = SamplerClampPoint;
};

singleton ShaderData( PFX_ChromaticLensShader )
{   
   DXVertexShaderFile 	= "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/chromaticLens.hlsl"; 
   pixVersion = 3.0;
};

singleton PostEffect( ChromaticLensPostFX )
{
   renderTime = "PFXAfterDiffuse";
   renderPriority = 0.2;
   isEnabled = false;
   allowReflectPass = false;

   shader = PFX_ChromaticLensShader;
   stateBlock = PFX_DefaultChromaticLensStateBlock;
   texture[0] = "$backBuffer";
   target = "$backBuffer";
};

function ChromaticLensPostFX::setShaderConsts( %this )
{
   %this.setShaderConst( "$distCoeff", $CAPostFx::distCoeffecient );   
   %this.setShaderConst( "$cubeDistort", $CAPostFx::cubeDistortionFactor );    
   %this.setShaderConst( "$colorDistort", $CAPostFx::colorDistortionFactor );   
}
