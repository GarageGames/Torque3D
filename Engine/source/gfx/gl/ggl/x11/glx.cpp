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

#include "os/osDlibrary.h"
#include "os/osLog.h"
#include "util/utilArray.h"

#include "glx.h"

namespace GL
{
using namespace Torque;
using namespace GL;

extern bool gglBindCoreFunctions(DLibrary* dll,GLXExtensionPtrs*);
void gglBindGLX(::Display* display,int screen,GLXExtensionFlags* glx);
bool gglBindExtensions(GLExtensionFlags* gl);

struct GGLContext;

//-----------------------------------------------------------------------------
// The OpenGL DLL is shared by all the devices.
static DLibraryRef _hGL;

// Unlike WGL, GLX does not require a set of function pointers per
// context pixel format. All functions in the DLL are bound in a single
// table and shared by all contexts.
static GLXExtensionPtrs _LibraryFunctions;


//-----------------------------------------------------------------------------

struct GGLDisplay: public _GLDisplay
{
   ::Display* display;
   int screen;
   GLXExtensionFlags glx;  // GLX extension flags
   Array<GGLContext*> contextList;
};

#define _hasGLXExtension(display,name) (display->glx.has_##name)

GLDisplay gglCreateDisplay(::Display* display,int screen)
{
   // Load and bind DLL functions
   if (!_hGL) {
      static LogCategory log("/Gfx/Device/GL");
      _hGL = LoadLibrary("libGL.so");
      if (!_hGL) {
         log.print("GLDevice: OpenGL dll failed to load");
         return false;
      }
      if (!gglBindCoreFunctions(_hGL.ptr(),&_LibraryFunctions)) {
         log.print("GLDevice: Failed to bind all core functions");
         return false;
      }
      _GGLptr = &_LibraryFunctions;
      log.print("OpenGL library loaded");
   }

   //
   GGLDisplay* dp = new GGLDisplay;
   dp->display = display;
   dp->screen = screen;
   gglBindGLX(display,screen,&dp->glx);
   return dp;
}

void gglDeleteDisplay(GLDisplay dp)
{
   GGLDisplay* display = (GGLDisplay*)dp;
   Assert(!display->contextList.size(),"gglDeleteDisplay: Not all context destroyed");
   delete display;
}


//-----------------------------------------------------------------------------

struct GGLFormat: public _GLFormat {
   GLFormatInfo mode;
   U32 pformat;
};

static bool _getFBConfig(GGLDisplay* display,U32 pformat,GLXFBConfig& config)
{
   // Get config info from format ID
   int attributes[] = { GLX_FBCONFIG_ID, pformat, None };
   int count = 0;
   GLXFBConfig* configList = glXChooseFBConfig(display->display,
      display->screen,attributes,&count);
   if (!count)
      return false;
   config = *configList;
   XFree(configList);
   return true;
}

GLFormat gglSelectFormat(GLDisplay dp,GLFormatInfo& mode)
{
   GGLDisplay* display = (GGLDisplay*)dp;
   U32 pformat;

   if (!_hasGLXExtension(display,GLX_VERSION_1_3)) {
      // Find GL compatible X visual using 1.2 interface.
      // 1.2 or earlier does not include pbuffer support.
      if (mode.target != GLFormatInfo::Window)
         return 0;

      int attributes[] = {
         GLX_RGBA,
         GLX_RED_SIZE,        mode.red,
         GLX_BLUE_SIZE,       mode.blue,
         GLX_GREEN_SIZE,      mode.green,
         GLX_DEPTH_SIZE,      mode.z,
         GLX_ALPHA_SIZE,      mode.alpha,
         GLX_STENCIL_SIZE,    mode.stencil,
         GLX_DOUBLEBUFFER,    true,
         GLX_DOUBLEBUFFER,
         None
      };
      XVisualInfo* visual = glXChooseVisual(display->display, display->screen, attributes);
      if (!visual) {
         static LogCategory log("/Gfx/Device/GL");
         log.print("GGLContext: Could not get an accelerated visual");
         return 0;
      }
      pformat = visual->visualid;
      XFree(visual);
   }
   else {
      // Find GL compatible X visual using 1.3 interface.
      int attributes[] = {
         GLX_DRAWABLE_TYPE,   (mode.target == GLFormatInfo::Buffer)?
                              GLX_PBUFFER_BIT: GLX_WINDOW_BIT,
         GLX_RENDER_TYPE,     GLX_RGBA_BIT,
         GLX_RED_SIZE,        mode.red,
         GLX_BLUE_SIZE,       mode.blue,
         GLX_GREEN_SIZE,      mode.green,
         GLX_DEPTH_SIZE,      mode.z,
         GLX_ALPHA_SIZE,      mode.alpha,
         GLX_STENCIL_SIZE,    mode.stencil,
         GLX_DOUBLEBUFFER,    true,
         None
      };
      int count = 0;
      GLXFBConfig* configList = glXChooseFBConfig(display->display,
         display->screen,attributes,&count);
      if (!count)
         return 0;

      // Return the config ID
      int xid;
      glXGetFBConfigAttrib(display->display,configList[0],GLX_FBCONFIG_ID,&xid);
      XFree(configList);
      pformat = xid;
   }

   if (pformat) {
      GGLFormat* format = new GGLFormat;
      format->pformat = pformat;
      format->mode = mode;
      return format;
   }
   return 0;
}

bool gglFormatInfo(GLDisplay display,GLFormat pformat,GLFormatInfo& mode)
{
   return 0;
}

bool gglAreCompatible(GLFormat,GLFormat)
{
   return false;
}

void gglDeleteFormat(GLFormat fp)
{
   GGLFormat* format = (GGLFormat*)fp;
   delete format;
}

XVisualInfo* gglGetFormatVisual(GLDisplay dp,GLFormat fp)
{
   GGLDisplay* display = (GGLDisplay*)dp;
   GGLFormat* format = (GGLFormat*)fp;

   if (!_hasGLXExtension(display,GLX_VERSION_1_3)) {
      // Format is a visual id
      int count;
      XVisualInfo match;
      match.visualid = format->pformat;
      return XGetVisualInfo(display->display,VisualIDMask,&match,&count);
   }
   else {
      // Format is an FBConfig id
      GLXFBConfig config;
      if (!_getFBConfig(display,format->pformat,config))
         return 0;
      return glXGetVisualFromFBConfig(display->display,config);
   }
}


//-----------------------------------------------------------------------------

struct GGLSurface: public _GLSurface {
   enum Type {
      Window, PBuffer,
   } type;
   GGLDisplay* display;
   GLXDrawable drawable;      // Used with GLX 1.3 interface
   ::Window window;           // Used with GLX 1.2 interface
};

GLSurface gglCreateSurface(GLDisplay dp,::Window hwin,GLFormat fp)
{
   GGLDisplay* display = (GGLDisplay*)dp;
   GGLFormat* format = (GGLFormat*)fp;
   GGLSurface* surface = 0;

   if (!_hasGLXExtension(display,GLX_VERSION_1_3)) {
      // Return our own GGL surface rep.
      surface = new GGLSurface;
      surface->type = GGLSurface::Window;
      surface->display = display;
      surface->drawable = 0;
      surface->window = hwin;
   }
   else {
      // Get config info from format ID
      GLXFBConfig config;
      if (!_getFBConfig(display,format->pformat,config))
         return 0;

      // Create GLX window
      GLXWindow window = glXCreateWindow(display->display,config,hwin,0);
      if (!window) {
         GLenum error = glGetError();
         static LogCategory log("/Gfx/Device/GL");
         log.print(format("Create window error: %d",error));
         return 0;
      }

      // Return our own GGL surface rep.
      surface = new GGLSurface;
      surface->type = GGLSurface::Window;
      surface->display = display;
      surface->drawable = window;
      surface->window = 0;
   }
   return surface;
}

bool gglGetSurfaceSize(GLSurface sp,Vector2I& size)
{
   GGLSurface* surface = (GGLSurface*)sp;
   if (surface->type != GGLSurface::Window)
      return false;
   if (!_hasGLXExtension(surface->display,GLX_VERSION_1_3)) {
      size.x = 300; size.y = 300;
   }
   else {
      glXQueryDrawable(surface->display->display,surface->drawable,
         GLX_WIDTH,(U32*)&size.x);
      glXQueryDrawable(surface->display->display,surface->drawable,
         GLX_HEIGHT,(U32*)&size.y);
   }
   return true;
}

void gglDeleteSurface(GLSurface sp)
{
   GGLSurface* surface = (GGLSurface*)sp;
   if (surface->type = GGLSurface::Window) {
      glXDestroyWindow(surface->display->display,surface->drawable);
      delete surface;
   }
}


//-----------------------------------------------------------------------------

struct GGLContext: public _GLContext
{
   GGLDisplay* display;
   GLXContext context;
   GLExtensionFlags glf;      // GL has extension flags
};

GLContext gglCreateContext(GLDisplay dp,GLFormat fp)
{
   GGLDisplay* display = (GGLDisplay*)dp;
   GGLFormat* format = (GGLFormat*)fp;
   GGLContext* context = 0;

   GLXContext ctx,share = display->contextList.size()?
      display->contextList[0]->context: 0;

   if (!_hasGLXExtension(display,GLX_VERSION_1_3)) {
      // Get visual from format id.
      int count;
      XVisualInfo match;
      match.visualid = format->pformat;
      XVisualInfo* visual =  XGetVisualInfo(display->display,VisualIDMask,&match,&count);

      ctx = glXCreateContext(display->display,visual,share,true);
      XFree(visual);
   }
   else {
      // Get FBConfig from format id
      GLXFBConfig config;
      if (!_getFBConfig(display,format->pformat,config))
         return 0;

      // Need to share contexts...
      ctx = glXCreateNewContext(display->display,config,
         GLX_RGBA_TYPE,share,true);
   }

   if (ctx) {
      context = new GGLContext;
      context->context = ctx;
      context->display = display;
      display->contextList.pushBack(context);
      return context;
   }
   return 0;
}

void gglDeleteContext(GLContext ctx)
{
   GGLContext* context = (GGLContext*)ctx;
   glXDestroyContext(context->display->display,context->context);
   erase(context->display->contextList,context);
   delete context;
}

bool gglMakeCurrent(GLSurface sp,GLContext ctx)
{
   GGLSurface* surface = (GGLSurface*)sp;
   GGLContext* context = (GGLContext*)ctx;
   if (!_hasGLXExtension(surface->display,GLX_VERSION_1_3)) {
      if (!glXMakeCurrent(surface->display->display,
            surface->window,context->context))
         return false;
   }
   else
      if (!glXMakeContextCurrent(surface->display->display,
            surface->drawable,surface->drawable,context->context))
         return false;

   // The first time a context is made current we need to
   // check which extensions are valid.
   if (!context->glf.bound)
      gglBindExtensions(&context->glf);
   _GGLflag = &context->glf;
   return true;
}

void gglSwapBuffers(GLSurface sp)
{
   GGLSurface* surface = (GGLSurface*)sp;
   if (!_hasGLXExtension(surface->display,GLX_VERSION_1_3))
      glXSwapBuffers(surface->display->display,surface->window);
   else
      glXSwapBuffers(surface->display->display,surface->drawable);
}


//-----------------------------------------------------------------------------

GLSurface gglCreatePBufferSurface(GLDisplay display,U32 width,U32 height,bool cubeMap,GLFormat pformat)
{
   return 0;
}

bool gglBindPBufferSurface(GLSurface sp,U32 target)
{
   return 0;
}

bool gglReleasePBufferSurface(GLSurface sp,U32 target)
{
   return 0;
}

bool gglSetPbufferTarget(GLSurface sp,U32 mip,U32 face)
{
   return 0;
}

} // Namespace

