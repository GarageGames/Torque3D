
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
#include "afx/ce/afxBillboard.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

void afxBillboard::prepRenderImage(SceneRenderState* state)
{
  if (!is_visible)
    return;

  ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
  ri->renderDelegate.bind(this, &afxBillboard::_renderBillboard);
  ri->type = RenderPassManager::RIT_ObjectTranslucent;
  ri->translucentSort = true;
  ri->defaultKey = (U32)(dsize_t)mDataBlock;
  ri->sortDistSq = getWorldBox().getSqDistanceToPoint( state->getCameraPosition() );      
  state->getRenderPass()->addInst(ri);
}

void afxBillboard::_renderBillboard(ObjectRenderInst *ri, SceneRenderState* state, BaseMatInstance* overrideMat)
{
  if (overrideMat)
    return;

  // predraw
  if (normal_sb.isNull())
  {
    GFXStateBlockDesc desc;

    // Culling -- it's a billboard, so no backfaces
    desc.setCullMode(GFXCullCW);

    // Blending
    desc.setBlend(true, mDataBlock->srcBlendFactor, mDataBlock->dstBlendFactor);
    desc.alphaTestEnable = (desc.blendSrc == GFXBlendSrcAlpha && 
                            (desc.blendDest == GFXBlendInvSrcAlpha || desc.blendDest == GFXBlendOne));
    desc.alphaTestRef = 1;
    desc.alphaTestFunc = GFXCmpGreaterEqual;
    
    desc.setZReadWrite(true);
    desc.zFunc = GFXCmpLessEqual;
    desc.zWriteEnable = false;

    desc.samplersDefined = true;
    switch (mDataBlock->texFunc)
    {
    case afxBillboardData::TexFuncReplace:
      desc.samplers[0].textureColorOp = GFXTOPDisable;
      break;
    case afxBillboardData::TexFuncModulate:
      desc.samplers[0].textureColorOp = GFXTOPModulate;
      break;
    case afxBillboardData::TexFuncAdd:
      desc.samplers[0].textureColorOp = GFXTOPAdd;
      break;
    }

    desc.samplers[1].textureColorOp = GFXTOPDisable;

    normal_sb = GFX->createStateBlock(desc);

    desc.setCullMode(GFXCullCCW);
    reflected_sb = GFX->createStateBlock(desc);
  }

  if (state->isReflectPass())
    GFX->setStateBlock(reflected_sb);
  else
    GFX->setStateBlock(normal_sb);

  GFXTransformSaver saver;
  GFX->multWorld(getRenderTransform());

  GFX->setTexture(0, mDataBlock->txr);

	MatrixF worldmod = GFX->getWorldMatrix();
	MatrixF viewmod = GFX->getViewMatrix();

  Point4F Position;
  MatrixF ModelView;
	ModelView.mul(viewmod, worldmod);
	ModelView.getColumn(3, &Position);
	ModelView.identity();
	ModelView.setColumn(3, Position);

	GFX->setWorldMatrix(ModelView);
	MatrixF ident;
	ident.identity();
	GFX->setViewMatrix(ident);

  F32 width = mDataBlock->dimensions.x * 0.5f * mObjScale.x;
  F32 height = mDataBlock->dimensions.y * 0.5f * mObjScale.z;

  Point3F points[4];
  points[0].set( width, 0.0f, -height);
  points[1].set(-width, 0.0f, -height);
  points[2].set(-width, 0.0f,  height);
  points[3].set( width, 0.0f,  height);

  PrimBuild::begin(GFXTriangleStrip, 4);
  {
	  PrimBuild::color4f(live_color.red, live_color.green, live_color.blue, live_color.alpha*fade_amt);
    PrimBuild::texCoord2f(mDataBlock->texCoords[1].x, mDataBlock->texCoords[1].y);
    PrimBuild::vertex3fv(points[1]);
    PrimBuild::texCoord2f(mDataBlock->texCoords[2].x, mDataBlock->texCoords[2].y);
    PrimBuild::vertex3fv(points[0]);
    PrimBuild::texCoord2f(mDataBlock->texCoords[0].x, mDataBlock->texCoords[0].y);
    PrimBuild::vertex3fv(points[2]);
    PrimBuild::texCoord2f(mDataBlock->texCoords[3].x, mDataBlock->texCoords[3].y);
    PrimBuild::vertex3fv(points[3]);
  }
  PrimBuild::end();
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//