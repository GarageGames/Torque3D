
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
#include "afx/ce/afxMooring.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

void afxMooring::prepRenderImage(SceneRenderState* state)
{
  if (!mDataBlock->display_axis_marker)
    return;

  ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
  ri->renderDelegate.bind(this, &afxMooring::_renderAxisLines);
  ri->type = RenderPassManager::RIT_ObjectTranslucent;
  ri->translucentSort = true;
  ri->defaultKey = (U32)(dsize_t)mDataBlock;
  ri->sortDistSq = getWorldBox().getSqDistanceToPoint( state->getCameraPosition() );      
  state->getRenderPass()->addInst(ri);
}

void afxMooring::_renderAxisLines(ObjectRenderInst *ri, SceneRenderState* state, BaseMatInstance* overrideMat)
{
  if (overrideMat)
    return;

  if (axis_sb.isNull())
  {
    GFXStateBlockDesc desc;

    desc.blendDefined = true;
    desc.blendEnable = false;
    desc.cullDefined = true;
    desc.cullMode = GFXCullNone;
    desc.ffLighting = false;
    desc.zDefined = true;
    desc.zWriteEnable = false;

    axis_sb = GFX->createStateBlock(desc);
  }

  GFX->setStateBlock(axis_sb);

  GFXTransformSaver saver;
  GFX->multWorld(getRenderTransform());

	PrimBuild::begin(GFXLineList, 6);
	PrimBuild::color(ColorF(1.0, 0.0, 0.0));
	PrimBuild::vertex3f(-0.5,  0.0,  0.0);
	PrimBuild::vertex3f( 0.5,  0.0,  0.0);
	PrimBuild::color(ColorF(0.0, 1.0, 0.0));
	PrimBuild::vertex3f( 0.0, -0.5,  0.0);
	PrimBuild::vertex3f( 0.0,  0.5,  0.0);
	PrimBuild::color(ColorF(0.0, 0.0, 1.0));
	PrimBuild::vertex3f( 0.0,  0.0, -0.5);
	PrimBuild::vertex3f( 0.0,  0.0,  0.5);
	PrimBuild::end();
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//