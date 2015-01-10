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


/// Blends between the scene and the tone mapped scene.
$HDRPostFX::enableToneMapping = 1.0;

/// The tone mapping middle grey or exposure value used
/// to adjust the overall "balance" of the image.
///
/// 0.18 is fairly common value.
///
$HDRPostFX::keyValue = 0.18;

/// The minimum luninace value to allow when tone mapping 
/// the scene.  Is particularly useful if your scene very 
/// dark or has a black ambient color in places.
$HDRPostFX::minLuminace = 0.001;

/// The lowest luminance value which is mapped to white.  This
/// is usually set to the highest visible luminance in your 
/// scene.  By setting this to smaller values you get a contrast
/// enhancement.
$HDRPostFX::whiteCutoff = 1.0;

/// The rate of adaptation from the previous and new 
/// average scene luminance. 
$HDRPostFX::adaptRate = 2.0;


/// Blends between the scene and the blue shifted version
/// of the scene for a cinematic desaturated night effect.
$HDRPostFX::enableBlueShift = 0.0;

/// The blue shift color value.
$HDRPostFX::blueShiftColor = "1.05 0.97 1.27";


/// Blends between the scene and the bloomed scene.
$HDRPostFX::enableBloom = 1.0;

/// The threshold luminace value for pixels which are
/// considered "bright" and need to be bloomed.
$HDRPostFX::brightPassThreshold = 1.0;

/// These are used in the gaussian blur of the
/// bright pass for the bloom effect.
$HDRPostFX::gaussMultiplier = 0.3;
$HDRPostFX::gaussMean = 0.0;
$HDRPostFX::gaussStdDev = 0.8;

/// The 1x255 color correction ramp texture used
/// by both the HDR shader and the GammaPostFx shader
/// for doing full screen color correction. 
$HDRPostFX::colorCorrectionRamp = "core/scripts/client/postFx/null_color_ramp.png";


singleton ShaderData( HDR_BrightPassShader )
{
   DXVertexShaderFile 	= "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/hdr/brightPassFilterP.hlsl";   
   OGLVertexShaderFile  = "shaders/common/postFx/gl/postFxV.glsl";
   OGLPixelShaderFile   = "shaders/common/postFx/hdr/gl/brightPassFilterP.glsl";
   
   samplerNames[0] = "$inputTex";
   samplerNames[1] = "$luminanceTex";
   
   pixVersion = 3.0;
};

singleton ShaderData( HDR_DownScale4x4Shader )
{
   DXVertexShaderFile 	= "shaders/common/postFx/hdr/downScale4x4V.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/hdr/downScale4x4P.hlsl";
   OGLVertexShaderFile  = "shaders/common/postFx/hdr/gl/downScale4x4V.glsl";
   OGLPixelShaderFile   = "shaders/common/postFx/hdr/gl/downScale4x4P.glsl";
   
   samplerNames[0] = "$inputTex";
   
   pixVersion = 2.0;
};

singleton ShaderData( HDR_BloomGaussBlurHShader )
{
   DXVertexShaderFile 	= "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/hdr/bloomGaussBlurHP.hlsl";
   OGLVertexShaderFile  = "shaders/common/postFx/gl/postFxV.glsl";
   OGLPixelShaderFile   = "shaders/common/postFx/hdr/gl/bloomGaussBlurHP.glsl";
   
   samplerNames[0] = "$inputTex";
   
   pixVersion = 3.0;
};

singleton ShaderData( HDR_BloomGaussBlurVShader )
{
   DXVertexShaderFile 	= "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/hdr/bloomGaussBlurVP.hlsl";
   OGLVertexShaderFile  = "shaders/common/postFx/gl/postFxV.glsl";
   OGLPixelShaderFile   = "shaders/common/postFx/hdr/gl/bloomGaussBlurVP.glsl";
   
   samplerNames[0] = "$inputTex";
   
   pixVersion = 3.0;
};

singleton ShaderData( HDR_SampleLumShader )
{
   DXVertexShaderFile 	= "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/hdr/sampleLumInitialP.hlsl";
   OGLVertexShaderFile  = "shaders/common/postFx/gl/postFxV.glsl";
   OGLPixelShaderFile   = "shaders/common/postFx/hdr/gl/sampleLumInitialP.glsl";
   
   samplerNames[0] = "$inputTex";
   
   pixVersion = 3.0;
};

singleton ShaderData( HDR_DownSampleLumShader )
{
   DXVertexShaderFile 	= "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/hdr/sampleLumIterativeP.hlsl";
   OGLVertexShaderFile  = "shaders/common/postFx/gl/postFxV.glsl";
   OGLPixelShaderFile   = "shaders/common/postFx/hdr/gl/sampleLumIterativeP.glsl";
   
   samplerNames[0] = "$inputTex";
   
   pixVersion = 3.0;
};

singleton ShaderData( HDR_CalcAdaptedLumShader )
{
   DXVertexShaderFile 	= "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/hdr/calculateAdaptedLumP.hlsl";
   OGLVertexShaderFile  = "shaders/common/postFx/gl/postFxV.glsl";
   OGLPixelShaderFile   = "shaders/common/postFx/hdr/gl/calculateAdaptedLumP.glsl";
   
   samplerNames[0] = "$currLum";
   samplerNames[1] = "$lastAdaptedLum";
   
   pixVersion = 3.0;
};

singleton ShaderData( HDR_CombineShader )
{
   DXVertexShaderFile 	= "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/hdr/finalPassCombineP.hlsl";
   OGLVertexShaderFile  = "shaders/common/postFx/gl/postFxV.glsl";
   OGLPixelShaderFile   = "shaders/common/postFx/hdr/gl/finalPassCombineP.glsl";
   
   samplerNames[0] = "$sceneTex";
   samplerNames[1] = "$luminanceTex";
   samplerNames[2] = "$bloomTex";
   samplerNames[3] = "$colorCorrectionTex";
   
   pixVersion = 3.0;
};


singleton GFXStateBlockData( HDR_SampleStateBlock : PFX_DefaultStateBlock )
{
   samplersDefined = true;
   samplerStates[0] = SamplerClampPoint;
   samplerStates[1] = SamplerClampPoint;
};

singleton GFXStateBlockData( HDR_DownSampleStateBlock : PFX_DefaultStateBlock )
{
   samplersDefined = true;
   samplerStates[0] = SamplerClampLinear;
   samplerStates[1] = SamplerClampLinear;
};

singleton GFXStateBlockData( HDR_CombineStateBlock : PFX_DefaultStateBlock )
{
   samplersDefined = true;
   samplerStates[0] = SamplerClampPoint;
   samplerStates[1] = SamplerClampLinear;
   samplerStates[2] = SamplerClampLinear;
   samplerStates[3] = SamplerClampLinear;
};

singleton GFXStateBlockData( HDRStateBlock )
{
   samplersDefined = true;
   samplerStates[0] = SamplerClampLinear;
   samplerStates[1] = SamplerClampLinear;
   samplerStates[2] = SamplerClampLinear;
   samplerStates[3] = SamplerClampLinear;
   
   blendDefined = true;
   blendDest = GFXBlendOne;
   blendSrc = GFXBlendZero;
   
   zDefined = true;
   zEnable = false;
   zWriteEnable = false;
   
   cullDefined = true;
   cullMode = GFXCullNone;
};


function HDRPostFX::setShaderConsts( %this )
{
   %this.setShaderConst( "$brightPassThreshold", $HDRPostFX::brightPassThreshold );
   %this.setShaderConst( "$g_fMiddleGray", $HDRPostFX::keyValue );   
         
   %bloomH = %this-->bloomH;
   %bloomH.setShaderConst( "$gaussMultiplier", $HDRPostFX::gaussMultiplier );
   %bloomH.setShaderConst( "$gaussMean", $HDRPostFX::gaussMean );
   %bloomH.setShaderConst( "$gaussStdDev", $HDRPostFX::gaussStdDev );   

   %bloomV = %this-->bloomV;
   %bloomV.setShaderConst( "$gaussMultiplier", $HDRPostFX::gaussMultiplier );
   %bloomV.setShaderConst( "$gaussMean", $HDRPostFX::gaussMean );
   %bloomV.setShaderConst( "$gaussStdDev", $HDRPostFX::gaussStdDev );   

   %minLuminace = $HDRPostFX::minLuminace;
   if ( %minLuminace <= 0.0 )
   {
      // The min should never be pure zero else the
      // log() in the shader will generate INFs.      
      %minLuminace = 0.00001;
   }
   %this-->adaptLum.setShaderConst( "$g_fMinLuminace", %minLuminace );
        
   %this-->finalLum.setShaderConst( "$adaptRate", $HDRPostFX::adaptRate );
   
   %combinePass = %this-->combinePass;   
   %combinePass.setShaderConst( "$g_fEnableToneMapping", $HDRPostFX::enableToneMapping );
   %combinePass.setShaderConst( "$g_fMiddleGray", $HDRPostFX::keyValue );
   %combinePass.setShaderConst( "$g_fBloomScale", $HDRPostFX::enableBloom );      
   %combinePass.setShaderConst( "$g_fEnableBlueShift", $HDRPostFX::enableBlueShift );   
   %combinePass.setShaderConst( "$g_fBlueShiftColor", $HDRPostFX::blueShiftColor );   
   
   %clampedGamma  = mClamp( $pref::Video::Gamma, 0.001, 2.2);
   %combinePass.setShaderConst( "$g_fOneOverGamma",  1 / %clampedGamma );       

   %whiteCutoff = ( $HDRPostFX::whiteCutoff * $HDRPostFX::whiteCutoff ) *
                  ( $HDRPostFX::whiteCutoff * $HDRPostFX::whiteCutoff );                  
   %combinePass.setShaderConst( "$g_fWhiteCutoff", %whiteCutoff );
}

function HDRPostFX::preProcess( %this )
{
   %combinePass = %this-->combinePass;
   
   if ( %combinePass.texture[3] !$= $HDRPostFX::colorCorrectionRamp )
      %combinePass.setTexture( 3, $HDRPostFX::colorCorrectionRamp );         
}

function HDRPostFX::onEnabled( %this )
{
   // We don't allow hdr on OSX yet.
   if ( $platform $= "macos" )
      return false;
      
   // See what HDR format would be best.
   %format = getBestHDRFormat();
   if ( %format $= "" || %format $= "GFXFormatR8G8B8A8" )
   {
      // We didn't get a valid HDR format... so fail.
      return false;
   }
   
   // HDR does it's own gamma calculation so 
   // disable this postFx.
   GammaPostFX.disable();
   
   // Set the right global shader define for HDR.
   if ( %format $= "GFXFormatR10G10B10A2" )
      addGlobalShaderMacro( "TORQUE_HDR_RGB10" );
   else if ( %format $= "GFXFormatR16G16B16A16" )
      addGlobalShaderMacro( "TORQUE_HDR_RGB16" );
                        
   echo( "HDR FORMAT: " @ %format );
   
   // Change the format of the offscreen surface
   // to an HDR compatible format.
   AL_FormatToken.format = %format;
   setReflectFormat( %format );
   
   // Reset the light manager which will ensure the new
   // hdr encoding takes effect in all the shaders and
   // that the offscreen surface is enabled.
   resetLightManager();
         
   return true;
}

function HDRPostFX::onDisabled( %this )
{
   // Enable a special GammaCorrection PostFX when this is disabled.
   GammaPostFX.enable();
   
   // Restore the non-HDR offscreen surface format.
   %format = "GFXFormatR8G8B8A8";
   AL_FormatToken.format = %format;
   setReflectFormat( %format );
   
   removeGlobalShaderMacro( "TORQUE_HDR_RGB10" );
   removeGlobalShaderMacro( "TORQUE_HDR_RGB16" );
            
   // Reset the light manager which will ensure the new
   // hdr encoding takes effect in all the shaders.
   resetLightManager();
}

singleton PostEffect( HDRPostFX )
{
   isEnabled = false;
   allowReflectPass = false;
      
   // Resolve the HDR before we render any editor stuff
   // and before we resolve the scene to the backbuffer.
   renderTime = "PFXBeforeBin";
   renderBin = "EditorBin";
   renderPriority = 9999;
      
   // The bright pass generates a bloomed version of 
   // the scene for pixels which are brighter than a 
   // fixed threshold value.
   //
   // This is then used in the final HDR combine pass
   // at the end of this post effect chain.
   //
                                
      shader = HDR_BrightPassShader;
      stateBlock = HDR_DownSampleStateBlock;
      texture[0] = "$backBuffer";
      texture[1] = "#adaptedLum";
      target = "$outTex";
      targetFormat = "GFXFormatR16G16B16A16F"; 
      targetScale = "0.5 0.5";
      
      new PostEffect()
      {
         shader = HDR_DownScale4x4Shader;
         stateBlock = HDR_DownSampleStateBlock;
         texture[0] = "$inTex";
         target = "$outTex";
         targetFormat = "GFXFormatR16G16B16A16F";
         targetScale = "0.25 0.25";
      };
      
      new PostEffect()
      {
         internalName = "bloomH";
         
         shader = HDR_BloomGaussBlurHShader;
         stateBlock = HDR_DownSampleStateBlock;
         texture[0] = "$inTex";
         target = "$outTex";
         targetFormat = "GFXFormatR16G16B16A16F";   
      };

      new PostEffect()
      {
         internalName = "bloomV";
                  
         shader = HDR_BloomGaussBlurVShader;
         stateBlock = HDR_DownSampleStateBlock;
         texture[0] = "$inTex";
         target = "#bloomFinal";
         targetFormat = "GFXFormatR16G16B16A16F";    
      };

   // BrightPass End
   
   // Now calculate the adapted luminance.
   new PostEffect()
   {
      internalName = "adaptLum";
      
      shader = HDR_SampleLumShader;
      stateBlock = HDR_DownSampleStateBlock;
      texture[0] = "$backBuffer";
      target = "$outTex";
      targetScale = "0.0625 0.0625"; // 1/16th
      targetFormat = "GFXFormatR16F";
      
      new PostEffect()
      {
         shader = HDR_DownSampleLumShader;
         stateBlock = HDR_DownSampleStateBlock;
         texture[0] = "$inTex";
         target = "$outTex";
         targetScale = "0.25 0.25"; // 1/4
         targetFormat = "GFXFormatR16F";
      };
      
      new PostEffect()
      {
         shader = HDR_DownSampleLumShader;
         stateBlock = HDR_DownSampleStateBlock;
         texture[0] = "$inTex";
         target = "$outTex";
         targetScale = "0.25 0.25"; // 1/4
         targetFormat = "GFXFormatR16F";
      };
      
      new PostEffect()
      {
         shader = HDR_DownSampleLumShader;
         stateBlock = HDR_DownSampleStateBlock;
         texture[0] = "$inTex";
         target = "$outTex";
         targetScale = "0.25 0.25"; // At this point the target should be 1x1.
         targetFormat = "GFXFormatR16F";
      };

      // Note that we're reading the adapted luminance
      // from the previous frame when generating this new
      // one... PostEffect takes care to manage that.
      new PostEffect()
      {
         internalName = "finalLum";         
         shader = HDR_CalcAdaptedLumShader;
         stateBlock = HDR_DownSampleStateBlock;
         texture[0] = "$inTex";
         texture[1] = "#adaptedLum";
         target = "#adaptedLum";
         targetFormat = "GFXFormatR16F";
         targetClear = "PFXTargetClear_OnCreate";
         targetClearColor = "1 1 1 1";
      };
   };
   
   // Output the combined bloom and toned mapped
   // version of the scene.
   new PostEffect()
   {
      internalName = "combinePass";
      
      shader = HDR_CombineShader;
      stateBlock = HDR_CombineStateBlock;
      texture[0] = "$backBuffer";
      texture[1] = "#adaptedLum";            
      texture[2] = "#bloomFinal";
      texture[3] = $HDRPostFX::colorCorrectionRamp;
      target = "$backBuffer";
   };
};

singleton ShaderData( LuminanceVisShader )
{
   DXVertexShaderFile 	= "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/hdr/luminanceVisP.hlsl";
   OGLVertexShaderFile  = "shaders/common/postFx/gl/postFxV.glsl";
   OGLPixelShaderFile   = "shaders/common/postFx/hdr/gl/luminanceVisP.glsl";
   
   samplerNames[0] = "$inputTex";
   
   pixVersion = 3.0;
};

singleton GFXStateBlockData( LuminanceVisStateBlock : PFX_DefaultStateBlock )
{
   samplersDefined = true;
   samplerStates[0] = SamplerClampLinear;   
};

function LuminanceVisPostFX::setShaderConsts( %this )
{
   %this.setShaderConst( "$brightPassThreshold", $HDRPostFX::brightPassThreshold );
}

singleton PostEffect( LuminanceVisPostFX )
{
   isEnabled = false;
   allowReflectPass = false;
      
   // Render before we do any editor rendering.  
   renderTime = "PFXBeforeBin";
   renderBin = "EditorBin";
   renderPriority = 9999;
   
   shader = LuminanceVisShader;
   stateBlock = LuminanceVisStateBlock;
   texture[0] = "$backBuffer";
   target = "$backBuffer";
   //targetScale = "0.0625 0.0625"; // 1/16th
   //targetFormat = "GFXFormatR16F";
};

function LuminanceVisPostFX::onEnabled( %this )
{
   if ( !HDRPostFX.isEnabled() )
   {
      HDRPostFX.enable();
   }
   
   HDRPostFX.skip = true;
   
   return true;
}

function LuminanceVisPostFX::onDisabled( %this )
{      
   HDRPostFX.skip = false; 
}

