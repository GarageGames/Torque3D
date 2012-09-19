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

#ifndef _RENDERFORMATCHANGER_H_
#define _RENDERFORMATCHANGER_H_

#ifndef _RENDERPASSSTATETOKEN_H_
#include "renderInstance/renderPassStateToken.h"
#endif
#ifndef _MATTEXTURETARGET_H_
#include "materials/matTextureTarget.h"
#endif

class PostEffect;


class RenderFormatToken : public RenderPassStateToken
{
   typedef RenderPassStateToken Parent;

public:
   enum FormatTokenState
   {
      FTSDisabled,
      FTSWaiting,
      FTSActive,
      FTSComplete,
   };

   const static U32 TargetChainLength = 1;

protected:

   FormatTokenState mFCState;
   GFXFormat mColorFormat;
   GFXFormat mDepthFormat;
   bool mTargetUpdatePending;
   U32 mTargetChainIdx;
   Point2I mTargetSize;
   S32 mTargetAALevel;
   SimObjectPtr<PostEffect> mCopyPostEffect;
   SimObjectPtr<PostEffect> mResolvePostEffect;

   NamedTexTarget mTarget;

   GFXTexHandle mTargetColorTexture[TargetChainLength];
   GFXTexHandle mTargetDepthStencilTexture[TargetChainLength];
   GFXTextureTargetRef mTargetChain[TargetChainLength];

   GFXTexHandle mStoredPassZTarget;
   
   void _updateTargets();
   void _teardownTargets();

   void _onTextureEvent( GFXTexCallbackCode code );
   virtual bool _handleGFXEvent(GFXDevice::GFXDeviceEventType event);

   static bool _setFmt( void *object, const char *index, const char *data );
   static const char* _getCopyPostEffect( void* object, const char* data );
   static const char* _getResolvePostEffect( void* object, const char* data );
   static bool _setCopyPostEffect( void* object, const char* index, const char* data );
   static bool _setResolvePostEffect( void* object, const char* index, const char* data );
   
public:

   DECLARE_CONOBJECT(RenderFormatToken);
   static void initPersistFields();
   virtual bool onAdd();
   virtual void onRemove();

   RenderFormatToken();
   virtual ~RenderFormatToken();

   virtual void process(SceneRenderState *state, RenderPassStateBin *callingBin);
   virtual void reset();
   virtual void enable(bool enabled = true);
   virtual bool isEnabled() const;
};

#endif // _RENDERFORMATCHANGER_H_
