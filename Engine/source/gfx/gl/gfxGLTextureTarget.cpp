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
   
private:
   StrongRefPtr<GFXGLCubemap> mTex;
   U32 mFace;
};

// Internal implementations
class _GFXGLTextureTargetImpl
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

// Handy macro for checking the status of a framebuffer.  Framebuffers can fail in 
// all sorts of interesting ways, these are just the most common.  Further, no existing GL profiling 
// tool catches framebuffer errors when the framebuffer is created, so we actually need this.
#define CHECK_FRAMEBUFFER_STATUS()\
{\
GLenum status;\
status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);\
switch(status) {\
case GL_FRAMEBUFFER_COMPLETE_EXT:\
break;\
case GL_FRAMEBUFFER_UNSUPPORTED_EXT:\
AssertFatal(false, "Unsupported FBO");\
break;\
case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:\
AssertFatal(false, "Incomplete FBO Attachment");\
break;\
case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:\
AssertFatal(false, "Incomplete FBO dimensions");\
break;\
case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:\
AssertFatal(false, "Incomplete FBO formats");\
default:\
/* programming error; will fail on all hardware */\
AssertFatal(false, "Something really bad happened with an FBO");\
}\
}

_GFXGLTextureTargetFBOImpl::_GFXGLTextureTargetFBOImpl(GFXGLTextureTarget* target)
{
   mTarget = target;
   glGenFramebuffersEXT(1, &mFramebuffer);
}

_GFXGLTextureTargetFBOImpl::~_GFXGLTextureTargetFBOImpl()
{
   glDeleteFramebuffersEXT(1, &mFramebuffer);
}

void _GFXGLTextureTargetFBOImpl::applyState()
{   
   // REMINDER: When we implement MRT support, check against GFXGLDevice::getNumRenderTargets()
   
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFramebuffer);
   
   _GFXGLTargetDesc* color0 = mTarget->getTargetDesc(GFXTextureTarget::Color0);
   if(color0)
   {
      if(color0->getDepth() == 0)
         glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, color0->getBinding(), color0->getHandle(), color0->getMipLevel());
      else
         glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, color0->getBinding(), color0->getHandle(), color0->getMipLevel(), color0->getZOffset());
   }
   else
   {
      // Clears the texture (note that the binding is irrelevent)
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, 0, 0);
   }
   
   _GFXGLTargetDesc* depthStecil = mTarget->getTargetDesc(GFXTextureTarget::DepthStencil);
   if(depthStecil)
   {
      // Certain drivers have issues with depth only FBOs.  That and the next two asserts assume we have a color target.
      AssertFatal(color0, "GFXGLTextureTarget::applyState() - Cannot set DepthStencil target without Color0 target!");
      AssertFatal(depthStecil->getWidth() == color0->getWidth(), "GFXGLTextureTarget::applyState() - DepthStencil and Color0 targets MUST have the same width!");
      AssertFatal(depthStecil->getHeight() == color0->getHeight(), "GFXGLTextureTarget::applyState() - DepthStencil and Color0 targets MUST have the same height!");
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, depthStecil->getBinding(), depthStecil->getHandle(), depthStecil->getMipLevel());
   }
   else
   {
      // Clears the texture (note that the binding is irrelevent)
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, 0, 0);
   }
   
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void _GFXGLTextureTargetFBOImpl::makeActive()
{
   glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, mFramebuffer);
   glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, mFramebuffer);
}

void _GFXGLTextureTargetFBOImpl::finish()
{
   glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);
   glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, 0);
   
   _GFXGLTargetDesc* color0 = mTarget->getTargetDesc(GFXTextureTarget::Color0);
   if(!color0 || !(color0->hasMips()))
      return;
   
   // Generate mips if necessary
   // Assumes a 2D texture.
   glActiveTexture(GL_TEXTURE0);
   PRESERVE_2D_TEXTURE();
   glBindTexture(GL_TEXTURE_2D, color0->getHandle());
   glGenerateMipmapEXT(GL_TEXTURE_2D);
}

// This implementations uses AUX buffers (we should always have at least one) to do render to texture.  It is currently only used when we need access to the windows depth buffer.
class _GFXGLTextureTargetAUXBufferImpl : public _GFXGLTextureTargetImpl
{
public:
   _GFXGLTextureTargetAUXBufferImpl(GFXGLTextureTarget* target);
   
   virtual void applyState();
   virtual void makeActive();
   virtual void finish();
};

_GFXGLTextureTargetAUXBufferImpl::_GFXGLTextureTargetAUXBufferImpl(GFXGLTextureTarget* target)
{
   mTarget = target;
}

void _GFXGLTextureTargetAUXBufferImpl::applyState()
{
   
}

void _GFXGLTextureTargetAUXBufferImpl::makeActive()
{
   glDrawBuffer(GL_AUX0);
   glReadBuffer(GL_AUX0);
}

void _GFXGLTextureTargetAUXBufferImpl::finish()
{
   // Bind the Color0 texture
   _GFXGLTargetDesc* color0 = mTarget->getTargetDesc(GFXTextureTarget::Color0);
   
   glActiveTexture(GL_TEXTURE0);
   // Assume we're a 2D texture for now.
   PRESERVE_2D_TEXTURE();
   glBindTexture(color0->getBinding(), color0->getHandle());
   glCopyTexSubImage2D(color0->getBinding(), 0, 0, 0, 0, 0, color0->getWidth(), color0->getHeight());
   
   glDrawBuffer(GL_BACK);
   glReadBuffer(GL_BACK);
}

// Actual GFXGLTextureTarget interface
GFXGLTextureTarget::GFXGLTextureTarget()
{
   for(U32 i=0; i<MaxRenderSlotId; i++)
      mTargets[i] = NULL;
   
   GFXTextureManager::addEventDelegate( this, &GFXGLTextureTarget::_onTextureEvent );

   _impl = new _GFXGLTextureTargetFBOImpl(this);
   _needsAux = false;
}

GFXGLTextureTarget::~GFXGLTextureTarget()
{
   GFXTextureManager::removeEventDelegate( this, &GFXGLTextureTarget::_onTextureEvent );
}

const Point2I GFXGLTextureTarget::getSize()
{
   if(mTargets[Color0].isValid())
      return Point2I(mTargets[Color0]->getWidth(), mTargets[Color0]->getHeight());

   return Point2I(0, 0);
}

GFXFormat GFXGLTextureTarget::getFormat()
{
   // TODO: Fix me!
   return GFXFormatR8G8B8A8;
}

void GFXGLTextureTarget::attachTexture( RenderSlot slot, GFXTextureObject *tex, U32 mipLevel/*=0*/, U32 zOffset /*= 0*/ )
{
   // GFXTextureTarget::sDefaultDepthStencil is a hint that we want the window's depth buffer.
   if(tex == GFXTextureTarget::sDefaultDepthStencil)
      _needsAux = true;
   
   if(slot == DepthStencil && tex != GFXTextureTarget::sDefaultDepthStencil)
      _needsAux = false;
   
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
   
   // Ensure we have the proper implementation (consider changing to an enum?)
   if(_needsAux && dynamic_cast<_GFXGLTextureTargetAUXBufferImpl*>(_impl.ptr()) == NULL)
      _impl = new _GFXGLTextureTargetAUXBufferImpl(this);
   else if(!_needsAux && dynamic_cast<_GFXGLTextureTargetFBOImpl*>(_impl.ptr()) == NULL)
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

   PRESERVE_FRAMEBUFFER();
   
   GLuint dest;
   GLuint src;
   
   glGenFramebuffersEXT(1, &dest);
   glGenFramebuffersEXT(1, &src);
   
   glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, dest);
   glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, glTexture->getHandle(), 0);
   
   glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, src);
   glFramebufferTexture2DEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D,mTargets[Color0]->getHandle(), 0);
   
   glBlitFramebufferEXT(0, 0, mTargets[Color0]->getWidth(), mTargets[Color0]->getHeight(),
      0, 0, glTexture->getWidth(), glTexture->getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
   
   glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);
   glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, 0);
   
   glDeleteFramebuffersEXT(1, &dest);
   glDeleteFramebuffersEXT(1, &src);
}
