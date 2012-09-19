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

#ifndef _GFXGLTEXTUREOBJECT_H
#define _GFXGLTEXTUREOBJECT_H

#include "gfx/gfxTextureObject.h"
#include "gfx/gl/ggl/ggl.h"

class GFXGLDevice;

class GFXGLTextureObject : public GFXTextureObject 
{
public:
   GFXGLTextureObject(GFXDevice * aDevice, GFXTextureProfile *profile); 
   virtual ~GFXGLTextureObject();
   
   void release();
   
   inline GLuint getHandle() const { return mHandle; }
   inline GLenum getBinding() const { return mBinding; }
   inline GLuint getBuffer() const { return mBuffer; }
   
   inline bool isZombie() const { return mIsZombie; }

   /// Binds the texture to the given texture unit
   /// and applies the current sampler state because GL tracks
   /// filtering and wrapper per object, while GFX tracks per sampler.
   void bind(U32 textureUnit) const;
   
   /// @return An array containing the texture data
   /// @note You are responsible for deleting the returned data! (Use delete[])
   U8* getTextureData();

   virtual F32 getMaxUCoord() const;
   virtual F32 getMaxVCoord() const;
   
   void reloadFromCache(); ///< Reloads texture from zombie cache, used by GFXGLTextureManager to resurrect the texture.
   
#ifdef TORQUE_DEBUG
   virtual void pureVirtualCrash() {}
#endif

   /// Get/set data from texture (for dynamic textures and render targets)
   /// @attention DO NOT READ FROM THE RETURNED RECT! It is not guaranteed to work and may incur significant performance penalties.
   virtual GFXLockedRect* lock(U32 mipLevel = 0, RectI *inRect = NULL);
   virtual void unlock(U32 mipLevel = 0 );

   virtual bool copyToBmp(GBitmap *); ///< Not implemented
   
   bool mIsNPoT2;

   // GFXResource interface
   virtual void zombify();
   virtual void resurrect();
   virtual const String describeSelf() const;
   
private:
   friend class GFXGLTextureManager;
   typedef GFXTextureObject Parent;
   /// Internal GL object
   GLuint mHandle;
   GLuint mBuffer;

   GLenum mBinding;
   
   U32 mBytesPerTexel;
   GFXLockedRect mLockedRect;
   RectI mLockedRectRect;

   /// Pointer to owner device
   GFXGLDevice* mGLDevice;
   
   bool mIsZombie;
   U8* mZombieCache;
   
   void copyIntoCache();
};

#endif