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


#include "T3D/cameraSpline.h"

#include "console/console.h"
#include "gfx/gfxDevice.h"


//-----------------------------------------------------------------------------

CameraSpline::Knot::Knot()
{
   mPosition = Point3F::Zero;
   mRotation = QuatF::Identity;
   mSpeed = 0.0f;
   mType = NORMAL;
   mPath = SPLINE;
   prev = NULL; next = NULL;
};

CameraSpline::Knot::Knot(const Knot &k)
{
   mPosition = k.mPosition;
   mRotation = k.mRotation;
   mSpeed    = k.mSpeed;
   mType = k.mType;
   mPath = k.mPath;
   prev = NULL; next = NULL;
}

CameraSpline::Knot::Knot(const Point3F &p, const QuatF &r, F32 s, Knot::Type type, Knot::Path path)
{
   mPosition = p;
   mRotation = r;
   mSpeed    = s;
   mType = type;
   mPath = path;
   prev = NULL; next = NULL;
}


//-----------------------------------------------------------------------------

CameraSpline::CameraSpline()
{
   mFront = NULL;
   mSize = 0;
   mIsMapDirty = true;
   VECTOR_SET_ASSOCIATION(mTimeMap);
}


CameraSpline::~CameraSpline()
{
   removeAll();
}


void CameraSpline::push_back(Knot *w)
{
   if (!mFront)
   {
      mFront = w;
      w->next = w;
      w->prev = w;
   }
   else
   {
      Knot *before = back();
      Knot *after  = before->next;

      w->next = before->next;
      w->prev = before;
      after->prev = w;
      before->next = w;
   }
   ++mSize;
   mIsMapDirty = true;
}

CameraSpline::Knot* CameraSpline::getKnot(S32 i)
{
   Knot *k = mFront;
   while(i--)
      k = k->next;
   return k;
}

CameraSpline::Knot* CameraSpline::remove(Knot *w)
{
   if (w->next == mFront && w->prev == mFront)
      mFront = NULL;
   else
   {
      w->prev->next = w->next;
      w->next->prev = w->prev;
      if (mFront == w)
         mFront = w->next;
   }
   --mSize;
   mIsMapDirty = true;
   return w;
}


void CameraSpline::removeAll()
{
   while(front())
      delete remove(front());
  mSize = 0;
}


//-----------------------------------------------------------------------------

static bool gBuilding = false;

void CameraSpline::buildTimeMap()
{
   if (!mIsMapDirty)
      return;

   gBuilding = true;

   mTimeMap.clear();
   mTimeMap.reserve(size()*3);      // preallocate

   // Initial node and knot value..
   TimeMap map;
   map.mTime = 0;
   map.mDistance = 0;
   mTimeMap.push_back(map);

   Knot ka,kj,ki;
   value(0, &kj, true);
   F32 length = 0.0f;
   ka = kj;

   // Loop through the knots and add nodes. Nodes are added for every knot and
   // whenever the spline length and segment length deviate by epsilon.
   F32 epsilon = Con::getFloatVariable("CameraSpline::epsilon", 0.90f);
   const F32 Step = 0.05f;
   F32 lt = 0,time = 0;
   do  
   {
      if ((time += Step) > F32(mSize - 1))
         time = (F32)mSize - 1.0f;

      value(time, &ki, true);
      length += (ki.mPosition - kj.mPosition).len();
      F32 segment = (ki.mPosition - ka.mPosition).len();

      if ((segment / length) < epsilon || time == (mSize - 1) || mFloor(lt) != mFloor(time)) 
      {
         map.mTime = time;
         map.mDistance = length;
         mTimeMap.push_back(map);
         ka = ki;
      }
      kj = ki;
      lt = time;
   }
   while (time < mSize - 1);

   mIsMapDirty = false;

   gBuilding = false;
}


//-----------------------------------------------------------------------------

void CameraSpline::renderTimeMap()
{
   buildTimeMap();

   gBuilding = true;

   // Build vertex buffer
   GFXVertexBufferHandle<GFXVertexPCT> vb;
   vb.set(GFX, mTimeMap.size(), GFXBufferTypeVolatile);
   void *ptr = vb.lock();
   if(!ptr) return;

   MRandomLCG random(1376312589 * (uintptr_t)this);
   S32 index = 0;
   for(Vector<TimeMap>::iterator itr=mTimeMap.begin(); itr != mTimeMap.end(); itr++)
   {
      Knot a;
      value(itr->mTime, &a, true);

      S32 cr = random.randI(0,255);
      S32 cg = random.randI(0,255);
      S32 cb = random.randI(0,255);
      vb[index].color.set(cr, cg, cb);
      vb[index].point.set(a.mPosition.x, a.mPosition.y, a.mPosition.z);
      index++;
   }

   gBuilding = false;

   vb.unlock();

   // Render the buffer
   GFX->pushWorldMatrix();
   GFX->setupGenericShaders();
   GFX->setVertexBuffer(vb);
   GFX->drawPrimitive(GFXLineStrip,0,index);
   GFX->popWorldMatrix();
}


//-----------------------------------------------------------------------------

F32 CameraSpline::advanceTime(F32 t, S32 delta_ms)
{
   buildTimeMap();
   Knot k;
   value(t, &k, false);
   F32 dist = getDistance(t) + k.mSpeed * (F32(delta_ms) / 1000.0f);
   return getTime(dist);
}


F32 CameraSpline::advanceDist(F32 t, F32 meters)
{
   buildTimeMap();
   F32 dist = getDistance(t) + meters;
   return getTime(dist);
}


F32 CameraSpline::getDistance(F32 t)
{
   if (mSize <= 1)
      return 0;

   // Find the nodes spanning the time
   Vector<TimeMap>::iterator end = mTimeMap.begin() + 1, start;
   for (; end < (mTimeMap.end() - 1) && end->mTime < t; end++)  {  }
   start = end - 1;

   // Interpolate between the two nodes
   F32 i = (t - start->mTime) / (end->mTime - start->mTime);
   return start->mDistance + (end->mDistance - start->mDistance) * i;
}


F32 CameraSpline::getTime(F32 d)
{
   if (mSize <= 1)
      return 0;

   // Find nodes spanning the distance
   Vector<TimeMap>::iterator end = mTimeMap.begin() + 1, start;
   for (; end < (mTimeMap.end() - 1) && end->mDistance < d; end++) {  }
   start = end - 1;

   // Check for duplicate points..
   F32 seg = end->mDistance - start->mDistance;
   if (!seg)
      return end->mTime;

   // Interpolate between the two nodes
   F32 i = (d - start->mDistance) / (end->mDistance - start->mDistance);
   return start->mTime + (end->mTime - start->mTime) * i;
}


//-----------------------------------------------------------------------------
void CameraSpline::value(F32 t, CameraSpline::Knot *result, bool skip_rotation)
{
   // Do some easing in and out for t.
   if(!gBuilding)
   {
      F32 oldT = t;
      if(oldT < 0.5f)
      {
         t = 0.5f - (mSin( (0.5 - oldT) * M_PI ) / 2.f);
      }

      if((F32(size()) - 1.5f) > 0.f && oldT - (F32(size()) - 1.5f) > 0.f)
      {
         oldT -= (F32(size()) - 1.5f);
         t = (F32(size()) - 1.5f) + (mCos( (0.5f - oldT) * F32(M_PI) ) / 2.f);
      }
   }

   // Verify that t is in range [0 >= t > size]
//   AssertFatal(t >= 0.0f && t < (F32)size(), "t out of range");
   Knot *p1 = getKnot((S32)mFloor(t));
   Knot *p2 = next(p1);

   F32 i = t - mFloor(t);  // adjust t to 0 to 1 on p1-p2 interval

   if (p1->mPath == Knot::SPLINE)
   {
      Knot *p0 = (p1->mType == Knot::KINK) ? p1 : prev(p1);
      Knot *p3 = (p2->mType == Knot::KINK) ? p2 : next(p2);
      result->mPosition.x = mCatmullrom(i, p0->mPosition.x, p1->mPosition.x, p2->mPosition.x, p3->mPosition.x);
      result->mPosition.y = mCatmullrom(i, p0->mPosition.y, p1->mPosition.y, p2->mPosition.y, p3->mPosition.y);
      result->mPosition.z = mCatmullrom(i, p0->mPosition.z, p1->mPosition.z, p2->mPosition.z, p3->mPosition.z);
   }
   else
   {   // Linear
      result->mPosition.interpolate(p1->mPosition, p2->mPosition, i);
   }

   if (skip_rotation)
      return;

   buildTimeMap();

   // find the two knots to interpolate rotation and velocity through since some
   // knots are only positional
   S32 start = (S32)mFloor(t);
   S32 end   = (p2 == p1) ? start : (start + 1);
   while (p1->mType == Knot::POSITION_ONLY && p1 != front())
   {
      p1 = prev(p1);
      start--;
   }

   while (p2->mType == Knot::POSITION_ONLY && p2 != back())
   {
      p2 = next(p2);
      end++;
   }

   if (start == end) 
   {
      result->mRotation = p1->mRotation;
      result->mSpeed = p1->mSpeed;
   }
   else 
   {
      
      F32 c = getDistance(t);
      F32 d1 = getDistance((F32)start);
      F32 d2 = getDistance((F32)end);
      
      if (d1 == d2) 
      {
         result->mRotation = p2->mRotation;
         result->mSpeed    = p2->mSpeed;
      }
      else 
      {
         i  = (c-d1)/(d2-d1);

         if(p1->mPath == Knot::SPLINE)
         {
            Knot *p0 = (p1->mType == Knot::KINK) ? p1 : prev(p1);
            Knot *p3 = (p2->mType == Knot::KINK) ? p2 : next(p2);

            F32 q,w,e;
            q = mCatmullrom(i, 0, 1, 1, 1);
            w = mCatmullrom(i, 0, 0, 0, 1);
            e = mCatmullrom(i, 0, 0, 1, 1);

            QuatF a; a.interpolate(p0->mRotation, p1->mRotation, q);
            QuatF b; b.interpolate(p2->mRotation, p3->mRotation, w);
            
            result->mRotation.interpolate(a, b, e);
            result->mSpeed = mCatmullrom(i, p0->mSpeed, p1->mSpeed, p2->mSpeed, p3->mSpeed);
         }
         else
         {
            result->mRotation.interpolate(p1->mRotation, p2->mRotation, i);
            result->mSpeed = (p1->mSpeed * (1.0f-i)) + (p2->mSpeed * i);
         }
      }
   }
}



