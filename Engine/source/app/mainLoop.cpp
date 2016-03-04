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

#include "app/mainLoop.h"
#include "app/game.h"

#include "platform/platformTimer.h"
#include "platform/platformRedBook.h"
#include "platform/platformVolume.h"
#include "platform/platformMemory.h"
#include "platform/platformTimer.h"
#include "platform/platformNet.h"
#include "platform/nativeDialogs/fileDialog.h"
#include "platform/threads/thread.h"

#include "core/module.h"
#include "core/threadStatic.h"
#include "core/iTickable.h"
#include "core/stream/fileStream.h"

#include "windowManager/platformWindowMgr.h"

#include "core/util/journal/process.h"
#include "util/fpsTracker.h"

#include "console/debugOutputConsumer.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"

#include "gfx/bitmap/gBitmap.h"
#include "gfx/gFont.h"
#include "gfx/video/videoCapture.h"
#include "gfx/gfxTextureManager.h"

#include "sim/netStringTable.h"
#include "sim/actionMap.h"
#include "sim/netInterface.h"

#include "util/sampler.h"
#include "platform/threads/threadPool.h"

// For the TickMs define... fix this for T2D...
#include "T3D/gameBase/processList.h"

#ifdef TORQUE_ENABLE_VFS
#include "platform/platformVFS.h"
#endif

#ifndef _MODULE_MANAGER_H
#include "module/moduleManager.h"
#endif

#ifndef _ASSET_MANAGER_H_
#include "assets/assetManager.h"
#endif

DITTS( F32, gTimeScale, 1.0 );
DITTS( U32, gTimeAdvance, 0 );
DITTS( U32, gFrameSkip, 0 );

extern S32 sgBackgroundProcessSleepTime;
extern S32 sgTimeManagerProcessInterval;

extern FPSTracker gFPS;

TimeManager* tm = NULL;

static bool gRequiresRestart = false;

#ifdef TORQUE_DEBUG

/// Temporary timer used to time startup times.
static PlatformTimer* gStartupTimer;

#endif

#if defined( TORQUE_MINIDUMP ) && defined( TORQUE_RELEASE )
StringTableEntry gMiniDumpDir;
StringTableEntry gMiniDumpExec;
StringTableEntry gMiniDumpParams;
StringTableEntry gMiniDumpExecDir;
#endif


namespace engineAPI
{
   // This is the magic switch for deciding which interop the engine
   // should use.  It will go away when we drop the console system
   // entirely but for now it is necessary for several behaviors that
   // differ between the interops to decide what to do.
   bool gUseConsoleInterop = true;
   
   bool gIsInitialized = false;
}



// The following are some tricks to make the memory leak checker run after global
// dtors have executed by placing some code in the termination segments.

#if defined( TORQUE_DEBUG ) && !defined( TORQUE_DISABLE_MEMORY_MANAGER )

   #ifdef TORQUE_COMPILER_VISUALC
   #  pragma data_seg( ".CRT$XTU" )
   
      static void* sCheckMemBeforeTermination = &Memory::ensureAllFreed;
      
   #  pragma data_seg()
   #elif defined( TORQUE_COMPILER_GCC )
   
       __attribute__ ( ( destructor ) ) static void _ensureAllFreed()
      {
         Memory::ensureAllFreed();
      }
      
   #endif

#endif

// Process a time event and update all sub-processes
void processTimeEvent(S32 elapsedTime)
{
   PROFILE_START(ProcessTimeEvent);

   // If recording a video and not playinb back a journal, override the elapsedTime
   if (VIDCAP->isRecording() && !Journal::IsPlaying())
      elapsedTime = VIDCAP->getMsPerFrame();   
   
   // cap the elapsed time to one second
   // if it's more than that we're probably in a bad catch-up situation
   if(elapsedTime > 1024)
      elapsedTime = 1024;
   
   U32 timeDelta;
   if(ATTS(gTimeAdvance))
      timeDelta = ATTS(gTimeAdvance);
   else
      timeDelta = (U32) (elapsedTime * ATTS(gTimeScale));
   
   Platform::advanceTime(elapsedTime);
   
   // Don't build up more time than a single tick... this makes the sim
   // frame rate dependent but is a useful hack for singleplayer.
   if ( ATTS(gFrameSkip) )
      if ( timeDelta > TickMs )
         timeDelta = TickMs;

   bool tickPass;
   
   PROFILE_START(ServerProcess);
   tickPass = serverProcess(timeDelta);
   PROFILE_END();
   
   PROFILE_START(ServerNetProcess);
   // only send packets if a tick happened
   if(tickPass)
      GNet->processServer();
   // Used to indicate if server was just ticked.
   Con::setBoolVariable( "$pref::hasServerTicked", tickPass );
   PROFILE_END();

   
   PROFILE_START(SimAdvanceTime);
   Sim::advanceTime(timeDelta);
   PROFILE_END();
   
   PROFILE_START(ClientProcess);
   tickPass = clientProcess(timeDelta);
   // Used to indicate if client was just ticked.
   Con::setBoolVariable( "$pref::hasClientTicked", tickPass );
   PROFILE_END_NAMED(ClientProcess);
   
   PROFILE_START(ClientNetProcess);
   if(tickPass)
      GNet->processClient();
   PROFILE_END();
   
   GNet->checkTimeouts();
   
   gFPS.update();

   // Give the texture manager a chance to cleanup any
   // textures that haven't been referenced for a bit.
   if( GFX )
      TEXMGR->cleanupCache( 5 );

   PROFILE_END();
   
   // Update the console time
   Con::setFloatVariable("Sim::Time",F32(Platform::getVirtualMilliseconds()) / 1000);
}

void StandardMainLoop::init()
{
   #ifdef TORQUE_DEBUG
   gStartupTimer = PlatformTimer::create();
   #endif
   
   #ifdef TORQUE_DEBUG_GUARD
      Memory::flagCurrentAllocs( Memory::FLAG_Global );
   #endif

   Platform::setMathControlStateKnown();
   
   // Asserts should be created FIRST
   PlatformAssert::create();
   
   ManagedSingleton< ThreadManager >::createSingleton();
   FrameAllocator::init(TORQUE_FRAME_SIZE);      // See comments in torqueConfig.h

   // Yell if we can't initialize the network.
   if(!Net::init())
   {
      AssertISV(false, "StandardMainLoop::initCore - could not initialize networking!");
   }

   _StringTable::create();

   // Set up the resource manager and get some basic file types in it.
   Con::init();
   Platform::initConsole();
   NetStringTable::create();

   // Use debug output logging on the Xbox and OSX builds
#if defined( _XBOX ) || defined( TORQUE_OS_MAC )
   DebugOutputConsumer::init();
#endif

   // init Filesystem first, so we can actually log errors for all components that follow
   Platform::FS::InstallFileSystems(); // install all drives for now until we have everything using the volume stuff
   Platform::FS::MountDefaults();

   // Set our working directory.
   Torque::FS::SetCwd( "game:/" );

   // Set our working directory.
   Platform::setCurrentDirectory( Platform::getMainDotCsDir() );

   Processor::init();
   Math::init();
   Platform::init();    // platform specific initialization
   RedBook::init();
   Platform::initConsole();
   
   ThreadPool::GlobalThreadPool::createSingleton();

   // Initialize modules.
   
   EngineModuleManager::initializeSystem();
         
   // Initialise ITickable.
#ifdef TORQUE_TGB_ONLY
   ITickable::init( 4 );
#endif

#ifdef TORQUE_ENABLE_VFS
   // [tom, 10/28/2006] Load the VFS here so that it stays loaded
   Zip::ZipArchive *vfs = openEmbeddedVFSArchive();
   gResourceManager->addVFSRoot(vfs);
#endif

   Con::addVariable("timeScale", TypeF32, &ATTS(gTimeScale), "Animation time scale.\n"
	   "@ingroup platform");
   Con::addVariable("timeAdvance", TypeS32, &ATTS(gTimeAdvance), "The speed at which system processing time advances.\n"
	   "@ingroup platform");
   Con::addVariable("frameSkip", TypeS32, &ATTS(gFrameSkip), "Sets the number of frames to skip while rendering the scene.\n"
	   "@ingroup platform");

   Con::setVariable( "defaultGame", StringTable->insert("scripts") );

   Con::addVariable( "_forceAllMainThread", TypeBool, &ThreadPool::getForceAllMainThread(), "Force all work items to execute on main thread. turns this into a single-threaded system. Primarily useful to find whether malfunctions are caused by parallel execution or not.\n"
	   "@ingroup platform" );

#if defined( TORQUE_MINIDUMP ) && defined( TORQUE_RELEASE )
	Con::addVariable("MiniDump::Dir",	TypeString, &gMiniDumpDir);
	Con::addVariable("MiniDump::Exec",	TypeString, &gMiniDumpExec);
	Con::addVariable("MiniDump::Params", TypeString, &gMiniDumpParams);
	Con::addVariable("MiniDump::ExecDir", TypeString, &gMiniDumpExecDir);
#endif

   // Register the module manager.
   ModuleDatabase.registerObject("ModuleDatabase");

   // Register the asset database.
   AssetDatabase.registerObject("AssetDatabase");

   // Register the asset database as a module listener.
   ModuleDatabase.addListener(&AssetDatabase);
   
   ActionMap* globalMap = new ActionMap;
   globalMap->registerObject("GlobalActionMap");
   Sim::getActiveActionMapSet()->pushObject(globalMap);
   
   // Do this before we init the process so that process notifiees can get the time manager
   tm = new TimeManager;
   tm->timeEvent.notify(&::processTimeEvent);
   
   // Start up the Input Event Manager
   INPUTMGR->start();

   Sampler::init();

   // Hook in for UDP notification
   Net::smPacketReceive.notify(GNet, &NetInterface::processPacketReceiveEvent);

   #ifdef TORQUE_DEBUG_GUARD
      Memory::flagCurrentAllocs( Memory::FLAG_Static );
   #endif
}

void StandardMainLoop::shutdown()
{
   // Stop the Input Event Manager
   INPUTMGR->stop();

   delete tm;
   preShutdown();

   // Unregister the module database.
   ModuleDatabase.unregisterObject();

   // Unregister the asset database.
   AssetDatabase.unregisterObject();
   
   // Shut down modules.
   
   EngineModuleManager::shutdownSystem();
   
   ThreadPool::GlobalThreadPool::deleteSingleton();

#ifdef TORQUE_ENABLE_VFS
   closeEmbeddedVFSArchive();
#endif

   RedBook::destroy();

   Platform::shutdown();
   
#if defined( _XBOX ) || defined( TORQUE_OS_MAC )
   DebugOutputConsumer::destroy();
#endif

   NetStringTable::destroy();
   Con::shutdown();

   _StringTable::destroy();
   FrameAllocator::destroy();
   Net::shutdown();
   Sampler::destroy();
   
   ManagedSingleton< ThreadManager >::deleteSingleton();

   // asserts should be destroyed LAST
   PlatformAssert::destroy();

#if defined( TORQUE_DEBUG ) && !defined( TORQUE_DISABLE_MEMORY_MANAGER )
   Memory::validate();
#endif
}

void StandardMainLoop::preShutdown()
{
#ifdef TORQUE_TOOLS
   // Tools are given a chance to do pre-quit processing
   // - This is because for tools we like to do things such
   //   as prompting to save changes before shutting down
   //   and onExit is packaged which means we can't be sure
   //   where in the shutdown namespace chain we are when using
   //   onExit since some components of the tools may already be
   //   destroyed that may be vital to saving changes to avoid
   //   loss of work [1/5/2007 justind]
   if( Con::isFunction("onPreExit") )
      Con::executef( "onPreExit");
#endif

   //exec the script onExit() function
   if ( Con::isFunction( "onExit" ) )
      Con::executef("onExit");
}

bool StandardMainLoop::handleCommandLine( S32 argc, const char **argv )
{
   // Allow the window manager to process command line inputs; this is
   // done to let web plugin functionality happen in a fairly transparent way.
   PlatformWindowManager::get()->processCmdLineArgs(argc, argv);

   Process::handleCommandLine( argc, argv );

   // Set up the command line args for the console scripts...
   Con::setIntVariable("Game::argc", argc);
   U32 i;
   for (i = 0; i < argc; i++)
      Con::setVariable(avar("Game::argv%d", i), argv[i]);

#ifdef TORQUE_PLAYER
   if(argc > 2 && dStricmp(argv[1], "-project") == 0)
   {
      char playerPath[1024];
      Platform::makeFullPathName(argv[2], playerPath, sizeof(playerPath));
      Platform::setCurrentDirectory(playerPath);

      argv += 2;
      argc -= 2;

      // Re-locate the game:/ asset mount.

      Torque::FS::Unmount( "game" );
      Torque::FS::Mount( "game", Platform::FS::createNativeFS( playerPath ) );
   }
#endif

   // Executes an entry script file. This is "main.cs"
   // by default, but any file name (with no whitespace
   // in it) may be run if it is specified as the first
   // command-line parameter. The script used, default
   // or otherwise, is not compiled and is loaded here
   // directly because the resource system restricts
   // access to the "root" directory.

#ifdef TORQUE_ENABLE_VFS
   Zip::ZipArchive *vfs = openEmbeddedVFSArchive();
   bool useVFS = vfs != NULL;
#endif

   Stream *mainCsStream = NULL;

   // The working filestream.
   FileStream str; 

   const char *defaultScriptName = "main.cs";
   bool useDefaultScript = true;

   // Check if any command-line parameters were passed (the first is just the app name).
   if (argc > 1)
   {
      // If so, check if the first parameter is a file to open.
      if ( (dStrcmp(argv[1], "") != 0 ) && (str.open(argv[1], Torque::FS::File::Read)) )
      {
         // If it opens, we assume it is the script to run.
         useDefaultScript = false;
#ifdef TORQUE_ENABLE_VFS
         useVFS = false;
#endif
         mainCsStream = &str;
      }
   }

   if (useDefaultScript)
   {
      bool success = false;

#ifdef TORQUE_ENABLE_VFS
      if(useVFS)
         success = (mainCsStream = vfs->openFile(defaultScriptName, Zip::ZipArchive::Read)) != NULL;
      else
#endif
         success = str.open(defaultScriptName, Torque::FS::File::Read);

#if defined( TORQUE_DEBUG ) && defined (TORQUE_TOOLS) && !defined(TORQUE_DEDICATED) && !defined( _XBOX )
      if (!success)
      {
         OpenFileDialog ofd;
         FileDialogData &fdd = ofd.getData();
         fdd.mFilters = StringTable->insert("Main Entry Script (main.cs)|main.cs|");
         fdd.mTitle   = StringTable->insert("Locate Game Entry Script");

         // Get the user's selection
         if( !ofd.Execute() )
            return false;

         // Process and update CWD so we can run the selected main.cs
         S32 pathLen = dStrlen( fdd.mFile );
         FrameTemp<char> szPathCopy( pathLen + 1);

         dStrcpy( szPathCopy, fdd.mFile );
         //forwardslash( szPathCopy );

         const char *path = dStrrchr(szPathCopy, '/');
         if(path)
         {
            U32 len = path - (const char*)szPathCopy;
            szPathCopy[len+1] = 0;

            Platform::setCurrentDirectory(szPathCopy);

            // Re-locate the game:/ asset mount.

            Torque::FS::Unmount( "game" );
            Torque::FS::Mount( "game", Platform::FS::createNativeFS( ( const char* ) szPathCopy ) );

            success = str.open(fdd.mFile, Torque::FS::File::Read);
            if(success)
               defaultScriptName = fdd.mFile;
         }
      }
#endif
      if( !success )
      {
         char msg[1024];
         dSprintf(msg, sizeof(msg), "Failed to open \"%s\".", defaultScriptName);
         Platform::AlertOK("Error", msg);
#ifdef TORQUE_ENABLE_VFS
         closeEmbeddedVFSArchive();
#endif

         return false;
      }

#ifdef TORQUE_ENABLE_VFS
      if(! useVFS)
#endif
         mainCsStream = &str;
   }

   // This should rarely happen, but lets deal with
   // it gracefully if it does.
   if ( mainCsStream == NULL )
      return false;

   U32 size = mainCsStream->getStreamSize();
   char *script = new char[size + 1];
   mainCsStream->read(size, script);

#ifdef TORQUE_ENABLE_VFS
   if(useVFS)
      vfs->closeFile(mainCsStream);
   else
#endif
      str.close();

   script[size] = 0;

   char buffer[1024], *ptr;
   Platform::makeFullPathName(useDefaultScript ? defaultScriptName : argv[1], buffer, sizeof(buffer), Platform::getCurrentDirectory());
   ptr = dStrrchr(buffer, '/');
   if(ptr != NULL)
      *ptr = 0;
   Platform::setMainDotCsDir(buffer);
   Platform::setCurrentDirectory(buffer);

   Con::evaluate(script, false, useDefaultScript ? defaultScriptName : argv[1]); 
   delete[] script;

#ifdef TORQUE_ENABLE_VFS
   closeEmbeddedVFSArchive();
#endif

   return true;
}

bool StandardMainLoop::doMainLoop()
{
   #ifdef TORQUE_DEBUG
   if( gStartupTimer )
   {
      Con::printf( "Started up in %.2f seconds...",
         F32( gStartupTimer->getElapsedMs() ) / 1000.f );
      SAFE_DELETE( gStartupTimer );
   }
   #endif
   
   bool keepRunning = true;
//   while(keepRunning)
   {
      tm->setBackgroundThreshold(mClamp(sgBackgroundProcessSleepTime, 1, 200));
      tm->setForegroundThreshold(mClamp(sgTimeManagerProcessInterval, 1, 200));
      // update foreground/background status
      if(WindowManager->getFirstWindow())
      {
         static bool lastFocus = false;
         bool newFocus = ( WindowManager->getFocusedWindow() != NULL );
         if(lastFocus != newFocus)
         {
#ifndef TORQUE_SHIPPING
            Con::printf("Window focus status changed: focus: %d", newFocus);
            if (!newFocus)
               Con::printf("  Using background sleep time: %u", Platform::getBackgroundSleepTime());
#endif

#ifdef TORQUE_OS_MAC
            if (newFocus)
               WindowManager->getFirstWindow()->show();
               
#endif
            lastFocus = newFocus;
         }
         
#ifndef TORQUE_OS_MAC         
         // under the web plugin do not sleep the process when the child window loses focus as this will cripple the browser perfomance
         if (!Platform::getWebDeployment())
            tm->setBackground(!newFocus);
         else
            tm->setBackground(false);
#else
         tm->setBackground(false);
#endif
      }
      else
      {
         tm->setBackground(false);
      }
      
      PROFILE_START(MainLoop);
      Sampler::beginFrame();

      if(!Process::processEvents())
         keepRunning = false;

      ThreadPool::processMainThreadWorkItems();
      Sampler::endFrame();
      PROFILE_END_NAMED(MainLoop);
   }
   
   return keepRunning;
}

S32 StandardMainLoop::getReturnStatus()
{
   return Process::getReturnStatus();
}

void StandardMainLoop::setRestart(bool restart )
{
   gRequiresRestart = restart;
}

bool StandardMainLoop::requiresRestart()
{
   return gRequiresRestart;
}
