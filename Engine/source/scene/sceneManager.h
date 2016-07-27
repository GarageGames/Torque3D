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

#ifndef _SCENEMANAGER_H_
#define _SCENEMANAGER_H_

#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif

#ifndef _SCENEZONESPACEMANAGER_H_
#include "scene/zones/sceneZoneSpaceManager.h"
#endif

#ifndef _MRECT_H_
#include "math/mRect.h"
#endif

#ifndef _COLOR_H_
#include "core/color.h"
#endif

#ifndef _INTERPOLATEDCHANGEPROPERTY_H_
#include "util/interpolatedChangeProperty.h"
#endif

#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif

#ifndef _FOGSTRUCTS_H_
#include "scene/fogStructs.h"
#endif

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

#ifndef _TSIGNAL_H_
#include "core/util/tSignal.h"
#endif


class LightManager;
class SceneRootZone;
class SceneRenderState;
class SceneCameraState;
class SceneZoneSpace;
class NetConnection;
class RenderPassManager;


/// The type of scene pass.
/// @see SceneManager
/// @see SceneRenderState
enum ScenePassType
{
   /// The regular diffuse scene pass.
   SPT_Diffuse,

   /// The scene pass made for reflection rendering.
   SPT_Reflect,

   /// The scene pass made for shadow map rendering.
   SPT_Shadow,

   /// A scene pass that isn't one of the other 
   /// predefined scene pass types.
   SPT_Other,
};


/// The type of scene render style
/// @see SceneRenderState
enum SceneRenderStyle
{
   /// The regular style of rendering
   SRS_Standard,

   /// Side-by-side style rendering
   SRS_SideBySide,
};


/// An object that manages the SceneObjects belonging to a scene.
class SceneManager
{
   public:

      /// A signal used to notify of render passes.
      typedef Signal< void( SceneManager*, const SceneRenderState* ) > RenderSignal;

      /// If true use the last stored locked frustum for culling
      /// the diffuse render pass.
      /// @see smLockedDiffuseFrustum
      static bool smLockDiffuseFrustum;

      /// If true, render the AABBs of objects for debugging.
      static bool smRenderBoundingBoxes;

      //A cache list of objects that made it through culling, so we don't have to attempt to re-test
      //visibility of objects later.
      Vector< SceneObject* > mRenderedObjectsList;

   protected:

      /// Whether this is the client-side scene.
      bool mIsClient;

      /// Manager for the zones in this scene.
      SceneZoneSpaceManager* mZoneManager;

      // NonClipProjection is the projection matrix without oblique frustum clipping
      // applied to it (in reflections)
      MatrixF mNonClipProj;

      ///
      bool mUsePostEffectFog;

      /// @see setDisplayTargetResolution
      Point2I mDisplayTargetResolution;

      /// The currently active render state or NULL if we're
      /// not in the process of rendering.
      SceneRenderState* mCurrentRenderState;

      F32 mVisibleDistance;

      F32 mVisibleGhostDistance;
      F32 mNearClip;

      FogData mFogData;

      WaterFogData mWaterFogData;

      /// The stored last diffuse pass frustum for locking the cull.
      static SceneCameraState smLockedDiffuseCamera;

      /// @name Lighting
      /// @{

      typedef InterpolatedChangeProperty< ColorF > AmbientLightInterpolator;

      /// Light manager that is active for the scene.
      LightManager* mLightManager;

      /// Global ambient light level in the scene.
      AmbientLightInterpolator mAmbientLightColor;

      /// Deactivates the previous light manager and activates the new one.
      bool _setLightManager( LightManager *lm );

      /// @}

      /// @name Rendering
      /// @{

      /// RenderPassManager for the default render pass.  This is set up
      /// in script and looked up by getDefaultRenderPass().
      mutable RenderPassManager* mDefaultRenderPass;

      ///
      Vector< SceneObject* > mBatchQueryList;

      /// Render scene using the given state.
      ///
      /// @param state SceneManager render state.
      /// @param objectMask Object type mask with which to filter scene objects.
      /// @param baseObject Zone manager to start traversal in.  If null, the zone manager
      ///   that contains @a state's camera position will be used.
      /// @param baseZone Zone in @a zone manager in which to start traversal.  Ignored if
      ///   @a baseObject is NULL.
      void _renderScene(   SceneRenderState* state,
                           U32 objectMask = ( U32 ) -1,
                           SceneZoneSpace* baseObject = NULL,
                           U32 baseZone = 0 );

      /// Callback for the container query.
      static void _batchObjectCallback( SceneObject* object, void* key );

      /// @}

   public:

      SceneManager( bool isClient );
      ~SceneManager();

      /// Return the SceneContainer for this scene.
      SceneContainer* getContainer() const { return mIsClient ? &gClientContainer : &gServerContainer; }

      /// Return the manager for the zones in this scene.
      /// @note Only client scenes have a zone manager as for the server, no zoning data is kept.
      const SceneZoneSpaceManager* getZoneManager() const { return mZoneManager; }
      SceneZoneSpaceManager* getZoneManager() { return mZoneManager; }

      /// @name SceneObject Management
      /// @{

      /// Add the given object to the scene.
      bool addObjectToScene( SceneObject* object );

      /// Remove the given object from the scene.
      void removeObjectFromScene( SceneObject* object );

      /// Let the scene manager know that the given object has changed its transform or
      /// sizing state.
      void notifyObjectDirty( SceneObject* object );

      /// @}

      /// @name Rendering
      /// @{

      /// Return the default RenderPassManager for the scene.
      RenderPassManager* getDefaultRenderPass() const;

      /// Set the default render pass for the scene.
      void setDefaultRenderPass( RenderPassManager* rpm ) { mDefaultRenderPass = rpm; }
      
      /// Render the scene with the default render pass.
      /// @note This uses the current GFX state (transforms, viewport, frustum) to initialize
      ///   the render state.
      void renderScene( ScenePassType passType, U32 objectMask = DEFAULT_RENDER_TYPEMASK );

      /// Render the scene with a custom rendering pass.
      void renderScene( SceneRenderState *state, U32 objectMask = DEFAULT_RENDER_TYPEMASK, SceneZoneSpace* baseObject = NULL, U32 baseZone = 0 );

      /// Render the scene with a custom rendering pass and no lighting set up.
      void renderSceneNoLights( SceneRenderState *state, U32 objectMask = DEFAULT_RENDER_TYPEMASK, SceneZoneSpace* baseObject = NULL, U32 baseZone = 0 );

      /// Returns the currently active scene state or NULL if no state is currently active.
      SceneRenderState* getCurrentRenderState() const { return mCurrentRenderState; }

      static RenderSignal& getPreRenderSignal() 
      { 
         static RenderSignal theSignal;
         return theSignal;
      }

      static RenderSignal& getPostRenderSignal() 
      { 
         static RenderSignal theSignal;
         return theSignal;
      }

      /// @}

      /// @name Lighting
      /// @{

      /// Finds the light manager by name and activates it.
      bool setLightManager( const char *lmName );

      /// Return the current global ambient light color.
      const ColorF& getAmbientLightColor() const { return mAmbientLightColor.getCurrentValue(); }

      /// Set the time it takes for a new ambient light color to take full effect.
      void setAmbientLightTransitionTime( SimTime time ) { mAmbientLightColor.setTransitionTime( time ); }

      /// Set the interpolation curve to use for blending from one global ambient light
      /// color to a different one.
      void setAmbientLightTransitionCurve( const EaseF& ease ) { mAmbientLightColor.setTransitionCurve( ease ); }

      /// @}

      /// @name Networking
      /// @{

      /// Set the scoping states of the objects in the scene.
      void scopeScene( CameraScopeQuery* query, NetConnection* netConnection );

      /// @}

      /// @name Fog/Visibility Management
      /// @{

      void setPostEffectFog( bool enable ) { mUsePostEffectFog = enable; }   
      bool usePostEffectFog() const { return mUsePostEffectFog; }

      /// Accessor for the FogData structure.
      const FogData& getFogData() { return mFogData; }

      /// Sets the FogData structure.
      void setFogData( const FogData &data ) { mFogData = data; }

      /// Accessor for the WaterFogData structure.
      const WaterFogData& getWaterFogData() { return mWaterFogData; }

      /// Sets the WaterFogData structure.
      void setWaterFogData( const WaterFogData &data ) { mWaterFogData = data; }

      /// Used by LevelInfo to set the default visible distance for
      /// rendering the scene.
      ///
      /// Note this should not be used to alter culling which is
      /// controlled by the active frustum when a SceneRenderState is created.
      ///
      /// @see SceneRenderState
      /// @see GameProcessCameraQuery
      /// @see LevelInfo
      void setVisibleDistance( F32 dist ) { mVisibleDistance = dist; }

      /// Returns the default visible distance for the scene.
      F32 getVisibleDistance() { return mVisibleDistance; }

      void setVisibleGhostDistance( F32 dist ) { mVisibleGhostDistance = dist; }
      F32  getVisibleGhostDistance() { return mVisibleGhostDistance;}

      /// Used by LevelInfo to set the default near clip plane 
      /// for rendering the scene.
      ///
      /// @see GameProcessCameraQuery
      /// @see LevelInfo
      void setNearClip( F32 nearClip ) { mNearClip = nearClip; }

      /// Returns the default near clip distance for the scene.
      F32 getNearClip() { return mNearClip; }

      /// @}

      /// @name dtr Display Target Resolution
      ///
      /// Some rendering must be targeted at a specific display resolution.
      /// This display resolution is distinct from the current RT's size
      /// (such as when rendering a reflection to a texture, for instance)
      /// so we store the size at which we're going to display the results of
      /// the current render.
      ///
      /// @{

      ///
      void setDisplayTargetResolution(const Point2I &size);
      const Point2I &getDisplayTargetResolution() const;

      /// @}

      // NonClipProjection is the projection matrix without oblique frustum clipping
      // applied to it (in reflections)
      void setNonClipProjection( const MatrixF &proj ) { mNonClipProj = proj; }
      const MatrixF& getNonClipProjection() const { return mNonClipProj; }
};

//-----------------------------------------------------------------------------

//TODO: these two need to go

/// The client-side scene graph.  Not used if the engine is running
/// as a dedicated server.
extern SceneManager* gClientSceneGraph;

/// The server-side scene graph.  Not used if the engine is running
/// as a pure client.
extern SceneManager* gServerSceneGraph;

#endif //_SCENEMANAGER_H_
