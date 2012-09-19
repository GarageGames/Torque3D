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

#ifndef GFX_WGL_H
#define GFX_WGL_H

#ifndef GFX_GGL_H
   #include "../ggl.h"
#endif
#ifndef _PLATFORMDLIBRARY_H
   #include "platform/platformDlibrary.h"
#endif

#ifndef _WIN32_WINNT
   #define _WIN32_WINNT 0x0500
#endif
#include <windows.h>


namespace GL
{

//-----------------------------------------------------------------------------
// Include WGL 1.0 definitions

#ifndef DOXYGEN
#include "../generated/wgle.h"


//-----------------------------------------------------------------------------
// All core functionality is implemented as function pointers.

#define GL_GROUP_BEGIN(name)
#define GL_FUNCTION(name, type, args) extern type (XGL_DLL *name) args;
#define GL_GROUP_END()
#include "../generated/wglfn.h"
#undef GL_GROUP_BEGIN
#undef GL_FUNCTION
#undef GL_GROUP_END


//-----------------------------------------------------------------------------
// Extensions use indirection in order to support multiple contexts

struct WGLExtensionPtrs: public GLExtensionPtrs {
   // Include all OpenGL extensions for all platform
   #define GL_GROUP_BEGIN(name)
   #define GL_FUNCTION(name, type, args) type (XGL_DLL *_##name) args;
   #define GL_GROUP_END()
   #include "../generated/wglefn.h"
   #undef GL_GROUP_BEGIN
   #undef GL_FUNCTION
   #undef GL_GROUP_END
};

struct WGLExtensionFlags: public GLExtensionFlags {
   // Define extension "has" variables
   #define GL_GROUP_BEGIN(name) bool has_##name;
   #define GL_FUNCTION(name, type, args)
   #define GL_GROUP_END()
   #include "../generated/wglefn.h"
   #undef GL_GROUP_BEGIN
   #undef GL_FUNCTION
   #undef GL_GROUP_END
};
#endif // Doyxygen


//-----------------------------------------------------------------------------

bool gglBindExtensions(DLibrary*,WGLExtensionPtrs*,WGLExtensionFlags*,HDC);

#undef XGL_FUNCPTR
#define XGL_FUNCPTR(name) (((GL::WGLExtensionPtrs*)GL::_GGLptr)->_##name)

#undef gglHasExtension
#define gglHasExtension(name) (((GL::WGLExtensionFlags*)GL::_GGLflag)->has_##name)


} // Namespace

#endif

