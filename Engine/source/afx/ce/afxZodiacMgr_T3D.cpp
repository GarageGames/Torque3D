
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

#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"
#include "scene/sceneRenderState.h"
#include "materials/shaderData.h"
#include "core/frameAllocator.h"
#include "terrain/terrRender.h"
#include "T3D/tsStatic.h"
#include "T3D/groundPlane.h"
#include "environment/meshRoad.h"
#include "collision/concretePolyList.h"
#include "gfx/primBuilder.h"
#include "terrain/terrCell.h"

#include "afx/ce/afxZodiac.h"
#include "afx/ce/afxZodiacMgr.h"
#include "afx/afxZodiacTerrainRenderer_T3D.h"
#include "afx/afxZodiacGroundPlaneRenderer_T3D.h"
#include "afx/afxZodiacMeshRoadRenderer_T3D.h"
#include "afx/afxZodiacPolysoupRenderer_T3D.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

// POLYSOUP INTERIOR ZODIACS //

void afxZodiacMgr::renderPolysoupZodiacs(SceneRenderState* state, TSStatic* tss)
{
  // check for active interior zodiacs
  S32 n_zodes = inter_zodes.size();
  if (n_zodes <= 0)
    return;
 
  static SphereF dummy_sphere;
  const Box3F& mWorldBox = tss->getWorldBox();
  const Point3F& cam_pos = state->getCameraPosition(); 

  // loop through the interior zodiacs
  for (U32 i = 0; i < n_zodes; i++) 
  {      
    // calculate zodiac extents
    F32 radius = inter_zodes[i].radius_xy;
    Box3F box_w; box_w.minExtents = box_w.maxExtents = inter_zodes[i].pos;
    box_w.minExtents -= Point3F(radius, radius, inter_zodes[i].vert_range.x);
    box_w.maxExtents += Point3F(radius, radius, inter_zodes[i].vert_range.y);

    // skip zodiacs not overlapping this object
    if (mWorldBox.isOverlapped(box_w) == false)
      continue;

    // collect list of zodiac-intersecting polygons
    ConcretePolyList* poly_list = new ConcretePolyList();
    ((SceneObject*)tss)->buildPolyList(PLC_Decal, poly_list, box_w, dummy_sphere);

    // render the polys if we get any
    if (!poly_list->mPolyList.empty())
    {
      // calculate zodiac distance from camera
      Point3F cam_vec = cam_pos - inter_zodes[i].pos;
      F32 cam_dist = cam_vec.lenSquared();

      // render zodiacs
      afxZodiacPolysoupRenderer::getMaster()->addZodiac(i, poly_list, inter_zodes[i].pos, inter_zodes[i].angle, tss, cam_dist);
      // note: poly_list will be deleted by render-manager after rendering is done.
    }
    else
    {
      delete poly_list; // avoids crash when overlapping box but not polygons
    }
  }
}

ShaderData* afxZodiacMgr::getPolysoupZodiacShader()
{
  if (!polysoup_zode_shader)
  {
    if( !Sim::findObject("afxZodiacPolysoupShader", polysoup_zode_shader))
    {
      Con::errorf("afxZodiacMgr::getPolysoupZodiacShader() - failed to find shader 'afxZodiacPolysoupShader'.");
      return 0;
    }
  }

  return polysoup_zode_shader;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
  
bool afxZodiacMgr::doesBoxOverlapZodiac(const Box3F& box, const afxZodiacMgr::ZodiacSpec& zode)
{
  if ((zode.pos.x - zode.radius_xy) > box.maxExtents.x ||
      (zode.pos.y - zode.radius_xy) > box.maxExtents.y)
     return false;
  if ((zode.pos.x + zode.radius_xy) < box.minExtents.x ||
     ( zode.pos.y + zode.radius_xy) < box.minExtents.y)
     return false;
  return true;
}

static U32 last_terr_zode_idx = 0;

bool afxZodiacMgr::doesBlockContainZodiacs(SceneRenderState* state, TerrainBlock* block)
{
  // for now, only look at diffuse-passes
  if (!state->isDiffusePass())
    return false;

  // check for active terrain zodiacs
  S32 n_zodes = terr_zodes.size();
  if (n_zodes <= 0)
    return false;

  const Box3F& block_box = block->getWorldBox();

  last_terr_zode_idx = 0;
  for (U32 i = 0; i < n_zodes; i++) 
  { 
    if (doesBoxOverlapZodiac(block_box, terr_zodes[i]))
    {
      last_terr_zode_idx = i;
      return true;
    }
  }

  return false;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

bool afxZodiacMgr::renderTerrainZodiacs(SceneRenderState* state, TerrainBlock* block, TerrCell* cell)
{
  // we assume here that afxZodiacMgr::doesBlockContainZodiacs() was recently called to
  // determine that at least one zodiac intersects the block and its index is last_terr_zode_idx.

  bool cell_has_zodiacs = false; 

  const Point3F& cam_pos = state->getCameraPosition(); 
  const Box3F& block_box = block->getWorldBox();

  S32 n_zodes = terr_zodes.size();
  for (U32 i = last_terr_zode_idx; i < n_zodes; i++) 
  { 
    // first test against block's box
    if (!doesBoxOverlapZodiac(block_box, terr_zodes[i]))
      continue;

    const MatrixF& mRenderObjToWorld = block->getRenderTransform();
    Box3F cell_box = cell->getBounds();
    mRenderObjToWorld.mul(cell_box);
    if (!doesBoxOverlapZodiac(cell_box, terr_zodes[i]))
      continue;

    // at this point we know the zodiac overlaps AABB of cell...

    // calculate zodiac distance from camera
    Point3F cam_vec = cam_pos - terr_zodes[i].pos;
    F32 cam_dist = cam_vec.lenSquared();
    //if (cam_dist > 2500)
    //  continue;

    // render zodiacs
    afxZodiacTerrainRenderer::getMaster()->addZodiac(i, terr_zodes[i].pos, terr_zodes[i].angle, block, cell, mRenderObjToWorld, cam_dist);

    cell_has_zodiacs = true;
  }

  last_terr_zode_idx = 0;

  return cell_has_zodiacs;
}

ShaderData* afxZodiacMgr::getTerrainZodiacShader()
{
  if (!terrain_zode_shader)
  {
    if (!Sim::findObject("afxZodiacTerrainShader", terrain_zode_shader))
    {
      Con::errorf("afxZodiacMgr::getTerrainZodiacShader() - failed to find shader 'afxZodiacTerrainShader'.");
      return 0;
    }
  }

  return terrain_zode_shader;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

void afxZodiacMgr::renderGroundPlaneZodiacs(SceneRenderState* state, GroundPlane* ground_plane)
{
  // check for active terrain zodiacs
  S32 n_zodes = terr_zodes.size();
  if (n_zodes <= 0)
    return;

  // we currently expect gound-plane to be infinite
  if (!ground_plane->isGlobalBounds())
  {
    return;
  }

  static SphereF dummy_sphere;
  static Box3F dummy_box;

  const Point3F& cam_pos = state->getCameraPosition(); 

  // loop through the terrain zodiacs
  for (U32 i = 0; i < n_zodes; i++) 
  { 
    // calculate zodiac distance from camera
    Point3F cam_vec = cam_pos - terr_zodes[i].pos;
    F32 cam_dist = cam_vec.lenSquared();

    // render zodiacs
    afxZodiacGroundPlaneRenderer::getMaster()->addZodiac(i, terr_zodes[i].pos, terr_zodes[i].angle, ground_plane, cam_dist);
  }
}

ShaderData* afxZodiacMgr::getGroundPlaneZodiacShader()
{
  if (!terrain_zode_shader)
  {
    if (!Sim::findObject("afxZodiacTerrainShader", terrain_zode_shader))
    {
      Con::errorf("afxZodiacMgr::getTerrainZodiacShader() - failed to find shader 'afxZodiacTerrainShader'.");
      return 0;
    }
  }

  return terrain_zode_shader;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

void afxZodiacMgr::renderMeshRoadZodiacs(SceneRenderState* state, MeshRoad* mesh_road)
{
  // check for active terrain zodiacs
  S32 n_zodes = terr_zodes.size();
  if (n_zodes <= 0)
    return;

  const Box3F& mWorldBox = mesh_road->getWorldBox();
  const Point3F& cam_pos = state->getCameraPosition(); 

  // loop through the terrain zodiacs
  for (U32 i = 0; i < n_zodes; i++) 
  {      
    // calculate zodiac extents
    F32 radius = terr_zodes[i].radius_xy;
    Box3F box_w; box_w.minExtents = box_w.maxExtents = terr_zodes[i].pos;
    box_w.minExtents -= Point3F(radius, radius, terr_zodes[i].vert_range.x);
    box_w.maxExtents += Point3F(radius, radius, terr_zodes[i].vert_range.y);

    // skip zodiacs not overlapping this object
    if (mWorldBox.isOverlapped(box_w) == false)
      continue;

    // collect list of zodiac-intersecting polygons
    ConcretePolyList* poly_list = new ConcretePolyList();

    mesh_road->buildTopPolyList(PLC_Decal, poly_list);

    // render the polys if we get any
    if (!poly_list->mPolyList.empty())
    {
      // calculate zodiac distance from camera
      Point3F cam_vec = cam_pos - terr_zodes[i].pos;
      F32 cam_dist = cam_vec.lenSquared();

      // render zodiacs
      afxZodiacMeshRoadRenderer::getMaster()->addZodiac(i, poly_list, terr_zodes[i].pos, terr_zodes[i].angle, mesh_road, cam_dist);
      // note: poly_list will be deleted by render-manager after rendering is done.
    }
    else
    {
      delete poly_list; // avoids crash when overlapping box but not polygons
    }
  }
}

ShaderData* afxZodiacMgr::getMeshRoadZodiacShader()
{
  if (!terrain_zode_shader)
  {
    if (!Sim::findObject("afxZodiacTerrainShader", terrain_zode_shader))
    {
      Con::errorf("afxZodiacMgr::getTerrainZodiacShader() - failed to find shader 'afxZodiacTerrainShader'.");
      return 0;
    }
  }

  return terrain_zode_shader;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//