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

#ifndef _GFXTARGET_H_
#define _GFXTARGET_H_

#ifndef _REFBASE_H_
#include "core/util/refBase.h"
#endif
#ifndef _GFXENUMS_H_
#include "gfx/gfxEnums.h"
#endif
#ifndef _GFXRESOURCE_H_
#include "gfx/gfxResource.h"
#endif
#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif


class Point2I;
class PlatformWindow;
class GFXCubemap;
class GFXTextureObject;

/// Base class for a target to which GFX can render.
///
/// Most modern graphics hardware supports selecting render targets. However,
/// there may be multiple types of render target, with wildly varying
/// device-level implementations, resource requirements, and so forth.
///
/// This base class is used to represent a render target; it might be a context
/// tied to a window, or a set of surfaces or textures.
class GFXTarget : public StrongRefBase, public GFXResource
{
   friend class GFXD3D9Device;
   friend class GFX360Device;

private:
   S32 mChangeToken;
   S32 mLastAppliedChange;

protected:

   /// Called whenever a change is made to this target.
   inline void invalidateState()
   {
      mChangeToken++;
   }

   /// Called when the device has applied pending state.
   inline void stateApplied()
   {
      mLastAppliedChange = mChangeToken;
   }
public:

   /// Constructor to initialize the state tracking logic.
   GFXTarget() : mChangeToken( 0 ),
                 mLastAppliedChange( 0 )
   {
   }
   virtual ~GFXTarget() {}

   /// Called to check if we have pending state for the device to apply.
   inline const bool isPendingState() const
   {
      return (mChangeToken != mLastAppliedChange);
   }

   /// Returns the size in pixels of the render target.
   virtual const Point2I getSize()=0;

   /// Returns the texture format of the render target.
   virtual GFXFormat getFormat()=0;

   // GFXResourceInterface
   /// The resource should put a description of itself (number of vertices, size/width of texture, etc.) in buffer
   virtual const String describeSelf() const;

   /// This is called to set the render target.
   virtual void activate() { }

   /// This is called when the target is not being used anymore.
   virtual void deactivate() { }

   /// This tells the target that the contents of this target should be restored
   /// when activate() is next called.
   virtual void preserve() { }

   /// Copy this surface to the passed GFXTextureObject.   
   /// @param tex The GFXTextureObject to copy to.
   virtual void resolveTo( GFXTextureObject *tex ) { }
};

/// A render target associated with an OS window.
///
/// Various API/OS combinations will implement their own GFXTargets for
/// rendering to a window. However, they are all subclasses of GFXWindowTarget.
///
/// This allows platform-neutral code to safely distinguish between various
/// types of render targets (using dynamic_cast<>), as well as letting it
/// gain access to useful things like the corresponding PlatformWindow.
class GFXWindowTarget : public GFXTarget
{
protected:
   PlatformWindow *mWindow;
public:
   GFXWindowTarget() : mWindow(NULL){};
   GFXWindowTarget( PlatformWindow *windowObject )
   {
      mWindow = windowObject;
   }
   virtual ~GFXWindowTarget() {}

   /// Returns a pointer to the window this target is bound to.
   inline PlatformWindow *getWindow() { return mWindow; };

   /// Present latest buffer, if buffer swapping is in effect.
   virtual bool present()=0;

   /// Notify the target that the video mode on the window has changed.
   virtual void resetMode()=0;
};

/// A render target associated with one or more textures.
///
/// Although some APIs allow directly selecting any texture or surfaces, in
/// some cases it is necessary to allocate helper resources to enable RTT
/// operations.
///
/// @note A GFXTextureTarget will retain references to textures that are 
/// attached to it, so be sure to clear them out when you're done!
///
/// @note Different APIs have different restrictions on what they can support
///       here. Be aware when mixing cubemaps vs. non-cubemaps, or targets of
///       different resolutions. The devices will attempt to limit behavior
///       to things that are safely portable, but they cannot catch every
///       possible situation for all drivers and API - so make sure to
///       actually test things!
class GFXTextureTarget : public GFXTarget
{
public:
   enum RenderSlot
   {
      DepthStencil,
      Color0, Color1, Color2, Color3, Color4,
      MaxRenderSlotId,
   };

   static GFXTextureObject *sDefaultDepthStencil;

   virtual ~GFXTextureTarget() {}

   /// Attach a surface to a given slot as part of this render target.
   ///
   /// @param slot What slot is used for multiple render target (MRT) effects.
   ///             Most of the time you'll use Color0.
   /// @param tex A texture and miplevel to bind for rendering, or else NULL/0
   ///            to clear a slot.
   /// @param mipLevel What level of this texture are we rendering to?
   /// @param zOffset  If this is a depth texture, what z level are we 
   ///                 rendering to?
   virtual void attachTexture(RenderSlot slot, GFXTextureObject *tex, U32 mipLevel=0, U32 zOffset = 0) = 0;

   /// Support binding to cubemaps.
   ///
   /// @param slot What slot is used for multiple render target (MRT) effects.
   ///             Most of the time you'll use Color0.
   /// @param tex  What cubemap will we be rendering to?
   /// @param face A face identifier.
   /// @param mipLevel What level of this texture are we rendering to?
   virtual void attachTexture(RenderSlot slot, GFXCubemap *tex, U32 face, U32 mipLevel=0) = 0;

   /// Resolve the current render target data to the associated textures. This method
   /// will get called automatically when a rendertarget is changed, before new geometry
   /// is drawn to a different rendertarget. This method can also be called to 
   /// gather render target data without switching targets.
   /// 
   /// By default, this method will resolve all color targets.
   virtual void resolve()=0;
};

typedef StrongRefPtr<GFXTarget> GFXTargetRef;
typedef StrongRefPtr<GFXWindowTarget> GFXWindowTargetRef;
typedef StrongRefPtr<GFXTextureTarget> GFXTextureTargetRef;

#endif // _GFXTARGET_H_
