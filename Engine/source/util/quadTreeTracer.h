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

#ifndef _QUADTREETRACER_H_
#define _QUADTREETRACER_H_

#include "platform/platform.h"
#include "math/mPoint3.h"
#include "scene/sceneObject.h"

/// Helper class to perform a fast, recursive ray cast against a set of
/// hierarchical bounding boxes.
///
/// This class assumes that it is working on a unit quadtree (ie, one that
/// extends from 0..1 in the XY dimensions. Z scale is unaffected).
///
/// Node indexing is done TGE Terrain style - 0 is the largest level of the
/// quadtree, while coordinates are always in the full range of the quadtree
/// (in a 6 deep tree, 0..63, for instance). This allows the quadtree descent
/// to be very fast!
class QuadTreeTracer
{
protected:

   struct StackNode
   {
      Point2I squarePos;
      U32     level;
   };

   struct RayStackNode : StackNode
   {
      F32     startT;
      F32     endT;
   };

   U32 mTreeDepth;

   QuadTreeTracer(U32 treeDepth)
      : mTreeDepth(treeDepth)
   {
   }

   /// Children better implement these! They return min/max height bounds
   /// of the specified square.
   virtual const F32 getSquareMin(const U32 &level, const Point2I &pos) const = 0;
   virtual const F32 getSquareMax(const U32 &level, const Point2I &pos) const = 0;

   /// And this does checks on leaf nodes.
   virtual bool castLeafRay(const Point2I pos, const Point3F &start, const Point3F &end, const F32 &startT, const F32 &endT, RayInfo *info) = 0;

   /// Helper function to calculate intercepts.
   inline const F32 calcIntercept(const F32 vStart, const F32 invDeltaV, const F32 intercept) const
   {
      return (intercept - vStart) * invDeltaV;
   }

public:

   /// Size of a quadtree of depth.
   static inline const U32 getNodeCount(const U32 depth)
   {
      return 0x55555555 & ((1 << depth*2) - 1);
   }

   /// Index of a node at given position in a quadtree.
   static inline const U32 getNodeIndex(const U32 level, const Point2I pos)
   {
      //AssertFatal(level < mTreeDepth, "QuadTreeTracer::getNodeIndex - out of range level!)
      AssertFatal(pos.x < BIT(level) && pos.x >= 0 , "QuadTreeTracer::getNodeIndex - out of range x for level!");
      AssertFatal(pos.y < BIT(level) && pos.y >= 0 , "QuadTreeTracer::getNodeIndex - out of range y for level!");

      const U32 base = getNodeCount(level);
      return base + (pos.x << level) + pos.y;
   }

   /// Cast a ray against a quadtree of hierarchical bounding boxes.
   ///
   /// This method assumes the quadtree extends from (0..1) along the
   /// X and Y axes. Z is unscaled. You may need to adjust the points
   /// you pass into this method to get the proper results.
   bool castRay(const Point3F &start, const Point3F &end, RayInfo *info);
};

#endif