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

#include "gfx/util/triListOpt.h"
#include "core/frameAllocator.h"
#include "platform/profiler.h"
#include "math/mMathFn.h"

namespace TriListOpt
{

void OptimizeTriangleOrdering(const dsize_t numVerts, const dsize_t numIndices, const U32 *indices, IndexType *outIndices)
{
   PROFILE_SCOPE(TriListOpt_OptimizeTriangleOrdering);

   if(numVerts == 0 || numIndices == 0)
   {
      dCopyArray(outIndices, indices, numIndices);
      return;
   }

   const U32 NumPrimitives = numIndices / 3;
   AssertFatal(NumPrimitives == U32(mFloor(numIndices / 3.0f)), "Number of indicies not divisible by 3, not a good triangle list.");

   //
   // Step 1: Run through the data, and initialize
   //
   FrameTemp<VertData> vertexData(numVerts);
   FrameTemp<TriData> triangleData(NumPrimitives);

   U32 curIdx = 0;
   for(S32 tri = 0; tri < NumPrimitives; tri++)
   {
      TriData &curTri = triangleData[tri];

      for(S32 c = 0; c < 3; c++)
      {  
         const U32 &curVIdx = indices[curIdx];
         AssertFatal(curVIdx < numVerts, "Out of range index.");

         // Add this vert to the list of verts that define the triangle
         curTri.vertIdx[c] = curVIdx;

         VertData &curVert = vertexData[curVIdx];

         // Increment the number of triangles that reference this vertex
         curVert.numUnaddedReferences++;

         curIdx++;
      }
   }

   // Allocate per-vertex triangle lists, and calculate the starting score of
   // each of the verts
   for(S32 v = 0; v < numVerts; v++)
   {
      VertData &curVert = vertexData[v];
      curVert.triIndex = new S32[curVert.numUnaddedReferences];
      curVert.score = FindVertexScore::score(curVert);
   }

   // These variables will be used later, but need to be declared now
   S32 nextNextBestTriIdx = -1, nextBestTriIdx = -1;
   F32 nextNextBestTriScore = -1.0f, nextBestTriScore = -1.0f;

#define _VALIDATE_TRI_IDX(idx) if(idx > -1) { AssertFatal(idx < NumPrimitives, "Out of range triangle index."); AssertFatal(!triangleData[idx].isInList, "Triangle already in list, bad."); }
#define _CHECK_NEXT_NEXT_BEST(score, idx) { if(score > nextNextBestTriScore) { nextNextBestTriIdx = idx; nextNextBestTriScore = score; } }
#define _CHECK_NEXT_BEST(score, idx) { if(score > nextBestTriScore) { _CHECK_NEXT_NEXT_BEST(nextBestTriScore, nextBestTriIdx); nextBestTriIdx = idx; nextBestTriScore = score; } _VALIDATE_TRI_IDX(nextBestTriIdx); }

   // Fill-in per-vertex triangle lists, and sum the scores of each vertex used
   // per-triangle, to get the starting triangle score
   curIdx = 0;
   for(S32 tri = 0; tri < NumPrimitives; tri++)
   {
      TriData &curTri = triangleData[tri];

      for(S32 c = 0; c < 3; c++)
      {  
         const U32 &curVIdx = indices[curIdx];
         AssertFatal(curVIdx < numVerts, "Out of range index.");
         VertData &curVert = vertexData[curVIdx];

         // Add triangle to triangle list
         curVert.triIndex[curVert.numReferences++] = tri;

         // Add vertex score to triangle score
         curTri.score += curVert.score;

         curIdx++;
      }

      // This will pick the first triangle to add to the list in 'Step 2'
      _CHECK_NEXT_BEST(curTri.score, tri);
      _CHECK_NEXT_NEXT_BEST(curTri.score, tri);
   }

   //
   // Step 2: Start emitting triangles...this is the emit loop
   //
   LRUCacheModel lruCache;
   for(S32 outIdx = 0; outIdx < numIndices; /* this space intentionally left blank */ )
   {
      // If there is no next best triangle, than search for the next highest
      // scored triangle that isn't in the list already
      if(nextBestTriIdx < 0)
      {
         // TODO: Something better than linear performance here...
         nextBestTriScore = nextNextBestTriScore = -1.0f;
         nextBestTriIdx = nextNextBestTriIdx = -1;

         for(S32 tri = 0; tri < NumPrimitives; tri++)
         {
            TriData &curTri = triangleData[tri];

            if(!curTri.isInList)
            {
               _CHECK_NEXT_BEST(curTri.score, tri);
               _CHECK_NEXT_NEXT_BEST(curTri.score, tri);
            }
         }
      }
      AssertFatal(nextBestTriIdx > -1, "Ran out of 'nextBestTriangle' before I ran out of indices...not good.");

      // Emit the next best triangle
      TriData &nextBestTri = triangleData[nextBestTriIdx];
      AssertFatal(!nextBestTri.isInList, "Next best triangle already in list, this is no good.");
      for(S32 i = 0; i < 3; i++)
      {
         // Emit index
         outIndices[outIdx++] = IndexType(nextBestTri.vertIdx[i]);

         // Update the list of triangles on the vert
         VertData &curVert = vertexData[nextBestTri.vertIdx[i]];
         curVert.numUnaddedReferences--;
         for(S32 t = 0; t < curVert.numReferences; t++)
         {
            if(curVert.triIndex[t] == nextBestTriIdx)
            {
               curVert.triIndex[t] = -1;
               break;
            }
         }

         // Update cache
         lruCache.useVertex(nextBestTri.vertIdx[i], &curVert);
      }
      nextBestTri.isInList = true;

      // Enforce cache size, this will update the cache position of all verts
      // still in the cache. It will also update the score of the verts in the
      // cache, and give back a list of triangle indicies that need updating.
      Vector<U32> trisToUpdate;
      lruCache.enforceSize(MaxSizeVertexCache, trisToUpdate);

      // Now update scores for triangles that need updates, and find the new best
      // triangle score/index
      nextBestTriIdx = -1;
      nextBestTriScore = -1.0f;
      for(Vector<U32>::iterator itr = trisToUpdate.begin(); itr != trisToUpdate.end(); itr++)
      {
         TriData &tri = triangleData[*itr];

         // If this triangle isn't already emitted, re-score it
         if(!tri.isInList)
         {
            tri.score = 0.0f;

            for(S32 i = 0; i < 3; i++)
               tri.score += vertexData[tri.vertIdx[i]].score;

            _CHECK_NEXT_BEST(tri.score, *itr);
            _CHECK_NEXT_NEXT_BEST(tri.score, *itr);
         }
      }

      // If there was no love finding a good triangle, than see if there is a
      // next-next-best triangle, and if there isn't one of those...well than
      // I guess we have to find one next time
      if(nextBestTriIdx < 0 && nextNextBestTriIdx > -1)
      {
         if(!triangleData[nextNextBestTriIdx].isInList)
         {
            nextBestTriIdx = nextNextBestTriIdx;
            nextBestTriScore = nextNextBestTriScore;
            _VALIDATE_TRI_IDX(nextNextBestTriIdx);
         }

         // Nuke the next-next best
         nextNextBestTriIdx = -1;
         nextNextBestTriScore = -1.0f;
      }

      // Validate triangle we are marking as next-best
      _VALIDATE_TRI_IDX(nextBestTriIdx);
   }

#undef _CHECK_NEXT_BEST
#undef _CHECK_NEXT_NEXT_BEST
#undef _VALIDATE_TRI_IDX

   // FrameTemp will call destructInPlace to clean up vertex lists
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

LRUCacheModel::~LRUCacheModel()
{
   for( LRUCacheEntry* entry = mCacheHead; entry != NULL; )
   {
      LRUCacheEntry* next = entry->next;
      delete entry;
      entry = next;
   }
}

//------------------------------------------------------------------------------

void LRUCacheModel::useVertex(const U32 vIdx, VertData *vData)
{
   LRUCacheEntry *search = mCacheHead;
   LRUCacheEntry *last = NULL;

   while(search != NULL)
   {
      if(search->vIdx == vIdx)
         break;

      last = search;
      search = search->next;
   }

   // If this vertex wasn't found in the cache, create a new entry
   if(search == NULL)
   {
      search = new LRUCacheEntry;
      search->vIdx = vIdx;
      search->vData = vData;
   }

   if(search != mCacheHead)
   {
      // Unlink the entry from the linked list
      if(last)
         last->next = search->next;

      // Vertex that got passed in is now at the head of the cache
      search->next = mCacheHead;
      mCacheHead = search;
   }
}

//------------------------------------------------------------------------------

void LRUCacheModel::enforceSize(const dsize_t maxSize, Vector<U32> &outTrisToUpdate)
{
   // Clear list of triangles to update scores for
   outTrisToUpdate.clear();

   dsize_t length = 0;
   LRUCacheEntry *next = mCacheHead;
   LRUCacheEntry *last = NULL;
   
   // Run through list, up to the max size
   while(next != NULL && length < MaxSizeVertexCache)
   {
      VertData &vData = *next->vData;

      // Update cache position on verts still in cache
      vData.cachePosition = length++;

      for(S32 i = 0; i < vData.numReferences; i++)
      {
         const S32 &triIdx = vData.triIndex[i];
         if(triIdx > -1)
         {
            S32 j = 0;
            for(; j < outTrisToUpdate.size(); j++)
               if(outTrisToUpdate[j] == triIdx)
                  break;
            if(j == outTrisToUpdate.size())
               outTrisToUpdate.push_back(triIdx);
         }
      }

      // Update score
      vData.score = FindVertexScore::score(vData);

      last = next;
      next = next->next;
   }

   // NULL out the pointer to the next entry on the last valid entry
   last->next = NULL;

   // If next != NULL, than we need to prune entries from the tail of the cache
   while(next != NULL)
   {
      // Update cache position on verts which are going to get tossed from cache
      next->vData->cachePosition = -1;

      LRUCacheEntry *curEntry = next;
      next = next->next;

      delete curEntry;
   }
}

//------------------------------------------------------------------------------

S32 LRUCacheModel::getCachePosition(const U32 vIdx)
{
   dsize_t length = 0;
   LRUCacheEntry *next = mCacheHead;
   while(next != NULL)
   {
      if(next->vIdx == vIdx)
         return length;
      
      length++;
      next = next->next;
   }

   return -1;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

// http://home.comcast.net/~tom_forsyth/papers/fast_vert_cache_opt.html
namespace FindVertexScore
{

F32 score(const VertData &vertexData)
{
   // If nobody needs this vertex, return -1.0
   if(vertexData.numUnaddedReferences < 1)
      return -1.0f;

   F32 Score = 0.0f;

   if(vertexData.cachePosition < 0)
   {
      // Vertex is not in FIFO cache - no score.
   }
   else
   {
      if(vertexData.cachePosition < 3)
      {
         // This vertex was used in the last triangle,
         // so it has a fixed score, whichever of the three
         // it's in. Otherwise, you can get very different
         // answers depending on whether you add
         // the triangle 1,2,3 or 3,1,2 - which is silly.
         Score = FindVertexScore::LastTriScore;
      }
      else
      {
         AssertFatal(vertexData.cachePosition < MaxSizeVertexCache, "Out of range cache position for vertex");

         // Points for being high in the cache.
         const F32 Scaler = 1.0f / (MaxSizeVertexCache - 3);
         Score = 1.0f - (vertexData.cachePosition - 3) * Scaler;
         Score = mPow(Score, FindVertexScore::CacheDecayPower);
      }
   }


   // Bonus points for having a low number of tris still to
   // use the vert, so we get rid of lone verts quickly.
   F32 ValenceBoost = mPow(vertexData.numUnaddedReferences, -FindVertexScore::ValenceBoostPower);
   Score += FindVertexScore::ValenceBoostScale * ValenceBoost;

   return Score;
}

} // namspace FindVertexScore

} // namespace TriListOpt
