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

#ifndef _SCENERENDERSTATE_H_
#define _SCENERENDERSTATE_H_

#ifndef _COLOR_H_
#include "core/color.h"
#endif

#ifndef _SCENEMANAGER_H_
#include "scene/sceneManager.h"
#endif

#ifndef _SCENECULLINGSTATE_H_
#include "scene/culling/sceneCullingState.h"
#endif

#ifndef _UTIL_DELEGATE_H_
#include "core/util/delegate.h"
#endif


class SceneObject;
class RenderPassManager;
class BaseMatInstance;



/// The SceneRenderState describes the state of the scene being rendered.
///
/// It keeps track of the information that objects need to render properly with regard to
/// the camera position, any fog information, viewing frustum, the global environment
/// map for reflections, viewable distance, etc.
///
/// It also owns the current culling state.
class SceneRenderState
{
   public:

      /// The delegate used for material overrides.
      /// @see getOverrideMaterial
      typedef Delegate< BaseMatInstance*( BaseMatInstance* ) > MatDelegate;

   protected:

      /// SceneManager being rendered in this state.
      SceneManager* mSceneManager;

      /// The type of scene render pass we're doing.
      ScenePassType mScenePassType;

      /// The render style being performed
      SceneRenderStyle mSceneRenderStyle;

      /// When doing stereo rendering, the current field that is being rendered
      S32 mRenderField;

      /// The render pass which we are setting up with this scene state.
      RenderPassManager* mRenderPass;

      /// Culling state of the scene.
      SceneCullingState mCullingState;

      /// The optional material override delegate.
      MatDelegate mMatDelegate;

      ///
      MatrixF mDiffuseCameraTransform;

      /// The world to screen space scalar used for LOD calculations.
      Point2F mWorldToScreenScale;

      /// The AABB that encloses the space in the scene that we render.
      Box3F mRenderArea;

      /// The camera vector normalized to 1 / far dist.
      Point3F mVectorEye;

      /// Global ambient light color.
      ColorF mAmbientLightColor;

      /// Forces bin based post effects to be disabled
      /// during rendering with this scene state.
      bool mUsePostEffects;

      /// Disables AdvancedLighting bin draws during rendering with this scene state.
      bool mDisableAdvancedLightingBins;

      /// If true (default) lightmapped meshes should be rendered.
      bool mRenderLightmappedMeshes;

      /// If true (default) non-lightmapped meshes should be rendered.
      bool mRenderNonLightmappedMeshes;

   public:

      /// Construct a new SceneRenderState.
      ///
      /// @param sceneManager SceneManager rendered in this SceneRenderState.
      /// @param passType Type of rendering pass that the SceneRenderState is for.
      /// @param view The view that is being rendered
      /// @param renderPass The render pass which is being set up by this SceneRenderState.  If NULL,
      ///   then Scene::getDefaultRenderPass() is used.
      /// @param usePostEffect Whether PostFX are enabled in the rendering pass.
      SceneRenderState( SceneManager* sceneManager,
                        ScenePassType passType,
                        const SceneCameraState& view = SceneCameraState::fromGFX(),
                        RenderPassManager* renderPass = NULL,
                        bool usePostEffects = true );

      ~SceneRenderState();

      /// Return the SceneManager that is being rendered in this SceneRenderState.
      SceneManager* getSceneManager() const { return mSceneManager; }

      /// If true then bin based post effects are disabled 
      /// during rendering with this scene state.
      bool usePostEffects() const { return mUsePostEffects; }
      void usePostEffects( bool value ) { mUsePostEffects = value; }

      /// @name Culling
      /// @{

      /// Return the culling state for the scene.
      const SceneCullingState& getCullingState() const { return mCullingState; }
      SceneCullingState& getCullingState() { return mCullingState; }

      /// Returns the root frustum.
      const Frustum& getFrustum() const { return getCullingState().getFrustum(); }

      /// @}

      /// @name Rendering
      /// @{

      /// Get the AABB around the scene portion that we render.
      const Box3F& getRenderArea() const { return mRenderArea; }

      /// Set the AABB of the space that should be rendered.
      void setRenderArea( const Box3F& area ) { mRenderArea = area; }

      /// Batch the given objects to the render pass manager and then
      /// render the batched instances.
      ///
      /// @param objects List of objects.
      /// @param numObjects Number of objects in @a objects.
      void renderObjects( SceneObject** objects, U32 numObjects );

      /// @}

      /// @name Lighting
      /// @{

      /// Return the ambient light color to use for rendering the scene.
      ///
      /// At the moment, we only support a single global ambient color with which
      /// all objects in the scene are rendered.  This is because when using
      /// Advanced Lighting, we are not resolving light contribution on a per-surface
      /// or per-object basis but rather do it globally by gathering light
      /// contribution to the whole scene and since the ambient factor is decided
      /// by the sun/vector light, it simply becomes a base light level onto which
      /// shadowing/lighting is blended based on the shadow maps of the sun/vector
      /// light.
      ///
      /// @return The ambient light color for rendering.
      ColorF getAmbientLightColor() const { return mAmbientLightColor; }

      /// Set the global ambient light color to render with.
      void setAmbientLightColor( const ColorF& color ) { mAmbientLightColor = color; }

      /// If true then Advanced Lighting bin draws are disabled during rendering with
      /// this scene state.
      bool disableAdvancedLightingBins() const { return mDisableAdvancedLightingBins; }
      void disableAdvancedLightingBins(bool enabled) { mDisableAdvancedLightingBins = enabled; }

      bool renderLightmappedMeshes() const { return mRenderLightmappedMeshes; }
      void renderLightmappedMeshes( bool enabled ) { mRenderLightmappedMeshes = enabled; }

      bool renderNonLightmappedMeshes() const { return mRenderNonLightmappedMeshes; }
      void renderNonLightmappedMeshes( bool enabled ) { mRenderNonLightmappedMeshes = enabled; }

      /// @}

      /// @name Passes
      /// @{

      /// Return the RenderPassManager that manages rendering objects batched
      /// for this SceneRenderState.
      RenderPassManager* getRenderPass() const { return mRenderPass; }

      /// Returns the type of scene rendering pass that we're doing.
      ScenePassType getScenePassType() const { return mScenePassType; }

      /// Returns true if this is a diffuse scene rendering pass.
      bool isDiffusePass() const { return mScenePassType == SPT_Diffuse; }

      /// Returns true if this is a reflection scene rendering pass.
      bool isReflectPass() const { return mScenePassType == SPT_Reflect; }

      /// Returns true if this is a shadow scene rendering pass.
      bool isShadowPass() const { return mScenePassType == SPT_Shadow; }

      /// Returns true if this is not one of the other rendering passes.
      bool isOtherPass() const { return mScenePassType >= SPT_Other; }

      /// @}

      /// @name Render Style
      /// @{

      /// Get the rendering style used for the scene
      SceneRenderStyle getSceneRenderStyle() const { return mSceneRenderStyle; }

      /// Set the rendering style used for the scene
      void setSceneRenderStyle(SceneRenderStyle style) { mSceneRenderStyle = style; }

      /// Get the stereo field being rendered
      S32 getSceneRenderField() const { return mRenderField; }

      /// Set the stereo field being rendered
      void setSceneRenderField(S32 field) { mRenderField = field; }

      /// @}

      /// @name Transforms, projections, and viewports.
      /// @{

      /// Return the screen-space viewport rectangle.
      const RectI& getViewport() const { return getCullingState().getCameraState().getViewport(); }

      /// Return the world->view transform matrix.
      const MatrixF& getWorldViewMatrix() const;

      /// Return the project transform matrix.
      const MatrixF& getProjectionMatrix() const;

      /// Returns the actual camera position.
      /// @see getDiffuseCameraPosition
      const Point3F& getCameraPosition() const { return getCullingState().getCameraState().getViewPosition(); }

      /// Returns the camera transform (view->world) this SceneRenderState is using.
      const MatrixF& getCameraTransform() const { return getCullingState().getCameraState().getViewWorldMatrix(); }

      /// Returns the minimum distance something must be from the camera to not be culled.
      F32 getNearPlane() const { return getFrustum().getNearDist();   }

      /// Returns the maximum distance something can be from the camera to not be culled.
      F32 getFarPlane() const { return getFrustum().getFarDist();    }

      /// Returns the camera vector normalized to 1 / far distance.
      const Point3F& getVectorEye() const { return mVectorEye; }

      /// Returns the possibly overloaded world to screen scale.
      /// @see projectRadius
      const Point2F& getWorldToScreenScale() const { return mWorldToScreenScale; }

      /// Set a new world to screen scale to overload
      /// future screen metrics operations.
      void setWorldToScreenScale( const Point2F& scale ) { mWorldToScreenScale = scale; }

      /// Returns the pixel size of the radius projected to the screen at a desired distance.
      /// 
      /// Internally this uses the stored world to screen scale and viewport extents.  This
      /// allows the projection to be overloaded in special cases like when rendering shadows
      /// or reflections.
      ///
      /// @see getWorldToScreenScale
      /// @see getViewportExtent
      F32 projectRadius( F32 dist, F32 radius ) const
      {
         // We fixup any negative or zero distance 
         // so we don't get a divide by zero.
         dist = dist > 0.0f ? dist : 0.001f;
         return ( radius / dist ) * mWorldToScreenScale.y;
      }

      /// Returns the camera position used during the diffuse rendering pass which may be different
      /// from the actual camera position.
      ///
      /// This is useful when doing level of detail calculations that need to be relative to the
      /// diffuse pass.
      ///
      /// @see getCameraPosition
      Point3F getDiffuseCameraPosition() const { return mDiffuseCameraTransform.getPosition(); }
      const MatrixF& getDiffuseCameraTransform() const { return mDiffuseCameraTransform; }

      /// Set a new diffuse camera transform.
      /// @see getDiffuseCameraTransform
      void setDiffuseCameraTransform( const MatrixF &mat ) { mDiffuseCameraTransform = mat; }

      /// @}

      /// @name Material Overrides
      /// @{

      /// When performing a special render pass like shadows this
      /// returns a specialized override material.  It can return 
      /// NULL if the override wants to disable rendering.  If 
      /// there is no override in place then the input material is
      /// returned unaltered.
      BaseMatInstance* getOverrideMaterial( BaseMatInstance* matInst ) const
      {
         if ( !matInst || mMatDelegate.empty() )
            return matInst;

         return mMatDelegate( matInst );
      }

      /// Returns the optional material override delegate which is
      /// used during some special render passes.
      /// @see getOverrideMaterial
      MatDelegate& getMaterialDelegate() { return mMatDelegate; }
      const MatDelegate& getMaterialDelegate() const { return mMatDelegate; }

      /// @}
};

#endif // _SCENERENDERSTATE_H_
