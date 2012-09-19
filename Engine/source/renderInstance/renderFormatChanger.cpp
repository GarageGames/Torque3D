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

#include "platform/platform.h"
#include "renderInstance/renderFormatChanger.h"

#include "console/consoleTypes.h"
#include "gfx/gfxStringEnumTranslate.h"
#include "gfx/gfxTextureManager.h"
#include "gfx/gfxDebugEvent.h"
#include "postFx/postEffect.h"
#include "postFx/postEffectManager.h"

extern ColorI gCanvasClearColor;

IMPLEMENT_CONOBJECT(RenderFormatToken);

ConsoleDocClass( RenderFormatToken,
   "@brief Used to change the render target format when rendering in AL.\n\n"

   "RenderFormatToken is an implementation which changes the format of the "
   "back buffer and/or the depth buffer.\n\n"
   
   "The RenderPassStateBin manager changes the rendering state associated with "
   "this token. In stock Torque 3D, a single example exists in the "
   "way of AL_FormatToken (found in renderManager.cs). In that script file, all the "
   "render managers are intialized, and a single RenderFormatToken is used. This "
   "implementation basically exists to ensure Advanced Lighting works with MSAA.\n\n"

   "The actions for this token toggle the format of the back/depth buffers "
   "and it lets you specify a custom shader to \"copy\" the data so it can "
   "be reformatted or altered. This is done through the variables copyEffect and "
   "resolveEffect (which are post processes just like fog or glow)\n\n"

   "@tsexample\n"
   "// This token, and the associated render managers, ensure "
   "that driver MSAA does not get used for Advanced Lighting renders.\n"
   "// The 'AL_FormatResolve' PostEffect copies the result to the backbuffer.\n"
   "new RenderFormatToken(AL_FormatToken)\n"
   "{\n"
   "   enabled = \"false\";\n\n"
   "   format = \"GFXFormatR8G8B8A8\";\n"
   "   depthFormat = \"GFXFormatD24S8\";\n"
   "   aaLevel = 0; // -1 = match backbuffer\n\n"
   "   // The contents of the back buffer before this format token is executed\n"
   "   // is provided in $inTex\n"
   "   copyEffect = \"AL_FormatCopy\";\n\n"
   "   // The contents of the render target created by this format token is\n"
   "   // provided in $inTex\n"
   "   resolveEffect = \"AL_FormatCopy\";\n"
   "};\n"
   "@endtsexample\n\n"

   "@see RenderPassToken\n\n"
   "@see RenderPassStateBin\n"
   "@see game/core/scripts/client/renderManager.cs\n"

   "@ingroup GFX\n"
);

RenderFormatToken::RenderFormatToken() 
   :  Parent(), 
      mFCState(FTSDisabled), 
      mColorFormat(GFXFormat_COUNT), 
      mDepthFormat(GFXFormat_COUNT),
      mTargetUpdatePending(true),
      mTargetChainIdx(0),
      mTargetSize(Point2I::Zero),
      mTargetAALevel(GFXTextureManager::AA_MATCH_BACKBUFFER),
      mCopyPostEffect(NULL),
      mResolvePostEffect(NULL)
{
   GFXDevice::getDeviceEventSignal().notify(this, &RenderFormatToken::_handleGFXEvent);
   GFXTextureManager::addEventDelegate(this, &RenderFormatToken::_onTextureEvent);
}

RenderFormatToken::~RenderFormatToken()
{
   GFXTextureManager::removeEventDelegate(this, &RenderFormatToken::_onTextureEvent);
   GFXDevice::getDeviceEventSignal().remove(this, &RenderFormatToken::_handleGFXEvent);

   _teardownTargets();
}

void RenderFormatToken::process(SceneRenderState *state, RenderPassStateBin *callingBin)
{
   switch(mFCState)
   {
   case FTSWaiting:
      {
         GFXDEBUGEVENT_SCOPE_EX(RFT_Waiting, ColorI::BLUE, avar("[%s Activate] (%s)", getName(), GFXStringTextureFormat[mColorFormat]));
         mFCState = FTSActive;

         mTarget.setViewport( GFX->getViewport() );

         // Update targets
         _updateTargets();

         // If we have a copy PostEffect then get the active backbuffer copy 
         // now before we swap the render targets.
         GFXTexHandle curBackBuffer;
         if(mCopyPostEffect.isValid())
            curBackBuffer = PFXMGR->getBackBufferTex();

         // Push target
         GFX->pushActiveRenderTarget();
         GFX->setActiveRenderTarget(mTargetChain[mTargetChainIdx]);

         // Set viewport
         GFX->setViewport( mTarget.getViewport() );

         // Clear
         GFX->clear(GFXClearTarget | GFXClearZBuffer | GFXClearStencil, gCanvasClearColor, 1.0f, 0);

         // Set active z target on render pass
         if(mTargetDepthStencilTexture[mTargetChainIdx].isValid())
         {
            if(callingBin->getRenderPass()->getDepthTargetTexture() != GFXTextureTarget::sDefaultDepthStencil)
               mStoredPassZTarget = callingBin->getRenderPass()->getDepthTargetTexture();
            else
               mStoredPassZTarget = NULL;

            callingBin->getRenderPass()->setDepthTargetTexture(mTargetDepthStencilTexture[mTargetChainIdx]);
         }

         // Run the PostEffect which copies data into the new target.
         if ( mCopyPostEffect.isValid() )
            mCopyPostEffect->process( state, curBackBuffer, &mTarget.getViewport() );
      }
      break;

   case FTSActive:
      {
         GFXDEBUGEVENT_SCOPE_EX(RFT_Active, ColorI::BLUE, avar("[%s Deactivate]", getName()));
         mFCState = FTSComplete;

         // Pop target
         AssertFatal(GFX->getActiveRenderTarget() == mTargetChain[mTargetChainIdx], "Render target stack went wrong somewhere");
         mTargetChain[mTargetChainIdx]->resolve();
         GFX->popActiveRenderTarget();
         mTarget.setTexture( mTargetColorTexture[mTargetChainIdx] ); 
         
         // This is the GFX viewport when we were first processed.
         GFX->setViewport( mTarget.getViewport() );

         // Restore active z-target
         if(mTargetDepthStencilTexture[mTargetChainIdx].isValid())
         {
            callingBin->getRenderPass()->setDepthTargetTexture(mStoredPassZTarget.getPointer());
            mStoredPassZTarget = NULL;
         }

         // Run the PostEffect which copies data to the backbuffer
         if(mResolvePostEffect.isValid())
         {
		      // Need to create a texhandle here, since inOutTex gets assigned during process()
            GFXTexHandle inOutTex = mTargetColorTexture[mTargetChainIdx];
            mResolvePostEffect->process( state, inOutTex, &mTarget.getViewport() );
         }
      }
      break;

   case FTSComplete:
      AssertFatal(false, "process() called on a RenderFormatToken which was already complete.");
      // fall through
   case FTSDisabled:
      break;
   }
}

void RenderFormatToken::reset()
{
   AssertFatal(mFCState != FTSActive, "RenderFormatToken still active during reset()!");
   if(mFCState != FTSDisabled)
      mFCState = FTSWaiting;
}

void RenderFormatToken::_updateTargets()
{
   if ( GFX->getActiveRenderTarget() == NULL )
      return;

   const Point2I &rtSize = GFX->getActiveRenderTarget()->getSize();

   if ( rtSize.x <= mTargetSize.x && 
        rtSize.y <= mTargetSize.y && 
        !mTargetUpdatePending )
      return;   

   mTargetSize = rtSize;
   mTargetUpdatePending = false;   
   mTargetChainIdx = 0;

   for( U32 i = 0; i < TargetChainLength; i++ )
   {
      if( !mTargetChain[i] )
         mTargetChain[i] = GFX->allocRenderToTextureTarget();

      // Update color target
      if(mColorFormat != GFXFormat_COUNT)
      {
         mTargetColorTexture[i].set( rtSize.x, rtSize.y, mColorFormat, 
            &GFXDefaultRenderTargetProfile, avar( "%s() - (line %d)", __FUNCTION__, __LINE__ ),
            1, mTargetAALevel );
         mTargetChain[i]->attachTexture( GFXTextureTarget::Color0, mTargetColorTexture[i] );
      }

      mTargetChain[i]->attachTexture( GFXTextureTarget::Color0, mTargetColorTexture[i] );
      

      // Update depth target
      if(mDepthFormat != GFXFormat_COUNT)
      {
         mTargetDepthStencilTexture[i].set( rtSize.x, rtSize.y, mDepthFormat, 
            &GFXDefaultZTargetProfile, avar( "%s() - (line %d)", __FUNCTION__, __LINE__ ),
            1, mTargetAALevel );
      }

      mTargetChain[i]->attachTexture( GFXTextureTarget::DepthStencil, mTargetDepthStencilTexture[i] );
   }
}

void RenderFormatToken::_teardownTargets()
{
   mTarget.release();

   for(int i = 0; i < TargetChainLength; i++)
   {
      mTargetColorTexture[i] = NULL;
      mTargetDepthStencilTexture[i] = NULL;
      mTargetChain[i] = NULL;
   }
}

bool RenderFormatToken::_setFmt( void *object, const char *index, const char *data )
{
   // Flag update pending
   reinterpret_cast<RenderFormatToken *>( object )->mTargetUpdatePending = true;

   // Allow console system to assign value
   return true;
}

const char* RenderFormatToken::_getCopyPostEffect( void* object, const char* data )
{
   RenderFormatToken* token = reinterpret_cast< RenderFormatToken* >( object );
   if( token->mCopyPostEffect.isValid() )
      return token->mCopyPostEffect->getIdString();
   return "0";
}

const char* RenderFormatToken::_getResolvePostEffect( void* object, const char* data )
{
   RenderFormatToken* token = reinterpret_cast< RenderFormatToken* >( object );
   if( token->mResolvePostEffect.isValid() )
      return token->mResolvePostEffect->getIdString();
   return "0";
}

bool RenderFormatToken::_setCopyPostEffect( void* object, const char* index, const char* data )
{
   RenderFormatToken* token = reinterpret_cast< RenderFormatToken* >( object );
   PostEffect* effect;
   Sim::findObject( data, effect );
   token->mCopyPostEffect = effect;
   return false;
}

bool RenderFormatToken::_setResolvePostEffect( void* object, const char* index, const char* data )
{
   RenderFormatToken* token = reinterpret_cast< RenderFormatToken* >( object );
   PostEffect* effect;
   Sim::findObject( data, effect );
   token->mResolvePostEffect = effect;
   return false;
}

void RenderFormatToken::enable( bool enabled /*= true*/ )
{
   AssertFatal(mFCState != FTSActive, "RenderFormatToken is active, cannot change state now!");

   if(enabled)
      mFCState = FTSWaiting;
   else
      mFCState = FTSDisabled;
}

bool RenderFormatToken::isEnabled() const
{
   return (mFCState != FTSDisabled);
}

void RenderFormatToken::initPersistFields()
{
   addProtectedField("format", TypeGFXFormat, Offset(mColorFormat, RenderFormatToken), &_setFmt, &defaultProtectedGetFn, 
      "Sets the color buffer format for this token.");

   addProtectedField("depthFormat", TypeGFXFormat, Offset(mDepthFormat, RenderFormatToken), &_setFmt, &defaultProtectedGetFn, 
      "Sets the depth/stencil buffer format for this token.");

   addProtectedField("copyEffect", TYPEID<PostEffect>(), Offset(mCopyPostEffect, RenderFormatToken),
      &_setCopyPostEffect, &_getCopyPostEffect,
      "This PostEffect will be run when the render target is changed to the format specified "
      "by this token. It is used to copy/format data into the token rendertarget");

   addProtectedField("resolveEffect", TYPEID<PostEffect>(), Offset(mResolvePostEffect, RenderFormatToken),
      &_setResolvePostEffect, &_getResolvePostEffect,
      "This PostEffect will be run when the render target is changed back to the format "
      "active prior to this token. It is used to copy/format data from the token rendertarget to the backbuffer.");

   addField("aaLevel", TypeS32, Offset(mTargetAALevel, RenderFormatToken), 
      "Anti-ailiasing level for the this token. 0 disables, -1 uses adapter default.");

   Parent::initPersistFields();
}


bool RenderFormatToken::_handleGFXEvent(GFXDevice::GFXDeviceEventType event_)
{
   if ( event_ == GFXDevice::deStartOfFrame )
   {
      mTargetChainIdx++;
      if ( mTargetChainIdx >= TargetChainLength )
         mTargetChainIdx = 0;
   }

   return true;
}

void RenderFormatToken::_onTextureEvent( GFXTexCallbackCode code )
{
   if(code == GFXZombify)
   {
      _teardownTargets();
      mTargetUpdatePending = true;
   }
}

bool RenderFormatToken::onAdd()
{
   if(!Parent::onAdd())
      return false;

   mTarget.registerWithName( getName() );
   mTarget.setSamplerState( GFXSamplerStateDesc::getClampPoint() );

   return true;
}

void RenderFormatToken::onRemove()
{
   mTarget.unregister();
   mTarget.release();

   Parent::onRemove();
}