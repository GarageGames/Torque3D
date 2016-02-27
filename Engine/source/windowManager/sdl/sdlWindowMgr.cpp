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

#include "windowManager/sdl/sdlWindowMgr.h"
#include "gfx/gfxDevice.h"
#include "core/util/journal/process.h"
#include "core/strings/unicode.h"

#include "SDL.h"

// ------------------------------------------------------------------------

void sdl_CloseSplashWindow(void* hinst);

#ifdef TORQUE_SDL

PlatformWindowManager * CreatePlatformWindowManager()
{
   return new PlatformWindowManagerSDL();
}

#endif

// ------------------------------------------------------------------------

PlatformWindowManagerSDL::PlatformWindowManagerSDL()
{
   // Register in the process list.
   mOnProcessSignalSlot.setDelegate( this, &PlatformWindowManagerSDL::_process );
   Process::notify( mOnProcessSignalSlot, PROCESS_INPUT_ORDER );

   // Init our list of allocated windows.
   mWindowListHead = NULL;

   // By default, we have no parent window.
   mParentWindow = NULL;

   mCurtainWindow = NULL;

   mDisplayWindow = true;
   mOffscreenRender = false;

   buildMonitorsList();
}

PlatformWindowManagerSDL::~PlatformWindowManagerSDL()
{
   // Kill all our windows first.
   while(mWindowListHead)
      // The destructors update the list, so this works just fine.
      delete mWindowListHead;
}

RectI PlatformWindowManagerSDL::getPrimaryDesktopArea()
{
   // TODO SDL
   AssertFatal(0, "");
   return RectI(0,0,0,0);
}

Point2I PlatformWindowManagerSDL::getDesktopResolution()
{
   SDL_DisplayMode mode;
   SDL_GetDesktopDisplayMode(0, &mode);

   // Return Resolution
   return Point2I(mode.w, mode.h);
}

S32 PlatformWindowManagerSDL::getDesktopBitDepth()
{
   // Return Bits per Pixel
   SDL_DisplayMode mode;
   SDL_GetDesktopDisplayMode(0, &mode);
   int bbp;
   unsigned int r,g,b,a;
   SDL_PixelFormatEnumToMasks(mode.format, &bbp, &r, &g, &b, &a);
   return bbp;
}

void PlatformWindowManagerSDL::buildMonitorsList()
{
   // TODO SDL
}

S32 PlatformWindowManagerSDL::findFirstMatchingMonitor(const char* name)
{
   /// TODO SDL
   AssertFatal(0, "");

   return 0;
}

U32 PlatformWindowManagerSDL::getMonitorCount()
{
   // TODO SDL
   AssertFatal(0, "");
   return 1;
}

const char* PlatformWindowManagerSDL::getMonitorName(U32 index)
{
   // TODO SDL
   AssertFatal(0, "");

   return "Monitor";
}

RectI PlatformWindowManagerSDL::getMonitorRect(U32 index)
{
   // TODO SDL
   AssertFatal(0, "");

   return RectI(0, 0, 0,0 );
}

void PlatformWindowManagerSDL::getMonitorRegions(Vector<RectI> &regions)
{
   // TODO SDL
   AssertFatal(0, "");
}

void PlatformWindowManagerSDL::getWindows(VectorPtr<PlatformWindow*> &windows)
{
   PlatformWindowSDL *win = mWindowListHead;
   while(win)
   {
      windows.push_back(win);
      win = win->mNextWindow;
   }
}

PlatformWindow *PlatformWindowManagerSDL::createWindow(GFXDevice *device, const GFXVideoMode &mode)
{
   // Do the allocation.
   PlatformWindowSDL *window = new PlatformWindowSDL();   
   U32 windowFlags = /*SDL_WINDOW_SHOWN |*/ SDL_WINDOW_RESIZABLE;

   if(GFX->getAdapterType() == OpenGL)
       windowFlags |= SDL_WINDOW_OPENGL;

   window->mWindowHandle = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, mode.resolution.x, mode.resolution.y, windowFlags );
   window->mWindowId = SDL_GetWindowID( window->mWindowHandle );
   window->mOwningManager = this;
   mWindowMap[ window->mWindowId ] = window;

   if(device)
   {
      window->mDevice = device;
      window->mTarget = device->allocWindowTarget(window);
      AssertISV(window->mTarget, "PlatformWindowManagerSDL::createWindow - failed to get a window target back from the device.");
   }
   else
   {
      Con::warnf("PlatformWindowManagerSDL::createWindow - created a window with no device!");
   }

   linkWindow(window);

   return window;
}


void PlatformWindowManagerSDL::setParentWindow(void* newParent)
{
   
}

void* PlatformWindowManagerSDL::getParentWindow()
{
   return NULL;
}

void PlatformWindowManagerSDL::_process()
{
   SDL_Event evt;
   while( SDL_PollEvent(&evt) )
   {      
      switch(evt.type)
      {
          case SDL_QUIT:
          {
             PlatformWindowSDL *window = static_cast<PlatformWindowSDL*>( getFirstWindow() );
             if(window)
               window->appEvent.trigger( window->getWindowId(), WindowClose );
             break;
          }

         case SDL_KEYDOWN:
         case SDL_KEYUP:
         {
            PlatformWindowSDL *window = mWindowMap[evt.key.windowID];
            if(window)
               window->_processSDLEvent(evt);
            break;
         }

         case SDL_MOUSEWHEEL:
         {
            PlatformWindowSDL *window = mWindowMap[evt.wheel.windowID];
            if (window)
               window->_processSDLEvent(evt);
            break;
         }

         case SDL_MOUSEMOTION:
         {
            PlatformWindowSDL *window = mWindowMap[evt.motion.windowID];
            if(window)
               window->_processSDLEvent(evt);
            break;
         }

         case SDL_MOUSEBUTTONDOWN:
         case SDL_MOUSEBUTTONUP:
         {
            PlatformWindowSDL *window = mWindowMap[evt.button.windowID];
            if(window)
               window->_processSDLEvent(evt);
            break;
         }

         case SDL_TEXTINPUT:
         {
            PlatformWindowSDL *window = mWindowMap[evt.text.windowID];
            if(window)
               window->_processSDLEvent(evt);
            break;
         }

         case SDL_WINDOWEVENT:
         {
            PlatformWindowSDL *window = mWindowMap[evt.window.windowID];
            if(window)
               window->_processSDLEvent(evt);
            break;
         }

         default:
         {
            //Con::printf("Event: %d", evt.type);
         }
      }
   }

}

PlatformWindow * PlatformWindowManagerSDL::getWindowById( WindowId id )
{
   // Walk the list and find the matching id, if any.
   PlatformWindowSDL *win = mWindowListHead;
   while(win)
   {
      if(win->getWindowId() == id)
         return win;

      win = win->mNextWindow;
   }

   return NULL; 
}

PlatformWindow * PlatformWindowManagerSDL::getFirstWindow()
{
   return mWindowListHead != NULL ? mWindowListHead : NULL;
}

PlatformWindow* PlatformWindowManagerSDL::getFocusedWindow()
{
   PlatformWindowSDL* window = mWindowListHead;
   while( window )
   {
      if( window->isFocused() )
         return window;

      window = window->mNextWindow;
   }

   return NULL;
}

void PlatformWindowManagerSDL::linkWindow( PlatformWindowSDL *w )
{
   w->mNextWindow = mWindowListHead;
   mWindowListHead = w;
}

void PlatformWindowManagerSDL::unlinkWindow( PlatformWindowSDL *w )
{
   PlatformWindowSDL **walk = &mWindowListHead;
   while(*walk)
   {
      if(*walk != w)
      {
         // Advance to next item in list.
         walk = &(*walk)->mNextWindow;
         continue;
      }

      // Got a match - unlink and return.
      *walk = (*walk)->mNextWindow;
      return;
   }
}

void PlatformWindowManagerSDL::_processCmdLineArgs( const S32 argc, const char **argv )
{
   // TODO SDL
}

void PlatformWindowManagerSDL::lowerCurtain()
{
   if(mCurtainWindow)
      return;

   // TODO SDL
}

void PlatformWindowManagerSDL::raiseCurtain()
{
   if(!mCurtainWindow)
      return;

   // TODO SDL
}

bool Platform::closeSplashWindow()
{
    return true;
}

void Platform::openFolder(const char* path )
{
    AssertFatal(0, "Not Implemented");
}

void Platform::openFile(const char* path )
{
    AssertFatal(0, "Not Implemented");
}

//------------------------------------------------------------------------------

namespace GL
{
   void gglPerformBinds();
}

void InitWindowingSystem()
{

}

AFTER_MODULE_INIT(gfx)
{   
   int res = SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS | SDL_INIT_NOPARACHUTE );
   AssertFatal(res != -1, "SDL init error");
}
