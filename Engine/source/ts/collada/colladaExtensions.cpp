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

#include "platform/platform.h"

#include "math/mRandom.h"
#include "ts/collada/colladaExtensions.h"

/// Check if any of the MAYA texture transform elements are animated within
/// the interval
bool ColladaExtension_effect::animatesTextureTransform(F32 start, F32 end)
{
   return repeatU.isAnimated(start, end)           || repeatV.isAnimated(start, end)         ||
          offsetU.isAnimated(start, end)           || offsetV.isAnimated(start, end)         ||
          rotateUV.isAnimated(start, end)          || noiseU.isAnimated(start, end)          ||
          noiseV.isAnimated(start, end);
}

/// Apply the MAYA texture transform to the given UV coordinates
void ColladaExtension_effect::applyTextureTransform(Point2F& uv, F32 time)
{
   // This function will be called for every tvert, every frame. So cache the
   // texture transform parameters to avoid interpolating them every call (since
   // they are constant for all tverts for a given 't')
   if (time != lastAnimTime) {
      // Update texture transform
      textureTransform.set(EulerF(0, 0, rotateUV.getValue(time)));
      textureTransform.setPosition(Point3F(
         offsetU.getValue(time) + noiseU.getValue(time)*gRandGen.randF(),
         offsetV.getValue(time) + noiseV.getValue(time)*gRandGen.randF(),
         0));
      textureTransform.scale(Point3F(repeatU.getValue(time), repeatV.getValue(time), 1.0f));

      lastAnimTime = time;
   }

   // Apply texture transform
   Point3F result;
   textureTransform.mulP(Point3F(uv.x, uv.y, 0), &result);

   uv.x = result.x;
   uv.y = result.y;
}
