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

#include "ts/tsPartInstance.h"
#include "math/mMath.h"

//-------------------------------------------------------------------------------------
// Constructors
//-------------------------------------------------------------------------------------

MRandomR250 TSPartInstance::smRandom;

TSPartInstance::TSPartInstance(TSShapeInstance * sourceShape)
{
   VECTOR_SET_ASSOCIATION(mMeshObjects);

   init(sourceShape);
}

TSPartInstance::TSPartInstance(TSShapeInstance * sourceShape, S32 objectIndex)
{
   init(sourceShape);
   addObject(objectIndex);
}

void TSPartInstance::init(TSShapeInstance * sourceShape)
{
   mSourceShape = sourceShape;
   mSizeCutoffs = NULL;
   mPolyCount = NULL;
   mNumDetails = 0;
   mCurrentObjectDetail = 0;
   mCurrentIntraDL = 1.0f;
   mData = 0;
}

TSPartInstance::~TSPartInstance()
{
   delete [] mPolyCount;
}

//-------------------------------------------------------------------------------------
// Methods for updating PartInstances
//-------------------------------------------------------------------------------------

void TSPartInstance::addObject(S32 objectIndex)
{
   if (mSourceShape->mMeshObjects[objectIndex].forceHidden ||
      mSourceShape->mMeshObjects[objectIndex].visible < 0.01f)
      // not visible, don't bother
      return;

   mMeshObjects.push_back(&mSourceShape->mMeshObjects[objectIndex]);
}

void TSPartInstance::updateBounds()
{
   // run through meshes and brute force it?
   Box3F bounds;
   mBounds.minExtents.set( 10E30f, 10E30f, 10E30f);
   mBounds.maxExtents.set(-10E30f,-10E30f,-10E30f);
   for (S32 i=0; i<mMeshObjects.size(); i++)
   {
      if (mMeshObjects[i]->getMesh(0))
         mMeshObjects[i]->getMesh(0)->computeBounds(mMeshObjects[i]->getTransform(),bounds,mMeshObjects[i]->frame);
      mBounds.minExtents.setMin(bounds.minExtents);
      mBounds.maxExtents.setMax(bounds.maxExtents);
   }
   mCenter = mBounds.minExtents + mBounds.maxExtents;
   mCenter *= 0.5f;
   Point3F r = mBounds.maxExtents-mCenter;
   mRadius = mSqrt(mDot(r,r));
}

//-------------------------------------------------------------------------------------
// Methods for breaking shapes into pieces
//-------------------------------------------------------------------------------------

void TSPartInstance::breakShape(TSShapeInstance * shape, S32 subShape, Vector<TSPartInstance*> & partList, F32 * probShatter, F32 * probBreak, S32 probDepth)
{
   AssertFatal(subShape>=0 && subShape<shape->mShape->subShapeFirstNode.size(),"TSPartInstance::breakShape: subShape out of range.");

   S32 start = shape->mShape->subShapeFirstNode[subShape];

   TSPartInstance::breakShape(shape, NULL, start, partList, probShatter, probBreak, probDepth);

   // update bounds (and get rid of empty parts)
   for (S32 i=0; i<partList.size(); i++)
   {
      if (partList[i]->mMeshObjects.size())
         partList[i]->updateBounds();
      else
      {
         partList.erase(i);
         i--;
      }
   }
}

void TSPartInstance::breakShape(TSShapeInstance * shape, TSPartInstance * currentPart, S32 currentNode, Vector<TSPartInstance*> & partList, F32 * probShatter, F32 * probBreak, S32 probDepth)
{
   AssertFatal( !probDepth || (probShatter && probBreak),"TSPartInstance::breakShape: probabilities improperly specified.");

   const TSShape::Node * node = &shape->mShape->nodes[currentNode];
   S32 object = node->firstObject;
   S32 child  = node->firstChild;

   // copy off probabilities and update probability lists for next level
   F32 ps = probShatter ? *probShatter : 1.0f;
   F32 pb = probBreak   ? *probBreak   : 1.0f;
   if (probDepth>1 && probShatter && probBreak)
   {
      probShatter++;
      probBreak++;
      probDepth--;
   }

   // what to do...depending on how the die roll, we can:
   // a) shatter the shape at this level -- meaning we make a part out of each object on this node and
   //    we make parts out of all the children (perhaps breaking them up further still)
   // b) break the shape off at this level -- meaning we make a part out of the intact piece from here
   //    on down (again, we might break the result further as we iterate through the nodes...what breaking
   //    the shape really does is separate this piece from the parent piece).
   // c) add this piece to the parent -- meaning all objects on this node are added to the parent, and children
   //    are also added (but children will be recursively sent through this routine, so if a parent gets option
   //    (c) and the child option (a) or (b), then the child will be ripped from the parents grasp.  Cruel
   //    people us coders are.
   // Note: (a) is the only way that two objects on the same node can be separated...that is why both
   //       option a and option b are needed.
   if (!probShatter || smRandom.randF() < ps)
   {
      // option a -- shatter the shape at this level

      // iterate through the objects, make part out of each one
      while (object>=0)
      {
         partList.increment();
         partList.last() = new TSPartInstance(shape,object);
         object = shape->mShape->objects[object].nextSibling;
      }

      // iterate through the child nodes, call ourselves on each one with currentPart = NULL
      while (child>=0)
      {
         TSPartInstance::breakShape(shape,NULL,child,partList,probShatter,probBreak,probDepth);
         child = shape->mShape->nodes[child].nextSibling;
      }

      return;
   }

   if (!probBreak || smRandom.randF() < pb)
      // option b -- break the shape off at this level
      currentPart = NULL; // fall through to option C

   // option c -- add this piece to the parent

   if (!currentPart)
   {
      currentPart = new TSPartInstance(shape);
      partList.push_back(currentPart);
   }

   // iterate through objects, add to currentPart
   while (object>=0)
   {
      currentPart->addObject(object);
      object = shape->mShape->objects[object].nextSibling;
   }

   // iterate through child nodes, call ourselves on each one with currentPart as is
   while (child>=0)
   {
      TSPartInstance::breakShape(shape,currentPart,child,partList,probShatter,probBreak,probDepth);
      child = shape->mShape->nodes[child].nextSibling;
   }
}

//-------------------------------------------------------------------------------------
// render methods -- we use TSShapeInstance code as much as possible
// issues: setupTexturing expects a detail level, we give it an object detail level
//-------------------------------------------------------------------------------------

void TSPartInstance::render(S32 od, const TSRenderState &rdata)
{
   S32 i;

   // render mesh objects
   for (i=0; i<mMeshObjects.size(); i++)
   {
      TSRenderState objState = rdata;
      const char *meshName = mSourceShape->mShape->names[mMeshObjects[i]->object->nameIndex];
      mMeshObjects[i]->render(od,mSourceShape->mShape->mShapeVertexBuffer,mSourceShape->getMaterialList(),objState,1.0, meshName);
   }
}

//-------------------------------------------------------------------------------------
// Detail selection
// 2 methods:
// method 1:  use source shapes detail levels...
// method 2:  pass in our own table...
// In either case, you can compute the pixel size on your own or let open gl do it.
// If you want to use method 2, you have to call setDetailData sometime before selecting detail
//-------------------------------------------------------------------------------------

void TSPartInstance::setDetailData(F32 * sizeCutoffs, S32 numDetails)
{
   if (mSizeCutoffs == sizeCutoffs && mNumDetails==numDetails)
      return;

   mSizeCutoffs = sizeCutoffs;
   mNumDetails = numDetails;
   delete [] mPolyCount;
   mPolyCount = NULL;
}

/*
void TSPartInstance::selectCurrentDetail(bool ignoreScale)
{
   if (mSizeCutoffs)
   {
      selectCurrentDetail(mSizeCutoffs,mNumDetails,ignoreScale);
      return;
   }

   mSourceShape->selectCurrentDetail(ignoreScale);
   mCurrentObjectDetail = mSourceShape->getCurrentDetail();
   mCurrentIntraDL = mSourceShape->getCurrentIntraDetail();
}

void TSPartInstance::selectCurrentDetail(F32 pixelSize)
{
   if (mSizeCutoffs)
   {
      selectCurrentDetail(pixelSize,mSizeCutoffs,mNumDetails);
      return;
   }

   mSourceShape->selectCurrentDetail(pixelSize);
   mCurrentObjectDetail = mSourceShape->getCurrentDetail();
   mCurrentIntraDL = mSourceShape->getCurrentIntraDetail();
}

void TSPartInstance::selectCurrentDetail(F32 dist, F32 invScale)
{
   if (mSizeCutoffs)
   {
      const RectI &viewport = GFX->getViewport();
      F32 pixelScale = viewport.extent.x * 1.6f / 640.0f;
      F32 pixelSize = GFX->projectRadius(dist*invScale,mSourceShape->getShape()->radius) * pixelScale * TSShapeInstance::smDetailAdjust;
      selectCurrentDetail(pixelSize,mSizeCutoffs,mNumDetails);
      return;
   }

   mSourceShape->selectCurrentDetail(dist, invScale);
   mCurrentObjectDetail = mSourceShape->getCurrentDetail();
   mCurrentIntraDL = mSourceShape->getCurrentIntraDetail();
}

void TSPartInstance::selectCurrentDetail(F32 * sizeCutoffs, S32 numDetails, bool ignoreScale)
{
   // compute pixel size
   Point3F p;
   MatrixF toCam = GFX->getWorldMatrix();
   toCam.mulP(mCenter,&p);
   F32 dist = mDot(p,p);
   F32 scale = 1.0f;
   if (!ignoreScale)
   {
      // any scale?
      Point3F x,y,z;
      toCam.getRow(0,&x);
      toCam.getRow(1,&y);
      toCam.getRow(2,&z);
      F32 scalex = mDot(x,x);
      F32 scaley = mDot(y,y);
      F32 scalez = mDot(z,z);
      scale = scalex;
      if (scaley > scale)
         scale = scaley;
      if (scalez > scale)
         scale = scalez;
   }
   dist /= scale;
   dist = mSqrt(dist);

   const RectI &viewport = GFX->getViewport();
   // JMQMERGE: is this using a hardcoded res/aspect ?  (and the code above)
   F32 pixelScale = viewport.extent.x * 1.6f / 640.0f;
   F32 pixelRadius = GFX->projectRadius(dist,mRadius) * pixelScale * TSShapeInstance::smDetailAdjust;

   selectCurrentDetail(pixelRadius,sizeCutoffs,numDetails);
}

void TSPartInstance::selectCurrentDetail(F32 pixelSize, F32 * sizeCutoffs, S32 numDetails)
{
   mCurrentObjectDetail = 0;
   while (numDetails)
   {
      if (pixelSize > *sizeCutoffs)
         return;
      mCurrentObjectDetail++;
      numDetails--;
      sizeCutoffs++;
   }
   mCurrentObjectDetail = -1;
}
*/

//-------------------------------------------------------------------------------------
// Detail query methods...complicated because there are two ways that detail information
// can be determined...1) using source shape, or 2) using mSizeCutoffs
//-------------------------------------------------------------------------------------

F32 TSPartInstance::getDetailSize(S32 dl) const
{
   if (dl<0)
      return 0;
   else if (mSizeCutoffs && dl<mNumDetails)
      return mSizeCutoffs[dl];
   else if (!mSizeCutoffs && dl<=mSourceShape->getShape()->mSmallestVisibleDL)
      return mSourceShape->getShape()->details[dl].size;
   else return 0;
}

S32 TSPartInstance::getPolyCount(S32 dl)
{
   if (!mPolyCount)
      computePolyCount();

   if (dl<0 || dl>=mNumDetails)
      return 0;
   else
      return mPolyCount[dl];
}

void TSPartInstance::computePolyCount()
{
   if (!mSizeCutoffs)
      mNumDetails = mSourceShape->getShape()->mSmallestVisibleDL+1;

   delete [] mPolyCount;
   mPolyCount = new S32[mNumDetails];

   for (S32 i=0; i<mNumDetails; i++)
   {
      mPolyCount[i] = 0;
      for (S32 j=0; j<mMeshObjects.size(); j++)
      {
         if (mMeshObjects[j]->getMesh(i))
            mPolyCount[i] += mMeshObjects[j]->getMesh(i)->getNumPolys();
      }
   }
}


