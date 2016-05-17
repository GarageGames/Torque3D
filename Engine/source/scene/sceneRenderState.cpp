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
#include "scene/sceneRenderState.h"

#include "renderInstance/renderPassManager.h"
#include "math/util/matrixSet.h"

#ifdef TORQUE_EXPERIMENTAL_EC
#include "T3D/components/render/renderComponentInterface.h"
#include "T3D/components/component.h"
#endif

//-----------------------------------------------------------------------------

SceneRenderState::SceneRenderState( SceneManager* sceneManager,
                                    ScenePassType passType,
                                    const SceneCameraState& view,
                                    RenderPassManager* renderPass /* = NULL */,
                                    bool usePostEffects /* = true */ )
   :  mSceneManager( sceneManager ),
      mCullingState( sceneManager, view ),
      mRenderPass( renderPass ? renderPass : sceneManager->getDefaultRenderPass() ),
      mScenePassType( passType ),
      mRenderNonLightmappedMeshes( true ),
      mRenderLightmappedMeshes( true ),
      mUsePostEffects( usePostEffects ),
      mDisableAdvancedLightingBins( false ),
      mRenderArea( view.getFrustum().getBounds() ),
      mAmbientLightColor( sceneManager->getAmbientLightColor() ),
      mSceneRenderStyle( SRS_Standard )
{
   // Setup the default parameters for the screen metrics methods.
   mDiffuseCameraTransform = view.getHeadWorldViewMatrix();
   mDiffuseCameraTransform.inverse();

   // The vector eye is the camera vector with its 
   // length normalized to 1 / zFar.
   getCameraTransform().getColumn( 1, &mVectorEye );
   mVectorEye.normalize( 1.0f / getFarPlane() );

   // TODO: What about ortho modes?  Is near plane ok
   // or do i need to remove it... maybe ortho has a near
   // plane of 1 and it just works out?

   const Frustum& frustum = view.getFrustum();
   const RectI& viewport = view.getViewport();

   mWorldToScreenScale.set(   ( frustum.getNearDist() * viewport.extent.x ) / ( frustum.getNearRight() - frustum.getNearLeft() ),
                              ( frustum.getNearDist() * viewport.extent.y ) / ( frustum.getNearTop() - frustum.getNearBottom() ) );

   // Assign shared matrix data to the render pass.

   mRenderPass->assignSharedXform( RenderPassManager::View, view.getWorldViewMatrix() );
   mRenderPass->assignSharedXform( RenderPassManager::Projection, view.getProjectionMatrix() );
}

//-----------------------------------------------------------------------------

SceneRenderState::~SceneRenderState()
{
}

//-----------------------------------------------------------------------------

const MatrixF& SceneRenderState::getWorldViewMatrix() const
{
   return getRenderPass()->getMatrixSet().getWorldToCamera();
}

//-----------------------------------------------------------------------------

const MatrixF& SceneRenderState::getProjectionMatrix() const
{
   return getRenderPass()->getMatrixSet().getCameraToScreen();
}

//-----------------------------------------------------------------------------

void SceneRenderState::renderObjects( SceneObject** objects, U32 numObjects )
{
   // Let the objects batch their stuff.

   PROFILE_START( SceneRenderState_prepRenderImages );
   for( U32 i = 0; i < numObjects; ++ i )
   {
      SceneObject* object = objects[ i ];
      object->prepRenderImage( this );
   }

#ifdef TORQUE_EXPERIMENTAL_EC
   U32 interfaceCount = RenderComponentInterface::all.size();
   for (U32 i = 0; i < RenderComponentInterface::all.size(); i++)
   {
      Component* comp = dynamic_cast<Component*>(RenderComponentInterface::all[i]);

      if (comp->isClientObject() && comp->isActive())
      {
         RenderComponentInterface::all[i]->prepRenderImage(this);
      }
   }
#endif

   PROFILE_END();

   // Render what the objects have batched.

   getRenderPass()->renderPass( this );
}
