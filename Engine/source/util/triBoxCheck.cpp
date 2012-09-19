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

//-----------------------------------------------------------------------------
// AABB-triangle overlap test code originally by Tomas Akenine-Möller
//               Assisted by Pierre Terdiman and David Hunt
// http://www.cs.lth.se/home/Tomas_Akenine_Moller/code/
// Ported to TSE by BJG, 2005-4-14
// Modified to avoid a lot of copying by ASM, 2007-9-28
//-----------------------------------------------------------------------------

#include "util/triBoxCheck.h"

#define FINDMINMAX(x0,x1,x2,theMin,theMax) \
   theMin = theMax = x0;   \
   if(x1<theMin) theMin=x1;\
   if(x1>theMax) theMax=x1;\
   if(x2<theMin) theMin=x2;\
   if(x2>theMax) theMax=x2;

static bool planeBoxOverlap(const Point3F &normal, const Point3F &vert, const Point3F &maxbox)
{
   S32 q;
   F32 v;
   Point3F vmin, vmax;

   for(q=0;q<=2;q++)
   {
      v=vert[q];
      if(normal[q]>0.0f)
      {
         vmin[q]=-maxbox[q] - v;
         vmax[q]= maxbox[q] - v;
      }
      else
      {
         vmin[q]= maxbox[q] - v;
         vmax[q]=-maxbox[q] - v;
      }
   }

   if(mDot(normal, vmin) > 0.f)
      return false;

   if(mDot(normal, vmax) >= 0.f)
      return true;

   return false;
}


/*======================== X-tests ========================*/
#define AXISTEST_X01(a, b, fa, fb)			   \
   p0 = a*v0.y - b*v0.z;			       	   \
   p2 = a*v2.y - b*v2.z;			       	   \
   if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
   rad = fa * boxhalfsize.y + fb * boxhalfsize.z;   \
   if(min>rad || max<-rad) return false;

#define AXISTEST_X2(a, b, fa, fb)			   \
   p0 = a*v0.y - b*v0.z;			           \
   p1 = a*v1.y - b*v1.z;			       	   \
   if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
   rad = fa * boxhalfsize.y + fb * boxhalfsize.z;   \
   if(min>rad || max<-rad) return false;

/*======================== Y-tests ========================*/
#define AXISTEST_Y02(a, b, fa, fb)			   \
   p0 = -a*v0.x + b*v0.z;		      	   \
   p2 = -a*v2.x + b*v2.z;	       	       	   \
   if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
   rad = fa * boxhalfsize.x + fb * boxhalfsize.z;   \
   if(min>rad || max<-rad) return false;

#define AXISTEST_Y1(a, b, fa, fb)			   \
   p0 = -a*v0.x + b*v0.z;		      	   \
   p1 = -a*v1.x + b*v1.z;	     	       	   \
   if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
   rad = fa * boxhalfsize.x + fb * boxhalfsize.z;   \
   if(min>rad || max<-rad) return false;

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

bool triBoxOverlap(const Point3F &boxcenter, const Point3F &boxhalfsize, const Point3F triverts[3])
{
   /*    use separating axis theorem to test overlap between triangle and box */
   /*    need to test for overlap in these directions: */
   /*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
   /*       we do not even need to test these) */
   /*    2) normal of the triangle */
   /*    3) crossproduct(edge from tri, {x,y,z}-directin) */
   /*       this gives 3x3=9 more tests */

   Point3F v0,v1,v2;

   F32 min,max,p0,p1,p2,rad,fex,fey,fez;		// -NJMP- "d" local variable removed
   Point3F normal,e0,e1,e2;

   /* This is the fastest branch on Sun */
   /* move everything so that the boxcenter is in (0,0,0) */
   v0 = triverts[0] - boxcenter;
   v1 = triverts[1] - boxcenter;
   v2 = triverts[2] - boxcenter;

   /* compute triangle edges */
   e0 = v1 - v0;      /* tri edge 0 */
   e1 = v2 - v1;      /* tri edge 1 */
   e2 = v0 - v2;      /* tri edge 2 */

   /* Bullet 3:  */
   /*  test the 9 tests first (this was faster) */
   fex = mFabs(e0.x);
   fey = mFabs(e0.y);
   fez = mFabs(e0.z);
   AXISTEST_X01(e0.z, e0.y, fez, fey);
   AXISTEST_Y02(e0.z, e0.x, fez, fex);
   AXISTEST_Z12(e0.y, e0.x, fey, fex);

   fex = mFabs(e1.x);
   fey = mFabs(e1.y);
   fez = mFabs(e1.z);
   AXISTEST_X01(e1.z, e1.y, fez, fey);
   AXISTEST_Y02(e1.z, e1.x, fez, fex);
   AXISTEST_Z0(e1.y, e1.x, fey, fex);

   fex = mFabs(e2.x);
   fey = mFabs(e2.y);
   fez = mFabs(e2.z);
   AXISTEST_X2(e2.z, e2.y, fez, fey);
   AXISTEST_Y1(e2.z, e2.x, fez, fex);
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

   /* test in Z-direction */
   FINDMINMAX(v0.z,v1.z,v2.z,min,max);
   if(min>boxhalfsize.z || max<-boxhalfsize.z) return false;

   /* Bullet 2: */
   /*  test if the box intersects the plane of the triangle */
   /*  compute plane equation of triangle: normal*x+d=0 */
   normal = mCross(e0, e1);

   if(!planeBoxOverlap(normal,v0,boxhalfsize)) return false;

   return true; /* box and triangle overlaps */
}
