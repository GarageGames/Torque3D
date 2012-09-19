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
#include "postFx/postEffectManager.h"

#include "postFx/postEffect.h"
#include "postFx/postEffectVis.h"
#include "renderInstance/renderBinManager.h"
#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "console/consoleTypes.h"
#include "core/module.h"


MODULE_BEGIN( PostEffectManager )

   MODULE_SHUTDOWN_AFTER( Sim )

   MODULE_INIT
   {
      ManagedSingleton< PostEffectManager >::createSingleton();
   }
   
   MODULE_SHUTDOWN
   {
      ManagedSingleton< PostEffectManager >::deleteSingleton();
   }

MODULE_END;


bool PostEffectManager::smRenderEffects = true;

PostEffectManager::PostEffectManager() : 
      mFrameStateSwitch( false ),
      mLastBackBufferTarget( NULL )
{
   GFXDevice::getDeviceEventSignal().notify( this, &PostEffectManager::_handleDeviceEvent );
   RenderPassManager::getRenderBinSignal().notify( this, &PostEffectManager::_handleBinEvent );
   SceneManager::getPostRenderSignal().notify( this, &PostEffectManager::_onPostRenderPass );

   Con::addVariable("pref::enablePostEffects", TypeBool, &smRenderEffects, 
      "@brief If true, post effects will be eanbled.\n\n"
	   "@ingroup Game");
}

PostEffectManager::~PostEffectManager()
{
   GFXDevice::getDeviceEventSignal().remove( this, &PostEffectManager::_handleDeviceEvent );
   RenderPassManager::getRenderBinSignal().remove( this, &PostEffectManager::_handleBinEvent );
   SceneManager::getPostRenderSignal().remove( this, &PostEffectManager::_onPostRenderPass );
}

bool PostEffectManager::_handleDeviceEvent( GFXDevice::GFXDeviceEventType evt )
{
   switch( evt )
   {
      case GFXDevice::deStartOfFrame:
         PFXVIS->onStartOfFrame();
         
         // Fall through

      case GFXDevice::deDestroy:

         // Free the back buffer as the device or
         // its content is now invalid.
         releaseBackBufferTex();

         break;

      case GFXDevice::deEndOfFrame:

         renderEffects( NULL, PFXEndOfFrame );

         // Toggle frame state history switch
         mFrameStateSwitch = !mFrameStateSwitch;

         break;
   
      default:
         break;
   }

   return true;
}

void PostEffectManager::_handleBinEvent( RenderBinManager *bin,
                                                const SceneRenderState* sceneState,
                                                bool isBinStart )
{
   if (  sceneState->isShadowPass() || 
         sceneState->isOtherPass() )
      return;

   // We require a bin name to process effects... without
   // it we can skip the bin entirely.
   String binName( bin->getName() );
   if ( binName.isEmpty() )
      return;

   renderEffects( sceneState, isBinStart ? PFXBeforeBin : PFXAfterBin, binName );
}

void PostEffectManager::_onPostRenderPass( SceneManager *sceneGraph, const SceneRenderState *sceneState )
{
   if ( !sceneState->isDiffusePass() )
      return;

   renderEffects( sceneState, PFXAfterDiffuse );
}

GFXTextureObject* PostEffectManager::getBackBufferTex()
{
   GFXTarget *target = GFX->getActiveRenderTarget();

   if (  mBackBufferCopyTex.isNull() ||
         target != mLastBackBufferTarget )
   {
      const Point2I &targetSize = target->getSize();
      GFXFormat targetFormat = target->getFormat();

      mBackBufferCopyTex.set( targetSize.x, targetSize.y, 
                              targetFormat, 
                              &PostFxTargetProfile, "mBackBufferCopyTex" );

      target->resolveTo( mBackBufferCopyTex );
      mLastBackBufferTarget = target;
   }

   return mBackBufferCopyTex;
}

void PostEffectManager::releaseBackBufferTex()
{
   mBackBufferCopyTex = NULL;
   mLastBackBufferTarget = NULL;
}

bool PostEffectManager::_addEffect( PostEffect *effect )
{
   EffectVector *effects = NULL;

   const String &binName = effect->getRenderBin();

   switch( effect->getRenderTime() )
   { 
   case PFXAfterDiffuse:
      effects = &mAfterDiffuseList;
      break;

   case PFXEndOfFrame:
      effects = &mEndOfFrameList;
      break;

   case PFXBeforeBin:
      effects = &mBeforeBinMap[binName];
      break;

   case PFXAfterBin:
      effects = &mAfterBinMap[binName];
      break;
   
   case PFXTexGenOnDemand:
   	break;
   }

   if ( effects == NULL )
      return false;

   effects->push_back( effect );

   // Resort the effects by priority.
   effects->sort( &_effectPrioritySort );

   return true;
}

bool PostEffectManager::_removeEffect( PostEffect *effect )
{
   // Check the end of frame list.
   EffectVector::iterator iter = find( mEndOfFrameList.begin(), mEndOfFrameList.end(), effect );
   if ( iter != mEndOfFrameList.end() )
   {
      mEndOfFrameList.erase( iter );
      return true;
   }

   // Check the diffuse list.
   iter = find( mAfterDiffuseList.begin(), mAfterDiffuseList.end(), effect );
   if ( iter != mAfterDiffuseList.end() )
   {
      mAfterDiffuseList.erase( iter );
      return true;
   }

   // Now check the bin maps.
   EffectMap::Iterator mapIter = mAfterBinMap.begin();
   for( ; mapIter != mAfterBinMap.end(); mapIter++ )
   {
      EffectVector &effects = mapIter->value;
      iter = find( effects.begin(), effects.end(), effect );
      if ( iter != effects.end() )
      {
         effects.erase( iter );
         return true;
      }
   }

   mapIter = mBeforeBinMap.begin();
   for( ; mapIter != mBeforeBinMap.end(); mapIter++ )
   {
      EffectVector &effects = mapIter->value;
      iter = find( effects.begin(), effects.end(), effect );
      if ( iter != effects.end() )
      {
         effects.erase( iter );
         return true;
      }
   }

   return false;
}

void PostEffectManager::renderEffects( const SceneRenderState *state,
                                       const PFXRenderTime effectTiming, 
                                       const String &binName )
{
   // MACHAX - The proper fix is to ensure that PostFX do not get rendered if 
   // their shader failed to load.
#ifdef TORQUE_OS_MAC
   return;
#endif

   // Check the global render effect state as 
   // well as the 
   if (  !smRenderEffects || 
         ( state && !state->usePostEffects() ))
      return;

   EffectVector *effects = NULL;

   switch( effectTiming )
   {
      case PFXBeforeBin:
         effects = &mBeforeBinMap[binName];
         break;

      case PFXAfterBin:
         effects = &mAfterBinMap[binName];
         break;

      case PFXAfterDiffuse:
         effects = &mAfterDiffuseList;
         break;

      case PFXEndOfFrame:
         effects = &mEndOfFrameList;
         break;
      
      case PFXTexGenOnDemand:
      	break;
   }

   AssertFatal( effects != NULL, "Bad effect time" );

   // Skip out if we don't have any effects.
   if ( effects->empty() )
      return;

   // This is used to pass the output texture
   // of one effect into the next effect.
   GFXTexHandle chainTex;

   // Process the effects.
   for ( U32 i = 0; i < effects->size(); i++ )
   {
      PostEffect *effect = (*effects)[i];
      AssertFatal( effect != NULL, "Somehow this happened" );
      effect->process( state, chainTex );
   }
}

void PostEffectManager::setFrameMatrices( const MatrixF &worldToCamera, const MatrixF &cameraToScreen )
{
   PFXFrameState &thisFrame = mFrameState[mFrameStateSwitch];
   thisFrame.worldToCamera = worldToCamera;
   thisFrame.cameraToScreen = cameraToScreen;
}

S32 PostEffectManager::_effectPrioritySort( PostEffect* const *e1, PostEffect* const *e2 )
{
   F32 p1 = (*e1)->getPriority();
   F32 p2 = (*e2)->getPriority();

   if( p1 > p2 )
		return -1;
   else if( p1 < p2 )
		return 1;

   return 0;
}
