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
#include "scene/sceneManager.h"

#include "scene/sceneObject.h"
#include "scene/zones/sceneTraversalState.h"
#include "scene/sceneRenderState.h"
#include "scene/zones/sceneRootZone.h"
#include "scene/zones/sceneZoneSpace.h"
#include "lighting/lightManager.h"
#include "renderInstance/renderPassManager.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/gfxDebugEvent.h"
#include "console/engineAPI.h"
#include "sim/netConnection.h"
#include "T3D/gameBase/gameConnection.h"
#include "math/mathUtils.h"

// For player object bounds workaround.
#include "T3D/player.h"

extern bool gEditingMission;


MODULE_BEGIN( Scene )

   MODULE_INIT_AFTER( Sim )   
   MODULE_SHUTDOWN_BEFORE( Sim )
   
   MODULE_INIT
   {
      // Client scene.
      gClientSceneGraph = new SceneManager( true );

      // Server scene.
      gServerSceneGraph = new SceneManager( false );

      Con::addVariable( "$Scene::lockCull", TypeBool, &SceneManager::smLockDiffuseFrustum,
         "Debug tool which locks the frustum culling to the current camera location.\n"
         "@ingroup Rendering\n" );

      Con::addVariable( "$Scene::disableTerrainOcclusion", TypeBool, &SceneCullingState::smDisableTerrainOcclusion,
         "Used to disable the somewhat expensive terrain occlusion testing.\n"
         "@ingroup Rendering\n" );

      Con::addVariable( "$Scene::disableZoneCulling", TypeBool, &SceneCullingState::smDisableZoneCulling,
         "If true, zone culling will be disabled and the scene contents will only be culled against the root frustum.\n\n"
         "@ingroup Rendering\n" );

      Con::addVariable( "$Scene::renderBoundingBoxes", TypeBool, &SceneManager::smRenderBoundingBoxes,
         "If true, the bounding boxes of objects will be displayed.\n\n"
         "@ingroup Rendering" );

      Con::addVariable( "$Scene::maxOccludersPerZone", TypeS32, &SceneCullingState::smMaxOccludersPerZone,
         "Maximum number of occluders that will be concurrently allowed into the scene culling state of any given zone.\n\n"
         "@ingroup Rendering" );

      Con::addVariable( "$Scene::occluderMinWidthPercentage", TypeF32, &SceneCullingState::smOccluderMinWidthPercentage,
         "TODO\n\n"
         "@ingroup Rendering" );

      Con::addVariable( "$Scene::occluderMinHeightPercentage", TypeF32, &SceneCullingState::smOccluderMinHeightPercentage,
         "TODO\n\n"
         "@ingroup Rendering" );
   }
   
   MODULE_SHUTDOWN
   {
      SAFE_DELETE( gClientSceneGraph );
      SAFE_DELETE( gServerSceneGraph );
   }

MODULE_END;


bool SceneManager::smRenderBoundingBoxes;
bool SceneManager::smLockDiffuseFrustum = false;
SceneCameraState SceneManager::smLockedDiffuseCamera = SceneCameraState( RectI(), Frustum(), MatrixF(), MatrixF() );

SceneManager* gClientSceneGraph = NULL;
SceneManager* gServerSceneGraph = NULL;


//-----------------------------------------------------------------------------

SceneManager::SceneManager( bool isClient )
   : mLightManager( NULL ),
     mCurrentRenderState( NULL ),
     mIsClient( isClient ),
     mUsePostEffectFog( true ),
     mDisplayTargetResolution( 0, 0 ),
     mDefaultRenderPass( NULL ),
     mVisibleDistance( 500.f ),
     mVisibleGhostDistance( 0 ),
     mNearClip( 0.1f ),
     mAmbientLightColor( ColorF( 0.1f, 0.1f, 0.1f, 1.0f ) ),
     mZoneManager( NULL )
{
   VECTOR_SET_ASSOCIATION( mBatchQueryList );

   // For the client, create a zone manager.

   if( isClient )
   {
      mZoneManager = new SceneZoneSpaceManager( getContainer() );

      // Add the root zone to the scene.

      addObjectToScene( mZoneManager->getRootZone() );
   }
}

//-----------------------------------------------------------------------------

SceneManager::~SceneManager()
{   
   SAFE_DELETE( mZoneManager );

   if( mLightManager )
      mLightManager->deactivate();   
}

//-----------------------------------------------------------------------------

void SceneManager::renderScene( ScenePassType passType, U32 objectMask )
{
   SceneCameraState cameraState = SceneCameraState::fromGFX();
   
   // Handle frustum locking.

   const bool lockedFrustum = ( smLockDiffuseFrustum && passType == SPT_Diffuse );
   if( lockedFrustum )
      cameraState = smLockedDiffuseCamera;
   else if( passType == SPT_Diffuse )
   {
      // Store the camera state so if we lock, this will become the
      // locked state.
      smLockedDiffuseCamera = cameraState;
   }
   
   // Create the render state.

   SceneRenderState renderState( this, passType, cameraState );

   // If we have locked the frustum, reset the view transform
   // on the render pass which the render state has just set
   // to the view matrix corresponding to the locked frustum.  For
   // rendering, however, we need the true view matrix from the
   // GFX state.

   if( lockedFrustum )
   {
      RenderPassManager* rpm = renderState.getRenderPass();
      rpm->assignSharedXform( RenderPassManager::View, GFX->getWorldMatrix() );
   }

   // Render.

   renderScene( &renderState, objectMask );
}

//-----------------------------------------------------------------------------

void SceneManager::renderScene( SceneRenderState* renderState, U32 objectMask, SceneZoneSpace* baseObject, U32 baseZone )
{
   PROFILE_SCOPE( SceneGraph_renderScene );

   // Get the lights for rendering the scene.

   PROFILE_START( SceneGraph_registerLights );
      LIGHTMGR->registerGlobalLights( &renderState->getCullingFrustum(), false );
   PROFILE_END();

   // If its a diffuse pass, update the current ambient light level.
   // To do that find the starting zone and determine whether it has a custom
   // ambient light color.  If so, pass it on to the ambient light manager.
   // If not, use the ambient light color of the sunlight.
   //
   // Note that we retain the starting zone information here and pass it
   // on to renderSceneNoLights so that we don't need to look it up twice.

   if( renderState->isDiffusePass() )
   {
      if( !baseObject && getZoneManager() )
      {
         getZoneManager()->findZone( renderState->getCameraPosition(), baseObject, baseZone );
         AssertFatal( baseObject != NULL, "SceneManager::renderScene - findZone() did not return an object" );
      }

      ColorF zoneAmbient;
      if( baseObject && baseObject->getZoneAmbientLightColor( baseZone, zoneAmbient ) )
         mAmbientLightColor.setTargetValue( zoneAmbient );
      else
      {
         const LightInfo* sunlight = LIGHTMGR->getSpecialLight( LightManager::slSunLightType );
         if( sunlight )
            mAmbientLightColor.setTargetValue( sunlight->getAmbient() );
      }

      renderState->setAmbientLightColor( mAmbientLightColor.getCurrentValue() );
   }

   // Trigger the pre-render signal.

   PROFILE_START( SceneGraph_preRenderSignal);
      mCurrentRenderState = renderState;
      getPreRenderSignal().trigger( this, renderState );
      mCurrentRenderState = NULL;
   PROFILE_END();

   // Render the scene.

   if(GFX->getCurrentRenderStyle() == GFXDevice::RS_StereoSideBySide)
   {
      // Store previous values
      RectI originalVP = GFX->getViewport();
      MatrixF originalWorld = GFX->getWorldMatrix();
      Frustum originalFrustum = GFX->getFrustum();

      Point2F projOffset = GFX->getCurrentProjectionOffset();
      const FovPort *currentFovPort = GFX->getStereoFovPort();
      const MatrixF *eyeTransforms = GFX->getStereoEyeTransforms();
      const MatrixF *worldEyeTransforms = GFX->getInverseStereoEyeTransforms();

      // Render left half of display
      GFX->activateStereoTarget(0);
      GFX->beginField();

      GFX->setWorldMatrix(worldEyeTransforms[0]);

      Frustum gfxFrustum = originalFrustum;
      MathUtils::makeFovPortFrustum(&gfxFrustum, gfxFrustum.isOrtho(), gfxFrustum.getNearDist(), gfxFrustum.getFarDist(), currentFovPort[0], eyeTransforms[0]);
      GFX->setFrustum(gfxFrustum);

      SceneCameraState cameraStateLeft = SceneCameraState::fromGFX();
      SceneRenderState renderStateLeft( this, renderState->getScenePassType(), cameraStateLeft );
      renderStateLeft.setSceneRenderStyle(SRS_SideBySide);
      renderStateLeft.setSceneRenderField(0);

      renderSceneNoLights( &renderStateLeft, objectMask, baseObject, baseZone );

      // Indicate that we've just finished a field
      //GFX->clear(GFXClearTarget | GFXClearZBuffer | GFXClearStencil, ColorI(255,0,0), 1.0f, 0);
      GFX->endField();
      
      // Render right half of display
      GFX->activateStereoTarget(1);
      GFX->beginField();
      GFX->setWorldMatrix(worldEyeTransforms[1]);

      gfxFrustum = originalFrustum;
      MathUtils::makeFovPortFrustum(&gfxFrustum, gfxFrustum.isOrtho(), gfxFrustum.getNearDist(), gfxFrustum.getFarDist(), currentFovPort[1], eyeTransforms[1]);
      GFX->setFrustum(gfxFrustum);

      SceneCameraState cameraStateRight = SceneCameraState::fromGFX();
      SceneRenderState renderStateRight( this, renderState->getScenePassType(), cameraStateRight );
      renderStateRight.setSceneRenderStyle(SRS_SideBySide);
      renderStateRight.setSceneRenderField(1);

      renderSceneNoLights( &renderStateRight, objectMask, baseObject, baseZone );

      // Indicate that we've just finished a field
      //GFX->clear(GFXClearTarget | GFXClearZBuffer | GFXClearStencil, ColorI(0,255,0), 1.0f, 0);
      GFX->endField();

      // Restore previous values
      GFX->setWorldMatrix(originalWorld);
      GFX->setFrustum(originalFrustum);
      GFX->setViewport(originalVP);
   }
   else
   {
      renderSceneNoLights( renderState, objectMask, baseObject, baseZone );
   }

   // Trigger the post-render signal.

   PROFILE_START( SceneGraphRender_postRenderSignal );
      mCurrentRenderState = renderState;
      getPostRenderSignal().trigger( this, renderState );
      mCurrentRenderState = NULL;
   PROFILE_END();

   // Remove the previously registered lights.

   PROFILE_START( SceneGraph_unregisterLights);
      LIGHTMGR->unregisterAllLights();
   PROFILE_END();
}

//-----------------------------------------------------------------------------

void SceneManager::renderSceneNoLights( SceneRenderState* renderState, U32 objectMask, SceneZoneSpace* baseObject, U32 baseZone )
{
   // Set the current state.

   mCurrentRenderState = renderState;

   // Render.

   _renderScene( mCurrentRenderState, objectMask, baseObject, baseZone );

   #ifdef TORQUE_DEBUG

   // If frustum is locked and this is a diffuse pass, render the culling volumes of
   // zones that are selected (or the volumes of the outdoor zone if no zone is
   // selected).

   if( gEditingMission && renderState->isDiffusePass() && smLockDiffuseFrustum )
      renderState->getCullingState().debugRenderCullingVolumes();
   
   #endif

   mCurrentRenderState = NULL;
}

//-----------------------------------------------------------------------------

void SceneManager::_renderScene( SceneRenderState* state, U32 objectMask, SceneZoneSpace* baseObject, U32 baseZone )
{
   AssertFatal( this == gClientSceneGraph, "SceneManager::_buildSceneGraph - Only the client scenegraph can support this call!" );

   PROFILE_SCOPE( SceneGraph_batchRenderImages );

   // In the editor, override the type mask for diffuse passes.

   if( gEditingMission && state->isDiffusePass() )
      objectMask = EDITOR_RENDER_TYPEMASK;

   // Update the zoning state and traverse zones.

   if( getZoneManager() )
   {
      // Update.

      getZoneManager()->updateZoningState();

      // If zone culling isn't disabled, traverse the
      // zones now.

      if( !state->getCullingState().disableZoneCulling() )
      {
         // Find the start zone if we haven't already.

         if( !baseObject )
         {
            getZoneManager()->findZone( state->getCameraPosition(), baseObject, baseZone );
            AssertFatal( baseObject != NULL, "SceneManager::_renderScene - findZone() did not return an object" );
         }

         // Traverse zones starting in base object.

         SceneTraversalState traversalState( &state->getCullingState() );
         PROFILE_START( Scene_traverseZones );
         baseObject->traverseZones( &traversalState, baseZone );
         PROFILE_END();

         // Set the scene render box to the area we have traversed.

         state->setRenderArea( traversalState.getTraversedArea() );
      }
   }

   // Set the query box for the container query.  Never
   // make it larger than the frustum's AABB.  In the editor,
   // always query the full frustum as that gives objects
   // the opportunity to render editor visualizations even if
   // they are otherwise not in view.

   if( !state->getCullingFrustum().getBounds().isOverlapped( state->getRenderArea() ) )
   {
      // This handles fringe cases like flying backwards into a zone where you
      // end up pretty much standing on a zone border and looking directly into
      // its "walls".  In that case the traversal area will be behind the frustum
      // (remember that the camera isn't where visibility starts, it's the near
      // distance).

      return;
   }

   Box3F queryBox = state->getCullingFrustum().getBounds();
   if( !gEditingMission )
   {
      queryBox.minExtents.setMax( state->getRenderArea().minExtents );
      queryBox.maxExtents.setMin( state->getRenderArea().maxExtents );
   }

   PROFILE_START( Scene_cullObjects );

   //TODO: We should split the codepaths here based on whether the outdoor zone has visible space.
   //    If it has, we should use the container query-based path.
   //    If it hasn't, we should fill the object list directly from the zone lists which will usually
   //       include way fewer objects.
   
   // Gather all objects that intersect the scene render box.

   mBatchQueryList.clear();
   getContainer()->findObjectList( queryBox, objectMask, &mBatchQueryList );

   // Cull the list.

   U32 numRenderObjects = state->getCullingState().cullObjects(
      mBatchQueryList.address(),
      mBatchQueryList.size(),
      !state->isDiffusePass() ? SceneCullingState::CullEditorOverrides : 0 // Keep forced editor stuff out of non-diffuse passes.
   );

   //HACK: If the control object is a Player and it is not in the render list, force
   // it into it.  This really should be solved by collision bounds being separate from
   // object bounds; only because the Player class is using bounds not encompassing
   // the actual player object is it that we have this problem in the first place.
   // Note that we are forcing the player object into ALL passes here but such
   // is the power of proliferation of things done wrong.

   GameConnection* connection = GameConnection::getConnectionToServer();
   if( connection )
   {
      Player* player = dynamic_cast< Player* >( connection->getControlObject() );
      if( player )
      {
         mBatchQueryList.setSize( numRenderObjects );
         if( !mBatchQueryList.contains( player ) )
         {
            mBatchQueryList.push_back( player );
            numRenderObjects ++;
         }
      }
   }

   PROFILE_END();

   // Render the remaining objects.

   PROFILE_START( Scene_renderObjects );
   state->renderObjects( mBatchQueryList.address(), numRenderObjects );
   PROFILE_END();

   // Render bounding boxes, if enabled.

   if( smRenderBoundingBoxes && state->isDiffusePass() )
   {
      GFXDEBUGEVENT_SCOPE( Scene_renderBoundingBoxes, ColorI::WHITE );

      GameBase* cameraObject = 0;
      if( connection )
         cameraObject = connection->getCameraObject();

      GFXStateBlockDesc desc;
      desc.setFillModeWireframe();
      desc.setZReadWrite( true, false );

      for( U32 i = 0; i < numRenderObjects; ++ i )
      {
         SceneObject* object = mBatchQueryList[ i ];

         // Skip global bounds object.
         if( object->isGlobalBounds() )
            continue;

         // Skip camera object as we're viewing the scene from it.
         if( object == cameraObject )
            continue;

         const Box3F& worldBox = object->getWorldBox();
         GFX->getDrawUtil()->drawObjectBox(
            desc,
            Point3F( worldBox.len_x(), worldBox.len_y(), worldBox.len_z() ),
            worldBox.getCenter(),
            MatrixF::Identity,
            ColorI::WHITE
         );
      }
   }
}

//-----------------------------------------------------------------------------

struct ScopingInfo
{
   Point3F        scopePoint;
   F32            scopeDist;
   F32            scopeDistSquared;
   NetConnection* connection;
};

static void _scopeCallback( SceneObject* object, void* data )
{
   if( !object->isScopeable() )
      return;

   ScopingInfo* info = reinterpret_cast< ScopingInfo* >( data );
   NetConnection* connection = info->connection;

   F32 difSq = ( object->getWorldSphere().center - info->scopePoint ).lenSquared();
   if( difSq < info->scopeDistSquared )
   {
      // Not even close, it's in...
      connection->objectInScope( object );
   }
   else
   {
      // Check a little more closely...
      F32 realDif = mSqrt( difSq );
      if( realDif - object->getWorldSphere().radius < info->scopeDist)
         connection->objectInScope( object );
   }
}

void SceneManager::scopeScene( CameraScopeQuery* query, NetConnection* netConnection )
{
   PROFILE_SCOPE( SceneGraph_scopeScene );

   // Note that this method does not use the zoning information in the scene
   // to scope objects.  The reason is that with the way that scoping is implemented
   // in the networking layer--i.e. by killing off ghosts of objects that are out
   // of scope--, it doesn't make sense to let, for example, all objects in the outdoor
   // zone go out of scope, just because there is no exterior portal that is visible from
   // the current camera viewpoint (in any direction).
   //
   // So, we perform a simple box query on the area covered by the camera query
   // and then scope in everything that is in range.
   
   // Set up scoping info.

   ScopingInfo info;

   info.scopePoint       = query->pos;
   info.scopeDist        = query->visibleDistance;
   info.scopeDistSquared = info.scopeDist * info.scopeDist;
   info.connection       = netConnection;

   // Scope all objects in the query area.

   Box3F area( query->visibleDistance );
   area.setCenter( query->pos );

   getContainer()->findObjects( area, 0xFFFFFFFF, _scopeCallback, &info );
}

//-----------------------------------------------------------------------------

bool SceneManager::addObjectToScene( SceneObject* object )
{
   AssertFatal( !object->mSceneManager, "SceneManager::addObjectToScene - Object already part of a scene" );

   // Mark the object as belonging to us.

   object->mSceneManager = this;

   // Register with managers except its the root zone.

   if( !dynamic_cast< SceneRootZone* >( object ) )
   {
      // Add to container.

      getContainer()->addObject( object );

      // Register the object with the zone manager.

      if( getZoneManager() )
         getZoneManager()->registerObject( object );
   }

   // Notify the object.

   return object->onSceneAdd();
}

//-----------------------------------------------------------------------------

void SceneManager::removeObjectFromScene( SceneObject* obj )
{
   AssertFatal( obj, "SceneManager::removeObjectFromScene - Object is not declared" );
   AssertFatal( obj->getSceneManager() == this, "SceneManager::removeObjectFromScene - Object not part of SceneManager" );

   // Notify the object.

   obj->onSceneRemove();

   // Remove the object from the container.

   if( getContainer() )
      getContainer()->removeObject( obj );

   // Remove the object from the zoning system.

   if( getZoneManager() )
      getZoneManager()->unregisterObject( obj );

   // Clear out the reference to us.

   obj->mSceneManager = NULL;
}

//-----------------------------------------------------------------------------

void SceneManager::notifyObjectDirty( SceneObject* object )
{
   // Update container state.

   if( object->mContainer )
      object->mContainer->checkBins( object );

   // Mark zoning state as dirty.

   if( getZoneManager() )
      getZoneManager()->notifyObjectChanged( object );
}

//-----------------------------------------------------------------------------

void SceneManager::setDisplayTargetResolution( const Point2I &size )
{
   mDisplayTargetResolution = size;
}

//-----------------------------------------------------------------------------

const Point2I & SceneManager::getDisplayTargetResolution() const
{
   return mDisplayTargetResolution;
}

//-----------------------------------------------------------------------------

bool SceneManager::setLightManager( const char* lmName )
{
   LightManager *lm = LightManager::findByName( lmName );
   if ( !lm )
      return false;

   return _setLightManager( lm );
}

//-----------------------------------------------------------------------------

bool SceneManager::_setLightManager( LightManager* lm )
{
   // Avoid unnecessary work reinitializing materials.
   if ( lm == mLightManager )
      return true;

   // Make sure its valid... else fail!
   if ( !lm->isCompatible() )
      return false;

   // We only deactivate it... all light managers are singletons
   // and will manager their own lifetime.
   if ( mLightManager )
      mLightManager->deactivate();

   mLightManager = lm;

   if ( mLightManager )
      mLightManager->activate( this );

   return true;
}

//-----------------------------------------------------------------------------

RenderPassManager* SceneManager::getDefaultRenderPass() const
{
   if( !mDefaultRenderPass )
   {
      Sim::findObject( "DiffuseRenderPassManager", mDefaultRenderPass );
      AssertISV( mDefaultRenderPass, "SceneManager::_setDefaultRenderPass - No DiffuseRenderPassManager defined!  Must be set up in script!" );
   }

   return mDefaultRenderPass;
}

//=============================================================================
//    Console API.
//=============================================================================
// MARK: ---- Console API ----

//-----------------------------------------------------------------------------

DefineConsoleFunction( sceneDumpZoneStates, void, ( bool updateFirst ), ( true ),
   "Dump the current zoning states of all zone spaces in the scene to the console.\n\n"
   "@param updateFirst If true, zoning states are brought up to date first; if false, the zoning states "
   "are dumped as is.\n\n"
   "@note Only valid on the client.\n"
   "@ingroup Game" )
{
   if( !gClientSceneGraph )
   {
      Con::errorf( "sceneDumpZoneStates - Only valid on client!" );
      return;
   }

   SceneZoneSpaceManager* manager = gClientSceneGraph->getZoneManager();
   if( !manager )
   {
      Con::errorf( "sceneDumpZoneStates - Scene is not using zones!" );
      return;
   }

   manager->dumpZoneStates( updateFirst );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( sceneGetZoneOwner, SceneObject*, ( U32 zoneId ), ( true ),
   "Return the SceneObject that contains the given zone.\n\n"
   "@param zoneId ID of zone.\n"
   "@return A SceneObject or NULL if the given @a zoneId is invalid.\n\n"
   "@note Only valid on the client.\n"
   "@ingroup Game" )
{
   if( !gClientSceneGraph )
   {
      Con::errorf( "sceneGetZoneOwner - Only valid on client!" );
      return NULL;
   }

   SceneZoneSpaceManager* manager = gClientSceneGraph->getZoneManager();
   if( !manager )
   {
      Con::errorf( "sceneGetZoneOwner - Scene is not using zones!" );
      return NULL;
   }

   if( !manager->isValidZoneId( zoneId ) )
   {
      Con::errorf( "sceneGetZoneOwner - Invalid zone ID: %i", zoneId );
      return NULL;
   }

   return manager->getZoneOwner( zoneId );
}
