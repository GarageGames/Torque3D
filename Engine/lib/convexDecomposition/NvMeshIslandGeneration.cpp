/*

NvMeshIslandGeneration.cpp : This code snippet walks the toplogy of a triangle mesh and detects the set of unique connected 'mesh islands'

*/

/*!
**
** Copyright (c) 2009 by John W. Ratcliff mailto:jratcliffscarab@gmail.com
**
** Portions of this source has been released with the PhysXViewer application, as well as
** Rocket, CreateDynamics, ODF, and as a number of sample code snippets.
**
** If you find this code useful or you are feeling particularily generous I would
** ask that you please go to http://www.amillionpixels.us and make a donation
** to Troy DeMolay.
**
** DeMolay is a youth group for young men between the ages of 12 and 21.
** It teaches strong moral principles, as well as leadership skills and
** public speaking.  The donations page uses the 'pay for pixels' paradigm
** where, in this case, a pixel is only a single penny.  Donations can be
** made for as small as $4 or as high as a $100 block.  Each person who donates
** will get a link to their own site as well as acknowledgement on the
** donations blog located here http://www.amillionpixels.blogspot.com/
**
** If you wish to contact me you can use the following methods:
**
** Skype ID: jratcliff63367
** Yahoo: jratcliff63367
** AOL: jratcliff1961
** email: jratcliffscarab@gmail.com
**
**
** The MIT license:
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is furnished
** to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.

** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
** WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#pragma warning(disable:4100 4288)
#include "NvMeshIslandGeneration.h"
#include "NvFloatMath.h"
#include "NvHashMap.h"

namespace CONVEX_DECOMPOSITION
{

typedef CONVEX_DECOMPOSITION::Array< NxU32 > NxU32Vector;

class Edge;
class Island;

class AABB
{
public:
  NxF32 mMin[3];
  NxF32 mMax[3];
};

class Triangle
{
public:
  Triangle(void)
  {
    mConsumed = false;
    mIsland   = 0;
    mHandle   = 0;
    mId       = 0;
  }

  void minmax(const NxF32 *p,AABB &box)
  {
    if ( p[0] < box.mMin[0] ) box.mMin[0] = p[0];
    if ( p[1] < box.mMin[1] ) box.mMin[1] = p[1];
    if ( p[2] < box.mMin[2] ) box.mMin[2] = p[2];

    if ( p[0] > box.mMax[0] ) box.mMax[0] = p[0];
    if ( p[1] > box.mMax[1] ) box.mMax[1] = p[1];
    if ( p[2] > box.mMax[2] ) box.mMax[2] = p[2];
  }

  void minmax(const NxF64 *p,AABB &box)
  {
    if ( (NxF32)p[0] < box.mMin[0] ) box.mMin[0] = (NxF32)p[0];
    if ( (NxF32)p[1] < box.mMin[1] ) box.mMin[1] = (NxF32)p[1];
    if ( (NxF32)p[2] < box.mMin[2] ) box.mMin[2] = (NxF32)p[2];
    if ( (NxF32)p[0] > box.mMax[0] ) box.mMax[0] = (NxF32)p[0];
    if ( (NxF32)p[1] > box.mMax[1] ) box.mMax[1] = (NxF32)p[1];
    if ( (NxF32)p[2] > box.mMax[2] ) box.mMax[2] = (NxF32)p[2];
  }

  void buildBox(const NxF32 *vertices_f,const NxF64 *vertices_d,NxU32 id);

  void render(NxU32 color)
  {
//    gRenderDebug->DebugBound(&mBox.mMin[0],&mBox.mMax[0],color,60.0f);
  }

  void getTriangle(NxF32 *tri,const NxF32 *vertices_f,const NxF64 *vertices_d);

  NxU32    mHandle;
  bool      mConsumed;
  Edge     *mEdges[3];
  Island   *mIsland;   // identifies which island it is a member of
  unsigned short  mId;
  AABB      mBox;
};


class Edge
{
public:
  Edge(void)
  {
    mI1 = 0;
    mI2 = 0;
    mHash = 0;
    mNext = 0;
    mPrevious = 0;
    mParent = 0;
    mNextTriangleEdge = 0;
  }

  void init(NxU32 i1,NxU32 i2,Triangle *parent)
  {
    assert( i1 < 65536 );
    assert( i2 < 65536 );

    mI1 = i1;
    mI2 = i2;
    mHash        = (i2<<16)|i1;
    mReverseHash = (i1<<16)|i2;
    mNext = 0;
    mPrevious = 0;
    mParent = parent;
  }

  NxU32  mI1;
  NxU32  mI2;
  NxU32  mHash;
  NxU32  mReverseHash;

  Edge     *mNext;
  Edge     *mPrevious;
  Edge     *mNextTriangleEdge;
  Triangle *mParent;
};

typedef CONVEX_DECOMPOSITION::HashMap< NxU32, Edge * > EdgeHashMap;
typedef CONVEX_DECOMPOSITION::Array< Triangle * > TriangleVector;

class EdgeCheck
{
public:
  EdgeCheck(Triangle *t,Edge *e)
  {
    mTriangle = t;
    mEdge     = e;
  }

  Triangle  *mTriangle;
  Edge      *mEdge;
};

typedef CONVEX_DECOMPOSITION::Array< EdgeCheck > EdgeCheckQueue;

class Island 
{
public:
  Island(Triangle *t,Triangle *root)
  {
    mVerticesFloat = 0;
    mVerticesDouble = 0;
    t->mIsland = this;
    mTriangles.pushBack(t);
    mCoplanar = false;
    fm_initMinMax(mMin,mMax);
  }

  void add(Triangle *t,Triangle *root)
  {
    t->mIsland = this;
    mTriangles.pushBack(t);
  }

  void merge(Island &isl)
  {
    TriangleVector::Iterator i;
    for (i=isl.mTriangles.begin(); i!=isl.mTriangles.end(); ++i)
    {
      Triangle *t = (*i);
      mTriangles.pushBack(t);
    }
    isl.mTriangles.clear();
  }

  bool isTouching(Island *isl,const NxF32 *vertices_f,const NxF64 *vertices_d)
  {
    bool ret = false;

    mVerticesFloat = vertices_f;
    mVerticesDouble = vertices_d;

    if ( fm_intersectAABB(mMin,mMax,isl->mMin,isl->mMax) ) // if the two islands has an intersecting AABB
    {
      // todo..
    }


    return ret;
  }


  void SAP_DeletePair(const void* object0, const void* object1, void* user_data, void* pair_user_data)
  {
  }

  void render(NxU32 color)
  {
//    gRenderDebug->DebugBound(mMin,mMax,color,60.0f);
    TriangleVector::Iterator i;
    for (i=mTriangles.begin(); i!=mTriangles.end(); ++i)
    {
      Triangle *t = (*i);
      t->render(color);
    }
  }


  const NxF64   *mVerticesDouble;
  const NxF32    *mVerticesFloat;

  NxF32           mMin[3];
  NxF32           mMax[3];
  bool            mCoplanar; // marked as co-planar..
  TriangleVector  mTriangles;
};


void Triangle::getTriangle(NxF32 *tri,const NxF32 *vertices_f,const NxF64 *vertices_d)
{
  NxU32 i1 = mEdges[0]->mI1;
  NxU32 i2 = mEdges[1]->mI1;
  NxU32 i3 = mEdges[2]->mI1;
  if ( vertices_f )
  {
    const NxF32 *p1 = &vertices_f[i1*3];
    const NxF32 *p2 = &vertices_f[i2*3];
    const NxF32 *p3 = &vertices_f[i3*3];
    fm_copy3(p1,tri);
    fm_copy3(p2,tri+3);
    fm_copy3(p3,tri+6);
  }
  else
  {
    const NxF64 *p1 = &vertices_d[i1*3];
    const NxF64 *p2 = &vertices_d[i2*3];
    const NxF64 *p3 = &vertices_d[i3*3];
    fm_doubleToFloat3(p1,tri);
    fm_doubleToFloat3(p2,tri+3);
    fm_doubleToFloat3(p3,tri+6);
  }
}

void Triangle::buildBox(const NxF32 *vertices_f,const NxF64 *vertices_d,NxU32 id)
{
  mId = (unsigned short)id;
  NxU32 i1 = mEdges[0]->mI1;
  NxU32 i2 = mEdges[1]->mI1;
  NxU32 i3 = mEdges[2]->mI1;

  if ( vertices_f )
  {
    const NxF32 *p1 = &vertices_f[i1*3];
    const NxF32 *p2 = &vertices_f[i2*3];
    const NxF32 *p3 = &vertices_f[i3*3];
    mBox.mMin[0] = p1[0];
    mBox.mMin[1] = p1[1];
    mBox.mMin[2] = p1[2];
    mBox.mMax[0] = p1[0];
    mBox.mMax[1] = p1[1];
    mBox.mMax[2] = p1[2];
    minmax(p2,mBox);
    minmax(p3,mBox);
  }
  else
  {
    const NxF64 *p1 = &vertices_d[i1*3];
    const NxF64 *p2 = &vertices_d[i2*3];
    const NxF64 *p3 = &vertices_d[i3*3];
    mBox.mMin[0] = (NxF32)p1[0];
    mBox.mMin[1] = (NxF32)p1[1];
    mBox.mMin[2] = (NxF32)p1[2];
    mBox.mMax[0] = (NxF32)p1[0];
    mBox.mMax[1] = (NxF32)p1[1];
    mBox.mMax[2] = (NxF32)p1[2];
    minmax(p2,mBox);
    minmax(p3,mBox);
  }

  assert(mIsland);
  if ( mIsland )
  {
    if ( mBox.mMin[0] < mIsland->mMin[0] ) mIsland->mMin[0] = mBox.mMin[0];
    if ( mBox.mMin[1] < mIsland->mMin[1] ) mIsland->mMin[1] = mBox.mMin[1];
    if ( mBox.mMin[2] < mIsland->mMin[2] ) mIsland->mMin[2] = mBox.mMin[2];

    if ( mBox.mMax[0] > mIsland->mMax[0] ) mIsland->mMax[0] = mBox.mMax[0];
    if ( mBox.mMax[1] > mIsland->mMax[1] ) mIsland->mMax[1] = mBox.mMax[1];
    if ( mBox.mMax[2] > mIsland->mMax[2] ) mIsland->mMax[2] = mBox.mMax[2];
  }

}


typedef CONVEX_DECOMPOSITION::Array< Island * > IslandVector;

class MyMeshIslandGeneration : public MeshIslandGeneration
{
public:
  MyMeshIslandGeneration(void)
  {
    mTriangles = 0;
    mEdges     = 0;
    mVerticesDouble = 0;
    mVerticesFloat  = 0;
  }

  ~MyMeshIslandGeneration(void)
  {
    reset();
  }

  void reset(void)
  {
    delete []mTriangles;
    delete []mEdges;
    mTriangles = 0;
    mEdges = 0;
    mTriangleEdges.clear();
    IslandVector::Iterator i;
    for (i=mIslands.begin(); i!=mIslands.end(); ++i)
    {
      Island *_i = (*i);
      delete _i;
    }
    mIslands.clear();
  }

  NxU32 islandGenerate(NxU32 tcount,const NxU32 *indices,const NxF64 *vertices)
  {
    mVerticesDouble = vertices;
    mVerticesFloat  = 0;
    return islandGenerate(tcount,indices);
  }

  NxU32 islandGenerate(NxU32 tcount,const NxU32 *indices,const NxF32 *vertices)
  {
    mVerticesDouble = 0;
    mVerticesFloat  = vertices;
    return islandGenerate(tcount,indices);
  }

  NxU32 islandGenerate(NxU32 tcount,const NxU32 *indices)
  {
    NxU32 ret = 0;

    reset();

    mTcount = tcount;
    mTriangles = new Triangle[tcount];
    mEdges     = new Edge[tcount*3];
    Edge *e = mEdges;

    for (NxU32 i=0; i<tcount; i++)
    {
      Triangle &t = mTriangles[i];

      NxU32 i1 = *indices++;
      NxU32 i2 = *indices++;
      NxU32 i3 = *indices++;

      t.mEdges[0] = e;
      t.mEdges[1] = e+1;
      t.mEdges[2] = e+2;

      e = addEdge(e,&t,i1,i2);
      e = addEdge(e,&t,i2,i3);
      e = addEdge(e,&t,i3,i1);

    }

    // while there are still edges to process...
    while ( mTriangleEdges.size() != 0 )
    {

      EdgeHashMap::Iterator iter = mTriangleEdges.getIterator();

      Triangle *t = iter->second->mParent;

      Island *i = new Island(t,mTriangles);  // the initial triangle...
      removeTriangle(t); // remove this triangle from the triangle-edges hashmap

      mIslands.pushBack(i);

      // now keep adding to this island until we can no longer walk any shared edges..
      addEdgeCheck(t,t->mEdges[0]);
      addEdgeCheck(t,t->mEdges[1]);
      addEdgeCheck(t,t->mEdges[2]);

      while ( !mEdgeCheckQueue.empty() )
      {

        EdgeCheck e = mEdgeCheckQueue.popBack();

        // Process all triangles which share this edge
        Edge *edge = locateSharedEdge(e.mEdge);

        while ( edge )
        {
          Triangle *t = edge->mParent;
          assert(!t->mConsumed);
          i->add(t,mTriangles);
          removeTriangle(t); // remove this triangle from the triangle-edges hashmap

          // now keep adding to this island until we can no longer walk any shared edges..

          if ( edge != t->mEdges[0] )
          {
            addEdgeCheck(t,t->mEdges[0]);
          }

          if ( edge != t->mEdges[1] )
          {
            addEdgeCheck(t,t->mEdges[1]);
          }

          if ( edge != t->mEdges[2] )
          {
            addEdgeCheck(t,t->mEdges[2]);
          }

          edge = locateSharedEdge(e.mEdge); // keep going until all shared edges have been processed!
        }

      }
    }

    ret = (NxU32)mIslands.size();

    return ret;
  }

  NxU32 *   getIsland(NxU32 index,NxU32 &otcount)
  {
    NxU32 *ret  = 0;

    mIndices.clear();
    if ( index < mIslands.size() )
    {
      Island *i = mIslands[index];
      otcount = (NxU32)i->mTriangles.size();
      TriangleVector::Iterator j;
      for (j=i->mTriangles.begin(); j!=i->mTriangles.end(); ++j)
      {
        Triangle *t = (*j);
        mIndices.pushBack(t->mEdges[0]->mI1);
        mIndices.pushBack(t->mEdges[1]->mI1);
        mIndices.pushBack(t->mEdges[2]->mI1);
      }
      ret = &mIndices[0];
    }

    return ret;
  }

private:

  void removeTriangle(Triangle *t)
  {
    t->mConsumed = true;

    removeEdge(t->mEdges[0]);
    removeEdge(t->mEdges[1]);
    removeEdge(t->mEdges[2]);

  }


  Edge * locateSharedEdge(Edge *e)
  {
    Edge *ret = 0;

    const EdgeHashMap::Entry *found = mTriangleEdges.find( e->mReverseHash );
    if ( found != NULL )
    {
      ret = (*found).second;
      assert( ret->mHash == e->mReverseHash );
    }
    return ret;
  }

  void removeEdge(Edge *e)
  {
    const EdgeHashMap::Entry *found = mTriangleEdges.find( e->mHash );

    if ( found != NULL )
    {
      Edge *prev = 0;
      Edge *scan = (*found).second;
      while ( scan && scan != e )
      {
        prev = scan;
        scan = scan->mNextTriangleEdge;
      }

      if ( scan )
      {
        if ( prev == 0 )
        {
          if ( scan->mNextTriangleEdge )
          {
            mTriangleEdges.erase(e->mHash);
            mTriangleEdges[e->mHash] = scan->mNextTriangleEdge;
          }
          else
          {
            mTriangleEdges.erase(e->mHash); // no more polygons have an edge here
          }
        }
        else
        {
          prev->mNextTriangleEdge = scan->mNextTriangleEdge;
        }
      }
      else
      {
        assert(0);
      }
    }
    else
    {
      assert(0); // impossible!
    }
  }


  Edge * addEdge(Edge *e,Triangle *t,NxU32 i1,NxU32 i2)
  {

    e->init(i1,i2,t);

    const EdgeHashMap::Entry *found = mTriangleEdges.find(e->mHash);
    if ( found == NULL )
    {
      mTriangleEdges[ e->mHash ] = e;
    }
    else
    {
      Edge *pn = (*found).second;
      e->mNextTriangleEdge = pn;
      mTriangleEdges.erase(e->mHash);
      mTriangleEdges[e->mHash] = e;
    }

    e++;

    return e;
  }

  void addEdgeCheck(Triangle *t,Edge *e)
  {
    EdgeCheck ec(t,e);
    mEdgeCheckQueue.pushBack(ec);
  }

  NxU32 mergeCoplanarIslands(const NxF32 *vertices)
  {
    mVerticesFloat = vertices;
    mVerticesDouble = 0;
    return mergeCoplanarIslands();
  }

  NxU32 mergeCoplanarIslands(const NxF64 *vertices)
  {
    mVerticesDouble = vertices;
    mVerticesFloat = 0;
    return mergeCoplanarIslands();
  }

  // this island needs to be merged
  void mergeTouching(Island *isl)
  {
    Island *touching = 0;

    IslandVector::Iterator i;
    for (i=mIslands.begin(); i!=mIslands.end(); ++i)
    {
      Island *_i = (*i);
      if ( !_i->mCoplanar ) // can't merge with coplanar islands!
      {
        if ( _i->isTouching(isl,mVerticesFloat,mVerticesDouble) )
        {
          touching = _i;
        }
      }
    }
  }

  NxU32 mergeCoplanarIslands(void)
  {
    NxU32  ret = 0;

    if ( !mIslands.empty() )
    {


      NxU32  cp_count  = 0;
      NxU32  npc_count = 0;

      NxU32  count = (NxU32)mIslands.size();

      for (NxU32 i=0; i<count; i++)
      {

        NxU32 otcount;
        const NxU32 *oindices = getIsland(i,otcount);

        if ( otcount )
        {

          bool isCoplanar;

          if ( mVerticesFloat )
            isCoplanar = fm_isMeshCoplanar(otcount, oindices, mVerticesFloat, true);
          else
            isCoplanar = fm_isMeshCoplanar(otcount, oindices, mVerticesDouble, true);

          if ( isCoplanar )
          {
            Island *isl = mIslands[i];
            isl->mCoplanar = true;
            cp_count++;
          }
          else
          {
            npc_count++;
          }
        }
        else
        {
          assert(0);
        }
      }

      if ( cp_count )
      {
        if ( npc_count == 0 ) // all islands are co-planar!
        {
          IslandVector temp = mIslands;
          mIslands.clear();
          Island *isl = mIslands[0];
          mIslands.pushBack(isl);
          for (NxU32 i=1; i<cp_count; i++)
          {
            Island *_i = mIslands[i];
            isl->merge(*_i);
            delete _i;
          }
        }
        else
        {


          Triangle *t = mTriangles;
          for (NxU32 i=0; i<mTcount; i++)
          {
            t->buildBox(mVerticesFloat,mVerticesDouble,i);
            t++;
          }

          IslandVector::Iterator i;
          for (i=mIslands.begin(); i!=mIslands.end(); ++i)
          {
            Island *isl = (*i);

            NxU32 color = 0x00FF00;

            if ( isl->mCoplanar )
            {
              color = 0xFFFF00;
            }

            mergeTouching(isl);

          }

          IslandVector temp = mIslands;
          mIslands.clear();
          for (i=temp.begin(); i!=temp.end(); i++)
          {
            Island *isl = (*i);
            if ( isl->mCoplanar )
            {
              delete isl; // kill it
            }
            else
            {
              mIslands.pushBack(isl);
            }
          }
          ret = (NxU32)mIslands.size();
        }
      }
      else
      {
        ret = npc_count;
      }
    }


    return ret;
  }

  NxU32 mergeTouchingIslands(const NxF32 *vertices)
  {
    NxU32 ret = 0;

    return ret;
  }

  NxU32 mergeTouchingIslands(const NxF64 *vertices)
  {
    NxU32 ret = 0;

    return ret;
  }

  NxU32           mTcount;
  Triangle        *mTriangles;
  Edge            *mEdges;
  EdgeHashMap      mTriangleEdges;
  IslandVector     mIslands;
  EdgeCheckQueue   mEdgeCheckQueue;
  const NxF64    *mVerticesDouble;
  const NxF32     *mVerticesFloat;
  NxU32Vector     mIndices;
};


MeshIslandGeneration * createMeshIslandGeneration(void)
{
  MyMeshIslandGeneration *mig = new MyMeshIslandGeneration;
  return static_cast< MeshIslandGeneration *>(mig);
}

void                   releaseMeshIslandGeneration(MeshIslandGeneration *cm)
{
  MyMeshIslandGeneration *mig = static_cast< MyMeshIslandGeneration *>(cm);
  delete mig;
}

}; // end of namespace

