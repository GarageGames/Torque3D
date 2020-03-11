// =============================================================================
// GRAPHICS MENU
// =============================================================================
//Mesh and Textures
//
function GraphicsMenu::onWake(%this)
{
   DisplaySettingsMenu.hidden = false;
   GeneralGraphicsSettingsMenu.hidden = true;
   
   %this.refresh();
}

function GraphicsMenu::refresh(%this)
{
   //
   // Display Menu
   GraphicsMenuFullScreen.setStateOn( Canvas.isFullScreen() );
   GraphicsMenuVSync.setStateOn( !$pref::Video::disableVerticalSync );
   
   %this.initResMenu();
   %resSelId = GraphicsMenuResolution.findText( _makePrettyResString( $pref::Video::mode ) );
   if( %resSelId != -1 )
      GraphicsMenuResolution.setSelected( %resSelId );
   
   GraphicsMenuDriver.clear();
   
   %buffer = getDisplayDeviceList();
   %count = getFieldCount( %buffer );   
   for(%i = 0; %i < %count; %i++)
      GraphicsMenuDriver.add(getField(%buffer, %i), %i);

   %selId = GraphicsMenuDriver.findText( getDisplayDeviceInformation() );
	if ( %selId == -1 )
		GraphicsMenuDriver.setFirstSelected();
   else
	   GraphicsMenuDriver.setSelected( %selId );
   //

   //
   // General Graphics menu
   GraphicsMenuShadowQlty.init(ShadowQualityList);
   GraphicsMenuSoftShadow.init(SoftShadowList);
   
   GraphicsMenuModelDtl.init(MeshQualityGroup);
   GraphicsMenuTextureDtl.init(TextureQualityGroup);
   GraphicsMenuTerrainDtl.init(TerrainQualityGroup);
   GraphicsMenuDecalLife.init(DecalLifetimeGroup);
   GraphicsMenuGroundClutter.init(GroundCoverDensityGroup);
   
   GraphicsMenuMaterialQlty.init(ShaderQualityGroup);
   
   // Setup the anisotropic filtering menu.
   %ansioCtrl = GraphicsMenuAniso;
   %ansioCtrl.clear();
   %ansioCtrl.add( "16X", 16 );
   %ansioCtrl.add( "8X", 8 );
   %ansioCtrl.add( "4X", 4 );
   %ansioCtrl.add( "Off", 0 );
   %ansioCtrl.setSelected( $pref::Video::defaultAnisotropy, false );
            
   // set up the Refresh Rate menu.
   %refreshMenu = GraphicsMenuRefreshRate;
   %refreshMenu.clear();
   // %refreshMenu.add("Auto", 60);
   %refreshMenu.add("60", 60);
   %refreshMenu.add("75", 75);
   %refreshMenu.setSelected( $pref::Video::RefreshRate );
	   
   // Populate the Anti-aliasing popup.
   %aaMenu = GraphicsMenuAA;
   %aaMenu.clear();
   %aaMenu.Add( "4x", 4 );
   %aaMenu.Add( "2x", 2 );
   %aaMenu.Add( "1x", 1 );
   %aaMenu.Add( "Off", 0 );
   %aaMenu.setSelected( $pref::Video::AA );
   
   //Parallax
   GraphicsMenuParallax.setStateOn(!$pref::Video::disableParallaxMapping);
   
   //water reflections
   GraphicsMenuWaterRefl.setStateOn(!$pref::Water::disableTrueReflections);
   
   GraphicsMenuParallax.setStateOn(!$pref::Video::disableParallaxMapping);
      
   GraphicsMenuAO.setStateOn($pref::PostFX::EnableSSAO);
   GraphicsMenuHDR.setStateOn($pref::PostFX::EnableHDR);
   GraphicsMenuDOF.setStateOn($pref::PostFX::EnableDOF);
   GraphicsMenuVignette.setStateOn($pref::PostFX::EnableVignette);
   GraphicsMenuLightRay.setStateOn($pref::PostFX::EnableLightRays);
}

function GraphicsMenuOKButton::onClick(%this)
{
    //save the settings and then back out
    GraphicsMenu.apply();
    OptionsMenu.backOut();
}

function GraphicsMenu::initResMenu( %this )
{
   // Clear out previous values
   %resMenu = GraphicsMenuResolution;	   
   %resMenu.clear();
   
   // If we are in a browser then we can't change our resolution through
   // the options dialog
   if (getWebDeployment())
   {
      %count = 0;
      %currRes = getWords(Canvas.getVideoMode(), $WORD::RES_X, $WORD::RES_Y);
      %resMenu.add(%currRes, %count);
      %count++;

      return;
   }
   
   // Loop through all and add all valid resolutions
   %count = 0;
   %resCount = Canvas.getModeCount();
   for (%i = 0; %i < %resCount; %i++)
   {
      %testResString = Canvas.getMode( %i );
      %testRes = _makePrettyResString( %testResString );
                     
      // Only add to list if it isn't there already.
      if (%resMenu.findText(%testRes) == -1)
      {
         %resMenu.add(%testRes, %i);
         %count++;
      }
   }
   
   %resMenu.sort();
}

function GraphicsQualityPopup::init( %this, %qualityGroup )
{
   assert( isObject( %this ) );
   assert( isObject( %qualityGroup ) );
   
   %this.qualityGroup = %qualityGroup;
            
   // Clear the existing content first.   
   %this.clear();
    
   // Fill it.
   %select = -1;
   for ( %i=0; %i < %qualityGroup.getCount(); %i++ )
   {
      %level = %qualityGroup.getObject( %i );
      if ( %level.isCurrent() )
         %select = %i;
         
      %this.add( %level.displayName, %i );
   }
   
   // Setup a default selection.
   if ( %select == -1 )
      %this.setText( "Custom" );
   else
      %this.setSelected( %select );      
}

function GraphicsQualityPopup::apply( %this )
{
   %levelName = %this.getText();
   %this.qualityGroup.applySetting(%levelName);
}
//
function GraphicsMenu::Autodetect(%this)
{
   $pref::Video::autoDetect = false;
   
   %shaderVer = getPixelShaderVersion();
   %intel = ( strstr( strupr( getDisplayDeviceInformation() ), "INTEL" ) != -1 ) ? true : false;
   %videoMem = GFXCardProfilerAPI::getVideoMemoryMB();
   
   return %this.Autodetect_Apply( %shaderVer, %intel, %videoMem );
}

function GraphicsMenu::Autodetect_Apply(%this, %shaderVer, %intel, %videoMem )
{
   if ( %shaderVer < 2.0 )
   {      
      return "Your video card does not meet the minimum requirment of shader model 2.0.";
   }
   
   if ( %shaderVer < 3.0 || %intel )
   {
      // Allow specular and normals for 2.0a and 2.0b
      if ( %shaderVer > 2.0 )
      {
         MeshQualityGroup.applySetting("Lowest");
         TextureQualityGroup.applySetting("Lowest");
         GroundCoverDensityGroup.applySetting("Lowest");
         DecalLifetimeGroup.applySetting("None");
         TerrainQualityGroup.applySetting("Lowest");
         ShaderQualityGroup.applySetting("High");
         
         ShadowQualityList.applySetting("None");
         
         SoftShadowList.applySetting("Off");
         
         $pref::Shadows::useShadowCaching = true;
         
         $pref::Water::disableTrueReflections = true;
         $pref::Video::disableParallaxMapping = true;
         $pref::PostFX::EnableSSAO = false;
         $pref::PostFX::EnableHDR = false;
         $pref::PostFX::EnableDOF = false;
         $pref::PostFX::EnableLightRays = false;
         $pref::PostFX::EnableVignette = false;
         
         $pref::Video::AA = 0;
         $pref::Video::disableVerticalSync = 0;
      }
      else
      {
         MeshQualityGroup.applySetting("Lowest");
         TextureQualityGroup.applySetting("Lowest");
         GroundCoverDensityGroup.applySetting("Lowest");
         DecalLifetimeGroup.applySetting("None");
         TerrainQualityGroup.applySetting("Lowest");
         ShaderQualityGroup.applySetting("Low");
         
         ShadowQualityList.applySetting("None");
         
         SoftShadowList.applySetting("Off");
         
         $pref::Shadows::useShadowCaching = true;
         
         $pref::Water::disableTrueReflections = true;
         $pref::Video::disableParallaxMapping = true;
         $pref::PostFX::EnableSSAO = false;
         $pref::PostFX::EnableHDR = false;
         $pref::PostFX::EnableDOF = false;
         $pref::PostFX::EnableLightRays = false;
         $pref::PostFX::EnableVignette = false;
         
         $pref::Video::AA = 0;
         $pref::Video::disableVerticalSync = 0;
      }
   }   
   else
   {
      if ( %videoMem > 1000 )
      {
         MeshQualityGroup.applySetting("High");
         TextureQualityGroup.applySetting("High");
         GroundCoverDensityGroup.applySetting("High");
         DecalLifetimeGroup.applySetting("High");
         TerrainQualityGroup.applySetting("High");
         ShaderQualityGroup.applySetting("High");
         
         ShadowQualityList.applySetting("High");
         
         SoftShadowList.applySetting("High");
         
         //Should this default to on in ultra settings?
         $pref::Shadows::useShadowCaching = true;
         
         $pref::Water::disableTrueReflections = false;
         $pref::Video::disableParallaxMapping = false;
         $pref::PostFX::EnableSSAO = true;
         $pref::PostFX::EnableHDR = true;
         $pref::PostFX::EnableDOF = true;
         $pref::PostFX::EnableLightRays = true;
         $pref::PostFX::EnableVignette = true;
         
         $pref::Video::AA = 4;
         $pref::Video::disableVerticalSync = 16;
      }
      else if ( %videoMem > 400 || %videoMem == 0 )
      {
         MeshQualityGroup.applySetting("Medium");
         TextureQualityGroup.applySetting("Medium");
         GroundCoverDensityGroup.applySetting("Medium");
         DecalLifetimeGroup.applySetting("Medium");
         TerrainQualityGroup.applySetting("Medium");
         ShaderQualityGroup.applySetting("High");
         
         ShadowQualityList.applySetting("Medium");
         
         SoftShadowList.applySetting("Low");
         
         $pref::Shadows::useShadowCaching = true;
         
         $pref::Water::disableTrueReflections = false;
         $pref::Video::disableParallaxMapping = true;
         $pref::PostFX::EnableSSAO = false;
         $pref::PostFX::EnableHDR = true;
         $pref::PostFX::EnableDOF = true;
         $pref::PostFX::EnableLightRays = true;
         $pref::PostFX::EnableVignette = true;
         
         $pref::Video::AA = 4;
         $pref::Video::disableVerticalSync = 4;
         
         if ( %videoMem == 0 )
            return "Torque was unable to detect available video memory. Applying 'Medium' quality.";
      }
      else
      {
         MeshQualityGroup.applySetting("Low");
         TextureQualityGroup.applySetting("Low");
         GroundCoverDensityGroup.applySetting("Low");
         DecalLifetimeGroup.applySetting("Low");
         TerrainQualityGroup.applySetting("Low");
         ShaderQualityGroup.applySetting("Low");
         
         ShadowQualityList.applySetting("None");
         
         SoftShadowList.applySetting("Off");
         
         $pref::Shadows::useShadowCaching = true;
         
         $pref::Water::disableTrueReflections = false;
         $pref::Video::disableParallaxMapping = true;
         $pref::PostFX::EnableSSAO = false;
         $pref::PostFX::EnableHDR = false;
         $pref::PostFX::EnableDOF = false;
         $pref::PostFX::EnableLightRays = false;
         $pref::PostFX::EnableVignette = false;
         
         $pref::Video::AA = 0;
         $pref::Video::disableVerticalSync = 0;
      }
   }
   
   %this.refresh();
   
   %this.apply();
   
   //force postFX updates
   PostFXManager.settingsEffectSetEnabled("SSAO", $pref::PostFX::EnableSSAO);
   PostFXManager.settingsEffectSetEnabled("HDR", $pref::PostFX::EnableHDR);
   PostFXManager.settingsEffectSetEnabled("DOF", $pref::PostFX::EnableDOF);
   PostFXManager.settingsEffectSetEnabled("LightRays", $pref::PostFX::EnableLightRays);
   PostFXManager.settingsEffectSetEnabled("Vignette", $pref::PostFX::EnableVignette);
   
   return "Graphics quality settings have been auto detected.";
}

function _makePrettyResString( %resString )
{
   %width = getWord( %resString, $WORD::RES_X );
   %height = getWord( %resString, $WORD::RES_Y );
   
   %aspect = %width / %height;
   %aspect = mRound( %aspect * 100 ) * 0.01;            
   
   switch$( %aspect )
   {
      case "1.33":
         %aspect = "4:3";
      case "1.78":
         %aspect = "16:9";
      default:
         %aspect = "";
   }
   
   %outRes = %width @ " x " @ %height;
   if ( %aspect !$= "" )
      %outRes = %outRes @ "  (" @ %aspect @ ")";
      
   return %outRes;   
}

//
function GraphicsMenuSetting::init( %this )
{
   assert( isObject( %this ) );
   assert( isObject( %this.qualitySettingGroup ) );
    
   // Fill it.
   %select = -1;
   %selectedName = "";
   for ( %i=0; %i < %this.qualitySettingGroup.getCount(); %i++ )
   {
      %level = %this.qualitySettingGroup.getObject( %i );
      
      %levelName = %level.displayName;
      if ( %level.isCurrent() )
      {
         %select = %i;
         %selectedName = %level.displayName;
      }
   }
   
   // Setup a default selection.
   if ( %select == -1 )
   {
      %this-->SettingText.setText( "Custom" );
      %this.selectedLevel = %this.qualitySettingGroup.getCount();
   }
   else
   {
      %this-->SettingText.setText(%selectedName);
      %this.selectedLevel = %select;
   }
}

function GraphicsQualityLevel::isCurrent( %this )
{
   // Test each pref to see if the current value
   // equals our stored value.
   
   for ( %i=0; %i < %this.count(); %i++ )
   {
      %pref = %this.getKey( %i );
      %value = %this.getValue( %i );
      
      if ( getVariable( %pref ) !$= %value )
         return false;
   }
   
   return true;
}

function GraphicsQualityLevel::apply( %this )
{
   for ( %i=0; %i < %this.count(); %i++ )
   {
      %pref = %this.getKey( %i );
      %value = %this.getValue( %i );
      setVariable( %pref, %value );
   }
   
   // If we have an overloaded onApply method then
   // call it now to finalize the changes.
   if ( %this.isMethod( "onApply" ) )   
      %this.onApply();
   else
   {
      %group = %this.getGroup();      
      if ( isObject( %group ) && %group.isMethod( "onApply" ) )
         %group.onApply( %this );
   }   
}

function GraphicsOptionsMenuGroup::applySetting(%this, %settingName)
{
   for(%i=0; %i < %this.getCount(); %i++)
   {
      %setting = %this.getObject(%i);
      if(%setting.displayName $= %settingName)
      {
         for ( %s=0; %s < %setting.count(); %s++ )
         {
            %pref = %setting.getKey( %s );
            %value = %setting.getValue( %s );
            setVariable( %pref, %value );
         }
      }
   }
}

function GraphicsMenu::apply(%this)
{
   %newAdapter    = GraphicsMenuDriver.getText();
	%numAdapters   = GFXInit::getAdapterCount();
	%newDevice     = $pref::Video::displayDevice;
							
	for( %i = 0; %i < %numAdapters; %i ++ )
	{
	   if( GFXInit::getAdapterName( %i ) $= %newAdapter )
	   {
	      %newDevice = GFXInit::getAdapterType( %i );
	      break;
	   }
	}
	   
   // Change the device.
   if ( %newDevice !$= $pref::Video::displayDevice )
   {
      if ( %testNeedApply )
         return true;
         
      $pref::Video::displayDevice = %newDevice;
      if( %newAdapter !$= getDisplayDeviceInformation() )
         MessageBoxOK( "Change requires restart", "Please restart the game for a display device change to take effect." );
   }
   
   GraphicsMenuShadowQlty.apply();
   GraphicsMenuSoftShadow.apply();
   
   GraphicsMenuModelDtl.apply();
   GraphicsMenuTextureDtl.apply();
   GraphicsMenuTerrainDtl.apply();
   GraphicsMenuDecalLife.apply();
   GraphicsMenuGroundClutter.apply();
   
   GraphicsMenuMaterialQlty.apply();
   
   //Update Textures
   reloadTextures();

   //Update lighting
   // Set the light manager.  This should do nothing 
   // if its already set or if its not compatible.   
   setLightManager( $pref::lightManager );   
   
   PostFXManager.settingsEffectSetEnabled("SSAO", $pref::PostFX::EnableSSAO);
   PostFXManager.settingsEffectSetEnabled("HDR", $pref::PostFX::EnableHDR);
   PostFXManager.settingsEffectSetEnabled("DOF", $pref::PostFX::EnableDOF);
   PostFXManager.settingsEffectSetEnabled("LightRays", $pref::PostFX::EnableLightRays);
   PostFXManager.settingsEffectSetEnabled("Vignette", $pref::PostFX::EnableVignette);
   
   $pref::Video::disableParallaxMapping = !GraphicsMenuParallax.isStateOn();
   
   //water reflections
   $pref::Water::disableTrueReflections = !GraphicsMenuWaterRefl.isStateOn();
   
   //Update the display settings now
   $pref::Video::Resolution = getWords( Canvas.getMode( GraphicsMenuResolution.getSelected() ), $WORD::RES_X, $WORD::RES_Y );
   %newBpp        = 32; // ... its not 1997 anymore.
	$pref::Video::FullScreen = GraphicsMenuFullScreen.isStateOn() ? "true" : "false";
	$pref::Video::RefreshRate    = GraphicsMenuRefreshRate.getSelected();
	$pref::Video::disableVerticalSync = !GraphicsMenuVSync.isStateOn();	
	$pref::Video::AA = GraphicsMenuAA.getSelected();
	
   if ( %newFullScreen $= "false" )
	{
      // If we're in windowed mode switch the fullscreen check
      // if the resolution is bigger than the desktop.
      %deskRes    = getDesktopResolution();      
      %deskResX   = getWord(%deskRes, $WORD::RES_X);
      %deskResY   = getWord(%deskRes, $WORD::RES_Y);
	   if (  getWord( %newRes, $WORD::RES_X ) > %deskResX || 
	         getWord( %newRes, $WORD::RES_Y ) > %deskResY )
      {
         $pref::Video::FullScreen = "true";
         GraphicsMenuFullScreen.setStateOn( true );
      }
	}

   // Build the final mode string.
	%newMode = $pref::Video::Resolution SPC $pref::Video::FullScreen SPC %newBpp SPC $pref::Video::RefreshRate SPC $pref::Video::AA;
	
   // Change the video mode.   
   if (  %newMode !$= $pref::Video::mode || 
         %newVsync != $pref::Video::disableVerticalSync )
   {
      if ( %testNeedApply )
         return true;

      $pref::Video::mode = %newMode;
      $pref::Video::disableVerticalSync = %newVsync;      
      configureCanvas();
   }
   
   // Check the anisotropic filtering.   
   %level = GraphicsMenuAniso.getSelected();
   if ( %level != $pref::Video::defaultAnisotropy )
   {
      if ( %testNeedApply )
         return true;
                                 
      $pref::Video::defaultAnisotropy = %level;
   }
   
   echo("Exporting client prefs");
   %prefPath = getPrefpath();
   export("$pref::*", %prefPath @ "/clientPrefs.cs", false);
}