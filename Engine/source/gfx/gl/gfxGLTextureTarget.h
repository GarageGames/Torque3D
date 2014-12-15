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

#ifndef _GFXGLTEXTURETARGET_H_
#define _GFXGLTEXTURETARGET_H_

#include "gfx/gfxTarget.h"
#include "core/util/autoPtr.h"

class GFXGLTextureObject;
class _GFXGLTargetDesc;
class _GFXGLTextureTargetImpl;

/// Render to texture support for OpenGL.
/// This class needs to make a number of assumptions due to the requirements
/// and complexity of render to texture in OpenGL.
/// 1) This class is only guaranteed to work with 2D textures or cubemaps.  3D textures
/// may or may not work.
/// 2) This class does not currently support multiple texture targets.  Regardless
/// of how many targets you bind, only Color0 will be used.
/// 3) This class requires that the DepthStencil and Color0 targets have identical
/// dimensions.
/// 4) If the DepthStencil target is GFXTextureTarget::sDefaultStencil, then the
/// Color0 target should be the same size as the current backbuffer and should also
/// be the same format (typically R8G8B8A8)
class GFXGLTextureTarget : public GFXTextureTarget
{
public:
   GFXGLTextureTarget();
   virtual ~GFXGLTextureTarget();

   virtual const Point2I getSize();
   virtual GFXFormat getFormat();
   virtual void attachTexture(RenderSlot slot, GFXTextureObject *tex, U32 mipLevel=0, U32 zOffset = 0);
   virtual void attachTexture(RenderSlot slot, GFXCubemap *tex, U32 face, U32 mipLevel=0);
   virtual void clearAttachments();

   /// Functions to query internal state
   /// @{
   
   /// Returns the internal structure for the given slot.  This should only be called by our internal implementations.
   _GFXGLTargetDesc* getTargetDesc(RenderSlot slot) const;

   /// @}
   
   void deactivate();
   void zombify();
   void resurrect();
   virtual const String describeSelf() const;
   
   virtual void resolve();
   
   virtual void resolveTo(GFXTextureObject* obj);
   
protected:

   friend class GFXGLDevice;

   /// The callback used to get texture events.
   /// @see GFXTextureManager::addEventDelegate
   void _onTextureEvent( GFXTexCallbackCode code );
   
   /// Pointer to our internal implementation
   AutoPtr<_GFXGLTextureTargetImpl> _impl;

   /// Array of _GFXGLTargetDesc's, an internal struct used to keep track of texture data.
   AutoPtr<_GFXGLTargetDesc> mTargets[MaxRenderSlotId];

   /// These redirect to our internal implementation
   /// @{
   
   void applyState();
   void makeActive();
   
   /// @}

   //copy FBO
   GLuint mCopyFboSrc, mCopyFboDst;

};

#endif
