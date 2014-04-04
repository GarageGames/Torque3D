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
#include "math/mPoint2.h"
#include "gfx/bitmap/gBitmap.h"

#include "gfx/util/distanceField.h"

//-----------------------------------------------------------------------------
struct DistanceFieldSearchSpaceStruct
{
   S32 xOffset;
   S32 yOffset;
   F32 distance;
};

S32 QSORT_CALLBACK cmpSortDistanceFieldSearchSpaceStruct(const void* p1, const void* p2)
{
   const DistanceFieldSearchSpaceStruct* sp1 = (const DistanceFieldSearchSpaceStruct*)p1;
   const DistanceFieldSearchSpaceStruct* sp2 = (const DistanceFieldSearchSpaceStruct*)p2;

   if (sp2->distance > sp1->distance)
      return -1;
   else if (sp2->distance == sp1->distance)
      return 0;
   else
      return 1;
}

//GBitmap * GFXUtil::DistanceField::makeDistanceField(GBitmap * sourceBmp, S32 targetSizeX, S32 targetSizeY, F32 rangePct)
//{
//   AssertFatal(sourceBmp->getFormat() == GFXFormatA8,"Use an alpha-only texture to create distance fields");
//
//   static Vector<DistanceFieldSearchSpaceStruct> searchSpace;
//
//   S32 sourceSizeX = sourceBmp->getWidth();
//   S32 sourceSizeY = sourceBmp->getHeight();
//
//   S32 targetToSourceScalarX = sourceSizeX / targetSizeX;
//   S32 targetToSourceScalarY = sourceSizeY / targetSizeY;
//   S32 targetToSourcePixOffsetX = targetToSourceScalarX / 2;
//   S32 targetToSourcePixOffsetY = targetToSourceScalarY / 2;
//
//   F32 range = getMin(sourceSizeX,sourceSizeY) * rangePct;
//   F32 range2 = range * 2.f;
//
//   {
//      S32 intRange = mCeil(range);
//      for(S32 spaceY = -intRange; spaceY < intRange; spaceY++)
//      {
//         for(S32 spaceX = -intRange; spaceX < intRange; spaceX++)
//         {
//            if(spaceX == 0 && spaceY == 0)
//               continue;
//
//            F32 distance = Point2F(spaceX,spaceY).len();
//            if(distance <= range)
//            {
//               searchSpace.increment();
//               searchSpace.last().distance = distance;
//               searchSpace.last().xOffset = spaceX;
//               searchSpace.last().yOffset = spaceY;
//            }
//         }
//      }
//   }
//   dQsort(searchSpace.address(), searchSpace.size(), sizeof(DistanceFieldSearchSpaceStruct), cmpSortDistanceFieldSearchSpaceStruct);
//
//   GBitmap * targetBmp = new GBitmap(targetSizeX,targetSizeY,false,GFXFormatA8);
//
//   U8 * targetPixel = targetBmp->getWritableBits();
//   for(S32 y = 0; y < targetSizeY; y++)
//   {
//      for(S32 x = 0; x < targetSizeX; x++)
//      {
//         S32 sourceX = x * targetToSourceScalarX + targetToSourcePixOffsetX;
//         S32 sourceY = y * targetToSourceScalarY + targetToSourcePixOffsetY;
//
//         const U8 * thisPixel = sourceBmp->getAddress(sourceX,sourceY);
//
//         bool thisPixelEmpty = *thisPixel == 0;
//
//         F32 closestDist = F32_MAX;
//
//         for(DistanceFieldSearchSpaceStruct * seachSpaceStructPtr = searchSpace.begin(); seachSpaceStructPtr <= searchSpace.end(); seachSpaceStructPtr++)
//         {
//            DistanceFieldSearchSpaceStruct & searchSpaceStruct = *seachSpaceStructPtr;
//            S32 cx = sourceX + searchSpaceStruct.xOffset;
//            if(cx < 0 || cx >= sourceSizeX)
//               continue;
//
//            S32 cy = sourceY + searchSpaceStruct.yOffset;
//            if(cy < 0 || cy >= sourceSizeY)
//               continue;
//
//            const U8 * checkPixel = sourceBmp->getAddress(cx,cy);
//            if((*checkPixel == 0) != thisPixelEmpty)
//            {
//               closestDist = searchSpaceStruct.distance;
//               break;
//            }
//         }
//
//         F32 diff = thisPixelEmpty ? getMax(-0.5f,-(closestDist / range2)) : getMin(0.5f,closestDist / range2);
//         F32 targetValue = 0.5f + diff;
//
//         *targetPixel = targetValue * 255;
//         targetPixel++;
//      }
//   }
//
//   searchSpace.clear();
//
//   return targetBmp;
//}

void GFXUtil::DistanceField::makeDistanceField( const U8 * sourceData, S32 sourceSizeX, S32 sourceSizeY, U8 * targetData, S32 targetSizeX, S32 targetSizeY, F32 radius )
{
   static Vector<DistanceFieldSearchSpaceStruct> searchSpace;

   S32 targetToSourceScalarX = sourceSizeX / targetSizeX;
   S32 targetToSourceScalarY = sourceSizeY / targetSizeY;
   S32 targetToSourcePixOffsetX = targetToSourceScalarX / 2;
   S32 targetToSourcePixOffsetY = targetToSourceScalarY / 2;

   F32 radius2 = radius * 2.f;

   {
      S32 intRange = mCeil(radius);
      for(S32 spaceY = -intRange; spaceY < intRange; spaceY++)
      {
         for(S32 spaceX = -intRange; spaceX < intRange; spaceX++)
         {
            if(spaceX == 0 && spaceY == 0)
               continue;

            F32 distance = Point2F(spaceX,spaceY).len();
            if(distance <= radius)
            {
               searchSpace.increment();
               searchSpace.last().distance = distance;
               searchSpace.last().xOffset = spaceX;
               searchSpace.last().yOffset = spaceY;
            }
         }
      }
   }
   dQsort(searchSpace.address(), searchSpace.size(), sizeof(DistanceFieldSearchSpaceStruct), cmpSortDistanceFieldSearchSpaceStruct);

   for(S32 y = 0; y < targetSizeY; y++)
   {
      for(S32 x = 0; x < targetSizeX; x++)
      {
         S32 sourceX = x * targetToSourceScalarX + targetToSourcePixOffsetX;
         S32 sourceY = y * targetToSourceScalarY + targetToSourcePixOffsetY;

         bool thisPixelEmpty = sourceData[sourceY * sourceSizeX + sourceX] < 127;

         F32 closestDist = F32_MAX;

         for(DistanceFieldSearchSpaceStruct * seachSpaceStructPtr = searchSpace.begin(); seachSpaceStructPtr <= searchSpace.end(); seachSpaceStructPtr++)
         {
            DistanceFieldSearchSpaceStruct & searchSpaceStruct = *seachSpaceStructPtr;
            S32 cx = sourceX + searchSpaceStruct.xOffset;
            if(cx < 0 || cx >= sourceSizeX)
               continue;

            S32 cy = sourceY + searchSpaceStruct.yOffset;
            if(cy < 0 || cy >= sourceSizeY)
               continue;

            if((sourceData[cy * sourceSizeX + cx] < 127) != thisPixelEmpty)
            {
               closestDist = searchSpaceStruct.distance;
               break;
            }
         }

         F32 diff = thisPixelEmpty ? getMax(-0.5f,-(closestDist / radius2)) : getMin(0.5f,closestDist / radius2);
         F32 targetValue = 0.5f + diff;

         *targetData = targetValue * 255;
         targetData++;
      }
   }

   searchSpace.clear();
}
