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
#include "collision/convex.h"

#include "platform/types.h"
#include "core/dataChunker.h"
#include "collision/collision.h"
#include "scene/sceneObject.h"
#include "collision/gjk.h"
#include "collision/concretePolyList.h"
#include "platform/profiler.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

static DataChunker sChunker;

CollisionStateList CollisionStateList::sFreeList;
CollisionWorkingList CollisionWorkingList::sFreeList;
F32 sqrDistanceEdges(const Point3F& start0,
                     const Point3F& end0,
                     const Point3F& start1,
                     const Point3F& end1,
                     Point3F*       is,
                     Point3F*       it);


//----------------------------------------------------------------------------
// Collision State
//----------------------------------------------------------------------------

CollisionState::CollisionState()
{
   mLista = mListb = 0;
}

CollisionState::~CollisionState()
{
   if (mLista)
      mLista->free();
   if (mListb)
      mListb->free();
}

void CollisionState::swap()
{
}

void CollisionState::set(Convex* a,Convex* b,const MatrixF& a2w, const MatrixF& b2w)
{
}


F32 CollisionState::distance(const MatrixF& a2w, const MatrixF& b2w, const F32 dontCareDist,
                       const MatrixF* w2a, const MatrixF* _w2b)
{
   return 0;
}

//----------------------------------------------------------------------------
// Feature Collision
//----------------------------------------------------------------------------

void ConvexFeature::reset()
{
   material = NULL;
   object = NULL;
   mVertexList.clear();
   mEdgeList.clear();
   mFaceList.clear();
}

bool ConvexFeature::collide(ConvexFeature& cf,CollisionList* cList, F32 tol)
{
   // Our vertices vs. other faces
   const Point3F* vert = mVertexList.begin();
   const Point3F* vend = mVertexList.end();
   while (vert != vend) {
      cf.testVertex(*vert,cList,false, tol);
      vert++;
   }

   // Other vertices vs. our faces
   vert = cf.mVertexList.begin();
   vend = cf.mVertexList.end();
   while (vert != vend) {
      U32 storeCount = cList->getCount();
      testVertex(*vert,cList,true, tol);

      // Fix up last reference.  material and object are copied from this rather
      //  than the object we're colliding against.
      if (storeCount != cList->getCount()) 
      {
         Collision &col = (*cList)[cList->getCount() - 1];
         col.material = cf.material;
         col.object   = cf.object;
      }
      vert++;
   }

   // Edge vs. Edge
   const Edge* edge = mEdgeList.begin();
   const Edge* eend = mEdgeList.end();
   while (edge != eend) {
      cf.testEdge(this,mVertexList[edge->vertex[0]],
         mVertexList[edge->vertex[1]],cList, tol);
      edge++;
   }

   return true;
}

inline bool isInside(const Point3F& p, const Point3F& a, const Point3F& b, const VectorF& n)
{
   VectorF v;
   mCross(n,b - a,&v);
   return mDot(v,p - a) < 0.0f;
}

void ConvexFeature::testVertex(const Point3F& v,CollisionList* cList,bool flip, F32 tol)
{
   // Test vertex against all faces
   const Face* face = mFaceList.begin();
   const Face* end  = mFaceList.end();
   for (; face != end; face++) {
      if (cList->getCount() >= CollisionList::MaxCollisions)
         return;

      const Point3F& p0 = mVertexList[face->vertex[0]];
      const Point3F& p1 = mVertexList[face->vertex[1]];
      const Point3F& p2 = mVertexList[face->vertex[2]];

      // Point near the plane?
      F32 distance = mDot(face->normal,v - p0);
      if (distance > tol || distance < -tol)
         continue;

      // Make sure it's within the bounding edges
      if (isInside(v,p0,p1,face->normal) && isInside(v,p1,p2,face->normal) &&
            isInside(v,p2,p0,face->normal)) {

         // Add collision to this face
         Collision& info = cList->increment();
         info.point = v;
         info.normal = face->normal;
         if (flip)
            info.normal.neg();
         info.material = material;
         info.object = object;
         info.distance = distance;
      }
   }
}

void ConvexFeature::testEdge(ConvexFeature* cf,const Point3F& s1, const Point3F& e1, CollisionList* cList, F32 tol)
{
   F32 tolSquared = tol*tol;

   // Test edges against edges
   const Edge* edge = mEdgeList.begin();
   const Edge* end  = mEdgeList.end();
   for (; edge != end; edge++) {
      if (cList->getCount() >= CollisionList::MaxCollisions)
         return;

      const Point3F& s2 = mVertexList[edge->vertex[0]];
      const Point3F& e2 = mVertexList[edge->vertex[1]];

      // Get the distance and closest points
      Point3F i1,i2;
      F32 distance = sqrDistanceEdges(s1, e1, s2, e2, &i1, &i2);
      if (distance > tolSquared)
         continue;
      distance = mSqrt(distance);

      // Need to figure out how to orient the collision normal.
      // The current test involves checking to see whether the collision
      // points are contained within the convex volumes, which is slow.
      if (inVolume(i1) || cf->inVolume(i2))
         distance = -distance;

      // Contact normal
      VectorF normal = i1 - i2;
      if ( mIsZero( distance ) )
         normal.zero();
      else
         normal *= 1 / distance;

      // Return a collision
      Collision& info = cList->increment();
      info.point    = i1;
      info.normal   = normal;
      info.distance = distance;
      info.material = material;
      info.object   = object;
   }
}

bool ConvexFeature::inVolume(const Point3F& v)
{
   // Test the point to see if it's inside the volume
   const Face* face = mFaceList.begin();
   const Face* end  = mFaceList.end();
   for (; face != end; face++) {
      const Point3F& p0 = mVertexList[face->vertex[0]];
      if (mDot(face->normal,v - p0) > 0)
         return false;
   }
   return true;
}


//----------------------------------------------------------------------------
// Collision State management
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

CollisionStateList::CollisionStateList()
{
   mPrev = mNext = this;
   mState = NULL;
}

void CollisionStateList::linkAfter(CollisionStateList* ptr)
{
   mPrev = ptr;
   mNext = ptr->mNext;
   ptr->mNext = this;
   mNext->mPrev = this;
}

void CollisionStateList::unlink()
{
   mPrev->mNext = mNext;
   mNext->mPrev = mPrev;
   mPrev = mNext = this;
}

CollisionStateList* CollisionStateList::alloc()
{
   if (!sFreeList.isEmpty()) {
      CollisionStateList* nxt = sFreeList.mNext;
      nxt->unlink();
      nxt->mState = NULL;
      return nxt;
   }
   return constructInPlace((CollisionStateList*)sChunker.alloc(sizeof(CollisionStateList)));
}

void CollisionStateList::free()
{
   unlink();
   linkAfter(&sFreeList);
}


//----------------------------------------------------------------------------

CollisionWorkingList::CollisionWorkingList()
{
   wLink.mPrev = wLink.mNext = this;
   rLink.mPrev = rLink.mNext = this;
}

void CollisionWorkingList::wLinkAfter(CollisionWorkingList* ptr)
{
   wLink.mPrev = ptr;
   wLink.mNext = ptr->wLink.mNext;
   ptr->wLink.mNext = this;
   wLink.mNext->wLink.mPrev = this;
}

void CollisionWorkingList::rLinkAfter(CollisionWorkingList* ptr)
{
   rLink.mPrev = ptr;
   rLink.mNext = ptr->rLink.mNext;
   ptr->rLink.mNext = this;
   rLink.mNext->rLink.mPrev = this;
}

void CollisionWorkingList::unlink()
{
   wLink.mPrev->wLink.mNext = wLink.mNext;
   wLink.mNext->wLink.mPrev = wLink.mPrev;
   wLink.mPrev = wLink.mNext = this;

   rLink.mPrev->rLink.mNext = rLink.mNext;
   rLink.mNext->rLink.mPrev = rLink.mPrev;
   rLink.mPrev = rLink.mNext = this;
}

CollisionWorkingList* CollisionWorkingList::alloc()
{
   if (sFreeList.wLink.mNext != &sFreeList) {
      CollisionWorkingList* nxt = sFreeList.wLink.mNext;
      nxt->unlink();
      return nxt;
   }
   return constructInPlace((CollisionWorkingList*)sChunker.alloc(sizeof(CollisionWorkingList)));
}

void CollisionWorkingList::free()
{
   unlink();
   wLinkAfter(&sFreeList);
}


//----------------------------------------------------------------------------
// Convex Base Class
//----------------------------------------------------------------------------

U32 Convex::sTag = (U32)-1;

//----------------------------------------------------------------------------

Convex::Convex()
{
   mNext = mPrev = this;
   mTag = 0;
}

Convex::~Convex()
{
   // Unlink from Convex Database
   unlink();

   // Delete collision states
   while (mList.mNext != &mList)
      delete mList.mNext->mState;

   // Free up working list
   while (mWorking.wLink.mNext != &mWorking)
      mWorking.wLink.mNext->free();

   // Free up references
   while (mReference.rLink.mNext != &mReference)
      mReference.rLink.mNext->free();
}


//----------------------------------------------------------------------------

void Convex::collectGarbage()
{
   // Delete unreferenced Convex Objects
   for (Convex* itr = mNext; itr != this; itr = itr->mNext) {
      if (itr->mReference.rLink.mNext == &itr->mReference) {
         Convex* ptr = itr;
         itr = itr->mPrev;
         delete ptr;
      }
   }
}

void Convex::nukeList()
{
   // Delete all Convex Objects
   for (Convex* itr = mNext; itr != this; itr = itr->mNext) {
      Convex* ptr = itr;
      itr = itr->mPrev;
      delete ptr;
   }
}

void Convex::registerObject(Convex *convex)
{
   convex->linkAfter(this);
}


//----------------------------------------------------------------------------

void Convex::linkAfter(Convex* ptr)
{
   mPrev = ptr;
   mNext = ptr->mNext;
   ptr->mNext = this;
   mNext->mPrev = this;
}

void Convex::unlink()
{
   mPrev->mNext = mNext;
   mNext->mPrev = mPrev;
   mPrev = mNext = this;
}


//----------------------------------------------------------------------------

Point3F Convex::support(const VectorF&) const
{
   return Point3F(0,0,0);
}

void Convex::getFeatures(const MatrixF&,const VectorF&,ConvexFeature* f)
{
   f->object = NULL;
}

const MatrixF& Convex::getTransform() const
{
   return mObject->getTransform();
}

const Point3F& Convex::getScale() const
{
   return mObject->getScale();
}

Box3F Convex::getBoundingBox() const
{
   return mObject->getWorldBox();
}

Box3F Convex::getBoundingBox(const MatrixF& mat, const Point3F& scale) const
{
   Box3F wBox = mObject->getObjBox();
   wBox.minExtents.convolve(scale);
   wBox.maxExtents.convolve(scale);
   mat.mul(wBox);
   return wBox;
}

//----------------------------------------------------------------------------

void Convex::addToWorkingList(Convex* ptr)
{
   CollisionWorkingList* cl = CollisionWorkingList::alloc();
   cl->wLinkAfter(&mWorking);
   cl->rLinkAfter(&ptr->mReference);
   cl->mConvex = ptr;
};


//----------------------------------------------------------------------------

void Convex::updateWorkingList(const Box3F& box, const U32 colMask)
{
   PROFILE_SCOPE( Convex_UpdateWorkingList );

   sTag++;

   // Clear objects off the working list that are no longer intersecting
   for (CollisionWorkingList* itr = mWorking.wLink.mNext; itr != &mWorking; itr = itr->wLink.mNext) {
      itr->mConvex->mTag = sTag;
      if ((!box.isOverlapped(itr->mConvex->getBoundingBox())) || (!itr->mConvex->getObject()->isCollisionEnabled())) {
         CollisionWorkingList* cl = itr;
         itr = itr->wLink.mPrev;
         cl->free();
      }
   }

   // Special processing for the terrain and interiors...
   AssertFatal(mObject->getContainer(), "Must be in a container!");

   SimpleQueryList sql;
   mObject->getContainer()->findObjects(box, colMask,SimpleQueryList::insertionCallback, &sql);
   for (U32 i = 0; i < sql.mList.size(); i++)
      sql.mList[i]->buildConvex(box, this);
}

void Convex::clearWorkingList()
{
   PROFILE_SCOPE( Convex_ClearWorkingList );

   sTag++;

   for (CollisionWorkingList* itr = mWorking.wLink.mNext; itr != &mWorking; itr = itr->wLink.mNext)
   {
      itr->mConvex->mTag = sTag;
      CollisionWorkingList* cl = itr;
      itr = itr->wLink.mPrev;
      cl->free();
   }
}

// ---------------------------------------------------------------------------

void Convex::updateStateList(const MatrixF& mat, const Point3F& scale, const Point3F* displacement)
{
   PROFILE_SCOPE( Convex_UpdateStateList );

   Box3F box1 = getBoundingBox(mat, scale);
   box1.minExtents -= Point3F(1, 1, 1);
   box1.maxExtents += Point3F(1, 1, 1);
   if (displacement) {
      Point3F oldMin = box1.minExtents;
      Point3F oldMax = box1.maxExtents;

      box1.minExtents.setMin(oldMin + *displacement);
      box1.minExtents.setMin(oldMax + *displacement);
      box1.maxExtents.setMax(oldMin + *displacement);
      box1.maxExtents.setMax(oldMax + *displacement);
   }
   sTag++;

   // Destroy states which are no longer intersecting
   for (CollisionStateList* itr = mList.mNext; itr != &mList; itr = itr->mNext) {
      Convex* cv = (itr->mState->a == this)? itr->mState->b: itr->mState->a;
      cv->mTag = sTag;
      if (!box1.isOverlapped(cv->getBoundingBox())) {
         CollisionState* cs = itr->mState;
         itr = itr->mPrev;
         delete cs;
      }
   }

   // Add collision states for new overlapping objects
   for (CollisionWorkingList* itr0 = mWorking.wLink.mNext; itr0 != &mWorking; itr0 = itr0->wLink.mNext) {
      Convex* cv = itr0->mConvex;
      if (cv->mTag != sTag && box1.isOverlapped(cv->getBoundingBox())) {
         CollisionState* state = new GjkCollisionState;
         state->set(this,cv,mat,cv->getTransform());
         state->mLista->linkAfter(&mList);
         state->mListb->linkAfter(&cv->mList);
      }
   }
}


//----------------------------------------------------------------------------

CollisionState* Convex::findClosestState(const MatrixF& mat, const Point3F& scale, const F32 dontCareDist)
{
   PROFILE_SCOPE( Convex_FindClosestState );

   updateStateList(mat, scale);
   F32 dist = +1E30f;
   CollisionState *st = 0;

   // Prepare scaled version of transform
   MatrixF axform = mat;
   axform.scale(scale);
   MatrixF axforminv(true);
   MatrixF temp(mat);
   axforminv.scale(Point3F(1.0f/scale.x, 1.0f/scale.y, 1.0f/scale.z));
   temp.affineInverse();
   axforminv.mul(temp);

   for (CollisionStateList* itr = mList.mNext; itr != &mList; itr = itr->mNext) 
   {
      CollisionState* state = itr->mState;
      if (state->mLista != itr)
         state->swap();

      // Prepare scaled version of transform
      MatrixF bxform = state->b->getTransform();
      temp = bxform;
      Point3F bscale = state->b->getScale();
      bxform.scale(bscale);
      MatrixF bxforminv(true);
      bxforminv.scale(Point3F(1.0f/bscale.x, 1.0f/bscale.y, 1.0f/bscale.z));
      temp.affineInverse();
      bxforminv.mul(temp);

      //
      F32 dd = state->distance(axform, bxform, dontCareDist, &axforminv, &bxforminv);
      if (dd < dist) 
      {
         dist = dd;
         st = state;
      }
   }
   if (dist < dontCareDist)
      return st;
   else
      return NULL;
}


//----------------------------------------------------------------------------

bool Convex::getCollisionInfo(const MatrixF& mat, const Point3F& scale, CollisionList* cList,F32 tol)
{
   PROFILE_SCOPE( Convex_GetCollisionInfo );

   // Making these static prevents needless Vector resizing that occurs
   // in the ConvexFeature constructor.
   static ConvexFeature fa;
   static ConvexFeature fb;

   for ( CollisionStateList* itr = mList.mNext; 
         itr != &mList; 
         itr = itr->mNext) 
   {

      CollisionState* state = itr->mState;

      if (state->mLista != itr)
         state->swap();

      if (state->dist <= tol) 
      {
         fa.reset();
         fb.reset();
         VectorF v;

         // The idea is that we need to scale the matrix, so we need to
         // make a copy of it, before we can pass it in to getFeatures.
         // This is used to scale us for comparison against the other
         // convex, which is correctly scaled.
         MatrixF omat = mat;
         omat.scale(scale);

         MatrixF imat = omat;
         imat.inverse();
         imat.mulV(-state->v,&v);

         getFeatures(omat,v,&fa);

         imat = state->b->getTransform();
         imat.scale(state->b->getScale());

         MatrixF bxform = imat;
         imat.inverse();
         imat.mulV(state->v,&v);

         state->b->getFeatures(bxform,v,&fb);

         fa.collide(fb,cList,tol);
      }
   }

   return (cList->getCount() != 0);
}

void Convex::getPolyList(AbstractPolyList*)
{

}

void Convex::renderWorkingList()
{
   //bool rendered = false;

   CollisionWorkingList& rList = getWorkingList();
   CollisionWorkingList* pList = rList.wLink.mNext;
   while (pList != &rList) {
      Convex* pConvex = pList->mConvex;
      pConvex->render();
      //rendered = true;
      pList = pList->wLink.mNext;
   }

   //Con::warnf( "convex rendered - %s", rendered ? "YES" : "NO" );
}

void Convex::render()
{
   ConcretePolyList polyList;
   getPolyList( &polyList );
   polyList.render();
}

//-----------------------------------------------------------------------------
// This function based on code originally written for the book:
// 3D Game Engine Design, by David H. Eberly
//
F32 sqrDistanceEdges(const Point3F& start0, const Point3F& end0,
   const Point3F& start1, const Point3F& end1,
   Point3F* is, Point3F* it)
{
   Point3F direction0 = end0 - start0;
   F32 fA00 = direction0.lenSquared();

   Point3F direction1 = end1 - start1;
   F32 fA11 = direction1.lenSquared();
   F32 fA01 = -mDot(direction0, direction1);

   Point3F kDiff = start0 - start1;
   F32 fC   = kDiff.lenSquared();
   F32 fB0  = mDot(kDiff, direction0);
   F32 fDet = mFabs(fA00*fA11 - fA01*fA01);

   // Since the endpoints are tested as vertices, we're not interested
   // in parallel lines, and intersections that don't involve end-points.
   if (fDet >= 0.00001) {

      // Calculate time of intersection for each line
      F32 fB1 = -mDot(kDiff, direction1);
      F32 fS  = fA01*fB1-fA11*fB0;
      F32 fT  = fA01*fB0-fA00*fB1;

      // Only interested in collisions that don't involve the end points
      if (fS >= 0.0 && fS <= fDet && fT >= 0.0 && fT <= fDet) {
         F32 fInvDet = 1.0 / fDet;
         fS *= fInvDet;
         fT *= fInvDet;
         F32 fSqrDist = (fS*(fA00*fS + fA01*fT + 2.0*fB0) +
            fT*(fA01*fS + fA11*fT + 2.0*fB1) + fC);

         // Intersection points.
         *is = start0 + direction0 * fS;
         *it = start1 + direction1 * fT;
         return mFabs(fSqrDist);
      }
   }

   // Return a large number in the cases where endpoints are involved.
   return 1e10f;
}
