#ifndef GFX_GL_STATE_CACHE
#define GFX_GL_STATE_CACHE


/// GFXGLStateCache store OpenGL state to avoid performance penalities of glGet* calls
/// GL_TEXTURE_1D/2D/3D, GL_FRAMEBUFFER, GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER
class GFXGLStateCache
{
public:
   GFXGLStateCache()
   {      
      mActiveTexture = 0;      
      mBindedVBO = 0;
      mBindedIBO = 0;
      mBindedFBO_W = 0;
      mBindedFBO_R = 0;
      mVertexAttribActive = 0;
   }

   class TextureUnit
   {
   public:
      TextureUnit() :  mTexture1D(0), mTexture2D(0), mTexture3D(0), mTextureCube(0)
      {

      }
      GLuint mTexture1D, mTexture2D, mTexture3D, mTextureCube;
   };

   /// after glBindTexture
   void setCacheBindedTex(U32 texUnit, GLenum biding, GLuint handle)
   { 
      mActiveTexture = texUnit;
      switch (biding)
      {
      case GL_TEXTURE_2D:
         mTextureUnits[mActiveTexture].mTexture2D = handle;
         break;
      case GL_TEXTURE_3D:
         mTextureUnits[mActiveTexture].mTexture3D = handle;
         break;
      case GL_TEXTURE_1D:
         mTextureUnits[mActiveTexture].mTexture1D = handle;
         break;
      case GL_TEXTURE_CUBE_MAP:
         mTextureUnits[mActiveTexture].mTextureCube = handle;
         break;
      default:
         AssertFatal(0, avar("GFXGLStateCache::setCacheBindedTex - binding (%x) not supported.", biding) );
         return;
      }
   }

   /// after opengl object binded
   void setCacheBinded(GLenum biding, GLuint handle) 
   { 
      switch (biding)
      {
      case GL_TEXTURE_2D:
         mTextureUnits[mActiveTexture].mTexture2D = handle;
         break;
      case GL_TEXTURE_3D:
         mTextureUnits[mActiveTexture].mTexture3D = handle;
         break;
      case GL_TEXTURE_1D:
         mTextureUnits[mActiveTexture].mTexture1D = handle;
         break;
      case GL_TEXTURE_CUBE_MAP:
         mTextureUnits[mActiveTexture].mTextureCube = handle;
         break;
      case GL_FRAMEBUFFER:
         mBindedFBO_W = mBindedFBO_R = handle;
         break;
      case GL_DRAW_FRAMEBUFFER:
         mBindedFBO_W = handle;
         break;
      case GL_READ_FRAMEBUFFER:
         mBindedFBO_R = handle;
         break;
      case GL_ARRAY_BUFFER:
         mBindedVBO = handle;
         break;
      case GL_ELEMENT_ARRAY_BUFFER:
         mBindedIBO = handle;
         break;
      default:
         AssertFatal(0, avar("GFXGLStateCache::setCacheBinded - binding (%x) not supported.", biding) );
         break;
      }
   }

   GLuint getCacheBinded(GLenum biding) const
   {
      switch (biding)
      {
      case GL_TEXTURE_2D:
         return mTextureUnits[mActiveTexture].mTexture2D;
      case GL_TEXTURE_3D:
         return mTextureUnits[mActiveTexture].mTexture3D;
      case GL_TEXTURE_1D:
         return mTextureUnits[mActiveTexture].mTexture1D;
      case GL_TEXTURE_CUBE_MAP:
         return mTextureUnits[mActiveTexture].mTextureCube;
      case GL_DRAW_FRAMEBUFFER:
         return mBindedFBO_W;
      case GL_READ_FRAMEBUFFER:
         return mBindedFBO_R;
      case GL_ARRAY_BUFFER:
         return mBindedVBO;
      case GL_ELEMENT_ARRAY_BUFFER:
         return mBindedIBO;
      default:
         AssertFatal(0, avar("GFXGLStateCache::getCacheBinded - binding (%x) not supported.", biding) );
         return 0;
      }
   }

   /// after glActiveTexture
   void setCacheActiveTexture(U32 unit) { mActiveTexture = unit; }
   U32 getCacheActiveTexture() const { return mActiveTexture;  }

   /// for cache glEnableVertexAttribArray / glDisableVertexAttribArray
   void setCacheVertexAttribActive(U32 activeMask) { mVertexAttribActive = activeMask; }
   U32 getCacheVertexAttribActive() const { return mVertexAttribActive;  }

protected:   
   GLuint mActiveTexture, mBindedVBO, mBindedIBO, mBindedFBO_W, mBindedFBO_R;
   TextureUnit mTextureUnits[TEXTURE_STAGE_COUNT];
   U32 mVertexAttribActive;
};


#endif