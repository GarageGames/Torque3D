
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
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
//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "afx/arcaneFX.h"

#include "scene/sceneRenderState.h"
#include "T3D/fx/particleEmitter.h"
#include "renderInstance/renderPassManager.h"
#include "lighting/lightInfo.h"
#include "lighting/lightManager.h"

#include "afx/util/afxParticlePool.h"

void afxParticlePool::prepRenderImage(SceneRenderState* state)
{ 
  const LightInfo *sunlight = LIGHTMGR->getSpecialLight( LightManager::slSunLightType );
  pool_prepBatchRender(state->getRenderPass(), state->getCameraPosition(), sunlight->getAmbient());
};

void afxParticlePool::pool_prepBatchRender(RenderPassManager *renderManager, const Point3F &camPos, const LinearColorF &ambientColor)
{
  if (emitters.empty()) 
    return;

  switch (mDataBlock->pool_type)
  {
  case afxParticlePoolData::POOL_TWOPASS :
    pool_renderObject_TwoPass(renderManager, camPos, ambientColor);
    break;
  case afxParticlePoolData::POOL_NORMAL :
  default:
    pool_renderObject_Normal(renderManager, camPos, ambientColor);
  }
}

void afxParticlePool::pool_renderObject_Normal(RenderPassManager *renderManager, const Point3F &camPos, const LinearColorF &ambientColor)
{
  S32 n_parts = 0;
  for (S32 i = 0; i < emitters.size(); i++)
    n_parts += emitters[i]->n_parts;

  if (n_parts == 0)
    return;

  ParticleEmitterData* main_emitter_data = emitters[0]->mDataBlock;
  
  main_emitter_data->allocPrimBuffer(n_parts);

  orderedVector.clear();

  MatrixF modelview = GFX->getWorldMatrix();
  Point3F viewvec; modelview.getRow(1, &viewvec);

  for (U32 i=0; i < emitters.size(); i++)
  {
    // add each particle and a distance based sort key to orderedVector
    for (Particle* pp = emitters[i]->part_list_head.next; pp != NULL; pp = pp->next)
    {
      orderedVector.increment();
      orderedVector.last().p = pp;
      orderedVector.last().k = mDot(pp->pos, viewvec);
      orderedVector.last().emitter = emitters[i];
    }
  }

  // qsort the list into far to near ordering
  dQsort(orderedVector.address(), orderedVector.size(), sizeof(SortParticlePool), cmpSortParticlePool);

  static Vector<GFXVertexPCT> tempBuff(2048);
  tempBuff.reserve(n_parts*4 + 64); // make sure tempBuff is big enough
  GFXVertexPCT *buffPtr = tempBuff.address(); // use direct pointer (faster)

  Point3F basePoints[4];
  basePoints[0] = Point3F(-1.0, 0.0, -1.0);
  basePoints[1] = Point3F( 1.0, 0.0, -1.0);
  basePoints[2] = Point3F( 1.0, 0.0,  1.0);
  basePoints[3] = Point3F(-1.0, 0.0,  1.0);

  MatrixF camView = GFX->getWorldMatrix();
  camView.transpose();  // inverse - this gets the particles facing camera

  for (U32 i = 0; i < orderedVector.size(); i++)
  {
    Particle* particle = orderedVector[i].p;
    ParticleEmitter* emitter = orderedVector[i].emitter;

    if (emitter->mDataBlock->orientParticles)
      emitter->setupOriented(particle, camPos, ambientColor, buffPtr);
    else
      emitter->setupBillboard(particle, basePoints, camView, ambientColor, buffPtr);
    buffPtr+=4;
  }

  // create new VB if emitter size grows
  if( !mVertBuff || n_parts > mCurBuffSize )
  {
    mCurBuffSize = n_parts;
    mVertBuff.set(GFX, n_parts*4, GFXBufferTypeDynamic);
  }
  // lock and copy tempBuff to video RAM
  GFXVertexPCT *verts = mVertBuff.lock();
  dMemcpy( verts, tempBuff.address(), n_parts * 4 * sizeof(GFXVertexPCT) );
  mVertBuff.unlock();

  //MeshRenderInst *ri = gRenderInstManager->allocInst<MeshRenderInst>();
  ParticleRenderInst *ri = renderManager->allocInst<ParticleRenderInst>();
  ri->vertBuff = &mVertBuff;
  ri->primBuff = &main_emitter_data->primBuff;
  ri->translucentSort = true;
  ri->type = RenderPassManager::RIT_Particle;
  ri->sortDistSq = getWorldBox().getSqDistanceToPoint( camPos );

  ri->defaultKey = (-sort_priority*100);

  ri->modelViewProj = renderManager->allocUniqueXform(  GFX->getProjectionMatrix() * 
                                                        GFX->getViewMatrix() * 
                                                        GFX->getWorldMatrix() );

  ri->count = n_parts;

  ri->blendStyle = main_emitter_data->blendStyle;

  // use first particle's texture unless there is an emitter texture to override it
  if (main_emitter_data->textureHandle)
    ri->diffuseTex = &*(main_emitter_data->textureHandle);
  else
    ri->diffuseTex = &*(main_emitter_data->particleDataBlocks[0]->textureHandle);

  ri->softnessDistance = main_emitter_data->softnessDistance;

  // Sort by texture too.
  //ri->defaultKey = ri->diffuseTex ? (U32)ri->diffuseTex : (U32)ri->vertBuff;

  renderManager->addInst( ri );
}

void afxParticlePool::pool_renderObject_TwoPass(RenderPassManager *renderManager, const Point3F &camPos, const LinearColorF &ambientColor)
{
  S32 n_parts = 0;
  for (S32 i = 0; i < emitters.size(); i++)
    n_parts += emitters[i]->n_parts;

  if (n_parts == 0)
    return;

  ParticleEmitterData* main_emitter_data = emitters[0]->mDataBlock;
  
  main_emitter_data->allocPrimBuffer(n_parts);

  orderedVector.clear();

  F32 min_d=0.0f, max_d=0.0f;

  for (U32 i=0; i < emitters.size(); i++)
  {
    if (!emitters[i]->mDataBlock->pool_depth_fade || !emitters[i]->mDataBlock->pool_radial_fade)
      continue;

    // add particles to orderedVector and calc distance and min/max distance
    for (Particle* pp = emitters[i]->part_list_head.next; pp != NULL; pp = pp->next)
    {
      F32 dist = (pp->pos-camPos).len();
      if (dist > max_d)
        max_d = dist;
      else if (dist < min_d)
        min_d = dist;

      orderedVector.increment();
      orderedVector.last().p = pp;
      orderedVector.last().k = dist;
      orderedVector.last().emitter = emitters[i];
    }
  }

  // Add remaining emitters particles to the orderedVector that do not participate in the 
  // above depth computations:
  for (U32 i=0; i < emitters.size(); i++)
  {
    if (emitters[i]->mDataBlock->pool_depth_fade || emitters[i]->mDataBlock->pool_radial_fade)
      continue;

    for (Particle* pp = emitters[i]->part_list_head.next; pp != NULL; pp = pp->next)
    {
      orderedVector.increment();
      orderedVector.last().p = pp;
      orderedVector.last().k = 0;  // no need to compute depth here
      orderedVector.last().emitter = emitters[i];
    }
  }

  static Vector<GFXVertexPCT> tempBuff(2048);
  tempBuff.reserve(n_parts*4 + 64); // make sure tempBuff is big enough
  GFXVertexPCT *buffPtr = tempBuff.address(); // use direct pointer (faster)

  Point3F basePoints[4];
  basePoints[0] = Point3F(-1.0, 0.0, -1.0);
  basePoints[1] = Point3F( 1.0, 0.0, -1.0);
  basePoints[2] = Point3F( 1.0, 0.0,  1.0);
  basePoints[3] = Point3F(-1.0, 0.0,  1.0);

  MatrixF camView = GFX->getWorldMatrix();
  camView.transpose();  // inverse - this gets the particles facing camera

  //~~~~~~~~~~~~~~~~~~~~//

  Point3F bbox_center; mObjBox.getCenter(&bbox_center);
  F32 d_range = max_d - min_d;
  bool d_safe = (d_range>0.0001);
  F32 d_half = min_d + (d_range*0.5f);

  //~~~~~~~~~~~~~~~~~~~~//

  for (U32 i = 0; i < orderedVector.size(); i++)
  {
    Particle* particle = orderedVector[i].p;
    ParticleEmitter* emitter = orderedVector[i].emitter;

    LinearColorF color_save = particle->color;
    particle->color.set(mDataBlock->base_color.red, mDataBlock->base_color.green, mDataBlock->base_color.blue, mDataBlock->base_color.alpha*particle->color.alpha);
    emitter->setupBillboard(particle, basePoints, camView, ambientColor, buffPtr);
    particle->color = color_save;

    buffPtr+=4;
  }

  // create new VB if emitter size grows
  if( !mVertBuff || n_parts > mCurBuffSize )
  {
    mCurBuffSize = n_parts;
    mVertBuff.set(GFX, n_parts*4, GFXBufferTypeDynamic);
  }
  // lock and copy tempBuff to video RAM
  GFXVertexPCT *verts = mVertBuff.lock();
  dMemcpy( verts, tempBuff.address(), n_parts * 4 * sizeof(GFXVertexPCT) );
  mVertBuff.unlock();

  ParticleRenderInst *ri = renderManager->allocInst<ParticleRenderInst>();
  ri->vertBuff = &mVertBuff;
  ri->primBuff = &main_emitter_data->primBuff;
  ri->translucentSort = true;
  ri->type = RenderPassManager::RIT_Particle;
  ri->sortDistSq = getWorldBox().getSqDistanceToPoint( camPos );

  ri->defaultKey = (-sort_priority*100);

  ri->modelViewProj = renderManager->allocUniqueXform(  GFX->getProjectionMatrix() * 
                                                        GFX->getViewMatrix() * 
                                                        GFX->getWorldMatrix() );

  ri->count = n_parts;

  ri->blendStyle = ParticleRenderInst::BlendNormal;

  // use first particle's texture unless there is an emitter texture to override it
  //if (main_emitter_data->textureHandle)
  //  ri->diffuseTex = &*(main_emitter_data->textureHandle);
  //else
    ri->diffuseTex = &*(main_emitter_data->particleDataBlocks[0]->textureExtHandle);

  F32 save_sort_dist = ri->sortDistSq;

  ri->softnessDistance = main_emitter_data->softnessDistance;

  renderManager->addInst( ri );

  //~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//
  //~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//
  // 2nd-pass

  buffPtr = tempBuff.address();

  bbox_center.z = mObjBox.minExtents.z;
  F32 max_radius = (max_d-min_d)*0.5f;

  // gather fade settings
  bool do_mixed_fades = false;
  bool do_radial_fades = (emitters[0]->mDataBlock->pool_radial_fade && (max_radius>0.0001f));
  bool do_depth_fades = (emitters[0]->mDataBlock->pool_depth_fade && d_safe);
  for (U32 i = 1; i < emitters.size(); i++)
  {
    if ( (do_radial_fades != (emitters[i]->mDataBlock->pool_radial_fade && (max_radius>0.0001f))) ||
      (do_depth_fades != (emitters[i]->mDataBlock->pool_depth_fade && d_safe)))
    {
      do_mixed_fades = true;
      break;
    }
  }

  if (do_mixed_fades)
  {
    for (U32 i = 0; i < orderedVector.size(); i++)
    {
      Particle* particle = orderedVector[i].p;
      ParticleEmitter* emitter = orderedVector[i].emitter;

      F32 bf = 1.0;  // blend factor

      // blend factor due to radius
      if (emitter->mDataBlock->pool_radial_fade && (max_radius>0.0001f))
      {
        F32 p_radius = (particle->pos-bbox_center).len();
        F32 bf_radius = p_radius/max_radius;
        if (bf_radius>1.0f) bf_radius = 1.0f;
        bf *= bf_radius*bf_radius;  // quadratic for faster falloff
      }

      // blend factor, depth based
      if (emitter->mDataBlock->pool_depth_fade && d_safe && (orderedVector[i].k > d_half))
      {
        F32 bf_depth = ((max_d-orderedVector[i].k) / (d_range*0.5f));
        bf *= bf_depth;
      }

      // overall blend factor weight
      bf *= mDataBlock->blend_weight;

      LinearColorF color_save = particle->color;
      particle->color = particle->color*bf;
      emitter->setupBillboard(particle, basePoints, camView, ambientColor, buffPtr);
      particle->color = color_save;

      buffPtr+=4;
    }
  }
  else if (do_radial_fades && do_depth_fades)
  {
    for (U32 i = 0; i < orderedVector.size(); i++)
    {
      Particle* particle = orderedVector[i].p;
      ParticleEmitter* emitter = orderedVector[i].emitter;

      F32 bf = 1.0;  // blend factor

      // blend factor due to radius
      F32 p_radius = (particle->pos-bbox_center).len();
      F32 bf_radius = p_radius/max_radius;
      if (bf_radius>1.0f) bf_radius = 1.0f;
      bf *= bf_radius*bf_radius;  // quadratic for faster falloff

      // blend factor, depth based
      if (orderedVector[i].k > d_half)
      {
        F32 bf_depth = ((max_d-orderedVector[i].k) / (d_range*0.5f));
        bf *= bf_depth;
      }

      // overall blend factor weight
      bf *= mDataBlock->blend_weight;

      LinearColorF color_save = particle->color;
      particle->color = particle->color*bf;
      emitter->setupBillboard(particle, basePoints, camView, ambientColor, buffPtr);
      particle->color = color_save;

      buffPtr+=4;
    }
  }
  else if (do_radial_fades) // && !do_depth_fades
  {
    for (U32 i = 0; i < orderedVector.size(); i++)
    {
      Particle* particle = orderedVector[i].p;
      ParticleEmitter* emitter = orderedVector[i].emitter;

      F32 bf = 1.0;  // blend factor

      // blend factor due to radius
      F32 p_radius = (particle->pos-bbox_center).len();
      F32 bf_radius = p_radius/max_radius;
      if (bf_radius>1.0f) bf_radius = 1.0f;
      bf *= bf_radius*bf_radius;  // quadratic for faster falloff

      // overall blend factor weight
      bf *= mDataBlock->blend_weight;

      LinearColorF color_save = particle->color;
      particle->color = particle->color*bf;
      emitter->setupBillboard(particle, basePoints, camView, ambientColor, buffPtr);
      particle->color = color_save;

      buffPtr+=4;
    }
  }
  else if (do_depth_fades) // && !do_radial_fades
  {
    for (U32 i = 0; i < orderedVector.size(); i++)
    {
      Particle* particle = orderedVector[i].p;
      ParticleEmitter* emitter = orderedVector[i].emitter;

      F32 bf = 1.0;  // blend factor

      // blend factor, depth based
      if (orderedVector[i].k > d_half)
      {
        F32 bf_depth = ((max_d-orderedVector[i].k) / (d_range*0.5f));
        bf *= bf_depth;
      }

      // overall blend factor weight
      bf *= mDataBlock->blend_weight;

      LinearColorF color_save = particle->color;
      particle->color = particle->color*bf;
      emitter->setupBillboard(particle, basePoints, camView, ambientColor, buffPtr);
      particle->color = color_save;

      buffPtr+=4;
    }
  }
  else // (no fades)
  {
    for (U32 i = 0; i < orderedVector.size(); i++)
    {
      Particle* particle = orderedVector[i].p;
      ParticleEmitter* emitter = orderedVector[i].emitter;

      F32 bf = mDataBlock->blend_weight;  // blend factor

      LinearColorF color_save = particle->color;
      particle->color = particle->color*bf;
      emitter->setupBillboard(particle, basePoints, camView, ambientColor, buffPtr);
      particle->color = color_save;

      buffPtr+=4;
    }
  }

  // create new VB if emitter size grows
  if( !mVertBuff2 || n_parts > mCurBuffSize2 )
  {
    mCurBuffSize2 = n_parts;
    mVertBuff2.set(GFX, n_parts*4, GFXBufferTypeDynamic);
  }

  // lock and copy tempBuff to video RAM
  verts = mVertBuff2.lock();
  dMemcpy( verts, tempBuff.address(), n_parts * 4 * sizeof(GFXVertexPCT) );
  mVertBuff2.unlock();

  ri = renderManager->allocInst<ParticleRenderInst>();
  ri->vertBuff = &mVertBuff2;
  ri->primBuff = &main_emitter_data->primBuff;
  ri->translucentSort = true;
  ri->type = RenderPassManager::RIT_Particle;
  ri->sortDistSq = save_sort_dist;

  ri->defaultKey = (-sort_priority*100) + 1;

  ri->modelViewProj = renderManager->allocUniqueXform(  GFX->getProjectionMatrix() * 
                                                        GFX->getViewMatrix() * 
                                                        GFX->getWorldMatrix() );

  ri->count = n_parts;

  ri->blendStyle = ParticleRenderInst::BlendAdditive;

  // use first particle's texture unless there is an emitter texture to override it
  if (main_emitter_data->textureHandle)
    ri->diffuseTex = &*(main_emitter_data->textureHandle);
  else
    ri->diffuseTex = &*(main_emitter_data->particleDataBlocks[0]->textureHandle);

  ri->softnessDistance = main_emitter_data->softnessDistance;

  renderManager->addInst( ri );
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
