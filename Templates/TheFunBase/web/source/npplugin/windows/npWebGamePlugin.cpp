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

#include <string>
#include <vector>

#include "npWebGamePlugin.h"
#include "../../common/webCommon.h"

NPWebGamePlugin* NPWebGamePlugin::sInstance = NULL; 


// we use a timer to update the Torque 3D game loop (tick) and handle rendering
VOID CALLBACK MyTimerProc( HWND hwnd,        // handle to window for timer messages 
                          UINT message,     // WM_TIMER message 
                          UINT idTimer,     // timer identifier 
                          DWORD dwTime)     // current system time 
{ 
   static bool reentrant = false;

   if (!reentrant)
   {
      reentrant = true;
      torque_enginetick();
      reentrant = false;
   }
} 

// custom window proc for our plugin's rendering window
static LRESULT CALLBACK NPWebGamePluginWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   NPWebGamePlugin* plugin = (NPWebGamePlugin*)GetWindowLongPtr(hWnd, GWL_USERDATA);
   if (plugin)
   {
      switch (msg)
      {   
      case WM_MOUSEACTIVATE:
         break;
      case WM_SIZE:
         // handle resize of browser (sub)window updating our Torque 3D child window accordingly
         int width = (int) LOWORD( lParam );
         int height = (int) HIWORD( lParam );
         torque_resizewindow(width,height);

         break;
      }

      return CallWindowProc((WNDPROC)plugin->mOriginalWinProc, hWnd, msg, wParam, lParam);
   }
   else
   {
      return DefWindowProc(hWnd, msg, wParam, lParam);
   }
}

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
   WebCommon::gPluginModule = (HMODULE) hInstance;
   return TRUE; 
}

NPWebGamePlugin::NPWebGamePlugin(NPP aInstance)
{

   mOpen = FALSE;
   mInstance = aInstance;
   sInstance = this;

   mOriginalWinProc = NULL;
   mHwnd = NULL;

}

NPWebGamePlugin::~NPWebGamePlugin()
{
   Close();
   sInstance = NULL;
}


NPBool NPWebGamePlugin::Open(NPWindow* aWindow)
{
   if (mOpen)
   {
      return TRUE; //firefox tries to open 2x
   }

   if (!aWindow)
      return FALSE;

   void* platformWindow = NULL;

   mHwnd = (HWND)aWindow->window;

   if (!mHwnd)
      return FALSE;

   platformWindow = mHwnd;

   // replace our plugin window proc with a custom one (for handling resizing,etc)
   mOriginalWinProc = SetWindowLongPtr(mHwnd, GWLP_WNDPROC, (LONG_PTR)NPWebGamePluginWinProc);

   LONG lStyle = GetWindowLong(mHwnd, GWL_STYLE);
   SetWindowLong(mHwnd, GWL_STYLE, lStyle | WS_CLIPCHILDREN);

   SetWindowLongPtr(mHwnd, GWL_USERDATA, (LONG_PTR)this);


   // load up the Torque 3D shared library and initialize it
   if (!WebCommon::InitTorque3D(platformWindow))
      return false;

   mOpen = true;

   // fire up our tick/update timer
   SetTimer( mHwnd, 1,              // timer identifier 
      1,                            // 1 millisecond
      (TIMERPROC) MyTimerProc);     // timer callback 

   return mOpen;
}

void NPWebGamePlugin::Close()
{

   if (!mOpen)
      return;

   if (mOriginalWinProc)
   {
      // restore original window proc
      SetWindowLongPtr(mHwnd, GWLP_WNDPROC, mOriginalWinProc);
      mOriginalWinProc = NULL;
   }

   if (mHwnd)
   {
      // no more ticks please
      KillTimer( mHwnd, 1); 
   }

   mHwnd = NULL;

   // shutdown and unload the Torque 3D DLL
   WebCommon::ShutdownTorque3D();

   mOpen = false;
}

NPBool NPWebGamePlugin::IsOpen()
{
   return mOpen;
}
