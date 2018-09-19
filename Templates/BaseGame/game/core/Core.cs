
function CoreModule::onCreate(%this)
{
   
   // ----------------------------------------------------------------------------
   // Initialize core sub system functionality such as audio, the Canvas, PostFX,
   // rendermanager, light managers, etc.
   //
   // Note that not all of these need to be initialized before the client, although
   // the audio should and the canvas definitely needs to be.  I've put things here
   // to distinguish between the purpose and functionality of the various client
   // scripts.  Game specific script isn't needed until we reach the shell menus
   // and start a game or connect to a server. We get the various subsystems ready
   // to go, and then use initClient() to handle the rest of the startup sequence.
   //
   // If this is too convoluted we can reduce this complexity after futher testing
   // to find exactly which subsystems should be readied before kicking things off. 
   // ----------------------------------------------------------------------------
   
   ModuleDatabase.LoadExplicit( "Core_Rendering" );
   ModuleDatabase.LoadExplicit( "Core_Utility" );
   ModuleDatabase.LoadExplicit( "Core_GUI" );
   ModuleDatabase.LoadExplicit( "CoreModule" );
   ModuleDatabase.LoadExplicit( "Core_Lighting" );
   ModuleDatabase.LoadExplicit( "Core_SFX" );
   ModuleDatabase.LoadExplicit( "Core_PostFX" );
   ModuleDatabase.LoadExplicit( "Core_ClientServer" );
   
   %prefPath = getPrefpath();
   if ( isFile( %prefPath @ "/clientPrefs.cs" ) )
      exec( %prefPath @ "/clientPrefs.cs" );
   else
      exec("data/defaults.cs");
      
   %der = $pref::Video::displayDevice;
   
   //We need to hook the missing/warn material stuff early, so do it here
   /*$Core::MissingTexturePath = "core/images/missingTexture";
   $Core::UnAvailableTexturePath = "core/images/unavailable";
   $Core::WarningTexturePath = "core/images/warnMat";
   $Core::CommonShaderPath = "core/shaders";
   
   /*%classList = enumerateConsoleClasses( "Component" );

   foreach$( %componentClass in %classList )
   {
      echo("Native Component of type: " @ %componentClass);
   }*/

   //exec("./helperFunctions.cs");

   // We need some of the default GUI profiles in order to get the canvas and
   // other aspects of the GUI system ready.
   //exec("./profiles.cs");

   //This is a bit of a shortcut, but we'll load the client's default settings to ensure all the prefs get initialized correctly
   

   // Initialization of the various subsystems requires some of the preferences
   // to be loaded... so do that first.
   /*exec("./globals.cs");

   exec("./canvas.cs");
   exec("./cursor.cs");

   exec("./renderManager.cs");
   exec("./lighting.cs");

   exec("./audio.cs");
   exec("./sfx/audioAmbience.cs");
   exec("./sfx/audioData.cs");
   exec("./sfx/audioDescriptions.cs");
   exec("./sfx/audioEnvironments.cs");
   exec("./sfx/audioStates.cs");

   exec("./parseArgs.cs");

   // Materials and Shaders for rendering various object types
   exec("./gfxData/commonMaterialData.cs");
   exec("./gfxData/shaders.cs");
   exec("./gfxData/terrainBlock.cs");
   exec("./gfxData/water.cs");
   exec("./gfxData/scatterSky.cs");
   exec("./gfxData/clouds.cs");

   // Initialize all core post effects.   
   exec("./postFx.cs");

   //VR stuff
   exec("./oculusVR.cs");*/

   // Seed the random number generator.
   setRandomSeed();
   
   // Parse the command line arguments
   echo("\n--------- Parsing Arguments ---------");
   parseArgs();
   
   // The canvas needs to be initialized before any gui scripts are run since
   // some of the controls assume that the canvas exists at load time.
   createCanvas($appName);

   //load canvas
   //exec("./console/main.cs");

   ModuleDatabase.LoadExplicit( "Core_Console" );
   
   // Init the physics plugin.
   physicsInit();

   sfxStartup();

   // Set up networking.
   setNetPort(0);

   // Start processing file change events.   
   startFileChangeNotifications();
   
   // If we have editors, initialize them here as well
   if (isToolBuild())
   {
      if(isFile("tools/main.cs") && !$isDedicated)
         exec("tools/main.cs");
         
      ModuleDatabase.scanModules( "tools", false );
      ModuleDatabase.LoadGroup( "Tools" );
   }
}

function CoreModule::onDestroy(%this)
{

}

//-----------------------------------------------------------------------------
// Called when the engine is shutting down.
function onExit() 
{
   // Stop file change events.
   stopFileChangeNotifications();
   
   ModuleDatabase.UnloadExplicit( "Game" );
}