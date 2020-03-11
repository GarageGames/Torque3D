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

#include "platform/platform.h"
#include "gfx/gl/gfxGLTextureManager.h"
#include "gfx/gl/gfxGLEnumTranslate.h"
#include "gfx/gfxCardProfile.h"
#include "core/util/safeDelete.h"
#include "gfx/gl/gfxGLUtils.h"

#include <squish.h>

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
GFXGLTextureManager::GFXGLTextureManager()
{
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
GFXGLTextureManager::~GFXGLTextureManager()
{
   SAFE_DELETE_ARRAY( mHashTable );
}

//-----------------------------------------------------------------------------
// createTexture
//-----------------------------------------------------------------------------
GFXTextureObject *GFXGLTextureManager::_createTextureObject(   U32 height, 
                                                               U32 width,
                                                               U32 depth,
                                                               GFXFormat format, 
                                                               GFXTextureProfile *profile, 
                                                               U32 numMipLevels,
                                                               bool forceMips,
                                                               S32 antialiasLevel,
                                                               GFXTextureObject *inTex )
{
   AssertFatal(format >= 0 && format < GFXFormat_COUNT, "GFXGLTextureManager::_createTexture - invalid format!");

   GFXGLTextureObject *retTex;
   if ( inTex )
   {
      AssertFatal( dynamic_cast<GFXGLTextureObject*>( inTex ), "GFXGLTextureManager::_createTexture() - Bad inTex type!" );
      retTex = static_cast<GFXGLTextureObject*>( inTex );
      retTex->release();
      retTex->reInit();
   }      
   else
   {
      retTex = new GFXGLTextureObject( GFX, profile );
      retTex->registerResourceWithDevice( GFX );
   }

   innerCreateTexture(retTex, height, width, depth, format, profile, numMipLevels, forceMips);

   return retTex;
}

//-----------------------------------------------------------------------------
// innerCreateTexture
//-----------------------------------------------------------------------------
// This just creates the texture, no info is actually loaded to it.  We do that later.
void GFXGLTextureManager::innerCreateTexture( GFXGLTextureObject *retTex, 
                                               U32 height, 
                                               U32 width, 
                                               U32 depth,
                                               GFXFormat format, 
                                               GFXTextureProfile *profile, 
                                               U32 numMipLevels,
                                               bool forceMips)
{
   // No 24 bit formats.  They trigger various oddities because hardware (and Apple's drivers apparently...) don't natively support them.
   if (format == GFXFormatR8G8B8)
      format = GFXFormatR8G8B8A8;
   else if (format == GFXFormatR8G8B8_SRGB)
      format = GFXFormatR8G8B8A8_SRGB;
      
   retTex->mFormat = format;
   retTex->mIsZombie = false;
   retTex->mIsNPoT2 = false;
   
   GLenum binding = ( (height == 1 || width == 1) && ( height != width ) ) ? GL_TEXTURE_1D : ( (depth == 0) ? GL_TEXTURE_2D : GL_TEXTURE_3D );
   if((profile->testFlag(GFXTextureProfile::RenderTarget) || profile->testFlag(GFXTextureProfile::ZTarget)) && (!isPow2(width) || !isPow2(height)) && !depth)
      retTex->mIsNPoT2 = true;
   retTex->mBinding = binding;
   
   // Bind it
   PRESERVE_TEXTURE(binding);
   glBindTexture(retTex->getBinding(), retTex->getHandle());
   
   // Create it
   // @todo OPENGL - Creating mipmaps for compressed formats. Not supported on OpenGL ES and bugged on AMD. We use mipmaps present on file.
   if( forceMips && !retTex->mIsNPoT2 && !ImageUtil::isCompressedFormat(format) )
   {
      retTex->mMipLevels = numMipLevels > 1 ? numMipLevels : 0;
   }
   else if(profile->testFlag(GFXTextureProfile::NoMipmap) || profile->testFlag(GFXTextureProfile::RenderTarget) || numMipLevels == 1 || retTex->mIsNPoT2)
   {
      retTex->mMipLevels = 1;
   }
   else
   {
      retTex->mMipLevels = numMipLevels;
   }

   // @todo OPENGL - OpenGL ES2 not support mipmaps on NPOT textures
#if 0
   if(!retTex->mIsNPoT2)
   {
      if(!isPow2(width))
         width = getNextPow2(width);
      if(!isPow2(height))
         height = getNextPow2(height);
      if(depth && !isPow2(depth))
         depth = getNextPow2(depth);
   }
#endif
   
   AssertFatal(GFXGLTextureInternalFormat[format] != GL_ZERO, "GFXGLTextureManager::innerCreateTexture - invalid internal format");
   AssertFatal(GFXGLTextureFormat[format] != GL_ZERO, "GFXGLTextureManager::innerCreateTexture - invalid format");
   AssertFatal(GFXGLTextureType[format] != GL_ZERO, "GFXGLTextureManager::innerCreateTexture - invalid type");

   //calculate num mipmaps
   if(retTex->mMipLevels == 0)
      retTex->mMipLevels = getMaxMipmaps(width, height, 1);

    glTexParameteri(binding, GL_TEXTURE_MAX_LEVEL, retTex->mMipLevels-1 );
    
    if( GFXGL->mCapabilities.textureStorage )
    {
        if(binding == GL_TEXTURE_2D)
            glTexStorage2D( retTex->getBinding(), retTex->mMipLevels, GFXGLTextureInternalFormat[format], width, height );
        else if(binding == GL_TEXTURE_1D)
            glTexStorage1D( retTex->getBinding(), retTex->mMipLevels, GFXGLTextureInternalFormat[format], getMax(width, height) );
        else
            glTexStorage3D( retTex->getBinding(), retTex->mMipLevels, GFXGLTextureInternalFormat[format], width, height, depth );
    }
    else
    {
        //If it wasn't for problems on amd drivers this next part could be really simplified and we wouldn't need to go through manually creating our
        //mipmap pyramid and instead just use glGenerateMipmap
        if(ImageUtil::isCompressedFormat(format))
        {
            AssertFatal(binding == GL_TEXTURE_2D, 
            "GFXGLTextureManager::innerCreateTexture - Only compressed 2D textures are supported");

            U32 tempWidth = width;
            U32 tempHeight = height;
            U32 size = getCompressedSurfaceSize(format,height,width);
            //Fill compressed images with 0's
            U8 *pTemp = (U8*)dMalloc(sizeof(U8)*size);
            dMemset(pTemp,0,size);
     
            for(U32 i=0;i< retTex->mMipLevels;i++)
            {
                tempWidth = getMax( U32(1), width >> i );
                tempHeight = getMax( U32(1), height >> i );
                size = getCompressedSurfaceSize(format,width,height,i);
                glCompressedTexImage2D(binding,i,GFXGLTextureInternalFormat[format],tempWidth,tempHeight,0,size,pTemp);
            }

            dFree(pTemp);
        }
        else
        {   
            if(binding == GL_TEXTURE_2D)
                glTexImage2D(binding, 0, GFXGLTextureInternalFormat[format], width, height, 0, GFXGLTextureFormat[format], GFXGLTextureType[format], NULL);
            else if(binding == GL_TEXTURE_1D)
                glTexImage1D(binding, 0, GFXGLTextureInternalFormat[format], (width > 1 ? width : height), 0, GFXGLTextureFormat[format], GFXGLTextureType[format], NULL);
            else
                glTexImage3D(GL_TEXTURE_3D, 0, GFXGLTextureInternalFormat[format], width, height, depth, 0, GFXGLTextureFormat[format], GFXGLTextureType[format], NULL);

            if(retTex->mMipLevels > 1)
                glGenerateMipmap(binding);
        }
    }
   
   // Complete the texture
   // Complete the texture - this does get changed later but we need to complete the texture anyway

   if(retTex->mMipLevels == 1)
      glTexParameteri(binding, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   else
      glTexParameteri(binding, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glTexParameteri(binding, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(binding, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(binding, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   if(binding == GL_TEXTURE_3D)
      glTexParameteri(binding, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

   if(GFXGLTextureSwizzle[format])         
      glTexParameteriv(binding, GL_TEXTURE_SWIZZLE_RGBA, GFXGLTextureSwizzle[format]);   
   
   // Get the size from GL (you never know...)
   GLint texHeight, texWidth, texDepth = 0;
   
   glGetTexLevelParameteriv(binding, 0, GL_TEXTURE_WIDTH, &texWidth);
   glGetTexLevelParameteriv(binding, 0, GL_TEXTURE_HEIGHT, &texHeight);
   if(binding == GL_TEXTURE_3D)
      glGetTexLevelParameteriv(binding, 0, GL_TEXTURE_DEPTH, &texDepth);
   
   retTex->mTextureSize.set(texWidth, texHeight, texDepth);
}

//-----------------------------------------------------------------------------
// loadTexture - GBitmap
//-----------------------------------------------------------------------------

static void _textureUpload(const S32 width, const S32 height,const S32 bytesPerPixel,const GFXGLTextureObject* texture, const GFXFormat fmt, const U8* data,const S32 mip=0, Swizzle<U8, 4> *pSwizzle = NULL)
{
   glBindBuffer(GL_PIXEL_UNPACK_BUFFER, texture->getBuffer());
   U32 bufSize = width * height * bytesPerPixel;
   glBufferData(GL_PIXEL_UNPACK_BUFFER, bufSize, NULL, GL_STREAM_DRAW);

   if(pSwizzle)
   {
      PROFILE_SCOPE(Swizzle32_Upload);
      U8* pboMemory = (U8*)dMalloc(bufSize);
      pSwizzle->ToBuffer(pboMemory, data, bufSize);
      glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, bufSize, pboMemory);
      dFree(pboMemory);
   }
   else
   {
      PROFILE_SCOPE(SwizzleNull_Upload);
      glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, bufSize, data);
   }

   if (texture->getBinding() == GL_TEXTURE_2D)
      glTexSubImage2D(texture->getBinding(), mip, 0, 0, width, height, GFXGLTextureFormat[fmt], GFXGLTextureType[fmt], NULL);
   else
      glTexSubImage1D(texture->getBinding(), mip, 0, (width > 1 ? width : height), GFXGLTextureFormat[fmt], GFXGLTextureType[fmt], NULL);

   glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

bool GFXGLTextureManager::_loadTexture(GFXTextureObject *aTexture, GBitmap *pDL)
{
   PROFILE_SCOPE(GFXGLTextureManager_loadTexture);
   GFXGLTextureObject *texture = static_cast<GFXGLTextureObject*>(aTexture);
   
   AssertFatal(texture->getBinding() == GL_TEXTURE_1D || texture->getBinding() == GL_TEXTURE_2D, 
      "GFXGLTextureManager::_loadTexture(GBitmap) - This method can only be used with 1D/2D textures");
      
   if(texture->getBinding() == GL_TEXTURE_3D)
      return false;
         
   // No 24bit formats.
   if(pDL->getFormat() == GFXFormatR8G8B8)
      pDL->setFormat(GFXFormatR8G8B8A8);
   else if (pDL->getFormat() == GFXFormatR8G8B8_SRGB)
      pDL->setFormat(GFXFormatR8G8B8A8_SRGB);
   // Bind to edit
   PRESERVE_TEXTURE(texture->getBinding());
   glBindTexture(texture->getBinding(), texture->getHandle());

  _textureUpload(pDL->getWidth(),pDL->getHeight(),pDL->getBytesPerPixel(),texture,pDL->getFormat(), pDL->getBits(), 0);

  if(!ImageUtil::isCompressedFormat(pDL->getFormat()))
   glGenerateMipmap(texture->getBinding());
   
   return true;
}

bool GFXGLTextureManager::_loadTexture(GFXTextureObject *aTexture, DDSFile *dds)
{
   GFXGLTextureObject* texture = static_cast<GFXGLTextureObject*>(aTexture);
   
   AssertFatal(texture->getBinding() == GL_TEXTURE_2D, 
      "GFXGLTextureManager::_loadTexture(DDSFile) - This method can only be used with 2D textures");
      
   if(texture->getBinding() != GL_TEXTURE_2D)
      return false;
   
   PRESERVE_TEXTURE(texture->getBinding());
   glBindTexture(texture->getBinding(), texture->getHandle());
   U32 numMips = dds->mSurfaces[0]->mMips.size();
   const GFXFormat fmt = texture->mFormat;

   for(U32 i = 0; i < numMips; i++)
   {
      PROFILE_SCOPE(GFXGLTexMan_loadSurface);

      if(ImageUtil::isCompressedFormat(texture->mFormat))
      {
         if((!isPow2(dds->getWidth()) || !isPow2(dds->getHeight())) && GFX->getCardProfiler()->queryProfile("GL::Workaround::noCompressedNPoTTextures"))
         {
            U8* uncompressedTex = new U8[dds->getWidth(i) * dds->getHeight(i) * 4];
            ImageUtil::decompress(dds->mSurfaces[0]->mMips[i],uncompressedTex, dds->getWidth(i), dds->getHeight(i), fmt);
            glTexSubImage2D(texture->getBinding(), i, 0, 0, dds->getWidth(i), dds->getHeight(i), GL_RGBA, GL_UNSIGNED_BYTE, uncompressedTex);
            delete[] uncompressedTex;
         }
         else
            glCompressedTexSubImage2D(texture->getBinding(), i, 0, 0, dds->getWidth(i), dds->getHeight(i), GFXGLTextureInternalFormat[fmt], dds->getSurfaceSize(dds->getHeight(), dds->getWidth(), i), dds->mSurfaces[0]->mMips[i]);
      }
      else
      {
         Swizzle<U8, 4> *pSwizzle = NULL;
         if (fmt == GFXFormatR8G8B8A8 || fmt == GFXFormatR8G8B8X8 || fmt == GFXFormatR8G8B8A8_SRGB || fmt == GFXFormatR8G8B8A8_LINEAR_FORCE || fmt == GFXFormatB8G8R8A8)
            pSwizzle = &Swizzles::bgra;

         _textureUpload(dds->getWidth(i), dds->getHeight(i),dds->mBytesPerPixel, texture, fmt, dds->mSurfaces[0]->mMips[i],i, pSwizzle);
      }
   }

   if(numMips !=1 && !ImageUtil::isCompressedFormat(texture->mFormat))
      glGenerateMipmap(texture->getBinding());
   
   return true;
}

bool GFXGLTextureManager::_loadTexture(GFXTextureObject *aTexture, void *raw)
{
   PROFILE_SCOPE(GFXGLTextureManager_loadTextureRaw);
   if(aTexture->getDepth() < 1)
      return false;
   
   GFXGLTextureObject* texture = static_cast<GFXGLTextureObject*>(aTexture);
   
   PRESERVE_3D_TEXTURE();
   glBindTexture(texture->getBinding(), texture->getHandle());
   glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, texture->getWidth(), texture->getHeight(), texture->getDepth(), GFXGLTextureFormat[texture->mFormat], GFXGLTextureType[texture->mFormat], raw);
   
   return true;
}

bool GFXGLTextureManager::_freeTexture(GFXTextureObject *texture, bool zombify /*= false*/)
{
   if(zombify)
      static_cast<GFXGLTextureObject*>(texture)->zombify();
   else
      static_cast<GFXGLTextureObject*>(texture)->release();
      
   return true;
}

bool GFXGLTextureManager::_refreshTexture(GFXTextureObject *texture)
{
   U32 usedStrategies = 0;
   GFXGLTextureObject* realTex = static_cast<GFXGLTextureObject*>(texture);
      
   if(texture->mProfile->doStoreBitmap())
   {
      if(realTex->isZombie())
      {
         realTex->resurrect();
         innerCreateTexture(realTex, texture->getHeight(), texture->getWidth(), texture->getDepth(), texture->mFormat, texture->mProfile, texture->mMipLevels);
      }
      if(texture->mBitmap)
         _loadTexture(texture, texture->mBitmap);
      
      if(texture->mDDS)
         return false;
      
      usedStrategies++;
   }
   
   if(texture->mProfile->isRenderTarget() || texture->mProfile->isDynamic() || texture->mProfile->isZTarget() || !usedStrategies)
   {
      realTex->release();
      realTex->resurrect();
      innerCreateTexture(realTex, texture->getHeight(), texture->getWidth(), texture->getDepth(), texture->mFormat, texture->mProfile, texture->mMipLevels);
      realTex->reloadFromCache();
      usedStrategies++;
   }
   
   AssertFatal(usedStrategies < 2, "GFXGLTextureManager::_refreshTexture - Inconsistent profile flags (store bitmap and dynamic/target");
   
   return true;
}
