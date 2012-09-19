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

$PostFXManager::defaultPreset  = "core/scripts/client/postFx/default.postfxpreset.cs";

function PostFXManager::settingsSetEnabled(%this, %bEnablePostFX)
{
   $PostFXManager::PostFX::Enabled = %bEnablePostFX;
   
   //if to enable the postFX, apply the ones that are enabled
   if ( %bEnablePostFX )
   {
      //SSAO, HDR, LightRays, DOF
      
      if ( $PostFXManager::PostFX::EnableSSAO )      
         SSAOPostFx.enable();      
      else
         SSAOPostFx.disable();
      
      if ( $PostFXManager::PostFX::EnableHDR )
         HDRPostFX.enable();
      else
         HDRPostFX.disable();

      if ( $PostFXManager::PostFX::EnableLightRays )
         LightRayPostFX.enable();
      else
         LightRayPostFX.disable();
      
      if ( $PostFXManager::PostFX::EnableDOF )
         DOFPostEffect.enable();
      else
         DOFPostEffect.disable();
     
      postVerbose("% - PostFX Manager - PostFX enabled");      
   }
   else
   {
      //Disable all postFX
      
      SSAOPostFx.disable();
      HDRPostFX.disable();
      LightRayPostFX.disable();
      DOFPostEffect.disable();
      
      postVerbose("% - PostFX Manager - PostFX disabled");
   }
}

function PostFXManager::settingsEffectSetEnabled(%this, %sName, %bEnable)
{
   %postEffect = 0;
   
   //Determine the postFX to enable, and apply the boolean
   if(%sName $= "SSAO")
   {
      %postEffect = SSAOPostFx;
      $PostFXManager::PostFX::EnableSSAO = %bEnable;
      //$pref::PostFX::SSAO::Enabled = %bEnable;
   }
   else if(%sName $= "HDR")
   {
      %postEffect = HDRPostFX;
      $PostFXManager::PostFX::EnableHDR = %bEnable;
      //$pref::PostFX::HDR::Enabled = %bEnable;
   }
   else if(%sName $= "LightRays")
   {
      %postEffect = LightRayPostFX;
      $PostFXManager::PostFX::EnableLightRays = %bEnable;
      //$pref::PostFX::LightRays::Enabled = %bEnable;      
   }
   else if(%sName $= "DOF")
   {
      %postEffect = DOFPostEffect;
      $PostFXManager::PostFX::EnableDOF = %bEnable;
      //$pref::PostFX::DOF::Enabled = %bEnable;
   }
   
   // Apply the change
   if ( %bEnable == true )
   {
      %postEffect.enable();
      postVerbose("% - PostFX Manager - " @ %sName @ " enabled");
   }
   else
   {
      %postEffect.disable();
      postVerbose("% - PostFX Manager - " @ %sName @ " disabled");
   }
}

function PostFXManager::settingsRefreshSSAO(%this)
{
   //Apply the enabled flag 
   ppOptionsEnableSSAO.setValue($PostFXManager::PostFX::EnableSSAO);   
   
   //Add the items we need to display
   ppOptionsSSAOQuality.clear();
   ppOptionsSSAOQuality.add("Low", 0);
   ppOptionsSSAOQuality.add("Medium", 1);
   ppOptionsSSAOQuality.add("High", 2);
   
   //Set the selected, after adding the items!
   ppOptionsSSAOQuality.setSelected($SSAOPostFx::quality);
   
   //SSAO - Set the values of the sliders, General Tab
   ppOptionsSSAOOverallStrength.setValue($SSAOPostFx::overallStrength);
   ppOptionsSSAOBlurDepth.setValue($SSAOPostFx::blurDepthTol);
   ppOptionsSSAOBlurNormal.setValue($SSAOPostFx::blurNormalTol);
      
   //SSAO - Set the values for the near tab
   ppOptionsSSAONearDepthMax.setValue($SSAOPostFx::sDepthMax);
   ppOptionsSSAONearDepthMin.setValue($SSAOPostFx::sDepthMin);
   ppOptionsSSAONearRadius.setValue($SSAOPostFx::sRadius);
   ppOptionsSSAONearStrength.setValue($SSAOPostFx::sStrength);
   ppOptionsSSAONearToleranceNormal.setValue($SSAOPostFx::sNormalTol);
   ppOptionsSSAONearTolerancePower.setValue($SSAOPostFx::sNormalPow);
   
   //SSAO - Set the values for the far tab
   ppOptionsSSAOFarDepthMax.setValue($SSAOPostFx::lDepthMax);
   ppOptionsSSAOFarDepthMin.setValue($SSAOPostFx::lDepthMin);
   ppOptionsSSAOFarRadius.setValue($SSAOPostFx::lRadius);
   ppOptionsSSAOFarStrength.setValue($SSAOPostFx::lStrength);
   ppOptionsSSAOFarToleranceNormal.setValue($SSAOPostFx::lNormalTol);
   ppOptionsSSAOFarTolerancePower.setValue($SSAOPostFx::lNormalPow);
}

function PostFXManager::settingsRefreshHDR(%this)
{
  //Apply the enabled flag 
   ppOptionsEnableHDR.setValue($PostFXManager::PostFX::EnableHDR);   
    
   ppOptionsHDRBloom.setValue($HDRPostFX::enableBloom);
   ppOptionsHDRBloomBlurBrightPassThreshold.setValue($HDRPostFX::brightPassThreshold);
   ppOptionsHDRBloomBlurMean.setValue($HDRPostFX::gaussMean);
   ppOptionsHDRBloomBlurMultiplier.setValue($HDRPostFX::gaussMultiplier);
   ppOptionsHDRBloomBlurStdDev.setValue($HDRPostFX::gaussStdDev);
   ppOptionsHDRBrightnessAdaptRate.setValue($HDRPostFX::adaptRate);
   ppOptionsHDREffectsBlueShift.setValue($HDRPostFX::enableBlueShift);
   ppOptionsHDREffectsBlueShiftColor.BaseColor = $HDRPostFX::blueShiftColor;
   ppOptionsHDREffectsBlueShiftColor.PickColor = $HDRPostFX::blueShiftColor;
   ppOptionsHDRKeyValue.setValue($HDRPostFX::keyValue);
   ppOptionsHDRMinLuminance.setValue($HDRPostFX::minLuminace);
   ppOptionsHDRToneMapping.setValue($HDRPostFX::enableToneMapping);
   ppOptionsHDRToneMappingAmount.setValue($HDRPostFX::enableToneMapping);
   ppOptionsHDRWhiteCutoff.setValue($HDRPostFX::whiteCutoff);
   
   %this-->ColorCorrectionFileName.Text = $HDRPostFX::colorCorrectionRamp;    
}

function PostFXManager::settingsRefreshLightrays(%this)
{
  //Apply the enabled flag 
   ppOptionsEnableLightRays.setValue($PostFXManager::PostFX::EnableLightRays);   
    
   ppOptionsLightRaysBrightScalar.setValue($LightRayPostFX::brightScalar);
}

function PostFXManager::settingsRefreshDOF(%this)
{
  //Apply the enabled flag 
   ppOptionsEnableDOF.setValue($PostFXManager::PostFX::EnableDOF);   


   //ppOptionsDOFEnableDOF.setValue($PostFXManager::PostFX::EnableDOF);
   ppOptionsDOFEnableAutoFocus.setValue($DOFPostFx::EnableAutoFocus);
   
   ppOptionsDOFFarBlurMinSlider.setValue($DOFPostFx::BlurMin);
   ppOptionsDOFFarBlurMaxSlider.setValue($DOFPostFx::BlurMax);
   
   ppOptionsDOFFocusRangeMinSlider.setValue($DOFPostFx::FocusRangeMin);
   ppOptionsDOFFocusRangeMaxSlider.setValue($DOFPostFx::FocusRangeMax);
   
   ppOptionsDOFBlurCurveNearSlider.setValue($DOFPostFx::BlurCurveNear);
   ppOptionsDOFBlurCurveFarSlider.setValue($DOFPostFx::BlurCurveFar);

}

function PostFXManager::settingsRefreshAll(%this)
{    
   $PostFXManager::PostFX::Enabled           = $pref::enablePostEffects;
   $PostFXManager::PostFX::EnableSSAO        = SSAOPostFx.isEnabled();
   $PostFXManager::PostFX::EnableHDR         = HDRPostFX.isEnabled();
   $PostFXManager::PostFX::EnableLightRays   = LightRayPostFX.isEnabled();
   $PostFXManager::PostFX::EnableDOF         = DOFPostEffect.isEnabled();
   
   //For all the postFX here, apply the active settings in the system
   //to the gui controls.
   
   %this.settingsRefreshSSAO();
   %this.settingsRefreshHDR();
   %this.settingsRefreshLightrays();
   %this.settingsRefreshDOF();
   
   ppOptionsEnable.setValue($PostFXManager::PostFX::Enabled);

   postVerbose("% - PostFX Manager - GUI values updated.");
}

function PostFXManager::settingsApplyFromPreset(%this)
{
   postVerbose("% - PostFX Manager - Applying from preset");

   //SSAO Settings
   $SSAOPostFx::blurDepthTol           = $PostFXManager::Settings::SSAO::blurDepthTol;
   $SSAOPostFx::blurNormalTol          = $PostFXManager::Settings::SSAO::blurNormalTol;
   $SSAOPostFx::lDepthMax              = $PostFXManager::Settings::SSAO::lDepthMax;
   $SSAOPostFx::lDepthMin              = $PostFXManager::Settings::SSAO::lDepthMin;
   $SSAOPostFx::lDepthPow              = $PostFXManager::Settings::SSAO::lDepthPow;
   $SSAOPostFx::lNormalPow             = $PostFXManager::Settings::SSAO::lNormalPow;
   $SSAOPostFx::lNormalTol             = $PostFXManager::Settings::SSAO::lNormalTol;
   $SSAOPostFx::lRadius                = $PostFXManager::Settings::SSAO::lRadius;
   $SSAOPostFx::lStrength              = $PostFXManager::Settings::SSAO::lStrength;
   $SSAOPostFx::overallStrength        = $PostFXManager::Settings::SSAO::overallStrength;
   $SSAOPostFx::quality                = $PostFXManager::Settings::SSAO::quality;
   $SSAOPostFx::sDepthMax              = $PostFXManager::Settings::SSAO::sDepthMax;
   $SSAOPostFx::sDepthMin              = $PostFXManager::Settings::SSAO::sDepthMin;
   $SSAOPostFx::sDepthPow              = $PostFXManager::Settings::SSAO::sDepthPow;
   $SSAOPostFx::sNormalPow             = $PostFXManager::Settings::SSAO::sNormalPow;
   $SSAOPostFx::sNormalTol             = $PostFXManager::Settings::SSAO::sNormalTol;
   $SSAOPostFx::sRadius                = $PostFXManager::Settings::SSAO::sRadius;
   $SSAOPostFx::sStrength              = $PostFXManager::Settings::SSAO::sStrength;
   
   //HDR settings
   $HDRPostFX::adaptRate               = $PostFXManager::Settings::HDR::adaptRate;
   $HDRPostFX::blueShiftColor          = $PostFXManager::Settings::HDR::blueShiftColor;
   $HDRPostFX::brightPassThreshold     = $PostFXManager::Settings::HDR::brightPassThreshold; 
   $HDRPostFX::enableBloom             = $PostFXManager::Settings::HDR::enableBloom;
   $HDRPostFX::enableBlueShift         = $PostFXManager::Settings::HDR::enableBlueShift;
   $HDRPostFX::enableToneMapping       = $PostFXManager::Settings::HDR::enableToneMapping;
   $HDRPostFX::gaussMean               = $PostFXManager::Settings::HDR::gaussMean;
   $HDRPostFX::gaussMultiplier         = $PostFXManager::Settings::HDR::gaussMultiplier;
   $HDRPostFX::gaussStdDev             = $PostFXManager::Settings::HDR::gaussStdDev;
   $HDRPostFX::keyValue                = $PostFXManager::Settings::HDR::keyValue;
   $HDRPostFX::minLuminace             = $PostFXManager::Settings::HDR::minLuminace;
   $HDRPostFX::whiteCutoff             = $PostFXManager::Settings::HDR::whiteCutoff;
   $HDRPostFX::colorCorrectionRamp     = $PostFXManager::Settings::ColorCorrectionRamp;
   
   //Light rays settings
   $LightRayPostFX::brightScalar       = $PostFXManager::Settings::LightRays::brightScalar;
   
   //DOF settings   
   $DOFPostFx::EnableAutoFocus         = $PostFXManager::Settings::DOF::EnableAutoFocus;
   $DOFPostFx::BlurMin                 = $PostFXManager::Settings::DOF::BlurMin;
   $DOFPostFx::BlurMax                 = $PostFXManager::Settings::DOF::BlurMax;
   $DOFPostFx::FocusRangeMin           = $PostFXManager::Settings::DOF::FocusRangeMin;
   $DOFPostFx::FocusRangeMax           = $PostFXManager::Settings::DOF::FocusRangeMax;
   $DOFPostFx::BlurCurveNear           = $PostFXManager::Settings::DOF::BlurCurveNear;
   $DOFPostFx::BlurCurveFar            = $PostFXManager::Settings::DOF::BlurCurveFar;
  
   if ( $PostFXManager::forceEnableFromPresets )
   {
      $PostFXManager::PostFX::Enabled           = $PostFXManager::Settings::EnablePostFX;
      $PostFXManager::PostFX::EnableDOF         = $PostFXManager::Settings::EnableDOF;
      $PostFXManager::PostFX::EnableLightRays   = $PostFXManager::Settings::EnableLightRays;
      $PostFXManager::PostFX::EnableHDR         = $PostFXManager::Settings::EnableHDR;
      $PostFXManager::PostFX::EnableSSAO        = $PostFXManager::Settings::EnabledSSAO;

      %this.settingsSetEnabled( true );
   }
   
   //make sure we apply the correct settings to the DOF
   ppOptionsUpdateDOFSettings();
   
   // Update the actual GUI controls if its awake ( otherwise it will when opened ).
   if ( PostFXManager.isAwake() )
      %this.settingsRefreshAll();      
}

function PostFXManager::settingsApplySSAO(%this)
{   
   $PostFXManager::Settings::SSAO::blurDepthTol             = $SSAOPostFx::blurDepthTol;
   $PostFXManager::Settings::SSAO::blurNormalTol            = $SSAOPostFx::blurNormalTol;
   $PostFXManager::Settings::SSAO::lDepthMax                = $SSAOPostFx::lDepthMax;
   $PostFXManager::Settings::SSAO::lDepthMin                = $SSAOPostFx::lDepthMin;
   $PostFXManager::Settings::SSAO::lDepthPow                = $SSAOPostFx::lDepthPow;
   $PostFXManager::Settings::SSAO::lNormalPow               = $SSAOPostFx::lNormalPow;
   $PostFXManager::Settings::SSAO::lNormalTol               = $SSAOPostFx::lNormalTol;
   $PostFXManager::Settings::SSAO::lRadius                  = $SSAOPostFx::lRadius;
   $PostFXManager::Settings::SSAO::lStrength                = $SSAOPostFx::lStrength;
   $PostFXManager::Settings::SSAO::overallStrength          = $SSAOPostFx::overallStrength;
   $PostFXManager::Settings::SSAO::quality                  = $SSAOPostFx::quality;
   $PostFXManager::Settings::SSAO::sDepthMax                = $SSAOPostFx::sDepthMax;
   $PostFXManager::Settings::SSAO::sDepthMin                = $SSAOPostFx::sDepthMin;
   $PostFXManager::Settings::SSAO::sDepthPow                = $SSAOPostFx::sDepthPow;
   $PostFXManager::Settings::SSAO::sNormalPow               = $SSAOPostFx::sNormalPow;
   $PostFXManager::Settings::SSAO::sNormalTol               = $SSAOPostFx::sNormalTol;
   $PostFXManager::Settings::SSAO::sRadius                  = $SSAOPostFx::sRadius;
   $PostFXManager::Settings::SSAO::sStrength                = $SSAOPostFx::sStrength;

   postVerbose("% - PostFX Manager - Settings Saved - SSAO");    
   
}

function PostFXManager::settingsApplyHDR(%this)
{   
   $PostFXManager::Settings::HDR::adaptRate                 = $HDRPostFX::adaptRate;
   $PostFXManager::Settings::HDR::blueShiftColor            = $HDRPostFX::blueShiftColor;
   $PostFXManager::Settings::HDR::brightPassThreshold       = $HDRPostFX::brightPassThreshold;
   $PostFXManager::Settings::HDR::enableBloom               = $HDRPostFX::enableBloom;
   $PostFXManager::Settings::HDR::enableBlueShift           = $HDRPostFX::enableBlueShift;
   $PostFXManager::Settings::HDR::enableToneMapping         = $HDRPostFX::enableToneMapping;
   $PostFXManager::Settings::HDR::gaussMean                 = $HDRPostFX::gaussMean;
   $PostFXManager::Settings::HDR::gaussMultiplier           = $HDRPostFX::gaussMultiplier;
   $PostFXManager::Settings::HDR::gaussStdDev               = $HDRPostFX::gaussStdDev;
   $PostFXManager::Settings::HDR::keyValue                  = $HDRPostFX::keyValue;
   $PostFXManager::Settings::HDR::minLuminace               = $HDRPostFX::minLuminace;
   $PostFXManager::Settings::HDR::whiteCutoff               = $HDRPostFX::whiteCutoff;
   $PostFXManager::Settings::ColorCorrectionRamp            = $HDRPostFX::colorCorrectionRamp;
   
   postVerbose("% - PostFX Manager - Settings Saved - HDR");      
}

function PostFXManager::settingsApplyLightRays(%this)
{   
   $PostFXManager::Settings::LightRays::brightScalar        = $LightRayPostFX::brightScalar;
   
   postVerbose("% - PostFX Manager - Settings Saved - Light Rays");   
   
}

function PostFXManager::settingsApplyDOF(%this)
{
   $PostFXManager::Settings::DOF::EnableAutoFocus           = $DOFPostFx::EnableAutoFocus;   
   $PostFXManager::Settings::DOF::BlurMin                   = $DOFPostFx::BlurMin;
   $PostFXManager::Settings::DOF::BlurMax                   = $DOFPostFx::BlurMax;
   $PostFXManager::Settings::DOF::FocusRangeMin             = $DOFPostFx::FocusRangeMin;
   $PostFXManager::Settings::DOF::FocusRangeMax             = $DOFPostFx::FocusRangeMax;
   $PostFXManager::Settings::DOF::BlurCurveNear             = $DOFPostFx::BlurCurveNear;
   $PostFXManager::Settings::DOF::BlurCurveFar              = $DOFPostFx::BlurCurveFar;
   
   postVerbose("% - PostFX Manager - Settings Saved - DOF");   
   
}

function PostFXManager::settingsApplyAll(%this, %sFrom)
{
   // Apply settings which control if effects are on/off altogether.
   $PostFXManager::Settings::EnablePostFX        = $PostFXManager::PostFX::Enabled;  
   $PostFXManager::Settings::EnableDOF           = $PostFXManager::PostFX::EnableDOF;
   $PostFXManager::Settings::EnableLightRays     = $PostFXManager::PostFX::EnableLightRays;
   $PostFXManager::Settings::EnableHDR           = $PostFXManager::PostFX::EnableHDR;
   $PostFXManager::Settings::EnabledSSAO         = $PostFXManager::PostFX::EnableSSAO;
      
   // Apply settings should save the values in the system to the 
   // the preset structure ($PostFXManager::Settings::*)

   // SSAO Settings
   %this.settingsApplySSAO();
   // HDR settings
   %this.settingsApplyHDR();
   // Light rays settings
   %this.settingsApplyLightRays();
   // DOF
   %this.settingsApplyDOF();
   
   postVerbose("% - PostFX Manager - All Settings applied to $PostFXManager::Settings");
}

function PostFXManager::settingsApplyDefaultPreset(%this)
{
   PostFXManager::loadPresetHandler($PostFXManager::defaultPreset);
}

