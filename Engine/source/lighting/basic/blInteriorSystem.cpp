//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
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

#include "lighting/basic/blInteriorSystem.h"
#include "lighting/lightingInterfaces.h"
#include "lighting/common/shadowVolumeBSP.h"
#include "interior/interiorInstance.h"
#include "lighting/common/sceneLightingGlobals.h"
#include "lighting/basic/basicLightManager.h"
#include "gfx/bitmap/gBitmap.h"

//#define SET_COLORS


bool blInteriorSystem::smUseVertexLighting = false;


//------------------------------------------------------------------------------
// Class SceneLighting::PersistInfo::InteriorChunk
//------------------------------------------------------------------------------
struct blInteriorChunk : public PersistInfo::PersistChunk
{
   typedef PersistChunk Parent;

   blInteriorChunk();
   ~blInteriorChunk();

   Vector<GBitmap*>     sgNormalLightMaps;

   Vector<U32>          mDetailLightmapCount;
   Vector<U32>          mDetailLightmapIndices;
   Vector<GBitmap*>     mLightmaps;

   bool                 mHasAlarmState;
   Vector<U32>          mDetailVertexCount;
   Vector<ColorI>       mVertexColorsNormal;
   Vector<ColorI>       mVertexColorsAlarm;

   bool read(Stream &);
   bool write(Stream &);
};

blInteriorChunk::blInteriorChunk()
{
   mChunkType = PersistChunk::InteriorChunkType;
}

blInteriorChunk::~blInteriorChunk()
{
   for(U32 i = 0; i < mLightmaps.size(); i++)
      delete mLightmaps[i];
}

//------------------------------------------------------------------------------
// - always read in vertex lighting, lightmaps may not be needed
bool blInteriorChunk::read(Stream & stream)
{
   if(!Parent::read(stream))
      return(false);

   U32 size;
   U32 i;

   // lightmaps->vertex-info
   // BTRTODO: FIX ME
   if (true)
   //if(!SceneLighting::smUseVertexLighting)
   {
      // size of this minichunk
      if(!stream.read(&size))
         return(false);

      // lightmaps
      stream.read(&size);
      mDetailLightmapCount.setSize(size);
      for(i = 0; i < size; i++)
         if(!stream.read(&mDetailLightmapCount[i]))
            return(false);

      stream.read(&size);
      mDetailLightmapIndices.setSize(size);
      for(i = 0; i < size; i++)
         if(!stream.read(&mDetailLightmapIndices[i]))
            return(false);

      if(!stream.read(&size))
         return(false);
      mLightmaps.setSize(size);

      for(i = 0; i < size; i++)
      {
         mLightmaps[i] = new GBitmap;
         if(!mLightmaps[i]->readBitmap("png",stream))
            return(false);
      }
   }
   else
   {
      // step past the lightmaps
      if(!stream.read(&size))
         return(false);
      if(!stream.setPosition(stream.getPosition() + size))
         return(false);
   }

   // size of the vertex lighting: need to reset stream position after zipStream reading
   U32 zipStreamEnd;
   if(!stream.read(&zipStreamEnd))
      return(false);
   zipStreamEnd += stream.getPosition();

   /*
   // vertex lighting
   ZipSubRStream zipStream;
   if(!zipStream.attachStream(&stream))
   return(false);

   if(!zipStream.read(&size))
   return(false);
   mHasAlarmState = bool(size);

   if(!zipStream.read(&size))
   return(false);
   mDetailVertexCount.setSize(size);
   for(i = 0; i < size; i++)
   if(!zipStream.read(&mDetailVertexCount[i]))
   return(false);

   size = 0;
   for(i = 0; i < mDetailVertexCount.size(); i++)
   size += mDetailVertexCount[i];

   mVertexColorsNormal.setSize(size);

   if(mHasAlarmState)
   mVertexColorsAlarm.setSize(size);

   U32 curPos = 0;
   for(i = 0; i < mDetailVertexCount.size(); i++)
   {
   U32 count = mDetailVertexCount[i];
   for(U32 j = 0; j < count; j++)
   if(!zipStream.read(&mVertexColorsNormal[curPos + j]))
   return(false);

   // read in the alarm info
   if(mHasAlarmState)
   {
   // same?
   if(!zipStream.read(&size))
   return(false);
   if(bool(size))
   dMemcpy(&mVertexColorsAlarm[curPos], &mVertexColorsNormal[curPos], count * sizeof(ColorI));
   else
   {
   for(U32 j = 0; j < count; j++)
   if(!zipStream.read(&mVertexColorsAlarm[curPos + j]))
   return(false);
   }
   }

   curPos += count;
   }

   zipStream.detachStream();
   */

   // since there is no resizeFilterStream the zipStream happily reads
   // off the end of the compressed block... reset the position
   stream.setPosition(zipStreamEnd);

   return(true);
}

bool blInteriorChunk::write(Stream & stream)
{
   if(!Parent::write(stream))
      return(false);

   // lightmaps
   U32 startPos = stream.getPosition();
   if(!stream.write(U32(0)))
      return(false);

   U32 i;
   if(!stream.write(U32(mDetailLightmapCount.size())))
      return(false);
   for(i = 0; i < mDetailLightmapCount.size(); i++)
      if(!stream.write(mDetailLightmapCount[i]))
         return(false);

   if(!stream.write(U32(mDetailLightmapIndices.size())))
      return(false);
   for(i = 0; i < mDetailLightmapIndices.size(); i++)
      if(!stream.write(mDetailLightmapIndices[i]))
         return(false);

   if(!stream.write(U32(mLightmaps.size())))
      return(false);
   for(i = 0; i < mLightmaps.size(); i++)
   {
      AssertFatal(mLightmaps[i], "SceneLighting::blInteriorChunk::Write: Invalid bitmap!");
      if(!mLightmaps[i]->writeBitmap("png",stream))
         return(false);
   }

   // write out the lightmap portions size
   U32 endPos = stream.getPosition();
   if(!stream.setPosition(startPos))
      return(false);

   // don't include the offset in the size
   if(!stream.write(U32(endPos - startPos - sizeof(U32))))
      return(false);
   if(!stream.setPosition(endPos))
      return(false);


   // vertex lighting: needs the size of the vertex info because the
   // zip stream may read off the end of the chunk
   startPos = stream.getPosition();
   if(!stream.write(U32(0)))
      return(false);

   // write out the vertex lighting portions size
   endPos = stream.getPosition();
   if(!stream.setPosition(startPos))
      return(false);

   // don't include the offset in the size
   if(!stream.write(U32(endPos - startPos - sizeof(U32))))
      return(false);
   if(!stream.setPosition(endPos))
      return(false);

   return(true);
}

//
// InteriorProxy (definition)
//
class blInteriorProxy : public SceneLighting::ObjectProxy
{
private:
   typedef  ObjectProxy       Parent;

   bool isShadowedBy(blInteriorProxy *);
   ShadowVolumeBSP::SVPoly * buildInteriorPoly(ShadowVolumeBSP * shadowVolumeBSP,
      Interior * detail, U32 surfaceIndex, LightInfo * light,
      bool createSurfaceInfo);
public:

   blInteriorProxy(SceneObject * obj);
   ~blInteriorProxy();
   InteriorInstance * operator->() {return(static_cast<InteriorInstance*>(static_cast<SceneObject*>(mObj)));}
   InteriorInstance * getObject() {return(static_cast<InteriorInstance*>(static_cast<SceneObject*>(mObj)));}

   // current light info
   ShadowVolumeBSP *                   mBoxShadowBSP;
   Vector<ShadowVolumeBSP::SVPoly*>    mLitBoxSurfaces;
   Vector<PlaneF>                      mOppositeBoxPlanes;
   Vector<PlaneF>                      mTerrainTestPlanes;


   struct sgSurfaceInfo
   {
      const Interior::Surface *sgSurface;
      U32 sgIndex;
      Interior *sgDetail;
      bool sgHasAlarm;
   };
   U32 sgCurrentSurfaceIndex;
   U32 sgSurfacesPerPass;
   InteriorInstance *sgInterior;
   Vector<LightInfo *> sgLights;
   Vector<sgSurfaceInfo> sgSurfaces;

   void sgAddLight(LightInfo *light, InteriorInstance *interior);
   //void sgLightUniversalPoint(LightInfo *light);
   void sgProcessSurface(const Interior::Surface &surface, U32 i, Interior *detail, bool hasAlarm);


   // lighting interface
   bool loadResources();
   void init();
   //bool tgePreLight(LightInfo* light);
   bool preLight(LightInfo *);
   void light(LightInfo *);
   void postLight(bool lastLight);

   //virtual void processLightingStart();
   //virtual bool processStartObjectLightingEvent(SceneLighting::ObjectProxy* objproxy, U32 current, U32 max);
   //virtual void processTGELightProcessEvent(U32 curr, U32 max, LightInfo*);

   virtual bool supportsShadowVolume();
   virtual void getClipPlanes(Vector<PlaneF>& planes);
   virtual void addToShadowVolume(ShadowVolumeBSP * shadowVolume, LightInfo * light, S32 level);

   // persist
   U32 getResourceCRC();
   bool setPersistInfo(PersistInfo::PersistChunk *);
   bool getPersistInfo(PersistInfo::PersistChunk *);
};

//------------------------------------------------------------------------------
// Class SceneLighting::InteriorProxy:
//------------------------------------------------------------------------------
blInteriorProxy::blInteriorProxy(SceneObject * obj) :
Parent(obj)
{
   mBoxShadowBSP = 0;
}

blInteriorProxy::~blInteriorProxy()
{
   delete mBoxShadowBSP;
}

bool blInteriorProxy::loadResources()
{
   InteriorInstance * interior = getObject();
   if(!interior)
      return(false);

   Resource<InteriorResource> & interiorRes = interior->getResource();
   if(!bool(interiorRes))
      return(false);

   return(true);
}

void blInteriorProxy::init()
{
   InteriorInstance * interior = getObject();
   if(!interior)
      return;
}

bool blInteriorProxy::supportsShadowVolume()
{
   return true;
}

void blInteriorProxy::getClipPlanes(Vector<PlaneF>& planes)
{
   for(U32 i = 0; i < mLitBoxSurfaces.size(); i++)
      planes.push_back(mLitBoxSurfaces[i]->mPlane);   
}

ShadowVolumeBSP::SVPoly * blInteriorProxy::buildInteriorPoly(ShadowVolumeBSP * shadowVolumeBSP,
                                                           Interior * detail, U32 surfaceIndex, LightInfo * light,
                                                           bool createSurfaceInfo)
{
   InteriorInstance* interior = dynamic_cast<InteriorInstance*>(getObject());
   if (!interior)
      return NULL;

   // transform and add the points...
   const MatrixF & transform = interior->getTransform();
   const VectorF & scale = interior->getScale();

   const Interior::Surface & surface = detail->mSurfaces[surfaceIndex];

   ShadowVolumeBSP::SVPoly * poly = shadowVolumeBSP->createPoly();

   poly->mWindingCount = surface.windingCount;

   // project these points
   for(U32 j = 0; j < poly->mWindingCount; j++)
   {
      Point3F iPnt = detail->mPoints[detail->mWindings[surface.windingStart + j]].point;
      Point3F tPnt;
      iPnt.convolve(scale);
      transform.mulP(iPnt, &tPnt);
      poly->mWinding[j] = tPnt;
   }

   // convert from fan
   U32 tmpIndices[ShadowVolumeBSP::SVPoly::MaxWinding];
   Point3F fanIndices[ShadowVolumeBSP::SVPoly::MaxWinding];

   tmpIndices[0] = 0;

   U32 idx = 1;
   U32 i;
   for(i = 1; i < poly->mWindingCount; i += 2)
      tmpIndices[idx++] = i;
   for(i = ((poly->mWindingCount - 1) & (~0x1)); i > 0; i -= 2)
      tmpIndices[idx++] = i;

   idx = 0;
   for(i = 0; i < poly->mWindingCount; i++)
      if(surface.fanMask & (1 << i))
         fanIndices[idx++] = poly->mWinding[tmpIndices[i]];

   // set the data
   poly->mWindingCount = idx;
   for(i = 0; i < poly->mWindingCount; i++)
      poly->mWinding[i] = fanIndices[i];

   // flip the plane - shadow volumes face inwards
   PlaneF plane = detail->getPlane(surface.planeIndex);
   if(!Interior::planeIsFlipped(surface.planeIndex))
      plane.neg();

   // transform the plane
   mTransformPlane(transform, scale, plane, &poly->mPlane);
   shadowVolumeBSP->buildPolyVolume(poly, light);

   // do surface info?
   if(createSurfaceInfo)
   {
      ShadowVolumeBSP::SurfaceInfo * surfaceInfo = new ShadowVolumeBSP::SurfaceInfo;
      shadowVolumeBSP->mSurfaces.push_back(surfaceInfo);

      // fill it
      surfaceInfo->mSurfaceIndex = surfaceIndex;
      surfaceInfo->mShadowVolume = shadowVolumeBSP->getShadowVolume(poly->mShadowVolume);

      // POLY and POLY node gets it too
      ShadowVolumeBSP::SVNode * traverse = shadowVolumeBSP->getShadowVolume(poly->mShadowVolume);
      while(traverse->mFront)
      {
         traverse->mSurfaceInfo = surfaceInfo;
         traverse = traverse->mFront;
      }

      // get some info from the poly node
      poly->mSurfaceInfo = traverse->mSurfaceInfo = surfaceInfo;
      surfaceInfo->mPlaneIndex = traverse->mPlaneIndex;
   }

   return(poly);
}


void blInteriorProxy::addToShadowVolume(ShadowVolumeBSP * shadowVolume, LightInfo * light, S32 level)
{
   if(light->getType() != LightInfo::Vector)
      return;

   ColorF ambient = light->getAmbient();

   bool shadowedTree = true;

   InteriorInstance* interior = dynamic_cast<InteriorInstance*>(getObject());
   if (!interior)
      return;
   Resource<InteriorResource> mInteriorRes = interior->getResource();

   // check if just getting shadow detail
   if(level == SceneLighting::SHADOW_DETAIL)
   {
      shadowedTree = false;
      level = mInteriorRes->getNumDetailLevels() - 1;
   }

   Interior * detail = mInteriorRes->getDetailLevel(level);
   bool hasAlarm = detail->hasAlarmState();

   // make sure surfaces do not get processed more than once
   BitVector surfaceProcessed;
   surfaceProcessed.setSize(detail->mSurfaces.size());
   surfaceProcessed.clear();

   ColorI color = light->getAmbient();

   // go through the zones of the interior and grab outside visible surfaces
   for(U32 i = 0; i < detail->getNumZones(); i++)
   {
      Interior::Zone & zone = detail->mZones[i];
      for(U32 j = 0; j < zone.surfaceCount; j++)
      {
         U32 surfaceIndex = detail->mZoneSurfaces[zone.surfaceStart + j];

         // dont reprocess a surface
         if(surfaceProcessed.test(surfaceIndex))
            continue;
         surfaceProcessed.set(surfaceIndex);

         Interior::Surface & surface = detail->mSurfaces[surfaceIndex];

         // outside visible?
         if(!(surface.surfaceFlags & Interior::SurfaceOutsideVisible))
            continue;

         // good surface?
         PlaneF plane = detail->getPlane(surface.planeIndex);
         if(Interior::planeIsFlipped(surface.planeIndex))
            plane.neg();

         // project the plane
         PlaneF projPlane;
         mTransformPlane(interior->getTransform(), interior->getScale(), plane, &projPlane);

         // fill with ambient? (need to do here, because surface will not be
         // added to the SVBSP tree)
         F32 dot = mDot(projPlane, light->getDirection());
         if(dot > -gParellelVectorThresh)// && !(GFX->getPixelShaderVersion() > 0.0) )
         {
            if(shadowedTree)
            {
               // alarm lighting
               GFXTexHandle normHandle = gInteriorLMManager.duplicateBaseLightmap(detail->getLMHandle(), interior->getLMHandle(), detail->getNormalLMapIndex(surfaceIndex));
               GFXTexHandle alarmHandle;

               GBitmap * normLightmap = normHandle->getBitmap();
               GBitmap * alarmLightmap = 0;

               // check if they share the lightmap
               if(hasAlarm)
               {
                  if(detail->getNormalLMapIndex(surfaceIndex) != detail->getAlarmLMapIndex(surfaceIndex))
                  {
                     alarmHandle = gInteriorLMManager.duplicateBaseLightmap(detail->getLMHandle(), interior->getLMHandle(), detail->getAlarmLMapIndex(surfaceIndex));
                     alarmLightmap = alarmHandle->getBitmap();
                  }
               }

               //
               // Support for interior light map border sizes.
               //
               U32 xlen, ylen, xoff, yoff;
               U32 lmborder = detail->getLightMapBorderSize();
               xlen = surface.mapSizeX + (lmborder * 2);
               ylen = surface.mapSizeY + (lmborder * 2);
               xoff = surface.mapOffsetX - lmborder;
               yoff = surface.mapOffsetY - lmborder;

               // attemp to light normal and alarm lighting
               for(U32 c = 0; c < 2; c++)
               {
                  GBitmap * lightmap = (c == 0) ? normLightmap : alarmLightmap;
                  if(!lightmap)
                     continue;

                  // fill it
                  for(U32 y = 0; y < ylen; y++)
                  {
                     for(U32 x = 0; x < xlen; x++)
                     {
                        ColorI outColor(255, 0, 0, 255);

#ifndef SET_COLORS
                        ColorI lmColor(0, 0, 0, 255);
                        lightmap->getColor(xoff + x, yoff + y, lmColor);

                        U32 _r = static_cast<U32>( color.red ) + static_cast<U32>( lmColor.red );
                        U32 _g = static_cast<U32>( color.green ) + static_cast<U32>( lmColor.green );
                        U32 _b = static_cast<U32>( color.blue ) + static_cast<U32>( lmColor.blue );

                        outColor.red   = mClamp(_r, 0, 255);
                        outColor.green = mClamp(_g, 0, 255);
                        outColor.blue  = mClamp(_b, 0, 255);
#endif

                        lightmap->setColor(xoff + x, yoff + y, outColor);
                     }
                  }
               }
            }
            continue;
         }

         ShadowVolumeBSP::SVPoly * poly = buildInteriorPoly(shadowVolume, detail,
            surfaceIndex, light, shadowedTree);

         // insert it into the SVBSP tree
         shadowVolume->insertPoly(poly);
      }
   }
}

bool blInteriorProxy::preLight(LightInfo * light)
{
   // create shadow volume of the bounding box of this object
   InteriorInstance * interior = getObject();
   if(!interior)
      return(false);

   if(light->getType() != LightInfo::Vector)
      return(false);

   // reset
   mLitBoxSurfaces.clear();

   const Box3F & objBox = interior->getObjBox();
   const MatrixF & objTransform = interior->getTransform();
   const VectorF & objScale = interior->getScale();

   // grab the surfaces which form the shadow volume
   U32 numPlanes = 0;
   PlaneF testPlanes[3];
   U32 planeIndices[3];

   // grab the bounding planes which face the light
   U32 i;
   for(i = 0; (i < 6) && (numPlanes < 3); i++)
   {
      PlaneF plane;
      plane.x = BoxNormals[i].x;
      plane.y = BoxNormals[i].y;
      plane.z = BoxNormals[i].z;

      if(i&1)
         plane.d = (((const float*)objBox.minExtents)[(i-1)>>1]);
      else
         plane.d = -(((const float*)objBox.maxExtents)[i>>1]);

      // project
      mTransformPlane(objTransform, objScale, plane, &testPlanes[numPlanes]);

      planeIndices[numPlanes] = i;

      if(mDot(testPlanes[numPlanes], light->getDirection()) < gParellelVectorThresh)
         numPlanes++;
   }
   AssertFatal(numPlanes, "blInteriorProxy::preLight: no planes found");

   // project the points
   Point3F projPnts[8];
   for(i = 0; i < 8; i++)
   {
      Point3F pnt;
      pnt.set(BoxPnts[i].x ? objBox.maxExtents.x : objBox.minExtents.x,
         BoxPnts[i].y ? objBox.maxExtents.y : objBox.minExtents.y,
         BoxPnts[i].z ? objBox.maxExtents.z : objBox.minExtents.z);

      // scale it
      pnt.convolve(objScale);
      objTransform.mulP(pnt, &projPnts[i]);
   }

   mBoxShadowBSP = new ShadowVolumeBSP;

   // insert the shadow volumes for the surfaces
   for(i = 0; i < numPlanes; i++)
   {
      ShadowVolumeBSP::SVPoly * poly = mBoxShadowBSP->createPoly();
      poly->mWindingCount = 4;

      U32 j;
      for(j = 0; j < 4; j++)
         poly->mWinding[j] = projPnts[BoxVerts[planeIndices[i]][j]];

      testPlanes[i].neg();
      poly->mPlane = testPlanes[i];

      mBoxShadowBSP->buildPolyVolume(poly, light);
      mLitBoxSurfaces.push_back(mBoxShadowBSP->copyPoly(poly));
      mBoxShadowBSP->insertPoly(poly);

      // create the opposite planes for testing against terrain
      Point3F pnts[3];
      for(j = 0; j < 3; j++)
         pnts[j] = projPnts[BoxVerts[planeIndices[i]^1][j]];
      PlaneF plane(pnts[2], pnts[1], pnts[0]);
      mOppositeBoxPlanes.push_back(plane);
   }

   // grab the unique planes for terrain checks
   for(i = 0; i < numPlanes; i++)
   {
      U32 mask = 0;
      for(U32 j = 0; j < numPlanes; j++)
         mask |= BoxSharedEdgeMask[planeIndices[i]][planeIndices[j]];

      ShadowVolumeBSP::SVNode * traverse = mBoxShadowBSP->getShadowVolume(mLitBoxSurfaces[i]->mShadowVolume);
      while(traverse->mFront)
      {
         if(!(mask & 1))
            mTerrainTestPlanes.push_back(mBoxShadowBSP->getPlane(traverse->mPlaneIndex));

         mask >>= 1;
         traverse = traverse->mFront;
      }
   }

   // there will be 2 duplicate node planes if there were only 2 planes lit
   if(numPlanes == 2)
   {
      for(S32 i = 0; i < mTerrainTestPlanes.size(); i++)
         for(U32 j = 0; j < mTerrainTestPlanes.size(); j++)
         {
            if(i == j)
               continue;

            if((mDot(mTerrainTestPlanes[i], mTerrainTestPlanes[j]) > gPlaneNormThresh) &&
               (mFabs(mTerrainTestPlanes[i].d - mTerrainTestPlanes[j].d) < gPlaneDistThresh))
            {
               mTerrainTestPlanes.erase(i);
               i--;
               break;
            }
         }
   }

   return(true);
}

bool blInteriorProxy::isShadowedBy(blInteriorProxy * test)
{
   // add if overlapping world box
   if((*this)->getWorldBox().isOverlapped((*test)->getWorldBox()))
      return(true);

   // test the box shadow volume
   for(U32 i = 0; i < mLitBoxSurfaces.size(); i++)
   {
      ShadowVolumeBSP::SVPoly * poly = mBoxShadowBSP->copyPoly(mLitBoxSurfaces[i]);
      if(test->mBoxShadowBSP->testPoly(poly))
         return(true);
   }

   return(false);
}

void blInteriorProxy::light(LightInfo * light)
{
   Platform::setMathControlStateKnown();

   InteriorInstance * interior = getObject();
   if(!interior)
      return;

   ColorF ambient = light->getAmbient();

   S32 time = Platform::getRealMilliseconds();

   // create own shadow volume
   ShadowVolumeBSP shadowVolume;

   // add the other objects lit surfaces into shadow volume
   for(ObjectProxy ** itr = gLighting->mLitObjects.begin(); itr != gLighting->mLitObjects.end(); itr++)
   {
      if(!(*itr)->getObject())
         continue;

      ObjectProxy* obj = *itr;
      if (obj != this)
      {
         if (obj->supportsShadowVolume())
            obj->addToShadowVolume(&shadowVolume, light, SceneLighting::SHADOW_DETAIL);
      }

/*
      // insert the terrain squares
      if(gLighting->isTerrain((*itr)->mObj))
      {
         TerrainProxy * terrain = static_cast<TerrainProxy*>(*itr);

         Vector<PlaneF> clipPlanes;
         clipPlanes = mTerrainTestPlanes;
         for(U32 i = 0; i < mOppositeBoxPlanes.size(); i++)
            clipPlanes.push_back(mOppositeBoxPlanes[i]);

         Vector<U16> shadowList;
         if(terrain->getShadowedSquares(clipPlanes, shadowList))
         {
            TerrainBlock * block = static_cast<TerrainBlock*>((*itr)->getObject());
            Point3F offset;
            block->getTransform().getColumn(3, &offset);

            F32 squareSize = block->getSquareSize();

            for(U32 j = 0; j < shadowList.size(); j++)
            {
               Point2I pos(shadowList[j] & TerrainBlock::BlockMask, shadowList[j] >> TerrainBlock::BlockShift);
               Point2F wPos(pos.x * squareSize + offset.x,
                  pos.y * squareSize + offset.y);

               Point3F pnts[4];
               pnts[0].set(wPos.x, wPos.y, fixedToFloat(block->getHeight(pos.x, pos.y)));
               pnts[1].set(wPos.x + squareSize, wPos.y, fixedToFloat(block->getHeight(pos.x + 1, pos.y)));
               pnts[2].set(wPos.x + squareSize, wPos.y + squareSize, fixedToFloat(block->getHeight(pos.x + 1, pos.y + 1)));
               pnts[3].set(wPos.x, wPos.y + squareSize, fixedToFloat(block->getHeight(pos.x, pos.y + 1)));

               GridSquare * gs = block->findSquare(0, pos);

               U32 squareIdx = (gs->flags & GridSquare::Split45) ? 0 : 2;

               for(U32 k = squareIdx; k < (squareIdx + 2); k++)
               {
                  // face plane inwards
                  PlaneF plane(pnts[TerrainSquareIndices[k][2]],
                     pnts[TerrainSquareIndices[k][1]],
                     pnts[TerrainSquareIndices[k][0]]);

                  if(mDot(plane, light->mDirection) > gParellelVectorThresh)
                  {
                     ShadowVolumeBSP::SVPoly * poly = shadowVolume.createPoly();
                     poly->mWindingCount = 3;

                     poly->mWinding[0] = pnts[TerrainSquareIndices[k][0]];
                     poly->mWinding[1] = pnts[TerrainSquareIndices[k][1]];
                     poly->mWinding[2] = pnts[TerrainSquareIndices[k][2]];
                     poly->mPlane = plane;

                     // create the shadow volume for this and insert
                     shadowVolume.buildPolyVolume(poly, light);
                     shadowVolume.insertPoly(poly);
                  }
               }
            }
         }
      }*/
   }

   // light all details
   for(U32 i = 0; i < interior->getResource()->getNumDetailLevels(); i++)
   {
      // clear lightmaps
      Interior * detail = interior->getResource()->getDetailLevel(i);
      gInteriorLMManager.clearLightmaps(detail->getLMHandle(), interior->getLMHandle());

      // clear out the last inserted interior
      shadowVolume.removeLastInterior();

      bool hasAlarm = detail->hasAlarmState();

      addToShadowVolume(&shadowVolume, light, i);

      //gLighting->addInterior(&shadowVolume, *this, light, i);

      for(U32 j = 0; j < shadowVolume.mSurfaces.size(); j++)
      {
         ShadowVolumeBSP::SurfaceInfo * surfaceInfo = shadowVolume.mSurfaces[j];

         U32 surfaceIndex = surfaceInfo->mSurfaceIndex;

         const Interior::Surface & surface = detail->getSurface(surfaceIndex);

         // alarm lighting
         GFXTexHandle normHandle = gInteriorLMManager.duplicateBaseLightmap(detail->getLMHandle(), interior->getLMHandle(), detail->getNormalLMapIndex(surfaceIndex));
         GFXTexHandle alarmHandle;

         GBitmap * normLightmap = normHandle->getBitmap();
         GBitmap * alarmLightmap = 0;

         // check if the lightmaps are shared
         if(hasAlarm)
         {
            if(detail->getNormalLMapIndex(surfaceIndex) != detail->getAlarmLMapIndex(surfaceIndex))
            {
               alarmHandle = gInteriorLMManager.duplicateBaseLightmap(detail->getLMHandle(), interior->getLMHandle(), detail->getAlarmLMapIndex(surfaceIndex));
               alarmLightmap = alarmHandle->getBitmap();
            }
         }

         // points right way?
         PlaneF plane = detail->getPlane(surface.planeIndex);
         if(Interior::planeIsFlipped(surface.planeIndex))
            plane.neg();

         const MatrixF & transform = interior->getTransform();
         const Point3F & scale = interior->getScale();

         //
         PlaneF projPlane;
         mTransformPlane(transform, scale, plane, &projPlane);

         F32 dot = mDot(projPlane, -light->getDirection());

         // cancel out lambert dot product and ambient lighting on hardware
         // with pixel shaders
         if( GFX->getPixelShaderVersion() > 0.0 )
         {
            dot = 1.0f;
            ambient.set( 0.0f, 0.0f, 0.0f );
         }

         // shadowed?
         if(!surfaceInfo->mShadowed.size())
         {
            // calc the color and convert to U8 rep
            ColorF tmp = (light->getColor() * dot) + ambient;
            tmp.clamp();
            ColorI color = tmp;

            // attempt to light both the normal and the alarm states
            for(U32 c = 0; c < 2; c++)
            {
               GBitmap * lightmap = (c == 0) ? normLightmap : alarmLightmap;
               if(!lightmap)
                  continue;

               //
               // Support for interior light map border sizes.
               //
               U32 xlen, ylen, xoff, yoff;
               U32 lmborder = detail->getLightMapBorderSize();
               xlen = surface.mapSizeX + (lmborder * 2);
               ylen = surface.mapSizeY + (lmborder * 2);
               xoff = surface.mapOffsetX - lmborder;
               yoff = surface.mapOffsetY - lmborder;

               // fill it
               for(U32 y = 0; y < ylen; y++)
               {
                  for(U32 x = 0; x < xlen; x++)
                  {
                     ColorI outColor(0, 255, 0, 255);

#ifndef SET_COLORS
                     ColorI lmColor(0, 0, 0, 255);
                     lightmap->getColor(xoff + x, yoff + y, lmColor);

                     U32 _r = static_cast<U32>( color.red ) + static_cast<U32>( lmColor.red );
                     U32 _g = static_cast<U32>( color.green ) + static_cast<U32>( lmColor.green );
                     U32 _b = static_cast<U32>( color.blue ) + static_cast<U32>( lmColor.blue );

                     outColor.red   = mClamp(_r, 0, 255);
                     outColor.green = mClamp(_g, 0, 255);
                     outColor.blue  = mClamp(_b, 0, 255);
#endif

                     lightmap->setColor(xoff + x, yoff + y, outColor);
                  }
               }
            }
            if(!surfaceInfo->mShadowed.size())
               continue;
         }

         //
         // Support for interior light map border sizes.
         //
         U32 xlen, ylen, xoff, yoff;
         U32 lmborder = detail->getLightMapBorderSize();
         xlen = surface.mapSizeX + (lmborder * 2);
         ylen = surface.mapSizeY + (lmborder * 2);
         xoff = surface.mapOffsetX - lmborder;
         yoff = surface.mapOffsetY - lmborder;

         // get the lmagGen...
         const Interior::TexGenPlanes & lmTexGenEQ = detail->getLMTexGenEQ(surfaceIndex);

         const F32 * const lGenX = lmTexGenEQ.planeX;
         const F32 * const lGenY = lmTexGenEQ.planeY;

         AssertFatal((lGenX[0] * lGenX[1] == 0.f) &&
            (lGenX[0] * lGenX[2] == 0.f) &&
            (lGenX[1] * lGenX[2] == 0.f), "Bad lmTexGen!");
         AssertFatal((lGenY[0] * lGenY[1] == 0.f) &&
            (lGenY[0] * lGenY[2] == 0.f) &&
            (lGenY[1] * lGenY[2] == 0.f), "Bad lmTexGen!");

         // get the axis index for the texgens (could be swapped)
         S32 si=0;
         S32 ti=0;
         S32 axis = -1;

         //
         if(lGenX[0] == 0.f && lGenY[0] == 0.f)          // YZ
         {
            axis = 0;
            if(lGenX[1] == 0.f) { // swapped?
               si = 2;
               ti = 1;
            } else {
               si = 1;
               ti = 2;
            }
         }
         else if(lGenX[1] == 0.f && lGenY[1] == 0.f)     // XZ
         {
            axis = 1;
            if(lGenX[0] == 0.f) { // swapped?
               si = 2;
               ti = 0;
            } else {
               si = 0;
               ti = 2;
            }
         }
         else if(lGenX[2] == 0.f && lGenY[2] == 0.f)     // XY
         {
            axis = 2;
            if(lGenX[0] == 0.f) { // swapped?
               si = 1;
               ti = 0;
            } else {
               si = 0;
               ti = 1;
            }
         }
         AssertFatal(!(axis == -1), "SceneLighting::lightInterior: bad TexGen!");

         const F32 * pNormal = ((const F32*)plane);

         Point3F start;
         F32 * pStart = ((F32*)start);

         F32 lumelScale = 1.0 / (lGenX[si] * normLightmap->getWidth());

         // get the start point on the lightmap
         pStart[si] = (((xoff * lumelScale) / (1.0 / lGenX[si])) - lGenX[3] ) / lGenX[si];
         pStart[ti] = (((yoff * lumelScale) / (1.0 / lGenY[ti])) - lGenY[3] ) / lGenY[ti];
         pStart[axis] = ((pNormal[si] * pStart[si]) + (pNormal[ti] * pStart[ti]) + plane.d) / -pNormal[axis];

         start.convolve(scale);
         transform.mulP(start);

         // get the s/t vecs oriented on the surface
         Point3F sVec;
         Point3F tVec;

         F32 * pSVec = ((F32*)sVec);
         F32 * pTVec = ((F32*)tVec);

         F32 angle;
         Point3F planeNormal;

         // s
         pSVec[si] = 1.f;
         pSVec[ti] = 0.f;

         planeNormal = plane;
         ((F32*)planeNormal)[ti] = 0.f;
         planeNormal.normalize();

         angle = mAcos(mClampF(((F32*)planeNormal)[axis], -1.f, 1.f));
         pSVec[axis] = (((F32*)planeNormal)[si] < 0.f) ? mTan(angle) : -mTan(angle);

         // t
         pTVec[ti] = 1.f;
         pTVec[si] = 0.f;

         planeNormal = plane;
         ((F32*)planeNormal)[si] = 0.f;
         planeNormal.normalize();

         angle = mAcos(mClampF(((F32*)planeNormal)[axis], -1.f, 1.f));
         pTVec[axis] = (((F32*)planeNormal)[ti] < 0.f) ? mTan(angle) : -mTan(angle);

         // scale the vectors

         sVec *= lumelScale;
         tVec *= lumelScale;

         // project vecs
         transform.mulV(sVec);
         sVec.convolve(scale);

         transform.mulV(tVec);
         tVec.convolve(scale);

         Point3F & curPos = start;
         Point3F sRun = sVec * xlen;

         // get the lexel area
         Point3F cross;
         mCross(sVec, tVec, &cross);
         F32 maxLexelArea = cross.len();

         const PlaneF & surfacePlane = shadowVolume.getPlane(surfaceInfo->mPlaneIndex);

         // get the world coordinate for each lexel
         for(U32 y = 0; y < ylen; y++)
         {
            for(U32 x = 0; x < xlen; x++)
            {
               ShadowVolumeBSP::SVPoly * poly = shadowVolume.createPoly();
               poly->mPlane = surfacePlane;
               poly->mWindingCount = 4;

               // set the poly indices
               poly->mWinding[0] = curPos;
               poly->mWinding[1] = curPos + sVec;
               poly->mWinding[2] = curPos + sVec + tVec;
               poly->mWinding[3] = curPos + tVec;

               //// insert poly which has been clipped to own shadow volume
               //ShadowVolumeBSP::SVPoly * store = 0;
               //shadowVolume.clipToSelf(surfaceInfo->mShadowVolume, &store, poly);

               //if(!store)
               //   continue;

               //F32 lexelArea = shadowVolume.getPolySurfaceArea(store);
               //F32 area = shadowVolume.getLitSurfaceArea(store, surfaceInfo);

               F32 area = shadowVolume.getLitSurfaceArea(poly, surfaceInfo);
               F32 shadowScale = mClampF(area / maxLexelArea, 0.f, 1.f);

               // get the color into U8
               ColorF tmp = (light->getColor() * dot * shadowScale) + ambient;
               tmp.clamp();
               ColorI color = tmp;

               // attempt to light both normal and alarm lightmaps
               for(U32 c = 0; c < 2; c++)
               {
                  GBitmap * lightmap = (c == 0) ? normLightmap : alarmLightmap;
                  if(!lightmap)
                     continue;

                  ColorI outColor(0, 0, 255, 255);

#ifndef SET_COLORS
                  ColorI lmColor(0, 0, 0, 255);
                  lightmap->getColor(xoff + x, yoff + y, lmColor);

                  U32 _r = static_cast<U32>( color.red ) + static_cast<U32>( lmColor.red );
                  U32 _g = static_cast<U32>( color.green ) + static_cast<U32>( lmColor.green );
                  U32 _b = static_cast<U32>( color.blue ) + static_cast<U32>( lmColor.blue );

                  outColor.red   = mClamp(_r, 0, 255);
                  outColor.green = mClamp(_g, 0, 255);
                  outColor.blue  = mClamp(_b, 0, 255);
#endif

                  lightmap->setColor(xoff + x, yoff + y, outColor);
               }

               curPos += sVec;
            }

            curPos -= sRun;
            curPos += tVec;
         }
      }
   }

   Con::printf("    = interior lit in %3.3f seconds", (Platform::getRealMilliseconds()-time)/1000.f);
}

void blInteriorProxy::postLight(bool lastLight)
{
   delete mBoxShadowBSP;
   mBoxShadowBSP = 0;

   InteriorInstance * interior = getObject();
   if(!interior)
      return;
}

//------------------------------------------------------------------------------
U32 blInteriorProxy::getResourceCRC()
{
   InteriorInstance * interior = getObject();
   if(!interior)
      return(0);
   return(interior->getCRC());
}

//------------------------------------------------------------------------------
bool blInteriorProxy::setPersistInfo(PersistInfo::PersistChunk * info)
{

   if(!Parent::setPersistInfo(info))
      return(false);

   blInteriorChunk * chunk = dynamic_cast<blInteriorChunk*>(info);
   AssertFatal(chunk, "blInteriorProxy::setPersistInfo: invalid info chunk!");

   InteriorInstance * interior = getObject();
   if(!interior)
      return(false);

   U32 numDetails = interior->getNumDetailLevels();

   // check the lighting method
// BTRTODO: Restore
//   AssertFatal(SceneLighting::smUseVertexLighting == Interior::smUseVertexLighting, "blInteriorProxy::setPersistInfo: invalid vertex lighting state");
//   if(SceneLighting::smUseVertexLighting != Interior::smUseVertexLighting)
//      return(false);

   /*
   // process the vertex lighting...
   if(chunk->mDetailVertexCount.size() != numDetails)
   return(false);

   AssertFatal(chunk->mVertexColorsNormal.size(), "blInteriorProxy::setPersistInfo: invalid chunk info");
   AssertFatal(!chunk->mHasAlarmState || chunk->mVertexColorsAlarm.size(), "blInteriorProxy::setPersistInfo: invalid chunk info");
   AssertFatal(!(chunk->mHasAlarmState ^ interior->getDetailLevel(0)->hasAlarmState()), "blInteriorProxy::setPersistInfo: invalid chunk info");


   U32 curPos = 0;
   for(U32 i = 0; i < numDetails; i++)
   {
   U32 count = chunk->mDetailVertexCount[i];
   Vector<ColorI>* normal = interior->getVertexColorsNormal(i);
   Vector<ColorI>* alarm  = interior->getVertexColorsAlarm(i);
   AssertFatal(normal != NULL && alarm != NULL, "Error, bad vectors returned!");

   normal->setSize(count);
   dMemcpy(normal->address(), &chunk->mVertexColorsNormal[curPos], count * sizeof(ColorI));

   if(chunk->mHasAlarmState)
   {
   alarm->setSize(count);
   dMemcpy(alarm->address(), &chunk->mVertexColorsAlarm[curPos], count * sizeof(ColorI));
   }

   curPos += count;
   }
   */
   // need lightmaps?
   //if(!SceneLighting::smUseVertexLighting)
   // BTRTODO: Fix me
   if (!false)
   {
      if(chunk->mDetailLightmapCount.size() != numDetails)
         return(false);

      LM_HANDLE instanceHandle = interior->getLMHandle();
      U32 idx = 0;

      for(U32 i = 0; i < numDetails; i++)
      {
         Interior * detail = interior->getDetailLevel(i);

         LM_HANDLE interiorHandle = detail->getLMHandle();
         Vector<GFXTexHandle> & baseHandles = gInteriorLMManager.getHandles(interiorHandle, 0);

         if(chunk->mDetailLightmapCount[i] > baseHandles.size())
            return(false);

         for(U32 j = 0; j < chunk->mDetailLightmapCount[i]; j++)
         {
            U32 baseIndex = chunk->mDetailLightmapIndices[idx];
            if(baseIndex >= baseHandles.size())
               return(false);

            AssertFatal(chunk->mLightmaps[idx], "blInteriorProxy::setPersistInfo: bunk bitmap!");
            if(chunk->mLightmaps[idx]->getWidth() != baseHandles[baseIndex]->getWidth() ||
               chunk->mLightmaps[idx]->getHeight() != baseHandles[baseIndex]->getHeight())
               return(false);

            GFXTexHandle tHandle = gInteriorLMManager.duplicateBaseLightmap(interiorHandle, instanceHandle, baseIndex);

            // create the diff bitmap
            tHandle->getBitmap()->combine( baseHandles[baseIndex]->getBitmap(), 
               chunk->mLightmaps[idx],
               GFXTOPAdd );

            idx++;
         }
      }
   }

   return(true);
}

bool blInteriorProxy::getPersistInfo(PersistInfo::PersistChunk * info)
{
   if(!Parent::getPersistInfo(info))
      return(false);

   blInteriorChunk* chunk = dynamic_cast<blInteriorChunk*>(info);
   AssertFatal(chunk, "blInteriorProxy::getPersistInfo: invalid info chunk!");

   InteriorInstance * interior = getObject();
   if(!interior)
      return(false);

   LM_HANDLE instanceHandle = interior->getLMHandle();

   AssertFatal(!chunk->mDetailLightmapCount.size(), "blInteriorProxy::getPersistInfo: invalid array!");
   AssertFatal(!chunk->mDetailLightmapIndices.size(), "blInteriorProxy::getPersistInfo: invalid array!");
   AssertFatal(!chunk->mLightmaps.size(), "blInteriorProxy::getPersistInfo: invalid array!");

   U32 numDetails = interior->getNumDetailLevels();
   U32 i;
   for(i = 0; i < numDetails; i++)
   {
      Interior * detail = interior->getDetailLevel(i);
      LM_HANDLE interiorHandle = detail->getLMHandle();

      Vector<GFXTexHandle> & baseHandles = gInteriorLMManager.getHandles(interiorHandle, 0);
      Vector<GFXTexHandle> & instanceHandles = gInteriorLMManager.getHandles(interiorHandle, instanceHandle);

      U32 litCount = 0;

      // walk all the instance lightmaps and grab diff lighting from them
      for(U32 j = 0; j < instanceHandles.size(); j++)
      {
         if(!instanceHandles[j])
            continue;

         litCount++;
         chunk->mDetailLightmapIndices.push_back(j);

         GBitmap * baseBitmap = baseHandles[j]->getBitmap();
         GBitmap * instanceBitmap = instanceHandles[j]->getBitmap();

         Point2I extent(baseBitmap->getWidth(), baseBitmap->getHeight());

         GBitmap * diffLightmap = new GBitmap(extent.x, extent.y, false);

         // diffLightmap = instanceBitmap - baseBitmap
         diffLightmap->combine( instanceBitmap, baseBitmap, GFXTOPSubtract );

         chunk->mLightmaps.push_back(diffLightmap);
      }

      chunk->mDetailLightmapCount.push_back(litCount);
   }

   // process the vertex lighting...
   AssertFatal(!chunk->mDetailVertexCount.size(), "blInteriorProxy::getPersistInfo: invalid chunk info");
   AssertFatal(!chunk->mVertexColorsNormal.size(), "blInteriorProxy::getPersistInfo: invalid chunk info");
   AssertFatal(!chunk->mVertexColorsAlarm.size(), "blInteriorProxy::getPersistInfo: invalid chunk info");

   chunk->mHasAlarmState = interior->getDetailLevel(0)->hasAlarmState();
   chunk->mDetailVertexCount.setSize(numDetails);

   U32 size = 0;
   for(i = 0; i < numDetails; i++)
   {
      Interior * detail = interior->getDetailLevel(i);

      U32 count = detail->getWindingCount();
      chunk->mDetailVertexCount[i] = count;
      size += count;
   }

   /*
   chunk->mVertexColorsNormal.setSize(size);
   if(chunk->mHasAlarmState)
   chunk->mVertexColorsAlarm.setSize(size);

   U32 curPos = 0;
   for(i = 0; i < numDetails; i++)
   {
   Vector<ColorI>* normal = interior->getVertexColorsNormal(i);
   Vector<ColorI>* alarm  = interior->getVertexColorsAlarm(i);
   AssertFatal(normal != NULL && alarm != NULL, "Error, no normal or alarm vertex colors!");

   U32 count = chunk->mDetailVertexCount[i];
   dMemcpy(&chunk->mVertexColorsNormal[curPos], normal->address(), count * sizeof(ColorI));

   if(chunk->mHasAlarmState)
   dMemcpy(&chunk->mVertexColorsAlarm[curPos], alarm->address(), count * sizeof(ColorI));

   curPos += count;
   }
   */

   return(true);
}


SceneLighting::ObjectProxy* blInteriorSystem::createObjectProxy(SceneObject* obj, SceneLighting::ObjectProxyList* sceneObjects)
{
   if ((obj->getTypeMask() & InteriorObjectType) != 0)
   {
      return new blInteriorProxy(obj);
   } else {
      return NULL;
   }
}

PersistInfo::PersistChunk* blInteriorSystem::createPersistChunk(const U32 chunkType) 
{
   if (chunkType == PersistInfo::PersistChunk::InteriorChunkType)
   {
      return new blInteriorChunk;
   } else {
      return NULL;
   }
}

bool blInteriorSystem::createPersistChunkFromProxy(SceneLighting::ObjectProxy* objproxy, PersistInfo::PersistChunk **ret)
{
   if ((objproxy->mObj->getTypeMask() & InteriorObjectType) != 0)
   {
      *ret = new blInteriorChunk;
      return true;
   } else {
      return false;
   }
}

void blInteriorSystem::init()
{

}

U32 blInteriorSystem::addObjectType()
{
   return InteriorObjectType;
}

U32 blInteriorSystem::addToClippingMask()
{
   return InteriorObjectType;
}

void blInteriorSystem::processLightingBegin()
{
   // Store the vertex lighting state when we being lighting, we compare this when we finish lighting 
   smUseVertexLighting = Interior::smUseVertexLighting;
}

void blInteriorSystem::processLightingCompleted(bool success)
{
   if(success)
   {
      AssertFatal(smUseVertexLighting == Interior::smUseVertexLighting, "SceneLighting::completed: vertex lighting state changed during scene light");

      // cannot do anything if vertex state has changed (since we only load in what is needed)
      if(smUseVertexLighting == Interior::smUseVertexLighting)
      {
         if(!smUseVertexLighting)
         {
            gInteriorLMManager.downloadGLTextures();
            gInteriorLMManager.destroyBitmaps();
         }
         else
            gInteriorLMManager.destroyTextures();
      }
   }
}

// Given a ray, this will return the color from the lightmap of this object, return true if handled
bool blInteriorSystem::getColorFromRayInfo(RayInfo collision, ColorF& result)
{
   InteriorInstance* interior = dynamic_cast<InteriorInstance*>(collision.object);
   if (interior == NULL)
      return false;

   interior->getRenderWorldTransform().mulP(collision.point);
   Interior *detail = interior->getDetailLevel(0);
   AssertFatal((detail), "SceneObject::getLightingAmbientColor: invalid interior");
   if(collision.face < detail->getSurfaceCount())
   {
      const Interior::Surface &surface = detail->getSurface(collision.face);
      const Interior::TexGenPlanes &texgen = detail->getLMTexGenEQ(collision.face);

      GBitmap* lightmap = gInteriorLMManager.getHandle(detail->getLMHandle(),
         interior->getLMHandle(), detail->getNormalLMapIndex(collision.face)).getBitmap();
      if (!lightmap)
         return false;

      Point2F uv;
      uv.x = mDot(texgen.planeX, collision.point) + texgen.planeX.d;
      uv.y = mDot(texgen.planeY, collision.point) + texgen.planeY.d;

      U32 size = (U32)(uv.x * F32(lightmap->getWidth()));
      size = mClamp(size, surface.mapOffsetX, (surface.mapOffsetX + surface.mapSizeX));
      uv.x = F32(size) / F32(lightmap->getWidth());

      size = (U32)(uv.y * F32(lightmap->getHeight()));
      size = mClamp(size, surface.mapOffsetY, (surface.mapOffsetY + surface.mapSizeY));
      uv.y = F32(size) / F32(lightmap->getHeight());

      result = lightmap->sampleTexel(uv.x, uv.y);
      return true;
   }
   return false;
}
