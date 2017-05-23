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
#include "scene/reflector.h"

#include "console/consoleTypes.h"
#include "gfx/gfxCubemap.h"
#include "gfx/gfxDebugEvent.h"
#include "gfx/gfxTransformSaver.h"
#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "core/stream/bitStream.h"
#include "scene/reflectionManager.h"
#include "gui/3d/guiTSControl.h"
#include "ts/tsShapeInstance.h"
#include "gfx/gfxOcclusionQuery.h"
#include "lighting/lightManager.h"
#include "lighting/shadowMap/lightShadowMap.h"
#include "math/mathUtils.h"
#include "math/util/frustum.h"
#include "gfx/screenshot.h"
#include "postFx/postEffectManager.h"

extern ColorI gCanvasClearColor;


//-------------------------------------------------------------------------
// ReflectorDesc
//-------------------------------------------------------------------------

IMPLEMENT_CO_DATABLOCK_V1( ReflectorDesc );

ConsoleDocClass( ReflectorDesc,
   "@brief A datablock which defines performance and quality properties for "
   "dynamic reflections.\n\n"

   "ReflectorDesc is not itself a reflection and does not render reflections. "
   "It is a dummy class for holding and exposing to the user a set of "
   "reflection related properties. Objects which support dynamic reflections "
   "may then reference a ReflectorDesc.\n\n"
   
   "@tsexample\n"
   "datablock ReflectorDesc( ExampleReflectorDesc )\n"
   "{\n"
   "   texSize = 256;\n"
   "   nearDist = 0.1;\n"
   "   farDist = 500;\n"
   "   objectTypeMask = 0xFFFFFFFF;\n"
   "   detailAdjust = 1.0;\n"
   "   priority = 1.0;\n"
   "   maxRateMs = 0;\n"
   "   useOcclusionQuery = true;\n"
   "};\n"
   "@endtsexample\n"

   "@see ShapeBaseData::cubeReflectorDesc\n"   
   "@ingroup enviroMisc"
);

ReflectorDesc::ReflectorDesc() 
{
   texSize = 256;
   nearDist = 0.1f;
   farDist = 1000.0f;
   objectTypeMask = 0xFFFFFFFF;
   detailAdjust = 1.0f;
   priority = 1.0f;
   maxRateMs = 15;
   useOcclusionQuery = true;
}

ReflectorDesc::~ReflectorDesc()
{
}

void ReflectorDesc::initPersistFields()
{
   addGroup( "ReflectorDesc" );

      addField( "texSize", TypeS32, Offset( texSize, ReflectorDesc ), 
         "Size in pixels of the (square) reflection texture. For a cubemap "
         "this value is interpreted as size of each face." );

      addField( "nearDist", TypeF32, Offset( nearDist, ReflectorDesc ),
         "Near plane distance to use when rendering this reflection. Adjust "
         "this to limit self-occlusion artifacts." );

      addField( "farDist", TypeF32, Offset( farDist, ReflectorDesc ),
         "Far plane distance to use when rendering reflections." );

      addField( "objectTypeMask", TypeS32, Offset( objectTypeMask, ReflectorDesc ),
         "Object types which render into this reflection." );

      addField( "detailAdjust", TypeF32, Offset( detailAdjust, ReflectorDesc ),
         "Scale applied to lod calculation of objects rendering into "
         "this reflection ( modulates $pref::TS::detailAdjust )." );

      addField( "priority", TypeF32, Offset( priority, ReflectorDesc ),
         "Priority for updating this reflection, relative to others." );

      addField( "maxRateMs", TypeS32, Offset( maxRateMs, ReflectorDesc ),
         "If less than maxRateMs has elapsed since this relfection was last "
         "updated, then do not update it again. This 'skip' can be disabled by "
         "setting maxRateMs to zero." );         

      addField( "useOcclusionQuery", TypeBool, Offset( useOcclusionQuery, ReflectorDesc ),
         "If available on the device use HOQs to determine if the reflective object "
         "is visible before updating its reflection." );

   endGroup( "ReflectorDesc" );

   Parent::initPersistFields();
}

void ReflectorDesc::packData( BitStream *stream )
{
   Parent::packData( stream );

   stream->write( texSize );
   stream->write( nearDist );
   stream->write( farDist );
   stream->write( objectTypeMask );
   stream->write( detailAdjust );
   stream->write( priority );
   stream->write( maxRateMs );
   stream->writeFlag( useOcclusionQuery );
}

void ReflectorDesc::unpackData( BitStream *stream )
{
   Parent::unpackData( stream );

   stream->read( &texSize );
   stream->read( &nearDist );
   stream->read( &farDist );
   stream->read( &objectTypeMask );
   stream->read( &detailAdjust );
   stream->read( &priority );
   stream->read( &maxRateMs );
   useOcclusionQuery = stream->readFlag();
}

bool ReflectorDesc::preload( bool server, String &errorStr )
{
   if ( !Parent::preload( server, errorStr ) )
      return false;

   return true;
}

//-------------------------------------------------------------------------
// ReflectorBase
//-------------------------------------------------------------------------
ReflectorBase::ReflectorBase()
{
   mEnabled = false;
   mOccluded = false;
   mIsRendering = false;
   mDesc = NULL;
   mObject = NULL;
   mOcclusionQuery = GFX->createOcclusionQuery();
   mQueryPending = false;
   score = 0.0f;
   lastUpdateMs = 0;
}

ReflectorBase::~ReflectorBase()
{
   delete mOcclusionQuery;
}

void ReflectorBase::unregisterReflector()
{
   if ( mEnabled )
   {
      REFLECTMGR->unregisterReflector( this );
      mEnabled = false;
   }
}

F32 ReflectorBase::calcScore( const ReflectParams &params )
{
   PROFILE_SCOPE( ReflectorBase_calcScore );

   // First check the occlusion query to see if we're hidden.
   if (  mDesc->useOcclusionQuery && 
         mOcclusionQuery )
   {
      GFXOcclusionQuery::OcclusionQueryStatus status = mOcclusionQuery->getStatus( false );
      
      if ( status == GFXOcclusionQuery::Waiting )
      {
         mQueryPending = true;
         // Don't change mOccluded since we don't know yet, use the value
         // from last frame.
      }
      else
      {
         mQueryPending = false;

         if ( status == GFXOcclusionQuery::Occluded )
            mOccluded = true;
         else if ( status == GFXOcclusionQuery::NotOccluded )
            mOccluded = false;
      }
   }

   // If we're disabled for any reason then there
   // is nothing more left to do.
   if (  !mEnabled || 
         mOccluded || 
         params.culler.isCulled( mObject->getWorldBox() ) )
   {
      score = 0;
      return score;
   }

   // This mess is calculating a score based on LOD.

   /*
   F32 sizeWS = getMax( object->getWorldBox().len_z(), 0.001f );      
   Point3F cameraOffset = params.culler.getPosition() - object->getPosition(); 
   F32 dist = getMax( cameraOffset.len(), 0.01f );      
   F32 worldToScreenScaleY = ( params.culler.getNearDist() * params.viewportExtent.y ) / 
                             ( params.culler.getNearTop() - params.culler.getNearBottom() );
   F32 sizeSS = sizeWS / dist * worldToScreenScaleY;
   */

   if ( mDesc->priority == -1.0f )
   {
      score = 1000.0f;
      return score;
   }

   F32 lodFactor = 1.0f; //sizeSS;

   F32 maxRate = getMax( (F32)mDesc->maxRateMs, 1.0f );
   U32 delta = params.startOfUpdateMs - lastUpdateMs;      
   F32 timeFactor = getMax( (F32)delta / maxRate - 1.0f, 0.0f );

   score = mDesc->priority * timeFactor * lodFactor;

   return score;
}


//-------------------------------------------------------------------------
// CubeReflector
//-------------------------------------------------------------------------

CubeReflector::CubeReflector()
 : mLastTexSize( 0 )
{
}

void CubeReflector::registerReflector( SceneObject *object, 
                                       ReflectorDesc *desc )
{
   if ( mEnabled )
      return;

   mEnabled = true;
   mObject = object;
   mDesc = desc;
   REFLECTMGR->registerReflector( this );
}

void CubeReflector::unregisterReflector()
{
   if ( !mEnabled )
      return;

   REFLECTMGR->unregisterReflector( this );

   mEnabled = false;
}

void CubeReflector::updateReflection( const ReflectParams &params )
{
   GFXDEBUGEVENT_SCOPE( CubeReflector_UpdateReflection, ColorI::WHITE );

   mIsRendering = true;

   // Setup textures and targets...
   S32 texDim = mDesc->texSize;
   texDim = getMax( texDim, 32 );

   // Protect against the reflection texture being bigger
   // than the current game back buffer.
   texDim = getMin( texDim, params.viewportExtent.x );
   texDim = getMin( texDim, params.viewportExtent.y );

   bool texResize = ( texDim != mLastTexSize );  

   const GFXFormat reflectFormat = REFLECTMGR->getReflectFormat();

   if (  texResize || 
         cubemap.isNull() ||
         cubemap->getFormat() != reflectFormat )
   {
      cubemap = GFX->createCubemap();
      cubemap->initDynamic( texDim, reflectFormat );
   }
   
   GFXTexHandle depthBuff = LightShadowMap::_getDepthTarget( texDim, texDim );

   if ( renderTarget.isNull() )
      renderTarget = GFX->allocRenderToTextureTarget();   

   GFX->pushActiveRenderTarget();
   renderTarget->attachTexture( GFXTextureTarget::DepthStencil, depthBuff );

  
   F32 oldVisibleDist = gClientSceneGraph->getVisibleDistance();
   gClientSceneGraph->setVisibleDistance( mDesc->farDist );   


   for ( U32 i = 0; i < 6; i++ )
      updateFace( params, i );
   

   GFX->popActiveRenderTarget();

   gClientSceneGraph->setVisibleDistance(oldVisibleDist);

   mIsRendering = false;
   mLastTexSize = texDim;
}

void CubeReflector::updateFace( const ReflectParams &params, U32 faceidx )
{
   GFXDEBUGEVENT_SCOPE( CubeReflector_UpdateFace, ColorI::WHITE );

   // store current matrices
   GFXTransformSaver saver;   

   // set projection to 90 degrees vertical and horizontal
   F32 left, right, top, bottom;
   MathUtils::makeFrustum( &left, &right, &top, &bottom, M_HALFPI_F, 1.0f, mDesc->nearDist );
   GFX->setFrustum( left, right, bottom, top, mDesc->nearDist, mDesc->farDist );

   // We don't use a special clipping projection, but still need to initialize 
   // this for objects like SkyBox which will use it during a reflect pass.
   gClientSceneGraph->setNonClipProjection( GFX->getProjectionMatrix() );

   // Standard view that will be overridden below.
   VectorF vLookatPt(0.0f, 0.0f, 0.0f), vUpVec(0.0f, 0.0f, 0.0f), vRight(0.0f, 0.0f, 0.0f);

   switch( faceidx )
   {
   case 0 : // D3DCUBEMAP_FACE_POSITIVE_X:
      vLookatPt = VectorF( 1.0f, 0.0f, 0.0f );
      vUpVec    = VectorF( 0.0f, 1.0f, 0.0f );
      break;
   case 1 : // D3DCUBEMAP_FACE_NEGATIVE_X:
      vLookatPt = VectorF( -1.0f, 0.0f, 0.0f );
      vUpVec    = VectorF( 0.0f, 1.0f, 0.0f );
      break;
   case 2 : // D3DCUBEMAP_FACE_POSITIVE_Y:
      vLookatPt = VectorF( 0.0f, 1.0f, 0.0f );
      vUpVec    = VectorF( 0.0f, 0.0f,-1.0f );
      break;
   case 3 : // D3DCUBEMAP_FACE_NEGATIVE_Y:
      vLookatPt = VectorF( 0.0f, -1.0f, 0.0f );
      vUpVec    = VectorF( 0.0f, 0.0f, 1.0f );
      break;
   case 4 : // D3DCUBEMAP_FACE_POSITIVE_Z:
      vLookatPt = VectorF( 0.0f, 0.0f, 1.0f );
      vUpVec    = VectorF( 0.0f, 1.0f, 0.0f );
      break;
   case 5: // D3DCUBEMAP_FACE_NEGATIVE_Z:
      vLookatPt = VectorF( 0.0f, 0.0f, -1.0f );
      vUpVec    = VectorF( 0.0f, 1.0f, 0.0f );
      break;
   }

   // create camera matrix
   VectorF cross = mCross( vUpVec, vLookatPt );
   cross.normalizeSafe();

   MatrixF matView(true);
   matView.setColumn( 0, cross );
   matView.setColumn( 1, vLookatPt );
   matView.setColumn( 2, vUpVec );
   matView.setPosition( mObject->getPosition() );
   matView.inverse();

   GFX->setWorldMatrix(matView);

   renderTarget->attachTexture( GFXTextureTarget::Color0, cubemap, faceidx );
   GFX->setActiveRenderTarget( renderTarget );
   GFX->clear( GFXClearStencil | GFXClearTarget | GFXClearZBuffer, gCanvasClearColor, 1.0f, 0 );

   SceneRenderState reflectRenderState
   (
      gClientSceneGraph,
      SPT_Reflect,
      SceneCameraState::fromGFX()
   );

   reflectRenderState.getMaterialDelegate().bind( REFLECTMGR, &ReflectionManager::getReflectionMaterial );
   reflectRenderState.setDiffuseCameraTransform( params.query->headMatrix );

   // render scene
   LIGHTMGR->registerGlobalLights( &reflectRenderState.getCullingFrustum(), false );
   gClientSceneGraph->renderSceneNoLights( &reflectRenderState, mDesc->objectTypeMask );
   LIGHTMGR->unregisterAllLights();

   // Clean up.
   renderTarget->resolve();   
}

F32 CubeReflector::calcFaceScore( const ReflectParams &params, U32 faceidx )
{
   if ( Parent::calcScore( params ) <= 0.0f )
      return score;
   
   VectorF vLookatPt(0.0f, 0.0f, 0.0f);

   switch( faceidx )
   {
   case 0 : // D3DCUBEMAP_FACE_POSITIVE_X:
      vLookatPt = VectorF( 1.0f, 0.0f, 0.0f );      
      break;
   case 1 : // D3DCUBEMAP_FACE_NEGATIVE_X:
      vLookatPt = VectorF( -1.0f, 0.0f, 0.0f );      
      break;
   case 2 : // D3DCUBEMAP_FACE_POSITIVE_Y:
      vLookatPt = VectorF( 0.0f, 1.0f, 0.0f );      
      break;
   case 3 : // D3DCUBEMAP_FACE_NEGATIVE_Y:
      vLookatPt = VectorF( 0.0f, -1.0f, 0.0f );      
      break;
   case 4 : // D3DCUBEMAP_FACE_POSITIVE_Z:
      vLookatPt = VectorF( 0.0f, 0.0f, 1.0f );      
      break;
   case 5: // D3DCUBEMAP_FACE_NEGATIVE_Z:
      vLookatPt = VectorF( 0.0f, 0.0f, -1.0f );      
      break;
   }

   VectorF cameraDir;
   params.query->cameraMatrix.getColumn( 1, &cameraDir );

   F32 dot = mDot( cameraDir, -vLookatPt );

   dot = getMax( ( dot + 1.0f ) / 2.0f, 0.1f );

   score *= dot;

   return score;
}

F32 CubeReflector::CubeFaceReflector::calcScore( const ReflectParams &params )
{
   score = cube->calcFaceScore( params, faceIdx );
   mOccluded = cube->isOccluded();
   return score;
}


//-------------------------------------------------------------------------
// PlaneReflector
//-------------------------------------------------------------------------

void PlaneReflector::registerReflector(   SceneObject *object,
                                          ReflectorDesc *desc )
{
   mEnabled = true;
   mObject = object;
   mDesc = desc;
   mLastDir = Point3F::One;
   mLastPos = Point3F::Max;

   REFLECTMGR->registerReflector( this );
}

F32 PlaneReflector::calcScore( const ReflectParams &params )
{
   if ( Parent::calcScore( params ) <= 0.0f || score >= 1000.0f )
      return score;

   // The planar reflection is view dependent to score it
   // higher if the view direction and/or position has changed.

   // Get the current camera info.
   VectorF camDir = params.query->cameraMatrix.getForwardVector();
   Point3F camPos = params.query->cameraMatrix.getPosition();

   // Scale up the score based on the view direction change.
   F32 dot = mDot( camDir, mLastDir );
   dot = ( 1.0f - dot ) * 1000.0f;
   score += dot * mDesc->priority;

   // Also account for the camera movement.
   score += ( camPos - mLastPos ).lenSquared() * mDesc->priority;   

   return score;
}

void PlaneReflector::updateReflection( const ReflectParams &params )
{
   PROFILE_SCOPE(PlaneReflector_updateReflection);   
   GFXDEBUGEVENT_SCOPE( PlaneReflector_updateReflection, ColorI::WHITE );

   mIsRendering = true;

   S32 texDim = mDesc->texSize;
   texDim = getMax( texDim, 32 );

   // Protect against the reflection texture being bigger
   // than the current game back buffer.
   texDim = getMin( texDim, params.viewportExtent.x );
   texDim = getMin( texDim, params.viewportExtent.y );

   S32 currentTarget = params.eyeId >= 0 ? params.eyeId : 0;

   const Point2I texSize = Point2I(texDim, texDim);

   bool texResize = (texSize != mLastTexSize);
   mLastTexSize = texSize;

   if (  texResize || 
         innerReflectTex[currentTarget].isNull() || 
       innerReflectTex[currentTarget]->getSize() != texSize || 
         reflectTex->getFormat() != REFLECTMGR->getReflectFormat() )
   {
      innerReflectTex[currentTarget] = REFLECTMGR->allocRenderTarget( texSize );
   }

   if ( texResize || depthBuff.isNull() )
   {
     depthBuff = LightShadowMap::_getDepthTarget(texSize.x, texSize.y);
   }

   reflectTex = innerReflectTex[currentTarget];

   // store current matrices
   GFXTransformSaver saver;

   Frustum frustum;

   S32 stereoTarget = GFX->getCurrentStereoTarget();
   if (stereoTarget != -1)
   {
      MathUtils::makeFovPortFrustum(&frustum, false, params.query->nearPlane, params.query->farPlane, params.query->fovPort[stereoTarget]);
   }
   else
   {
      Point2I viewport(params.viewportExtent);
      if (GFX->getCurrentRenderStyle() == GFXDevice::RS_StereoSideBySide)
      {
         viewport.x *= 0.5f;
      }
      F32 aspectRatio = F32(viewport.x) / F32(viewport.y);
      frustum.set(false, params.query->fov, aspectRatio, params.query->nearPlane, params.query->farPlane);
   }

   // Manipulate the frustum for tiled screenshots
   const bool screenShotMode = gScreenShot && gScreenShot->isPending();
   if ( screenShotMode )
      gScreenShot->tileFrustum( frustum );

   GFX->setFrustum( frustum );
      
   // Store the last view info for scoring.
   mLastDir = params.query->cameraMatrix.getForwardVector();
   mLastPos = params.query->cameraMatrix.getPosition();

   setGFXMatrices( params.query->cameraMatrix );

   // Adjust the detail amount
   F32 detailAdjustBackup = TSShapeInstance::smDetailAdjust;
   TSShapeInstance::smDetailAdjust *= mDesc->detailAdjust;


   if(reflectTarget.isNull())
      reflectTarget = GFX->allocRenderToTextureTarget();
   reflectTarget->attachTexture( GFXTextureTarget::Color0, innerReflectTex[currentTarget] );
   reflectTarget->attachTexture( GFXTextureTarget::DepthStencil, depthBuff );
   GFX->pushActiveRenderTarget();
   GFX->setActiveRenderTarget( reflectTarget );

   U32 objTypeFlag = -1;
   SceneCameraState reflectCameraState = SceneCameraState::fromGFX();
   LIGHTMGR->registerGlobalLights( &reflectCameraState.getFrustum(), false );

   // Since we can sometime be rendering a reflection for 1 or 2 frames before
   // it gets updated do to the lag associated with getting the results from
   // a HOQ we can sometimes see into parts of the reflection texture that
   // have nothing but clear color ( eg. under the water ).
   // To make this look less crappy use the ambient color of the sun.
   //
   // In the future we may want to fix this instead by having the scatterSky
   // render a skirt or something in its lower half.
   //
   ColorF clearColor = gClientSceneGraph->getAmbientLightColor();
   GFX->clear( GFXClearZBuffer | GFXClearStencil | GFXClearTarget, clearColor, 1.0f, 0 );

   if(GFX->getCurrentRenderStyle() == GFXDevice::RS_StereoSideBySide)
   {
      // Store previous values
      RectI originalVP = GFX->getViewport();
      MatrixF origNonClipProjection = gClientSceneGraph->getNonClipProjection();
      PFXFrameState origPFXState = PFXMGR->getFrameState();

     MatrixF inverseEyeTransforms[2];
     Frustum gfxFrustum;

     // Calculate viewport based on texture size
     RectI stereoViewports[2];
     stereoViewports[0] = params.query->stereoViewports[0];
     stereoViewports[1] = params.query->stereoViewports[1];
     stereoViewports[0].extent.x = stereoViewports[1].extent.x = texSize.x / 2;
     stereoViewports[0].extent.y = stereoViewports[1].extent.y = texSize.y;
     stereoViewports[0].point.x = 0;
     stereoViewports[1].point.x = stereoViewports[0].extent.x;

      // Calculate world transforms for eyes
      inverseEyeTransforms[0] = params.query->eyeTransforms[0];
      inverseEyeTransforms[1] = params.query->eyeTransforms[1];
      inverseEyeTransforms[0].inverse();
      inverseEyeTransforms[1].inverse();

     //
      // Render left half of display
      //

     GFX->setViewport(stereoViewports[0]);
     GFX->setCurrentStereoTarget(0);
      MathUtils::makeFovPortFrustum(&gfxFrustum, params.query->ortho, params.query->nearPlane, params.query->farPlane, params.query->fovPort[0]);
     gfxFrustum.update();
      GFX->setFrustum(gfxFrustum);

      setGFXMatrices( params.query->eyeTransforms[0] );

     SceneRenderState renderStateLeft
      (
        gClientSceneGraph,
        SPT_Reflect,
        SceneCameraState::fromGFX()
      );

      renderStateLeft.setSceneRenderStyle(SRS_SideBySide);
      renderStateLeft.getMaterialDelegate().bind( REFLECTMGR, &ReflectionManager::getReflectionMaterial );
     renderStateLeft.setDiffuseCameraTransform(params.query->headMatrix);
     //renderStateLeft.disableAdvancedLightingBins(true);

      gClientSceneGraph->renderSceneNoLights( &renderStateLeft, objTypeFlag );

     //
      // Render right half of display
     //

     GFX->setViewport(stereoViewports[1]);
     GFX->setCurrentStereoTarget(1);
     MathUtils::makeFovPortFrustum(&gfxFrustum, params.query->ortho, params.query->nearPlane, params.query->farPlane, params.query->fovPort[1]);
     gfxFrustum.update();
      GFX->setFrustum(gfxFrustum);

      setGFXMatrices( params.query->eyeTransforms[1] );

     SceneRenderState renderStateRight
     (
        gClientSceneGraph,
        SPT_Reflect,
        SceneCameraState::fromGFX()
     );

      renderStateRight.setSceneRenderStyle(SRS_SideBySide);
      renderStateRight.getMaterialDelegate().bind( REFLECTMGR, &ReflectionManager::getReflectionMaterial );
      renderStateRight.setDiffuseCameraTransform( params.query->headMatrix );
      //renderStateRight.disableAdvancedLightingBins(true);

      gClientSceneGraph->renderSceneNoLights( &renderStateRight, objTypeFlag );

      // Restore previous values
      GFX->setFrustum(frustum);
      GFX->setViewport(originalVP);
      gClientSceneGraph->setNonClipProjection(origNonClipProjection);
      PFXMGR->setFrameState(origPFXState);
     GFX->setCurrentStereoTarget(-1);
   }
   else
   {
      SceneRenderState reflectRenderState
      (
         gClientSceneGraph,
         SPT_Reflect,
         SceneCameraState::fromGFX()
      );

      reflectRenderState.getMaterialDelegate().bind( REFLECTMGR, &ReflectionManager::getReflectionMaterial );
      reflectRenderState.setDiffuseCameraTransform( params.query->headMatrix );

      gClientSceneGraph->renderSceneNoLights( &reflectRenderState, objTypeFlag );
   }

   LIGHTMGR->unregisterAllLights();

   // Clean up.
   reflectTarget->resolve();
   GFX->popActiveRenderTarget();

#ifdef DEBUG_REFLECT_TEX
   static U32 reflectStage = 0;
   char buf[128]; dSprintf(buf, 128, "F:\\REFLECT-OUT%i.PNG", reflectStage);
   //reflectTex->dumpToDisk("PNG", buf);
   reflectStage++;
   if (reflectStage > 1) reflectStage = 0;
#endif

   // Restore detail adjust amount.
   TSShapeInstance::smDetailAdjust = detailAdjustBackup;

   mIsRendering = false;
}

void PlaneReflector::setGFXMatrices( const MatrixF &camTrans )
{
   if ( objectSpace )
   {
      // set up camera transform relative to object
      MatrixF invObjTrans = mObject->getRenderTransform();
      invObjTrans.inverse();
      MatrixF relCamTrans = invObjTrans * camTrans;

      MatrixF camReflectTrans = getCameraReflection( relCamTrans );
      MatrixF camTrans = mObject->getRenderTransform() * camReflectTrans;
      camTrans.inverse();

      GFX->setWorldMatrix( camTrans );

      // use relative reflect transform for modelview since clip plane is in object space
      camTrans = camReflectTrans;
      camTrans.inverse();

      // set new projection matrix
      gClientSceneGraph->setNonClipProjection( (MatrixF&) GFX->getProjectionMatrix() );
      MatrixF clipProj = getFrustumClipProj( camTrans );
      GFX->setProjectionMatrix( clipProj );
   }    
   else
   {
      // set world mat from new camera view
      MatrixF camReflectTrans = getCameraReflection( camTrans );
      camReflectTrans.inverse();
      GFX->setWorldMatrix( camReflectTrans );

      // set new projection matrix
      gClientSceneGraph->setNonClipProjection( (MatrixF&) GFX->getProjectionMatrix() );
      MatrixF clipProj = getFrustumClipProj( camReflectTrans );
      GFX->setProjectionMatrix( clipProj );
   }   
}

MatrixF PlaneReflector::getCameraReflection( const MatrixF &camTrans )
{
   Point3F normal = refplane;

   // Figure out new cam position
   Point3F camPos = camTrans.getPosition();
   F32 dist = refplane.distToPlane( camPos );
   Point3F newCamPos = camPos - normal * dist * 2.0;

   // Figure out new look direction
   Point3F i, j, k;
   camTrans.getColumn( 0, &i );
   camTrans.getColumn( 1, &j );
   camTrans.getColumn( 2, &k );

   i = MathUtils::reflect( i, normal );
   j = MathUtils::reflect( j, normal );
   k = MathUtils::reflect( k, normal );
   //mCross( i, j, &k );


   MatrixF newTrans(true);
   newTrans.setColumn( 0, i );
   newTrans.setColumn( 1, j );
   newTrans.setColumn( 2, k );

   newTrans.setPosition( newCamPos );

   return newTrans;
}

inline F32 sgn(F32 a)
{
   if (a > 0.0F) return (1.0F);
   if (a < 0.0F) return (-1.0F);
   return (0.0F);
}

MatrixF PlaneReflector::getFrustumClipProj( MatrixF &modelview )
{
   static MatrixF rotMat(EulerF( static_cast<F32>(M_PI / 2.f), 0.0, 0.0));
   static MatrixF invRotMat(EulerF( -static_cast<F32>(M_PI / 2.f), 0.0, 0.0));


   MatrixF revModelview = modelview;
   revModelview = rotMat * revModelview;  // add rotation to modelview because it needs to be removed from projection

   // rotate clip plane into modelview space
   Point4F clipPlane;
   Point3F pnt = refplane * -(refplane.d + 0.0 );
   Point3F norm = refplane;

   revModelview.mulP( pnt );
   revModelview.mulV( norm );
   norm.normalize();

   clipPlane.set( norm.x, norm.y, norm.z, -mDot( pnt, norm ) );


   // Manipulate projection matrix
   //------------------------------------------------------------------------
   MatrixF proj = GFX->getProjectionMatrix();
   proj.mul( invRotMat );  // reverse rotation imposed by Torque
   proj.transpose();       // switch to row-major order

   // Calculate the clip-space corner point opposite the clipping plane
   // as (sgn(clipPlane.x), sgn(clipPlane.y), 1, 1) and
   // transform it into camera space by multiplying it
   // by the inverse of the projection matrix
   Vector4F   q;
   q.x = sgn(clipPlane.x) / proj(0,0);
   q.y = sgn(clipPlane.y) / proj(1,1);
   q.z = -1.0F;
   q.w = ( 1.0F - proj(2,2) ) / proj(3,2);

   F32 a = 1.0 / (clipPlane.x * q.x + clipPlane.y * q.y + clipPlane.z * q.z + clipPlane.w * q.w);

   Vector4F c = clipPlane * a;

   // CodeReview [ags 1/23/08] Come up with a better way to deal with this.
   if(GFX->getAdapterType() == OpenGL)
      c.z += 1.0f;

   // Replace the third column of the projection matrix
   proj.setColumn( 2, c );
   proj.transpose(); // convert back to column major order
   proj.mul( rotMat );  // restore Torque rotation

   return proj;
}