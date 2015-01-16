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

#ifndef  _WINDOWMANAGER_SDL_WINDOWMANAGER_
#define  _WINDOWMANAGER_SDL_WINDOWMANAGER_

#include "math/mMath.h"
#include "gfx/gfxStructs.h"
#include "windowManager/sdl/sdlWindow.h"
#include "core/util/tVector.h"

struct SDL_Window;
class FileDialog; // TODO SDL REMOVE

/// SDL2 implementation of the window manager interface.
class PlatformWindowManagerSDL : public PlatformWindowManager
{
   friend class PlatformWindowSDL;
   friend class FileDialog; // TODO SDL REMOVE

   virtual void _processCmdLineArgs(const S32 argc, const char **argv);

   /// Link the specified window into the window list.
   void linkWindow(PlatformWindowSDL *w);

   /// Remove specified window from the window list.
   void unlinkWindow(PlatformWindowSDL *w);

   /// Callback for the process list.
   void _process();

   /// List of allocated windows.
   PlatformWindowSDL *mWindowListHead;

   /// Parent window, used in window setup in web plugin scenarios.
   SDL_Window *mParentWindow;

   /// This is set as part of the canvas being shown, and flags that the windows should render as normal from now on.
   // Basically a flag that lets the window manager know that we've handled the splash screen, and to operate as normal.
   bool mDisplayWindow;

   /// set via command line -offscreen option, controls whether rendering/input
   // is intended for offscreen rendering
   bool mOffscreenRender;

   /// If a curtain window is present, then will be stored here.
   SDL_Window *mCurtainWindow;

   Map<U32, PlatformWindowSDL*> mWindowMap;

   SignalSlot<void()> mOnProcessSignalSlot;

public:
   PlatformWindowManagerSDL();
   ~PlatformWindowManagerSDL();

   virtual RectI getPrimaryDesktopArea();
   virtual S32       getDesktopBitDepth();
   virtual Point2I   getDesktopResolution();

   /// Build out the monitors list.
   virtual void buildMonitorsList();

   virtual S32 findFirstMatchingMonitor(const char* name);
   virtual U32 getMonitorCount();
   virtual const char* getMonitorName(U32 index);
   virtual RectI getMonitorRect(U32 index);

   virtual void getMonitorRegions(Vector<RectI> &regions);
   virtual PlatformWindow *createWindow(GFXDevice *device, const GFXVideoMode &mode);
   virtual void getWindows(VectorPtr<PlatformWindow*> &windows);

   virtual void setParentWindow(void* newParent);
   virtual void* getParentWindow();

   virtual PlatformWindow *getWindowById(WindowId id);
   virtual PlatformWindow *getFirstWindow();
   virtual PlatformWindow* getFocusedWindow();

   virtual void lowerCurtain();
   virtual void raiseCurtain();

   virtual void setDisplayWindow(bool set) { mDisplayWindow = set; }
};

#endif