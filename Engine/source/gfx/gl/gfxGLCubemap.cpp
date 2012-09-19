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

#include "gfx/gl/gfxGLDevice.h"
#include "gfx/gl/gfxGLTextureObject.h"
#include "gfx/gl/gfxGLEnumTranslate.h"
#include "gfx/gl/gfxGLUtils.h"
#include "gfx/gl/gfxGLCubemap.h"
#include "gfx/gfxTextureManager.h"
#include "gfx/gfxCardProfile.h"
#include "gfx/bitmap/DDSFile.h"


GLenum GFXGLCubemap::faceList[6] = 
{ 
   GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
   GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
   GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

GFXGLCubemap::GFXGLCubemap() :
      mCubemap(0), 
      mDynamicTexSize(0),
      mFaceFormat( GFXFormatR8G8B8A8 )
{
   for(U32 i = 0; i < 6; i++)
      mTextures[i] = NULL;
   
   GFXTextureManager::addEventDelegate( this, &GFXGLCubemap::_onTextureEvent );
}

GFXGLCubemap::~GFXGLCubemap()
{
   glDeleteTextures(1, &mCubemap);
   GFXTextureManager::removeEventDelegate( this, &GFXGLCubemap::_onTextureEvent );
}

void GFXGLCubemap::fillCubeTextures(GFXTexHandle* faces)
{
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_CUBE_MAP, mCubemap);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_TRUE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
   
   U32 reqWidth = faces[0]->getWidth();
   U32 reqHeight = faces[0]->getHeight();
   GFXFormat regFaceFormat = faces[0]->getFormat();
   mWidth = reqWidth;
   mHeight = reqHeight;
   mFaceFormat = regFaceFormat;
   mMipLevels = 1; // Lie for now
   AssertFatal(reqWidth == reqHeight, "GFXGLCubemap::fillCubeTextures - Width and height must be equal!");
   
   for(U32 i = 0; i < 6; i++)
   {
      AssertFatal(faces[i], avar("GFXGLCubemap::fillCubeFaces - texture %i is NULL!", i));
      AssertFatal((faces[i]->getWidth() == reqWidth) && (faces[i]->getHeight() == reqHeight), "GFXGLCubemap::fillCubeFaces - All textures must have identical dimensions!");
      AssertFatal(faces[i]->getFormat() == regFaceFormat, "GFXGLCubemap::fillCubeFaces - All textures must have identical formats!");
      
      mTextures[i] = faces[i];
      GFXFormat faceFormat = faces[i]->getFormat();

      GFXGLTextureObject* glTex = static_cast<GFXGLTextureObject*>(faces[i].getPointer());
      U8* buf = glTex->getTextureData();
      glTexImage2D(faceList[i], 0, GFXGLTextureInternalFormat[faceFormat], faces[i]->getWidth(), faces[i]->getHeight(), 
         0, GFXGLTextureFormat[faceFormat], GFXGLTextureType[faceFormat], buf);
      delete[] buf;
   }
   
   glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
   glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
}

void GFXGLCubemap::initStatic(GFXTexHandle* faces)
{
   if(mCubemap)
      return;
      
   if(faces)
   {
      AssertFatal(faces[0], "GFXGLCubemap::initStatic - empty texture passed");
      glGenTextures(1, &mCubemap);
      fillCubeTextures(faces);
   }
}

void GFXGLCubemap::initStatic( DDSFile *dds )
{
   if(mCubemap)
      return;
      
   AssertFatal( dds, "GFXGLCubemap::initStatic - Got null DDS file!" );
   AssertFatal( dds->isCubemap(), "GFXGLCubemap::initStatic - Got non-cubemap DDS file!" );
   AssertFatal( dds->mSurfaces.size() == 6, "GFXGLCubemap::initStatic - DDS has less than 6 surfaces!" );

   // HACK: I cannot put the genie back in the bottle and assign a
   // DDSFile pointer back to a Resource<>.  
   //
   // So we do a second lookup which works out ok for now, but shows
   // the weakness in the ResourceManager not having a common base 
   // reference type.
   //
   mDDSFile = ResourceManager::get().load( dds->getSourcePath() );
   AssertFatal( mDDSFile == dds, "GFXGLCubemap::initStatic - Couldn't find DDSFile resource!" );

   glGenTextures(1, &mCubemap);

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_CUBE_MAP, mCubemap);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_TRUE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
   
   mWidth = dds->getWidth();
   mHeight = dds->getHeight();
   mFaceFormat = dds->getFormat();
   mMipLevels = 1;

   // TODO: Support mipmaps here as well as decompressing the 
   // DDS if the format is unsupported.

   AssertFatal(mWidth == mHeight, "GFXGLCubemap::initStatic - Width and height must be equal!");
   
   for(U32 i = 0; i < 6; i++)
   {
      if ( !dds->mSurfaces[i] )
      {
         // TODO: The DDS can skip surfaces, but i'm unsure what i should
         // do here when creating the cubemap.  Ignore it for now.
         continue;
      }

      const U8 *buffer = dds->mSurfaces[i]->mMips[0];
      U32 surfaceSize = dds->getSurfaceSize( mHeight, mWidth, i );

      glCompressedTexImage2D( faceList[i], 0, GFXGLTextureInternalFormat[mFaceFormat], 
                              mWidth, mHeight, 0, surfaceSize, buffer );
   }
   
   glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
   glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
}

void GFXGLCubemap::initDynamic(U32 texSize, GFXFormat faceFormat)
{
   mDynamicTexSize = texSize;
   mFaceFormat = faceFormat;

   glGenTextures(1, &mCubemap);
   glBindTexture(GL_TEXTURE_CUBE_MAP, mCubemap);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
   mWidth = texSize;
   mHeight = texSize;
   mMipLevels = 1;
   for(U32 i = 0; i < 6; i++)
   {
      glTexImage2D(  faceList[i], 0, GFXGLTextureInternalFormat[faceFormat], texSize, texSize, 
                     0, GFXGLTextureFormat[faceFormat], GFXGLTextureType[faceFormat], NULL);
   }
   glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void GFXGLCubemap::zombify()
{
   glDeleteTextures(1, &mCubemap);
   mCubemap = 0;
}

void GFXGLCubemap::resurrect()
{
   // Handled in tmResurrect
}

void GFXGLCubemap::tmResurrect()
{
   if(mDynamicTexSize)
      initDynamic(mDynamicTexSize,mFaceFormat);
   else
   {
      if ( mDDSFile )
         initStatic( mDDSFile );
      else
         initStatic( mTextures );
   }
}

void GFXGLCubemap::setToTexUnit(U32 tuNum)
{
   static_cast<GFXGLDevice*>(getOwningDevice())->setCubemapInternal(tuNum, this);
}

void GFXGLCubemap::bind(U32 textureUnit) const
{
   glActiveTexture(GL_TEXTURE0 + textureUnit);
   glBindTexture(GL_TEXTURE_CUBE_MAP, mCubemap);
   
   GFXGLStateBlockRef sb = static_cast<GFXGLDevice*>(GFX)->getCurrentStateBlock();
   AssertFatal(sb, "GFXGLCubemap::bind - No active stateblock!");
   if (!sb)
      return;   
      
   const GFXSamplerStateDesc& ssd = sb->getDesc().samplers[textureUnit];
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minificationFilter(ssd.minFilter, ssd.mipFilter, 0));   
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GFXGLTextureFilter[ssd.magFilter]);   
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GFXGLTextureAddress[ssd.addressModeU]);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GFXGLTextureAddress[ssd.addressModeV]);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GFXGLTextureAddress[ssd.addressModeW]);

   glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, ssd.mipLODBias);
}

void GFXGLCubemap::_onTextureEvent( GFXTexCallbackCode code )
{
   if ( code == GFXZombify )
      zombify();
   else
      tmResurrect();
}
