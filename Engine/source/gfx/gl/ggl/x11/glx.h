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

/*
** The contents of this file are subject to the GLX Public License Version 1.0
** (the "License"). You may not use this file except in compliance with the
** License. You may obtain a copy of the License at Silicon Graphics, Inc.,
** attn: Legal Services, 2011 N. Shoreline Blvd., Mountain View, CA 94043
** or at http://www.sgi.com/software/opensource/glx/license.html.
**
** Software distributed under the License is distributed on an "AS IS"
** basis. ALL WARRANTIES ARE DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY
** IMPLIED WARRANTIES OF MERCHANTABILITY, OF FITNESS FOR A PARTICULAR
** PURPOSE OR OF NON- INFRINGEMENT. See the License for the specific
** language governing rights and limitations under the License.
**
** The Original Software is GLX version 1.2 source code, released February,
** 1999. The developer of the Original Software is Silicon Graphics, Inc.
** Those portions of the Subject Software created by Silicon Graphics, Inc.
** are Copyright (c) 1991-9 Silicon Graphics, Inc. All Rights Reserved.
*/

#ifndef GFX_GLX_H
#define GFX_GLX_H

#ifndef GFX_GGL_H
   #include "../ggl.h"
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace GL
{

//-----------------------------------------------------------------------------
#ifndef DOXYGEN

#define GLX_VERSION_1_0 1

#define GLX_USE_GL 1
#define GLX_BUFFER_SIZE 2
#define GLX_LEVEL 3
#define GLX_RGBA 4
#define GLX_DOUBLEBUFFER 5
#define GLX_STEREO 6
#define GLX_AUX_BUFFERS 7
#define GLX_RED_SIZE 8
#define GLX_GREEN_SIZE 9
#define GLX_BLUE_SIZE 10
#define GLX_ALPHA_SIZE 11
#define GLX_DEPTH_SIZE 12
#define GLX_STENCIL_SIZE 13
#define GLX_ACCUM_RED_SIZE 14
#define GLX_ACCUM_GREEN_SIZE 15
#define GLX_ACCUM_BLUE_SIZE 16
#define GLX_ACCUM_ALPHA_SIZE 17
#define GLX_BAD_SCREEN 1
#define GLX_BAD_ATTRIBUTE 2
#define GLX_NO_EXTENSION 3
#define GLX_BAD_VISUAL 4
#define GLX_BAD_CONTEXT 5
#define GLX_BAD_VALUE 6
#define GLX_BAD_ENUM 7

typedef XID GLXDrawable;
typedef XID GLXPixmap;
typedef struct __GLXcontextRec *GLXContext;

#include "../generated/glxe.h"

//-----------------------------------------------------------------------------
// All core functionality is implemented as function pointers.

#define GL_GROUP_BEGIN(name)
#define GL_FUNCTION(name, type, args) extern type (XGL_DLL *name) args;
#define GL_GROUP_END()
#include "../generated/glxfn.h"
#undef GL_GROUP_BEGIN
#undef GL_FUNCTION
#undef GL_GROUP_END


//-----------------------------------------------------------------------------
// Extensions use indirection in order to support multiple contexts

struct GLXExtensionPtrs: public GLExtensionPtrs {
   // Include all GLX extentions in global function table
   #define GL_GROUP_BEGIN(name)
   #define GL_FUNCTION(name, type, args) type (XGL_DLL *_##name) args;
   #define GL_GROUP_END()
   #include "../generated/glxefn.h"
   #undef GL_GROUP_BEGIN
   #undef GL_FUNCTION
   #undef GL_GROUP_END
};

struct GLXExtensionFlags {
   // Define extension "has" variables
   #define GL_GROUP_BEGIN(name) bool has_##name;
   #define GL_FUNCTION(name, type, args)
   #define GL_GROUP_END()
   #include "../generated/glxefn.h"
   #undef GL_GROUP_BEGIN
   #undef GL_FUNCTION
   #undef GL_GROUP_END
};

#endif // Doxygen

//-----------------------------------------------------------------------------

GLDisplay gglCreateDisplay(::Display*,int screen);
GLSurface gglCreateSurface(GLDisplay,Window,GLFormat);
XVisualInfo* gglGetFormatVisual(GLDisplay dp,GLFormat format);

#undef XGL_FUNCPTR
#define XGL_FUNCPTR(name) (((GLXExtensionPtrs*)GL::_GGLptr)->_##name)

} // Namespace
#endif

