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

#ifndef _BOXCONVEX_H_
#define _BOXCONVEX_H_

#ifndef _CONVEX_H_
#include "collision/convex.h"
#endif


//----------------------------------------------------------------------------

class BoxConvex: public Convex
{
   Point3F getVertex(S32 v);
   void emitEdge(S32 v1,S32 v2,const MatrixF& mat,ConvexFeature* cf);
   void emitFace(S32 fi,const MatrixF& mat,ConvexFeature* cf);
public:
   //
   Point3F mCenter;
   VectorF mSize;

   BoxConvex() { mType = BoxConvexType; }
   void init(SceneObject* obj) { mObject = obj; }

   Point3F support(const VectorF& v) const;
   void getFeatures(const MatrixF& mat,const VectorF& n, ConvexFeature* cf);
   void getPolyList(AbstractPolyList* list);
};


class OrthoBoxConvex: public BoxConvex
{
   typedef BoxConvex Parent;
   mutable MatrixF mOrthoMatrixCache;

 public:
   OrthoBoxConvex() { mOrthoMatrixCache.identity(); }

   virtual const MatrixF& getTransform() const;
};

#endif
