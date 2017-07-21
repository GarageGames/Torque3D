
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
#include "afx/afxZodiacPolysoupRenderer_T3D.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

const RenderInstType afxZodiacPolysoupRenderer::RIT_PolysoupZodiac("PolysoupZodiac");

afxZodiacPolysoupRenderer* afxZodiacPolysoupRenderer::master = 0;

IMPLEMENT_CONOBJECT(afxZodiacPolysoupRenderer);

ConsoleDocClass( afxZodiacPolysoupRenderer, 
   "@brief A render bin for zodiac rendering on polysoup TSStatic objects.\n\n"

   "This bin renders instances of AFX zodiac effects onto polysoup TSStatic surfaces.\n\n"

   "@ingroup RenderBin\n"
   "@ingroup AFX\n"
);

afxZodiacPolysoupRenderer::afxZodiacPolysoupRenderer()
: RenderBinManager(RIT_PolysoupZodiac, 1.0f, 1.0f)
{
   if (!master)
     master = this;
   shader_initialized = false;
}

afxZodiacPolysoupRenderer::afxZodiacPolysoupRenderer(F32 renderOrder, F32 processAddOrder)
: RenderBinManager(RIT_PolysoupZodiac, renderOrder, processAddOrder)
{
   if (!master)
     master = this;
   shader_initialized = false;
}

afxZodiacPolysoupRenderer::~afxZodiacPolysoupRenderer()
{
  if (this == master)
    master = 0;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

void afxZodiacPolysoupRenderer::initShader()
{
   if (shader_initialized)
     return;

   shader_initialized = true;

   shader_consts = 0;
   norm_norefl_zb_SB = norm_refl_zb_SB;
   add_norefl_zb_SB = add_refl_zb_SB;
   sub_norefl_zb_SB = sub_refl_zb_SB;

   zodiac_shader = afxZodiacMgr::getPolysoupZodiacShader();
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

void afxZodiacPolysoupRenderer::clear()
{
  Parent::clear();
  for (S32 i = 0; i < polysoup_zodes.size(); i++)
    if (polysoup_zodes[i].polys)
      delete polysoup_zodes[i].polys;
  polysoup_zodes.clear();
}

void afxZodiacPolysoupRenderer::addZodiac(U32 zode_idx, ConcretePolyList* polys, const Point3F& pos, F32 ang, const TSStatic* tss, F32 camDist)
{
  polysoup_zodes.increment();
  PolysoupZodiacElem& elem = polysoup_zodes.last();

  elem.tss = tss;
  elem.polys = polys;
  elem.zode_idx = zode_idx;
  elem.ang = ang;
  elem.camDist = camDist;
}

afxZodiacPolysoupRenderer* afxZodiacPolysoupRenderer::getMaster()
{
  if (!master)
    master = new afxZodiacPolysoupRenderer;
  return master;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

GFXStateBlock* afxZodiacPolysoupRenderer::chooseStateBlock(U32 blend, bool isReflectPass)
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

void afxZodiacPolysoupRenderer::render(SceneRenderState* state)
{
   PROFILE_SCOPE(afxRenderZodiacPolysoupMgr_render);

   // Early out if no polysoup zodiacs to draw.
   if (polysoup_zodes.size() == 0)
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
   for (S32 zz = 0; zz < polysoup_zodes.size(); zz++)
   {
      PolysoupZodiacElem& elem = polysoup_zodes[zz];

      afxZodiacMgr::ZodiacSpec* zode = &afxZodiacMgr::inter_zodes[elem.zode_idx];
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

      // FILTER USING GRADIENT RANGE
      if ((zode->zflags & afxZodiacData::USE_GRADE_RANGE) != 0)
      {
        bool skip_oob;
        F32 grade_min, grade_max;
        if (elem.tss->mHasGradients && ((zode->zflags & afxZodiacData::PREFER_DEST_GRADE) != 0))
        {
          skip_oob = (elem.tss->mInvertGradientRange == false);
          grade_min = elem.tss->mGradientRange.x;
          grade_max = elem.tss->mGradientRange.y;
        }
        else
        {
          skip_oob = ((zode->zflags & afxZodiacData::INVERT_GRADE_RANGE) == 0);
          grade_min = zode->grade_range.x;
          grade_max = zode->grade_range.y;
        }

        PrimBuild::begin(GFXTriangleList, 3*elem.polys->mPolyList.size());
        for (U32 i = 0; i < elem.polys->mPolyList.size(); i++)
        {
          ConcretePolyList::Poly* poly = &elem.polys->mPolyList[i];

          const PlaneF& plane = poly->plane;

          bool oob = (plane.z > grade_max || plane.z < grade_min);          
          if (oob == skip_oob)
            continue;

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

      // FILTER USING OTHER FILTERS
      else if (zode->zflags & afxZodiacData::INTERIOR_FILTERS)
      {
        PrimBuild::begin(GFXTriangleList, 3*elem.polys->mPolyList.size());
        for (U32 i = 0; i < elem.polys->mPolyList.size(); i++)
        {
          ConcretePolyList::Poly* poly = &elem.polys->mPolyList[i];

          const PlaneF& plane = poly->plane;
          if (zode->zflags & afxZodiacData::INTERIOR_HORIZ_ONLY)
          {
            if (!plane.isHorizontal())
              continue;

            if (zode->zflags & afxZodiacData::INTERIOR_BACK_IGNORE)
            {
              if (plane.whichSide(zode->pos) == PlaneF::Back)
                continue;
            }
          }
          else
          {
            if (zode->zflags & afxZodiacData::INTERIOR_VERT_IGNORE)
            {
              if (plane.isVertical())
                continue;
            }

            if (zode->zflags & afxZodiacData::INTERIOR_BACK_IGNORE)
            {
              if (plane.whichSide(zode->pos) == PlaneF::Back)
                continue;
            }
          }

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

      // NO FILTERING
      else
      {
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
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
