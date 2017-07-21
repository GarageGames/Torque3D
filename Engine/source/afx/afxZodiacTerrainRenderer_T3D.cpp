
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

#include "materials/shaderData.h"
#include "gfx/gfxTransformSaver.h"
#include "scene/sceneRenderState.h"
#include "terrain/terrData.h"
#include "terrain/terrCell.h"

#include "gfx/primBuilder.h"

#include "afx/ce/afxZodiacMgr.h"
#include "afx/afxZodiacTerrainRenderer_T3D.h"
#include "afx/util/afxTriBoxCheck2D_T3D.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class TerrCellSpy : public TerrCell
{
public:
  static const U32 getMinCellSize() { return smMinCellSize; }
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

const RenderInstType afxZodiacTerrainRenderer::RIT_TerrainZodiac("TerrainZodiac");

afxZodiacTerrainRenderer* afxZodiacTerrainRenderer::master = 0;

IMPLEMENT_CONOBJECT(afxZodiacTerrainRenderer);

ConsoleDocClass( afxZodiacTerrainRenderer, 
   "@brief A render bin for zodiac rendering on Terrain objects.\n\n"

   "This bin renders instances of AFX zodiac effects onto Terrain surfaces.\n\n"

   "@ingroup RenderBin\n"
   "@ingroup AFX\n"
);

afxZodiacTerrainRenderer::afxZodiacTerrainRenderer()
: RenderBinManager(RIT_TerrainZodiac, 1.0f, 1.0f)
{
   if (!master)
     master = this;
   shader_initialized = false;
}

afxZodiacTerrainRenderer::afxZodiacTerrainRenderer(F32 renderOrder, F32 processAddOrder)
: RenderBinManager(RIT_TerrainZodiac, renderOrder, processAddOrder)
{
   if (!master)
     master = this;
   shader_initialized = false;
}

afxZodiacTerrainRenderer::~afxZodiacTerrainRenderer()
{
  if (this == master)
    master = 0;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

void afxZodiacTerrainRenderer::initShader()
{
   if (shader_initialized)
     return;

   shader_initialized = true;

   shader_consts = 0;
   norm_norefl_zb_SB = norm_refl_zb_SB;
   add_norefl_zb_SB = add_refl_zb_SB;
   sub_norefl_zb_SB = sub_refl_zb_SB;

   zodiac_shader = afxZodiacMgr::getTerrainZodiacShader();
   if (!zodiac_shader)
     return;

   GFXStateBlockDesc d;

   d.cullDefined = true;
   d.ffLighting = false;
   d.blendDefined = true;
   d.blendEnable = true;
   d.zDefined = false;
   d.zEnable = true;
   d.zWriteEnable = false;
   d.zFunc = GFXCmpLessEqual;
   d.zSlopeBias = 0;
   d.alphaDefined = true;
   d.alphaTestEnable = true; 
   d.alphaTestRef = 0;
   d.alphaTestFunc = GFXCmpGreater;
   d.samplersDefined = true;
   d.samplers[0] = GFXSamplerStateDesc::getClampLinear();

   // normal
   d.blendSrc = GFXBlendSrcAlpha;
   d.blendDest = GFXBlendInvSrcAlpha;
   //
   d.cullMode = GFXCullCCW;
   d.zBias = arcaneFX::sTerrainZodiacZBias;
   norm_norefl_zb_SB = GFX->createStateBlock(d);
   //
   d.cullMode = GFXCullCW;
   d.zBias = arcaneFX::sTerrainZodiacZBias;
   norm_refl_zb_SB = GFX->createStateBlock(d);

   // additive
   d.blendSrc = GFXBlendSrcAlpha;
   d.blendDest = GFXBlendOne;
   //
   d.cullMode = GFXCullCCW;
   d.zBias = arcaneFX::sTerrainZodiacZBias;
   add_norefl_zb_SB = GFX->createStateBlock(d);
   //
   d.cullMode = GFXCullCW;
   d.zBias = arcaneFX::sTerrainZodiacZBias;
   add_refl_zb_SB = GFX->createStateBlock(d);

   // subtractive
   d.blendSrc = GFXBlendZero;
   d.blendDest = GFXBlendInvSrcColor;
   //
   d.cullMode = GFXCullCCW;
   d.zBias = arcaneFX::sTerrainZodiacZBias;
   sub_norefl_zb_SB = GFX->createStateBlock(d);
   //
   d.cullMode = GFXCullCW;
   d.zBias = arcaneFX::sTerrainZodiacZBias;
   sub_refl_zb_SB = GFX->createStateBlock(d);

   shader_consts = zodiac_shader->getShader()->allocConstBuffer();
   projection_sc = zodiac_shader->getShader()->getShaderConstHandle("$modelView");
   color_sc = zodiac_shader->getShader()->getShaderConstHandle("$zodiacColor");
}

void afxZodiacTerrainRenderer::clear()
{
  Parent::clear();

  terrain_zodes.clear();
}

void afxZodiacTerrainRenderer::addZodiac(U32 zode_idx, const Point3F& pos, F32 ang, 
                                         const TerrainBlock* block, const TerrCell* cell, 
                                         const MatrixF& mRenderObjToWorld, F32 camDist)
{
  terrain_zodes.increment();
  TerrainZodiacElem& elem = terrain_zodes.last();

  elem.block = block;
  elem.cell = cell;
  elem.zode_idx = zode_idx;
  elem.ang = ang;
  elem.mRenderObjToWorld = mRenderObjToWorld;
  elem.camDist = camDist;
}

afxZodiacTerrainRenderer* afxZodiacTerrainRenderer::getMaster()
{
  if (!master)
    master = new afxZodiacTerrainRenderer;
  return master;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

GFXStateBlock* afxZodiacTerrainRenderer::chooseStateBlock(U32 blend, bool isReflectPass)
{
  GFXStateBlock* sb = 0;
  
  switch (blend)
  {
  case afxZodiacData::BLEND_ADDITIVE:
    sb = (isReflectPass) ? add_refl_zb_SB : add_norefl_zb_SB;
    break;
  case afxZodiacData::BLEND_SUBTRACTIVE:
    sb = (isReflectPass) ? sub_refl_zb_SB : sub_norefl_zb_SB;
    break;
  default: // afxZodiacData::BLEND_NORMAL:
    sb = (isReflectPass) ? norm_refl_zb_SB : norm_norefl_zb_SB;
    break;
  }

  return sb;
}

void afxZodiacTerrainRenderer::render(SceneRenderState* state)
{
   PROFILE_SCOPE(afxRenderZodiacTerrainMgr_render);

   // Early out if no terrain zodiacs to draw.
   if (terrain_zodes.size() == 0)
     return;

   initShader();
   if (!zodiac_shader)
     return;

   bool is_reflect_pass = state->isReflectPass();

   // Automagically save & restore our viewport and transforms.
   GFXTransformSaver saver;

   MatrixF proj = GFX->getProjectionMatrix();

   // Set up world transform
   MatrixF world = GFX->getWorldMatrix();
   proj.mul(world);
   shader_consts->set(projection_sc, proj);

   //~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//
   // RENDER EACH ZODIAC
   //
   for (S32 zz = 0; zz < terrain_zodes.size(); zz++)
   {
      TerrainZodiacElem& elem = terrain_zodes[zz];

      TerrainBlock* block = (TerrainBlock*) elem.block;

      afxZodiacMgr::ZodiacSpec* zode = &afxZodiacMgr::terr_zodes[elem.zode_idx];
      if (!zode)
         continue;

      if (is_reflect_pass)
      {
         if ((zode->zflags & afxZodiacData::SHOW_IN_REFLECTIONS) == 0)
            continue;
      }
      else
      {
         if ((zode->zflags & afxZodiacData::SHOW_IN_NON_REFLECTIONS) == 0)
            continue;
      }

      F32 fadebias = zode->calcDistanceFadeBias(elem.camDist);
      if (fadebias < 0.01f)
        continue;

      F32 cos_ang = mCos(elem.ang);
      F32 sin_ang = mSin(elem.ang);

      GFXStateBlock* sb = chooseStateBlock(zode->zflags & afxZodiacData::BLEND_MASK, is_reflect_pass);

      GFX->setShader(zodiac_shader->getShader());
      GFX->setStateBlock(sb);
      GFX->setShaderConstBuffer(shader_consts);

      // set the texture
      GFX->setTexture(0, *zode->txr);
      ColorF zode_color = (ColorF)zode->color;
      zode_color.alpha *= fadebias;
      shader_consts->set(color_sc, zode_color);

      Point3F half_size(zode->radius_xy,zode->radius_xy,zode->radius_xy);

      F32 inv_radius = 1.0f/zode->radius_xy;

      GFXPrimitive cell_prim;
      GFXVertexBufferHandle<TerrVertex> cell_verts;
      GFXPrimitiveBufferHandle  primBuff;
      elem.cell->getRenderPrimitive(&cell_prim, &cell_verts, &primBuff);

      U32 n_nonskirt_tris = TerrCellSpy::getMinCellSize()*TerrCellSpy::getMinCellSize()*2;

      const Point3F* verts = ((TerrCell*)elem.cell)->getZodiacVertexBuffer();
      const U16 *tris = block->getZodiacPrimitiveBuffer();
      if (!tris)
         continue; 

      PrimBuild::begin(GFXTriangleList, 3*n_nonskirt_tris);

      /////////////////////////////////
      U32 n_overlapping_tris = 0;
      U32 idx = 0;
      for (U32 i = 0; i < n_nonskirt_tris; i++)
      {
        Point3F tri_v[3];
        tri_v[0] = verts[tris[idx++]];
        tri_v[1] = verts[tris[idx++]];
        tri_v[2] = verts[tris[idx++]];

        elem.mRenderObjToWorld.mulP(tri_v[0]);
        elem.mRenderObjToWorld.mulP(tri_v[1]);
        elem.mRenderObjToWorld.mulP(tri_v[2]);

        if (!afxTriBoxOverlap2D(zode->pos, half_size, tri_v[0], tri_v[1], tri_v[2]))
          continue;

        n_overlapping_tris++;

        for (U32 j = 0; j < 3; j++)
        {
          // compute UV
          F32 u1 = (tri_v[j].x - zode->pos.x)*inv_radius;
          F32 v1 = (tri_v[j].y - zode->pos.y)*inv_radius;
          F32 ru1 = u1*cos_ang - v1*sin_ang;
          F32 rv1 = u1*sin_ang + v1*cos_ang;

          F32 uu = (ru1 + 1.0f)/2.0f;
          F32 vv = 1.0f - (rv1 + 1.0f)/2.0f;

          PrimBuild::texCoord2f(uu, vv);
          PrimBuild::vertex3fv(tri_v[j]);
        }
      }

      /////////////////////////////////

      PrimBuild::end(false);
   }
   //
   // RENDER EACH ZODIAC
   //~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
