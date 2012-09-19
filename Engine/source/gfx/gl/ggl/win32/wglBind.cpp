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

#include "wgl.h"

#include "core/strings/stringFunctions.h"

#include "console/console.h"

//-----------------------------------------------------------------------------
// Instantiation of GL function pointers in global namespace.

#define GL_GROUP_BEGIN(name)
#define GL_FUNCTION(name, type, args) type (__stdcall *name) args;
#define GL_GROUP_END()
#include "../generated/glcfn.h"
#undef GL_GROUP_BEGIN
#undef GL_FUNCTION
#undef GL_GROUP_END


namespace GL
{

bool bindFunction(DLibrary* dll,void *&fnAddress, const char *name);
bool hasExtension(const char *name,const char* extensions);
bool hasVersion(const char *name,const char* prefix,int major,int minor);


//-----------------------------------------------------------------------------
// Instantiation of WGL function pointers. These are in the GL namespace to
// avoid problems with definitions in windows.h

#define GL_GROUP_BEGIN(name)
#define GL_FUNCTION(name, type, args) type (__stdcall *name) args;
#define GL_GROUP_END()
#include "../generated/wglfn.h"
#undef GL_GROUP_BEGIN
#undef GL_FUNCTION
#undef GL_GROUP_END


//-----------------------------------------------------------------------------

static bool bindExtension(void *&fnAddress, const char *name)
{
   fnAddress = (void*)wglGetProcAddress(name);
   if (!fnAddress)
      Con::printf("GLExtensions: Extension bind failed for %s",name);

   return fnAddress != 0;
}


//-----------------------------------------------------------------------------

bool gglBindCoreFunctions(DLibrary* dll)
{
   bool bound = true;

   // Bind static functions which are quarenteed to be part of the
   // OpenGL library. In this case, OpenGL 1.0 and WGL functions
   #define GL_GROUP_BEGIN(name)
   #define GL_GROUP_END()
   #define GL_FUNCTION(fn_name, fn_return, fn_args) \
         bound &= bindFunction(dll, *(void**)&fn_name, #fn_name);
   #include "../generated/glcfn.h"
   #include "../generated/wglfn.h"
   #undef GL_FUNCTION
   #undef GL_GROUP_BEGIN
   #undef GL_GROUP_END

   return bound;
}

bool gglBindExtensions(DLibrary* dll,WGLExtensionPtrs* glp,WGLExtensionFlags* glf,HDC hDC)
{
   dMemset(glp,0,sizeof(WGLExtensionPtrs));
   dMemset(glf,0,sizeof(WGLExtensionFlags));

   // Get WGL extensions, there is no version
   const char* wgl = 0;
   if (bindExtension(*(void**)&glp->_wglGetExtensionsStringARB,"wglGetExtensionsStringARB"))
      wgl = glp->_wglGetExtensionsStringARB(hDC);
   else
      if (bindExtension(*(void**)&glp->_wglGetExtensionsStringEXT,"wglGetExtensionsStringEXT"))
         wgl = glp->_wglGetExtensionsStringEXT();

   // Get OpenGL version and extensions
   const char* glExtensions = (const char*)glGetString(GL_EXTENSIONS);
   const char* glVersion = (const char*) glGetString(GL_VERSION);
   if (!glExtensions || !glVersion)
      return false;

   // Parse the version string major.minor
   const char *itr = glVersion;
   int glMajor = dAtoi(itr);
   while (dIsdigit(*itr))
      *itr++;
   int glMinor = dAtoi(++itr);

   // Test for, and bind, all known extensions. GL and GLX versions ubove 1.0
   // are also treated as extensions and bound here.
   #define GL_GROUP_BEGIN(name) \
         if (hasVersion(#name,"GL_VERSION",glMajor,glMinor) || \
            hasExtension(#name,glExtensions) || \
            hasExtension(#name,wgl)) \
         { \
            glf->has_##name = true;
   #define GL_FUNCTION(fn_name, fn_return, fn_args) \
            bindExtension(*(void**)&glp->_##fn_name, #fn_name);
   #define GL_GROUP_END() \
         }
   #include "../generated/glefn.h"
   #include "../generated/wglefn.h"
   #undef GL_FUNCTION
   #undef GL_GROUP_BEGIN
   #undef GL_GROUP_END

   glf->bound = glp->bound = true;
   return true;
}

static DLibraryRef _hGL = NULL;

void gglPerformBinds()
{
   if(!_hGL)
   {
      _hGL = OsLoadLibrary("opengl32.dll");
      gglBindCoreFunctions(_hGL);
   }
}

void gglPerformExtensionBinds(void *context)
{
	if(!_hGL)
	{
		_hGL = OsLoadLibrary("opengl32.dll");
	}
	if(!_GGLptr)
	{
      static WGLExtensionPtrs ptrs;
      static WGLExtensionFlags flags;

		_GGLptr = &ptrs;
		_GGLflag = &flags;
	}

	 gglBindExtensions(_hGL, (WGLExtensionPtrs*)_GGLptr, (WGLExtensionFlags*)_GGLflag, (HDC)context);
}

} // Namespace

