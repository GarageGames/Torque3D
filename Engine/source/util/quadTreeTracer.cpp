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

#include "util/quadTreeTracer.h"
#include "platform/profiler.h"
#include "core/frameAllocator.h"

static F32 calcInterceptV(F32 vStart, F32 invDeltaV, F32 intercept)
{
   return (intercept - vStart) * invDeltaV;
}

static F32 calcInterceptNone(F32, F32, F32)
{
   return F32_MAX;
}

static F32 (*calcInterceptX)(F32, F32, F32);
static F32 (*calcInterceptY)(F32, F32, F32);

bool QuadTreeTracer::castRay(const Point3F &start, const Point3F &end, RayInfo *info)
{
   PROFILE_START(QuadTreeTracer_castRay);

   // Do some precalculations we'll use for the rest of this routine.
   // Set up our intercept calculation methods.
   F32 invDeltaX;
   if(end.x == start.x)
   {
      calcInterceptX = calcInterceptNone;
      invDeltaX = 0;
   }
   else
   {
      invDeltaX = 1.f / (end.x - start.x);
      calcInterceptX = calcInterceptV;
   }

   F32 invDeltaY;
   if(end.y == start.y)
   {
      calcInterceptY = calcInterceptNone;
      invDeltaY = 0;
   }
   else
   {
      invDeltaY = 1.f / (end.y - start.y);
      calcInterceptY = calcInterceptV;
   }

   // Subdivide our space based on the size of the lowest level of the tree...
   const F32 invSize   = 1.f / F32(BIT(mTreeDepth-1));

   // Grab this off the frame allocator, we don't want to do a proper alloc
   // on every ray!
   FrameAllocatorMarker stackAlloc;
   RayStackNode *stack = (RayStackNode*)stackAlloc.alloc(sizeof(RayStackNode) * (mTreeDepth * 3 + 1));

   U32 stackSize = 1;

   // Kick off the stack with the root node.
   stack[0].startT   = 0;
   stack[0].endT     = 1;
   stack[0].squarePos.set(0,0);
   stack[0].level    = mTreeDepth - 1;

   //Con::printf("QuadTreeTracer::castRay(%x)", this);

   // Aright, now let's do some raycasting!
   while(stackSize--)
   {
      // Get the current node for easy access...
      RayStackNode *sn = stack + stackSize;

      const U32 level         = sn->level;
      const F32 startT        = sn->startT;
      const F32 endT          = sn->endT;
      const Point2I squarePos = sn->squarePos;

      AssertFatal((startT >= 0.f) && (startT <= 1.f),  "QuadTreeTracer::castRay - out of range startT on stack!");
      AssertFatal((endT >= 0.f) && (endT <= 1.f),     "QuadTreeTracer::castRay - out of range endT   on stack!");

      //Con::printf(" -- node(%d, %d @ %d), sT=%f, eT=%f", squarePos.x, squarePos.y, level, startT, endT);

      // Figure our start and end Z.
      const F32 startZ = startT * (end.z - start.z) + start.z;
      const F32 endZ   = endT   * (end.z - start.z) + start.z;

      // Ok, now let's see if we hit the lower bound
      const F32 squareMin = getSquareMin(level, squarePos);

      if(startZ < squareMin && endZ < squareMin)
         continue; //Nope, skip out.

      // Hmm, let's check the upper bound.
      const F32 squareMax = getSquareMax(level, squarePos);

      if(startZ > squareMax && endZ > squareMax)
         continue; //Nope, skip out.

      // We might be intersecting something... If we've hit
      // the tree depth let's deal with the leaf intersection.
      if(level == 0)
      {
         //Con::printf(" ++ check node(%d, %d @ %d), sT=%f, eT=%f", squarePos.x, squarePos.y, level, startT, endT);

         if(castLeafRay(squarePos, start, end, startT, endT, info))
         {
            PROFILE_END();
            return true; // We hit, tell 'em so!
         }
         continue; // Otherwise, keep looking.
      }
      else
      {
         // Ok, we have to push our children as we're an inner node.

         // First, figure out some widths...
         U32 subSqSize  = BIT(level - 1);

         // Now, calculate intercepts so we know how to deal with this
         // situation... (intercept = position, int = t value for that pos)

         const F32 xIntercept = (squarePos.x + subSqSize) * invSize;
         F32 xInt = calcInterceptX(start.x, invDeltaX, xIntercept);

         const F32 yIntercept = (squarePos.y + subSqSize) * invSize;
         F32 yInt = calcInterceptY(start.y, invDeltaY, yIntercept);

         // Our starting position for this subray...
         const F32 startX = startT * (end.x - start.x) + start.x;
         const F32 startY = startT * (end.y - start.y) + start.y;

         // Deal with squares that might be "behind" the ray.
         if(xInt < startT) xInt = F32_MAX;
         if(yInt < startT) yInt = F32_MAX;

         // Do a little magic to calculate our next checks...
         const U32 x0 = (startX > xIntercept) * subSqSize;
         const U32 y0 = (startY > yIntercept) * subSqSize;

         const U32 x1 = subSqSize - x0;
         const U32 y1 = subSqSize - y0;

         const U32 nextLevel = level - 1;

         // Ok, now let's figure out what nodes, in what order, need to go
         // on the stack. We push things on in reverse order of processing.
         if(xInt > endT && yInt > endT)
         {
            stack[stackSize].squarePos.set(squarePos.x + x0, squarePos.y + y0);
            stack[stackSize].level = nextLevel;
            stackSize++;
         }
         else if(xInt < yInt)
         {
            F32 nextIntersect = endT;

            if(yInt <= endT)
            {
               stack[stackSize].squarePos.set(squarePos.x + x1, squarePos.y + y1);
               stack[stackSize].startT = yInt;
               stack[stackSize].endT   = endT;
               stack[stackSize].level  = nextLevel;
               nextIntersect = yInt;
               stackSize++;
            }

            // Do middle two, order doesn't matter.
            stack[stackSize].squarePos.set(squarePos.x + x1, squarePos.y + y0);
            stack[stackSize].startT = xInt;
            stack[stackSize].endT   = nextIntersect;
            stack[stackSize].level  = nextLevel;

            stack[stackSize+1].squarePos.set(squarePos.x + x0, squarePos.y + y0);
            stack[stackSize+1].startT = startT;
            stack[stackSize+1].endT   = xInt;
            stack[stackSize+1].level  = nextLevel;
            stackSize += 2;
         }
         else if(yInt < xInt)
         {
            F32 nextIntersect = endT;
            if(xInt <= endT)
            {
               stack[stackSize].squarePos.set(squarePos.x + x1, squarePos.y + y1);
               stack[stackSize].startT = xInt;
               stack[stackSize].endT   = endT;
               stack[stackSize].level  = nextLevel;
               nextIntersect = xInt;
               stackSize++;
            }
            stack[stackSize].squarePos.set(squarePos.x + x0, squarePos.y + y1);
            stack[stackSize].startT  = yInt;
            stack[stackSize].endT    = nextIntersect;
            stack[stackSize].level   = nextLevel;

            stack[stackSize+1].squarePos.set(squarePos.x + x0, squarePos.y + y0);
            stack[stackSize+1].startT = startT;
            stack[stackSize+1].endT   = yInt;
            stack[stackSize+1].level  = nextLevel;
            stackSize += 2;
         }
         else
         {
            stack[stackSize].squarePos.set(squarePos.x + x1, squarePos.y + y1);
            stack[stackSize].startT = xInt;
            stack[stackSize].endT   = endT;
            stack[stackSize].level  = nextLevel;

            stack[stackSize+1].squarePos.set(squarePos.x + x0, squarePos.y + y0);
            stack[stackSize+1].startT = startT;
            stack[stackSize+1].endT   = xInt;
            stack[stackSize+1].level  = nextLevel;
            stackSize += 2;
         }
      }
   }

   // Nothing found, so give up.
   PROFILE_END();
   return false;
}