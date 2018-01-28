#include "T3D/systems/render/meshRenderSystem.h"
#include "gfx/gfxTransformSaver.h"
#include "lighting/lightQuery.h"

#include "renderInstance/renderPassManager.h"
#include "materials/materialManager.h"
#include "materials/baseMatInstance.h"

Vector<MeshRenderSystem::BufferMaterials> MeshRenderSystem::mBufferMaterials(0);
Vector<MeshRenderSystem::BufferSet> MeshRenderSystem::mStaticBuffers(0);

void MeshRenderSystem::render(SceneManager *sceneManager, SceneRenderState* state)
{
   Frustum viewFrustum = state->getCullingFrustum();
   MatrixF camTransform = state->getCameraTransform();

   U32 count = MeshRenderSystemInterface::all.size();
   for (U32 i = 0; i < count; i++)
   {
      //Server side items exist for data, but we don't actually render them
      bool isClient = MeshRenderSystemInterface::all[i]->mIsClient;
      if (!MeshRenderSystemInterface::all[i]->mIsClient)
         continue;

      bool isStatic = MeshRenderSystemInterface::all[i]->mStatic;
      if (MeshRenderSystemInterface::all[i]->mStatic)
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

   //Static Batch rendering
   if ( /*!mMaterialInst ||*/ !state)
      return;

   BaseMatInstance *matInst = MATMGR->getWarningMatInstance();

   // Get a handy pointer to our RenderPassmanager
   RenderPassManager *renderPass = state->getRenderPass();

   for (U32 i = 0; i < mStaticBuffers.size(); i++)
   {
      for (U32 b = 0; b < mStaticBuffers[i].buffers.size(); b++)
      {
         if (mStaticBuffers[i].buffers[b].vertData.empty())
            continue;

         MeshRenderInst *ri = renderPass->allocInst<MeshRenderInst>();

         // Set our RenderInst as a standard mesh render
         ri->type = RenderPassManager::RIT_Mesh;

         //If our material has transparency set on this will redirect it to proper render bin
         if (matInst->getMaterial()->isTranslucent())
         {
            ri->type = RenderPassManager::RIT_Translucent;
            ri->translucentSort = true;
         }

         // Calculate our sorting point
         if (state)
         {
            // Calculate our sort point manually.
            const Box3F& rBox = Box3F(1000);// getRenderWorldBox();
            ri->sortDistSq = rBox.getSqDistanceToPoint(state->getCameraPosition());
         }
         else
            ri->sortDistSq = 0.0f;

         // Set up our transforms
         MatrixF objectToWorld = MatrixF::Identity;//getRenderTransform();
                                                   //objectToWorld.scale(getScale());

         ri->objectToWorld = renderPass->allocUniqueXform(objectToWorld);
         ri->worldToCamera = renderPass->allocSharedXform(RenderPassManager::View);
         ri->projection = renderPass->allocSharedXform(RenderPassManager::Projection);

         // If our material needs lights then fill the RIs 
         // light vector with the best lights.
         /*if (matInst->isForwardLit())
         {
         LightQuery query;
         query.init(getWorldSphere());
         query.getLights(ri->lights, 8);
         }*/

         // Make sure we have an up-to-date backbuffer in case
         // our Material would like to make use of it
         // NOTICE: SFXBB is removed and refraction is disabled!
         //ri->backBuffTex = GFX->getSfxBackBuffer();

         // Set our Material
         ri->matInst = matInst;

         // Set up our vertex buffer and primitive buffer
         ri->vertBuff = &mStaticBuffers[i].buffers[b].vertexBuffer;
         ri->primBuff = &mStaticBuffers[i].buffers[b].primitiveBuffer;

         ri->prim = renderPass->allocPrim();
         ri->prim->type = GFXTriangleList;
         ri->prim->minIndex = 0;
         ri->prim->startIndex = 0;
         ri->prim->numPrimitives = mStaticBuffers[i].buffers[b].primData.size();
         ri->prim->startVertex = 0;
         ri->prim->numVertices = mStaticBuffers[i].buffers[b].vertData.size();

         // We sort by the material then vertex buffer
         ri->defaultKey = matInst->getStateHint();
         ri->defaultKey2 = (uintptr_t)ri->vertBuff; // Not 64bit safe!

                                                    // Submit our RenderInst to the RenderPassManager
         state->getRenderPass()->addInst(ri);
      }
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

void MeshRenderSystem::rebuildBuffers()
{
   U32 BUFFER_SIZE = 65000;
   Vector<U32> tempIndices;
   tempIndices.reserve(4);

   Box3F newBounds = Box3F::Zero;

   mStaticBuffers.clear();

   for (U32 i = 0; i < MeshRenderSystemInterface::all.size(); i++)
   {
      if (!MeshRenderSystemInterface::all[i]->mIsEnabled)
         continue;

      if (!MeshRenderSystemInterface::all[i]->mIsClient || !MeshRenderSystemInterface::all[i]->mStatic)
         continue;

      //TODO: Properly re-implement StaticElements to container owner interfaces and buffer sets
      for (U32 j = 0; j < MeshRenderSystemInterface::all[i]->mGeometry.mPolyList.size(); j++)
      {
         const OptimizedPolyList::Poly& poly = MeshRenderSystemInterface::all[i]->mGeometry.mPolyList[j];

         if (poly.vertexCount < 3)
            continue;

         tempIndices.setSize(poly.vertexCount);
         dMemset(tempIndices.address(), 0, poly.vertexCount);

         if (poly.type == OptimizedPolyList::TriangleStrip ||
            poly.type == OptimizedPolyList::TriangleFan)
         {
            tempIndices[0] = 0;
            U32 idx = 1;

            for (U32 k = 1; k < poly.vertexCount; k += 2)
               tempIndices[idx++] = k;

            for (U32 k = ((poly.vertexCount - 1) & (~0x1)); k > 0; k -= 2)
               tempIndices[idx++] = k;
         }
         else if (poly.type == OptimizedPolyList::TriangleList)
         {
            for (U32 k = 0; k < poly.vertexCount; k++)
               tempIndices[k] = k;
         }
         else
            continue;

         //got our data, now insert it into the correct buffer!
         S32 bufferId = findBufferSetByMaterial(poly.material);

         if (bufferId == -1)
         {
            //add a new buffer set if we didn't get a match!
            BufferSet newSet;
            newSet.surfaceMaterialId = poly.material;

            mStaticBuffers.push_back(newSet);

            bufferId = mStaticBuffers.size() - 1;
         }

         //see if this would push us over our buffer size limit, if it is, make a new buffer for this set
         if (mStaticBuffers[bufferId].buffers.last().vertData.size() + 3 > BUFFER_SIZE
            || mStaticBuffers[bufferId].buffers.last().primData.size() + 1 > BUFFER_SIZE)
         {
            //yep, we'll overstep with this, so spool up a new buffer in this set
            BufferSet::Buffers newBuffer = BufferSet::Buffers();
            mStaticBuffers[bufferId].buffers.push_back(newBuffer);
         }

         const U32& firstIdx = MeshRenderSystemInterface::all[i]->mGeometry.mIndexList[poly.vertexStart];
         const OptimizedPolyList::VertIndex& firstVertIdx = MeshRenderSystemInterface::all[i]->mGeometry.mVertexList[firstIdx];

         //Vector<Point3F> geomPoints = MeshRenderSystemInterface::all[i]->mGeometry.mPoints;
         //Vector<Point3F> geomNormals = MeshRenderSystemInterface::all[i]->mGeometry.mNormals;
         //Vector<Point2F> geoUVs = MeshRenderSystemInterface::all[i]->mGeometry.mUV0s;

         for (U32 k = 1; k < poly.vertexCount - 1; k++)
         {
            const U32& secondIdx = MeshRenderSystemInterface::all[i]->mGeometry.mIndexList[poly.vertexStart + tempIndices[k]];
            const U32& thirdIdx = MeshRenderSystemInterface::all[i]->mGeometry.mIndexList[poly.vertexStart + tempIndices[k + 1]];

            const OptimizedPolyList::VertIndex& secondVertIdx = MeshRenderSystemInterface::all[i]->mGeometry.mVertexList[secondIdx];
            const OptimizedPolyList::VertIndex& thirdVertIdx = MeshRenderSystemInterface::all[i]->mGeometry.mVertexList[thirdIdx];

            Point3F points[3];
            points[0] = MeshRenderSystemInterface::all[i]->mGeometry.mPoints[firstVertIdx.vertIdx];
            points[1] = MeshRenderSystemInterface::all[i]->mGeometry.mPoints[secondVertIdx.vertIdx];
            points[2] = MeshRenderSystemInterface::all[i]->mGeometry.mPoints[thirdVertIdx.vertIdx];

            Point3F normals[3];
            normals[0] = MeshRenderSystemInterface::all[i]->mGeometry.mNormals[firstVertIdx.normalIdx];
            normals[1] = MeshRenderSystemInterface::all[i]->mGeometry.mNormals[secondVertIdx.normalIdx];
            normals[2] = MeshRenderSystemInterface::all[i]->mGeometry.mNormals[thirdVertIdx.normalIdx];

            Point3F tangents[3];
            tangents[0] = mCross(points[1] - points[0], normals[0]);
            tangents[1] = mCross(points[2] - points[1], normals[1]);
            tangents[2] = mCross(points[0] - points[2], normals[2]);

            Point2F uvs[3];
            uvs[0] = MeshRenderSystemInterface::all[i]->mGeometry.mUV0s[firstVertIdx.uv0Idx];
            uvs[1] = MeshRenderSystemInterface::all[i]->mGeometry.mUV0s[secondVertIdx.uv0Idx];
            uvs[2] = MeshRenderSystemInterface::all[i]->mGeometry.mUV0s[thirdVertIdx.uv0Idx];

            mStaticBuffers[bufferId].vertCount += 3;
            mStaticBuffers[bufferId].primCount += 1;

            for (U32 v = 0; v < 3; ++v)
            {
               //Build the vert and store it to the buffers!
               GFXVertexPNTT bufVert;
               bufVert.point = points[v];
               bufVert.normal = normals[v];
               bufVert.tangent = tangents[v];
               bufVert.texCoord = uvs[v];

               newBounds.extend(points[v]);

               mStaticBuffers[bufferId].buffers.last().vertData.push_back(bufVert);

               U32 vertPrimId = mStaticBuffers[bufferId].buffers.last().vertData.size() - 1;
               mStaticBuffers[bufferId].buffers.last().primData.push_back(vertPrimId);

               mStaticBuffers[bufferId].center += points[v];
            }
         }
      }
   }

   //Now, iterate through the organized data and turn them into renderable buffers
   for (U32 i = 0; i < mStaticBuffers.size(); i++)
   {
      for (U32 b = 0; b < mStaticBuffers[i].buffers.size(); b++)
      {
         BufferSet::Buffers& buffers = mStaticBuffers[i].buffers[b];

         //if there's no data to be had in this buffer, just skip it
         if (buffers.vertData.empty())
            continue;

         buffers.vertexBuffer.set(GFX, buffers.vertData.size(), GFXBufferTypeStatic);
         GFXVertexPNTT *pVert = buffers.vertexBuffer.lock();

         for (U32 v = 0; v < buffers.vertData.size(); v++)
         {
            pVert->normal = buffers.vertData[v].normal;
            pVert->tangent = buffers.vertData[v].tangent;
            //pVert->color = buffers.vertData[v].color;
            pVert->point = buffers.vertData[v].point;
            pVert->texCoord = buffers.vertData[v].texCoord;

            pVert++;
         }

         buffers.vertexBuffer.unlock();

         // Allocate PB
         buffers.primitiveBuffer.set(GFX, buffers.primData.size(), buffers.primData.size() / 3, GFXBufferTypeStatic);

         U16 *pIndex;
         buffers.primitiveBuffer.lock(&pIndex);

         for (U16 i = 0; i < buffers.primData.size(); i++)
         {
            *pIndex = i;
            pIndex++;
         }

         buffers.primitiveBuffer.unlock();
      }

      mStaticBuffers[i].center /= mStaticBuffers[i].vertCount;
   }

   //mObjBox = newBounds;
   //resetWorldBox();
}

U32 MeshRenderSystem::findBufferSetByMaterial(U32 matId)
{
   for (U32 i = 0; i < mStaticBuffers.size(); i++)
   {
      if (mStaticBuffers[i].surfaceMaterialId == matId)
         return i;
   }

   return -1;
}