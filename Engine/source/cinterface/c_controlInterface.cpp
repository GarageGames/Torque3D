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

#include "c_controlInterface.h"

#include "console/consoleInternal.h"
#include "console/simSet.h"
#include "app/mainLoop.h"
#include "windowManager/platformWindow.h"
#include "windowManager/platformWindowMgr.h"

#ifdef TORQUE_OS_WIN
#include "windowManager/win32/win32Window.h"
#include "windowManager/win32/winDispatch.h"
extern void createFontInit(void);
extern void createFontShutdown(void);
#endif


#if defined( TORQUE_MINIDUMP ) && defined( TORQUE_RELEASE )
extern S32 CreateMiniDump(LPEXCEPTION_POINTERS ExceptionInfo);
#endif

extern bool LinkConsoleFunctions;

extern "C" {

   // reset the engine, unloading any current level and returning to the main menu
   void torque_reset()
   {
      Con::evaluate("disconnect();");
   }

   // initialize Torque 3D including argument handling
   bool torque_engineinit(S32 argc, const char **argv)
   {

#if defined( TORQUE_MINIDUMP ) && defined( TORQUE_RELEASE )
      __try {
#endif

         LinkConsoleFunctions = true;

#if defined(_MSC_VER)
         createFontInit();
#endif

         // Initialize the subsystems.
         StandardMainLoop::init();

         // Handle any command line args.
         if (!StandardMainLoop::handleCommandLine(argc, argv))
         {
            Platform::AlertOK("Error", "Failed to initialize game, shutting down.");
            return false;
         }

#if defined( TORQUE_MINIDUMP ) && defined( TORQUE_RELEASE )
      }

      __except (CreateMiniDump(GetExceptionInformation()))
      {
         _exit(0);
      }
#endif

      return true;

   }

   // tick Torque 3D's main loop
   S32 torque_enginetick()
   {

#if defined( TORQUE_MINIDUMP ) && defined( TORQUE_RELEASE )
      __try {
#endif

         bool ret = StandardMainLoop::doMainLoop();
         return ret;

#if defined( TORQUE_MINIDUMP ) && defined( TORQUE_RELEASE )
      }
      __except (CreateMiniDump(GetExceptionInformation()))
      {
         _exit(0);
      }
#endif

   }

   S32 torque_getreturnstatus()
   {
      return StandardMainLoop::getReturnStatus();
   }

   // signal an engine shutdown (as with the quit(); console command)
   void torque_enginesignalshutdown()
   {
      Con::evaluate("quit();");
   }

   // shutdown the engine
   S32 torque_engineshutdown()
   {

#if defined( TORQUE_MINIDUMP ) && defined( TORQUE_RELEASE )
      __try {
#endif

         // Clean everything up.
         StandardMainLoop::shutdown();

#if defined(_MSC_VER)
         createFontShutdown();
#endif

#if defined( TORQUE_MINIDUMP ) && defined( TORQUE_RELEASE )
      }

      __except (CreateMiniDump(GetExceptionInformation()))
      {
         _exit(0);
      }
#endif

      // Return.  
      return true;

   }

   bool torque_isdebugbuild()
   {
#ifdef _DEBUG
      return true;
#else
      return false;
#endif

   }

   // set Torque 3D into web deployment mode (disable fullscreen exlusive mode, etc)
   void torque_setwebdeployment()
   {
      Platform::setWebDeployment(true);
   }

   // resize the Torque 3D child window to the specified width and height
   void torque_resizewindow(S32 width, S32 height)
   {
      if (PlatformWindowManager::get() && PlatformWindowManager::get()->getFirstWindow())
         PlatformWindowManager::get()->getFirstWindow()->setSize(Point2I(width, height));
   }

#if defined(TORQUE_OS_WIN) && !defined(TORQUE_SDL)
   // retrieve the hwnd of our render window
   void* torque_gethwnd()
   {
      if (PlatformWindowManager::get() && PlatformWindowManager::get()->getFirstWindow())
      {
         Win32Window* w = (Win32Window*)PlatformWindowManager::get()->getFirstWindow();
         return (void *)w->getHWND();
      }

      return NULL;
   }

   // directly add a message to the Torque 3D event queue, bypassing the Windows event queue
   // this is useful in the case of the IE plugin, where we are hooking into an application 
   // level message, and posting to the windows queue would cause a hang
   void torque_directmessage(U32 message, U32 wparam, U32 lparam)
   {
      if (PlatformWindowManager::get() && PlatformWindowManager::get()->getFirstWindow())
      {
         Win32Window* w = (Win32Window*)PlatformWindowManager::get()->getFirstWindow();
         Dispatch(DelayedDispatch, w->getHWND(), message, wparam, lparam);
      }
   }

#endif

#ifdef TORQUE_OS_WIN
   void torque_inputevent(S32 type, S32 value1, S32 value2)
   {
      if (PlatformWindowManager::get() && PlatformWindowManager::get()->getFirstWindow())
      {
         Win32Window* w = (Win32Window*)PlatformWindowManager::get()->getFirstWindow();
         WindowId devId = w->getWindowId();

         switch (type)
         {
         case 0:
            w->mouseEvent.trigger(devId, 0, value1, value2, w->isMouseLocked());
            break;
         case 1:
            if (value2)
               w->buttonEvent.trigger(devId, 0, IA_MAKE, value1);
            else
               w->buttonEvent.trigger(devId, 0, IA_BREAK, value1);
            break;

         }
      }
   }
#endif

   static char* gExecutablePath = NULL;

   const char* torque_getexecutablepath()
   {
      return gExecutablePath;
   }

   void torque_setexecutablepath(const char* directory)
   {
      dsize_t pathLen = dStrlen(directory) + 1;
      gExecutablePath = new char[pathLen];
      dStrcpy(gExecutablePath, directory, pathLen);
   }
}