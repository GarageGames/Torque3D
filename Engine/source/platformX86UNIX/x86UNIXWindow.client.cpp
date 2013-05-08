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



#include "console/console.h"
#include "core/fileStream.h"
#include "game/resource.h"
#include "game/version.h"
#include "math/mRandom.h"
#include "platformX86UNIX/platformX86UNIX.h"
#include "platformX86UNIX/x86UNIXStdConsole.h"
#include "platform/event.h"
#include "platform/gameInterface.h"
#include "platform/platform.h"
#include "platform/platformAL.h"
#include "platform/platformInput.h"
#include "platform/platformVideo.h"
#include "platform/profiler.h"
#include "platformX86UNIX/platformGL.h"
#include "platformX86UNIX/x86UNIXOGLVideo.h"
#include "platformX86UNIX/x86UNIXState.h"

#ifndef TORQUE_DEDICATED
#include "platformX86UNIX/x86UNIXMessageBox.h"
#include "platformX86UNIX/x86UNIXInputManager.h"
#endif

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h> // fork, execvp, chdir
#include <time.h> // nanosleep

#ifndef TORQUE_DEDICATED
#include <X11/Xlib.h>
#include <X11/Xos.h>

#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>
#include <SDL/SDL_version.h>
#endif

x86UNIXPlatformState *x86UNIXState;

bool DisplayPtrManager::sgDisplayLocked = false;
LockFunc_t DisplayPtrManager::sgLockFunc = NULL;
LockFunc_t DisplayPtrManager::sgUnlockFunc = NULL;

static U32 lastTimeTick;
static MRandomLCG sgPlatRandom;

#ifndef TORQUE_DEDICATED
extern void InstallRedBookDevices();
extern void PollRedbookDevices();
extern bool InitOpenGL();
// This is called when some X client sends 
// a selection event (e.g. SelectionRequest)
// to the window
extern void NotifySelectionEvent(XEvent& event);
#endif

//------------------------------------------------------------------------------
static S32 ParseCommandLine(S32 argc, const char **argv, 
   Vector<char*>& newCommandLine)
{
   x86UNIXState->setExePathName(argv[0]);
   bool foundDedicated = false;

   for ( int i=0; i < argc; i++ )
   {
      // look for platform specific args
      if (dStrcmp(argv[i], "-version") == 0)
      {
         dPrintf("%s (built on %s)\n", getVersionString(), getCompileTimeString());
         dPrintf("gcc: %s\n", __VERSION__);
         return 1;
      }
      if (dStrcmp(argv[i], "-cdaudio") == 0)
      {
         x86UNIXState->setCDAudioEnabled(true);
         continue;
      }
      if (dStrcmp(argv[i], "-dedicated") == 0)
      {
         foundDedicated = true;
         // no continue because dedicated is also handled by script
      }
      if (dStrcmp(argv[i], "-dsleep") == 0)
      {
         x86UNIXState->setDSleep(true);
         continue;
      }
      if (dStrcmp(argv[i], "-nohomedir") == 0)
      {
         x86UNIXState->setUseRedirect(false);
         continue;
      }
      if (dStrcmp(argv[i], "-chdir") == 0)
      {
         if ( ++i >= argc )
         {
            dPrintf("Follow -chdir option with the desired working directory.\n");
            return 1;
         }
         if (chdir(argv[i]) == -1)
         {
            dPrintf("Unable to chdir to %s: %s\n", argv[i], strerror(errno));
            return 1;
         }
         continue;
      }
      
      // copy the arg into newCommandLine
      int argLen = dStrlen(argv[i]) + 1;
      char* argBuf = new char[argLen]; // this memory is deleted in main()
      dStrncpy(argBuf, argv[i], argLen);
      newCommandLine.push_back(argBuf);
   }
   x86UNIXState->setDedicated(foundDedicated);
#if defined(DEDICATED) && !defined(TORQUE_ENGINE)
   if (!foundDedicated)
   {
      dPrintf("This is a dedicated server build.  You must supply the -dedicated command line parameter.\n");
      return 1;
   }
#endif
   return 0;
}

static void DetectWindowingSystem()
{
#ifndef TORQUE_DEDICATED
   Display* dpy = XOpenDisplay(NULL);
   if (dpy != NULL)
   {
      x86UNIXState->setXWindowsRunning(true);
      XCloseDisplay(dpy);
   }
#endif
}

//------------------------------------------------------------------------------
static void InitWindow(const Point2I &initialSize, const char *name)
{
   x86UNIXState->setWindowSize(initialSize);
   x86UNIXState->setWindowName(name);
}

#ifndef TORQUE_DEDICATED
//------------------------------------------------------------------------------
static bool InitSDL()
{
   if (SDL_Init(SDL_INIT_VIDEO) != 0)
      return false;

   atexit(SDL_Quit);

   SDL_SysWMinfo sysinfo;
   SDL_VERSION(&sysinfo.version);
   if (SDL_GetWMInfo(&sysinfo) == 0)
      return false;

   x86UNIXState->setDisplayPointer(sysinfo.info.x11.display);
   DisplayPtrManager::setDisplayLockFunction(sysinfo.info.x11.lock_func);
   DisplayPtrManager::setDisplayUnlockFunction(sysinfo.info.x11.unlock_func);

   DisplayPtrManager xdisplay;
   Display* display = xdisplay.getDisplayPointer();

   x86UNIXState->setScreenNumber( 
      DefaultScreen( display ) );
   x86UNIXState->setScreenPointer( 
      DefaultScreenOfDisplay( display ) );

   x86UNIXState->setDesktopSize( 
      (S32) DisplayWidth( 
         display,
         x86UNIXState->getScreenNumber()),
      (S32) DisplayHeight( 
         display,
         x86UNIXState->getScreenNumber())
      );
   x86UNIXState->setDesktopBpp( 
      (S32) DefaultDepth( 
         display,
         x86UNIXState->getScreenNumber()));

   // indicate that we want sys WM messages
   SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

   return true;
}

//------------------------------------------------------------------------------
static void ProcessSYSWMEvent(const SDL_Event& event)
{
   XEvent& xevent = event.syswm.msg->event.xevent;
   //Con::printf("xevent : %d", xevent.type);
   switch (xevent.type)
   {
      case SelectionRequest:
         // somebody wants our clipboard
         NotifySelectionEvent(xevent);
         break;
   }
}

//------------------------------------------------------------------------------
static void SetAppState()
{
   U8 state = SDL_GetAppState();

   // if we're not active but we have appactive and inputfocus, set window
   // active and reactivate input
   if ((!x86UNIXState->windowActive() || !Input::isActive()) &&
      state & SDL_APPACTIVE &&
      state & SDL_APPINPUTFOCUS)
   {
      x86UNIXState->setWindowActive(true);
      Input::reactivate();
   }
   // if we are active, but we don't have appactive or input focus,
   // deactivate input (if window not locked) and clear windowActive
   else if (x86UNIXState->windowActive() && 
      !(state & SDL_APPACTIVE && state & SDL_APPINPUTFOCUS))
   {
      if (x86UNIXState->windowLocked())
         Input::deactivate();
      x86UNIXState->setWindowActive(false);
   }
}

//------------------------------------------------------------------------------
static S32 NumEventsPending()
{
   static const int MaxEvents = 255;
   static SDL_Event events[MaxEvents];

   SDL_PumpEvents();
   return SDL_PeepEvents(events, MaxEvents, SDL_PEEKEVENT, SDL_ALLEVENTS);
}

//------------------------------------------------------------------------------
static void PrintSDLEventQueue()
{
   static const int MaxEvents = 255;
   static SDL_Event events[MaxEvents];

   SDL_PumpEvents();
   S32 numEvents = SDL_PeepEvents(
      events, MaxEvents, SDL_PEEKEVENT, SDL_ALLEVENTS);
   if (numEvents <= 0)
   {
      dPrintf("SDL Event Queue is empty\n");
      return;
   }

   dPrintf("SDL Event Queue:\n");
   for (int i = 0; i < numEvents; ++i)
   {
      const char *eventType;
      switch (events[i].type)
      {
         case SDL_NOEVENT: eventType = "SDL_NOEVENT"; break;
         case SDL_ACTIVEEVENT: eventType = "SDL_ACTIVEEVENT"; break;
         case SDL_KEYDOWN: eventType = "SDL_KEYDOWN"; break; 
         case SDL_KEYUP: eventType = "SDL_KEYUP"; break; 
         case SDL_MOUSEMOTION: eventType = "SDL_MOUSEMOTION"; break; 
         case SDL_MOUSEBUTTONDOWN: eventType = "SDL_MOUSEBUTTONDOWN"; break; 
         case SDL_MOUSEBUTTONUP: eventType = "SDL_MOUSEBUTTONUP"; break; 
         case SDL_JOYAXISMOTION: eventType = "SDL_JOYAXISMOTION"; break; 
         case SDL_JOYBALLMOTION: eventType = "SDL_JOYBALLMOTION"; break; 
         case SDL_JOYHATMOTION: eventType = "SDL_JOYHATMOTION"; break; 
         case SDL_JOYBUTTONDOWN: eventType = "SDL_JOYBUTTONDOWN"; break; 
         case SDL_JOYBUTTONUP: eventType = "SDL_JOYBUTTONUP"; break; 
         case SDL_QUIT: eventType = "SDL_QUIT"; break; 
         case SDL_SYSWMEVENT: eventType = "SDL_SYSWMEVENT"; break; 
         case SDL_VIDEORESIZE: eventType = "SDL_VIDEORESIZE"; break; 
         case SDL_VIDEOEXPOSE: eventType = "SDL_VIDEOEXPOSE"; break; 
       /* Events SDL_USEREVENT through SDL_MAXEVENTS-1 are for your use */
         case SDL_USEREVENT: eventType = "SDL_USEREVENT"; break; 
         default: eventType = "UNKNOWN!"; break;
      }
      dPrintf("Event %d: %s\n", i, eventType);
   }
}

//------------------------------------------------------------------------------
static bool ProcessMessages()
{
   static const int MaxEvents = 255;
   static const U32 Mask = 
      SDL_QUITMASK | SDL_VIDEORESIZEMASK | SDL_VIDEOEXPOSEMASK |
      SDL_ACTIVEEVENTMASK | SDL_SYSWMEVENTMASK | 
      SDL_EVENTMASK(SDL_USEREVENT);
   static SDL_Event events[MaxEvents];
 
   SDL_PumpEvents();
   S32 numEvents = SDL_PeepEvents(events, MaxEvents, SDL_GETEVENT, Mask);
   if (numEvents == 0)
      return true;
   for (int i = 0; i < numEvents; ++i)
   {
      SDL_Event& event = events[i];
      switch (event.type)
      {
         case SDL_QUIT:
            return false;
            break;
         case SDL_VIDEORESIZE:
         case SDL_VIDEOEXPOSE:
            Game->refreshWindow();
            break;
         case SDL_USEREVENT:
            if (event.user.code == TORQUE_SETVIDEOMODE)
            {
               SetAppState();
               // SDL will send a motion event to restore the mouse position
               // on the new window.  Ignore that if the window is locked.
               if (x86UNIXState->windowLocked())
               {
                  SDL_Event tempEvent;
                  SDL_PeepEvents(&tempEvent, 1, SDL_GETEVENT, 
                     SDL_MOUSEMOTIONMASK);
               }
            }
            break;
         case SDL_ACTIVEEVENT:
            SetAppState();
            break;          
         case SDL_SYSWMEVENT:
            ProcessSYSWMEvent(event);
            break;
      }
   }
   return true;
}

//------------------------------------------------------------------------------
// send a destroy window event to the window.  assumes
// window is created.
void SendQuitEvent()
{
   SDL_Event quitevent;
   quitevent.type = SDL_QUIT;
   SDL_PushEvent(&quitevent);
}
#endif // DEDICATED

//------------------------------------------------------------------------------
static inline void Sleep(int secs, int nanoSecs)
{
   timespec sleeptime;
   sleeptime.tv_sec = secs;
   sleeptime.tv_nsec = nanoSecs;
   nanosleep(&sleeptime, NULL);
}

#ifndef TORQUE_DEDICATED
struct AlertWinState
{
      bool fullScreen;
      bool cursorHidden;
      bool inputGrabbed;
};

//------------------------------------------------------------------------------
void DisplayErrorAlert(const char* errMsg, bool showSDLError)
{
   char fullErrMsg[2048];
   dStrncpy(fullErrMsg, errMsg, sizeof(fullErrMsg));
   
   if (showSDLError)
   {
      char* sdlerror = SDL_GetError();
      if (sdlerror != NULL && dStrlen(sdlerror) > 0)
      {
         dStrcat(fullErrMsg, "  (Error: ");
         dStrcat(fullErrMsg, sdlerror);
         dStrcat(fullErrMsg, ")");
      }
   }
   
   Platform::AlertOK("Error", fullErrMsg);
}


//------------------------------------------------------------------------------
static inline void AlertDisableVideo(AlertWinState& state)
{

   state.fullScreen = Video::isFullScreen();
   state.cursorHidden = (SDL_ShowCursor(SDL_QUERY) == SDL_DISABLE);
   state.inputGrabbed = (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_ON);

   if (state.fullScreen)
      SDL_WM_ToggleFullScreen(SDL_GetVideoSurface());
   if (state.cursorHidden)
      SDL_ShowCursor(SDL_ENABLE);
   if (state.inputGrabbed)
      SDL_WM_GrabInput(SDL_GRAB_OFF);
}

//------------------------------------------------------------------------------
static inline void AlertEnableVideo(AlertWinState& state)
{
   if (state.fullScreen)
      SDL_WM_ToggleFullScreen(SDL_GetVideoSurface());
   if (state.cursorHidden)
      SDL_ShowCursor(SDL_DISABLE);
   if (state.inputGrabbed)
      SDL_WM_GrabInput(SDL_GRAB_ON);
}
#endif // DEDICATED

//------------------------------------------------------------------------------
void Platform::AlertOK(const char *windowTitle, const char *message)
{
#ifndef TORQUE_DEDICATED
   if (x86UNIXState->isXWindowsRunning())
   {
      AlertWinState state;
      AlertDisableVideo(state);

      DisplayPtrManager xdisplay;
      XMessageBox mBox(xdisplay.getDisplayPointer());
      mBox.alertOK(windowTitle, message);

      AlertEnableVideo(state);
   }
   else
#endif
   {
      if (Con::isActive() && StdConsole::isEnabled())
         Con::printf("Alert: %s %s", windowTitle, message);
      else
         dPrintf("Alert: %s %s\n", windowTitle, message);
   }
}

//------------------------------------------------------------------------------
bool Platform::AlertOKCancel(const char *windowTitle, const char *message)
{
#ifndef TORQUE_DEDICATED
   if (x86UNIXState->isXWindowsRunning())
   {
      AlertWinState state;
      AlertDisableVideo(state);

      DisplayPtrManager xdisplay;
      XMessageBox mBox(xdisplay.getDisplayPointer());
      bool val = 
         mBox.alertOKCancel(windowTitle, message) == XMessageBox::OK;

      AlertEnableVideo(state);
      return val;
   }
   else
#endif
   {
      if (Con::isActive() && StdConsole::isEnabled())
         Con::printf("Alert: %s %s", windowTitle, message);
      else
         dPrintf("Alert: %s %s\n", windowTitle, message);
      return false;
   }
}

//------------------------------------------------------------------------------
bool Platform::AlertRetry(const char *windowTitle, const char *message)
{
#ifndef TORQUE_DEDICATED
   if (x86UNIXState->isXWindowsRunning())
   {
      AlertWinState state;
      AlertDisableVideo(state);

      DisplayPtrManager xdisplay;
      XMessageBox mBox(xdisplay.getDisplayPointer());
      bool val = 
         mBox.alertRetryCancel(windowTitle, message) == XMessageBox::Retry;
      
      AlertEnableVideo(state);
      return val;
   }
   else
#endif
   {
      if (Con::isActive() && StdConsole::isEnabled())
         Con::printf("Alert: %s %s", windowTitle, message);
      else
         dPrintf("Alert: %s %s\n", windowTitle, message);
      return false;
   }
}

//------------------------------------------------------------------------------
bool Platform::excludeOtherInstances(const char *mutexName)
{
   return AcquireProcessMutex(mutexName);
}


//------------------------------------------------------------------------------
void Platform::enableKeyboardTranslation(void)
{
#ifndef TORQUE_DEDICATED
   // JMQ: not sure if this is needed for i18n keyboards
   //SDL_EnableUNICODE( 1 );
//    SDL_EnableKeyRepeat(
//       SDL_DEFAULT_REPEAT_DELAY, 
//       SDL_DEFAULT_REPEAT_INTERVAL);
#endif
}

//------------------------------------------------------------------------------
void Platform::disableKeyboardTranslation(void)
{
#ifndef TORQUE_DEDICATED
   //SDL_EnableUNICODE( 0 );
   //   SDL_EnableKeyRepeat(0, 0);
#endif
}

//------------------------------------------------------------------------------
void Platform::setWindowLocked(bool locked)
{
#ifndef TORQUE_DEDICATED
   x86UNIXState->setWindowLocked(locked);

   UInputManager* uInputManager = 
      dynamic_cast<UInputManager*>( Input::getManager() );

   if ( uInputManager && uInputManager->isEnabled() && 
      Input::isActive() )
      uInputManager->setWindowLocked(locked);
#endif
}

//------------------------------------------------------------------------------
void Platform::minimizeWindow()
{
#ifndef TORQUE_DEDICATED
   if (x86UNIXState->windowCreated())
      SDL_WM_IconifyWindow();
#endif
}

//------------------------------------------------------------------------------
void Platform::process()
{
   PROFILE_START(XUX_PlatformProcess);
   stdConsole->process();

   if (x86UNIXState->windowCreated())
   {
#ifndef TORQUE_DEDICATED
      // process window events
      PROFILE_START(XUX_ProcessMessages);
      bool quit = !ProcessMessages();
      PROFILE_END();
      if(quit)
      {
         // generate a quit event
         Event quitEvent;
         quitEvent.type = QuitEventType;
         Game->postEvent(quitEvent);
      }

      // process input events
      PROFILE_START(XUX_InputProcess);
      Input::process();
      PROFILE_END();

      // poll redbook state
      PROFILE_START(XUX_PollRedbookDevices);
      PollRedbookDevices();
      PROFILE_END();

      // if we're not the foreground window, sleep for 1 ms
      if (!x86UNIXState->windowActive())
         Sleep(0, getBackgroundSleepTime() * 1000000);
#endif
   }
   else
   {
      // no window
      // if we're not in journal mode, sleep for 1 ms
      // JMQ: since linux's minimum sleep latency seems to be 20ms, this can
      // increase player pings by 10-20ms in the dedicated server.  So 
      // you have to use -dsleep to enable it.  the server sleeps anyway when
      // there are no players connected.
      // JMQ: recent kernels (such as RH 8.0 2.4.18) reduce the latency
      // to 2-4 ms on average.
      if (!Game->isJournalReading() && (x86UNIXState->getDSleep() || 
             Con::getIntVariable("Server::PlayerCount") - 
             Con::getIntVariable("Server::BotCount") <= 0))
      {
         PROFILE_START(XUX_Sleep);
         Sleep(0, getBackgroundSleepTime() * 1000000);
         PROFILE_END();
      }
   }

#ifndef TORQUE_DEDICATED
#if 0 
// JMQ: disabled this because it may fire mistakenly in some configurations.
// sdl's default event handling scheme should be enough.
   // crude check to make sure that we're not loading up events.  the sdl 
   // event queue should never have more than (say) 25 events in it at this
   // point
   const int MaxEvents = 25;
   if (NumEventsPending() > MaxEvents)
   {
      PrintSDLEventQueue();
      AssertFatal(false, "The SDL event queue has too many events!");
   }
#endif
#endif
   PROFILE_END();
}

// extern U32 calculateCRC(void * buffer, S32 len, U32 crcVal );

// #if defined(DEBUG) || defined(INTERNAL_RELEASE)
// static U32 stubCRC = 0;
// #else
// static U32 stubCRC = 0xEA63F56C;
// #endif

//------------------------------------------------------------------------------
const Point2I &Platform::getWindowSize()
{
   return x86UNIXState->getWindowSize();
}


//------------------------------------------------------------------------------
void Platform::setWindowSize( U32 newWidth, U32 newHeight )
{
   x86UNIXState->setWindowSize( (S32) newWidth, (S32) newHeight );
}


//------------------------------------------------------------------------------
void Platform::initWindow(const Point2I &initialSize, const char *name)
{
#ifndef TORQUE_DEDICATED
   // initialize window
   InitWindow(initialSize, name);
   if (!InitOpenGL())
      ImmediateShutdown(1);
#endif
}


//------------------------------------------------------------------------------
// Web browser function:
//------------------------------------------------------------------------------
bool Platform::openWebBrowser( const char* webAddress )
{
   if (!webAddress || dStrlen(webAddress)==0)
      return false;

   // look for a browser preference variable
   // JMQTODO: be nice to implement some UI to customize this
   const char* webBrowser = Con::getVariable("Pref::Unix::WebBrowser");
   if (dStrlen(webBrowser) == 0)
      webBrowser = NULL;

   pid_t pid = fork();
   if (pid == -1)
   {
      Con::printf("WARNING: Platform::openWebBrowser failed to fork");
      return false;
   }
   else if (pid != 0)
   {
      // parent
      if (Video::isFullScreen())
         Video::toggleFullScreen();

      return true;
   }
   else if (pid == 0)
   {
      // child
      // try to exec konqueror, then netscape
      char* argv[3];
      argv[0] = "";
      argv[1] = const_cast<char*>(webAddress);
      argv[2] = 0;

      int ok = -1;

      // if execvp returns, it means it couldn't execute the program
      if (webBrowser != NULL)
         ok = execvp(webBrowser, argv);

      ok = execvp("konqueror", argv);
      ok = execvp("mozilla", argv);
      ok = execvp("netscape", argv);
      // use dPrintf instead of Con here since we're now in another process, 
      dPrintf("WARNING: Platform::openWebBrowser: couldn't launch a web browser\n");
      _exit(-1);     
      return false;
   }
   else
   {
      Con::printf("WARNING: Platform::openWebBrowser: forking problem");
      return false;
   }
}

//------------------------------------------------------------------------------
// Login password routines:
//------------------------------------------------------------------------------
const char* Platform::getLoginPassword()
{
   Con::printf("WARNING: Platform::getLoginPassword() is unimplemented");
   return "";
}

//------------------------------------------------------------------------------
bool Platform::setLoginPassword( const char* password )
{
   Con::printf("WARNING: Platform::setLoginPassword is unimplemented");
   return false;
}

//-------------------------------------------------------------------------------
void TimeManager::process()
{
   U32 curTime = Platform::getRealMilliseconds();
   TimeEvent event;
   event.elapsedTime = curTime - lastTimeTick;
   if(event.elapsedTime > sgTimeManagerProcessInterval)
   {
      lastTimeTick = curTime;
      Game->postEvent(event);
   }
}

//------------------------------------------------------------------------------
ConsoleFunction( getDesktopResolution, const char*, 1, 1, 
   "getDesktopResolution()" )
{
   if (!x86UNIXState->windowCreated())
      return "0 0 0";

   char buffer[256];
   char* returnString = Con::getReturnBuffer( dStrlen( buffer ) + 1 );

   dSprintf( buffer, sizeof( buffer ), "%d %d %d", 
      x86UNIXState->getDesktopSize().x,
      x86UNIXState->getDesktopSize().y, 
      x86UNIXState->getDesktopBpp() );
   dStrcpy( returnString, buffer );
   return( returnString );
}

//------------------------------------------------------------------------------
// Silly Korean registry key checker:
//------------------------------------------------------------------------------
ConsoleFunction( isKoreanBuild, bool, 1, 1, "isKoreanBuild()" )
{
   Con::printf("WARNING: isKoreanBuild() is unimplemented");
   return false;
}

//------------------------------------------------------------------------------
int main(S32 argc, const char **argv)
{
   // init platform state
   x86UNIXState = new x86UNIXPlatformState;

   // parse the command line for unix-specific params
   Vector<char *> newCommandLine;
   S32 returnVal = ParseCommandLine(argc, argv, newCommandLine);
   if (returnVal != 0)
      return returnVal;

   // init lastTimeTick for TimeManager::process()
   lastTimeTick = Platform::getRealMilliseconds();

   // init process control stuff 
   ProcessControlInit();

   // check to see if X is running
   DetectWindowingSystem();
  
   // run the game
   returnVal = Game->main(newCommandLine.size(), 
      const_cast<const char**>(newCommandLine.address()));

   // dispose of command line
   for(U32 i = 0; i < newCommandLine.size(); i++)
      delete [] newCommandLine[i];

   // dispose of state
   delete x86UNIXState;

   return returnVal;
}

void Platform::setWindowTitle( const char* title )
{
#ifndef TORQUE_DEDICATED
   x86UNIXState->setWindowName(title);
   SDL_WM_SetCaption(x86UNIXState->getWindowName(), NULL);
#endif
}

Resolution Video::getDesktopResolution()
{
   Resolution  Result;
   Result.h   = x86UNIXState->getDesktopSize().x;
   Result.w   = x86UNIXState->getDesktopSize().y;
   Result.bpp = x86UNIXState->getDesktopBpp();

  return Result;
}


//-----------------------------------------------------------------------------
void Platform::restartInstance()
{

   if (Game->isRunning() )
   {
      //Con::errorf( "Error restarting Instance. Game is Still running!");
      return;
   }

   char cmd[2048];
   sprintf(cmd, "%s &", x86UNIXState->getExePathName());
   system(cmd);
   exit(0);
}
