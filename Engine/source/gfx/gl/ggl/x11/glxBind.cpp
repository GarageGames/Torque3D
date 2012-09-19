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

#include "glx.h"

#include "util/utilAssert.h"
#include "util/utilStr.h"
#include "util/utilString.h"
#include "util/utilMemory.h"
#include "os/osLog.h"
#include "os/osDlibrary.h"

//-----------------------------------------------------------------------------
// Instantiation of GL function pointers in global namespace.

#define GL_GROUP_BEGIN(name)
#define GL_FUNCTION(name, type, args) type (XGL_DLL *name) args;
#define GL_GROUP_END()
#include "../generated/glcfn.h"
#undef GL_GROUP_BEGIN
#undef GL_FUNCTION
#undef GL_GROUP_END


namespace GL
{
using namespace Torque;

bool bindFunction(DLibrary* dll,void *&fnAddress, const char *name);
bool hasExtension(const char *name,const char* extensions);
bool hasVersion(const char *name,const char* prefix,int major,int minor);


//-----------------------------------------------------------------------------
// Instantiation of GLX function pointers.

#define GL_GROUP_BEGIN(name)
#define GL_FUNCTION(name, type, args) type (XGL_DLL *name) args;
#define GL_GROUP_END()
#include "../generated/glxfn.h"
#undef GL_GROUP_BEGIN
#undef GL_FUNCTION
#undef GL_GROUP_END


//-----------------------------------------------------------------------------

bool bindFunction(DLibrary* dll,GLFunction (*gproc)(const GLubyte*),
   void *&fnAddress, const char *name)
{
   // Use the getProcAddress if we have it.
   fnAddress = gproc? (void*)(*gproc)((const GLubyte*)name): dll->bind(name);
   return fnAddress != 0;
}


//-----------------------------------------------------------------------------

bool gglBindCoreFunctions(DLibrary* dll,GLXExtensionPtrs* glp)
{
   bool bound = true;

   // Bind static functions which are quarenteed to be part of the
   // OpenGL library. In this case, OpenGL 1.0 and GLX 1.0 functions
   #define GL_GROUP_BEGIN(name)
   #define GL_GROUP_END()
   #define GL_FUNCTION(fn_name, fn_return, fn_args) \
         bound &= bindFunction(dll,*(void**)&fn_name, #fn_name);
   #include "../generated/glcfn.h"
   #include "../generated/glxfn.h"
   #undef GL_FUNCTION
   #undef GL_GROUP_BEGIN
   #undef GL_GROUP_END

   // Check for the getProcAddress first, otherwise we'll expect everything
   // to be in the DLL.
   memset(glp,0,sizeof(GLXExtensionPtrs));
   glp->_glXGetProcAddressARB = (GLFunction (*)(const GLubyte*))dll->bind("glXGetProcAddressARB");

   // Try and bind all known extension functions. We'll check later to
   // see which ones are actually valid for a context.
   #define GL_GROUP_BEGIN(name)
   #define GL_FUNCTION(fn_name, fn_return, fn_args) \
            bindFunction(dll,glp->_glXGetProcAddressARB,*(void**)&glp->_##fn_name, #fn_name);
   #define GL_GROUP_END()
   #include "../generated/glefn.h"
   #include "../generated/glxefn.h"
   #undef GL_FUNCTION
   #undef GL_GROUP_BEGIN
   #undef GL_GROUP_END

   return bound;
}


//-----------------------------------------------------------------------------

void gglBindGLX(::Display* display,int screen,GLXExtensionFlags* glx)
{
   // Check GLX version and glx extensions
   int glxMajor,glxMinor;
   glXQueryVersion(display,&glxMajor,&glxMinor);
   const char* glxExtensions = glXQueryExtensionsString(display,screen);

   static LogCategory log("/Gfx/Device/GL");
   log.print(format("GLX Version: %d.%d",glxMajor,glxMinor));
   Assert(glxMajor == 1 && glxMinor >= 1,"GLXBind: Need GLX version 1.1 or greater");

   #define GL_GROUP_BEGIN(name) \
         glx->has_##name = hasVersion(#name,"GLX_VERSION",glxMajor,glxMinor) || \
             hasExtension(#name,glxExtensions);
   #define GL_FUNCTION(fn_name, fn_return, fn_args)
   #define GL_GROUP_END()
   #include "../generated/glxefn.h"
   #undef GL_FUNCTION
   #undef GL_GROUP_BEGIN
   #undef GL_GROUP_END
}


//-----------------------------------------------------------------------------

bool gglBindExtensions(GLExtensionFlags* gl)
{
   memset(gl,0,sizeof(GLExtensionFlags));

   // Get GL version and extensions
   const char* glExtensions = (const char*)glGetString(GL_EXTENSIONS);
   const char* glVersion = (const char*) glGetString(GL_VERSION);
   if (!glExtensions || !glVersion)
      return false;

   // Parse the GL version string "major.minor"
   const char *itr = glVersion;
   int glMajor = atoi(itr);
   while (isDigit(*itr))
      *itr++;
   int glMinor = atoi(++itr);

   // Check which extensions are available on the active context.
   // GL and GLX versions ubove 1.0 are also tested here.
   #define GL_GROUP_BEGIN(name) \
         gl->has_##name = hasVersion(#name,"GL_VERSION",glMajor,glMinor) || \
            hasExtension(#name,glExtensions);
   #define GL_FUNCTION(fn_name, fn_return, fn_args)
   #define GL_GROUP_END()
   #include "../generated/glefn.h"
   #undef GL_FUNCTION
   #undef GL_GROUP_BEGIN
   #undef GL_GROUP_END

   gl->bound = true;
   return true;
}

} // Namespace

