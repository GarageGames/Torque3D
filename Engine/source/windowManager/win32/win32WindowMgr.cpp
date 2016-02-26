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

#include "platformWin32/platformWin32.h"
#include "windowManager/win32/win32WindowMgr.h"
#include "gfx/gfxDevice.h"
#include "windowManager/win32/winDispatch.h"
#include "core/util/journal/process.h"
#include "core/strings/unicode.h"

#if !defined( TORQUE_SDL )

// ------------------------------------------------------------------------

void CloseSplashWindow(HINSTANCE hinst);

PlatformWindowManager * CreatePlatformWindowManager()
{
   return new Win32WindowManager();
}

// ------------------------------------------------------------------------

Win32WindowManager::Win32WindowManager()
{
   // Register in the process list.
   mOnProcessSignalSlot.setDelegate( this, &Win32WindowManager::_process );
   Process::notify( mOnProcessSignalSlot, PROCESS_INPUT_ORDER );

   // Init our list of allocated windows.
   mWindowListHead = NULL;

   // By default, we have no parent window.
   mParentWindow = NULL;

   mCurtainWindow = NULL;

   mOffscreenRender = false;

   mDisplayWindow = false;

   buildMonitorsList();
}

Win32WindowManager::~Win32WindowManager()
{
   // Kill all our windows first.
   while(mWindowListHead)
      // The destructors update the list, so this works just fine.
      delete mWindowListHead;
}

RectI Win32WindowManager::getPrimaryDesktopArea()
{
   RECT primaryWorkRect; 
   SystemParametersInfo(SPI_GETWORKAREA, 0, &primaryWorkRect, 0);

   RectI res;
   res.point.x  = primaryWorkRect.left;
   res.point.y  = primaryWorkRect.top;
   res.extent.x = primaryWorkRect.right - primaryWorkRect.left;
   res.extent.y = primaryWorkRect.bottom - primaryWorkRect.top;

   return res;
}

Point2I Win32WindowManager::getDesktopResolution()
{
   DEVMODE devMode;
   dMemset( &devMode, 0, sizeof( devMode ) );
   devMode.dmSize = sizeof( devMode );

   if (!::EnumDisplaySettings(NULL, ENUM_REGISTRY_SETTINGS, &devMode))
      return Point2I(-1,-1);

   // Return Resolution
   return Point2I(devMode.dmPelsWidth, devMode.dmPelsHeight);
}

S32 Win32WindowManager::getDesktopBitDepth()
{
   DEVMODE devMode;
   dMemset( &devMode, 0, sizeof( devMode ) );
   devMode.dmSize = sizeof( devMode );

   if (!::EnumDisplaySettings(NULL, ENUM_REGISTRY_SETTINGS, &devMode))
      return -1;

   // Return Bits per Pixel
   return (S32)devMode.dmBitsPerPel;
}

BOOL Win32WindowManager::MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData )
{
   Vector<MonitorInfo> * monitors = (Vector<MonitorInfo>*)dwData;

   // Fill out the new monitor structure
   monitors->increment();
   MonitorInfo& monitor = monitors->last();
   monitor.monitorHandle = hMonitor;
   monitor.region.point.x = lprcMonitor->left;
   monitor.region.point.y = lprcMonitor->top;
   monitor.region.extent.x = lprcMonitor->right - lprcMonitor->left;
   monitor.region.extent.y = lprcMonitor->bottom - lprcMonitor->top;

   MONITORINFOEX info;
   info.cbSize = sizeof(MONITORINFOEX);
   if(GetMonitorInfo(hMonitor, &info))
   {
      monitor.name = info.szDevice;
   }

   return true;
}

void Win32WindowManager::buildMonitorsList()
{
   // Clear the list
   mMonitors.clear();

   // Enumerate all monitors
   EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (uintptr_t)&mMonitors);
}

S32 Win32WindowManager::findFirstMatchingMonitor(const char* name)
{
   // Try and match the first part of the output device display name.  For example,
   // a Monitor name of "\\.\DISPLAY1" might correspond to a display name
   // of "\\.\DISPLAY1\Monitor0".  If two monitors are set up in duplicate mode then
   // they will have the same 'display' part in their display name.
   for(U32 i=0; i<mMonitors.size(); ++i)
   {
      if(dStrstr(name, mMonitors[i].name) == name)
         return i;
   }

   return -1;
}

U32 Win32WindowManager::getMonitorCount()
{
   return mMonitors.size();
}

const char* Win32WindowManager::getMonitorName(U32 index)
{
   if(index >= mMonitors.size())
      return "";

   return mMonitors[index].name.c_str();
}

RectI Win32WindowManager::getMonitorRect(U32 index)
{
   if(index >= mMonitors.size())
      return RectI(0, 0, 0, 0);

   return mMonitors[index].region;
}

BOOL Win32WindowManager::MonitorRegionEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData )
{
   Vector<RectI> * regions = (Vector<RectI>*)dwData;

   regions->increment();
   RectI& lastRegion = regions->last();
   lastRegion.point.x = lprcMonitor->left;
   lastRegion.point.y = lprcMonitor->top;
   lastRegion.extent.x = lprcMonitor->right - lprcMonitor->left;
   lastRegion.extent.y = lprcMonitor->bottom - lprcMonitor->top;

   return true;
}

void Win32WindowManager::getMonitorRegions(Vector<RectI> &regions)
{
   EnumDisplayMonitors(NULL, NULL, MonitorRegionEnumProc, (U32)(void*)&regions);
}

void Win32WindowManager::getWindows(VectorPtr<PlatformWindow*> &windows)
{
   Win32Window *win = mWindowListHead;
   while(win)
   {
      windows.push_back(win);
      win = win->mNextWindow;
   }
}

PlatformWindow *Win32WindowManager::createWindow(GFXDevice *device, const GFXVideoMode &mode)
{
   // Do the allocation.
   Win32Window *w32w = new Win32Window();
   w32w->setOffscreenRender(mOffscreenRender);
   w32w->mWindowId = getNextId();
   w32w->mOwningManager = this;

   // Link into our list of windows.
   linkWindow(w32w);

   DWORD	dwExStyle;
   DWORD dwStyle = WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
   dwStyle       |= WS_OVERLAPPEDWINDOW | WS_THICKFRAME | WS_CAPTION; 
   dwExStyle     = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;

   // If we're parented, we want a different set of window styles.
   if(mParentWindow)
      dwStyle = WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW;

   if (mOffscreenRender)
   {
      dwStyle = WS_OVERLAPPEDWINDOW;
      dwExStyle = 0;
   }

   // Create the window handle
   w32w->mWindowHandle = CreateWindowEx(
      dwExStyle,
      Win32Window::getWindowClassName(),           //class name
      String( getEngineProductString() ).utf16(),  //window title
      dwStyle,                                     //style - need clip siblings/children for opengl
      0,
      0,
      0,
      0,
      mParentWindow,                     //parent window
      NULL,                              //menu? No.
      NULL,                              //the hInstance
      NULL );                            //no funky params

   // Note the style we created with so we can switch back to it when we're
   // done with full-screen mode.
   w32w->mWindowedWindowStyle = dwStyle;

   // Set the video mode on the window
   w32w->setVideoMode(mode);

   // Associate our window struct with the HWND.
   SetWindowLongPtr(w32w->mWindowHandle, GWLP_USERDATA, (LONG_PTR)w32w);

   // Do some error checking.
   AssertFatal(w32w->mWindowHandle != NULL, "Win32WindowManager::createWindow - Could not create window!");
   if(w32w->mWindowHandle == NULL)
   {
      Con::errorf("Win32WindowManager::createWindow - Could not create window!");
      delete w32w;
      return NULL;
   }

   // If we're not rendering offscreen, make sure our window is shown and drawn to.

   w32w->setDisplayWindow(mDisplayWindow);

   if (!mOffscreenRender && mDisplayWindow)
   {
      ShowWindow( w32w->mWindowHandle, SW_SHOWDEFAULT );
      CloseSplashWindow(winState.appInstance);
   }

   // Bind the window to the specified device.
   if(device)
   {
      w32w->mDevice = device;
      w32w->mTarget = device->allocWindowTarget(w32w);
      AssertISV(w32w->mTarget, 
         "Win32WindowManager::createWindow - failed to get a window target back from the device.");
   }
   else
   {
      Con::warnf("Win32WindowManager::createWindow - created a window with no device!");
   }

   // Update it if needed.
   UpdateWindow( w32w->mWindowHandle );

   return w32w;
}


void Win32WindowManager::setParentWindow(void* newParent)
{
   Con::printf( "Setting parent HWND: %d", newParent );
   mParentWindow = (HWND)newParent;
   if( mWindowListHead && mWindowListHead->mWindowHandle )
      ::SetParent( mWindowListHead->mWindowHandle, mParentWindow);
}

void* Win32WindowManager::getParentWindow()
{
   return (void*)mParentWindow;
}

void Win32WindowManager::_process()
{
   MSG msg;
   bool _blocking = false;

   // CodeReview [tom, 4/30/2007] Maintaining two completely separate message
   // handlers that are essentially the same is silly. The first one never
   // seems to run as _blocking is hard coded to false above, so is this even
   // needed ? If it is, this should be rewritten to use the one loop that
   // adjusts as needed based on _blocking and Journal::IsPlaying()

   if (_blocking && !Journal::IsPlaying()) 
   {
      // In blocking mode, we process one message at a time.
      if (GetMessage(&msg, NULL, 0, 0)) 
      {
         bool noTranslate = false;
         Win32Window *w32w = mWindowListHead;
         while(w32w)
         {
            noTranslate = w32w->translateMessage(msg);
            if(noTranslate) break;
            w32w = w32w->mNextWindow;
         }

         if(! noTranslate)
         {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
         }
      }
      else
         // This should be WM_QUIT
         Dispatch(ImmediateDispatch,0,msg.message,msg.wParam,msg.lParam);
   }
   else
   {
      // Process all queued up messages
      while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
      {
         bool translated = false;

//          Win32Window *w32w = mWindowListHead;
//          while(w32w)
//          {
//             noTranslate = w32w->translateMessage(msg);
//             if(noTranslate) break;
//             w32w = w32w->mNextWindow;
//          }

         // [tom, 4/30/2007] I think this should work, but leaving the above commented
         // out just in case this is actually fubared with multiple windows.
         Win32Window* window = (Win32Window*)(GetWindowLongPtr(msg.hwnd, GWLP_USERDATA));
         if(window)
            translated = window->translateMessage(msg);
         
         if(! translated)
         {
            // Win32Window::translateMessage() will post a WM_COMMAND event for
            // translated accelerator events, so dispatching again will cause a
            // the input event to be dispatched, which is usually not what we want.
            TranslateMessage(&msg);
            DispatchMessage(&msg);
         }

         if (msg.message == WM_QUIT) 
         {
            Dispatch(ImmediateDispatch,0,msg.message,msg.wParam,msg.lParam);
            break;
         }
      }
   }

   // Dispatch any delayed events
   while (DispatchNext());

   // Fire off idle events for every window.
   Win32Window *w32w = mWindowListHead;
   while(w32w)
   {
      w32w->idleEvent.trigger();
      w32w = w32w->mNextWindow;
   }

}

PlatformWindow * Win32WindowManager::getWindowById( WindowId id )
{
   // Walk the list and find the matching id, if any.
   Win32Window *win = mWindowListHead;
   while(win)
   {
      if(win->getWindowId() == id)
         return win;

      win = win->mNextWindow;
   }

   return NULL; 
}

PlatformWindow * Win32WindowManager::getFirstWindow()
{
   return mWindowListHead != NULL ? mWindowListHead : NULL;
}

PlatformWindow* Win32WindowManager::getFocusedWindow()
{
   Win32Window* window = mWindowListHead;
   while( window )
   {
      if( window->isFocused() )
         return window;

      window = window->mNextWindow;
   }

   return NULL;
}

void Win32WindowManager::linkWindow( Win32Window *w )
{
   w->mNextWindow = mWindowListHead;
   mWindowListHead = w;
}

void Win32WindowManager::unlinkWindow( Win32Window *w )
{
   Win32Window **walk = &mWindowListHead;
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

void Win32WindowManager::_processCmdLineArgs( const S32 argc, const char **argv )
{
   if (argc > 1)
   {
      for (S32 i = 1; i < argc; i++)
      {
         if ( dStrnicmp( argv[i], "-window", 7 ) == 0 )
         {
            i++;

            if ( i >= argc )
            {
               Con::errorf( "Command line error: -window requires an argument" );
               break;
            }

            S32   hwnd = dAtoi( argv[i] );

            if ( hwnd == 0 || hwnd == S32_MAX )
            {
               Con::errorf( "Command line error: -window requires a number, found [%s]", argv[i] );
               break;
            }

            mParentWindow = (HWND)hwnd;
            Con::printf( "HWND from command line: %d", hwnd );
         }
         
         if ( dStrnicmp( argv[i], "-offscreen", 10 ) == 0 )
         {
            mOffscreenRender = true;
         }

      }
   }
}

void Win32WindowManager::lowerCurtain()
{
   if(mCurtainWindow)
      return;

   // For now just grab monitor of the first window... we may need to
   // beef this up later on, maybe by passing in the window that's entering
   // leaving full-screen to lowerCurtain.
   HMONITOR hMon = MonitorFromWindow(mWindowListHead->getHWND(), MONITOR_DEFAULTTOPRIMARY);

   // Get the monitor's extents.
   MONITORINFO monInfo;
   dMemset(&monInfo, 0, sizeof(MONITORINFO));
   monInfo.cbSize = sizeof(MONITORINFO);

   GetMonitorInfo(hMon, &monInfo);
 
   mCurtainWindow = CreateWindow(Win32Window::getCurtainWindowClassName(), 
                           dT(""), (WS_POPUP | WS_MAXIMIZE | WS_VISIBLE),
                           monInfo.rcWork.left, monInfo.rcWork.top, 
                           monInfo.rcWork.right - monInfo.rcWork.left, 
                           monInfo.rcWork.bottom - monInfo.rcWork.top, 
                           NULL, NULL, NULL, NULL);

   if (!mOffscreenRender)
      SetWindowPos(mCurtainWindow, HWND_TOPMOST, 0, 0, 0, 0,  SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
}

void Win32WindowManager::raiseCurtain()
{
   if(!mCurtainWindow)
      return;

   DestroyWindow(mCurtainWindow);
   mCurtainWindow = NULL;
}

#endif
