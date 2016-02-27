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

#include "windowManager/platformWindow.h"
#include "gfx/gl/gfxGLDevice.h"
#include "gfx/gl/gfxGLWindowTarget.h"
#include "gfx/gl/gfxGLTextureObject.h"
#include "gfx/gl/gfxGLUtils.h"
#include "postFx/postEffect.h"

GFX_ImplementTextureProfile( BackBufferDepthProfile,
                             GFXTextureProfile::DiffuseMap,
                             GFXTextureProfile::PreserveSize |
                             GFXTextureProfile::NoMipmap |
                             GFXTextureProfile::ZTarget |
                             GFXTextureProfile::Pooled,
                             GFXTextureProfile::NONE );

GFXGLWindowTarget::GFXGLWindowTarget(PlatformWindow *win, GFXDevice *d)
      : GFXWindowTarget(win), mDevice(d), mContext(NULL), mFullscreenContext(NULL)
      , mCopyFBO(0), mBackBufferFBO(0)
{      
   win->appEvent.notify(this, &GFXGLWindowTarget::_onAppSignal);
}

GFXGLWindowTarget::~GFXGLWindowTarget()
{
   if(glIsFramebuffer(mCopyFBO))
   {
      glDeleteFramebuffers(1, &mCopyFBO);
   }
}

void GFXGLWindowTarget::resetMode()
{
   if(mWindow->getVideoMode().fullScreen != mWindow->isFullscreen())
   {
      _teardownCurrentMode();
      _setupNewMode();
   }
   GFX->beginReset();
}

void GFXGLWindowTarget::_onAppSignal(WindowId wnd, S32 event)
{
   if(event != WindowHidden)
      return;
      
   // TODO: Investigate this further.
   // Opening and then closing the console results in framerate dropping at an alarming rate down to 3-4 FPS and then
   // rebounding to it's usual level.  Clearing all the volatile VBs prevents this behavior, but I can't explain why.
   // My fear is there is something fundamentally wrong with how we share objects between contexts and this is simply 
   // masking the issue for the most common case.
   static_cast<GFXGLDevice*>(mDevice)->mVolatileVBs.clear();
}

void GFXGLWindowTarget::resolveTo(GFXTextureObject* obj)
{
   AssertFatal(dynamic_cast<GFXGLTextureObject*>(obj), "GFXGLTextureTarget::resolveTo - Incorrect type of texture, expected a GFXGLTextureObject");
   GFXGLTextureObject* glTexture = static_cast<GFXGLTextureObject*>(obj);

   if( gglHasExtension(ARB_copy_image) )
   {
      if(mBackBufferColorTex.getWidth() == glTexture->getWidth()
         && mBackBufferColorTex.getHeight() == glTexture->getHeight()
         && mBackBufferColorTex.getFormat() == glTexture->getFormat())
      {
         glCopyImageSubData(
           static_cast<GFXGLTextureObject*>(mBackBufferColorTex.getPointer())->getHandle(), GL_TEXTURE_2D, 0, 0, 0, 0,
           glTexture->getHandle(), GL_TEXTURE_2D, 0, 0, 0, 0,
           getSize().x, getSize().y, 1);
         return;
      }
   }

   PRESERVE_FRAMEBUFFER();

   if(!mCopyFBO)
   {
      glGenFramebuffers(1, &mCopyFBO);
   }
   
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mCopyFBO);
   glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glTexture->getHandle(), 0);
   
   glBindFramebuffer(GL_READ_FRAMEBUFFER, mBackBufferFBO);
   
   glBlitFramebuffer(0, 0, getSize().x, getSize().y,
      0, 0, glTexture->getWidth(), glTexture->getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

inline void GFXGLWindowTarget::_setupAttachments()
{
   glBindFramebuffer( GL_FRAMEBUFFER, mBackBufferFBO);
   GFXGL->getOpenglCache()->setCacheBinded(GL_FRAMEBUFFER, mBackBufferFBO);
   const Point2I dstSize = getSize();
   mBackBufferColorTex.set(dstSize.x, dstSize.y, getFormat(), &PostFxTargetProfile, "backBuffer");
   GFXGLTextureObject *color = static_cast<GFXGLTextureObject*>(mBackBufferColorTex.getPointer());
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color->getHandle(), 0);
   mBackBufferDepthTex.set(dstSize.x, dstSize.y, GFXFormatD24S8, &BackBufferDepthProfile, "backBuffer");
   GFXGLTextureObject *depth = static_cast<GFXGLTextureObject*>(mBackBufferDepthTex.getPointer());
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth->getHandle(), 0);
}

void GFXGLWindowTarget::makeActive()
{
   if(mBackBufferFBO)
   {
      glBindFramebuffer( GL_FRAMEBUFFER, mBackBufferFBO);
      GFXGL->getOpenglCache()->setCacheBinded(GL_FRAMEBUFFER, mBackBufferFBO);
   }
   else
   {
      glGenFramebuffers(1, &mBackBufferFBO);
      _setupAttachments();
      CHECK_FRAMEBUFFER_STATUS();
   }
}

bool GFXGLWindowTarget::present()
{
    PRESERVE_FRAMEBUFFER();

   const Point2I srcSize = mBackBufferColorTex.getWidthHeight();
   const Point2I dstSize = getSize();

   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
   glBindFramebuffer(GL_READ_FRAMEBUFFER, mBackBufferFBO);

   // OpenGL render upside down for make render more similar to DX.
   // Final screen are corrected here
   glBlitFramebuffer(
      0, 0, srcSize.x, srcSize.y,
      0, dstSize.y, dstSize.x, 0, // Y inverted
      GL_COLOR_BUFFER_BIT, GL_NEAREST);

   _WindowPresent();

   if(srcSize != dstSize || mBackBufferDepthTex.getWidthHeight() != dstSize)
      _setupAttachments();

   return true;
}
