#ifndef DECOMPOSE_POLY_H
#define DECOMPOSE_POLY_H

#include "core/util/tVector.h"
#include "math/mathTypes.h"
#include "math/mPoint3.h"

struct twoIndices{
   U8 i1, i2;

public:
   twoIndices();
   twoIndices(U8 a, U8 b);
   bool operator==(const twoIndices&) const;
};

inline bool twoIndices::operator==(const twoIndices& _test) const
{
   return ((i1 == _test.i1) && (i2 == _test.i2) || (i1 == _test.i2) && (i2 == _test.i1));
}


class decompTri
{
private:

   U8 mVertIdx[3];

public:

   decompTri();
   void setVert(U8 val, U8 idx);
   U8 getVert(U8 idx);
   twoIndices getOtherVerts(U8 idx);
   void orderVerts();
};

class decompPoly
{
private:

   Vector<Point3F>      mVertList;
   Vector<twoIndices>   mEdgeList;
   Vector<U8>           mInsideVerts;
   Vector<decompTri>	   mTris;

   decompTri            mTestTri;

protected:

   void initEdgeList();
   bool sameSide(Point3F &p1, Point3F &p2, Point3F &l1, Point3F &l2);
   bool isInside(decompTri &tri, U8 vertIdx);
   U8 leftmost();
   U8 rightmost();
   U8 uppermost();
   U8 lowermost();
   twoIndices findEdges(U8 idx, bool &notUnique);
   decompTri formTriFromEdges(U8 idx1, U8 idx2);
   twoIndices leftmostInsideVerts(U8 idx1, U8 idx2);
   twoIndices rightmostInsideVerts(U8 idx1, U8 idx2);
   twoIndices uppermostInsideVerts(U8 idx1, U8 idx2);
   twoIndices lowermostInsideVerts(U8 idx1, U8 idx2);
   void findPointsInside();
   void addRemoveEdge(U8 idx1, U8 idx2);
   bool isVertInEdgeList(U8 idx);

public:

   void addVert(Point3F &newVert);
   Point3F getVert(U8 idx);
   void newPoly();
   U8 getNumVerts() { return mVertList.size(); }
   U32 getNumTris() { return mTris.size(); }
   U8 getTriIdx(U32 tri, U8 idx);
   bool checkEdgeLength(F32 len);
   bool decompose();

};

#endif
