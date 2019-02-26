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
#include "platformSDL/sdlInputManager.h"
#include "gfx/gfxDevice.h"
#include "core/util/journal/process.h"
#include "core/strings/unicode.h"
#include "gfx/bitmap/gBitmap.h"

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

   mInputState = KeyboardInputState::NONE;
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
   // Primary is monitor 0
   return getMonitorRect(0);
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

S32 PlatformWindowManagerSDL::findFirstMatchingMonitor(const char* name)
{
   S32 count = SDL_GetNumVideoDisplays();
   for (U32 index = 0; index < count; ++index)
   {
      if (dStrstr(name, SDL_GetDisplayName(index)) == name)
         return index;
   }

   return 0;
}

U32 PlatformWindowManagerSDL::getMonitorCount()
{
   S32 monitorCount = SDL_GetNumVideoDisplays();
   if (monitorCount < 0)
   {
      Con::errorf("SDL_GetNumVideoDisplays() failed: %s", SDL_GetError());
      monitorCount = 0;
   }

   return (U32)monitorCount;
}

const char* PlatformWindowManagerSDL::getMonitorName(U32 index)
{
   const char* monitorName = SDL_GetDisplayName(index);
   if (monitorName == NULL)
      Con::errorf("SDL_GetDisplayName() failed: %s", SDL_GetError());

   return monitorName;
}

RectI PlatformWindowManagerSDL::getMonitorRect(U32 index)
{
   SDL_Rect sdlRect;
   if (0 != SDL_GetDisplayBounds(index, &sdlRect))
   {
      Con::errorf("SDL_GetDisplayBounds() failed: %s", SDL_GetError());
      return RectI(0, 0, 0, 0);
   }

   return RectI(sdlRect.x, sdlRect.y, sdlRect.w, sdlRect.h);
}

void PlatformWindowManagerSDL::getMonitorRegions(Vector<RectI> &regions)
{
   SDL_Rect sdlRect;
   S32 monitorCount = SDL_GetNumVideoDisplays();
   for (S32 index = 0; index < monitorCount; ++index)
   {
      if (0 == SDL_GetDisplayBounds(index, &sdlRect))
         regions.push_back(RectI(sdlRect.x, sdlRect.y, sdlRect.w, sdlRect.h));
   }
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
   U32 windowFlags = /*SDL_WINDOW_SHOWN |*/ SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;

   if(GFX->getAdapterType() == OpenGL)
       windowFlags |= SDL_WINDOW_OPENGL;

   window->mWindowHandle = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, mode.resolution.x, mode.resolution.y, windowFlags );
   window->mWindowId = SDL_GetWindowID( window->mWindowHandle );
   window->mOwningManager = this;
   mWindowMap[ window->mWindowId ] = window;

   //Now, fetch our window icon, if any
   Torque::Path iconPath = Torque::Path(Con::getVariable( "$Core::windowIcon" ));

   if (iconPath.getExtension() == String("bmp"))
   {
      Con::errorf("Unable to use bmp format images for the window icon. Please use a different format.");
   }
   else
   {
      Resource<GBitmap> img = GBitmap::load(iconPath);
      if (img != NULL)
      {
         U32 pitch;
         U32 width = img->getWidth();
         bool hasAlpha = img->getHasTransparency();
         U32 depth;

         if (hasAlpha)
         {
            pitch = 4 * width;
            depth = 32;
         }
         else
         {
            pitch = 3 * width;
            depth = 24;
         }

         Uint32 rmask, gmask, bmask, amask;
         if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
         {
            S32 shift = hasAlpha ? 8 : 0;
            rmask = 0xff000000 >> shift;
            gmask = 0x00ff0000 >> shift;
            bmask = 0x0000ff00 >> shift;
            amask = 0x000000ff >> shift;
         }
         else
         {
            rmask = 0x000000ff;
            gmask = 0x0000ff00;
            bmask = 0x00ff0000;
            amask = hasAlpha ? 0xff000000 : 0;
         }

         SDL_Surface* iconSurface = SDL_CreateRGBSurfaceFrom(img->getAddress(0, 0), img->getWidth(), img->getHeight(), depth, pitch, rmask, gmask, bmask, amask);

         SDL_SetWindowIcon(window->mWindowHandle, iconSurface);

         SDL_FreeSurface(iconSurface);
      }
   }

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

   //Set it up for drag-n-drop events 
#ifdef TORQUE_TOOLS
   SDL_EventState(SDL_DROPBEGIN, SDL_ENABLE);
   SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
   SDL_EventState(SDL_DROPCOMPLETE, SDL_ENABLE);
#endif

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
      if (evt.type >= SDL_JOYAXISMOTION && evt.type <= SDL_CONTROLLERDEVICEREMAPPED)
      {
         SDLInputManager* mgr = static_cast<SDLInputManager*>(Input::getManager());
         if (mgr)
            mgr->processEvent(evt);
         continue;
      }
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

         case(SDL_DROPBEGIN):
         {
            if (!Con::isFunction("onDropBegin"))
               break;

            Con::executef("onDropBegin");
         }

         case (SDL_DROPFILE):
         {
            // In case if dropped file
            if (!Con::isFunction("onDropFile"))
               break;

            char* fileName = evt.drop.file;

            if (!Platform::isFile(fileName))
               break;

            Con::executef("onDropFile", StringTable->insert(fileName));

            SDL_free(fileName);    // Free dropped_filedir memory
            break;
         }

         case(SDL_DROPCOMPLETE):
         {
            if (Con::isFunction("onDropEnd"))
               Con::executef("onDropEnd");
            break;
         }

         default:
         {
#ifdef TORQUE_DEBUG
            Con::warnf("Unhandled SDL input event: 0x%04x", evt.type);
#endif
         }
      }
   }

   // After the event loop is processed, we can now see if we have to notify
   // SDL that we want text based events. This fixes a bug where text based
   // events would be generated while key presses would still be happening.
   // See KeyboardInputState for further documentation.
   if (mInputState != KeyboardInputState::NONE)
   {
      // Update text mode toggling.
      if (mInputState == KeyboardInputState::TEXT_INPUT)
         SDL_StartTextInput();
      else
         SDL_StopTextInput();

      // Done until we need to update it again.
      mInputState = KeyboardInputState::NONE;
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

void PlatformWindowManagerSDL::updateSDLTextInputState(KeyboardInputState state)
{
   // Force update state. This will respond at the end of the event loop.
   mInputState = state;
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
   int res = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS | SDL_INIT_NOPARACHUTE);
   AssertFatal(res != -1, avar("SDL error:%s", SDL_GetError()));

   // By default, SDL enables text input. We disable it on initialization, and
   // we will enable it whenever the time is right.
   SDL_StopTextInput();
}
