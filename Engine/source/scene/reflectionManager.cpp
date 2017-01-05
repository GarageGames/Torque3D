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
#include "scene/reflectionManager.h"

#include "platform/profiler.h"
#include "platform/platformTimer.h"
#include "console/consoleTypes.h"
#include "core/tAlgorithm.h"
#include "math/mMathFn.h"
#include "math/mathUtils.h"
#include "T3D/gameBase/gameConnection.h"
#include "ts/tsShapeInstance.h"
#include "gui/3d/guiTSControl.h"
#include "scene/sceneManager.h"
#include "gfx/gfxDebugEvent.h"
#include "gfx/gfxStringEnumTranslate.h"
#include "gfx/screenshot.h"
#include "core/module.h"
#include "scene/reflectionMatHook.h"
#include "console/engineAPI.h"


MODULE_BEGIN( ReflectionManager )

   MODULE_INIT
   {
      ManagedSingleton< ReflectionManager >::createSingleton();
      ReflectionManager::initConsole();
   }
   
   MODULE_SHUTDOWN
   {
      ManagedSingleton< ReflectionManager >::deleteSingleton();
   }

MODULE_END;


GFX_ImplementTextureProfile( ReflectRenderTargetProfile, 
                             GFXTextureProfile::DiffuseMap, 
                             GFXTextureProfile::PreserveSize | GFXTextureProfile::RenderTarget | GFXTextureProfile::Pooled, 
                             GFXTextureProfile::NONE );

GFX_ImplementTextureProfile( RefractTextureProfile,
                             GFXTextureProfile::DiffuseMap,
                             GFXTextureProfile::PreserveSize | 
                             GFXTextureProfile::RenderTarget |
                             GFXTextureProfile::Pooled,
                             GFXTextureProfile::NONE );

static S32 QSORT_CALLBACK compareReflectors( const void *a, const void *b )
{
   const ReflectorBase *A = *((ReflectorBase**)a);
   const ReflectorBase *B = *((ReflectorBase**)b);     
   
   F32 dif = B->score - A->score;
   return (S32)mFloor( dif );
}


U32 ReflectionManager::smFrameReflectionMS = 10;
F32 ReflectionManager::smRefractTexScale = 0.5f;

ReflectionManager::ReflectionManager() 
 : mUpdateRefract( true ),
   mReflectFormat( GFXFormatR8G8B8A8 ),
   mLastUpdateMs( 0 )
{
   mTimer = PlatformTimer::create();

   GFXDevice::getDeviceEventSignal().notify( this, &ReflectionManager::_handleDeviceEvent );
}

void ReflectionManager::initConsole()
{
   Con::addVariable( "$pref::Reflect::refractTexScale", TypeF32, &ReflectionManager::smRefractTexScale, "RefractTex has dimensions equal to the active render target scaled in both x and y by this float.\n"
      "@ingroup Rendering");
   Con::addVariable( "$pref::Reflect::frameLimitMS", TypeS32, &ReflectionManager::smFrameReflectionMS, "ReflectionManager tries not to spend more than this amount of time updating reflections per frame.\n"
      "@ingroup Rendering");
}

ReflectionManager::~ReflectionManager()
{
   SAFE_DELETE( mTimer );
   AssertFatal( mReflectors.size() == 0, "ReflectionManager, some reflectors were left nregistered!" );

   GFXDevice::getDeviceEventSignal().remove( this, &ReflectionManager::_handleDeviceEvent );
}

void ReflectionManager::registerReflector( ReflectorBase *reflector )
{
   mReflectors.push_back_unique( reflector );
}

void ReflectionManager::unregisterReflector( ReflectorBase *reflector )
{
   mReflectors.remove( reflector );   
}

void ReflectionManager::update(  F32 timeSlice, 
                                 const Point2I &resolution, 
                                 const CameraQuery &query )
{
   GFXDEBUGEVENT_SCOPE( UpdateReflections, ColorI::WHITE );

   if ( mReflectors.empty() )
      return;

   PROFILE_SCOPE( ReflectionManager_Update );

   // Calculate our target time from the slice.
   U32 targetMs = timeSlice * smFrameReflectionMS;

   // Setup a culler for testing the 
   // visibility of reflectors.
   Frustum culler;

   // jamesu - normally we just need a frustum which covers the current ports, however for SBS mode 
   // we need something which covers both viewports.
   S32 stereoTarget = GFX->getCurrentStereoTarget();
   if (stereoTarget != -1)
   {
      // In this case we're rendering in stereo using a specific eye
      MathUtils::makeFovPortFrustum(&culler, false, query.nearPlane, query.farPlane, query.fovPort[stereoTarget], query.headMatrix);
   }
   else if (GFX->getCurrentRenderStyle() == GFXDevice::RS_StereoSideBySide)
   {
      // Calculate an ideal culling size here, we'll just assume double fov based on the first fovport based on 
      // the head position.
      FovPort port = query.fovPort[0];
      F32 upSize = query.nearPlane * port.upTan;
      F32 downSize = query.nearPlane * port.downTan;

      F32 top = upSize;
      F32 bottom = -downSize;

      F32 fovInRadians = mAtan2((top - bottom) / 2.0f, query.nearPlane) * 3.0f;

      culler.set(false,
         fovInRadians,
         (F32)(query.stereoViewports[0].extent.x + query.stereoViewports[1].extent.x) / (F32)query.stereoViewports[0].extent.y,
         query.nearPlane,
         query.farPlane,
         query.headMatrix);
   }
   else
   {
      // Normal culling
      culler.set(false,
         query.fov,
         (F32)resolution.x / (F32)resolution.y,
         query.nearPlane,
         query.farPlane,
         query.cameraMatrix);
   }

   // Manipulate the frustum for tiled screenshots
   const bool screenShotMode = gScreenShot && gScreenShot->isPending();
   if ( screenShotMode )
      gScreenShot->tileFrustum( culler );

   // We use the frame time and not real time 
   // here as this may be called multiple times 
   // within a frame.
   U32 startOfUpdateMs = Platform::getVirtualMilliseconds();

   // Save this for interested parties.
   mLastUpdateMs = startOfUpdateMs;

   ReflectParams refparams;
   refparams.query = &query;
   refparams.viewportExtent = resolution;
   refparams.culler = culler;
   refparams.startOfUpdateMs = startOfUpdateMs;
   refparams.eyeId = stereoTarget;

   // Update the reflection score.
   ReflectorList::iterator reflectorIter = mReflectors.begin();
   for ( ; reflectorIter != mReflectors.end(); reflectorIter++ )
      (*reflectorIter)->calcScore( refparams );

   // Sort them by the score.
   dQsort( mReflectors.address(), mReflectors.size(), sizeof(ReflectorBase*), compareReflectors );
   
   // Update as many reflections as we can 
   // within the target time limit.
   mTimer->getElapsedMs();
   mTimer->reset();
   U32 numUpdated = 0;
   reflectorIter = mReflectors.begin();
   for ( ; reflectorIter != mReflectors.end(); reflectorIter++ )
   {      
      // We're sorted by score... so once we reach 
      // a zero score we have nothing more to update.
      if ( (*reflectorIter)->score <= 0.0f && !screenShotMode )
         break;

      (*reflectorIter)->updateReflection( refparams );

     if (stereoTarget != 0) // only update MS if we're not rendering the left eye in separate mode
     {
        (*reflectorIter)->lastUpdateMs = startOfUpdateMs;
     }

      numUpdated++;

      // If we run out of update time then stop.
      if ( mTimer->getElapsedMs() > targetMs && !screenShotMode && (*reflectorIter)->score < 1000.0f )
         break;
   }

   // Set metric/debug related script variables...

   U32 numVisible = 0;
   U32 numOccluded = 0;

   reflectorIter = mReflectors.begin();
   for ( ; reflectorIter != mReflectors.end(); reflectorIter++ )
   {      
      ReflectorBase *pReflector = (*reflectorIter);
      if ( pReflector->isOccluded() )
         numOccluded++;
      else
         numVisible++;
   }

#ifdef TORQUE_GATHER_METRICS
   U32 numEnabled = mReflectors.size();   
   U32 totalElapsed = mTimer->getElapsedMs();
   const GFXTextureProfileStats &stats = ReflectRenderTargetProfile.getStats();
   
   F32 mb = ( stats.activeBytes / 1024.0f ) / 1024.0f;
   char temp[256];

   dSprintf( temp, 256, "%s %d %0.2f\n", 
      ReflectRenderTargetProfile.getName().c_str(),
      stats.activeCount,
      mb );   

   Con::setVariable( "$Reflect::textureStats", temp );
   Con::setIntVariable( "$Reflect::renderTargetsAllocated", stats.allocatedTextures );
   Con::setIntVariable( "$Reflect::poolSize", stats.activeCount );
   Con::setIntVariable( "$Reflect::numObjects", numEnabled );   
   Con::setIntVariable( "$Reflect::numVisible", numVisible ); 
   Con::setIntVariable( "$Reflect::numOccluded", numOccluded );
   Con::setIntVariable( "$Reflect::numUpdated", numUpdated );
   Con::setIntVariable( "$Reflect::elapsed", totalElapsed );
#endif
}

GFXTexHandle ReflectionManager::allocRenderTarget( const Point2I &size )
{
   return GFXTexHandle( size.x, size.y, mReflectFormat, 
                        &ReflectRenderTargetProfile, 
                        avar("%s() - mReflectTex (line %d)", __FUNCTION__, __LINE__) );
}

GFXTextureObject* ReflectionManager::getRefractTex( bool forceUpdate )
{
   GFXTarget *target = GFX->getActiveRenderTarget();
   GFXFormat targetFormat = target->getFormat();
   const Point2I &targetSize = target->getSize();

#if defined(TORQUE_OS_XENON)
   // On the Xbox360, it needs to do a resolveTo from the active target, so this
   // may as well be the full size of the active target
   const U32 desWidth = targetSize.x;
   const U32 desHeight = targetSize.y;
#else
   U32 desWidth, desHeight;
   // D3D11 needs to be the same size as the active target
   if (GFX->getAdapterType() == Direct3D11)
   {
      desWidth = targetSize.x;
      desHeight = targetSize.y;
   }
   else
   {
      desWidth = mFloor((F32)targetSize.x * smRefractTexScale);
      desHeight = mFloor((F32)targetSize.y * smRefractTexScale);
   }
#endif

   if ( mRefractTex.isNull() || 
        mRefractTex->getWidth() != desWidth ||
        mRefractTex->getHeight() != desHeight ||
        mRefractTex->getFormat() != targetFormat )
   {
      mRefractTex.set( desWidth, desHeight, targetFormat, &RefractTextureProfile, "mRefractTex" );    
      mUpdateRefract = true;
   }

   if ( forceUpdate || mUpdateRefract )
   {
      target->resolveTo( mRefractTex );   
      mUpdateRefract = false;
   }

   return mRefractTex;
}

BaseMatInstance* ReflectionManager::getReflectionMaterial( BaseMatInstance *inMat ) const
{
   // See if we have an existing material hook.
   ReflectionMaterialHook *hook = static_cast<ReflectionMaterialHook*>( inMat->getHook( ReflectionMaterialHook::Type ) );
   if ( !hook )
   {
      // Create a hook and initialize it using the incoming material.
      hook = new ReflectionMaterialHook;
      hook->init( inMat );
      inMat->addHook( hook );
   }

   return hook->getReflectMat();
}

bool ReflectionManager::_handleDeviceEvent( GFXDevice::GFXDeviceEventType evt )
{
   switch( evt )
   {
   case GFXDevice::deStartOfFrame:
   case GFXDevice::deStartOfField:

      mUpdateRefract = true;
      break;

   case GFXDevice::deDestroy:
      
      mRefractTex = NULL;      
      break;  
      
   default:
      break;
   }

   return true;
}

DefineEngineFunction( setReflectFormat, void, ( GFXFormat format ),,
   "Set the reflection texture format.\n"
   "@ingroup GFX\n" )
{
   REFLECTMGR->setReflectFormat( format );
}

