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

#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <tchar.h>

#include "platform/platform.h"
#include "console/console.h"

// from Torque.rc
#define IDI_ICON1 103

// Window Class name
static const TCHAR* c_szSplashClass = L"Torque3DSplashWindow";

static HWND gSplashWndOwner = NULL;
static HWND gSplashWnd = NULL;
static HBITMAP gSplashImage = NULL;

// Registers a window class for the splash and splash owner windows.
static void RegisterWindowClass(HINSTANCE hinst)

{
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = DefWindowProc;
	wc.hInstance = hinst;
	wc.hIcon = LoadIcon(hinst, MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = c_szSplashClass;
	RegisterClass(&wc);
}

static void UnregisterSplashWindowClass(HINSTANCE hinst)
{
	WNDCLASSEX classInfo;
	if (GetClassInfoEx(hinst,c_szSplashClass,&classInfo))
		UnregisterClass(c_szSplashClass,hinst);
}

// Creates the splash owner window and the splash window.
static HWND CreateSplashWindow(HINSTANCE hinst)

{
	RegisterWindowClass(hinst);

	gSplashWndOwner = CreateWindow(c_szSplashClass, NULL, WS_POPUP,
		0, 0, 0, 0, NULL, NULL, hinst, NULL);

	return CreateWindowEx(WS_EX_LAYERED, c_szSplashClass, NULL, WS_POPUP | WS_VISIBLE,
		0, 0, 0, 0, gSplashWndOwner, NULL, hinst, NULL);
}

// Calls UpdateLayeredWindow to set a bitmap (with alpha) as the content of the splash window.
static void SetSplashImage(HWND hwndSplash, HBITMAP hbmpSplash)

{
	// get the size of the bitmap
	BITMAP bm;
	GetObject(hbmpSplash, sizeof(bm), &bm);
	SIZE sizeSplash = { bm.bmWidth, bm.bmHeight };

	// get the primary monitor's info
	POINT ptZero = { 0 };
	HMONITOR hmonPrimary = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO monitorinfo = { 0 };
	monitorinfo.cbSize = sizeof(monitorinfo);
	GetMonitorInfo(hmonPrimary, &monitorinfo);

	// center the splash screen in the middle of the primary work area
	const RECT & rcWork = monitorinfo.rcWork;
	POINT ptOrigin;
	ptOrigin.x = rcWork.left + (rcWork.right - rcWork.left - sizeSplash.cx) / 2;
	ptOrigin.y = rcWork.top + (rcWork.bottom - rcWork.top - sizeSplash.cy) / 2;

	// create a memory DC holding the splash bitmap
	HDC hdcScreen = GetDC(NULL);
	HDC hdcMem = CreateCompatibleDC(hdcScreen);
	HBITMAP hbmpOld = (HBITMAP) SelectObject(hdcMem, hbmpSplash);

	// paint the window (in the right location) with the alpha-blended bitmap
	UpdateLayeredWindow(hwndSplash, hdcScreen, &ptOrigin, &sizeSplash,
		hdcMem, &ptZero, RGB(0, 0, 0), NULL, ULW_OPAQUE);

	// delete temporary objects
	SelectObject(hdcMem, hbmpOld);
	DeleteDC(hdcMem);
	ReleaseDC(NULL, hdcScreen);
}

void CloseSplashWindow(HINSTANCE hinst)
{
	if (gSplashWndOwner)
	{
		//ShowWindow(gSplashWnd, 0);	
		DestroyWindow(gSplashWndOwner);
		UnregisterSplashWindowClass(hinst);
	}

	gSplashWndOwner = NULL;
	gSplashWnd = NULL;
	
}

#ifndef TORQUE_SDL

bool Platform::closeSplashWindow()
{
   CloseSplashWindow(GetModuleHandle(NULL));

   return true;
}

bool Platform::displaySplashWindow( String path )
{
   if(path.isEmpty())
      return false;

#ifdef UNICODE
   const UTF16 *lFileName = path.utf16();
#else
   const UTF8  *lFileName = path.c_str();
#endif

	gSplashImage = (HBITMAP) ::LoadImage(0, lFileName, 
		IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

	if (!gSplashImage)
		return false;

	gSplashWnd = CreateSplashWindow(GetModuleHandle(NULL));

	if (!gSplashWnd)
		return false;

	SetSplashImage(gSplashWnd, gSplashImage);

	return true;
}

#endif
