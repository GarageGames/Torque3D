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

#ifndef _GFXGLWINDOWTARGET_H_
#define _GFXGLWINDOWTARGET_H_

#include "gfx/gfxTarget.h"

class GFXGLWindowTarget : public GFXWindowTarget
{
public:

   GFXGLWindowTarget(PlatformWindow *win, GFXDevice *d);
   ~GFXGLWindowTarget();

   const Point2I getSize() 
   { 
      return mWindow->getClientExtent();
   }
   virtual GFXFormat getFormat()
   {
      // TODO: Fix me!
      return GFXFormatR8G8B8A8;
   }
   void makeActive();
   virtual bool present();
   virtual void resetMode();
   virtual void zombify() { }
   virtual void resurrect() { }
   
   virtual void resolveTo(GFXTextureObject* obj);
   
   void _onAppSignal(WindowId wnd, S32 event);

   // create pixel format for the window
   void createPixelFormat();
   
private:
   friend class GFXGLDevice;

   GLuint mCopyFBO, mBackBufferFBO;
   GFXTexHandle mBackBufferColorTex, mBackBufferDepthTex;
   Point2I size;   
   GFXDevice* mDevice;
   /// Is this a secondary window
   bool mSecondaryWindow;
   void* mContext;
   void* mFullscreenContext;
   void _teardownCurrentMode();
   void _setupNewMode();
   void _setupAttachments();
   void _WindowPresent();
   //set this windows context to be current
   void _makeContextCurrent();
   
};

#endif
