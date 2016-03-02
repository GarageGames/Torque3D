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

#include "console/console.h"
#include "gfx/gl/gfxGLDevice.h"
#include "gfx/gl/gfxGLTextureTarget.h"
#include "gfx/gl/gfxGLTextureObject.h"
#include "gfx/gl/gfxGLCubemap.h"
#include "gfx/gfxTextureManager.h"
#include "gfx/gl/gfxGLUtils.h"

/// Internal struct used to track texture information for FBO attachments
/// This serves as an abstract base so we can deal with cubemaps and standard 
/// 2D/Rect textures through the same interface
class _GFXGLTargetDesc
{
public:
   _GFXGLTargetDesc(U32 _mipLevel, U32 _zOffset) :
      mipLevel(_mipLevel), zOffset(_zOffset)
   {
   }
   
   virtual ~_GFXGLTargetDesc() {}
   
   virtual U32 getHandle() = 0;
   virtual U32 getWidth() = 0;
   virtual U32 getHeight() = 0;
   virtual U32 getDepth() = 0;
   virtual bool hasMips() = 0;
   virtual GLenum getBinding() = 0;
   virtual GFXFormat getFormat() = 0;
   virtual bool isCompatible(const GFXGLTextureObject* tex) = 0;
   
   U32 getMipLevel() { return mipLevel; }
   U32 getZOffset() { return zOffset; }
   
private:
   U32 mipLevel;
   U32 zOffset;
};

/// Internal struct used to track 2D/Rect texture information for FBO attachment
class _GFXGLTextureTargetDesc : public _GFXGLTargetDesc
{
public:
   _GFXGLTextureTargetDesc(GFXGLTextureObject* tex, U32 _mipLevel, U32 _zOffset) 
      : _GFXGLTargetDesc(_mipLevel, _zOffset), mTex(tex)
   {
   }
   
   virtual ~_GFXGLTextureTargetDesc() {}
   
   virtual U32 getHandle() { return mTex->getHandle(); }
   virtual U32 getWidth() { return mTex->getWidth(); }
   virtual U32 getHeight() { return mTex->getHeight(); }
   virtual U32 getDepth() { return mTex->getDepth(); }
   virtual bool hasMips() { return mTex->mMipLevels != 1; }
   virtual GLenum getBinding() { return mTex->getBinding(); }
   virtual GFXFormat getFormat() { return mTex->getFormat(); }
   virtual bool isCompatible(const GFXGLTextureObject* tex)
   {
      return mTex->getFormat() == tex->getFormat()
         && mTex->getWidth() == tex->getWidth()
         && mTex->getHeight() == tex->getHeight();
   }
   GFXGLTextureObject* getTextureObject() const {return mTex; }
   
private:
   StrongRefPtr<GFXGLTextureObject> mTex;
};

/// Internal struct used to track Cubemap texture information for FBO attachment
class _GFXGLCubemapTargetDesc : public _GFXGLTargetDesc
{
public:
   _GFXGLCubemapTargetDesc(GFXGLCubemap* tex, U32 _face, U32 _mipLevel, U32 _zOffset) 
      : _GFXGLTargetDesc(_mipLevel, _zOffset), mTex(tex), mFace(_face)
   {
   }
   
   virtual ~_GFXGLCubemapTargetDesc() {}
   
   virtual U32 getHandle() { return mTex->getHandle(); }
   virtual U32 getWidth() { return mTex->getWidth(); }
   virtual U32 getHeight() { return mTex->getHeight(); }
   virtual U32 getDepth() { return 0; }
   virtual bool hasMips() { return mTex->getNumMipLevels() != 1; }
   virtual GLenum getBinding() { return GFXGLCubemap::getEnumForFaceNumber(mFace); }
   virtual GFXFormat getFormat() { return mTex->getFormat(); }
   virtual bool isCompatible(const GFXGLTextureObject* tex)
   {
      return mTex->getFormat() == tex->getFormat()
         && mTex->getWidth() == tex->getWidth()
         && mTex->getHeight() == tex->getHeight();
   }
   
private:
   StrongRefPtr<GFXGLCubemap> mTex;
   U32 mFace;
};

// Internal implementations
class _GFXGLTextureTargetImpl // TODO OPENGL remove and implement on GFXGLTextureTarget
{
public:
   GFXGLTextureTarget* mTarget;
   
   virtual ~_GFXGLTextureTargetImpl() {}
   
   virtual void applyState() = 0;
   virtual void makeActive() = 0;
   virtual void finish() = 0;
};

// Use FBOs to render to texture.  This is the preferred implementation and is almost always used.
class _GFXGLTextureTargetFBOImpl : public _GFXGLTextureTargetImpl
{
public:
   GLuint mFramebuffer;
   
   _GFXGLTextureTargetFBOImpl(GFXGLTextureTarget* target);
   virtual ~_GFXGLTextureTargetFBOImpl();
   
   virtual void applyState();
   virtual void makeActive();
   virtual void finish();
};

_GFXGLTextureTargetFBOImpl::_GFXGLTextureTargetFBOImpl(GFXGLTextureTarget* target)
{
   mTarget = target;
   glGenFramebuffers(1, &mFramebuffer);
}

_GFXGLTextureTargetFBOImpl::~_GFXGLTextureTargetFBOImpl()
{
   glDeleteFramebuffers(1, &mFramebuffer);
}

void _GFXGLTextureTargetFBOImpl::applyState()
{   
   // REMINDER: When we implement MRT support, check against GFXGLDevice::getNumRenderTargets()
   
   PRESERVE_FRAMEBUFFER();
   glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

   bool drawbufs[16];
   int bufsize = 0;
   for (int i = 0; i < 16; i++)
           drawbufs[i] = false;
   bool hasColor = false;
   for(int i = 0; i < GFXGL->getNumRenderTargets(); ++i)
   {   
      _GFXGLTargetDesc* color = mTarget->getTargetDesc( static_cast<GFXTextureTarget::RenderSlot>(GFXTextureTarget::Color0+i ));
      if(color)
      {
         hasColor = true;
         const GLenum binding = color->getBinding();
         if( binding == GL_TEXTURE_2D || (binding >= GL_TEXTURE_CUBE_MAP_POSITIVE_X && binding <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z) )
            glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, color->getBinding( ), color->getHandle( ), color->getMipLevel( ) );
         else if( binding == GL_TEXTURE_1D )
            glFramebufferTexture1D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, color->getBinding( ), color->getHandle( ), color->getMipLevel( ) );
         else if( binding == GL_TEXTURE_3D )
            glFramebufferTexture3D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, color->getBinding( ), color->getHandle( ), color->getMipLevel( ), color->getZOffset( ) );
         else
             Con::errorf("_GFXGLTextureTargetFBOImpl::applyState - Bad binding");
      }
      else
      {
         // Clears the texture (note that the binding is irrelevent)
         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, 0, 0);
      }
   }
   
   _GFXGLTargetDesc* depthStecil = mTarget->getTargetDesc(GFXTextureTarget::DepthStencil);
   if(depthStecil)
   {
      // Certain drivers have issues with depth only FBOs.  That and the next two asserts assume we have a color target.
      AssertFatal(hasColor, "GFXGLTextureTarget::applyState() - Cannot set DepthStencil target without Color0 target!");
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthStecil->getBinding(), depthStecil->getHandle(), depthStecil->getMipLevel());
   }
   else
   {
      // Clears the texture (note that the binding is irrelevent)
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
   }

   GLenum *buf = new GLenum[bufsize];
   int count = 0;
   for (int i = 0; i < bufsize; i++)
   {
           if (drawbufs[i])
           {
                   buf[count] = GL_COLOR_ATTACHMENT0 + i;
                   count++;
           }
   }
 
   glDrawBuffers(bufsize, buf);
 
   delete[] buf;
   CHECK_FRAMEBUFFER_STATUS();
}

void _GFXGLTextureTargetFBOImpl::makeActive()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    GFXGL->getOpenglCache()->setCacheBinded(GL_FRAMEBUFFER, mFramebuffer);

    int i = 0;
    GLenum draws[16];
    for( i = 0; i < GFXGL->getNumRenderTargets(); ++i)
    {
        _GFXGLTargetDesc* color = mTarget->getTargetDesc( static_cast<GFXTextureTarget::RenderSlot>(GFXTextureTarget::Color0+i ));
        if(color)
            draws[i] = GL_COLOR_ATTACHMENT0 + i;
        else
            break;
    }

    glDrawBuffers( i, draws );
}

void _GFXGLTextureTargetFBOImpl::finish()
{
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   GFXGL->getOpenglCache()->setCacheBinded(GL_FRAMEBUFFER, 0);

   for(int i = 0; i < GFXGL->getNumRenderTargets(); ++i)
   {   
      _GFXGLTargetDesc* color = mTarget->getTargetDesc( static_cast<GFXTextureTarget::RenderSlot>(GFXTextureTarget::Color0+i ) );
      if(!color || !(color->hasMips()))
         continue;
   
      // Generate mips if necessary
      // Assumes a 2D texture.
      GLenum binding = color->getBinding();
      binding = (binding >= GL_TEXTURE_CUBE_MAP_POSITIVE_X && binding <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z) ? GL_TEXTURE_CUBE_MAP : binding;

      PRESERVE_TEXTURE( binding );
      glBindTexture( binding, color->getHandle() );
      glGenerateMipmap( binding );
   }
}

// Actual GFXGLTextureTarget interface
GFXGLTextureTarget::GFXGLTextureTarget() : mCopyFboSrc(0), mCopyFboDst(0)
{
   for(U32 i=0; i<MaxRenderSlotId; i++)
      mTargets[i] = NULL;
   
   GFXTextureManager::addEventDelegate( this, &GFXGLTextureTarget::_onTextureEvent );

   _impl = new _GFXGLTextureTargetFBOImpl(this);
    
   glGenFramebuffers(1, &mCopyFboSrc);
   glGenFramebuffers(1, &mCopyFboDst);
}

GFXGLTextureTarget::~GFXGLTextureTarget()
{
   GFXTextureManager::removeEventDelegate(this, &GFXGLTextureTarget::_onTextureEvent);

   glDeleteFramebuffers(1, &mCopyFboSrc);
   glDeleteFramebuffers(1, &mCopyFboDst);
}

const Point2I GFXGLTextureTarget::getSize()
{
   if(mTargets[Color0].isValid())
      return Point2I(mTargets[Color0]->getWidth(), mTargets[Color0]->getHeight());

   return Point2I(0, 0);
}

GFXFormat GFXGLTextureTarget::getFormat()
{
   if(mTargets[Color0].isValid())
      return mTargets[Color0]->getFormat();

   return GFXFormatR8G8B8A8;
}

void GFXGLTextureTarget::attachTexture( RenderSlot slot, GFXTextureObject *tex, U32 mipLevel/*=0*/, U32 zOffset /*= 0*/ )
{
   if( tex == GFXTextureTarget::sDefaultDepthStencil )
      tex = GFXGL->getDefaultDepthTex();

   _GFXGLTextureTargetDesc* mTex = static_cast<_GFXGLTextureTargetDesc*>(mTargets[slot].ptr());
   if( (!tex && !mTex) || (mTex && mTex->getTextureObject() == tex) )
      return;
   
   // Triggers an update when we next render
   invalidateState();

   // We stash the texture and info into an internal struct.
   GFXGLTextureObject* glTexture = static_cast<GFXGLTextureObject*>(tex);
   if(tex && tex != GFXTextureTarget::sDefaultDepthStencil)
      mTargets[slot] = new _GFXGLTextureTargetDesc(glTexture, mipLevel, zOffset);
   else
      mTargets[slot] = NULL;
}

void GFXGLTextureTarget::attachTexture( RenderSlot slot, GFXCubemap *tex, U32 face, U32 mipLevel/*=0*/ )
{
   // No depth cubemaps, sorry
   AssertFatal(slot != DepthStencil, "GFXGLTextureTarget::attachTexture (cube) - Cube depth textures not supported!");
   if(slot == DepthStencil)
      return;
    
   // Triggers an update when we next render
   invalidateState();
   
   // We stash the texture and info into an internal struct.
   GFXGLCubemap* glTexture = static_cast<GFXGLCubemap*>(tex);
   if(tex)
      mTargets[slot] = new _GFXGLCubemapTargetDesc(glTexture, face, mipLevel, 0);
   else
      mTargets[slot] = NULL;
}

void GFXGLTextureTarget::clearAttachments()
{
   deactivate();
   for(S32 i=1; i<MaxRenderSlotId; i++)
      attachTexture((RenderSlot)i, NULL);
}

void GFXGLTextureTarget::zombify()
{
   invalidateState();
   
   // Will be recreated in applyState
   _impl = NULL;
}

void GFXGLTextureTarget::resurrect()
{
   // Dealt with when the target is next bound
}

void GFXGLTextureTarget::makeActive()
{
   _impl->makeActive();
}

void GFXGLTextureTarget::deactivate()
{
   _impl->finish();
}

void GFXGLTextureTarget::applyState()
{
   if(!isPendingState())
      return;

   // So we don't do this over and over again
   stateApplied();
   
   if(_impl.isNull())
      _impl = new _GFXGLTextureTargetFBOImpl(this);
           
   _impl->applyState();
}

_GFXGLTargetDesc* GFXGLTextureTarget::getTargetDesc(RenderSlot slot) const
{
   // This can only be called by our implementations, and then will not actually store the pointer so this is (almost) safe
   return mTargets[slot].ptr();
}

void GFXGLTextureTarget::_onTextureEvent( GFXTexCallbackCode code )
{
   invalidateState();
}

const String GFXGLTextureTarget::describeSelf() const
{
   String ret = String::ToString("   Color0 Attachment: %i", mTargets[Color0].isValid() ? mTargets[Color0]->getHandle() : 0);
   ret += String::ToString("   Depth Attachment: %i", mTargets[DepthStencil].isValid() ? mTargets[DepthStencil]->getHandle() : 0);
   
   return ret;
}

void GFXGLTextureTarget::resolve()
{
}

void GFXGLTextureTarget::resolveTo(GFXTextureObject* obj)
{
   AssertFatal(dynamic_cast<GFXGLTextureObject*>(obj), "GFXGLTextureTarget::resolveTo - Incorrect type of texture, expected a GFXGLTextureObject");
   GFXGLTextureObject* glTexture = static_cast<GFXGLTextureObject*>(obj);

   if( gglHasExtension(ARB_copy_image) && mTargets[Color0]->isCompatible(glTexture) )
   {
      GLenum binding = mTargets[Color0]->getBinding();      
      binding = (binding >= GL_TEXTURE_CUBE_MAP_POSITIVE_X && binding <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z) ? GL_TEXTURE_CUBE_MAP : binding;
      U32 srcStartDepth = binding == GL_TEXTURE_CUBE_MAP ? mTargets[Color0]->getBinding() - GL_TEXTURE_CUBE_MAP_POSITIVE_X : 0;
      glCopyImageSubData(
        mTargets[Color0]->getHandle(), binding, 0, 0, 0, srcStartDepth,
        glTexture->getHandle(), glTexture->getBinding(), 0, 0, 0, 0,
        mTargets[Color0]->getWidth(), mTargets[Color0]->getHeight(), 1);

      return;
   }

   PRESERVE_FRAMEBUFFER();
   
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mCopyFboDst);
   glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, glTexture->getBinding(), glTexture->getHandle(), 0);
   
   glBindFramebuffer(GL_READ_FRAMEBUFFER, mCopyFboSrc);
   glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTargets[Color0]->getBinding(), mTargets[Color0]->getHandle(), 0);
   
   glBlitFramebuffer(0, 0, mTargets[Color0]->getWidth(), mTargets[Color0]->getHeight(),
      0, 0, glTexture->getWidth(), glTexture->getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
}
