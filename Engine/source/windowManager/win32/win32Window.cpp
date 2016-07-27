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

#if !defined(TORQUE_SDL)

#include <windows.h>
#include <tchar.h>
#include <winuser.h>
#include "math/mMath.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxStructs.h"

#include "windowManager/platformWindowMgr.h"
#include "windowManager/win32/win32Window.h"
#include "windowManager/win32/win32WindowMgr.h"
#include "windowManager/win32/win32CursorController.h"
#include "windowManager/win32/winDispatch.h"

#include "platform/menus/popupMenu.h"
#include "platform/platformInput.h"

// for winState structure
#include "platformWin32/platformWin32.h"

const UTF16* _MainWindowClassName = L"TorqueJuggernaughtWindow";
const UTF16* _CurtainWindowClassName = L"TorqueJuggernaughtCurtainWindow";

#define SCREENSAVER_QUERY_DENY 0 // Disable screensaver

#ifndef IDI_ICON1 
#define IDI_ICON1 107
#endif

static bool isScreenSaverRunning()
{
#ifndef SPI_GETSCREENSAVERRUNNING
#define SPI_GETSCREENSAVERRUNNING 114
#endif
	// Windows 2K, and higher. It might be better to hook into
	// the broadcast WM_SETTINGCHANGE message instead of polling for
	// the screen saver status.
	BOOL sreensaver = false;
	SystemParametersInfo(SPI_GETSCREENSAVERRUNNING,0,&sreensaver,0);
	return sreensaver;
}

DISPLAY_DEVICE GetPrimaryDevice()
{
	int index = 0;
	DISPLAY_DEVICE dd;
	dd.cb = sizeof(DISPLAY_DEVICE);

	while (EnumDisplayDevices(NULL, index++, &dd, 0))
	{
		if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) return dd;
	}
	return dd;
}

Win32Window::Win32Window(): mMouseLockPosition(0,0),
mShouldLockMouse(false),
mMouseLocked(false),
mOwningManager(NULL),
mNextWindow(NULL),
mWindowHandle(NULL),
mOldParent(NULL),
mTarget(NULL),
mDevice(NULL),
mAccelHandle(NULL),
mSuppressReset(false),
mMenuHandle(NULL),
mWindowedWindowStyle(0),
mPosition(0,0),
mFullscreen(false)
{
	mCursorController = new Win32CursorController( this );

	mVideoMode.bitDepth = 32;
	mVideoMode.fullScreen = false;
	mVideoMode.refreshRate = 60;
	mVideoMode.resolution.set(800,600);

	_registerWindowClass();
}

Win32Window::~Win32Window()
{
	if(mAccelHandle)
	{
		DestroyAcceleratorTable(mAccelHandle);
		mAccelHandle = NULL;
	}

	// delete our win handle..
	DestroyWindow(mWindowHandle);

	// unlink ourselves from the window list...
	AssertFatal(mOwningManager, "Win32Window::~Win32Window - orphan window, cannot unlink!");
	mOwningManager->unlinkWindow(this);

	_unregisterWindowClass();
}

void* Win32Window::getSystemWindow(const WindowSystem system)
{
   if( system == WindowSystem_Windows)
      return getHWND();

     return NULL;
}

GFXDevice * Win32Window::getGFXDevice()
{
	return mDevice;
}

GFXWindowTarget * Win32Window::getGFXTarget()
{
	return mTarget;
}

const GFXVideoMode & Win32Window::getVideoMode()
{
	return mVideoMode;
}

void Win32Window::setVideoMode( const GFXVideoMode &mode )
{
   bool needCurtain = ( mVideoMode.fullScreen != mode.fullScreen );

   if( needCurtain )
   {
      Con::printf( "Win32Window::setVideoMode - invoking curtain" );
      mOwningManager->lowerCurtain();
   }

   mVideoMode = mode;
   mSuppressReset = true;

   // Can't switch to fullscreen while a child of another window
   if( mode.fullScreen && !Platform::getWebDeployment() && mOwningManager->getParentWindow() )
   {
      mOldParent = reinterpret_cast<HWND>( mOwningManager->getParentWindow() );
      mOwningManager->setParentWindow( NULL );
   }
   else if( !mode.fullScreen && mOldParent )
   {
      mOwningManager->setParentWindow( mOldParent );
      mOldParent = NULL;
   }

   // Set our window to have the right style based on the mode
   if( mode.fullScreen && !Platform::getWebDeployment() && !mOffscreenRender )
   {
      WINDOWPLACEMENT wplacement = { sizeof( wplacement ) };
      DWORD dwStyle = GetWindowLong( getHWND(), GWL_STYLE );
      MONITORINFO mi = { sizeof(mi) };

      if ( GetWindowPlacement( getHWND(), &wplacement ) && GetMonitorInfo( MonitorFromWindow( getHWND(), MONITOR_DEFAULTTOPRIMARY ), &mi ) )
      {
         DISPLAY_DEVICE dd = GetPrimaryDevice();
         DEVMODE dv;
         ZeroMemory( &dv, sizeof( dv ) );
         dv.dmSize = sizeof( DEVMODE );
         EnumDisplaySettings( dd.DeviceName, ENUM_CURRENT_SETTINGS, &dv );
         dv.dmPelsWidth = mode.resolution.x;
         dv.dmPelsHeight = mode.resolution.y;
         dv.dmBitsPerPel = mode.bitDepth;
         dv.dmDisplayFrequency = mode.refreshRate;
         dv.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
         ChangeDisplaySettings( &dv, CDS_FULLSCREEN );
         SetWindowLong( getHWND(), GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW );
         SetWindowPos( getHWND(), HWND_TOP,  mi.rcMonitor.left,
                                             mi.rcMonitor.top,
                                             mi.rcMonitor.right - mi.rcMonitor.left,
                                             mi.rcMonitor.bottom - mi.rcMonitor.top,
                                             SWP_NOOWNERZORDER | SWP_FRAMECHANGED );
      }

      if( mDisplayWindow )
         ShowWindow( getHWND(), SW_SHOWNORMAL );

      // Clear the menu bar from the window for full screen
      if( GetMenu( getHWND() ) )
         SetMenu( getHWND(), NULL );

      // When switching to Fullscreen, reset device after setting style
      if( mTarget.isValid() )
         mTarget->resetMode();

      mFullscreen = true;
   }
   else
   {
      DISPLAY_DEVICE dd = GetPrimaryDevice();
      DEVMODE dv;
      ZeroMemory( &dv, sizeof( dv ) );
      dv.dmSize = sizeof( DEVMODE );
      EnumDisplaySettings( dd.DeviceName, ENUM_CURRENT_SETTINGS, &dv );

      if (  ( WindowManager->getDesktopResolution() != mode.resolution || 
            ( mode.resolution.x != dv.dmPelsWidth ) || ( mode.resolution.y != dv.dmPelsHeight ) ) )
         ChangeDisplaySettings( NULL, 0 );

      // Reset device *first*, so that when we call setSize() and let it
      // access the monitor settings, it won't end up with our fullscreen
      // geometry that is just about to change.

      if( mTarget.isValid() )
         mTarget->resetMode();

      if ( !mOffscreenRender )
      {
         SetWindowLong( getHWND(), GWL_STYLE, mWindowedWindowStyle);
         SetWindowPos( getHWND(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

         // Put back the menu bar, if any
         if(mMenuHandle)
         {
            SetMenu(getHWND(), mMenuHandle);
         }
      }

      // Make sure we're the correct resolution for web deployment
      if (!Platform::getWebDeployment() || !mOwningManager->getParentWindow() || mOffscreenRender)
      {
         setSize(mode.resolution);
      }
      else
      {
         HWND parentWin = reinterpret_cast<HWND>( mOwningManager->getParentWindow() );
         RECT windowRect;
         GetClientRect( parentWin, &windowRect );
         Point2I res( windowRect.right - windowRect.left, windowRect.bottom - windowRect.top );

         if ( res.x == 0 || res.y == 0 )
            setSize( mode.resolution ); // Must be too early in the window set up to obtain the parent's size.
         else
            setSize( res );
      }

      if ( !mOffscreenRender )
      {
         // We have to force Win32 to update the window frame and make the window
         // visible and no longer topmost - this code might be possible to simplify.
         SetWindowPos( getHWND(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED );

         if(mDisplayWindow)
            ShowWindow( getHWND(), SW_SHOWNORMAL );
      }

      mFullscreen = false;
   }

   mSuppressReset = false;

   if( needCurtain )
      mOwningManager->raiseCurtain();

   SetForegroundWindow( getHWND() );
   getScreenResChangeSignal().trigger( this, true );
}

bool Win32Window::clearFullscreen()
{
	return true;
}

bool Win32Window::isFullscreen()
{   
	return mFullscreen;
}

void Win32Window::_setFullscreen(const bool fullscreen)
{
	if (fullscreen == mFullscreen)
		return;

	mFullscreen = fullscreen;
	if(fullscreen && !mOffscreenRender)
	{
		Con::printf("Win32Window::setFullscreen (full) enter");
		SetWindowLong( getHWND(), GWL_STYLE, WS_POPUP|WS_SYSMENU );
		SetWindowPos( getHWND(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
	}
	else
	{
		Con::printf("Win32Window::setFullscreen (windowed) enter");
      if (!mOffscreenRender)
      {
	      SetWindowLong( getHWND(), GWL_STYLE, mWindowedWindowStyle);
	      SetWindowPos( getHWND(), HWND_NOTOPMOST, 0, 0, mVideoMode.resolution.x, mVideoMode.resolution.y, SWP_FRAMECHANGED | SWP_SHOWWINDOW);         
      }

      setSize(mVideoMode.resolution);

	}
	Con::printf("Win32Window::setFullscreen exit");   
}

bool Win32Window::setCaption( const char *cap )
{
	return SetWindowTextA(mWindowHandle, cap);
}

const char * Win32Window::getCaption()
{
	char buff[512];
	S32 strLen = GetWindowTextA(mWindowHandle, buff, 512);

	if(strLen==0)
		return NULL;

	return StringTable->insert(buff);
}

void Win32Window::setFocus()
{
   ::SetFocus( mWindowHandle );
}

void Win32Window::setClientExtent( const Point2I newExtent )
{
	Point2I oldExtent = getClientExtent();
	if (oldExtent == newExtent)
		return;   

	RECT rtClient;
	DWORD Style, ExStyle;
	SetRect( &rtClient, 0, 0, newExtent.x, newExtent.y );
	Style = GetWindowLong( mWindowHandle, GWL_STYLE);
	ExStyle = GetWindowLong( mWindowHandle, GWL_EXSTYLE );

	AdjustWindowRectEx( &rtClient, Style, getMenuHandle() != NULL, ExStyle );
	if( Style & WS_VSCROLL ) 
		rtClient.right += GetSystemMetrics( SM_CXVSCROLL );
	if( Style & WS_HSCROLL ) 
		rtClient.bottom += GetSystemMetrics( SM_CYVSCROLL );

	SetWindowPos( mWindowHandle, NULL, 0, 0, rtClient.right - rtClient.left, rtClient.bottom - rtClient.top, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

const Point2I Win32Window::getClientExtent()
{
	// Fetch Client Rect from Windows
	RECT clientRect;
	::GetClientRect(mWindowHandle, &clientRect);

	// Return as a Torque Point2I - We don't care about origin as it's always 0,0
	return Point2I(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
}

void Win32Window::setBounds( const RectI &newBounds )
{
	RECT newRect;
	newRect.left = newBounds.point.x;
	newRect.top  = newBounds.point.y;
	newRect.bottom = newRect.top + newBounds.extent.y;
	newRect.right  = newRect.left + newBounds.extent.x;

	MoveWindow(mWindowHandle, newRect.left, newRect.top, newRect.right - newRect.left, newRect.bottom - newRect.top, true);
}

const RectI Win32Window::getBounds() const
{
	// Fetch Window Rect from OS
	RECT windowRect;
	::GetWindowRect(mWindowHandle, &windowRect);

	// Return as a Torque RectI
	return RectI(windowRect.left,windowRect.top,windowRect.right - windowRect.left, windowRect.bottom - windowRect.top);   
}

void Win32Window::setPosition( const Point2I newPosition )
{
	SetWindowPos( mWindowHandle, HWND_NOTOPMOST, newPosition.x, newPosition.y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE );
}

const Point2I Win32Window::getPosition()
{
	RECT windowRect;
	GetWindowRect( mWindowHandle, &windowRect );

	// Return position
	return Point2I(windowRect.left,windowRect.top);
}

Point2I Win32Window::clientToScreen( const Point2I& pos )
{
   POINT p = { pos.x, pos.y };
   
   ClientToScreen( mWindowHandle, &p );
   return Point2I( p.x, p.y );
}

Point2I Win32Window::screenToClient( const Point2I& pos )
{
   POINT p = { pos.x, pos.y };
   
   ScreenToClient( mWindowHandle, &p );
   return Point2I( p.x, p.y );
}

void Win32Window::centerWindow()
{
	RECT newRect;
	GetWindowRect(mWindowHandle,&newRect);
	newRect.bottom -= newRect.top;
	newRect.right -= newRect.left;
	newRect.top    = 0;
	newRect.left   = 0;

	HMONITOR hMon = MonitorFromWindow(mWindowHandle, MONITOR_DEFAULTTONEAREST);

	// Get the monitor's extents.
	MONITORINFO monInfo;
	dMemset(&monInfo, 0, sizeof(MONITORINFO));
	monInfo.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(hMon, &monInfo);

   // Calculate the offset to center the window in the working area
	S32 deltaX = ((monInfo.rcWork.right - monInfo.rcWork.left) / 2) - ((newRect.right - newRect.left) / 2);
	S32 deltaY = ((monInfo.rcWork.bottom - monInfo.rcWork.top) / 2) - ((newRect.bottom - newRect.top) / 2);

   // Calculate the new left and top position for the window
   S32 newLeft = newRect.left + deltaX;
   S32 newTop  = newRect.top  + deltaY;

   // Clamp these to be greater than 0 so that the top left corner is never offscreen
   newLeft = mClamp(newLeft, 0, newLeft);
   newTop  = mClamp(newLeft, 0, newTop);

   // Calculate the new width and height
   S32 newWidth  = newRect.right - newRect.left;
   S32 newHeight = newRect.bottom - newRect.top;

   // If the new width and height of the window is larger
   // than the working area of the monitor but is smaller
   // than the monitor size then have it max out at the
   // working area so that it will remain uncovered. We
   // leave it alone if it is bigger than the monitor size
   // (with a small fudge) to support multiple monitors.
   if (newLeft + newWidth > (monInfo.rcWork.right - monInfo.rcWork.left) &&
       newLeft + newWidth <= (monInfo.rcMonitor.right - monInfo.rcMonitor.left) + 4)
       newWidth = (monInfo.rcWork.right - monInfo.rcWork.left) - newLeft;
   if (newTop + newHeight > (monInfo.rcWork.bottom - monInfo.rcWork.top) &&
       newTop + newHeight <= (monInfo.rcMonitor.bottom - monInfo.rcMonitor.top) + 4)
       newHeight = (monInfo.rcWork.bottom - monInfo.rcWork.top) - newTop;

   MoveWindow( mWindowHandle, newLeft, newTop, newWidth, newHeight, true );

   // Make sure the resolution matches the client extent
   Point2I clientExt = getClientExtent();
   mVideoMode.resolution.set( clientExt.x, clientExt.y );

   // Let GFX get an update about the new resolution
   if (mTarget.isValid())
		mTarget->resetMode();
}

bool Win32Window::setSize( const Point2I &newSize )
{
	// Create the window rect (screen centered if not owned by a parent)
	RECT newRect;
	newRect.left = 0;
	newRect.top  = 0;
	newRect.bottom = newRect.top + newSize.y;
	newRect.right  = newRect.left + newSize.x;

	// Adjust the window rect to ensure the client rectangle is the desired resolution
	AdjustWindowRect( &newRect, mWindowedWindowStyle, false);//(bool)(getMenuHandle() != NULL) );

	// Center the window on the screen if we're not a child
	if( !mOwningManager->mParentWindow )
	{
		HMONITOR hMon = MonitorFromWindow(mWindowHandle, MONITOR_DEFAULTTONEAREST);

		// Get the monitor's extents.
		MONITORINFO monInfo;
		dMemset(&monInfo, 0, sizeof(MONITORINFO));
		monInfo.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(hMon, &monInfo);

      // Calculate the offset to center the window in the working area
		S32 deltaX = ((monInfo.rcWork.right - monInfo.rcWork.left) / 2) - ((newRect.right - newRect.left) / 2);
		S32 deltaY = ((monInfo.rcWork.bottom - monInfo.rcWork.top) / 2) - ((newRect.bottom - newRect.top) / 2);

      // Calculate the new left and top position for the window
      S32 newLeft = newRect.left + deltaX;
      S32 newTop  = newRect.top  + deltaY;

      // Clamp these to be greater than 0 so that the top left corner is never offscreen
      newLeft = mClamp(newLeft, 0, newLeft);
      newTop  = mClamp(newLeft, 0, newTop);

      // Calculate the new width and height
      S32 newWidth  = newRect.right - newRect.left;
      S32 newHeight = newRect.bottom - newRect.top;

      // If the new width and height of the window is larger
      // than the working area of the monitor but is smaller
      // than the monitor size then have it max out at the
      // working area so that it will remain uncovered. We
      // leave it alone if it is bigger than the monitor size
      // (with a small fudge) to support multiple monitors.
      if (newLeft + newWidth > (monInfo.rcWork.right - monInfo.rcWork.left) &&
          newLeft + newWidth <= (monInfo.rcMonitor.right - monInfo.rcMonitor.left) + 4)
          newWidth = (monInfo.rcWork.right - monInfo.rcWork.left) - newLeft;
      if (newTop + newHeight > (monInfo.rcWork.bottom - monInfo.rcWork.top) &&
          newTop + newHeight <= (monInfo.rcMonitor.bottom - monInfo.rcMonitor.top) + 4)
          newHeight = (monInfo.rcWork.bottom - monInfo.rcWork.top) - newTop;

      MoveWindow( mWindowHandle, newLeft, newTop, newWidth, newHeight, true );
	}
	else // Just position it according to the mPosition plus new extent
		MoveWindow(mWindowHandle, newRect.left, newRect.top, newRect.right - newRect.left, newRect.bottom - newRect.top, true);

   // Make sure the resolution matches the client extent
   Point2I clientExt = getClientExtent();
   mVideoMode.resolution.set( clientExt.x, clientExt.y );

   // Let GFX get an update about the new resolution
   if (mTarget.isValid())
		mTarget->resetMode();

	InvalidateRect( NULL, NULL, true );

	return true;
}

bool Win32Window::isOpen()
{
	return true;
}

bool Win32Window::isVisible()
{
	// Is the window open and visible, ie. not minimized?

	if(!mWindowHandle)
		return false;

   if (mOffscreenRender)
      return true;

	return IsWindowVisible(mWindowHandle) 
		&& !IsIconic(mWindowHandle)
		&& !isScreenSaverRunning();
}

bool Win32Window::isFocused()
{

   if (mOffscreenRender)
      return true;

	// CodeReview This is enough to make the plugin and normal/editor scenarios
	// coexist but it seems brittle. I think we need a better way to detect
	// if we're the foreground window, maybe taking into account if any of our
	// window's parents are foreground? [bjg 4/30/07]
	if(mOwningManager->mParentWindow)
		return (GetFocus() == mWindowHandle || IsChild(mWindowHandle, GetFocus()));
	else
		return ((GetFocus() == mWindowHandle ||  IsChild(mWindowHandle, GetFocus())) && GetForegroundWindow() == mWindowHandle);
}

bool Win32Window::isMinimized()
{
   if (mOffscreenRender)
      return false;

    WINDOWPLACEMENT wd;
    if ( GetWindowPlacement( mWindowHandle, &wd ) )
    {
        return ( wd.showCmd == SW_SHOWMINIMIZED );
    }

    return false;
}

bool Win32Window::isMaximized()
{
   if (mOffscreenRender)
      return true;

    WINDOWPLACEMENT wd;
    if ( GetWindowPlacement( mWindowHandle, &wd ) )
    {
        return ( wd.showCmd == SW_SHOWMAXIMIZED );
    }

    return false;
}

WindowId Win32Window::getWindowId()
{
	return mWindowId;
}

void Win32Window::minimize()
{
   if (mOffscreenRender)
      return;

	ShowWindow( mWindowHandle, SW_MINIMIZE );
}

void Win32Window::maximize()
{
   if (mOffscreenRender)
      return;

	ShowWindow( mWindowHandle, SW_MAXIMIZE );
}

void Win32Window::restore()
{
   if (mOffscreenRender)
      return;

	ShowWindow( mWindowHandle, SW_RESTORE );
}

void Win32Window::hide()
{
   if (mOffscreenRender)
      return;

	ShowWindow( mWindowHandle, SW_HIDE );
}

void Win32Window::show()
{
   if (mOffscreenRender)
      return;

	ShowWindow( mWindowHandle, SW_SHOWNORMAL );
}

void Win32Window::close()
{
	delete this;
}

void Win32Window::_registerWindowClass()
{
	// Check to see if it exists already.
	WNDCLASSEX classInfo;
	if (GetClassInfoEx(GetModuleHandle(NULL),_MainWindowClassName,&classInfo))
		return;

	HMODULE appInstance = GetModuleHandle(NULL);
	HICON   appIcon = LoadIcon(appInstance, MAKEINTRESOURCE(IDI_ICON1));

	// Window class shared by all MainWindow objects
	classInfo.lpszClassName = _MainWindowClassName;
	classInfo.cbSize        = sizeof(WNDCLASSEX);
	classInfo.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	classInfo.lpfnWndProc   = (WNDPROC)WindowProc;
	classInfo.hInstance     = appInstance;       // Owner of this class
	classInfo.hIcon         = appIcon;           // Icon name
	classInfo.hIconSm       = appIcon;           // Icon name
	classInfo.hCursor       = LoadCursor(NULL, IDC_ARROW); // Cursor
	classInfo.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);    // Default color
	classInfo.lpszMenuName  = NULL;
	classInfo.cbClsExtra    = 0;
	classInfo.cbWndExtra    = 0;
	if (!RegisterClassEx(&classInfo))
		AssertISV(false,"Window class initialization failed");

	classInfo.lpfnWndProc = DefWindowProc;
	classInfo.hCursor = NULL;
	classInfo.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	classInfo.lpszClassName = _CurtainWindowClassName;
	if (!RegisterClassEx(&classInfo))
		AssertISV(false,"Curtain window class initialization failed");
}

void Win32Window::_unregisterWindowClass()
{
	WNDCLASSEX classInfo;
	if (GetClassInfoEx(GetModuleHandle(NULL),_MainWindowClassName,&classInfo))
		UnregisterClass(_MainWindowClassName,GetModuleHandle(NULL));
	if (GetClassInfoEx(GetModuleHandle(NULL),_CurtainWindowClassName,&classInfo))
		UnregisterClass(_CurtainWindowClassName,GetModuleHandle(NULL));
}

LRESULT PASCAL Win32Window::WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	// CodeReview [tom, 4/30/2007] The two casts here seem somewhat silly and redundant ?
	Win32Window* window = (Win32Window*)((PlatformWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA));
	const WindowId devId = window ? window->getWindowId() : 0;

   if (window && window->getOffscreenRender())
      return DefWindowProc(hWnd, message, wParam, lParam);

	switch (message)
	{

	case WM_DISPLAYCHANGE:
      // Update the monitor list
      PlatformWindowManager::get()->buildMonitorsList();

		if(window && window->isVisible() && !window->mSuppressReset && window->getVideoMode().bitDepth != wParam)
		{
			Con::warnf("Win32Window::WindowProc - resetting device due to display mode BPP change.");
			window->getGFXTarget()->resetMode();
		}
		break;

	case WM_MOUSEACTIVATE:
		SetFocus(hWnd);
		return MA_ACTIVATE;

	case WM_MOUSEMOVE:
      if (window && GetFocus() != hWnd && IsChild(hWnd, GetFocus()))
      {
         SetFocus(hWnd);
         break;
      }

		// If our foreground window is the browser and we don't have focus grab it
		if (Platform::getWebDeployment() && GetFocus() != hWnd)
		{
			HWND phwnd = GetParent(hWnd);
			while (phwnd)
			{
				if (GetForegroundWindow() == phwnd)
				{
					SetFocus(hWnd);
					break;
				}
				phwnd = GetParent(phwnd);
			}
		}
		break;

		// Associate the window pointer with this window
	case WM_CREATE:
		// CodeReview [tom, 4/30/2007] Why don't we just cast this to a LONG 
		//            instead of having a ton of essentially pointless casts ?
		SetWindowLongPtr(hWnd, GWLP_USERDATA,
			(LONG_PTR)((PlatformWindow*)((CREATESTRUCT*)lParam)->lpCreateParams));
		break;

	case WM_SETFOCUS:
		// NOTE: if wParam is NOT equal to our window handle then we are GAINING focus
		Dispatch(DelayedDispatch, hWnd, message, wParam, lParam);
		return 0;

	case WM_KILLFOCUS:
		// NOTE: if wParam is NOT equal to our window handle then we are LOSING focus
		Dispatch(DelayedDispatch, hWnd, message, wParam, lParam);
		return 0;

		// The window is being dragged
	case WM_MOVE:
		if(!window)
			break;

		window->mPosition.x = (int)LOWORD(lParam);
		window->mPosition.y = (int)HIWORD(lParam);
		return 0;

		// Update viewport when the window moves
	case WM_SIZE:
		if(window && window->mSuppressReset)
			break;

		// This is dispatched immediately to prevent a race condition with journaling and window minimizing
		if (wParam != SIZE_MINIMIZED && !Journal::IsPlaying()) 
			Dispatch( ImmediateDispatch, hWnd,message,wParam,lParam );

		if(wParam != SIZE_MINIMIZED && window != NULL )
		{
			if(!window->mVideoMode.fullScreen)
			{
				U32 width = LOWORD( lParam );
				U32 height = HIWORD( lParam );

				window->mVideoMode.resolution.set( width, height );
			}

			if(window->getGFXTarget())
			{
				Con::warnf("Win32Window::WindowProc - resetting device due to window size change.");
				window->getGFXTarget()->resetMode();
			}

         window->getScreenResChangeSignal().trigger(window, true);
		}
		return 0;

		// Limit resize to a safe minimum
	case WM_GETMINMAXINFO:
		MINMAXINFO *winfo;
		winfo = (MINMAXINFO*)(lParam);
		
		if(window && window->mMinimumSize.lenSquared() > 0)
		{
			winfo->ptMinTrackSize.x = window->mMinimumSize.x;
			winfo->ptMinTrackSize.y = window->mMinimumSize.y;
		}		

		//Is the window size locked?
		if (window && window->isSizeLocked())
		{
			Point2I lockedSize = window->getLockedSize();

			winfo->ptMinTrackSize.x = lockedSize.x;
			winfo->ptMinTrackSize.y = lockedSize.y;
			winfo->ptMaxTrackSize.x = lockedSize.x;
			winfo->ptMaxTrackSize.y = lockedSize.y;
		}

		break;

		// Override background erase so window doesn't get cleared
	case WM_ERASEBKGND:
		return 1;

	case WM_MENUSELECT:
		winState.renderThreadBlocked = true;
		break;

		// Refresh the window
	case WM_PAINT:
		// Use validate instead of begin/end paint, which seem to installs
		// some Dx clipping state that isn't getting restored properly
		ValidateRect(hWnd,0);

		// Skip it if we're dispatching.
		if(Journal::IsDispatching())
			break;

		if( window == NULL )
			break;

		//// Default render if..
		//// 1. We have no device
		//// 2. We have a device but it's not allowing rendering
		if( !window->getGFXDevice() || !window->getGFXDevice()->allowRender() )
			window->defaultRender();
		if( winState.renderThreadBlocked )
			window->displayEvent.trigger(devId);
		break;

		// Power shutdown query
	case WM_POWERBROADCAST: {
		if (wParam == PBT_APMQUERYSUSPEND)
			if (GetForegroundWindow() == hWnd)
				return BROADCAST_QUERY_DENY;
		break;
							}

							// Screensaver activation and monitor power requests
	case WM_SYSCOMMAND:
		switch (wParam) {
	case SC_SCREENSAVE:
	case SC_MONITORPOWER:
		if (GetForegroundWindow() == hWnd)
			return SCREENSAVER_QUERY_DENY;
		break;
		}
		break;

		// Menus
	case WM_COMMAND:
		{
			winState.renderThreadBlocked = false;

			if( window == NULL )
				break;

			// [tom, 8/21/2006] Pass off to the relevant PopupMenu if it's a menu
			// or accelerator command. PopupMenu will in turn hand off to script.
			//
			// Note: PopupMenu::handleSelect() will not do anything if the menu
			// item is disabled, so we don't need to deal with that here.

			S32 numItems = GetMenuItemCount(window->getMenuHandle());
			for(S32 i = 0;i < numItems;i++)
			{
				MENUITEMINFOA mi;
				mi.cbSize = sizeof(mi);
				mi.fMask = MIIM_DATA;
				if(GetMenuItemInfoA(window->getMenuHandle(), i, TRUE, &mi))
				{
					if(mi.fMask & MIIM_DATA && mi.dwItemData != 0)
					{
						PopupMenu *mnu = (PopupMenu *)mi.dwItemData;

						PopupMenu::smSelectionEventHandled = false;
						PopupMenu::smPopupMenuEvent.trigger(mnu->getPopupGUID(), LOWORD(wParam));
						if (PopupMenu::smSelectionEventHandled)
							return 0;
					}
				}
			}
		}
		break;

	case WM_INITMENUPOPUP:
		{
			HMENU menu = (HMENU)wParam;
			MENUINFO mi;
			mi.cbSize = sizeof(mi);
			mi.fMask = MIM_MENUDATA;
			if(GetMenuInfo(menu, &mi) && mi.dwMenuData != 0)
			{
				PopupMenu *pm = (PopupMenu *)mi.dwMenuData;
				if(pm != NULL)
					pm->onMenuSelect();
			}
		}
		break;
		// Some events need to be consumed as well as queued up
		// for later dispatch.
	case WM_CLOSE:
	case WM_MOUSEWHEEL:
#ifdef WM_MOUSEHWHEEL // Vista
   case WM_MOUSEHWHEEL:
#endif

		// CodeReview This fixes some issues with inappropriate event handling
		//            around device resets and in full-screen mode but feels 
		//            heavy-handed. Is it clobbering something important?
		//            [bjg 6/13/07]
	case WM_KEYUP:
	case WM_KEYDOWN:
	case WM_SYSKEYUP:
	case WM_SYSKEYDOWN:
		Dispatch(DelayedDispatch,hWnd,message,wParam,lParam);
		return 0;
	}

	// Queue up for later and invoke the Windows default handler.
	Dispatch(DelayedDispatch,hWnd,message,wParam,lParam);
	return DefWindowProc(hWnd, message, wParam, lParam);
}


void Win32Window::defaultRender()
{
	// Get Window Device Context
	HDC logoDC = GetDC(mWindowHandle);

	// Get Window Rectangle
	RECT lRect;
	GetClientRect(mWindowHandle,&lRect);

	// Fill with AppWorkspace color
	FillRect( logoDC, &lRect, (HBRUSH)GetSysColorBrush(COLOR_APPWORKSPACE) );

	// Release Device Context
	ReleaseDC(mWindowHandle,logoDC);
}

//-----------------------------------------------------------------------------
// Accelerators
//-----------------------------------------------------------------------------

void Win32Window::addAccelerator(Accelerator &accel)
{
	ACCEL winAccel;
	winAccel.fVirt = FVIRTKEY;
	winAccel.cmd = accel.mID;

	if(accel.mDescriptor.flags & SI_SHIFT)
		winAccel.fVirt |= FSHIFT;
	if(accel.mDescriptor.flags & SI_CTRL)
		winAccel.fVirt |= FCONTROL;
	if(accel.mDescriptor.flags & SI_ALT)
		winAccel.fVirt |= FALT;

	winAccel.key = TranslateKeyCodeToOS(accel.mDescriptor.eventCode);

	for(WinAccelList::iterator i = mWinAccelList.begin();i != mWinAccelList.end();++i)
	{
		if(i->cmd == winAccel.cmd)
		{
			// Already in list, just update it
			i->fVirt = winAccel.fVirt;
			i->key = winAccel.key;
			return;
		}

		if(i->fVirt == winAccel.fVirt && i->key == winAccel.key)
		{
			// Existing accelerator in list, don't add this one
			return;
		}
	}

	mWinAccelList.push_back(winAccel);
}

void Win32Window::removeAccelerator(Accelerator &accel)
{
	for(WinAccelList::iterator i = mWinAccelList.begin();i != mWinAccelList.end();++i)
	{
		if(i->cmd == accel.mID)
		{
			mWinAccelList.erase(i);
			return;
		}
	}
}

//-----------------------------------------------------------------------------

static bool isMenuItemIDEnabled(HMENU menu, U32 id)
{
	S32 numItems = GetMenuItemCount(menu);
	for(S32 i = 0;i < numItems;i++)
	{
		MENUITEMINFOA mi;
		mi.cbSize = sizeof(mi);
		mi.fMask = MIIM_ID|MIIM_STATE|MIIM_SUBMENU|MIIM_DATA;
		if(GetMenuItemInfoA(menu, i, TRUE, &mi))
		{
			if(mi.fMask & MIIM_ID && mi.wID == id)
			{
				// This is an item on this menu
				return (mi.fMask & MIIM_STATE) && ! (mi.fState & MFS_DISABLED);
			}

			if((mi.fMask & MIIM_SUBMENU) && mi.hSubMenu != 0 && (mi.fMask & MIIM_DATA) && mi.dwItemData != 0)
			{
				// This is a submenu, if it can handle this ID then recurse to find correct state
				PopupMenu *mnu = (PopupMenu *)mi.dwItemData;
				if(mnu->canHandleID(id))
					return isMenuItemIDEnabled(mi.hSubMenu, id);
			}
		}
	}

	return false;
}

bool Win32Window::isAccelerator(const InputEventInfo &info)
{
	U32 virt;
	virt = FVIRTKEY;
	if(info.modifier & SI_SHIFT)
		virt |= FSHIFT;
	if(info.modifier & SI_CTRL)
		virt |= FCONTROL;
	if(info.modifier & SI_ALT)
		virt |= FALT;

	U8 keyCode = TranslateKeyCodeToOS(info.objInst);

	for(S32 i = 0;i < mWinAccelList.size();++i)
	{
		const ACCEL &accel = mWinAccelList[i];
		if(accel.key == keyCode && accel.fVirt == virt && isMenuItemIDEnabled(getMenuHandle(), accel.cmd))
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------

void Win32Window::addAccelerators(AcceleratorList &list)
{
	if(mAccelHandle)
	{
		DestroyAcceleratorTable(mAccelHandle);
		mAccelHandle = NULL;
	}

	for(AcceleratorList::iterator i = list.begin();i != list.end();++i)
	{
		addAccelerator(*i);
	}

	if(mWinAccelList.size() > 0)
		mAccelHandle = CreateAcceleratorTable(&mWinAccelList[0], mWinAccelList.size());
}

void Win32Window::removeAccelerators(AcceleratorList &list)
{
	if(mAccelHandle)
	{
		DestroyAcceleratorTable(mAccelHandle);
		mAccelHandle = NULL;
	}

	for(AcceleratorList::iterator i = list.begin();i != list.end();++i)
	{
		removeAccelerator(*i);
	}

	if(mWinAccelList.size() > 0)
		mAccelHandle = CreateAcceleratorTable(mWinAccelList.address(), mWinAccelList.size());
}

bool Win32Window::translateMessage(MSG &msg)
{
	if(mAccelHandle == NULL || mWindowHandle == NULL || !mEnableAccelerators)
		return false;

	S32 ret = TranslateAccelerator(mWindowHandle, mAccelHandle, &msg);
	return ret != 0;
}

//-----------------------------------------------------------------------------
// Mouse Locking
//-----------------------------------------------------------------------------

void Win32Window::setMouseLocked( bool enable )
{

   if (mOffscreenRender)
      return;

	// Maintain a good state without unnecessary 
	//  cursor hides/modifications
	if( enable && mMouseLocked && mShouldLockMouse )
		return;
	else if(!enable && !mMouseLocked && !mShouldLockMouse )
		return;

	// Need to be focused to enable mouse lock
	// but we can disable it no problem if we're 
	// not focused
	if( !isFocused() && enable )
	{
		mShouldLockMouse = enable;
		return;
	}

	// Set Flag
	mMouseLocked = enable;

	if( enable )
	{
		getCursorPosition( mMouseLockPosition );

		RECT r;
		GetWindowRect(getHWND(), &r);

		// Hide the cursor before it's moved
		setCursorVisible( false );

		// We have to nudge the cursor clip rect in a bit so we don't go out
		// side the bounds of the window... We'll just do it by 32 in all
		// directions, which will break for very small windows (< 200x200 or so)
		// but otherwise won't matter.
		RECT rCopy = r;
		rCopy.top  += 32; rCopy.bottom -= 64;
		rCopy.left += 32; rCopy.right  -= 64;
		ClipCursor(&rCopy);

		S32 centerX = (r.right + r.left) >> 1;
		S32 centerY = ((r.bottom + r.top) >> 1);


		// Consume all existing mouse events and those posted to our own dispatch queue
		MSG msg;
		PeekMessage( &msg, 0,WM_MOUSEFIRST,WM_MOUSELAST , PM_QS_POSTMESSAGE | PM_NOYIELD | PM_REMOVE );
		RemoveMessages( NULL, WM_MOUSEMOVE, WM_MOUSEMOVE );

		// Set the CursorPos
		SetCursorPos(centerX, centerY);

		// reset should lock flag
		mShouldLockMouse = true;
	}
	else
	{
		// This belongs before the unlock code
		mShouldLockMouse = false;

		ClipCursor(NULL);
		setCursorPosition( mMouseLockPosition.x,mMouseLockPosition.y );

		// Consume all existing mouse events and those posted to our own dispatch queue
		MSG msg;
		PeekMessage( &msg, NULL,WM_MOUSEFIRST,WM_MOUSELAST , PM_QS_POSTMESSAGE | PM_NOYIELD | PM_REMOVE );
		RemoveMessages( NULL, WM_MOUSEMOVE, WM_MOUSEMOVE );

		// Show the Cursor
		setCursorVisible( true );

	}
}

const UTF16 *Win32Window::getWindowClassName()
{
	return _MainWindowClassName;
}

const UTF16 *Win32Window::getCurtainWindowClassName()
{
	return _CurtainWindowClassName;
}

#endif
