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
#if 0
#include "platformX86UNIX/platformGL.h"
#include "platformX86UNIX/platformX86UNIX.h"
#include "console/console.h"

#include <dlfcn.h>
#include <SDL/SDL.h>

// declare stub functions
#define GL_FUNCTION(fn_return, fn_name, fn_args, fn_value) fn_return stub_##fn_name fn_args{ fn_value }
#include "platform/GLCoreFunc.h"
#include "platform/GLExtFunc.h"
#undef GL_FUNCTION

// point gl function pointers at stub functions
#define GL_FUNCTION(fn_return,fn_name,fn_args, fn_value) fn_return (*fn_name)fn_args = stub_##fn_name;
#include "platform/GLCoreFunc.h"
#include "platform/GLExtFunc.h"
#undef GL_FUNCTION

static void* dlHandle = NULL;

//------------------------------------------------------------------
//bind functions for each function prototype
//------------------------------------------------------------------

//GL_EXT/ARB
enum {
   ARB_multitexture              = BIT(0),
   ARB_texture_compression       = BIT(1),
   EXT_compiled_vertex_array     = BIT(2),
   EXT_fog_coord                 = BIT(3),
   EXT_paletted_texture          = BIT(4),
   NV_vertex_array_range         = BIT(5),
   EXT_blend_color               = BIT(6),
   EXT_blend_minmax              = BIT(7)
};

//WGL_ARB
enum {
   WGL_ARB_extensions_string  = BIT(0),
   WGL_EXT_swap_control       = BIT(1),
   WGL_3DFX_gamma_control     = BIT(2)
};


static bool isFnOk( const char *name)
{
   bool ok = false;

   // JMQ: these are specific to torque's d3d->gl wrapper.  They are not used under linux.
   if (dStrcmp(name, "glAvailableVertexBufferEXT")==0)
      ok = true;
   else if (dStrcmp(name, "glAllocateVertexBufferEXT")==0)
      ok = true;
   else if (dStrcmp(name, "glLockVertexBufferEXT")==0)
      ok = true;
   else if (dStrcmp(name, "glUnlockVertexBufferEXT")==0)
      ok = true;
   else if (dStrcmp(name, "glSetVertexBufferEXT")==0)
      ok = true;
   else if (dStrcmp(name, "glOffsetVertexBufferEXT")==0)
      ok = true;
   else if (dStrcmp(name, "glFillVertexBufferEXT")==0)
      ok = true;
   else if (dStrcmp(name, "glFreeVertexBufferEXT")==0)
      ok = true;

   return ok;
}

//------------------------------------------------------------------
//bind functions for each function prototype
//------------------------------------------------------------------
static bool bindGLFunction( void *&fnAddress, const char *name )
{
   void* addr = (void*)SDL_GL_GetProcAddress(name);
   bool ok = (bool)addr;
   if( !ok )
   {
      if (!isFnOk(name))
         Con::errorf(ConsoleLogEntry::General, " Missing OpenGL function '%s'", name);
      else
         ok = true;
   }
   else
      fnAddress = addr;
   return ok;
}

static bool bindEXTFunction( void *&fnAddress, const char *name )
{
   void* addr = (void*)SDL_GL_GetProcAddress(name);
   if( !addr )
      Con::errorf(ConsoleLogEntry::General, " Missing OpenGL extension '%s'", name);
   else
      fnAddress = addr;
   return (addr != NULL);
}

//------------------------------------------------------------------
//binds for each function group
//------------------------------------------------------------------
static bool bindGLFunctions()
{
   bool result = true;
   #define GL_FUNCTION(fn_return, fn_name, fn_args, fn_value) \
   result &= bindGLFunction( *(void**)&fn_name, #fn_name);
   #include "platform/GLCoreFunc.h"
   #undef GL_FUNCTION
   return result;
}

static bool bindEXTFunctions(U32 extMask)
{
   bool result = true;

   #define GL_GROUP_BEGIN( flag ) \
      if( extMask & flag ) {
   #define GL_GROUP_END() }

   #define GL_FUNCTION(fn_return, fn_name, fn_args, fn_value) \
   result &= bindEXTFunction( *(void**)&fn_name, #fn_name);
   #include "platform/GLExtFunc.h"
   #undef GL_FUNCTION

   #undef GL_GROUP_BEGIN
   #undef GL_GROUP_END

   return result;
}

static void unbindGLFunctions()
{
   // point gl function pointers at stub functions
#define GL_FUNCTION(fn_return, fn_name, fn_args, fn_value) fn_name = stub_##fn_name;
#include "platform/GLCoreFunc.h"
#include "platform/GLExtFunc.h"
#undef GL_FUNCTION
}

namespace GLLoader
{

   bool OpenGLInit()
   {
      return OpenGLDLLInit();
   }

   void OpenGLShutdown()
   {
      OpenGLDLLShutdown();
   }

   bool OpenGLDLLInit()
   {
      OpenGLDLLShutdown();

      // load libGL.so
      if (SDL_GL_LoadLibrary("libGL.so") == -1 &&
         SDL_GL_LoadLibrary("libGL.so.1") == -1)
      {
         Con::errorf("Error loading GL library: %s", SDL_GetError());
         return false;
      }

      // bind functions
      if (!bindGLFunctions())
      {
         Con::errorf("Error binding GL functions");
         OpenGLDLLShutdown();
         return false;
      }

      return true;
   }

   void OpenGLDLLShutdown()
   {
      // there is no way to tell SDL to unload the library
      if (dlHandle != NULL)
      {
         dlclose(dlHandle);
         dlHandle = NULL;
      }

      unbindGLFunctions();
   }

}

GLState gGLState;

bool  gOpenGLDisablePT                   = false;
bool  gOpenGLDisableCVA                  = false;
bool  gOpenGLDisableTEC                  = false;
bool  gOpenGLDisableARBMT                = false;
bool  gOpenGLDisableFC                   = false;
bool  gOpenGLDisableTCompress            = false;
bool  gOpenGLNoEnvColor                  = false;
float gOpenGLGammaCorrection             = 0.5;
bool  gOpenGLNoDrawArraysAlpha           = false;

// JMQTODO: really need a platform-shared version of this nastiness
bool GL_EXT_Init( )
{
   // Load extensions...
   //
   const char* pExtString = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
   gGLState.primMode = 0;
   U32 extBitMask = 0;

   // GL_EXT_paletted_texture
   if (pExtString && dStrstr(pExtString, (const char*)"GL_EXT_paletted_texture") != NULL)
   {
      extBitMask |= EXT_paletted_texture;
      gGLState.suppPalettedTexture = true;
   }
   else
      gGLState.suppPalettedTexture = false;

   // EXT_compiled_vertex_array
   if (pExtString && dStrstr(pExtString, (const char*)"GL_EXT_compiled_vertex_array") != NULL)
   {
      extBitMask |= EXT_compiled_vertex_array;
      gGLState.suppLockedArrays = true;
   }
   else
   {
      gGLState.suppLockedArrays = false;
   }

   // ARB_multitexture
   if (pExtString && dStrstr(pExtString, (const char*)"GL_ARB_multitexture") != NULL)
   {
      extBitMask |= ARB_multitexture;
      gGLState.suppARBMultitexture = true;
   } else {
      gGLState.suppARBMultitexture = false;
   }
   
   // EXT_blend_color
   if(pExtString && dStrstr(pExtString, (const char*)"GL_EXT_blend_color") != NULL)
   {
      extBitMask |= EXT_blend_color;
      gGLState.suppEXTblendcolor = true;
   } else {
      gGLState.suppEXTblendcolor = false;
   }

   // EXT_blend_minmax
   if(pExtString && dStrstr(pExtString, (const char*)"GL_EXT_blend_minmax") != NULL)
   {
      extBitMask |= EXT_blend_color;
      gGLState.suppEXTblendminmax = true;
   } else {
      gGLState.suppEXTblendminmax = false;
   }

   // EXT_fog_coord
   if (pExtString && dStrstr(pExtString, (const char*)"GL_EXT_fog_coord") != NULL)
   {
      extBitMask |= EXT_fog_coord;
      gGLState.suppFogCoord = true;
   } else {
      gGLState.suppFogCoord = false;
   }

   // EXT_texture_compression_s3tc
   if (pExtString && dStrstr(pExtString, (const char*)"GL_EXT_texture_compression_s3tc") != NULL)
      gGLState.suppS3TC = true;
   else
      gGLState.suppS3TC = false;

   // ARB_texture_compression
   if (pExtString && dStrstr(pExtString, (const char*)"GL_ARB_texture_compression") != NULL)
   {
      extBitMask |= ARB_texture_compression;
      gGLState.suppTextureCompression = true;
   } else {
      gGLState.suppTextureCompression = false;
   }

   // NV_vertex_array_range (not on *nix)
   gGLState.suppVertexArrayRange = false;

   // 3DFX_texture_compression_FXT1
   if (pExtString && dStrstr(pExtString, (const char*)"3DFX_texture_compression_FXT1") != NULL)
      gGLState.suppFXT1 = true;
   else
      gGLState.suppFXT1 = false;

   if (!bindEXTFunctions(extBitMask))
      Con::warnf("You are missing some OpenGL Extensions.  You may experience rendering problems.");

   // Binary states, i.e., no supporting functions
   // EXT_packed_pixels
   // EXT_texture_env_combine
   //
   // dhc note: a number of these can have multiple matching 'versions', private, ext, and arb.
   gGLState.suppPackedPixels      = pExtString? (dStrstr(pExtString, (const char*)"GL_EXT_packed_pixels") != NULL) : false;
   gGLState.suppTextureEnvCombine = pExtString? (dStrstr(pExtString, (const char*)"GL_EXT_texture_env_combine") != NULL) : false;
   gGLState.suppEdgeClamp         = pExtString? (dStrstr(pExtString, (const char*)"GL_EXT_texture_edge_clamp") != NULL) : false;
   gGLState.suppEdgeClamp        |= pExtString? (dStrstr(pExtString, (const char*)"GL_SGIS_texture_edge_clamp") != NULL) : false;
   gGLState.suppTexEnvAdd         = pExtString? (dStrstr(pExtString, (const char*)"GL_ARB_texture_env_add") != NULL) : false;
   gGLState.suppTexEnvAdd        |= pExtString? (dStrstr(pExtString, (const char*)"GL_EXT_texture_env_add") != NULL) : false;

   // Anisotropic filtering
   gGLState.suppTexAnisotropic    = pExtString? (dStrstr(pExtString, (const char*)"GL_EXT_texture_filter_anisotropic") != NULL) : false;
   if (gGLState.suppTexAnisotropic)
      glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gGLState.maxAnisotropy);
   if (gGLState.suppARBMultitexture)
      glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &gGLState.maxTextureUnits);
   else
      gGLState.maxTextureUnits = 1;

   // JMQ: vsync/swap interval skipped
   gGLState.suppSwapInterval = false;

   Con::printf("OpenGL Init: Enabled Extensions");
   if (gGLState.suppARBMultitexture)    Con::printf("  ARB_multitexture (Max Texture Units: %d)", gGLState.maxTextureUnits);
   if (gGLState.suppEXTblendcolor)        Con::printf("  EXT_blend_color");
   if (gGLState.suppEXTblendminmax)       Con::printf("  EXT_blend_minmax");
   if (gGLState.suppPalettedTexture)    Con::printf("  EXT_paletted_texture");
   if (gGLState.suppLockedArrays)       Con::printf("  EXT_compiled_vertex_array");
   if (gGLState.suppVertexArrayRange)   Con::printf("  NV_vertex_array_range");
   if (gGLState.suppTextureEnvCombine)  Con::printf("  EXT_texture_env_combine");
   if (gGLState.suppPackedPixels)       Con::printf("  EXT_packed_pixels");
   if (gGLState.suppFogCoord)           Con::printf("  EXT_fog_coord");
   if (gGLState.suppTextureCompression) Con::printf("  ARB_texture_compression");
   if (gGLState.suppS3TC)               Con::printf("  EXT_texture_compression_s3tc");
   if (gGLState.suppFXT1)               Con::printf("  3DFX_texture_compression_FXT1");
   if (gGLState.suppTexEnvAdd)          Con::printf("  (ARB|EXT)_texture_env_add");
   if (gGLState.suppTexAnisotropic)     Con::printf("  EXT_texture_filter_anisotropic (Max anisotropy: %f)", gGLState.maxAnisotropy);
   if (gGLState.suppSwapInterval)       Con::printf("  WGL_EXT_swap_control");

   Con::warnf("OpenGL Init: Disabled Extensions");
   if (!gGLState.suppARBMultitexture)    Con::warnf("  ARB_multitexture");
   if (!gGLState.suppEXTblendcolor)      Con::warnf("  EXT_blend_color");
   if (!gGLState.suppEXTblendminmax)     Con::warnf("  EXT_blend_minmax");
   if (!gGLState.suppPalettedTexture)    Con::warnf("  EXT_paletted_texture");
   if (!gGLState.suppLockedArrays)       Con::warnf("  EXT_compiled_vertex_array");
   if (!gGLState.suppVertexArrayRange)   Con::warnf("  NV_vertex_array_range");
   if (!gGLState.suppTextureEnvCombine)  Con::warnf("  EXT_texture_env_combine");
   if (!gGLState.suppPackedPixels)       Con::warnf("  EXT_packed_pixels");
   if (!gGLState.suppFogCoord)           Con::warnf("  EXT_fog_coord");
   if (!gGLState.suppTextureCompression) Con::warnf("  ARB_texture_compression");
   if (!gGLState.suppS3TC)               Con::warnf("  EXT_texture_compression_s3tc");
   if (!gGLState.suppFXT1)               Con::warnf("  3DFX_texture_compression_FXT1");
   if (!gGLState.suppTexEnvAdd)          Con::warnf("  (ARB|EXT)_texture_env_add");
   if (!gGLState.suppTexAnisotropic)     Con::warnf("  EXT_texture_filter_anisotropic");
   if (!gGLState.suppSwapInterval)       Con::warnf("  WGL_EXT_swap_control");
   Con::printf(" ");

   // Set some console variables:
   Con::setBoolVariable( "$FogCoordSupported", gGLState.suppFogCoord );
   Con::setBoolVariable( "$TextureCompressionSupported", gGLState.suppTextureCompression );
   Con::setBoolVariable( "$AnisotropySupported", gGLState.suppTexAnisotropic );
   Con::setBoolVariable( "$PalettedTextureSupported", gGLState.suppPalettedTexture );
   Con::setBoolVariable( "$SwapIntervalSupported", gGLState.suppSwapInterval );

   if (!gGLState.suppPalettedTexture && Con::getBoolVariable("$pref::OpenGL::forcePalettedTexture",false))
   {
      Con::setBoolVariable("$pref::OpenGL::forcePalettedTexture", false);
      Con::setBoolVariable("$pref::OpenGL::force16BitTexture", true);
   }

   return true;
}

#endif // 0