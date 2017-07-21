
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

#include "terrain/terrRender.h"

#include "afx/ce/afxZodiac.h"
#include "afx/ce/afxZodiacMgr.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxZodiacMgr

Vector<afxZodiacMgr::ZodiacSpec>  afxZodiacMgr::terr_zodes;
Vector<afxZodiacMgr::ZodiacSpec>  afxZodiacMgr::inter_zodes;

afxZodiacMgr::ZodiacTriangle*   afxZodiacMgr::zode_tris_head = NULL;
afxZodiacMgr::ZodiacTriangle*   afxZodiacMgr::zode_tris_tail = NULL;
afxZodiacMgr::ZodiacTriangle*   afxZodiacMgr::zode_tris = NULL;
U32                             afxZodiacMgr::zode_tris_idx = 0;
U32                             afxZodiacMgr::n_zode_tris = 0;

afxZodiacMgr::ZodiacSpec*       afxZodiacMgr::live_zodiac = 0;
ShaderData*                     afxZodiacMgr::terrain_zode_shader = 0;
ShaderData*                     afxZodiacMgr::atlas_zode_shader = 0;
ShaderData*                     afxZodiacMgr::interior_zode_shader = 0;
ShaderData*                     afxZodiacMgr::polysoup_zode_shader = 0;

void afxZodiacMgr::addTerrainZodiac(Point3F& pos, F32 radius, ColorF& color, F32 angle, afxZodiacData* zode)
{
  if (radius < 0.001f)
    return;

  ZodiacSpec z;
  z.pos = pos;
  z.radius_xy = radius;
  z.vert_range.set(0.0f, 0.0f);
  z.grade_range.set(0.0f, 1.0f);
  z.color = color;
  z.angle = mDegToRad(angle);
  z.zflags = zode->zflags;
  z.txr = &zode->txr;

  z.distance_max = zode->distance_max*zode->distance_max;
  z.distance_falloff = zode->distance_falloff*zode->distance_falloff;
  z.distance_delta = z.distance_max - z.distance_falloff;

  if (terr_zodes.size() < MAX_ZODIACS)
    terr_zodes.push_back(z);
}

void afxZodiacMgr::addInteriorZodiac(Point3F& pos, F32 radius, Point2F& vert_range, ColorF& color, F32 angle, afxZodiacData* zode)
{
  if (radius < 0.001f)
    return;

  ZodiacSpec z;
  z.pos = pos;
  z.radius_xy = radius;
  z.vert_range = vert_range;
  z.grade_range = zode->grade_range;
  z.color = color;
  z.angle = mDegToRad(angle);
  z.zflags = zode->zflags;
  z.txr = &zode->txr;

  z.distance_max = zode->distance_max*zode->distance_max;
  z.distance_falloff = zode->distance_falloff*zode->distance_falloff;
  z.distance_delta = z.distance_max - z.distance_falloff;

  if (inter_zodes.size() < MAX_ZODIACS)
    inter_zodes.push_back(z);
}

void afxZodiacMgr::frameReset()
{
  terr_zodes.clear();
  inter_zodes.clear();
}

void afxZodiacMgr::missionCleanup()
{
  terrain_zode_shader = 0;
  atlas_zode_shader = 0;
  interior_zode_shader = 0;
  polysoup_zode_shader = 0;
}

// REGULAR TERRAIN ZODIACS //

void afxZodiacMgr::transformTerrainZodiacs(const MatrixF& world_xfm)
{
  VectorF facing_vec;
  world_xfm.getColumn(1, &facing_vec);
  F32 yaw = mAtan2(facing_vec.x, facing_vec.y);
  while (yaw < 0.0) yaw += M_2PI_F;

  for (S32 i = 0; i < terr_zodes.size(); i++)
  {
    world_xfm.mulP(terr_zodes[i].pos, &terr_zodes[i].loc_pos);
    F32 ang = terr_zodes[i].angle + yaw;
    terr_zodes[i].loc_cos_ang = mCos(ang);
    terr_zodes[i].loc_sin_ang = mSin(ang);
  }

  zode_tris_head = zode_tris_tail = NULL;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
