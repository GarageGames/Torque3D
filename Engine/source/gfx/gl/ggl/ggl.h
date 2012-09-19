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

#ifndef GL_GGL_H
#define GL_GGL_H

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif

//-----------------------------------------------------------------------------
// Configuration file which defines which parts of GL to include

#ifndef DOXYGEN
#include "gfx/gl/ggl/gglConfig.h"


//-----------------------------------------------------------------------------

#if defined(__CYGWIN__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32)
   #define XGL_DLL __stdcall
#else
   #define XGL_DLL
#endif


//-----------------------------------------------------------------------------
// Include core (OpenGL 1.1) definitions

#include "gfx/gl/ggl/generated/glc.h"
#include "gfx/gl/ggl/generated/gle.h"


//-----------------------------------------------------------------------------
// All core functionality is implemented as function pointers.

#define GL_GROUP_BEGIN(name)
#define GL_FUNCTION(name, type, args) extern type (XGL_DLL *name) args;
#define GL_GROUP_END()
#include "gfx/gl/ggl/generated/glcfn.h"
#undef GL_GROUP_BEGIN
#undef GL_FUNCTION
#undef GL_GROUP_END

/// OpenGL interface.
namespace GL
{

//-----------------------------------------------------------------------------
// Extensions use indirection in order to support multiple contexts

struct GLExtensionPtrs {
   bool bound;

   // Include all OpenGL extensions for all platform
   #define GL_GROUP_BEGIN(name)
   #define GL_FUNCTION(name, type, args) type (XGL_DLL *_##name) args;
   #define GL_GROUP_END()
   #include "gfx/gl/ggl/generated/glefn.h"
   #undef GL_GROUP_BEGIN
   #undef GL_FUNCTION
   #undef GL_GROUP_END

   GLExtensionPtrs() { bound = false; }
};

struct GLExtensionFlags {
   bool bound;

   // Define extension "has" variables
   #define GL_GROUP_BEGIN(name) bool has_##name;
   #define GL_FUNCTION(name, type, args)
   #define GL_GROUP_END()
   #include "gfx/gl/ggl/generated/glefn.h"
   #undef GL_GROUP_BEGIN
   #undef GL_FUNCTION
   #undef GL_GROUP_END

   GLExtensionFlags() { bound = false; }
};

// Extension loading has been reimplemented on each platform, and each platform
// has a different version of GLExtensionPtrs and GLExtensionFlags.  When binding
// extensions for that platform, you MUST use these pointers.  From there the XGL_FUNCPTR
// define will handle converting a gl*EXT call into the proper member function call.
extern GLExtensionPtrs* _GGLptr;
#define XGL_FUNCPTR(name) (GL::_GGLptr->_##name)
#endif // Doxygen

extern GLExtensionFlags* _GGLflag;
#define gglHasExtension(name) (GL::_GGLflag->has_##name)



} // Namespace

#endif

