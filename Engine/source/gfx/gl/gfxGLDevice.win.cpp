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
#include "gfx/gfxCubemap.h"
#include "gfx/screenshot.h"

#include "gfx/GL/gfxGLDevice.h"
#include "gfx/GL/gfxGLEnumTranslate.h"
#include "gfx/GL/gfxGLVertexBuffer.h"
#include "gfx/GL/gfxGLPrimitiveBuffer.h"
#include "gfx/gl/gfxGLTextureTarget.h"
#include "gfx/GL/gfxGLWindowTarget.h"
#include "gfx/GL/gfxGLTextureManager.h"
#include "gfx/GL/gfxGLTextureObject.h"
#include "gfx/GL/gfxGLCubemap.h"
#include "gfx/GL/gfxGLCardProfiler.h"
#include "windowManager/win32/win32Window.h"
#include "ggl/Win32/wgl.h"

#define GETHWND(x) static_cast<Win32Window*>(x)->getHWND()

// yonked from winWindow.cc
void CreatePixelFormat( PIXELFORMATDESCRIPTOR *pPFD, S32 colorBits, S32 depthBits, S32 stencilBits, bool stereo )
{
   PIXELFORMATDESCRIPTOR src =
   {
      sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd
      1,                      // version number
      PFD_DRAW_TO_WINDOW |    // support window
      PFD_SUPPORT_OPENGL |    // support OpenGL
      PFD_DOUBLEBUFFER,       // double buffered
      PFD_TYPE_RGBA,          // RGBA type
      colorBits,              // color depth
      0, 0, 0, 0, 0, 0,       // color bits ignored
      0,                      // no alpha buffer
      0,                      // shift bit ignored
      0,                      // no accumulation buffer
      0, 0, 0, 0,             // accum bits ignored
      depthBits,              // z-buffer
      stencilBits,            // stencil buffer
      0,                      // no auxiliary buffer
      PFD_MAIN_PLANE,         // main layer
      0,                      // reserved
      0, 0, 0                 // layer masks ignored
    };

   if ( stereo )
   {
      //ri.Printf( PRINT_ALL, "...attempting to use stereo\n" );
      src.dwFlags |= PFD_STEREO;
      //glConfig.stereoEnabled = true;
   }
   else
   {
      //glConfig.stereoEnabled = qfalse;
   }
   *pPFD = src;
}

extern void loadGLCore();
extern void loadGLExtensions(void* context);

void GFXGLDevice::enumerateAdapters( Vector<GFXAdapter*> &adapterList )
{
//   GL_ERROR_CHECK();
   WNDCLASS windowclass;
   dMemset( &windowclass, 0, sizeof( WNDCLASS ) );

   windowclass.lpszClassName = L"GFX-OpenGL";
   windowclass.style         = CS_OWNDC;
   windowclass.lpfnWndProc   = DefWindowProc;
   windowclass.hInstance     = winState.appInstance;

   if( !RegisterClass( &windowclass ) )
      AssertFatal( false, "Failed to register the window class for the GL test window." );

   // Now create a window
   HWND hwnd = CreateWindow( L"GFX-OpenGL", L"", WS_POPUP, 0, 0, 640, 480, 
                             NULL, NULL, winState.appInstance, NULL );
   AssertFatal( hwnd != NULL, "Failed to create the window for the GL test window." );

   // Create a device context
   HDC tempDC = GetDC( hwnd );
   AssertFatal( tempDC != NULL, "Failed to create device context" );

   // Create pixel format descriptor...
   PIXELFORMATDESCRIPTOR pfd;
   CreatePixelFormat( &pfd, 16, 16, 8, false ); // 16 bit color, 16 bit depth, 8 bit stencil...everyone can do this
   if( !SetPixelFormat( tempDC, ChoosePixelFormat( tempDC, &pfd ), &pfd ) )
      AssertFatal( false, "I don't know who's responcible for this, but I want caught..." );

   // Create a rendering context!
   HGLRC tempGLRC = wglCreateContext( tempDC );
   if( !wglMakeCurrent( tempDC, tempGLRC ) )
      AssertFatal( false, "I want them caught and killed." );

   // Add the GL renderer
   loadGLCore();
   loadGLExtensions(tempDC);

   GFXAdapter *toAdd = new GFXAdapter;
   toAdd->mIndex = 0;

   const char* renderer = (const char*) glGetString( GL_RENDERER );
   AssertFatal( renderer != NULL, "GL_RENDERER returned NULL!" );

   if (renderer)
   {
      dStrcpy(toAdd->mName, renderer);
      dStrncat(toAdd->mName, " OpenGL", GFXAdapter::MaxAdapterNameLen);
   }
   else
      dStrcpy(toAdd->mName, "OpenGL");

   toAdd->mType = OpenGL;
   toAdd->mShaderModel = 0.f;
   toAdd->mCreateDeviceInstanceDelegate = mCreateDeviceInstance;

   // Enumerate all available resolutions:
   DEVMODE devMode;
   U32 modeNum = 0;
   U32 stillGoing = true;
   while ( stillGoing )
   {
      dMemset( &devMode, 0, sizeof( devMode ) );
      devMode.dmSize = sizeof( devMode );

      stillGoing = EnumDisplaySettings( NULL, modeNum++, &devMode );

      if (( devMode.dmPelsWidth >= 480) && (devMode.dmPelsHeight >= 360 )
         && ( devMode.dmBitsPerPel == 16 || devMode.dmBitsPerPel == 32 )) 
      {
         GFXVideoMode vmAdd;

         vmAdd.bitDepth     = devMode.dmBitsPerPel;
         vmAdd.fullScreen   = true;
         vmAdd.refreshRate  = devMode.dmDisplayFrequency;
         vmAdd.resolution.x = devMode.dmPelsWidth;
         vmAdd.resolution.y = devMode.dmPelsHeight;

         // Only add this resolution if it is not already in the list:
         bool alreadyInList = false;
         for (Vector<GFXVideoMode>::iterator i = toAdd->mAvailableModes.begin(); i != toAdd->mAvailableModes.end(); i++)
         {
            if (vmAdd == *i)
            {
               alreadyInList = true;
               break;
            }
         }

         if(alreadyInList)
            continue;

         toAdd->mAvailableModes.push_back( vmAdd );
      }
   }

   // Add to the list of available adapters.
   adapterList.push_back(toAdd);

   // Cleanup our window
   wglMakeCurrent(NULL, NULL);
   wglDeleteContext(tempGLRC);
   ReleaseDC(hwnd, tempDC);
   DestroyWindow(hwnd);
   UnregisterClass(L"GFX-OpenGL", winState.appInstance);
}

void GFXGLDevice::enumerateVideoModes() 
{
   mVideoModes.clear();

   // Enumerate all available resolutions:
   DEVMODE devMode;
   U32 modeNum = 0;
   U32 stillGoing = true;
   while ( stillGoing )
   {
      dMemset( &devMode, 0, sizeof( devMode ) );
      devMode.dmSize = sizeof( devMode );

      stillGoing = EnumDisplaySettings( NULL, modeNum++, &devMode );

      if (( devMode.dmPelsWidth >= 480) && (devMode.dmPelsHeight >= 360 )
           && ( devMode.dmBitsPerPel == 16 || devMode.dmBitsPerPel == 32 )) 
           //( smCanSwitchBitDepth || devMode.dmBitsPerPel == winState.desktopBitsPixel ) )
      {
         GFXVideoMode toAdd;

         toAdd.bitDepth = devMode.dmBitsPerPel;
         toAdd.fullScreen = false;
         toAdd.refreshRate = devMode.dmDisplayFrequency;
         toAdd.resolution.x = devMode.dmPelsWidth;
         toAdd.resolution.y = devMode.dmPelsHeight;

         // Only add this resolution if it is not already in the list:
         bool alreadyInList = false;
         for (Vector<GFXVideoMode>::iterator i = mVideoModes.begin(); i != mVideoModes.end(); i++)
         {
            if (toAdd == *i)
            {
               alreadyInList = true;
               break;
            }
         }
         if ( !alreadyInList )
         {
            //Con::printf("Resolution: %dx%d %d bpp %d Refresh rate: %d", toAdd.resolution.x, toAdd.resolution.y, toAdd.bitDepth, toAdd.refreshRate);
            mVideoModes.push_back( toAdd );
         }
      }
   }
}

void GFXGLDevice::init( const GFXVideoMode &mode, PlatformWindow *window )
{
   AssertFatal(window, "GFXGLDevice::init - no window specified, can't init device without a window!");
   AssertFatal(dynamic_cast<Win32Window*>(window), "Invalid window class type!");
   HWND hwnd = GETHWND(window);

   RECT rect;
   GetClientRect(hwnd, &rect);

   Point2I resolution;
   resolution.x = rect.right - rect.left;
   resolution.y = rect.bottom - rect.top;

   // Create a device context
   HDC hdcGL = GetDC( hwnd );
   AssertFatal( hdcGL != NULL, "Failed to create device context" );

   // Create pixel format descriptor...
   PIXELFORMATDESCRIPTOR pfd;
   CreatePixelFormat( &pfd, 16, 16, 8, false ); // 16 bit color, 16 bit depth, 8 bit stencil...everyone can do this
   if( !SetPixelFormat( hdcGL, ChoosePixelFormat( hdcGL, &pfd ), &pfd ) )
   {
      AssertFatal( false, "GFXGLDevice::init - cannot get the one and only pixel format we check for." );
   }

   // Create a rendering context!
   mContext = wglCreateContext( hdcGL );
   if( !wglMakeCurrent( hdcGL, (HGLRC)mContext ) )
      AssertFatal( false , "GFXGLDevice::init - cannot make our context current. Or maybe we can't create it." );

   loadGLCore();
   loadGLExtensions(hdcGL);

   wglSwapIntervalEXT(0);
   
   // It is very important that extensions be loaded
   // before we call initGLState()
   initGLState();

   mProjectionMatrix.identity();

   mInitialized = true;
   deviceInited();
}

bool GFXGLDevice::beginSceneInternal() 
{
   glGetError();
   return true;
}

U32 GFXGLDevice::getTotalVideoMemory()
{
   // CodeReview [ags 12/21/07] Figure out how to do this.
   return 0;
}

//------------------------------------------------------------------------------

GFXWindowTarget *GFXGLDevice::allocWindowTarget( PlatformWindow *window )
{
   HDC hdcGL = GetDC(GETHWND(window));

   if(!mContext)
   {
      init(window->getVideoMode(), window);
      GFXGLWindowTarget *ggwt = new GFXGLWindowTarget(window, this);
      ggwt->registerResourceWithDevice(this);
      ggwt->mContext = wglCreateContext(hdcGL);
      AssertFatal(ggwt->mContext, "GFXGLDevice::allocWindowTarget - failed to allocate window target!");

      return ggwt;
   }

   GFXGLWindowTarget *ggwt = new GFXGLWindowTarget(window, this);
   ggwt->registerResourceWithDevice(this);

   // Create pixel format descriptor...
   PIXELFORMATDESCRIPTOR pfd;
   CreatePixelFormat( &pfd, 16, 16, 8, false ); // 16 bit color, 16 bit depth, 8 bit stencil...everyone can do this
   if( !SetPixelFormat( hdcGL, ChoosePixelFormat( hdcGL, &pfd ), &pfd ) )
   {
      AssertFatal( false, "GFXGLDevice::allocWindowTarget - cannot get the one and only pixel format we check for." );
   }

   ggwt->mContext = wglCreateContext(hdcGL);
   DWORD w = GetLastError();
   AssertFatal(ggwt->mContext, "GFXGLDevice::allocWindowTarget - failed to allocate window target!");

   wglMakeCurrent(NULL, NULL);
   bool res = wglShareLists((HGLRC)mContext, (HGLRC)ggwt->mContext);
   w = GetLastError();

   wglMakeCurrent(hdcGL, (HGLRC)ggwt->mContext);
   AssertFatal(res, "GFXGLDevice::allocWindowTarget - wasn't able to share contexts!");

   return ggwt;
}

void GFXGLDevice::_updateRenderTargets()
{
   if ( mRTDirty || mCurrentRT->isPendingState() )
   {
      // GL doesn't need to deactivate targets.
      mRTDeactivate = NULL;

      // NOTE: The render target changes is not really accurate
      // as the GFXTextureTarget supports MRT internally.  So when
      // we activate a GFXTarget it could result in multiple calls
      // to SetRenderTarget on the actual device.
      mDeviceStatistics.mRenderTargetChanges++;

      GFXGLTextureTarget *tex = dynamic_cast<GFXGLTextureTarget*>( mCurrentRT.getPointer() );
      if ( tex )
      {
         tex->applyState();
         tex->makeActive();
      }
      else
      {
         GFXGLWindowTarget *win = dynamic_cast<GFXGLWindowTarget*>( mCurrentRT.getPointer() );
         AssertFatal( win != NULL, 
            "GFXGLDevice::_updateRenderTargets() - invalid target subclass passed!" );

         //DWORD w1 = GetLastError();
         HWND hwnd = GETHWND(win->getWindow());
         HDC winDc = GetDC(hwnd);      
         bool res = wglMakeCurrent(winDc,(HGLRC)win->mContext);
         //DWORD w2 = GetLastError();
         AssertFatal(res==true,"GFXGLDevice::setActiveRenderTarget - failed");
      }

      mRTDirty = false;
   }

   if ( mViewportDirty )
   {
      glViewport( mViewport.point.x, mViewport.point.y, mViewport.extent.x, mViewport.extent.y ); 
      mViewportDirty = false;
   }
}

GFXFence* GFXGLDevice::_createPlatformSpecificFence()
{
   return NULL;
}


//-----------------------------------------------------------------------------

void GFXGLWindowTarget::makeActive()
{
}

bool GFXGLWindowTarget::present()
{
   HWND hwnd = GETHWND(getWindow());
   SwapBuffers(GetDC(hwnd));
   return true;
}

void GFXGLWindowTarget::_teardownCurrentMode()
{
}

void GFXGLWindowTarget::_setupNewMode()
{
}
