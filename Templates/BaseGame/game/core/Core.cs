
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
   ModuleDatabase.LoadExplicit( "Core_Lighting" );
   ModuleDatabase.LoadExplicit( "Core_SFX" );
   ModuleDatabase.LoadExplicit( "Core_PostFX" );
   ModuleDatabase.LoadExplicit( "Core_Components" );
   ModuleDatabase.LoadExplicit( "Core_GameObjects" );
   ModuleDatabase.LoadExplicit( "Core_ClientServer" );
   
   %prefPath = getPrefpath();
   if ( isFile( %prefPath @ "/clientPrefs.cs" ) )
      exec( %prefPath @ "/clientPrefs.cs" );
   else
      exec("data/defaults.cs");
      
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