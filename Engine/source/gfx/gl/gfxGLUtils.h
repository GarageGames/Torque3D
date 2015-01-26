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

#ifndef TORQUE_GFX_GL_GFXGLUTILS_H_
#define TORQUE_GFX_GL_GFXGLUTILS_H_

#include "core/util/preprocessorHelpers.h"
#include "gfx/gl/gfxGLEnumTranslate.h"
#include "gfx/gl/gfxGLStateCache.h"

inline U32 getMaxMipmaps(U32 width, U32 height, U32 depth)
{
   return getMax( getBinLog2(depth), getMax(getBinLog2(width), getBinLog2(height)));
}

inline GLenum minificationFilter(U32 minFilter, U32 mipFilter, U32 /*mipLevels*/)
{
   // the compiler should interpret this as array lookups
   switch( minFilter ) 
   {
      case GFXTextureFilterLinear:
         switch( mipFilter ) 
         {
         case GFXTextureFilterLinear:
            return GL_LINEAR_MIPMAP_LINEAR;
         case GFXTextureFilterPoint:
            return GL_LINEAR_MIPMAP_NEAREST;
         default: 
            return GL_LINEAR;
         }
      default:
         switch( mipFilter ) {
      case GFXTextureFilterLinear:
         return GL_NEAREST_MIPMAP_LINEAR;
      case GFXTextureFilterPoint:
         return GL_NEAREST_MIPMAP_NEAREST;
      default:
         return GL_NEAREST;
         }
   }
}

// Check if format is compressed format.
// Even though dxt2/4 are not supported, they are included because they are a compressed format.
// Assert checks on supported formats are done elsewhere.
inline bool isCompressedFormat( GFXFormat format )
{
   bool compressed = false;
   if(format == GFXFormatDXT1 || format == GFXFormatDXT2
         || format == GFXFormatDXT3
         || format == GFXFormatDXT4
         || format == GFXFormatDXT5 )
   {
      compressed = true;
   }

   return compressed;
}

//Get the surface size of a compressed mip map level - see ddsLoader.cpp
inline U32 getCompressedSurfaceSize(GFXFormat format,U32 width, U32 height, U32 mipLevel=0 )
{
   if(!isCompressedFormat(format))
      return 0;

   // Bump by the mip level.
   height = getMax(U32(1), height >> mipLevel);
   width = getMax(U32(1), width >> mipLevel);

   U32 sizeMultiple = 0;
   if(format == GFXFormatDXT1)
      sizeMultiple = 8;
   else
      sizeMultiple = 16;

   return getMax(U32(1), width/4) * getMax(U32(1), height/4) * sizeMultiple;
}

/// Simple class which preserves a given GL integer.
/// This class determines the integer to preserve on construction and restores 
/// it on destruction.
class GFXGLPreserveInteger
{
public:
   typedef void(STDCALL *BindFn)(GLenum, GLuint);

   /// Preserve the integer.
   /// @param binding The binding which should be set on destruction.
   /// @param getBinding The parameter to be passed to glGetIntegerv to determine
   /// the integer to be preserved.
   /// @param binder The gl function to call to restore the integer.
   GFXGLPreserveInteger(GLenum binding, GLint getBinding, BindFn binder) :
      mBinding(binding), mPreserved(0), mBinder(binder)
   {
      AssertFatal(mBinder, "GFXGLPreserveInteger - Need a valid binder function");
      mPreserved = GFXGL->getOpenglCache()->getCacheBinded(mBinding);
#if defined(TORQUE_DEBUG) && defined(TORQUE_DEBUG_GFX)
      GLint bindedOnOpenglDriver;
      glGetIntegerv(getBinding, &bindedOnOpenglDriver);
      AssertFatal( mPreserved == bindedOnOpenglDriver, "GFXGLPreserveInteger - GFXGLDevice/OpenGL mismatch on cache binded resource.");
#endif
   }
   
   /// Restores the integer.
   ~GFXGLPreserveInteger()
   {
      mBinder(mBinding, mPreserved);
   }

private:
   GLenum mBinding;
   GLint mPreserved;
   BindFn mBinder;
};

class GFXGLPreserveTexture
{
public:
   typedef void(STDCALL *BindFn)(GLenum, GLuint);
   
   GFXGLPreserveTexture(GLenum binding, GLint getBinding, BindFn binder) :
      mBinding(binding), mPreserved(0), mBinder(binder)
   {
      AssertFatal(mBinder, "GFXGLPreserveTexture - Need a valid binder function");
      GFXGLDevice *gfx = GFXGL;
      mPreserved = gfx->getOpenglCache()->getCacheBinded(mBinding);
      mActiveTexture = gfx->getOpenglCache()->getCacheActiveTexture();
#if defined(TORQUE_DEBUG) && defined(TORQUE_DEBUG_GFX)
      GLint activeTextureOnOpenglDriver, bindedTextureOnOpenglDriver;
      glGetIntegerv(getBinding, &bindedTextureOnOpenglDriver);
      glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTextureOnOpenglDriver);
      activeTextureOnOpenglDriver -= GL_TEXTURE0;
      AssertFatal( mPreserved == bindedTextureOnOpenglDriver, "GFXGLPreserveTexture - GFXGLDevice/OpenGL mismatch on cache binded resource.");
      AssertFatal( activeTextureOnOpenglDriver == mActiveTexture, "GFXGLPreserveTexture - GFXGLDevice/OpenGL mismatch on cache binded resource.");
#endif
   }
   
   /// Restores the texture.
   ~GFXGLPreserveTexture()
   {
#if defined(TORQUE_DEBUG) && defined(TORQUE_DEBUG_GFX)
      GLint activeTextureOnOpenglDriver;
      glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTextureOnOpenglDriver);
      activeTextureOnOpenglDriver -= GL_TEXTURE0;
      GLint cacheActiveTexture = GFXGL->getOpenglCache()->getCacheActiveTexture();
      AssertFatal( cacheActiveTexture == activeTextureOnOpenglDriver, "GFXGLPreserveTexture - GFXGLDevice/OpenGL mismatch on cache ActiveTexture.");
#endif
      mBinder(mBinding, mPreserved);
   }

private:
   GLenum mBinding;
   GLint mPreserved;
   BindFn mBinder;
   S16 mActiveTexture;
};

/// Helper macro to preserve the current VBO binding.
#define PRESERVE_VERTEX_BUFFER() \
GFXGLPreserveInteger TORQUE_CONCAT(preserve_, __LINE__) (GL_ARRAY_BUFFER, GL_ARRAY_BUFFER_BINDING, (GFXGLPreserveInteger::BindFn)glBindBuffer)

/// Helper macro to preserve the current element array binding.
#define PRESERVE_INDEX_BUFFER() \
GFXGLPreserveInteger TORQUE_CONCAT(preserve_, __LINE__) (GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER_BINDING, (GFXGLPreserveInteger::BindFn)glBindBuffer)

#define _GET_BUFFER_BINDING( BINDING ) \
BINDING == GL_ARRAY_BUFFER ? GL_ARRAY_BUFFER_BINDING : ( BINDING == GL_ELEMENT_ARRAY_BUFFER ?  GL_ELEMENT_ARRAY_BUFFER_BINDING : 0 )

/// Helper macro to preserve the current element array binding.
#define PRESERVE_BUFFER( BINDING ) \
GFXGLPreserveInteger TORQUE_CONCAT(preserve_, __LINE__) (BINDING, _GET_BUFFER_BINDING(BINDING), (GFXGLPreserveInteger::BindFn)glBindBuffer)

/// ASSERT: Never call glActiveTexture for a "bind to modify" or in a PRESERVER_TEXTURE MACRO scope.

/// Helper macro to preserve the current 1D texture binding.
#define PRESERVE_1D_TEXTURE() \
GFXGLPreserveTexture TORQUE_CONCAT(preserve_, __LINE__) (GL_TEXTURE_1D, GL_TEXTURE_BINDING_1D, (GFXGLPreserveInteger::BindFn)glBindTexture)

/// Helper macro to preserve the current 2D texture binding.
#define PRESERVE_2D_TEXTURE() \
GFXGLPreserveTexture TORQUE_CONCAT(preserve_, __LINE__) (GL_TEXTURE_2D, GL_TEXTURE_BINDING_2D, (GFXGLPreserveInteger::BindFn)glBindTexture)

/// Helper macro to preserve the current 3D texture binding.
#define PRESERVE_3D_TEXTURE() \
GFXGLPreserveTexture TORQUE_CONCAT(preserve_, __LINE__) (GL_TEXTURE_3D, GL_TEXTURE_BINDING_3D, (GFXGLPreserveInteger::BindFn)glBindTexture)

/// Helper macro to preserve the current 3D texture binding.
#define PRESERVE_CUBEMAP_TEXTURE() \
GFXGLPreserveTexture TORQUE_CONCAT(preserve_, __LINE__) (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BINDING_CUBE_MAP, (GFXGLPreserveInteger::BindFn)glBindTexture)

#define _GET_TEXTURE_BINDING(binding) \
binding == GL_TEXTURE_2D ? GL_TEXTURE_BINDING_2D : (binding == GL_TEXTURE_3D ?  GL_TEXTURE_BINDING_3D : GL_TEXTURE_BINDING_1D )

#define PRESERVE_TEXTURE(binding) \
GFXGLPreserveTexture TORQUE_CONCAT(preserve_, __LINE__) (binding, _GET_TEXTURE_BINDING(binding), (GFXGLPreserveInteger::BindFn)glBindTexture)

#define PRESERVE_FRAMEBUFFER() \
GFXGLPreserveInteger TORQUE_CONCAT(preserve_, __LINE__) (GL_READ_FRAMEBUFFER, GL_READ_FRAMEBUFFER_BINDING, (GFXGLPreserveInteger::BindFn)glBindFramebuffer);\
GFXGLPreserveInteger TORQUE_CONCAT(preserve2_, __LINE__) (GL_DRAW_FRAMEBUFFER, GL_DRAW_FRAMEBUFFER_BINDING, (GFXGLPreserveInteger::BindFn)glBindFramebuffer)


#if TORQUE_DEBUG

    // Handy macro for checking the status of a framebuffer.  Framebuffers can fail in 
    // all sorts of interesting ways, these are just the most common.  Further, no existing GL profiling 
    // tool catches framebuffer errors when the framebuffer is created, so we actually need this.
    #define CHECK_FRAMEBUFFER_STATUS()\
    {\
    GLenum status;\
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);\
    switch(status) {\
    case GL_FRAMEBUFFER_COMPLETE:\
    break;\
    case GL_FRAMEBUFFER_UNSUPPORTED:\
    AssertFatal(false, "Unsupported FBO");\
    break;\
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:\
    AssertFatal(false, "Incomplete FBO Attachment");\
    break;\
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:\
    AssertFatal(false, "Incomplete FBO Missing Attachment");\
    break;\
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:\
    AssertFatal(false, "Incomplete FBO Draw buffer");\
    break;\
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:\
    AssertFatal(false, "Incomplete FBO Read buffer");\
    break;\
    default:\
    /* programming error; will fail on all hardware */\
    AssertFatal(false, "Something really bad happened with an FBO");\
    }\
    }
#else
    #define CHECK_FRAMEBUFFER_STATUS()
#endif //TORQUE_DEBUG

#endif
