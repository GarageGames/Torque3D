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
#if defined( TORQUE_SDL ) && !defined( TORQUE_DEDICATED )

#include "gfx/gfxCubemap.h"
#include "gfx/screenshot.h"

#include "gfx/gl/gfxGLDevice.h"
#include "gfx/gl/gfxGLEnumTranslate.h"
#include "gfx/gl/gfxGLVertexBuffer.h"
#include "gfx/gl/gfxGLPrimitiveBuffer.h"
#include "gfx/gl/gfxGLTextureTarget.h"
#include "gfx/gl/gfxGLWindowTarget.h"
#include "gfx/gl/gfxGLTextureManager.h"
#include "gfx/gl/gfxGLTextureObject.h"
#include "gfx/gl/gfxGLCubemap.h"
#include "gfx/gl/gfxGLCardProfiler.h"

#include "windowManager/sdl/sdlWindow.h"
#include "platform/platformGL.h"
#include "SDL.h"

extern void loadGLCore();
extern void loadGLExtensions(void* context);

void EnumerateVideoModes(Vector<GFXVideoMode>& outModes)
{
   int count = SDL_GetNumDisplayModes( 0 );
   if( count < 0)
   {
      AssertFatal(0, "");
      return;     
   }
   
   SDL_DisplayMode mode;
   for(int i = 0; i < count; ++i)
   {
      SDL_GetDisplayMode( 0, i, &mode);
      GFXVideoMode outMode;
      outMode.resolution.set( mode.w, mode.h );
      outMode.refreshRate = mode.refresh_rate;
      outMode.bitDepth = SDL_BYTESPERPIXEL( mode.format );
      outMode.wideScreen = (mode.w / mode.h) > (4 / 3);
      outMode.fullScreen = true;
      
      outModes.push_back( outMode );
   }
}

void GFXGLDevice::enumerateAdapters( Vector<GFXAdapter*> &adapterList )
{
   AssertFatal( SDL_WasInit(SDL_INIT_VIDEO), "");

   PlatformGL::init(); // for hints about context creation

    // Create a dummy window & openGL context so that gl functions can be used here
   SDL_Window* tempWindow =  SDL_CreateWindow(
        "",                                // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        640,                               // width, in pixels
        480,                               // height, in pixels
        SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN // flags - see below
    );

   SDL_ClearError();
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

   SDL_GLContext tempContext = SDL_GL_CreateContext( tempWindow );
   if( !tempContext )
   {
       const char *err = SDL_GetError();
       Con::printf( err );
       AssertFatal(0, err );
       return;
   }

   SDL_ClearError();
   SDL_GL_MakeCurrent( tempWindow, tempContext );

   const char *err = SDL_GetError();
   if( err && err[0] )
   {
       Con::printf( err );
       AssertFatal(0, err );
   }

   // Init GL
   loadGLCore();
   loadGLExtensions(tempContext);

   //check minimun Opengl 3.2
   int major, minor;
   glGetIntegerv(GL_MAJOR_VERSION, &major);
   glGetIntegerv(GL_MINOR_VERSION, &minor);
   if( major < 3 || ( major == 3 && minor < 2 ) )
   {
      return;
   }
    
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
   EnumerateVideoModes(toAdd->mAvailableModes);

   // Add to the list of available adapters.
   adapterList.push_back(toAdd);

   // Cleanup window & open gl context
   SDL_DestroyWindow( tempWindow );
   SDL_GL_DeleteContext( tempContext );
}

void GFXGLDevice::enumerateVideoModes() 
{
    mVideoModes.clear();
    EnumerateVideoModes(mVideoModes);
}

void GFXGLDevice::init( const GFXVideoMode &mode, PlatformWindow *window )
{
    AssertFatal(window, "GFXGLDevice::init - no window specified, can't init device without a window!");
    PlatformWindowSDL* sdlWindow = dynamic_cast<PlatformWindowSDL*>(window);
    AssertFatal(sdlWindow, "Window is not a valid PlatformWindowSDL object");

    // Create OpenGL context
    mContext = PlatformGL::CreateContextGL( sdlWindow );
    PlatformGL::MakeCurrentGL( sdlWindow, mContext );
        
    loadGLCore();
    loadGLExtensions(mContext);
    
    // It is very important that extensions be loaded before we call initGLState()
    initGLState();
    
    mProjectionMatrix.identity();
    
    mInitialized = true;
    deviceInited();
}

bool GFXGLDevice::beginSceneInternal() 
{
   mCanCurrentlyRender = true;
   return true;
}

U32 GFXGLDevice::getTotalVideoMemory()
{
   return getTotalVideoMemory_GL_EXT();
}

//------------------------------------------------------------------------------

GFXWindowTarget *GFXGLDevice::allocWindowTarget( PlatformWindow *window )
{
   GFXGLWindowTarget* ggwt = new GFXGLWindowTarget(window, this);

   //first window
   if (!mContext)
   {
      init(window->getVideoMode(), window);
      ggwt->mSecondaryWindow = false;
   }
   else
      ggwt->mSecondaryWindow = true;

   ggwt->registerResourceWithDevice(this);
   ggwt->mContext = mContext;

   return ggwt;
}

GFXFence* GFXGLDevice::_createPlatformSpecificFence()
{
    return NULL;
}


//-----------------------------------------------------------------------------

void GFXGLWindowTarget::_WindowPresent()
{   
   SDL_GL_SwapWindow( static_cast<PlatformWindowSDL*>( getWindow() )->getSDLWindow() );
}

void GFXGLWindowTarget::_teardownCurrentMode()
{

}

void GFXGLWindowTarget::_setupNewMode()
{
}

void GFXGLWindowTarget::_makeContextCurrent()
{
   PlatformGL::MakeCurrentGL(mWindow, mContext);
}

#endif
