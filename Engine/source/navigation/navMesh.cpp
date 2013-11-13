//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
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
//-----------------------------------------------------------------------------

#include <stdio.h>

#include "navMesh.h"
#include <DetourDebugDraw.h>
#include <RecastDebugDraw.h>

#include "math/mathUtils.h"
#include "math/mRandom.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "console/typeValidators.h"

#include "scene/sceneRenderState.h"
#include "gfx/gfxDrawUtil.h"
#include "renderInstance/renderPassManager.h"
#include "gfx/primBuilder.h"

#include "core/stream/bitStream.h"
#include "math/mathIO.h"

extern bool gEditingMission;

IMPLEMENT_CO_NETOBJECT_V1(NavMesh);

const U32 NavMesh::mMaxVertsPerPoly = 3;

NavMesh::NavMesh()
{
   mTypeMask |= StaticShapeObjectType | MarkerObjectType;
   mFileName = StringTable->insert("");
   mNetFlags.clear(Ghostable);

   nm = NULL;

   dMemset(&cfg, 0, sizeof(cfg));
   mCellSize = mCellHeight = 0.2f;
   mWalkableHeight = 2.0f;
   mWalkableClimb = 0.3f;
   mWalkableRadius = 0.5f;
   mWalkableSlope = 40.0f;
   mBorderSize = 1;
   mDetailSampleDist = 6.0f;
   mDetailSampleMaxError = 1.0f;
   mMaxEdgeLen = 12;
   mMaxSimplificationError = 1.3f;
   mMinRegionArea = 8;
   mMergeRegionArea = 20;
   mTileSize = 10.0f;
   mMaxPolysPerTile = 128;

   mAlwaysRender = false;

   mBuilding = false;
}

NavMesh::~NavMesh()
{
   dtFreeNavMesh(nm);
   nm = NULL;
}

bool NavMesh::setProtectedDetailSampleDist(void *obj, const char *index, const char *data)
{
   F32 dist = dAtof(data);
   if(dist == 0.0f || dist >= 0.9f)
      return true;
   Con::errorf("NavMesh::detailSampleDist must be 0 or greater than 0.9!");
   return false;
}

bool NavMesh::setProtectedAlwaysRender(void *obj, const char *index, const char *data)
{
   NavMesh *mesh = static_cast<NavMesh*>(obj);
   bool always = dAtob(data);
   if(always)
   {
      if(!gEditingMission)
         mesh->mNetFlags.set(Ghostable);
   }
   else
   {
      if(!gEditingMission)
         mesh->mNetFlags.clear(Ghostable);
   }
   mesh->mAlwaysRender = always;
   mesh->setMaskBits(LoadFlag);
   return true;
}

FRangeValidator ValidCellSize(0.01f, 10.0f);
FRangeValidator ValidSlopeAngle(0.0f, 89.9f);
IRangeValidator PositiveInt(0, S32_MAX);
IRangeValidator NaturalNumber(1, S32_MAX);
FRangeValidator CornerAngle(0.0f, 90.0f);

void NavMesh::initPersistFields()
{
   addGroup("NavMesh Options");

   addField("fileName", TypeString, Offset(mFileName, NavMesh),
      "Name of the data file to store this navmesh in (relative to engine executable).");

   addFieldV("cellSize", TypeF32, Offset(mCellSize, NavMesh), &ValidCellSize,
      "Length/width of a voxel.");
   addFieldV("cellHeight", TypeF32, Offset(mCellHeight, NavMesh), &ValidCellSize,
      "Height of a voxel.");
   addFieldV("tileSize", TypeF32, Offset(mTileSize, NavMesh), &CommonValidators::PositiveNonZeroFloat,
      "The horizontal size of tiles.");

   addFieldV("actorHeight", TypeF32, Offset(mWalkableHeight, NavMesh), &CommonValidators::PositiveFloat,
      "Height of an actor.");
   addFieldV("actorClimb", TypeF32, Offset(mWalkableClimb, NavMesh), &CommonValidators::PositiveFloat,
      "Maximum climbing height of an actor.");
   addFieldV("actorRadius", TypeF32, Offset(mWalkableRadius, NavMesh), &CommonValidators::PositiveFloat,
      "Radius of an actor.");
   addFieldV("walkableSlope", TypeF32, Offset(mWalkableSlope, NavMesh), &ValidSlopeAngle,
      "Maximum walkable slope in degrees.");

   endGroup("NavMesh Options");

   addGroup("NavMesh Rendering");

   addProtectedField("alwaysRender", TypeBool, Offset(mAlwaysRender, NavMesh),
      &setProtectedAlwaysRender, &defaultProtectedGetFn,
      "Display this NavMesh even outside the editor.");

   endGroup("NavMesh Rendering");

   addGroup("NavMesh Advanced Options");

   addFieldV("borderSize", TypeS32, Offset(mBorderSize, NavMesh), &PositiveInt,
      "Size of the non-walkable border around the navigation mesh (in voxels).");
   addProtectedField("detailSampleDist", TypeF32, Offset(mDetailSampleDist, NavMesh),
      &setProtectedDetailSampleDist, &defaultProtectedGetFn,
      "Sets the sampling distance to use when generating the detail mesh.");
   addFieldV("detailSampleError", TypeF32, Offset(mDetailSampleMaxError, NavMesh), &CommonValidators::PositiveFloat,
      "The maximum distance the detail mesh surface should deviate from heightfield data.");
   addFieldV("maxEdgeLen", TypeS32, Offset(mDetailSampleDist, NavMesh), &PositiveInt,
      "The maximum allowed length for contour edges along the border of the mesh.");
   addFieldV("simplificationError", TypeF32, Offset(mMaxSimplificationError, NavMesh), &CommonValidators::PositiveFloat,
      "The maximum distance a simplfied contour's border edges should deviate from the original raw contour.");
   addFieldV("minRegionArea", TypeS32, Offset(mMinRegionArea, NavMesh), &PositiveInt,
      "The minimum number of cells allowed to form isolated island areas.");
   addFieldV("mergeRegionArea", TypeS32, Offset(mMergeRegionArea, NavMesh), &PositiveInt,
      "Any regions with a span count smaller than this value will, if possible, be merged with larger regions.");
   addFieldV("maxPolysPerTile", TypeS32, Offset(mMaxPolysPerTile, NavMesh), &NaturalNumber,
      "The maximum number of polygons allowed in a tile.");

   endGroup("NavMesh Advanced Options");

   Parent::initPersistFields();
}

bool NavMesh::onAdd()
{
   if(!Parent::onAdd())
      return false;

   mObjBox.set(Point3F(-10.0f, -10.0f, -1.0f),
      Point3F( 10.0f,  10.0f,  1.0f));
   resetWorldBox();

   addToScene();

   if(gEditingMission || mAlwaysRender)
   {
      mNetFlags.set(Ghostable);
      if(isClientObject())
         renderToDrawer();
   }

   if(isServerObject())
   {
      setProcessTick(true);
   }

   load();

   return true;
}

void NavMesh::onRemove()
{
   removeFromScene();

   Parent::onRemove();
}

void NavMesh::setTransform(const MatrixF &mat)
{
   Parent::setTransform(mat);
}

void NavMesh::setScale(const VectorF &scale)
{
   Parent::setScale(scale);
}

bool NavMesh::build(bool background, bool saveIntermediates)
{
   if(mBuilding)
      cancelBuild();

   mBuilding = true;

   dtFreeNavMesh(nm);
   // Allocate a new navmesh.
   nm = dtAllocNavMesh();
   if(!nm)
   {
      Con::errorf("Could not allocate dtNavMesh for NavMesh %s", getIdString());
      return false;
   }

   updateConfig();

   // Build navmesh parameters from console members.
   dtNavMeshParams params;
   rcVcopy(params.orig, cfg.bmin);
   params.tileWidth = cfg.tileSize * mCellSize;
   params.tileHeight = cfg.tileSize * mCellSize;
   params.maxTiles = mCeil(getWorldBox().len_x() / params.tileWidth) * mCeil(getWorldBox().len_y() / params.tileHeight);
   params.maxPolys = mMaxPolysPerTile;

   // Initialise our navmesh.
   if(dtStatusFailed(nm->init(&params)))
   {
      Con::errorf("Could not init dtNavMesh for NavMesh %s", getIdString());
      return false;
   }

   updateTiles(true);

   if(!background)
   {
      while(mDirtyTiles.size())
         buildNextTile();
   }

   return true;
}

DefineEngineMethod(NavMesh, build, bool, (bool background, bool save), (true, false),
   "@brief Create a Recast nav mesh.")
{
   return object->build(background, save);
}

void NavMesh::cancelBuild()
{
   while(mDirtyTiles.size()) mDirtyTiles.pop();
   mBuilding = false;
}

DefineEngineMethod(NavMesh, cancelBuild, void, (),,
   "@brief Cancel the current NavMesh build.")
{
   object->cancelBuild();
}

void NavMesh::inspectPostApply()
{
   if(mBuilding)
      cancelBuild();
}

void NavMesh::updateConfig()
{
   // Build rcConfig object from our console members.
   dMemset(&cfg, 0, sizeof(cfg));
   cfg.cs = mCellSize;
   cfg.ch = mCellHeight;
   Box3F box = DTStoRC(getWorldBox());
   rcVcopy(cfg.bmin, box.minExtents);
   rcVcopy(cfg.bmax, box.maxExtents);
   rcCalcGridSize(cfg.bmin, cfg.bmax, cfg.cs, &cfg.width, &cfg.height);

   cfg.walkableHeight = mCeil(mWalkableHeight / mCellHeight);
   cfg.walkableClimb = mCeil(mWalkableClimb / mCellHeight);
   cfg.walkableRadius = mCeil(mWalkableRadius / mCellSize);
   cfg.walkableSlopeAngle = mWalkableSlope;
   cfg.borderSize = cfg.walkableRadius + 3;

   cfg.detailSampleDist = mDetailSampleDist;
   cfg.detailSampleMaxError = mDetailSampleMaxError;
   cfg.maxEdgeLen = mMaxEdgeLen;
   cfg.maxSimplificationError = mMaxSimplificationError;
   cfg.maxVertsPerPoly = mMaxVertsPerPoly;
   cfg.minRegionArea = mMinRegionArea;
   cfg.mergeRegionArea = mMergeRegionArea;
   cfg.tileSize = mTileSize / cfg.cs;
}

S32 NavMesh::getTile(Point3F pos)
{
   if(mBuilding)
      return -1;
   for(U32 i = 0; i < mTiles.size(); i++)
   {
      if(mTiles[i].box.isContained(pos))
         return i;
   }
   return -1;
}

Box3F NavMesh::getTileBox(U32 id)
{
   if(mBuilding || id >= mTiles.size())
      return Box3F::Invalid;
   return mTiles[id].box;
}

void NavMesh::updateTiles(bool dirty)
{
   if(!isProperlyAdded())
      return;

   mTiles.clear();
   while(mDirtyTiles.size()) mDirtyTiles.pop();

   const Box3F &box = DTStoRC(getWorldBox());
   if(box.isEmpty())
      return;

   updateConfig();

   // Calculate tile dimensions.
   const U32 ts = cfg.tileSize;
   const U32 tw = (cfg.width  + ts-1) / ts;
   const U32 th = (cfg.height + ts-1) / ts;
   const F32 tcs = cfg.tileSize * cfg.cs;

   // Iterate over tiles.
   F32 tileBmin[3], tileBmax[3];
   for(U32 y = 0; y < th; ++y)
   {
      for(U32 x = 0; x < tw; ++x)
      {
         tileBmin[0] = cfg.bmin[0] + x*tcs;
         tileBmin[1] = cfg.bmin[1];
         tileBmin[2] = cfg.bmin[2] + y*tcs;

         tileBmax[0] = cfg.bmin[0] + (x+1)*tcs;
         tileBmax[1] = cfg.bmax[1];
         tileBmax[2] = cfg.bmin[2] + (y+1)*tcs;

         mTiles.push_back(
            Tile(RCtoDTS(tileBmin, tileBmax),
                  x, y,
                  tileBmin, tileBmax));

         if(dirty)
            mDirtyTiles.push(mTiles.size() - 1);
      }
   }
}

void NavMesh::processTick(const Move *move)
{
   buildNextTile();
}

void NavMesh::buildNextTile()
{
   if(mDirtyTiles.size())
   {
      // Pop a single dirty tile and process it.
      U32 i = mDirtyTiles.front();
      mDirtyTiles.pop();
      const Tile &tile = mTiles[i];
      // Intermediate data for tile build.
      TileData tempdata;
      // Generate navmesh for this tile.
      U32 dataSize = 0;
      unsigned char* data = buildTileData(tile, tempdata, dataSize);
      if(data)
      {
         // Remove any previous data.
         nm->removeTile(nm->getTileRefAt(tile.x, tile.y, 0), 0, 0);
         // Add new data (navmesh owns and deletes the data).
         dtStatus status = nm->addTile(data, dataSize, DT_TILE_FREE_DATA, 0, 0);
         int success = 1;
         if(dtStatusFailed(status))
         {
            success = 0;
            dtFree(data);
         }
      }
      // Did we just build the last tile?
      if(!mDirtyTiles.size())
      {
         mBuilding = false;
      }
      setMaskBits(BuildFlag);
   }
}

static void buildCallback(SceneObject* object,void *key)
{
   SceneContainer::CallbackInfo* info = reinterpret_cast<SceneContainer::CallbackInfo*>(key);
   object->buildPolyList(info->context,info->polyList,info->boundingBox,info->boundingSphere);
}

unsigned char *NavMesh::buildTileData(const Tile &tile, TileData &data, U32 &dataSize)
{
   // Push out tile boundaries a bit.
   F32 tileBmin[3], tileBmax[3];
   rcVcopy(tileBmin, tile.bmin);
   rcVcopy(tileBmax, tile.bmax);
   tileBmin[0] -= cfg.borderSize * cfg.cs;
   tileBmin[2] -= cfg.borderSize * cfg.cs;
   tileBmax[0] += cfg.borderSize * cfg.cs;
   tileBmax[2] += cfg.borderSize * cfg.cs;

   // Parse objects from level into RC-compatible format.
   Box3F box = RCtoDTS(tileBmin, tileBmax);
   SceneContainer::CallbackInfo info;
   info.context = PLC_Navigation;
   info.boundingBox = box;
   info.polyList = &data.geom;
   getContainer()->findObjects(box, StaticObjectType, buildCallback, &info);

   // Check for no geometry.
   if(!data.geom.getVertCount())
      return false;

   // Figure out voxel dimensions of this tile.
   U32 width = 0, height = 0;
   width = cfg.tileSize + cfg.borderSize * 2;
   height = cfg.tileSize + cfg.borderSize * 2;

   // Create a dummy context.
   rcContext ctx(false);

   // Create a heightfield to voxelise our input geometry.
   data.hf = rcAllocHeightfield();
   if(!data.hf)
   {
      Con::errorf("Out of memory (rcHeightField) for NavMesh %s", getIdString());
      return NULL;
   }
   if(!rcCreateHeightfield(&ctx, *data.hf, width, height, tileBmin, tileBmax, cfg.cs, cfg.ch))
   {
      Con::errorf("Could not generate rcHeightField for NavMesh %s", getIdString());
      return NULL;
   }

   unsigned char *areas = new unsigned char[data.geom.getTriCount()];
   if(!areas)
   {
      Con::errorf("Out of memory (area flags) for NavMesh %s", getIdString());
      return NULL;
   }
   dMemset(areas, 0, data.geom.getTriCount() * sizeof(unsigned char));

   // Filter triangles by angle and rasterize.
   rcMarkWalkableTriangles(&ctx, cfg.walkableSlopeAngle,
      data.geom.getVerts(), data.geom.getVertCount(),
      data.geom.getTris(), data.geom.getTriCount(), areas);
   rcRasterizeTriangles(&ctx, data.geom.getVerts(), data.geom.getVertCount(),
      data.geom.getTris(), areas, data.geom.getTriCount(),
      *data.hf, cfg.walkableClimb);

   delete[] areas;

   // Filter out areas with low ceilings and other stuff.
   rcFilterLowHangingWalkableObstacles(&ctx, cfg.walkableClimb, *data.hf);
   rcFilterLedgeSpans(&ctx, cfg.walkableHeight, cfg.walkableClimb, *data.hf);
   rcFilterWalkableLowHeightSpans(&ctx, cfg.walkableHeight, *data.hf);

   data.chf = rcAllocCompactHeightfield();
   if(!data.chf)
   {
      Con::errorf("Out of memory (rcCompactHeightField) for NavMesh %s", getIdString());
      return NULL;
   }
   if(!rcBuildCompactHeightfield(&ctx, cfg.walkableHeight, cfg.walkableClimb, *data.hf, *data.chf))
   {
      Con::errorf("Could not generate rcCompactHeightField for NavMesh %s", getIdString());
      return NULL;
   }
   if(!rcErodeWalkableArea(&ctx, cfg.walkableRadius, *data.chf))
   {
      Con::errorf("Could not erode walkable area for NavMesh %s", getIdString());
      return NULL;
   }

   //--------------------------
   // Todo: mark areas here.
   //const ConvexVolume* vols = m_geom->getConvexVolumes();
   //for (int i  = 0; i < m_geom->getConvexVolumeCount(); ++i)
      //rcMarkConvexPolyArea(m_NULL, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char)vols[i].area, *m_chf);
   //--------------------------

   if(false)
   {
      if(!rcBuildRegionsMonotone(&ctx, *data.chf, cfg.borderSize, cfg.minRegionArea, cfg.mergeRegionArea))
      {
         Con::errorf("Could not build regions for NavMesh %s", getIdString());
         return NULL;
      }
   }
   else
   {
      if(!rcBuildDistanceField(&ctx, *data.chf))
      {
         Con::errorf("Could not build distance field for NavMesh %s", getIdString());
         return NULL;
      }
      if(!rcBuildRegions(&ctx, *data.chf, cfg.borderSize, cfg.minRegionArea, cfg.mergeRegionArea))
      {
         Con::errorf("Could not build regions for NavMesh %s", getIdString());
         return NULL;
      }
   }

   data.cs = rcAllocContourSet();
   if(!data.cs)
   {
      Con::errorf("Out of memory (rcContourSet) for NavMesh %s", getIdString());
      return NULL;
   }
   if(!rcBuildContours(&ctx, *data.chf, cfg.maxSimplificationError, cfg.maxEdgeLen, *data.cs))
   {
      Con::errorf("Could not construct rcContourSet for NavMesh %s", getIdString());
      return NULL;
   }
   if(data.cs->nconts <= 0)
   {
      Con::errorf("No contours in rcContourSet for NavMesh %s", getIdString());
      return NULL;
   }

   data.pm = rcAllocPolyMesh();
   if(!data.pm)
   {
      Con::errorf("Out of memory (rcPolyMesh) for NavMesh %s", getIdString());
      return NULL;
   }
   if(!rcBuildPolyMesh(&ctx, *data.cs, cfg.maxVertsPerPoly, *data.pm))
   {
      Con::errorf("Could not construct rcPolyMesh for NavMesh %s", getIdString());
      return NULL;
   }

   data.pmd = rcAllocPolyMeshDetail();
   if(!data.pmd)
   {
      Con::errorf("Out of memory (rcPolyMeshDetail) for NavMesh %s", getIdString());
      return NULL;
   }
   if(!rcBuildPolyMeshDetail(&ctx, *data.pm, *data.chf, cfg.detailSampleDist, cfg.detailSampleMaxError, *data.pmd))
   {
      Con::errorf("Could not construct rcPolyMeshDetail for NavMesh %s", getIdString());
      return NULL;
   }

   if(data.pm->nverts >= 0xffff)
   {
      Con::errorf("Too many vertices in rcPolyMesh for NavMesh %s", getIdString());
      return NULL;
   }
   for(U32 i = 0; i < data.pm->npolys; i++)
   {
      if(data.pm->areas[i] == RC_WALKABLE_AREA)
         data.pm->areas[i] = GroundArea;

      if(data.pm->areas[i] == GroundArea)
         data.pm->flags[i] |= WalkFlag;
      if(data.pm->areas[i] == WaterArea)
         data.pm->flags[i] |= SwimFlag;
   }

   unsigned char* navData = 0;
   int navDataSize = 0;

   dtNavMeshCreateParams params;
   dMemset(&params, 0, sizeof(params));

   params.verts = data.pm->verts;
   params.vertCount = data.pm->nverts;
   params.polys = data.pm->polys;
   params.polyAreas = data.pm->areas;
   params.polyFlags = data.pm->flags;
   params.polyCount = data.pm->npolys;
   params.nvp = data.pm->nvp;

   params.detailMeshes = data.pmd->meshes;
   params.detailVerts = data.pmd->verts;
   params.detailVertsCount = data.pmd->nverts;
   params.detailTris = data.pmd->tris;
   params.detailTriCount = data.pmd->ntris;

   params.walkableHeight = mWalkableHeight;
   params.walkableRadius = mWalkableRadius;
   params.walkableClimb = mWalkableClimb;
   params.tileX = tile.x;
   params.tileY = tile.y;
   params.tileLayer = 0;
   rcVcopy(params.bmin, data.pm->bmin);
   rcVcopy(params.bmax, data.pm->bmax);
   params.cs = cfg.cs;
   params.ch = cfg.ch;
   params.buildBvTree = true;

   if(!dtCreateNavMeshData(&params, &navData, &navDataSize))
   {
      Con::errorf("Could not create dtNavMeshData for tile (%d, %d) of NavMesh %s",
         tile.x, tile.y, getIdString());
      return NULL;
   }

   dataSize = navDataSize;

   return navData;
}

/// This method should never be called in a separate thread to the rendering
/// or pathfinding logic. It directly replaces data in the dtNavMesh for
/// this NavMesh object.
void NavMesh::buildTiles(const Box3F &box)
{
   // Make sure we've already built or loaded.
   if(!nm)
      return;
   // Iterate over tiles.
   for(U32 i = 0; i < mTiles.size(); i++)
   {
      const Tile &tile = mTiles[i];
      // Check tile box.
      if(!tile.box.isOverlapped(box))
         continue;
      // Mark as dirty.
      mDirtyTiles.push(i);
   }
}

DefineEngineMethod(NavMesh, buildTiles, void, (Box3F box),,
   "@brief Rebuild the tiles overlapped by the input box.")
{
   return object->buildTiles(box);
}

void NavMesh::buildTile(const U32 &tile)
{
   if(tile < mTiles.size())
   {
      mDirtyTiles.push(tile);
   }
}

void NavMesh::renderToDrawer()
{
   dd.clear();
   // Recast debug draw
   NetObject *no = getServerObject();
   if(no)
   {
      NavMesh *n = static_cast<NavMesh*>(no);

      if(n->nm)
      {
         dd.beginGroup(0);
         duDebugDrawNavMesh       (&dd, *n->nm, 0);
         dd.beginGroup(1);
         duDebugDrawNavMeshPortals(&dd, *n->nm);
         dd.beginGroup(2);
         duDebugDrawNavMeshBVTree (&dd, *n->nm);
      }
   }
}

void NavMesh::prepRenderImage(SceneRenderState *state)
{
   ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
   ri->renderDelegate.bind(this, &NavMesh::render);
   ri->type = RenderPassManager::RIT_Object;
   ri->translucentSort = true;
   ri->defaultKey = 1;
   state->getRenderPass()->addInst(ri);
}

void NavMesh::render(ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat)
{
   if(overrideMat)
      return;

   if(state->isReflectPass())
      return;

   PROFILE_SCOPE(NavMesh_Render);
   
   // Recast debug draw
   NetObject *no = getServerObject();
   if(no)
   {
      NavMesh *n = static_cast<NavMesh*>(no);

      if(n->isSelected())
      {
         GFXDrawUtil *drawer = GFX->getDrawUtil();

         GFXStateBlockDesc desc;
         desc.setZReadWrite(true, false);
         desc.setBlend(true);
         desc.setCullMode(GFXCullNone);

         drawer->drawCube(desc, getWorldBox(), n->mBuilding
            ? ColorI(255, 0, 0, 80)
            : ColorI(136, 228, 255, 45));
         desc.setFillModeWireframe();
         drawer->drawCube(desc, getWorldBox(), ColorI::BLACK);
      }

      if(n->mBuilding)
      {
         int alpha = 80;
         if(!n->isSelected() || !Con::getBoolVariable("$Nav::EditorOpen"))
            alpha = 20;
         dd.overrideColor(duRGBA(255, 0, 0, alpha));
      }
      else
      {
         dd.cancelOverride();
      }
      
      if((!gEditingMission && n->mAlwaysRender) || (gEditingMission && Con::getBoolVariable("$Nav::Editor::renderMesh", 1))) dd.renderGroup(0);
      if(Con::getBoolVariable("$Nav::Editor::renderPortals")) dd.renderGroup(1);
      if(Con::getBoolVariable("$Nav::Editor::renderBVTree"))  dd.renderGroup(2);
   }
}

void NavMesh::onEditorEnable()
{
   mNetFlags.set(Ghostable);
   if(isClientObject() && !mAlwaysRender)
      addToScene();
}

void NavMesh::onEditorDisable()
{
   if(!mAlwaysRender)
   {
      mNetFlags.clear(Ghostable);
      if(isClientObject())
         removeFromScene();
   }
}

U32 NavMesh::packUpdate(NetConnection *conn, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(conn, mask, stream);

   mathWrite(*stream, getTransform());
   mathWrite(*stream, getScale());
   stream->writeFlag(mAlwaysRender);

   return retMask;
}

void NavMesh::unpackUpdate(NetConnection *conn, BitStream *stream)
{
   Parent::unpackUpdate(conn, stream);

   mathRead(*stream, &mObjToWorld);
   mathRead(*stream, &mObjScale);
   mAlwaysRender = stream->readFlag();

   setTransform(mObjToWorld);

   renderToDrawer();
}

static const int NAVMESHSET_MAGIC = 'M'<<24 | 'S'<<16 | 'E'<<8 | 'T'; //'MSET';
static const int NAVMESHSET_VERSION = 1;

struct NavMeshSetHeader
{
   int magic;
   int version;
   int numTiles;
   dtNavMeshParams params;
};

struct NavMeshTileHeader
{
   dtTileRef tileRef;
   int dataSize;
};

bool NavMesh::load()
{
   if(!dStrlen(mFileName))
      return false;

   FILE* fp = fopen(mFileName, "rb");
   if(!fp)
      return false;

   // Read header.
   NavMeshSetHeader header;
   fread(&header, sizeof(NavMeshSetHeader), 1, fp);
   if(header.magic != NAVMESHSET_MAGIC)
   {
      fclose(fp);
      return 0;
   }
   if(header.version != NAVMESHSET_VERSION)
   {
      fclose(fp);
      return 0;
   }

   if(nm)
      dtFreeNavMesh(nm);
   nm = dtAllocNavMesh();
   if(!nm)
   {
      fclose(fp);
      return false;
   }

   dtStatus status = nm->init(&header.params);
   if(dtStatusFailed(status))
   {
      fclose(fp);
      return false;
   }

   // Read tiles.
   for(U32 i = 0; i < header.numTiles; ++i)
   {
      NavMeshTileHeader tileHeader;
      fread(&tileHeader, sizeof(tileHeader), 1, fp);
      if(!tileHeader.tileRef || !tileHeader.dataSize)
         break;

      unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
      if(!data) break;
      memset(data, 0, tileHeader.dataSize);
      fread(data, tileHeader.dataSize, 1, fp);

      nm->addTile(data, tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
   }

   fclose(fp);

   updateTiles();

   if(isServerObject())
   {
      setMaskBits(LoadFlag);
   }

   return true;
}

DefineEngineMethod(NavMesh, load, bool, (),,
   "@brief Load this NavMesh from its file.")
{
   return object->load();
}

bool NavMesh::save()
{
   if(!dStrlen(mFileName) || !nm)
      return false;

   // Save our navmesh into a file to load from next time
   FILE* fp = fopen(mFileName, "wb");
   if(!fp)
      return false;

   // Store header.
   NavMeshSetHeader header;
   header.magic = NAVMESHSET_MAGIC;
   header.version = NAVMESHSET_VERSION;
   header.numTiles = 0;
   for(U32 i = 0; i < nm->getMaxTiles(); ++i)
   {
      const dtMeshTile* tile = ((const dtNavMesh*)nm)->getTile(i);
      if (!tile || !tile->header || !tile->dataSize) continue;
      header.numTiles++;
   }
   memcpy(&header.params, nm->getParams(), sizeof(dtNavMeshParams));
   fwrite(&header, sizeof(NavMeshSetHeader), 1, fp);

   // Store tiles.
   for(U32 i = 0; i < nm->getMaxTiles(); ++i)
   {
      const dtMeshTile* tile = ((const dtNavMesh*)nm)->getTile(i);
      if(!tile || !tile->header || !tile->dataSize) continue;

      NavMeshTileHeader tileHeader;
      tileHeader.tileRef = nm->getTileRef(tile);
      tileHeader.dataSize = tile->dataSize;
      fwrite(&tileHeader, sizeof(tileHeader), 1, fp);

      fwrite(tile->data, tile->dataSize, 1, fp);
   }

   fclose(fp);

   return true;
}

DefineEngineMethod(NavMesh, save, void, (),,
   "@brief Save this NavMesh to its file.")
{
   object->save();
}

void NavMesh::write(Stream &stream, U32 tabStop, U32 flags)
{
   save();
   Parent::write(stream, tabStop, flags);
}
