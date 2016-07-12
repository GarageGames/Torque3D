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
#include "renderInstance/renderTexTargetBinManager.h"

#include "shaderGen/conditionerFeature.h"
#include "core/util/safeDelete.h"
#include "gfx/gfxTextureManager.h"
#include "gfx/gfxStringEnumTranslate.h"
#include "scene/sceneRenderState.h"
#include "scene/sceneManager.h"


IMPLEMENT_CONOBJECT(RenderTexTargetBinManager);

ConsoleDocClass( RenderTexTargetBinManager, 
   "@brief An abstract base class for render bin managers that render to a named textue target.\n\n"
   "This bin itself doesn't do any rendering work.  It offers functionality to manage "
   "a texture render target which derived render bin classes can render into.\n\n"
   "@see RenderPrePassMgr\n"
   "@ingroup RenderBin\n" );


RenderTexTargetBinManager::RenderTexTargetBinManager( const RenderInstType& ritype, 
                                                      F32 renderOrder, 
                                                      F32 processAddOrder,
                                                      const GFXFormat targetFormat,
                                                      const Point2I &targetSize,
                                                      const U32 targetChainLength )

   :  Parent( ritype, renderOrder, processAddOrder ),      
      mTargetFormat(targetFormat), 
      mTargetSize(targetSize), 
      mTargetScale(1.0f, 1.0f), 
      mTargetSizeType(FixedSize),
      mTargetChainLength(targetChainLength), 
      mTargetChainIdx(0), 
      mNumRenderTargets(1),
      mTargetChain(NULL), 
      mTargetChainTextures(NULL)
      #ifndef TORQUE_SHIPPING
         ,m_NeedsOnPostRender(false)
      #endif
{
   GFXDevice::getDeviceEventSignal().notify(this, &RenderTexTargetBinManager::_handleGFXEvent);
   GFXTextureManager::addEventDelegate( this, &RenderTexTargetBinManager::_onTextureEvent );

   mNamedTarget.setSamplerState( GFXSamplerStateDesc::getClampPoint() );
}

RenderTexTargetBinManager::~RenderTexTargetBinManager()
{
   _teardownTargets();

   GFXTextureManager::removeEventDelegate( this, &RenderTexTargetBinManager::_onTextureEvent );
   GFXDevice::getDeviceEventSignal().remove(this, &RenderTexTargetBinManager::_handleGFXEvent);
}

bool RenderTexTargetBinManager::onAdd()
{
   if(!Parent::onAdd())
      return false;

   _setupTargets();

   return true;
}

ImplementEnumType( RenderTexTargetSize,
   "What size to render the target texture. Sizes are based on the Window the render is occuring in.\n"
   "@ingroup RenderBin\n\n")
   { RenderTexTargetBinManager::WindowSize, "windowsize", "Render to the size of the window.\n" },
   { RenderTexTargetBinManager::WindowSizeScaled,  "windowsizescaled", "Render to the size of the window, scaled to the render target's size.\n" },
   { RenderTexTargetBinManager::FixedSize,  "fixedsize", "Don't scale the target texture, and render to its default size.\n" },
EndImplementEnumType;

void RenderTexTargetBinManager::initPersistFields()
{
   // TOM_TODO:
   //addField( "targetScale", mTargetScale );
   //addPropertyNOPS( "targetSizeType", mTargetSizeType)->setEnumTable(gSizeTypeEnumTable);
   //addPropertyNOPS<S32>( "targetFormat")->setEnumTable(gTextureFormatEnumTable)->addGet(this, &RenderTexTargetBinManager::getTargetFormatConsole)->addSet(this, &RenderTexTargetBinManager::setTargetFormatConsole);
   //addProperty<bool>( "blur" )->addSet(this, &RenderTexTargetBinManager::setBlur)->addGet(this, &RenderTexTargetBinManager::getBlur);

   Parent::initPersistFields();
}

bool RenderTexTargetBinManager::setTargetSize(const Point2I &newTargetSize)
{
   if( mTargetSize.x >= newTargetSize.x &&
       mTargetSize.y >= newTargetSize.y )
      return true;

   mTargetSize = newTargetSize;
   mNamedTarget.setViewport( RectI( Point2I::Zero, mTargetSize ) );

   return _updateTargets();
}

bool RenderTexTargetBinManager::setTargetFormat(const GFXFormat &newTargetFormat)
{
   if(mTargetFormat == newTargetFormat)
      return true;

   mTargetFormat = newTargetFormat;
   ConditionerFeature *conditioner = mNamedTarget.getConditioner();
   if(conditioner)
      conditioner->setBufferFormat(mTargetFormat);

   return _updateTargets();
}

void RenderTexTargetBinManager::setTargetChainLength(const U32 chainLength)
{
   if(mTargetChainLength != chainLength)
   {
      mTargetChainLength = chainLength;
      _setupTargets();
   }
}

GFXTextureObject* RenderTexTargetBinManager::getTargetTexture( U32 mrtIndex, S32 chainIndex ) const
{
   const U32 chainIdx = ( chainIndex > -1 ) ? chainIndex : mTargetChainIdx;
   if(chainIdx < mTargetChainLength)
      return mTargetChainTextures[chainIdx][mrtIndex];
   return NULL;
}

bool RenderTexTargetBinManager::_updateTargets()
{
   PROFILE_SCOPE(RenderTexTargetBinManager_updateTargets);

   bool ret = true;

   mNamedTarget.release();

   // Update the target size
   for( U32 i = 0; i < mTargetChainLength; i++ )
   {
      AssertFatal( mTargetChain != NULL, "RenderTexTargetBinManager - target chain not set up" );

      if( !mTargetChain[i] )
         mTargetChain[i] = GFX->allocRenderToTextureTarget();

      for( U32 j = 0; j < mNumRenderTargets; j++ )
      {
         // try reuse of old color texture
         if( mTargetChainTextures[i][j].getWidthHeight() != mTargetSize 
            || mTargetChainTextures[i][j].getFormat() != mTargetFormat)
         {
         ret &= mTargetChainTextures[i][j].set( mTargetSize.x, mTargetSize.y, mTargetFormat,
            &GFXDefaultRenderTargetProfile, avar( "%s() - (line %d)", __FUNCTION__, __LINE__ ),
            1, GFXTextureManager::AA_MATCH_BACKBUFFER );

         mTargetChain[i]->attachTexture( GFXTextureTarget::RenderSlot(GFXTextureTarget::Color0 + j), mTargetChainTextures[i][j] );
         }
      }
   }

   return ret;
}

bool RenderTexTargetBinManager::_handleGFXEvent( GFXDevice::GFXDeviceEventType event_ )
{
   if ( event_ == GFXDevice::deStartOfFrame )
   {
      mTargetChainIdx++;
      if ( mTargetChainIdx >= mTargetChainLength )
         mTargetChainIdx = 0;
   }

   return true;
}

void RenderTexTargetBinManager::_onTextureEvent( GFXTexCallbackCode code )
{
   switch(code)
   {
      case GFXZombify:
         _teardownTargets();
         break;

      case GFXResurrect:
         _setupTargets();
         break;
   }
}

bool RenderTexTargetBinManager::_setupTargets()
{
   _teardownTargets();

   mTargetChain = new GFXTextureTargetRef[mTargetChainLength];
   mTargetChainTextures = new GFXTexHandle*[mTargetChainLength];

   for( U32 i = 0; i < mTargetChainLength; i++ )
      mTargetChainTextures[i] = new GFXTexHandle[mNumRenderTargets];

   mTargetChainIdx = 0;

   mTargetSize = Point2I::Zero;

   return true;
}

void RenderTexTargetBinManager::_teardownTargets()
{
   mNamedTarget.release();

   SAFE_DELETE_ARRAY(mTargetChain);
   if(mTargetChainTextures != NULL)
   {
      for( U32 i = 0; i < mTargetChainLength; i++ )
         SAFE_DELETE_ARRAY(mTargetChainTextures[i]);
   }
   SAFE_DELETE_ARRAY(mTargetChainTextures);
}

GFXTextureTargetRef RenderTexTargetBinManager::_getTextureTarget(const U32 idx /* = 0 */)
{
   return mTargetChain[idx];
}

bool RenderTexTargetBinManager::_onPreRender(SceneRenderState * state, bool preserve /* = false */)
{
   PROFILE_SCOPE(RenderTexTargetBinManager_onPreRender);

#ifndef TORQUE_SHIPPING
   AssertFatal( m_NeedsOnPostRender == false, "_onPostRender not called on RenderTexTargetBinManager, or sub-class." );
   m_NeedsOnPostRender = false;
#endif

   // Update the render target size
   const Point2I &rtSize = GFX->getActiveRenderTarget()->getSize();
   switch(mTargetSizeType)
   {
   case WindowSize:
      setTargetSize(rtSize);
      break;
   case WindowSizeScaled:
      {
         Point2I scaledTargetSize(mFloor(rtSize.x * mTargetScale.x), mFloor(rtSize.y * mTargetScale.y));
         setTargetSize(scaledTargetSize);
         break;
      }
   case FixedSize:
      // No adjustment necessary
      break;
   }

   if( mTargetChainLength == 0 )
      return false;

   GFXTextureTargetRef binTarget = _getTextureTarget(mTargetChainIdx);

   if( binTarget.isNull() )
      return false;

   // Attach active depth target texture
   binTarget->attachTexture(GFXTextureTarget::DepthStencil, getRenderPass()->getDepthTargetTexture());

   // Preserve contents
   if(preserve)
      GFX->getActiveRenderTarget()->preserve();

   GFX->pushActiveRenderTarget();
   GFX->setActiveRenderTarget(binTarget);
   GFX->setViewport( mNamedTarget.getViewport() );

   #ifndef TORQUE_SHIPPING
      m_NeedsOnPostRender = true;
   #endif

   return true;
}

void RenderTexTargetBinManager::_onPostRender()
{
   PROFILE_SCOPE(RenderTexTargetBinManager_onPostRender);

   #ifndef TORQUE_SHIPPING
      m_NeedsOnPostRender = false;
   #endif

   GFXTextureTargetRef binTarget = _getTextureTarget(mTargetChainIdx);
   binTarget->resolve();

   GFX->popActiveRenderTarget();

   for ( U32 i=0; i < mNumRenderTargets; i++ )
      mNamedTarget.setTexture( i, mTargetChainTextures[mTargetChainIdx][i] );
}
