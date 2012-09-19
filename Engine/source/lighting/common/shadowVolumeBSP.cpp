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
#include "lighting/common/shadowVolumeBSP.h"

#include "lighting/lightInfo.h"
#include "math/mPlane.h"


ShadowVolumeBSP::ShadowVolumeBSP() :
   mSVRoot(0),
   mNodeStore(0),
   mPolyStore(0),
   mFirstInteriorNode(0)
{
}

ShadowVolumeBSP::~ShadowVolumeBSP()
{
   for(U32 i = 0; i < mSurfaces.size(); i++)
      delete mSurfaces[i];
}

void ShadowVolumeBSP::insertShadowVolume(SVNode ** root, U32 volume)
{
   SVNode * traverse = mShadowVolumes[volume];

   // insert 'em
   while(traverse)
   {
      // copy it
      *root = createNode();
      (*root)->mPlaneIndex = traverse->mPlaneIndex;
      (*root)->mSurfaceInfo = traverse->mSurfaceInfo;
      (*root)->mShadowVolume = traverse->mShadowVolume;

      // do the next
      root = &(*root)->mFront;
      traverse = traverse->mFront;
   }
}

ShadowVolumeBSP::SVNode::Side ShadowVolumeBSP::whichSide(SVPoly * poly, const PlaneF & plane) const
{
   bool front = false;
   bool back = false;

   for(U32 i = 0; i < poly->mWindingCount; i++)
   {
      switch(plane.whichSide(poly->mWinding[i]))
      {
         case PlaneF::Front:
            if(back)
               return(SVNode::Split);
            front = true;
            break;

         case PlaneF::Back:
            if(front)
               return(SVNode::Split);
            back = true;
            break;

         default:
            break;
      }
   }

   AssertFatal(!(front && back), "ShadowVolumeBSP::whichSide - failed to classify poly");

   if(!front && !back)
      return(SVNode::On);

   return(front ? SVNode::Front : SVNode::Back);
}

void ShadowVolumeBSP::splitPoly(SVPoly * poly, const PlaneF & plane, SVPoly ** front, SVPoly ** back)
{
   PlaneF::Side sides[SVPoly::MaxWinding];

   U32 i;
   for(i = 0; i < poly->mWindingCount; i++)
      sides[i] = plane.whichSide(poly->mWinding[i]);

   // create the polys
   (*front) = createPoly();
   (*back) = createPoly();

   // copy the info
   (*front)->mWindingCount = (*back)->mWindingCount = 0;
   (*front)->mPlane = (*back)->mPlane = poly->mPlane;
   (*front)->mTarget = (*back)->mTarget = poly->mTarget;
   (*front)->mSurfaceInfo = (*back)->mSurfaceInfo = poly->mSurfaceInfo;
   (*front)->mShadowVolume = (*back)->mShadowVolume = poly->mShadowVolume;

   //
   for(i = 0; i < poly->mWindingCount; i++)
   {
      U32 j = (i+1) % poly->mWindingCount;

      if(sides[i] == PlaneF::On)
      {
         (*front)->mWinding[(*front)->mWindingCount++] = poly->mWinding[i];
         (*back)->mWinding[(*back)->mWindingCount++] = poly->mWinding[i];
      }
      else if(sides[i] == PlaneF::Front)
      {
         (*front)->mWinding[(*front)->mWindingCount++] = poly->mWinding[i];

         if(sides[j] == PlaneF::Back)
         {
            const Point3F & a = poly->mWinding[i];
            const Point3F & b = poly->mWinding[j];

            F32 t = plane.intersect(a, b);
            AssertFatal(t >=0 && t <= 1, "ShadowVolumeBSP::splitPoly - bad plane intersection");

            Point3F pos;
            pos.interpolate(a, b, t);

            //
            (*front)->mWinding[(*front)->mWindingCount++] =
               (*back)->mWinding[(*back)->mWindingCount++] = pos;
         }
      }
      else if(sides[i] == PlaneF::Back)
      {
         (*back)->mWinding[(*back)->mWindingCount++] = poly->mWinding[i];

         if(sides[j] == PlaneF::Front)
         {
            const Point3F & a = poly->mWinding[i];
            const Point3F & b = poly->mWinding[j];

            F32 t = plane.intersect(a, b);
            AssertFatal(t >=0 && t <= 1, "ShadowVolumeBSP::splitPoly - bad plane intersection");

            Point3F pos;
            pos.interpolate(a, b, t);

            (*front)->mWinding[(*front)->mWindingCount++] =
               (*back)->mWinding[(*back)->mWindingCount++] = pos;
         }
      }
   }

   AssertFatal((*front)->mWindingCount && (*back)->mWindingCount, "ShadowVolume::split - invalid split");
}

void ShadowVolumeBSP::addUniqueVolume(SurfaceInfo * surfaceInfo, U32 volume)
{
   if(!surfaceInfo)
      return;

   for(U32 i = 0; i < surfaceInfo->mShadowed.size(); i++)
      if(surfaceInfo->mShadowed[i] == volume)
         return;

   // add it
   surfaceInfo->mShadowed.push_back(volume);
}

void ShadowVolumeBSP::insertPoly(SVNode ** root, SVPoly * poly)
{
   if(!(*root))
   {
      insertShadowVolume(root, poly->mShadowVolume);

      if(poly->mSurfaceInfo && !mFirstInteriorNode)
         mFirstInteriorNode = mShadowVolumes[poly->mShadowVolume];

      if(poly->mTarget)
         addUniqueVolume(poly->mTarget->mSurfaceInfo, poly->mShadowVolume);

      recyclePoly(poly);
      return;
   }

   const PlaneF & plane = getPlane((*root)->mPlaneIndex);

   //
   switch(whichSide(poly, plane))
   {
      case SVNode::On:
      case SVNode::Front:
         insertPolyFront(root, poly);
         break;

      case SVNode::Back:
         insertPolyBack(root, poly);
         break;

      case SVNode::Split:
      {
         SVPoly * front;
         SVPoly * back;

         splitPoly(poly, plane, &front, &back);
         recyclePoly(poly);

         insertPolyFront(root, front);
         insertPolyBack(root, back);

         break;
      }
   }
}

void ShadowVolumeBSP::insertPolyFront(SVNode ** root, SVPoly * poly)
{
   // POLY type node?
   if(!(*root)->mFront)
   {
      if(poly->mSurfaceInfo && !mFirstInteriorNode)
         mFirstInteriorNode = mShadowVolumes[poly->mShadowVolume];
      addUniqueVolume(poly->mSurfaceInfo, (*root)->mShadowVolume);
      recyclePoly(poly);
   }
   else
      insertPoly(&(*root)->mFront, poly);
}

void ShadowVolumeBSP::insertPolyBack(SVNode ** root, SVPoly * poly)
{
   // list of nodes where an interior has been added
   if(poly->mSurfaceInfo && !(*root)->mSurfaceInfo && !(*root)->mBack)
   {
      if(!mFirstInteriorNode)
         mFirstInteriorNode = mShadowVolumes[poly->mShadowVolume];
      mParentNodes.push_back(*root);
   }

   // POLY type node?
   if(!(*root)->mFront)
   {
      poly->mTarget = (*root);
      insertPoly(&(*root)->mBack, poly);
   }
   else
      insertPoly(&(*root)->mBack, poly);
}

//------------------------------------------------------------------------------

ShadowVolumeBSP::SVNode * ShadowVolumeBSP::createNode()
{
   SVNode * node;
   if(mNodeStore)
   {
      node = mNodeStore;
      mNodeStore = mNodeStore->mFront;
   }
   else
      node = mNodeChunker.alloc();

   //
   node->mFront = node->mBack = 0;
   node->mShadowVolume = 0;
   node->mSurfaceInfo = 0;

   return(node);
}

void ShadowVolumeBSP::recycleNode(SVNode * node)
{
   if(!node)
      return;

   recycleNode(node->mFront);
   recycleNode(node->mBack);

   //
   node->mFront = mNodeStore;
   node->mBack = 0;
   mNodeStore = node;
}

ShadowVolumeBSP::SVPoly * ShadowVolumeBSP::createPoly()
{
   SVPoly * poly;

   if(mPolyStore)
   {
      poly = mPolyStore;
      mPolyStore = mPolyStore->mNext;
   }
   else
      poly = mPolyChunker.alloc();

   //
   poly->mNext = 0;
   poly->mTarget = 0;
   poly->mSurfaceInfo = 0;
   poly->mShadowVolume = 0;
   poly->mWindingCount = 0;

   for (U32 i = 0; i < SVPoly::MaxWinding; i++)
      poly->mWinding[i] = Point3F(0.0f, 0.0f, 0.0f);

   return(poly);
}

void ShadowVolumeBSP::recyclePoly(SVPoly * poly)
{
   if(!poly)
      return;

   recyclePoly(poly->mNext);

   //
   poly->mNext = mPolyStore;
   mPolyStore = poly;
}

U32 ShadowVolumeBSP::insertPlane(const PlaneF & plane)
{
   mPlanes.push_back(plane);
   return(mPlanes.size() - 1);
}

const PlaneF & ShadowVolumeBSP::getPlane(U32 index) const
{
   AssertFatal(index < mPlanes.size(), "ShadowVolumeBSP::getPlane - index out of range");
   return(mPlanes[index]);
}

ShadowVolumeBSP::SVNode * ShadowVolumeBSP::getShadowVolume(U32 index)
{
   AssertFatal(index < mShadowVolumes.size(), "ShadowVolumeBSP::getShadowVolume - index out of range");
   return(mShadowVolumes[index]);
}

bool ShadowVolumeBSP::testPoint(SVNode * root, const Point3F & pnt)
{
   const PlaneF & plane = getPlane(root->mPlaneIndex);
   switch(plane.whichSide(pnt))
   {
      case PlaneF::On:

         if(!root->mFront)
            return(true);
         else
         {
            if(testPoint(root->mFront, pnt))
               return(true);
            else
            {
               if(!root->mBack)
                  return(false);
               else
                  return(testPoint(root->mBack, pnt));
            }
         }
         break;

      //
      case PlaneF::Front:
         if(root->mFront)
            return(testPoint(root->mFront, pnt));
         else
            return(true);
         break;

      //
      case PlaneF::Back:
         if(root->mBack)
            return(testPoint(root->mBack, pnt));
         else
            return(false);
         break;
   }

   return(false);
}

//------------------------------------------------------------------------------
bool ShadowVolumeBSP::testPoly(SVNode * root, SVPoly * poly)
{
   const PlaneF & plane = getPlane(root->mPlaneIndex);
   switch(whichSide(poly, plane))
   {
      case SVNode::On:
      case SVNode::Front:
         if(root->mFront)
            return(testPoly(root->mFront, poly));

         recyclePoly(poly);
         return(true);

      case SVNode::Back:
         if(root->mBack)
            return(testPoly(root->mBack, poly));
         recyclePoly(poly);
         break;

      case SVNode::Split:
      {
         if(!root->mFront)
         {
            recyclePoly(poly);
            return(true);
         }

         SVPoly * front;
         SVPoly * back;

         splitPoly(poly, plane, &front, &back);
         recyclePoly(poly);

         if(testPoly(root->mFront, front))
         {
            recyclePoly(back);
            return(true);
         }

         if(root->mBack)
            return(testPoly(root->mBack, back));

         recyclePoly(back);
         break;
      }
   }
   return(false);
}

//------------------------------------------------------------------------------
void ShadowVolumeBSP::buildPolyVolume(SVPoly * poly, LightInfo * light)
{
   if(light->getType() != LightInfo::Vector)
      return;

   // build the poly
   Point3F pointOffset = light->getDirection() * 10.f;

   // create the shadow volume
   mShadowVolumes.increment();
   SVNode ** traverse = &mShadowVolumes.last();
   U32 shadowVolumeIndex = mShadowVolumes.size() - 1;

   for(U32 i = 0; i < poly->mWindingCount; i++)
   {
      U32 j = (i + 1) % poly->mWindingCount;
      if(poly->mWinding[i] == poly->mWinding[j])
         continue;

      (*traverse) = createNode();
      Point3F & a = poly->mWinding[i];
      Point3F & b = poly->mWinding[j];
      Point3F c = b + pointOffset;

      (*traverse)->mPlaneIndex = insertPlane(PlaneF(a,b,c));
      (*traverse)->mShadowVolume = shadowVolumeIndex;

      traverse = &(*traverse)->mFront;
   }

   // do the poly node
   (*traverse) = createNode();
   (*traverse)->mPlaneIndex = insertPlane(poly->mPlane);
   (*traverse)->mShadowVolume = poly->mShadowVolume = shadowVolumeIndex;
}

ShadowVolumeBSP::SVPoly * ShadowVolumeBSP::copyPoly(SVPoly * src)
{
   SVPoly * poly = createPoly();
   dMemcpy(poly, src, sizeof(SVPoly));

   poly->mTarget = 0;
   poly->mNext = 0;

   return(poly);
}

//------------------------------------------------------------------------------
void ShadowVolumeBSP::addToPolyList(SVPoly ** store, SVPoly * poly) const
{
   poly->mNext = *store;
   *store = poly;
}

//------------------------------------------------------------------------------
void ShadowVolumeBSP::clipPoly(SVNode * root, SVPoly ** store, SVPoly * poly)
{
   if(!root)
   {
      recyclePoly(poly);
      return;
   }

   const PlaneF & plane = getPlane(root->mPlaneIndex);

   switch(whichSide(poly, plane))
   {
      case SVNode::On:
      case SVNode::Back:
         if(root->mBack)
            clipPoly(root->mBack, store, poly);
         else
            addToPolyList(store, poly);
         break;

      case SVNode::Front:
         // encountered POLY node?
         if(!root->mFront)
         {
            recyclePoly(poly);
            return;
         }
         else
            clipPoly(root->mFront, store, poly);
         break;

      case SVNode::Split:
      {
         SVPoly * front;
         SVPoly * back;

         splitPoly(poly, plane, &front, &back);
         AssertFatal(front && back, "ShadowVolumeBSP::clipPoly: invalid split");
         recyclePoly(poly);

         // front
         if(!root->mFront)
         {
            recyclePoly(front);
            return;
         }
         else
            clipPoly(root->mFront, store, front);

         // back
         if(root->mBack)
            clipPoly(root->mBack, store, back);
         else
            addToPolyList(store, back);
         break;
      }
   }
}

// clip a poly to it's own shadow volume
void ShadowVolumeBSP::clipToSelf(SVNode * root, SVPoly ** store, SVPoly * poly)
{
   if(!root)
   {
      addToPolyList(store, poly);
      return;
   }

   const PlaneF & plane = getPlane(root->mPlaneIndex);

   switch(whichSide(poly, plane))
   {
      case SVNode::Front:
         clipToSelf(root->mFront, store, poly);
         break;

      case SVNode::On:
         addToPolyList(store, poly);
         break;

      case SVNode::Back:
         recyclePoly(poly);
         break;

      case SVNode::Split:
      {
         SVPoly * front = 0;
         SVPoly * back = 0;
         splitPoly(poly, plane, &front, &back);
         AssertFatal(front && back, "ShadowVolumeBSP::clipToSelf: invalid split");

         recyclePoly(poly);
         recyclePoly(back);

         clipToSelf(root->mFront, store, front);
         break;
      }
   }
}

//------------------------------------------------------------------------------
F32 ShadowVolumeBSP::getPolySurfaceArea(SVPoly * poly) const
{
   if(!poly)
      return(0.f);

   Point3F areaNorm(0,0,0);
   for(U32 i = 0; i < poly->mWindingCount; i++)
   {
      U32 j = (i + 1) % poly->mWindingCount;

      Point3F tmp;
      mCross(poly->mWinding[i], poly->mWinding[j], &tmp);

      areaNorm += tmp;
   }

   F32 area = mDot(poly->mPlane, areaNorm);

   if(area < 0.f)
      area *= -0.5f;
   else
      area *= 0.5f;

   if(poly->mNext)
      area += getPolySurfaceArea(poly->mNext);

   return(area);
}

//------------------------------------------------------------------------------
F32 ShadowVolumeBSP::getClippedSurfaceArea(SVNode * root, SVPoly * poly)
{
   SVPoly * store = 0;
   clipPoly(root, &store, poly);

   F32 area = getPolySurfaceArea(store);
   recyclePoly(store);
   return(area);
}


//-------------------------------------------------------------------------------
// Class SceneLighting::ShadowVolumeBSP
//-------------------------------------------------------------------------------

void ShadowVolumeBSP::movePolyList(SVPoly ** dest, SVPoly * list) const
{
   while(list)
   {
      SVPoly * next = list->mNext;
      addToPolyList(dest, list);
      list = next;
   }
}


F32 ShadowVolumeBSP::getLitSurfaceArea(SVPoly * poly, SurfaceInfo * surfaceInfo)
{
   // clip the poly to the shadow volumes
   SVPoly * polyStore = poly;

   for(U32 i = 0; polyStore && (i < surfaceInfo->mShadowed.size()); i++)
   {
      SVPoly * polyList = 0;
      SVPoly * traverse = polyStore;

      while(traverse)
      {
         SVPoly * next = traverse->mNext;
         traverse->mNext = 0;

         SVPoly * currentStore = 0;

         clipPoly(mShadowVolumes[surfaceInfo->mShadowed[i]], &currentStore, traverse);

         if(currentStore)
            movePolyList(&polyList, currentStore);

         traverse = next;
      }
      polyStore = polyList;
   }

   // get the lit area
   F32 area = getPolySurfaceArea(polyStore);
   recyclePoly(polyStore);
   return(area);
}

//------------------------------------------------------------------------------
void ShadowVolumeBSP::removeLastInterior()
{
   if(!mSVRoot || !mFirstInteriorNode)
      return;

   AssertFatal(mFirstInteriorNode->mSurfaceInfo, "No surface info for first interior node!");

   // reset the planes
   mPlanes.setSize(mFirstInteriorNode->mPlaneIndex);

   U32 i;

   // flush the shadow volumes
   for(i = mFirstInteriorNode->mShadowVolume; i < mShadowVolumes.size(); i++)
      recycleNode(mShadowVolumes[i]);
   mShadowVolumes.setSize(mFirstInteriorNode->mShadowVolume);

   // flush the interior nodes
   if(!mParentNodes.size() && (mFirstInteriorNode->mShadowVolume == mSVRoot->mShadowVolume))
   {
      recycleNode(mSVRoot);
      mSVRoot = 0;
   }
   else
   {
      for(i = 0; i < mParentNodes.size(); i++)
      {
         recycleNode(mParentNodes[i]->mBack);
         mParentNodes[i]->mBack = 0;
      }
   }

   // flush the surfaces
   for(i = 0; i < mSurfaces.size(); i++)
      delete mSurfaces[i];
   mSurfaces.clear();

   mFirstInteriorNode = 0;
}
