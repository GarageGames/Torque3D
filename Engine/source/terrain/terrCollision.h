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

#ifndef _TERRCOLL_H_
#define _TERRCOLL_H_

#ifndef _CONVEX_H_
#include "collision/convex.h"
#endif


class TerrainConvex : public Convex
{
   friend class TerrainBlock;
   TerrainConvex *square;     ///< Alternate convex if square is concave
   bool halfA;                ///< Which half of square
   bool split45;              ///< Square split pattern
   U32 squareId;              ///< Used to match squares
   U32 material;
   Point3F point[4];          ///< 3-4 vertices
   VectorF normal[2];
   Box3F box;                 ///< Bounding box

  public:

   TerrainConvex();
   
   TerrainConvex( const TerrainConvex& cv );

   // Convex
   Box3F getBoundingBox() const;
   Box3F getBoundingBox(const MatrixF& mat, const Point3F& scale) const;
   Point3F support(const VectorF& v) const;
   void getFeatures(const MatrixF& mat,const VectorF& n, ConvexFeature* cf);
   void getPolyList(AbstractPolyList* list);
};

#endif // _TERRCOLL_H_
