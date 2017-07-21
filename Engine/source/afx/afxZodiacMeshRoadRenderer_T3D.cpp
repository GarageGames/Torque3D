
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
#include "collision/concretePolyList.h"
#include "T3D/tsStatic.h"
#include "gfx/primBuilder.h"

#include "afx/ce/afxZodiacMgr.h"
#include "afx/afxZodiacMeshRoadRenderer_T3D.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

const RenderInstType afxZodiacMeshRoadRenderer::RIT_MeshRoadZodiac("MeshRoadZodiac");

afxZodiacMeshRoadRenderer* afxZodiacMeshRoadRenderer::master = 0;

IMPLEMENT_CONOBJECT(afxZodiacMeshRoadRenderer);

ConsoleDocClass( afxZodiacMeshRoadRenderer, 
   "@brief A render bin for zodiac rendering on MeshRoad objects.\n\n"

   "This bin renders instances of AFX zodiac effects onto MeshRoad surfaces.\n\n"

   "@ingroup RenderBin\n"
   "@ingroup AFX\n"
);

afxZodiacMeshRoadRenderer::afxZodiacMeshRoadRenderer()
: RenderBinManager(RIT_MeshRoadZodiac, 1.0f, 1.0f)
{
   if (!master)
     master = this;
   shader_initialized = false;
}

afxZodiacMeshRoadRenderer::afxZodiacMeshRoadRenderer(F32 renderOrder, F32 processAddOrder)
: RenderBinManager(RIT_MeshRoadZodiac, renderOrder, processAddOrder)
{
   if (!master)
     master = this;
   shader_initialized = false;
}

afxZodiacMeshRoadRenderer::~afxZodiacMeshRoadRenderer()
{
  if (this == master)
    master = 0;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

void afxZodiacMeshRoadRenderer::initShader()
{
   if (shader_initialized)
     return;

   shader_initialized = true;

   shader_consts = 0;
   norm_norefl_zb_SB = norm_refl_zb_SB;
   add_norefl_zb_SB = add_refl_zb_SB;
   sub_norefl_zb_SB = sub_refl_zb_SB;

   zodiac_shader = afxZodiacMgr::getMeshRoadZodiacShader();
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
   d.zBias = arcaneFX::sPolysoupZodiacZBias;
   norm_norefl_zb_SB = GFX->createStateBlock(d);
   //
   d.cullMode = GFXCullCW;
   d.zBias = arcaneFX::sPolysoupZodiacZBias;
   norm_refl_zb_SB = GFX->createStateBlock(d);

   // additive
   d.blendSrc = GFXBlendSrcAlpha;
   d.blendDest = GFXBlendOne;
   //
   d.cullMode = GFXCullCCW;
   d.zBias = arcaneFX::sPolysoupZodiacZBias;
   add_norefl_zb_SB = GFX->createStateBlock(d);
   //
   d.cullMode = GFXCullCW;
   d.zBias = arcaneFX::sPolysoupZodiacZBias;
   add_refl_zb_SB = GFX->createStateBlock(d);

   // subtractive
   d.blendSrc = GFXBlendZero;
   d.blendDest = GFXBlendInvSrcColor;
   //
   d.cullMode = GFXCullCCW;
   d.zBias = arcaneFX::sPolysoupZodiacZBias;
   sub_norefl_zb_SB = GFX->createStateBlock(d);
   //
   d.cullMode = GFXCullCW;
   d.zBias = arcaneFX::sPolysoupZodiacZBias;
   sub_refl_zb_SB = GFX->createStateBlock(d);

   shader_consts = zodiac_shader->getShader()->allocConstBuffer();
   projection_sc = zodiac_shader->getShader()->getShaderConstHandle("$modelView");
   color_sc = zodiac_shader->getShader()->getShaderConstHandle("$zodiacColor");
}

void afxZodiacMeshRoadRenderer::clear()
{
  Parent::clear();
  for (S32 i = 0; i < meshRoad_zodiacs.size(); i++)
    if (meshRoad_zodiacs[i].polys)
      delete meshRoad_zodiacs[i].polys;
  meshRoad_zodiacs.clear();
}

void afxZodiacMeshRoadRenderer::addZodiac(U32 zode_idx, ConcretePolyList* polys, const Point3F& pos, F32 ang, const MeshRoad* road, F32 camDist)
{
  meshRoad_zodiacs.increment();
  MeshRoadZodiacElem& elem = meshRoad_zodiacs.last();

  elem.road = road;
  elem.polys = polys;
  elem.zode_idx = zode_idx;
  elem.ang = ang;
  elem.camDist = camDist;
}

afxZodiacMeshRoadRenderer* afxZodiacMeshRoadRenderer::getMaster()
{
  if (!master)
    master = new afxZodiacMeshRoadRenderer;
  return master;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

GFXStateBlock* afxZodiacMeshRoadRenderer::chooseStateBlock(U32 blend, bool isReflectPass)
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

void afxZodiacMeshRoadRenderer::render(SceneRenderState* state)
{
   PROFILE_SCOPE(afxRenderZodiacMeshRoadMgr_render);

   // Early out if no ground-plane zodiacs to draw.
   if (meshRoad_zodiacs.size() == 0)
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
   for (S32 zz = 0; zz < meshRoad_zodiacs.size(); zz++)
   {
      MeshRoadZodiacElem& elem = meshRoad_zodiacs[zz];

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

      F32 inv_radius = 1.0f/zode->radius_xy;

      PrimBuild::begin(GFXTriangleList, 3*elem.polys->mPolyList.size());
      for (U32 i = 0; i < elem.polys->mPolyList.size(); i++) 
      {
        ConcretePolyList::Poly* poly = &elem.polys->mPolyList[i];

        S32 vertind[3];
        vertind[0] = elem.polys->mIndexList[poly->vertexStart];
        vertind[1] = elem.polys->mIndexList[poly->vertexStart + 1];
        vertind[2] = elem.polys->mIndexList[poly->vertexStart + 2];

        for (U32 j = 0; j < 3; j++) 
        {
          Point3F vtx = elem.polys->mVertexList[vertind[j]];

          // compute UV
          F32 u1 = (vtx.x - zode->pos.x)*inv_radius;
          F32 v1 = (vtx.y - zode->pos.y)*inv_radius;
          F32 ru1 = u1*cos_ang - v1*sin_ang;
          F32 rv1 = u1*sin_ang + v1*cos_ang;

          F32 uu = (ru1 + 1.0f)/2.0f;
          F32 vv = 1.0f - (rv1 + 1.0f)/2.0f;

          PrimBuild::texCoord2f(uu, vv);
          PrimBuild::vertex3fv(vtx);
        }
      }
      PrimBuild::end(false);
   }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
