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
#include "math/mMath.h"
#include "console/console.h"
#include "collision/depthSortList.h"
#include "core/color.h"
#include "core/stream/fileStream.h" // TODO, remove this

//----------------------------------------------------------------------------

// some defines and global parameters that affect poly split routine
F32 SPLIT_TOL = 0.0005f;
bool ALWAYS_RETURN_FRONT_AND_BACK = false; // if false, split routine will return polys only if a split occurs

// more global parameters
F32 XZ_TOL    = 0.0f;
F32 DEPTH_TOL = 0.01f;
#define MIN_Y_DOT 0.05f
DepthSortList * gCurrentSort = NULL;
S32 gForceOverlap = -1; // force an overlap test to result in an overlap
S32 gNoOverlapCount;
S32 gBadSpots = 0;

//----------------------------------------------------------------------------

DepthSortList::DepthSortList()
{
   VECTOR_SET_ASSOCIATION(mPolyExtentsList);
   VECTOR_SET_ASSOCIATION(mPolyIndexList);
}

DepthSortList::~DepthSortList()
{
}

//----------------------------------------------------------------------------

void DepthSortList::clear()
{
   Parent::clear();

   mPolyExtentsList.clear();
   mPolyIndexList.clear();

   clearSort();
}

void DepthSortList::clearSort()
{
   mBase = -1;
   mMaxTouched = 0;
   gNoOverlapCount = 0;
}

//----------------------------------------------------------------------------

void DepthSortList::end()
{
   S32 numPoly = mPolyList.size();

   if (mPolyList.last().plane.y >= -MIN_Y_DOT)
   {
      mIndexList.setSize(mPolyList.last().vertexStart);
      mPolyList.decrement();
      return;
   }

   Parent::end();

   // we deleted this poly, be sure not to add anything more...
   if (mPolyList.size()!=numPoly)
      return;

   AssertFatal(mPolyList.last().vertexCount>2,"DepthSortList::end: only two vertices in poly");

   mPolyExtentsList.increment();
   setExtents(mPolyList.last(),mPolyExtentsList.last());
   mPolyIndexList.push_back(numPoly-1);
}

//----------------------------------------------------------------------------

bool DepthSortList::getMapping(MatrixF * mat, Box3F * box)
{
   // return list transform and bounds in list space...optional
   *mat = mMatrix;
   mat->inverse();
   box->minExtents.set(-mExtent.x,             0.0f, -mExtent.z);
   box->maxExtents.set( mExtent.x, 2.0f * mExtent.y,  mExtent.z);

   return true;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void DepthSortList::setExtents(Poly & poly, PolyExtents & polyExtents)
{
   Point3F p = mVertexList[mIndexList[poly.vertexStart]].point;
   polyExtents.xMin = polyExtents.xMax = p.x;
   polyExtents.yMin = polyExtents.yMax = p.y;
   polyExtents.zMin = polyExtents.zMax = p.z;
   for (S32 i=poly.vertexStart+1; i<poly.vertexStart+poly.vertexCount; i++)
   {
      Point3F p = mVertexList[mIndexList[i]].point;

      // x
      if (p.x < polyExtents.xMin)
         polyExtents.xMin = p.x;
      else if (p.x > polyExtents.xMax)
         polyExtents.xMax = p.x;

      // y
      if (p.y < polyExtents.yMin)
         polyExtents.yMin = p.y;
      else if (p.y > polyExtents.yMax)
         polyExtents.yMax = p.y;

      // z
      if (p.z < polyExtents.zMin)
         polyExtents.zMin = p.z;
      else if (p.z > polyExtents.zMax)
         polyExtents.zMax = p.z;
   }
}

//----------------------------------------------------------------------------

// function for comparing two poly indices
S32 FN_CDECL compareYExtents( const void* e1, const void* e2)
{
   DepthSortList::PolyExtents & poly1 = gCurrentSort->getExtents(*(U32*)e1);
   DepthSortList::PolyExtents & poly2 = gCurrentSort->getExtents(*(U32*)e2);

   if (poly1.yMin < poly2.yMin)
      return -1;
   if (poly2.yMin < poly1.yMin)
      return 1;
   return 0;
}

//----------------------------------------------------------------------------

void DepthSortList::sortByYExtents()
{
   gCurrentSort = this;
   dQsort(mPolyIndexList.address(),mPolyIndexList.size(),sizeof(U32),compareYExtents);
}

//----------------------------------------------------------------------------

void DepthSortList::set(const MatrixF & mat, Point3F & extents)
{
   setBaseTransform(mat);
   mNormal.set(0,1,0); // ignore polys not facing up...
   mExtent = extents;
   mExtent *= 0.5f;

   // set clip planes
   mPlaneList.clear();

   mPlaneList.increment();
   mPlaneList.last().set(-1.0f, 0.0f, 0.0f);
   mPlaneList.last().d = -mExtent.x;
   mPlaneList.increment();
   mPlaneList.last().set( 1.0f, 0.0f, 0.0f);
   mPlaneList.last().d = -mExtent.x;

   mPlaneList.increment();
   mPlaneList.last().set( 0.0f,-1.0f, 0.0f);
   mPlaneList.last().d = 0;
   mPlaneList.increment();
   mPlaneList.last().set( 0.0f, 1.0f, 0.0f);
   mPlaneList.last().d = -2.0f * mExtent.y;

   mPlaneList.increment();
   mPlaneList.last().set( 0.0f, 0.0f,-1.0f);
   mPlaneList.last().d = -mExtent.z;
   mPlaneList.increment();
   mPlaneList.last().set( 0.0f, 0.0f, 1.0f);
   mPlaneList.last().d = -mExtent.z;
}

//----------------------------------------------------------------------------

void DepthSortList::setBase(S32 base)
{
   mBase = base;
   getOrderedPoly(mBase, &mBasePoly, &mBaseExtents);
   mBaseNormal = &mBasePoly->plane;
   mBaseDot = -mBasePoly->plane.d;
   mBaseYMax = mBaseExtents->yMax;
}

//----------------------------------------------------------------------------

bool DepthSortList::sortNext()
{
   // find the next poly in the depth order
   // NOTE: a closer poly may occur before a farther away poly so long as
   // they don't overlap in the x-z plane...
   if (++mBase>=mPolyIndexList.size())
      return false;

   setBase(mBase);

   gBadSpots = 0;
   ALWAYS_RETURN_FRONT_AND_BACK = false; // split routine will return polys only if a split occurs

   bool switched = false; // haven't switched base poly yet
   S32 i = 0; // currently comparing base to base+i
   Poly * testPoly;
   PolyExtents * testExtents;

   while (mBase+i+1<mPolyIndexList.size())
   {
      i++;

      // get test poly...
      getOrderedPoly(mBase+i,&testPoly,&testExtents);
      Point3F & testNormal = testPoly->plane;
      F32 testDot = -testPoly->plane.d;

      // if base poly's y extents don't overlap test poly's, base poly can stay where it is...
      if (mBase+i>mMaxTouched && mBaseYMax<=testExtents->yMin+DEPTH_TOL)
         break;

      // if base poly and test poly don't have overlapping x & z extents, then order doesn't matter...stay the same
      if (mBaseExtents->xMin>=testExtents->xMax-XZ_TOL || mBaseExtents->xMax<=testExtents->xMin+XZ_TOL ||
          mBaseExtents->zMin>=testExtents->zMax-XZ_TOL || mBaseExtents->zMax<=testExtents->zMin+XZ_TOL)
         continue;

      // is test poly completely behind base poly? if so, order is fine as it is
      S32 v;
      for (v=0; v<testPoly->vertexCount; v++)
         if (mDot(mVertexList[mIndexList[testPoly->vertexStart+v]].point,*mBaseNormal)>mBaseDot+DEPTH_TOL)
            break;
      if (v==testPoly->vertexCount)
         // test behind base
         continue;

      // is base poly completely in front of test poly?  if so, order is fine as it is
      for (v=0; v<mBasePoly->vertexCount; v++)
         if (mDot(mVertexList[mIndexList[mBasePoly->vertexStart+v]].point,testNormal)<testDot-DEPTH_TOL)
            break;
      if (v==mBasePoly->vertexCount)
         // base in front of test
         continue;

      // if the polys don't overlap in the x-z plane, then order doesn't matter, stay as we are
      if (!overlap(mBasePoly,testPoly))
      {
         gNoOverlapCount++;
         if (gNoOverlapCount!=gForceOverlap)
            continue;
      }

      // handle switching/splitting of polys due to overlap
      handleOverlap(testPoly,testNormal,testDot,i,switched);
   }
   return true;
}

//----------------------------------------------------------------------------

void DepthSortList::sort()
{
   // depth sort mPolyIndexList -- entries are indices into mPolyList (where poly is found) & mPolyExtentsList

   // sort by min y extent
   sortByYExtents();

   mBase = -1;

   while (sortNext())
      ;
}

//----------------------------------------------------------------------------

void DepthSortList::handleOverlap(Poly * testPoly, Point3F & testNormal, F32 testDot, S32 & testOffset, bool & switched)
{
   // first reverse the plane tests (i.e., test to see if basePoly behind testPoly or testPoly in front of basePoly...
   // if either succeeds, switch poly
   // if they both fail, split base poly
   // But split anyway if basePoly has already been switched...
   bool doSwitch = false;

   if (!switched)
   {
      S32 v;
      for (v=0; v<mBasePoly->vertexCount; v++)
         if (mDot(mVertexList[mIndexList[mBasePoly->vertexStart+v]].point,testNormal)>testDot+DEPTH_TOL)
            break;
      if (v==mBasePoly->vertexCount)
         doSwitch = true;
      else
      {
         for (v=0; v<testPoly->vertexCount; v++)
            if (mDot(mVertexList[mIndexList[testPoly->vertexStart+v]].point,*mBaseNormal)<mBaseDot-DEPTH_TOL)
               break;
         if (v==testPoly->vertexCount)
            doSwitch = true;
      }
   }

   // try to split base poly along plane of test poly
   Poly frontPoly, backPoly;
   bool splitBase = false, splitTest = false;
   if (!doSwitch)
   {
      splitBase = splitPoly(*mBasePoly,testNormal,testDot,frontPoly,backPoly);
      if (!splitBase)
         // didn't take...no splitting happened...try splitting test poly by base poly
         splitTest = splitPoly(*testPoly,*mBaseNormal,mBaseDot,frontPoly,backPoly);
   }

   U32 testIdx = mPolyIndexList[mBase+testOffset];

   // should we switch order of test and base poly?  Might have to even if we
   // don't want to if there's no splitting to do...
   // Note: possibility that infinite loop can be introduced here...if that happens,
   // then we need to split along edges of polys
   if (doSwitch || (!splitTest && !splitBase))
      {
      if (!doSwitch && gBadSpots++ > (mPolyIndexList.size()-mBase)<<1)
         // got here one too many times...just leave and don't touch poly -- avoid infinite loop
         return;

      // move test poly to the front of the order
      dMemmove(&mPolyIndexList[mBase+1],&mPolyIndexList[mBase],testOffset*sizeof(U32));
      mPolyIndexList[mBase] = testIdx;

      // base poly changed...
      setBase(mBase);

      if (mBase+testOffset>mMaxTouched)
         mMaxTouched=mBase+testOffset;

      testOffset=1; // don't need to compare against old base
      switched=true;
      return;
   }

   if (splitBase)
   {
      // need one more spot...frontPoly and backPoly replace basePoly
      setExtents(frontPoly,mPolyExtentsList[mPolyIndexList[mBase]]);
      mPolyExtentsList.increment();
      setExtents(backPoly,mPolyExtentsList.last());

      mPolyList[mPolyIndexList[mBase]] = frontPoly;
      mPolyIndexList.insert(mBase+1);
      mPolyIndexList[mBase+1] = mPolyList.size();
      mPolyList.push_back(backPoly);

      // new base poly...
      setBase(mBase);

      // increase tsetOffset & mMaxTouched because of insertion of back poly
      testOffset++;
      mMaxTouched++;

      //
      switched=false;
      return;
   }

   // splitTest -- no other way to get here
   AssertFatal(splitTest,"DepthSortList::handleOverlap: how did we get here like this?");

   // put frontPoly in front of basePoly, leave backPoly where testPoly was

   // we need one more spot (testPoly broken into front and back)
   // and we need to shift everything from base up to test down one spot
   mPolyIndexList.insert(mBase);

   // need one more poly for front poly
   mPolyIndexList[mBase] = mPolyList.size();
   mPolyList.push_back(frontPoly);
   mPolyExtentsList.increment();
   setExtents(mPolyList.last(),mPolyExtentsList.last());

   // set up back poly
   mPolyList[testIdx] = backPoly;
   setExtents(mPolyList[testIdx],mPolyExtentsList[testIdx]);

   // new base poly...
   setBase(mBase);

   // we inserted an element, increase mMaxTouched...
   mMaxTouched++;

   testOffset=0;
   switched = false;
}

//----------------------------------------------------------------------------

bool DepthSortList::overlap(Poly * poly1, Poly * poly2)
{
   // check for overlap without any shortcuts
   S32 sz1 = poly1->vertexCount;
   S32 sz2 = poly2->vertexCount;

   Point3F * a1, * b1;
   Point3F * a2, * b2;
   Point2F norm;
   F32 dot;
   b1 = &mVertexList[mIndexList[poly1->vertexStart+sz1-1]].point;
   S32 i;
   for (i=0; i<sz1; i++)
   {
      a1 = b1;
      b1 = &mVertexList[mIndexList[poly1->vertexStart+i]].point;

      // get the mid-point of this edge
      Point3F mid1 = *a1+*b1;
      mid1 *= 0.5f;
      bool midOutside = false;

      b2 = &mVertexList[mIndexList[poly2->vertexStart+sz2-1]].point;
      for (S32 j=0; j<sz2; j++)
      {
         a2 = b2;
         b2 = &mVertexList[mIndexList[poly2->vertexStart+j]].point;

         // test for intersection of a1-b1 and a2-b2 (on x-z plane)

         // they intersect if a1 & b1 are on opp. sides of line a2-b2
         // and a2 & b2 are on opp. sides of line a1-b1

         norm.set(a2->z - b2->z, b2->x - a2->x); // normal to line a2-b2
         dot = norm.x * a2->x + norm.y * a2->z; // dot of a2 and norm
         if (norm.x * mid1.x + norm.y * mid1.z - dot >= 0) // special check for midpoint of line
            midOutside = true;
         if ( ((norm.x * a1->x + norm.y * a1->z) - dot) * ((norm.x * b1->x + norm.y * b1->z) - dot) >= 0 )
            // a1 & b1 are on the same side of the line a2-b2...edges don't overlap
            continue;

         norm.set(a1->z - b1->z, b1->x - a1->x); // normal to line a1-b1
         dot = norm.x * a1->x + norm.y * a1->z; // dot of a1 and norm
         if ( ((norm.x * a2->x + norm.y * a2->z) - dot) * ((norm.x * b2->x + norm.y * b2->z) - dot) >= 0 )
            // a2 & b2 are on the same side of the line a1-b1...edges don't overlap
            continue;

         return true; // edges overlap, so polys overlap
      }
      if (!midOutside)
         return true; // midpoint of a1-b1 is inside the poly
   }

   // edges don't overlap...but one poly might be contained inside the other
   Point3F center = mVertexList[mIndexList[poly2->vertexStart]].point;
   for (i=1; i<sz2; i++)
      center += mVertexList[mIndexList[poly2->vertexStart+i]].point;
   center *= 1.0f / (F32)poly2->vertexCount;
   b1 = &mVertexList[mIndexList[poly1->vertexStart+sz1-1]].point;
   for (i=0; i<sz1; i++)
   {
      a1 = b1;
      b1 = &mVertexList[mIndexList[poly1->vertexStart+i]].point;

      norm.set(a1->z - b1->z, b1->x - a1->x); // normal to line a1-b1
      dot = norm.x * a1->x + norm.y * a1->z; // dot of a1 and norm
      if (center.x * norm.x + center.z * norm.y > dot)
         // center is outside this edge, poly2 is not inside poly1
         break;
   }
   if (i==sz1)
      return true; // first vert of poly2 inside poly1 (so all of poly2 inside poly1)

   center = mVertexList[mIndexList[poly1->vertexStart]].point;
   for (i=1; i<sz1; i++)
      center += mVertexList[mIndexList[poly1->vertexStart+i]].point;
   center *= 1.0f / (F32)poly1->vertexCount;
   b2 = &mVertexList[mIndexList[poly2->vertexStart+sz2-1]].point;
   for (i=0; i<sz2; i++)
   {
      a2 = b2;
      b2 = &mVertexList[mIndexList[poly2->vertexStart+i]].point;

      norm.set(a2->z - b2->z, b2->x - a2->x); // normal to line a2-b2
      dot = norm.x * a2->x + norm.y * a2->z; // dot of a1 and norm
      if (center.x * norm.x + center.z * norm.y > dot)
         // v is outside this edge, poly1 is not inside poly2
         break;
   }
   if (i==sz2)
      return true; // first vert of poly1 inside poly2 (so all of poly1 inside poly2)

   return false; // we survived, no overlap
}

//----------------------------------------------------------------------------

Vector<U32> frontVerts(__FILE__, __LINE__);
Vector<U32> backVerts(__FILE__, __LINE__);

// Split source poly into front and back.  If either front or back is degenerate, don't do anything.
// If we have a front and a back, then add the verts to our vertex list and fill out the poly structures.
bool DepthSortList::splitPoly(const Poly & src, Point3F & normal, F32 k, Poly & frontPoly, Poly & backPoly)
{
   frontVerts.clear();
   backVerts.clear();

   // already degenerate...
   AssertFatal(src.vertexCount>=3,"DepthSortList::splitPoly - Don't need to split a triangle!");

   S32 startSize = mVertexList.size();

	// Assume back and front are degenerate polygons until proven otherwise.
	bool backDegen = true, frontDegen = true;

   U32 bIdx;
   Point3F * a, * b;
   F32 dota, dotb;
   S32 signA, signB;

   F32 splitTolSq = SPLIT_TOL * SPLIT_TOL * mDot(normal,normal);

   bIdx = mIndexList[src.vertexStart+src.vertexCount-1];
   b = &mVertexList[bIdx].point;
   dotb = mDot(normal,*b)-k;

   // Sign variable coded as follows: 1 for outside, 0 on the plane and -1 for inside.
   if (dotb*dotb > splitTolSq)
      signB = dotb > 0.0f ? 1 : -1;
   else
      signB = 0;

   S32 i;
	for (i = 0; i<src.vertexCount; i++)
	{
      a = b;
      bIdx = mIndexList[src.vertexStart+i];
      b = &mVertexList[bIdx].point;
      dota = dotb;
      dotb = mDot(normal,*b)-k;
      signA = signB;
      if (dotb*dotb > splitTolSq)
         signB = dotb > 0.0f ? 1 : -1;
      else
         signB = 0;

      switch(signA*3 + signB + 4) // +4 is to make values go from 0 up...hopefully enticing compiler to make a jump-table
      {
			case 0:		// A-, B-
			case 3:		// A., B-
            backVerts.push_back(bIdx);
				backDegen = false;
				break;
			case 8:		// A+, B+
			case 5:		// A., B+
            frontVerts.push_back(bIdx);
            frontDegen = false;
				break;

			case 1:		// A-, B.
			case 4:		// A., B.
			case 7:		// A+, B.
            backVerts.push_back(bIdx);
            frontVerts.push_back(bIdx);
				break;

			case 2:		// A-, B+
         {
            // intersect line A-B with plane
            F32 dotA = mDot(*a,normal);
            F32 dotB = mDot(*b,normal);
            Vertex v;
            v.point  = *a-*b;
            v.point *= (k-dotB)/(dotA-dotB);
            v.point += *b;
            v.mask = 0;
            frontVerts.push_back(mVertexList.size());
            backVerts.push_back(mVertexList.size());
            frontVerts.push_back(bIdx);
            mVertexList.push_back(v);
            b = &mVertexList[bIdx].point; // better get this pointer again since we just incremented vector
            frontDegen = false;
				break;
         }
			case 6:		// A+, B-
         {
            // intersect line A-B with plane
            F32 dotA = mDot(*a,normal);
            F32 dotB = mDot(*b,normal);
            Vertex v;
            v.point  = *a-*b;
            v.point *= (k-dotB)/(dotA-dotB);
            v.point += *b;
            v.mask = 0;
            frontVerts.push_back(mVertexList.size());
            backVerts.push_back(mVertexList.size());
            backVerts.push_back(bIdx);
            mVertexList.push_back(v);
            b = &mVertexList[bIdx].point; // better get this pointer again since we just incremented vector
            backDegen = false;
				break;
         }
      }
   }

	// Check for degeneracy.

   if (!ALWAYS_RETURN_FRONT_AND_BACK)
   {
      if (frontVerts.size()<3 || backVerts.size()<3 || frontDegen || backDegen)
      {
         // didn't need to be split...return two empty polys
         // and restore vertex list to how it was
         mVertexList.setSize(startSize);
         frontPoly.vertexCount = backPoly.vertexCount = 0;
         return false;
      }
   }
   else
   {
      if (frontDegen)
         frontVerts.clear();
      if (backDegen)
         backVerts.clear();
   }

   // front poly
   frontPoly.plane = src.plane;
   frontPoly.object = src.object;
   frontPoly.material = src.material;
   frontPoly.vertexStart = mIndexList.size();
   frontPoly.vertexCount = frontVerts.size();
   frontPoly.surfaceKey = src.surfaceKey;
   frontPoly.polyFlags = src.polyFlags;

   // back poly
   backPoly.plane = src.plane;
   backPoly.object = src.object;
   backPoly.material = src.material;
   backPoly.vertexStart = frontPoly.vertexStart + frontPoly.vertexCount;
   backPoly.vertexCount = backVerts.size();
   backPoly.surfaceKey = src.surfaceKey;
   backPoly.polyFlags = src.polyFlags;

   // add indices
   mIndexList.setSize(backPoly.vertexStart+backPoly.vertexCount);

   if( frontPoly.vertexCount ) {
	   dMemcpy(&mIndexList[frontPoly.vertexStart],
		   frontVerts.address(),
		   sizeof(U32)*frontPoly.vertexCount);
   }

   if( backPoly.vertexCount ) {
	   dMemcpy(&mIndexList[backPoly.vertexStart],
		   backVerts.address(),
		   sizeof(U32)*backPoly.vertexCount);
   }

   return true;
}

//----------------------------------------------------------------------------

Vector<DepthSortList::Poly> gWorkListA(256, __FILE__, __LINE__ );
Vector<DepthSortList::Poly> gWorkListB(256, __FILE__, __LINE__ );
Vector<DepthSortList::Poly> gWorkListJunkBin(256, __FILE__, __LINE__ );

void DepthSortList::depthPartition(const Point3F * sourceVerts, U32 numVerts, Vector<Poly> & partition, Vector<Point3F> & partitionVerts)
{
   // create the depth partition of the passed poly
   // a depth partition is a partition of the poly on the
   // x-z plane so that each sub-poly in the partition can be
   // mapped onto exactly one plane in the depth list (i.e.,
   // those polys found in mPolyIndexList... the ones that are
   // depth sorted).  The plane the sub-polys are mapped onto
   // is the plane of the closest facing poly.
   //
   // y-coord of input polys are ignored, and are remapped
   // on output to put the output polys on the
   // corresponding planes.

   // This routine is confusing because there are three lists of polys.
   //
   // The source list (passed in as a single poly, but becomes a list as
   // it is split up) comprises the poly to be partitioned.  Verts for sourcePoly
   // are held in sourceVerts when passed to this routine, but immediately copied
   // to mVertexList (and indices are added for each vert to mIndexList).
   //
   // The scraps list is generated from the source poly (it contains the outside
   // piece of each cut that is made).  Indices for polys in the scraps list are
   // found in mIndexList and verts are found in mVerts list.  Note that the depthPartition
   // routine will add verts and indices to the member lists, but not polys.
   //
   // Finally, the partition list is the end result -- the depth partition.  These
   // polys are not indexed polys.  The vertexStart field indexes directly into partitionVerts
   // array.

   if (mBase<0)
      // begin the depth sort
      sortByYExtents();

   // apply cookie cutter to these polys
   Vector<Poly> * sourceList = &gWorkListA;
   sourceList->clear();

   // add source poly for to passed verts
   sourceList->increment();
   sourceList->last().vertexStart = mIndexList.size();
   sourceList->last().vertexCount = numVerts;

   // add verts of source poly to mVertexList and mIndexList
   mVertexList.setSize(mVertexList.size()+numVerts);
   mIndexList.setSize(mIndexList.size()+numVerts);
   for (S32 v=0; v<numVerts; v++)
   {
      mVertexList[mVertexList.size()-numVerts+v].point = sourceVerts[v];
      mIndexList[mIndexList.size()-numVerts+v] = mVertexList.size()-numVerts+v;
   }

   // put scraps from cookie cutter in this list
   Vector<Poly> * scraps = &gWorkListB;
   scraps->clear();

   gWorkListJunkBin.clear();

   S32 i=0;
   while (sourceList->size())
   {
      if (i>=mBase && !sortNext())
         // that's it, no more polys to sort
         break;
      AssertFatal(i<=mBase,"DepthSortList::depthPartition - exceeded mBase.");

      // use the topmost poly as the cookie cutter
      Poly & cutter = mPolyList[mPolyIndexList[i]];
      S32 startVert = partitionVerts.size();

	  bool allowclipping = cutter.polyFlags & CLIPPEDPOLYLIST_FLAG_ALLOWCLIPPING;

      S32 j;
      for (j=0; j<sourceList->size(); j++)
      {
         Poly toCut = (*sourceList)[j];

		 if(allowclipping)
            cookieCutter(cutter,toCut,*scraps,partition,partitionVerts);
		 else
		    cookieCutter(cutter,toCut,gWorkListJunkBin,partition,partitionVerts);
      }

      // project all the new verts onto the cutter's plane
      AssertFatal(mFabs(cutter.plane.y)>=MIN_Y_DOT,"DepthSortList::depthPartition - below MIN_Y_DOT.");
      F32 invY = -1.0f / cutter.plane.y;
      for (j=startVert; j<partitionVerts.size(); j++)
         partitionVerts[j].y = invY * (cutter.plane.d + cutter.plane.x * partitionVerts[j].x + cutter.plane.z * partitionVerts[j].z);

	  if(allowclipping)
	  {
         sourceList->clear();
         // swap work lists -- scraps become source for next closest poly
         Vector<Poly> * tmpListPtr = sourceList;
         sourceList = scraps;
         scraps = tmpListPtr;
	  }
      i++;
   }
}

//----------------------------------------------------------------------------

void DepthSortList::cookieCutter(Poly & cutter, Poly & cuttee,
                                 Vector<Poly> & scraps,                                   // outsides
                                 Vector<Poly> & cookies, Vector<Point3F> & cookieVerts)   // insides
{
   // perhaps going too far with the cookie cutter analogy, but...
   // cutter is used to cut cuttee
   //
   // the part that is inside of all cutter edges (on x-z plane)
   // is put into the "cookie" list, parts that are outside are put
   // onto the "scraps" list.  "scraps" are indexed polys with indices
   // and vertices in mIndexList and mVertexList.  Cookies aren't indexed
   // and points go into cookieVerts list.

   ALWAYS_RETURN_FRONT_AND_BACK = true; // split routine will return polys even if no split occurs

   // save off current state in case nothing inside all the edges of cutter (i.e., no "cookie")
   S32 vsStart  = cuttee.vertexStart;
   S32 vcStart  = cuttee.vertexCount;
   S32 milStart = mIndexList.size();
   S32 mvlStart = mVertexList.size();
   S32 scStart  = scraps.size();

   Point3F a, b;
   Poly scrap;
   a = mVertexList[mIndexList[cutter.vertexStart+cutter.vertexCount-1]].point;
   for (S32 i=0; i<cutter.vertexCount; i++)
   {
      b = mVertexList[mIndexList[cutter.vertexStart+i]].point;
      Point3F n(a.z-b.z,0.0f,b.x-a.x);
      F32 k = mDot(n,a);
      splitPoly(cuttee,n,k,scrap,cuttee);
      if (scrap.vertexCount)
         scraps.push_back(scrap);
      if (!cuttee.vertexCount)
         // cut down to nothing...no need to keep cutting
         break;
      a = b;
   }
   if (cuttee.vertexCount)
   {
      // cuttee is non-degenerate, add it to cookies
      cookies.push_back(cuttee);
      cookies.last().vertexStart = cookieVerts.size();
      for (S32 i=0; i<cuttee.vertexCount; i++)
         cookieVerts.push_back(mVertexList[mIndexList[cuttee.vertexStart+i]].point);
   }
   else
   {
      // no cookie -- leave things as they were (except add cuttee to scraps)
      cuttee.vertexStart = vsStart;
      cuttee.vertexCount = vcStart;
      mIndexList.setSize(milStart);
      mVertexList.setSize(mvlStart);
      scraps.setSize(scStart);
      scraps.push_back(cuttee);
   }
}

//----------------------------------------------------------------------------
