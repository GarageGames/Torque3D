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

#include "gfx/gl/gfxGLCardProfiler.h"
#include "gfx/gl/gfxGLDevice.h"
#include "gfx/gl/gfxGLEnumTranslate.h"

void GFXGLCardProfiler::init()
{
   mChipSet = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
   mRendererString = "OpenGL";
   mCardDescription = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
   mVersionString = reinterpret_cast<const char*>(glGetString(GL_VERSION));   
   mVideoMemory = static_cast<GFXGLDevice*>(GFX)->getTotalVideoMemory();

   Parent::init();
}

void GFXGLCardProfiler::setupCardCapabilities()
{
   GLint maxTexSize;
   glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
   
   // OpenGL doesn't have separate maximum width/height.
   setCapability("maxTextureWidth", maxTexSize);
   setCapability("maxTextureHeight", maxTexSize);
   setCapability("maxTextureSize", maxTexSize);

   // Check for anisotropic filtering support.
   setCapability("GL_EXT_texture_filter_anisotropic", gglHasExtension(EXT_texture_filter_anisotropic));

   // Check for buffer storage
#ifdef TORQUE_NSIGHT_WORKAROUND
   setCapability("GL_ARB_buffer_storage", false);
#else
   setCapability("GL_ARB_buffer_storage", gglHasExtension(ARB_buffer_storage));
#endif

   // Check for shader model 5.0
   setCapability("GL_ARB_gpu_shader5", gglHasExtension(ARB_gpu_shader5));

   // Check for texture storage
   setCapability("GL_ARB_texture_storage", gglHasExtension(ARB_texture_storage));

   // Check for sampler objects
   setCapability("GL_ARB_sampler_objects", gglHasExtension(ARB_sampler_objects));

   // Check for copy image support
   setCapability("GL_ARB_copy_image", gglHasExtension(ARB_copy_image));

   // Check for vertex attrib binding
   setCapability("GL_ARB_vertex_attrib_binding", gglHasExtension(ARB_vertex_attrib_binding));    

}

bool GFXGLCardProfiler::_queryCardCap(const String& query, U32& foundResult)
{
   // Just doing what the D3D9 layer does
   return 0;
}

bool GFXGLCardProfiler::_queryFormat(const GFXFormat fmt, const GFXTextureProfile *profile, bool &inOutAutogenMips)
{
	// We assume if the format is valid that we can use it for any purpose.
   // This may not be the case, but we have no way to check short of in depth 
   // testing of every format for every purpose.  And by testing, I mean sitting
   // down and doing it by hand, because there is no OpenGL API to check these
   // things.
   return GFXGLTextureInternalFormat[fmt] != GL_ZERO;
}
