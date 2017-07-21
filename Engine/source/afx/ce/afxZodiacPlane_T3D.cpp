
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

#include "gfx/gfxTransformSaver.h"
#include "gfx/primBuilder.h"

#include "afx/afxChoreographer.h"
#include "afx/ce/afxZodiacPlane.h"

void afxZodiacPlane::prepRenderImage(SceneRenderState* state)
{
  if (!is_visible)
    return;

  ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
  ri->renderDelegate.bind(this, &afxZodiacPlane::_renderZodiacPlane);
  ri->type = RenderPassManager::RIT_ObjectTranslucent;
  ri->translucentSort = true;
  ri->defaultKey = (U32)(dsize_t)mDataBlock;

  if (false)
  {
    ri->sortDistSq = getWorldBox().getSqDistanceToPoint( state->getCameraPosition() );      
  }
  else // (sort by radius distance)
  {
    Point3F xyz_scale = getScale();
    F32 uni_scalar = getMax(xyz_scale.x, xyz_scale.y);
    uni_scalar = getMax(uni_scalar, xyz_scale.z);
    Point3F uni_scale(uni_scalar, uni_scalar, uni_scalar);

    Point3F local_cam_pos = state->getCameraPosition();
    getRenderWorldTransform().mulP(local_cam_pos);
    local_cam_pos.convolveInverse(uni_scale);

    switch (mDataBlock->face_dir)
    {
    case afxZodiacPlaneData::FACES_UP:
    case afxZodiacPlaneData::FACES_DOWN:
      local_cam_pos.z = 0;
      break;  
    case afxZodiacPlaneData::FACES_FORWARD:
    case afxZodiacPlaneData::FACES_BACK:
      local_cam_pos.y = 0;
      break;  
    case afxZodiacPlaneData::FACES_RIGHT:
    case afxZodiacPlaneData::FACES_LEFT:
      local_cam_pos.x = 0;
      break;  
    }

    /* AFX_T3D_BROKEN -- enhanced transparency sorting of ZodiacPlanes JTF Note: evaluate this
    if (local_cam_pos.lenSquared() <= radius*radius)
    {
    ri->sortPoint = local_cam_pos;
    }
    else
    {
    local_cam_pos.normalize();
    ri->sortPoint = local_cam_pos*radius;
    }

    ri->sortPoint.convolve(uni_scale);
    getRenderTransform().mulP(ri->sortPoint);      
    */
  }
  state->getRenderPass()->addInst(ri);
}

void afxZodiacPlane::_renderZodiacPlane(ObjectRenderInst *ri, SceneRenderState* state, BaseMatInstance* overrideMat)
{
  if (overrideMat)
    return;

  // projection

  // predraw
  if (normal_sb.isNull())
  {
    GFXStateBlockDesc desc;

    // Culling
    desc.setCullMode((mDataBlock->double_sided) ? GFXCullNone : GFXCullCW);

    // Blending
    U32 blend = (mDataBlock->zflags & BLEND_MASK);
    switch (blend)
    {
    case BLEND_ADDITIVE:
      desc.setBlend(true, GFXBlendSrcAlpha, GFXBlendOne);
      break;
    case BLEND_SUBTRACTIVE:
      desc.setBlend(true, GFXBlendZero, GFXBlendInvSrcColor);
      break;
    case BLEND_NORMAL:
      desc.setBlend(true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
      break;
    }

    // JTF Note: check this desc.setAlphaTest((blend != BLEND_SUBTRACTIVE), GFXCmpGreater, 0);
    desc.setAlphaTest(true, GFXCmpGreater, 0);

    desc.setZReadWrite(true);
    desc.zFunc = GFXCmpLessEqual;
    desc.zWriteEnable = false;
    desc.samplersDefined = true;
    desc.samplers[0].textureColorOp = GFXTOPModulate;
    desc.samplers[1].textureColorOp = GFXTOPDisable;

    normal_sb = GFX->createStateBlock(desc);
    if (mDataBlock->double_sided)
      reflected_sb = normal_sb;
    else
    {
      desc.setCullMode(GFXCullCCW);
      reflected_sb = GFX->createStateBlock(desc);
    }
  }

  if (state->isReflectPass())
    GFX->setStateBlock(reflected_sb);
  else
    GFX->setStateBlock(normal_sb);

  Point3F basePoints[4];

  switch (mDataBlock->face_dir)
  {
  case afxZodiacPlaneData::FACES_UP:
    basePoints[0].set( 0.5f, -0.5f, 0.0f);
    basePoints[1].set(-0.5f, -0.5f, 0.0f);
    basePoints[2].set(-0.5f,  0.5f, 0.0f);
    basePoints[3].set( 0.5f,  0.5f, 0.0f);
    break;
  case afxZodiacPlaneData::FACES_DOWN:
    basePoints[3].set(-0.5f,  0.5f, 0.0f);
    basePoints[2].set( 0.5f,  0.5f, 0.0f);
    basePoints[1].set( 0.5f, -0.5f, 0.0f);
    basePoints[0].set(-0.5f, -0.5f, 0.0f);
    break;  
  case afxZodiacPlaneData::FACES_FORWARD:
    basePoints[0].set( 0.5f, 0.0f, -0.5f);
    basePoints[1].set(-0.5f, 0.0f, -0.5f);
    basePoints[2].set(-0.5f, 0.0f,  0.5f);
    basePoints[3].set( 0.5f, 0.0f,  0.5f);
    break;
  case afxZodiacPlaneData::FACES_BACK:
    basePoints[3].set(-0.5f, 0.0f,  0.5f);
    basePoints[2].set( 0.5f, 0.0f,  0.5f);
    basePoints[1].set( 0.5f, 0.0f, -0.5f);
    basePoints[0].set(-0.5f, 0.0f, -0.5f);
    break;  
  case afxZodiacPlaneData::FACES_RIGHT:
    basePoints[0].set(0.0f,  0.5f, -0.5f);
    basePoints[1].set(0.0f, -0.5f, -0.5f);
    basePoints[2].set(0.0f, -0.5f,  0.5f);
    basePoints[3].set(0.0f,  0.5f,  0.5f);
    break;
  case afxZodiacPlaneData::FACES_LEFT:
    basePoints[3].set(0.0f, -0.5f,  0.5f);
    basePoints[2].set(0.0f,  0.5f,  0.5f);
    basePoints[1].set(0.0f,  0.5f, -0.5f);
    basePoints[0].set(0.0f, -0.5f, -0.5f);
    break;  
  }

  F32 len = 2*radius;

  Point3F points[4];

  Point2F texCoords[4];   // default: {{0.0,0.0}, {0.0,1.0}, {1.0,1.0}, {1.0,0.0}}
  texCoords[0].set(1.0,1.0);
  texCoords[1].set(0.0,1.0);
  texCoords[2].set(0.0,0.0);
  texCoords[3].set(1.0,0.0);

  for( int i=0; i<4; i++ )
  {
    points[i].x = basePoints[i].x;
    points[i].y = basePoints[i].y;
    points[i].z = basePoints[i].z;
    points[i] *= len;
  }

  GFXTransformSaver saver;
  GFX->multWorld(getRenderTransform());

  GFX->setTexture(0, mDataBlock->txr);

  PrimBuild::begin(GFXTriangleStrip, 4);
  {
    PrimBuild::color4f(color.red, color.green, color.blue, color.alpha);
    PrimBuild::texCoord2f(texCoords[1].x, texCoords[1].y);
    PrimBuild::vertex3f(points[1].x,  points[1].y,  points[1].z);
    PrimBuild::texCoord2f(texCoords[0].x, texCoords[0].y);
    PrimBuild::vertex3f(points[0].x,  points[0].y,  points[0].z);
    PrimBuild::texCoord2f(texCoords[2].x, texCoords[2].y);
    PrimBuild::vertex3f(points[2].x,  points[2].y,  points[2].z);
    PrimBuild::texCoord2f(texCoords[3].x, texCoords[3].y);
    PrimBuild::vertex3f(points[3].x,  points[3].y,  points[3].z);
  }
  PrimBuild::end();
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
