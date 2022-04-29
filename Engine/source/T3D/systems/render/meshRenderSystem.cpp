#include "T3D/systems/render/meshRenderSystem.h"
#include "gfx/gfxTransformSaver.h"
#include "lighting/lightQuery.h"

#include "renderInstance/renderPassManager.h"
#include "materials/materialManager.h"
#include "materials/baseMatInstance.h"

void MeshRenderSystem::render(SceneManager *sceneManager, SceneRenderState* state)
{
   if (sceneManager == nullptr || state == nullptr)
      return;

   Frustum viewFrustum = state->getCullingFrustum();
   MatrixF camTransform = state->getCameraTransform();

   U32 count = MeshRenderSystemInterface::all.size();
   for (U32 i = 0; i < count; i++)
   {
      //Server side items exist for data, but we don't actually render them
      if (!MeshRenderSystemInterface::all[i]->mIsClient)
         continue;

      //First, do frustum culling
      if (viewFrustum.isCulled(MeshRenderSystemInterface::all[i]->mBounds))
         continue;

      // Set the query box for the container query.  Never
      // make it larger than the frustum's AABB.  In the editor,
      // always query the full frustum as that gives objects
      // the opportunity to render editor visualizations even if
      // they are otherwise not in view.
      if (!state->getCullingFrustum().getBounds().isOverlapped(state->getRenderArea()))
      {
         // This handles fringe cases like flying backwards into a zone where you
         // end up pretty much standing on a zone border and looking directly into
         // its "walls".  In that case the traversal area will be behind the frustum
         // (remember that the camera isn't where visibility starts, it's the near
         // distance).

         continue;
      }

      //We can then sort our objects by range since we have it already, so we can do occlusion culling be rendering front-to-back

      //if we've made it this far, call down to the render function to actually display our stuff
      renderInterface(i, state);
   }
}

void MeshRenderSystem::renderInterface(U32 interfaceIndex, SceneRenderState* state)
{
   //Fetch
   MeshRenderSystemInterface* interface = MeshRenderSystemInterface::all[interfaceIndex];

   if (interface->mShapeInstance == nullptr)
      return;

   Point3F cameraOffset;
   interface->mTransform.getColumn(3, &cameraOffset);
   cameraOffset -= state->getDiffuseCameraPosition();
   F32 dist = cameraOffset.len();
   if (dist < 0.01f)
      dist = 0.01f;

   Point3F objScale = interface->mScale;
   F32 invScale = (1.0f / getMax(getMax(objScale.x, objScale.y), objScale.z));

   interface->mShapeInstance->setDetailFromDistance(state, dist * invScale);

   if (interface->mShapeInstance->getCurrentDetail() < 0)
      return;

   GFXTransformSaver saver;

   // Set up our TS render state.
   TSRenderState rdata;
   rdata.setSceneState(state);
   rdata.setFadeOverride(1.0f);
   rdata.setOriginSort(false);

   // We might have some forward lit materials
   // so pass down a query to gather lights.
   LightQuery query;
   query.init(interface->mSphere);
   rdata.setLightQuery(&query);

   MatrixF mat = interface->mTransform;

   mat.scale(objScale);
   GFX->setWorldMatrix(mat);

   interface->mShapeInstance->render(rdata);
}