
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

#ifndef _AFX_ZODIAC_MGR_H_
#define _AFX_ZODIAC_MGR_H_

#ifndef _ARCANE_FX_H_
#include "afx/arcaneFX.h"
#endif
#ifndef _AFX_ZODIAC_DEFS_H_
#include "afx/ce/afxZodiacDefs.h"
#endif
#ifndef _AFX_ZODIAC_H_
#include "afx/ce/afxZodiac.h"
#endif

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

struct GridSquare;

class ShaderData;
class TSStatic;

class GroundPlane;
class MeshRoad;
class TerrainBlock;
class TerrCell;

class afxZodiacMgr : public afxZodiacDefs
{
  friend class afxZodiacTerrainRenderer;
  friend class afxZodiacPolysoupRenderer;
  friend class afxZodiacGroundPlaneRenderer;
  friend class afxZodiacMeshRoadRenderer;

  friend struct TerrainRender;

private:
  struct ZodiacSpec
  {
     Point3F        pos;              //12// world position
     F32            radius_xy;        // 4// radius of zodiac
     Point2F        vert_range;       // 8// vertical range
     Point2F        grade_range;      // 8// plane gradient range
     ColorI         color;            // 4// color of zodiac
     F32            angle;            // 4// angle in radians
     U32            zflags;           // 4// 0=normal,1=additive,2=subtractive
     GFXTexHandle*  txr;              // 4// zodiac texture

     F32            distance_max;
     F32            distance_falloff;
     F32            distance_delta;

     Point3F        loc_pos;          //12// transformed to local position
     F32            loc_cos_ang;      // 4// cosine of local rotation angle
     F32            loc_sin_ang;      // 4// sine of local rotation angle

     F32            calcDistanceFadeBias(F32 camDist) const
                    {
                      if (camDist < distance_falloff)
                        return 1.0f;
                      if (camDist < distance_max)
                        return (1.0f - (camDist - distance_falloff)/distance_delta);
                      return 0.0f;
                    }
  };

  struct ZodiacTriangle 
  {
     ColorI           color;
     Point2F          texco1;
     Point3F          point1;
     Point2F          texco2;
     Point3F          point2;
     Point2F          texco3;
     Point3F          point3;
     U32              zflags; // 0=normal,1=additive,2=subtractive
     GFXTexHandle*    txr;
     ZodiacTriangle*  next;
  };

  static Vector<ZodiacSpec> terr_zodes;
  static Vector<ZodiacSpec> inter_zodes;

  static ZodiacTriangle*  zode_tris_head;
  static ZodiacTriangle*  zode_tris_tail;
  static ZodiacTriangle*  zode_tris;
  static U32              zode_tris_idx;
  static U32              n_zode_tris;

public:
  static ZodiacSpec*      live_zodiac;
private:
  static ShaderData*      terrain_zode_shader;
  static ShaderData*      atlas_zode_shader;
  static ShaderData*      interior_zode_shader;
  static ShaderData*      polysoup_zode_shader;
public:
  static void   addTerrainZodiac(Point3F& pos, F32 rad, ColorF&, F32 ang, afxZodiacData*);
  static void   addInteriorZodiac(Point3F& pos, F32 rad, Point2F& vrange, ColorF&, F32 ang, afxZodiacData*);
  static void   frameReset();
  static void   missionCleanup();

  static S32    numTerrainZodiacs() { return terr_zodes.size(); }
  static S32    numInteriorZodiacs() { return inter_zodes.size(); }

  static void   transformTerrainZodiacs(const MatrixF& world_xfm);
  static void   testTerrainOverlap(GridSquare*, S32 level, Point2I sq_pos, afxZodiacBitmask&);

  static bool   doesBoxOverlapZodiac(const Box3F& box, const  ZodiacSpec& zode);
  static bool   doesBlockContainZodiacs(SceneRenderState*, TerrainBlock* block_);

  static bool   renderTerrainZodiacs(SceneRenderState*, TerrainBlock*, TerrCell*);
  static ShaderData* getTerrainZodiacShader();

  static void   renderPolysoupZodiacs(SceneRenderState*, TSStatic*);
  static ShaderData* getPolysoupZodiacShader();

  static void   renderGroundPlaneZodiacs(SceneRenderState*, GroundPlane*);
  static ShaderData* getGroundPlaneZodiacShader();

  static void   renderMeshRoadZodiacs(SceneRenderState*, MeshRoad*);
  static ShaderData* getMeshRoadZodiacShader();
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_ZODIAC_MGR_H_
