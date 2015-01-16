#include "math/util/decomposePoly.h"

// twoIndices Methods

twoIndices::twoIndices()
{
   i1 = i2 = 0;
}

twoIndices::twoIndices(U8 a, U8 b)
{
   i1 = a;
   i2 = b;
}

// decompTri Methods

decompTri::decompTri()
{
   mVertIdx[0] = 0;
   mVertIdx[1] = 0;
   mVertIdx[2] = 0;
}

void decompTri::orderVerts()
{
   // Bubble sort, smallest to largest
   U8 temp;

   for (U8 i = 2; i > 0; i--)
   {
      for (U8 j = 0; j < i; j++)
      {
         if (mVertIdx[j] > mVertIdx[j + 1])
         {
            temp = mVertIdx[j];
            mVertIdx[j] = mVertIdx[j + 1];
            mVertIdx[j + 1] = temp;
         }
      }
   }
}

void decompTri::setVert(U8 val, U8 idx)
{
   if(idx < 3)
      mVertIdx[idx] = val;
}

U8 decompTri::getVert(U8 idx)
{
   if(idx < 3)
      return mVertIdx[idx];

   return mVertIdx[2];
}

twoIndices decompTri::getOtherVerts(U8 idx)
{
   if(idx == mVertIdx[0])
      return twoIndices(mVertIdx[1], mVertIdx[2]);

   if(idx == mVertIdx[1])
      return twoIndices(mVertIdx[0], mVertIdx[2]);

   if(idx == mVertIdx[2])
      return twoIndices(mVertIdx[0], mVertIdx[1]);

   return twoIndices(0,0);
}

// decompPoly Methods

void decompPoly::addVert(Point3F &newVert)
{
   bool found = false;

   for(U8 i = 0; i < mVertList.size(); i++)
   {
      if(newVert == mVertList[i])
         found = true;
   }

   if(!found)
      mVertList.push_back(newVert);
}

void decompPoly::initEdgeList()
{
   mEdgeList.clear();

   S32 next = 0;

   for(S32 i = 0; i < mVertList.size(); i++)
   {
      if(i == mVertList.size()-1)
         next = 0;
      else
         next = i+1;

      mEdgeList.push_back(twoIndices(i, next));
   }
}

bool decompPoly::sameSide(Point3F &p1, Point3F &p2, Point3F &l1, Point3F &l2)
{
   return ((p1.x-l1.x)*(l2.y-l1.y)-(l2.x-l1.x)*(p1.y-l1.y))*((p2.x-l1.x)*(l2.y-l1.y)-(l2.x-l1.x)*(p2.y-l1.y)) > 0;
}

bool decompPoly::isInside(decompTri &tri, U8 vertIdx)
{
   Point3F a(mVertList[tri.getVert(0)]);
   Point3F b(mVertList[tri.getVert(1)]);
   Point3F c(mVertList[tri.getVert(2)]);
   Point3F p(mVertList[vertIdx]);

   return (sameSide(p,a,b,c) && sameSide(p,b,a,c) && sameSide(p,c,a,b));
}

U8 decompPoly::leftmost()
{
   F32 xMin = 9999999.f;
   U8 idx = 0;

   for(U8 i = 0; i < mVertList.size(); i++)
   {
      if(mVertList[i].x < xMin && isVertInEdgeList(i))
      {
         xMin = mVertList[i].x;
         idx = i;
      }
   }

   return idx;
}

U8 decompPoly::rightmost()
{
   F32 xMax = -9999999.f;
   U8 idx = 0;

   for(U8 i = 0; i < mVertList.size(); i++)
   {
      if(mVertList[i].x > xMax && isVertInEdgeList(i))
      {
         xMax = mVertList[i].x;
         idx = i;
      }
   }

   return idx;
}

U8 decompPoly::uppermost()
{
   F32 yMax = -9999999.f;
   U8 idx = 0;

   for(U8 i = 0; i < mVertList.size(); i++)
   {
      if(mVertList[i].y > yMax && isVertInEdgeList(i))
      {
         yMax = mVertList[i].y;
         idx = i;
      }
   }

   return idx;
}

U8 decompPoly::lowermost()
{
   F32 yMin = 9999999.f;
   U8 idx = 0;

   for(U8 i = 0; i < mVertList.size(); i++)
   {
      if(mVertList[i].y < yMin && isVertInEdgeList(i))
      {
         yMin = mVertList[i].y;
         idx = i;
      }
   }

   return idx;
}

twoIndices decompPoly::findEdges(U8 vertIdx, bool &notUnique)
{
   U8 found = 0;
   U8 edgeIdx[2];

   for(U8 i = 0; i < mEdgeList.size(); i++)
   {
      if(mEdgeList[i].i1 == vertIdx || mEdgeList[i].i2 == vertIdx)
      {
         edgeIdx[found++] = i;
      }
   }

   notUnique = found > 2;

   return twoIndices(edgeIdx[0], edgeIdx[1]);
}

decompTri decompPoly::formTriFromEdges(U8 idx1, U8 idx2)
{
   decompTri tri;

   tri.setVert(mEdgeList[idx1].i1, 0);
   tri.setVert(mEdgeList[idx1].i2, 1);

   if(mEdgeList[idx2].i1 == tri.getVert(0) || mEdgeList[idx2].i1 == tri.getVert(1))
   {
      tri.setVert(mEdgeList[idx2].i2, 2);
   }
   else
   {
      tri.setVert(mEdgeList[idx2].i1, 2);
   }

   return tri;
}

twoIndices decompPoly::leftmostInsideVerts(U8 idx1, U8 idx2)
{
   F32 xMin = 9999999.f;
   S32 left[] = {-1, -1};

   for(U8 i = 0; i < 2; i++)
   {
      for(U8 j = 0; j < mInsideVerts.size(); j++)
      {
         if(mVertList[mInsideVerts[j]].x < xMin && mInsideVerts[j] != left[0])
         {
            xMin = mVertList[mInsideVerts[j]].x;
            left[i] = mInsideVerts[j];
         }
      }

      if(mVertList[idx1].x < xMin && idx1 != left[0])
      {
         xMin = mVertList[idx1].x;
         left[i] = idx1;
      }

      if(mVertList[idx2].x < xMin && idx2 != left[0])
      {
         xMin = mVertList[idx2].x;
         left[i] = idx2;
      }

      xMin = 9999999.f;
   }

   return twoIndices(left[0], left[1]);
}

twoIndices decompPoly::rightmostInsideVerts(U8 idx1, U8 idx2)
{
   F32 xMax = -9999999.f;
   S32 right[] = {-1, -1};

   for(U8 i = 0; i < 2; i++)
   {
      for(U8 j = 0; j < mInsideVerts.size(); j++)
      {
         if(mVertList[mInsideVerts[j]].x > xMax && mInsideVerts[j] != right[0])
         {
            xMax = mVertList[mInsideVerts[j]].x;
            right[i] = mInsideVerts[j];
         }
      }

      if(mVertList[idx1].x > xMax && idx1 != right[0])
      {
         xMax = mVertList[idx1].x;
         right[i] = idx1;
      }

      if(mVertList[idx2].x > xMax && idx2 != right[0])
      {
         xMax = mVertList[idx2].x;
         right[i] = idx2;
      }

      xMax = -9999999.f;
   }

   return twoIndices(right[0], right[1]);
}

twoIndices decompPoly::uppermostInsideVerts(U8 idx1, U8 idx2)
{
   F32 yMax = -9999999.f;
   S32 up[] = {-1, -1};

   for(U8 i = 0; i < 2; i++)
   {
      for(U8 j = 0; j < mInsideVerts.size(); j++)
      {
         if(mVertList[mInsideVerts[j]].y > yMax && mInsideVerts[j] != up[0])
         {
            yMax = mVertList[mInsideVerts[j]].y;
            up[i] = mInsideVerts[j];
         }
      }

      if(mVertList[idx1].y > yMax && idx1 != up[0])
      {
         yMax = mVertList[idx1].y;
         up[i] = idx1;
      }

      if(mVertList[idx2].y > yMax && idx2 != up[0])
      {
         yMax = mVertList[idx2].y;
         up[i] = idx2;
      }

      yMax = -9999999.f;
   }

   return twoIndices(up[0], up[1]);
}

twoIndices decompPoly::lowermostInsideVerts(U8 idx1, U8 idx2)
{
   F32 yMin = 9999999.f;
   S32 down[] = {-1, -1};

   for(U8 i = 0; i < 2; i++)
   {
      for(U8 j = 0; j < mInsideVerts.size(); j++)
      {
         if(mVertList[mInsideVerts[j]].y < yMin && mInsideVerts[j] != down[0])
         {
            yMin = mVertList[mInsideVerts[j]].y;
            down[i] = mInsideVerts[j];
         }
      }

      if(mVertList[idx1].y < yMin && idx1 != down[0])
      {
         yMin = mVertList[idx1].y;
         down[i] = idx1;
      }

      if(mVertList[idx2].y < yMin && idx2 != down[0])
      {
         yMin = mVertList[idx2].y;
         down[i] = idx2;
      }

      yMin = 9999999.f;
   }

   return twoIndices(down[0], down[1]);
}

void decompPoly::findPointsInside()
{
   mInsideVerts.clear();

   for(U8 i = 0; i < mVertList.size(); i++)
   {
      if(i != mTestTri.getVert(0) && i != mTestTri.getVert(1) && i != mTestTri.getVert(2))
      {
         if(isInside(mTestTri, i))
         {
            mInsideVerts.push_back(i);
         }
      }
   }
}

void decompPoly::addRemoveEdge(U8 idx1, U8 idx2)
{
   twoIndices edge1(idx1, idx2);

   if(!mEdgeList.remove(edge1))
      mEdgeList.push_back(edge1);
}

void decompPoly::newPoly()
{
   mVertList.clear();
   mEdgeList.clear();
   mInsideVerts.clear();
   mTris.clear();
}

bool decompPoly::isVertInEdgeList(U8 idx)
{
   for(U8 i = 0; i < mEdgeList.size(); i++)
   {
      if(mEdgeList[i].i1 == idx || mEdgeList[i].i2 == idx)
         return true;
   }

   return false;
}

Point3F decompPoly::getVert(U8 idx)
{
   if(idx < mVertList.size())
      return mVertList[idx];

   return Point3F(0.0f, 0.0f, 0.0f);
}

U8 decompPoly::getTriIdx(U32 tri, U8 idx)
{
   if(tri < mTris.size() && idx < 3)
      return mTris[tri].getVert(idx);

   return 0;
}

bool decompPoly::checkEdgeLength(F32 len)
{
   Point3F p1, p2;

   for(U8 i = 0; i < mEdgeList.size(); i++)
   {
      p1 = mVertList[mEdgeList[i].i1];
      p2 = mVertList[mEdgeList[i].i2];

      p1 = p2 - p1;

      if(p1.len() < len)
         return false;
   }

   return true;
}

//---------------------------------------------------
// Concave polygon decomposition into triangles
//
// Based upon this resource:
//   http://www.siggraph.org/education/materials/HyperGraph/scanline/outprims/polygon1.htm
//
// 1. Find leftmost vertex in polygon (smallest value along x-axis).
// 2. Find the two edges adjacent to this vertex.
// 3. Form a test tri by connecting the open side of these two edges.
// 4. See if any of the vertices in the poly, besides the ones that form the test tri, are inside the
//    test tri.
// 5. If there are verts inside, find the 3 leftmost of the inside verts and the test tri verts, form
//    a new test tri from these verts, and repeat from step 4.  When no verts are inside, continue.
// 6. Save test tri as a valid triangle.
// 7. Remove the edges that the test tri and poly share, and add the new edges from the test tri to
//    the poly to form a new closed poly with the test tri subtracted out.
//
// Note: our polygon can contain no more than 255 verts.  Complex polygons can create situations where
// a vertex has more than 2 adjacent edges, causing step 2 to fail.  This method improves on the resource
// listed above by not limiting the decomposition to the leftmost side only.  If step 2 fails, the
// algorithm also tries to decompose the polygon from the top, right, and bottom until one succeeds or
// they all fail.  If they all fail, the polygon is too complex to be decomposed with this method.

bool decompPoly::decompose()
{
   // Must have at least 3 verts to form a poly
   if(mVertList.size() < 3)
      return false;

   // Clear out any previously stored tris
   mTris.clear();

   // Initialize the edge list with the default edges
   initEdgeList();

   twoIndices otherVerts, outerVerts2;
   U32 counter = 0;
   U8 uniqueVertAttempt = 0;
   U8 formTriAttempt = 0;
   bool notUnique = false;

   // The main decomposition loop
   while(mEdgeList.size() > 3)
   {
      // Find outermost vert in poly, LMV
      U8 outerVertIdx;

      switch(uniqueVertAttempt)
      {
      case 0:  outerVertIdx = leftmost();    break;
      case 1:  outerVertIdx = rightmost();   break;
      case 2:  outerVertIdx = uppermost();   break;
      case 3:  outerVertIdx = uppermost();   break;
      default:	outerVertIdx = leftmost();
      }

      // Find edges that share LMV
      twoIndices edgesIdx = findEdges(outerVertIdx, notUnique);

      // If vert shares more than two edges, try decomposing from different direction
      if(notUnique)
      {
         if(uniqueVertAttempt < 4)
         {
            uniqueVertAttempt++;
            continue;
         }
         else
         {
            newPoly();
            return false;
         }
      }

      // Sanity check
      if(edgesIdx.i1 >= mEdgeList.size() || edgesIdx.i2 >= mEdgeList.size())
      {
         newPoly();
         return false;
      }

      // Form the test tri from these 2 edges
      mTestTri = formTriFromEdges(edgesIdx.i1, edgesIdx.i2);

      // See if there are any other poly verts inside the test tri
      findPointsInside();

      // If there are verts inside, repeat procedure until there are none
      while(mInsideVerts.size() > 0 && formTriAttempt++ < mVertList.size())
      {
         // Get the two verts of the test tri that are not the LMV
         otherVerts = mTestTri.getOtherVerts(outerVertIdx);

         // Get the 2 outermost verts of the inside and test tri verts (excluding LMV)
         switch(uniqueVertAttempt)
         {
         case 0:  outerVerts2 = leftmostInsideVerts(otherVerts.i1, otherVerts.i2);  break;
         case 1:  outerVerts2 = rightmostInsideVerts(otherVerts.i1, otherVerts.i2); break;
         case 2:  outerVerts2 = uppermostInsideVerts(otherVerts.i1, otherVerts.i2); break;
         case 3:  outerVerts2 = uppermostInsideVerts(otherVerts.i1, otherVerts.i2); break;
         default: outerVerts2 = leftmostInsideVerts(otherVerts.i1, otherVerts.i2);
			}

         // Form a new test tri from LMV, and the 2 new leftmost verts above
         mTestTri.setVert(outerVerts2.i1, 0);
         mTestTri.setVert(outerVertIdx, 1);
         mTestTri.setVert(outerVerts2.i2, 2);

         // See if there are verts inside this new test tri
         findPointsInside();
      }

      // We have found a valid tri
      mTris.push_back(mTestTri);

      // Remove edges common to test tri and poly... add unique edges from test tri to poly
      addRemoveEdge(mTestTri.getVert(0), mTestTri.getVert(1));
      addRemoveEdge(mTestTri.getVert(1), mTestTri.getVert(2));
      addRemoveEdge(mTestTri.getVert(0), mTestTri.getVert(2));

      // This should never take 255 iterations... we must be stuck in an infinite loop
      if(counter++ > 255)
      {
         newPoly();
         return false;
      }

      // Reset attempts
      formTriAttempt = 0;
      uniqueVertAttempt = 0;
   }

   // The last tri
   mTris.push_back(formTriFromEdges(0,1));

   // The verts need to be ordered to draw correctly
   for(U8 i = 0; i < mTris.size(); i++)
   {
      mTris[i].orderVerts();
   }

   return true;
}
