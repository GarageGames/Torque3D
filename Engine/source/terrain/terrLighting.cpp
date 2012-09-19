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

#include "terrain/terrRender.h"
#include "lighting/lightInfo.h"
#include "scene/sceneRenderState.h"

/*

U32 TerrainRender::testSquareLights(GridSquare *sq, S32 level, const Point2I &pos, U32 lightMask)
{
   
   // Calculate our Box3F for this GridSquare
   Point3F boxMin(pos.x * mSquareSize + mBlockPos.x, pos.y * mSquareSize + mBlockPos.y, fixedToFloat(sq->minHeight));
   F32 blockSize = F32(mSquareSize * (1 << level));
   F32 blockHeight = fixedToFloat(sq->maxHeight - sq->minHeight);
   Point3F boxMax(boxMin);
   boxMax += Point3F(blockSize, blockSize, blockHeight);
   Box3F gridBox(boxMin, boxMax);

   U32 retMask = 0;

   for(S32 i = 0; (lightMask >> i) != 0; i++)
   {
      if(lightMask & (1 << i))
      {
         if (mTerrainLights[i].light->mType != LightInfo::Vector)
         {
            // test the visibility of this light to box         
            F32 dist = gridBox.getDistanceFromPoint(mTerrainLights[i].pos);
            static F32 minDist = 1e14f;
            minDist = getMin(minDist, dist);
            if(dist < mTerrainLights[i].radius)
               retMask |= (1 << i);
         } else {
            retMask  |= (1 << i);
         }
      }
   }
   return retMask;
}

void TerrainRender::buildLightArray(SceneState * state)
{
   PROFILE_SCOPE(TerrainRender_buildLightArray);

   mDynamicLightCount = 0;
   if ((mTerrainLighting == NULL) || (!TerrainRender::mEnableTerrainDynLights))
      return;

   static LightInfoList lights;
   lights.clear();   

   LIGHTMGR->getBestLights(lights);
   // create terrain lights from these...
   U32 curIndex = 0;
   for(U32 i = 0; i < lights.size(); i++)
   {
      LightInfo* light = lights[i];
      if((light->mType != LightInfo::Point) && (light->mType != LightInfo::Spot))
         continue;

      // set the 'fo
      TerrLightInfo & info = mTerrainLights[curIndex++];
      mCurrentBlock->getWorldTransform().mulP(light->mPos, &info.pos);
      info.radius = light->getRadius();
      info.light = light;
   }

   mDynamicLightCount = curIndex;
}

*/
