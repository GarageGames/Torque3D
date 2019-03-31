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

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
//
// Adapted to a 2D test for intersecting atlas triangles with zodiacs.
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

//-----------------------------------------------------------------------------
// AABB-triangle overlap test code originally by Tomas Akenine-Möller
//               Assisted by Pierre Terdiman and David Hunt
// http://www.cs.lth.se/home/Tomas_Akenine_Moller/code/
// Ported to TSE by BJG, 2005-4-14
//-----------------------------------------------------------------------------

#include "afx/arcaneFX.h"
#include "afx/util/afxTriBoxCheck2D_T3D.h"

#define FINDMINMAX(x0,x1,x2,min,max) \
   min = max = x0;   \
   if(x1<min) min=x1;\
   if(x1>max) max=x1;\
   if(x2<min) min=x2;\
   if(x2>max) max=x2;

/*======================== Z-tests ========================*/

#define AXISTEST_Z12(a, b, fa, fb)			   \
   p1 = a*v1.x - b*v1.y;			           \
   p2 = a*v2.x - b*v2.y;			       	   \
   if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
   rad = fa * boxhalfsize.x + fb * boxhalfsize.y;   \
   if(min>rad || max<-rad) return false;

#define AXISTEST_Z0(a, b, fa, fb)			   \
   p0 = a*v0.x - b*v0.y;				   \
   p1 = a*v1.x - b*v1.y;			           \
   if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
   rad = fa * boxhalfsize.x + fb * boxhalfsize.y;   \
   if(min>rad || max<-rad) return false;

bool afxTriBoxOverlap2D(const Point3F& boxcenter, const Point3F& boxhalfsize, const Point3F& a, const Point3F& b, const Point3F& c)
{
   /*    use separating axis theorem to test overlap between triangle and box */
   /*    need to test for overlap in these directions: */
   /*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
   /*       we do not even need to test these) */
   /*    2) normal of the triangle */
   /*    3) crossproduct(edge from tri, {x,y,z}-directin) */
   /*       this gives 3x3=9 more tests */

   F32 min,max,p0,p1,p2,rad;

   /* move everything so that the boxcenter is in (0,0,0) */
   Point3F v0 = a - boxcenter;
   Point3F v1 = b - boxcenter;
   Point3F v2 = c - boxcenter;

   /* compute triangle edges */
   Point3F e0 = v1 - v0;      /* tri edge 0 */
   Point3F e1 = v2 - v1;      /* tri edge 1 */
   Point3F e2 = v0 - v2;      /* tri edge 2 */

   /* Bullet 3:  */
   /*  test the 3 tests first */
   F32 fex = mFabs(e0.x);
   F32 fey = mFabs(e0.y);
   AXISTEST_Z12(e0.y, e0.x, fey, fex);

   fex = mFabs(e1.x);
   fey = mFabs(e1.y);
   AXISTEST_Z0(e1.y, e1.x, fey, fex);

   fex = mFabs(e2.x);
   fey = mFabs(e2.y);
   AXISTEST_Z12(e2.y, e2.x, fey, fex);

   /* Bullet 1: */
   /*  first test overlap in the {x,y,z}-directions */
   /*  find min, max of the triangle each direction, and test for overlap in */
   /*  that direction -- this is equivalent to testing a minimal AABB around */
   /*  the triangle against the AABB */

   /* test in X-direction */
   FINDMINMAX(v0.x,v1.x,v2.x,min,max);
   if(min>boxhalfsize.x || max<-boxhalfsize.x) return false;

   /* test in Y-direction */
   FINDMINMAX(v0.y,v1.y,v2.y,min,max);
   if(min>boxhalfsize.y || max<-boxhalfsize.y) return false;

   return true; /* box and triangle overlaps */
}

