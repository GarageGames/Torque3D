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
#include "T3D/gameBase/gameBase.h"
#include "collision/boxConvex.h"
#include "abstractPolyList.h"


//----------------------------------------------------------------------------

static struct Corner {
   S32 a,b,c;
   S32 ab,ac,bc;
} sCorner[] =
{
   { 1,2,4, 4,0,1 },
   { 0,3,5, 4,0,3 },
   { 0,3,6, 4,1,2 },
   { 1,2,7, 4,3,2 },
   { 0,5,6, 0,1,5 },
   { 1,4,7, 0,3,5 },
   { 2,4,7, 1,2,5 },
   { 3,5,6, 3,2,5 },
};

static struct Face {
   S32 vertex[4];
   S32 axis;
   bool flip;
} sFace[] =
{
   { {0,4,5,1}, 1,true  },
   { {0,2,6,4}, 0,true  },
   { {3,7,6,2}, 1,false },
   { {3,1,5,7}, 0,false },
   { {0,1,3,2}, 2,true  },
   { {4,6,7,5}, 2,false },
};

Point3F BoxConvex::support(const VectorF& v) const
{
   Point3F p = mCenter;
   p.x += (v.x >= 0)? mSize.x: -mSize.x;
   p.y += (v.y >= 0)? mSize.y: -mSize.y;
   p.z += (v.z >= 0)? mSize.z: -mSize.z;
   return p;
}

Point3F BoxConvex::getVertex(S32 v)
{
   Point3F p = mCenter;
   p.x += (v & 1)? mSize.x: -mSize.x;
   p.y += (v & 2)? mSize.y: -mSize.y;
   p.z += (v & 4)? mSize.z: -mSize.z;
   return p;
}

inline bool isOnPlane(Point3F p,PlaneF& plane)
{
   F32 dist = mDot(plane,p) + plane.d;
   return dist < 0.1 && dist > -0.1;
}

void BoxConvex::getFeatures(const MatrixF& mat,const VectorF& n, ConvexFeature* cf)
{
   cf->material = 0;
   cf->object = mObject;

   S32 v = 0;
   v += (n.x >= 0)? 1: 0;
   v += (n.y >= 0)? 2: 0;
   v += (n.z >= 0)? 4: 0;

   PlaneF plane;
   plane.set(getVertex(v),n);

   // Emit vertex and edge
   S32 mask = 0;
   Corner& corner = sCorner[v];
   mask |= isOnPlane(getVertex(corner.a),plane)? 1: 0;
   mask |= isOnPlane(getVertex(corner.b),plane)? 2: 0;
   mask |= isOnPlane(getVertex(corner.c),plane)? 4: 0;

   switch(mask) {
      case 0: {
         cf->mVertexList.increment();
         mat.mulP(getVertex(v),&cf->mVertexList.last());
         break;
      }
      case 1:
         emitEdge(v,corner.a,mat,cf);
         break;
      case 2:
         emitEdge(v,corner.b,mat,cf);
         break;
      case 4:
         emitEdge(v,corner.c,mat,cf);
         break;
      case 1 | 2:
         emitFace(corner.ab,mat,cf);
         break;
      case 2 | 4:
         emitFace(corner.bc,mat,cf);
         break;
      case 1 | 4:
         emitFace(corner.ac,mat,cf);
         break;
   }
}

void BoxConvex::getPolyList(AbstractPolyList* list)
{
   list->setTransform(&getTransform(), getScale());
   list->setObject(getObject());

   U32 base = list->addPoint(mCenter + Point3F(-mSize.x, -mSize.y, -mSize.z));
              list->addPoint(mCenter + Point3F( mSize.x, -mSize.y, -mSize.z));
              list->addPoint(mCenter + Point3F(-mSize.x,  mSize.y, -mSize.z));
              list->addPoint(mCenter + Point3F( mSize.x,  mSize.y, -mSize.z));
              list->addPoint(mCenter + Point3F(-mSize.x, -mSize.y,  mSize.z));
              list->addPoint(mCenter + Point3F( mSize.x, -mSize.y,  mSize.z));
              list->addPoint(mCenter + Point3F(-mSize.x,  mSize.y,  mSize.z));
              list->addPoint(mCenter + Point3F( mSize.x,  mSize.y,  mSize.z));

   for (U32 i = 0; i < 6; i++) {
      list->begin(0, i);

      list->vertex(base + sFace[i].vertex[0]);
      list->vertex(base + sFace[i].vertex[1]);
      list->vertex(base + sFace[i].vertex[2]);
      list->vertex(base + sFace[i].vertex[3]);

      list->plane(base + sFace[i].vertex[0],
                  base + sFace[i].vertex[1],
                  base + sFace[i].vertex[2]);
      list->end();
   }
}


void BoxConvex::emitEdge(S32 v1,S32 v2,const MatrixF& mat,ConvexFeature* cf)
{
   S32 vc = cf->mVertexList.size();
   cf->mVertexList.increment(2);
   Point3F *vp = cf->mVertexList.begin();
   mat.mulP(getVertex(v1),&vp[vc]);
   mat.mulP(getVertex(v2),&vp[vc + 1]);

   cf->mEdgeList.increment();
   ConvexFeature::Edge& edge = cf->mEdgeList.last();
   edge.vertex[0] = vc;
   edge.vertex[1] = vc + 1;
}

void BoxConvex::emitFace(S32 fi,const MatrixF& mat,ConvexFeature* cf)
{
   Face& face = sFace[fi];

   // Emit vertices
   S32 vc = cf->mVertexList.size();
   cf->mVertexList.increment(4);
   Point3F *vp = cf->mVertexList.begin();
   for (S32 v = 0; v < 4; v++)
      mat.mulP(getVertex(face.vertex[v]),&vp[vc + v]);

   // Emit edges
   cf->mEdgeList.increment(4);
   ConvexFeature::Edge* edge = cf->mEdgeList.end() - 4;
   for (S32 e = 0; e < 4; e++) {
      edge[e].vertex[0] = vc + e;
      edge[e].vertex[1] = vc + ((e + 1) & 3);
   }

   // Emit 2 triangle faces
   cf->mFaceList.increment(2);
   ConvexFeature::Face* ef = cf->mFaceList.end() - 2;
   mat.getColumn(face.axis,&ef->normal);
   if (face.flip)
      ef[0].normal.neg();
   ef[1].normal = ef[0].normal;
   ef[1].vertex[0] = ef[0].vertex[0] = vc;
   ef[1].vertex[1] = ef[0].vertex[2] = vc + 2;
   ef[0].vertex[1] = vc + 1;
   ef[1].vertex[2] = vc + 3;
}



const MatrixF& OrthoBoxConvex::getTransform() const
{
   Point3F translation;
   Parent::getTransform().getColumn(3, &translation);
   mOrthoMatrixCache.setColumn(3, translation);
   return mOrthoMatrixCache;
}

