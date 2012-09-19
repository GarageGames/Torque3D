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

// Don't include Apple's GL header
#define __gl_h_
// Include our GL header before Apple headers.
#include "gfx/gl/ggl/ggl.h"

#include "platform/tmm_off.h"
#include <Cocoa/Cocoa.h>
#include <OpenGL/OpenGL.h>
#include "gfx/gl/gfxGLDevice.h"
#include "platform/tmm_on.h"

#include "gfx/gl/gfxGLTextureTarget.h"
#include "gfx/gl/gfxGLCardProfiler.h"
#include "gfx/gl/gfxGLAppleFence.h"
#include "gfx/gl/gfxGLWindowTarget.h"
#include "platformMac/macGLUtils.h"
#include "windowManager/mac/macWindow.h"

extern void loadGLCore();
extern void loadGLExtensions(void* context);

static String _getRendererForDisplay(CGDirectDisplayID display)
{
   Vector<NSOpenGLPixelFormatAttribute> attributes = _createStandardPixelFormatAttributesForDisplay(display);
 
   NSOpenGLPixelFormat* fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes.address()];
   AssertFatal(fmt, "_getRendererForDisplay - Unable to create a pixel format object");
   
   NSOpenGLContext* ctx = [[NSOpenGLContext alloc] initWithFormat:fmt shareContext:nil];
   AssertFatal(ctx, "_getRendererForDisplay - Unable to create an OpenGL context");
   
   // Save the current context, just in case
   NSOpenGLContext* currCtx = [NSOpenGLContext currentContext];
   [ctx makeCurrentContext];
   
   // CodeReview [ags 12/19/07] This is a hack.  We should call loadGLCore somewhere else, before we reach here.  
   // On Macs we can safely assume access to the OpenGL framework at anytime, perhaps this should go in main()?
   loadGLCore();

   // get the renderer string
   String ret((const char*)glGetString(GL_RENDERER));
   
   // Restore our old context, release the context and pixel format.
   [currCtx makeCurrentContext];
   [ctx release];
   [fmt release];
   return ret;
}

static void _createInitialContextAndFormat(void* &ctx, void* &fmt)
{
   AssertFatal(!fmt && !ctx, "_createInitialContextAndFormat - Already created initial context and format");

   fmt = _createStandardPixelFormat();
   AssertFatal(fmt, "_createInitialContextAndFormat - Unable to create an OpenGL pixel format");

   ctx = [[NSOpenGLContext alloc] initWithFormat: (NSOpenGLPixelFormat*)fmt shareContext: nil];
   AssertFatal(ctx, "_createInitialContextAndFormat - Unable to create an OpenGL context");
}

static NSOpenGLContext* _createContextForWindow(PlatformWindow *window, void* &context, void* &pixelFormat)
{  
   NSOpenGLView* view = (NSOpenGLView*)window->getPlatformDrawable();
   AssertFatal([view isKindOfClass:[NSOpenGLView class]], avar("_createContextForWindow - Supplied a %s instead of a NSOpenGLView", [[view className] UTF8String]));

   NSOpenGLContext* ctx = NULL;
   if(!context || !pixelFormat)
   {
      // Create the initial opengl context that the device and the first window will hold.
      _createInitialContextAndFormat(context, pixelFormat);
      ctx = (NSOpenGLContext*)context;
   }
   else
   {
      // Create a context which shares its resources with the device's initial context
      ctx = [[NSOpenGLContext alloc] initWithFormat: (NSOpenGLPixelFormat*)pixelFormat shareContext: (NSOpenGLContext*)context];
      AssertFatal(ctx, "Unable to create a shared OpenGL context");
   }

   [view setPixelFormat: (NSOpenGLPixelFormat*)pixelFormat];
   [view setOpenGLContext: ctx];
      
   return ctx;
}

void GFXGLDevice::init( const GFXVideoMode &mode, PlatformWindow *window )
{
   if(mInitialized)
      return;
      
   NSOpenGLContext* ctx = _createContextForWindow(window, mContext, mPixelFormat);
   [ctx makeCurrentContext];
   
   loadGLCore();
   loadGLExtensions(ctx);
   
   initGLState();
   
   mInitialized = true;
   deviceInited();
}

void GFXGLDevice::enumerateAdapters( Vector<GFXAdapter*> &adapterList )
{
   GFXAdapter *toAdd;
   
   Vector<GFXVideoMode> videoModes;
   
   CGDirectDisplayID display = CGMainDisplayID();
   
   // Enumerate all available resolutions:
   NSArray* modeArray = (NSArray*)CGDisplayAvailableModes(display);
   
   GFXVideoMode vmAdd;   
   NSEnumerator* enumerator = [modeArray objectEnumerator];
   NSDictionary* mode;
   while((mode = [enumerator nextObject]))
   {
      vmAdd.resolution.x = [[mode valueForKey:@"Width"] intValue];
      vmAdd.resolution.y = [[mode valueForKey:@"Height"] intValue];
      vmAdd.bitDepth = [[mode valueForKey:@"BitsPerPixel"] intValue];
      vmAdd.refreshRate = [[mode valueForKey:@"RefreshRate"] intValue];

      vmAdd.fullScreen = false;      
            
      // skip if mode claims to be 8bpp
      if( vmAdd.bitDepth == 8 )
         continue;
      
      // Only add this resolution if it is not already in the list:
      bool alreadyInList = false;
      for(Vector<GFXVideoMode>::iterator i = videoModes.begin(); i != videoModes.end(); i++)
      {
         if(vmAdd == *i)
         {
            alreadyInList = true;
            break;
         }
      }
      if( !alreadyInList )
      {
         videoModes.push_back( vmAdd );
      }
   }

   
   // Get number of displays
   CGDisplayCount dispCnt;
   CGGetActiveDisplayList(0, NULL, &dispCnt);
   
   // Take advantage of GNU-C
   CGDirectDisplayID displays[dispCnt];
   
   CGGetActiveDisplayList(dispCnt, displays, &dispCnt);
   for(U32 i = 0; i < dispCnt; i++)
   {
      toAdd = new GFXAdapter();
      toAdd->mType = OpenGL;
      toAdd->mIndex = (U32)displays[i];
      toAdd->mCreateDeviceInstanceDelegate = mCreateDeviceInstance;
      String renderer = _getRendererForDisplay(displays[i]);
      AssertFatal(dStrlen(renderer.c_str()) < GFXAdapter::MaxAdapterNameLen, "GFXGLDevice::enumerateAdapter - renderer name too long, increae the size of GFXAdapter::MaxAdapterNameLen (or use String!)"); 
      dStrncpy(toAdd->mName, renderer.c_str(), GFXAdapter::MaxAdapterNameLen);
      adapterList.push_back(toAdd);
      
      for (S32 j = videoModes.size() - 1 ; j >= 0 ; j--)
         toAdd->mAvailableModes.push_back(videoModes[j]);
   }
}

void GFXGLDevice::enumerateVideoModes() 
{
   mVideoModes.clear();
   
   CGDirectDisplayID display = CGMainDisplayID();
   
   // Enumerate all available resolutions:
   NSArray* modeArray = (NSArray*)CGDisplayAvailableModes(display);
   
   GFXVideoMode toAdd;   
   NSEnumerator* enumerator = [modeArray objectEnumerator];
   NSDictionary* mode;
   while((mode = [enumerator nextObject]))
   {
      toAdd.resolution.x = [[mode valueForKey:@"Width"] intValue];
      toAdd.resolution.y = [[mode valueForKey:@"Height"] intValue];
      toAdd.bitDepth = [[mode valueForKey:@"BitsPerPixel"] intValue];
      toAdd.refreshRate = [[mode valueForKey:@"RefreshRate"] intValue];

      toAdd.fullScreen = false;      
            
      // skip if mode claims to be 8bpp
      if( toAdd.bitDepth == 8 )
         continue;
      
      // Only add this resolution if it is not already in the list:
      bool alreadyInList = false;
      for(Vector<GFXVideoMode>::iterator i = mVideoModes.begin(); i != mVideoModes.end(); i++)
      {
         if(toAdd == *i)
         {
            alreadyInList = true;
            break;
         }
      }
      if( !alreadyInList )
      {
         mVideoModes.push_back( toAdd );
      }
   }
}

bool GFXGLDevice::beginSceneInternal() 
{
   // Nothing to do here for GL.
   mCanCurrentlyRender = true;
   return true;
}

U32 GFXGLDevice::getTotalVideoMemory()
{
   // Convert our adapterIndex (i.e. our CGDirectDisplayID) into an OpenGL display mask
   GLuint display = CGDisplayIDToOpenGLDisplayMask((CGDirectDisplayID)mAdapterIndex);
   CGLRendererInfoObj rend;
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5
   GLint nrend;
#else
   long int nrend;
#endif
   CGLQueryRendererInfo(display, &rend, &nrend);
   if(!nrend)
      return 0;
   
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5
   GLint vidMem;
#else
   long int vidMem;
#endif
   CGLDescribeRenderer(rend, 0, kCGLRPVideoMemory, &vidMem);
   CGLDestroyRendererInfo(rend);
   
   // convert bytes to MB
   vidMem /= (1024 * 1024);
   
   return vidMem;
}

//-----------------------------------------------------------------------------
GFXWindowTarget *GFXGLDevice::allocWindowTarget(PlatformWindow *window)
{
   void *ctx = NULL;
      
   // Init if needed, or create a new context
   if(!mInitialized)
      init(window->getVideoMode(), window);
   else
      ctx = _createContextForWindow(window, mContext, mPixelFormat);
   
   // Allocate the wintarget and create a new context.
   GFXGLWindowTarget *gwt = new GFXGLWindowTarget(window, this);
   gwt->mContext = ctx ? ctx : mContext;

   // And return...
   return gwt;
}

GFXFence* GFXGLDevice::_createPlatformSpecificFence()
{
   if(!mCardProfiler->queryProfile("GL::APPLE::suppFence"))
      return NULL;
   
   return new GFXGLAppleFence(this);
}

void GFXGLWindowTarget::makeActive()
{
   // If we're supposed to be running fullscreen, but haven't yet set up for it,
   // do it now.
   
   if( !mFullscreenContext && mWindow->getVideoMode().fullScreen )
   {
      static_cast< GFXGLDevice* >( mDevice )->zombify();
      _setupNewMode();
   }
      
   mFullscreenContext ? [(NSOpenGLContext*)mFullscreenContext makeCurrentContext] : [(NSOpenGLContext*)mContext makeCurrentContext];
}

bool GFXGLWindowTarget::present() 
{
   GFX->updateStates();
   mFullscreenContext ? [(NSOpenGLContext*)mFullscreenContext flushBuffer] : [(NSOpenGLContext*)mContext flushBuffer];
   return true;
}

void GFXGLWindowTarget::_teardownCurrentMode()
{
   GFX->setActiveRenderTarget(this);
   static_cast<GFXGLDevice*>(mDevice)->zombify();
   if(mFullscreenContext)
   {
      [NSOpenGLContext clearCurrentContext];
      [(NSOpenGLContext*)mFullscreenContext clearDrawable];
   }
}

void GFXGLWindowTarget::_setupNewMode()
{
   if(mWindow->getVideoMode().fullScreen && !mFullscreenContext)
   {
      // We have to create a fullscreen context.
      Vector<NSOpenGLPixelFormatAttribute> attributes = _beginPixelFormatAttributesForDisplay(static_cast<MacWindow*>(mWindow)->getDisplay());
      _addColorAlphaDepthStencilAttributes(attributes, 24, 8, 24, 8);
      _addFullscreenAttributes(attributes);
      _endAttributeList(attributes);
      
      NSOpenGLPixelFormat* fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes.address()];
      mFullscreenContext = [[NSOpenGLContext alloc] initWithFormat:fmt shareContext:nil];
      [fmt release];
      [(NSOpenGLContext*)mFullscreenContext setFullScreen];
      [(NSOpenGLContext*)mFullscreenContext makeCurrentContext];
      // Restore resources in new context
      static_cast<GFXGLDevice*>(mDevice)->resurrect();
      GFX->updateStates(true);
   }
   else if(!mWindow->getVideoMode().fullScreen && mFullscreenContext)
   {
      [(NSOpenGLContext*)mFullscreenContext release];
      mFullscreenContext = NULL;
      [(NSOpenGLContext*)mContext makeCurrentContext];
      GFX->clear(GFXClearTarget | GFXClearZBuffer | GFXClearStencil, ColorI(0, 0, 0), 1.0f, 0);
      static_cast<GFXGLDevice*>(mDevice)->resurrect();
      GFX->updateStates(true);
   }
}
