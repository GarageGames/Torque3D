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
#include "ts/tsShape.h"

#include "ts/tsLastDetail.h"
#include "ts/tsMaterialList.h"
#include "core/stringTable.h"
#include "console/console.h"
#include "ts/tsShapeInstance.h"
#include "collision/convex.h"
#include "materials/matInstance.h"
#include "materials/materialManager.h"
#include "math/mathIO.h"
#include "core/util/endian.h"
#include "core/stream/fileStream.h"
#include "console/compiler.h"
#include "core/fileObject.h"

#ifdef TORQUE_COLLADA
extern TSShape* loadColladaShape(const Torque::Path &path);
#endif

/// most recent version -- this is the version we write
S32 TSShape::smVersion = 26;
/// the version currently being read...valid only during a read
S32 TSShape::smReadVersion = -1;
const U32 TSShape::smMostRecentExporterVersion = DTS_EXPORTER_CURRENT_VERSION;

F32 TSShape::smAlphaOutLastDetail = -1.0f;
F32 TSShape::smAlphaInBillboard = 0.15f;
F32 TSShape::smAlphaOutBillboard = 0.15f;
F32 TSShape::smAlphaInDefault = -1.0f;
F32 TSShape::smAlphaOutDefault = -1.0f;

// don't bother even loading this many of the highest detail levels (but
// always load last renderable detail)
S32 TSShape::smNumSkipLoadDetails = 0;

bool TSShape::smInitOnRead = true;


TSShape::TSShape()
{
   mMaterialList = NULL;
   mReadVersion = -1; // -1 means constructed from scratch (e.g., in exporter or no read yet)
   mHasSkinMesh = false;
   mSequencesConstructed = false;
   mShapeData = NULL;
   mShapeDataSize = 0;

   mUseDetailFromScreenError = false;

   mDetailLevelLookup.setSize( 1 );
   mDetailLevelLookup[0].set( -1, 0 );

   VECTOR_SET_ASSOCIATION(mSequences);
   VECTOR_SET_ASSOCIATION(mNodeRotations);
   VECTOR_SET_ASSOCIATION(mNodeTranslations);
   VECTOR_SET_ASSOCIATION(mNodeUniformScales);
   VECTOR_SET_ASSOCIATION(mNodeAlignedScales);
   VECTOR_SET_ASSOCIATION(mNodeArbitraryScaleRots);
   VECTOR_SET_ASSOCIATION(mNodeArbitraryScaleFactors);
   VECTOR_SET_ASSOCIATION(mGroundRotations);
   VECTOR_SET_ASSOCIATION(mGroundTranslations);
   VECTOR_SET_ASSOCIATION(mTriggers);
   VECTOR_SET_ASSOCIATION(mBillboardDetails);
   VECTOR_SET_ASSOCIATION(mDetailCollisionAccelerators);
   VECTOR_SET_ASSOCIATION(mNames);

   VECTOR_SET_ASSOCIATION( mNodes );
   VECTOR_SET_ASSOCIATION( mObjects );
   VECTOR_SET_ASSOCIATION( mObjectStates );
   VECTOR_SET_ASSOCIATION( mSubShapeFirstNode );
   VECTOR_SET_ASSOCIATION( mSubShapeFirstObject );
   VECTOR_SET_ASSOCIATION( mDetailFirstSkin );
   VECTOR_SET_ASSOCIATION( mSubShapeNumNodes );
   VECTOR_SET_ASSOCIATION( mSubShapeNumObjects );
   VECTOR_SET_ASSOCIATION( mDetails );
   VECTOR_SET_ASSOCIATION( mDefaultRotations );
   VECTOR_SET_ASSOCIATION( mDefaultTranslations );

   VECTOR_SET_ASSOCIATION( mSubShapeFirstTranslucentObject );
   VECTOR_SET_ASSOCIATION( mMeshes );

   VECTOR_SET_ASSOCIATION( mAlphaIn );
   VECTOR_SET_ASSOCIATION( mAlphaOut );
}

TSShape::~TSShape()
{
   delete mMaterialList;

   S32 i;

   // everything left over here is a legit mesh
   for (i=0; i<mMeshes.size(); i++)
   {
      if (!mMeshes[i])
         continue;

      // Handle meshes that were either assembled with the shape or added later
      if (((S8*)mMeshes[i] >= mShapeData) && ((S8*)mMeshes[i] < (mShapeData + mShapeDataSize)))
         destructInPlace(mMeshes[i]);
      else
         delete mMeshes[i];
   }

   for (i=0; i<mBillboardDetails.size(); i++)
   {
      delete mBillboardDetails[i];
      mBillboardDetails[i] = NULL;
   }
   mBillboardDetails.clear();

   // Delete any generated accelerators
   S32 dca;
   for (dca = 0; dca < mDetailCollisionAccelerators.size(); dca++)
   {
      ConvexHullAccelerator* accel = mDetailCollisionAccelerators[dca];
      if (accel != NULL) {
         delete [] accel->vertexList;
         delete [] accel->normalList;
         for (S32 j = 0; j < accel->numVerts; j++)
            delete [] accel->emitStrings[j];
         delete [] accel->emitStrings;
         delete accel;
      }
   }
   for (dca = 0; dca < mDetailCollisionAccelerators.size(); dca++)
      mDetailCollisionAccelerators[dca] = NULL;

   if( mShapeData )
      delete[] mShapeData;
}

const String& TSShape::getName( S32 nameIndex ) const
{
   AssertFatal(nameIndex>=0 && nameIndex<mNames.size(),"TSShape::getName");
   return mNames[nameIndex];
}

const String& TSShape::getMeshName( S32 meshIndex ) const
{
   S32 nameIndex = mObjects[meshIndex].nameIndex;
   if ( nameIndex < 0 )
      return String::EmptyString;

   return mNames[nameIndex];
}

const String& TSShape::getNodeName( S32 nodeIndex ) const
{   
   S32 nameIdx = mNodes[nodeIndex].nameIndex;
   if ( nameIdx < 0 )
      return String::EmptyString;

   return mNames[nameIdx];
}

const String& TSShape::getSequenceName( S32 seqIndex ) const
{
   AssertFatal(seqIndex >= 0 && seqIndex<mSequences.size(),"TSShape::getSequenceName index beyond range");

   S32 nameIdx = mSequences[seqIndex].nameIndex;
   if ( nameIdx < 0 )
      return String::EmptyString;

   return mNames[nameIdx];
}

S32 TSShape::findName(const String &name) const
{
   for (S32 i=0; i<mNames.size(); i++)
   {
      if (mNames[i].equal( name, String::NoCase ))
         return i;
   }
  
   return -1;
}

const String& TSShape::getTargetName( S32 mapToNameIndex ) const
{
	S32 targetCount = mMaterialList->getMaterialNameList().size();

	if(mapToNameIndex < 0 || mapToNameIndex >= targetCount)
		return String::EmptyString;

	return mMaterialList->getMaterialNameList()[mapToNameIndex];
}

S32 TSShape::getTargetCount() const
{
	if(!this)
		return -1;

	return mMaterialList->getMaterialNameList().size();

}

S32 TSShape::findNode(S32 nameIndex) const
{
   for (S32 i=0; i<mNodes.size(); i++)
      if (mNodes[i].nameIndex==nameIndex)
         return i;
   return -1;
}

S32 TSShape::findObject(S32 nameIndex) const
{
   for (S32 i=0; i<mObjects.size(); i++)
      if (mObjects[i].nameIndex==nameIndex)
         return i;
   return -1;
}

S32 TSShape::findDetail(S32 nameIndex) const
{
   for (S32 i=0; i<mDetails.size(); i++)
      if (mDetails[i].nameIndex==nameIndex)
         return i;
   return -1;
}

S32 TSShape::findDetailBySize(S32 size) const
{
   for (S32 i=0; i<mDetails.size(); i++)
      if (mDetails[i].size==size)
         return i;
   return -1;
}

S32 TSShape::findSequence(S32 nameIndex) const
{
   for (S32 i=0; i<mSequences.size(); i++)
      if (mSequences[i].nameIndex==nameIndex)
         return i;
   return -1;
}

bool TSShape::findMeshIndex(const String& meshName, S32& objIndex, S32& meshIndex)
{
   // Determine the object name and detail size from the mesh name
   S32 detailSize = 999;  
   objIndex = findObject(String::GetTrailingNumber(meshName, detailSize));
   if (objIndex < 0)
      return false;

   // Determine the subshape this object belongs to
   S32 subShapeIndex = getSubShapeForObject(objIndex);
   AssertFatal(subShapeIndex < mSubShapeFirstObject.size(), "Could not find subshape for object!");

   // Get the detail levels for the subshape
   Vector<S32> validDetails;
   getSubShapeDetails(subShapeIndex, validDetails);

   // Find the detail with the correct size
   for (meshIndex = 0; meshIndex < validDetails.size(); meshIndex++)
   {
      const TSShape::Detail& det = mDetails[validDetails[meshIndex]];
      if (detailSize == det.size)
         return true;
   }

   return false;
}

TSMesh* TSShape::findMesh(const String& meshName)
{
   S32 objIndex, meshIndex;
   if (!findMeshIndex(meshName, objIndex, meshIndex))
      return 0;
   return mMeshes[mObjects[objIndex].startMeshIndex + meshIndex];
}

S32 TSShape::getSubShapeForNode(S32 nodeIndex)
{
   for (S32 i = 0; i < mSubShapeFirstNode.size(); i++)
   {
      S32 start = mSubShapeFirstNode[i];
      S32 end = start + mSubShapeNumNodes[i];
      if ((nodeIndex >= start) && (nodeIndex < end))
         return i;;
   }
   return -1;
}

S32 TSShape::getSubShapeForObject(S32 objIndex)
{
   for (S32 i = 0; i < mSubShapeFirstObject.size(); i++)
   {
      S32 start = mSubShapeFirstObject[i];
      S32 end = start + mSubShapeNumObjects[i];
      if ((objIndex >= start) && (objIndex < end))
         return i;
   }
   return -1;
}

void TSShape::getSubShapeDetails(S32 subShapeIndex, Vector<S32>& validDetails)
{
   validDetails.clear();
   for (S32 i = 0; i < mDetails.size(); i++)
   {
      if ((mDetails[i].subShapeNum == subShapeIndex) ||
          (mDetails[i].subShapeNum < 0))
          validDetails.push_back(i);
   }
}

void TSShape::getNodeWorldTransform(S32 nodeIndex, MatrixF* mat) const
{
   if ( nodeIndex == -1 )
   {
      mat->identity();
   }
   else
   {
      // Calculate the world transform of the given node
      mDefaultRotations[nodeIndex].getQuatF().setMatrix(mat);
      mat->setPosition(mDefaultTranslations[nodeIndex]);

      S32 parentIndex = mNodes[nodeIndex].parentIndex;
      while (parentIndex != -1)
      {
         MatrixF mat2(*mat);
         mDefaultRotations[parentIndex].getQuatF().setMatrix(mat);
         mat->setPosition(mDefaultTranslations[parentIndex]);
         mat->mul(mat2);

         parentIndex = mNodes[parentIndex].parentIndex;
      }
   }
}

void TSShape::getNodeObjects(S32 nodeIndex, Vector<S32>& nodeObjects)
{
   for (S32 i = 0; i < mObjects.size(); i++)
   {
      if ((nodeIndex == -1) || (mObjects[i].nodeIndex == nodeIndex))
         nodeObjects.push_back(i);
   }
}

void TSShape::getNodeChildren(S32 nodeIndex, Vector<S32>& nodeChildren)
{
   for (S32 i = 0; i < mNodes.size(); i++)
   {
      if (mNodes[i].parentIndex == nodeIndex)
         nodeChildren.push_back(i);
   }
}

void TSShape::getObjectDetails(S32 objIndex, Vector<S32>& objDetails)
{
   // Get the detail levels for this subshape
   Vector<S32> validDetails;
   getSubShapeDetails(getSubShapeForObject(objIndex), validDetails);

   // Get the non-null details for this object
   const TSShape::Object& obj = mObjects[objIndex];
   for (S32 i = 0; i < obj.numMeshes; i++)
   {
      if (mMeshes[obj.startMeshIndex + i])
         objDetails.push_back(validDetails[i]);
   }
}

void TSShape::init()
{
   S32 numSubShapes = mSubShapeFirstNode.size();
   AssertFatal(numSubShapes==mSubShapeFirstObject.size(),"TSShape::init");

   S32 i,j;

   // set up parent/child relationships on nodes and objects
   for (i=0; i<mNodes.size(); i++)
      mNodes[i].firstObject = mNodes[i].firstChild = mNodes[i].nextSibling = -1;
   for (i=0; i<mNodes.size(); i++)
   {
      S32 parentIndex = mNodes[i].parentIndex;
      if (parentIndex>=0)
      {
         if (mNodes[parentIndex].firstChild<0)
            mNodes[parentIndex].firstChild=i;
         else
         {
            S32 child = mNodes[parentIndex].firstChild;
            while (mNodes[child].nextSibling>=0)
               child = mNodes[child].nextSibling;
            mNodes[child].nextSibling = i;
         }
      }
   }
   for (i=0; i<mObjects.size(); i++)
   {
      mObjects[i].nextSibling = -1;

      S32 nodeIndex = mObjects[i].nodeIndex;
      if (nodeIndex>=0)
      {
         if (mNodes[nodeIndex].firstObject<0)
            mNodes[nodeIndex].firstObject = i;
         else
         {
            S32 objectIndex = mNodes[nodeIndex].firstObject;
            while (mObjects[objectIndex].nextSibling>=0)
               objectIndex = mObjects[objectIndex].nextSibling;
            mObjects[objectIndex].nextSibling = i;
         }
      }
   }

   mFlags = 0;
   for (i=0; i<mSequences.size(); i++)
   {
      if (!mSequences[i].animatesScale())
         continue;

      U32 curVal = mFlags & AnyScale;
      U32 newVal = mSequences[i].flags & AnyScale;
      mFlags &= ~(AnyScale);
      mFlags |= getMax(curVal,newVal); // take the larger value (can only convert upwards)
   }

   // set up alphaIn and alphaOut vectors...
   mAlphaIn.setSize(mDetails.size());
   mAlphaOut.setSize(mDetails.size());

   for (i=0; i<mDetails.size(); i++)
   {
      if (mDetails[i].size<0)
      {
         // we don't care...
         mAlphaIn[i]  = 0.0f;
         mAlphaOut[i] = 0.0f;
      }
      else if (i+1==mDetails.size() || mDetails[i+1].size<0)
      {
         mAlphaIn[i]  = 0.0f;
         mAlphaOut[i] = smAlphaOutLastDetail;
      }
      else
      {
         if (mDetails[i+1].subShapeNum<0)
         {
            // following detail is a billboard detail...treat special...
            mAlphaIn[i]  = smAlphaInBillboard;
            mAlphaOut[i] = smAlphaOutBillboard;
         }
         else
         {
            // next detail is normal detail
            mAlphaIn[i] = smAlphaInDefault;
            mAlphaOut[i] = smAlphaOutDefault;
         }
      }
   }

   for (i=mSmallestVisibleDL-1; i>=0; i--)
   {
      if (i<smNumSkipLoadDetails)
      {
         // this detail level renders when pixel size
         // is larger than our cap...zap all the meshes and decals
         // associated with it and use the next detail level
         // instead...
         S32 ss    = mDetails[i].subShapeNum;
         S32 od    = mDetails[i].objectDetailNum;

         if (ss==mDetails[i+1].subShapeNum && od==mDetails[i+1].objectDetailNum)
            // doh! already done this one (init can be called multiple times on same shape due
            // to sequence importing).
            continue;
         mDetails[i].subShapeNum = mDetails[i+1].subShapeNum;
         mDetails[i].objectDetailNum = mDetails[i+1].objectDetailNum;
      }
   }

   for (i=0; i<mDetails.size(); i++)
   {
      S32 count = 0;
      S32 ss = mDetails[i].subShapeNum;
      S32 od = mDetails[i].objectDetailNum;
      if (ss<0)
      {
         // billboard detail...
         mDetails[i].polyCount = 2;
         continue;
      }
      S32 start = mSubShapeFirstObject[ss];
      S32 end   = start + mSubShapeNumObjects[ss];
      for (j=start; j<end; j++)
      {
         Object & obj = mObjects[j];
         if (od<obj.numMeshes)
         {
            TSMesh * mesh = mMeshes[obj.startMeshIndex+od];
            count += mesh ? mesh->getNumPolys() : 0;
         }
      }
      mDetails[i].polyCount = count;
   }

   // Init the collision accelerator array.  Note that we don't compute the
   //  accelerators until the app requests them
   {
      S32 dca;
      for (dca = 0; dca < mDetailCollisionAccelerators.size(); dca++)
      {
         ConvexHullAccelerator* accel = mDetailCollisionAccelerators[dca];
         if (accel != NULL) {
            delete [] accel->vertexList;
            delete [] accel->normalList;
            for (S32 j = 0; j < accel->numVerts; j++)
               delete [] accel->emitStrings[j];
            delete [] accel->emitStrings;
            delete accel;
         }
      }

      mDetailCollisionAccelerators.setSize(mDetails.size());
      for (dca = 0; dca < mDetailCollisionAccelerators.size(); dca++)
         mDetailCollisionAccelerators[dca] = NULL;
   }

   initVertexFeatures();
   initMaterialList();
}

void TSShape::initVertexFeatures()
{
   bool hasColors = false;
   bool hasTexcoord2 = false;

   Vector<TSMesh*>::iterator iter = mMeshes.begin();
   for ( ; iter != mMeshes.end(); iter++ )
   {
      TSMesh *mesh = *iter;
      if (  mesh &&
            (  mesh->getMeshType() == TSMesh::StandardMeshType ||
               mesh->getMeshType() == TSMesh::SkinMeshType ) )
      {
         if ( mesh->mVertexData.isReady() )
         {
            hasColors |= mesh->mHasColor;
            hasTexcoord2 |= mesh->mHasTVert2;
         }
         else
         {
            hasColors |= !mesh->mColors.empty();
            hasTexcoord2 |= !mesh->mTVerts2.empty();
         }
      }
   }

   mVertSize = ( hasTexcoord2 || hasColors ) ? sizeof(TSMesh::__TSMeshVertex_3xUVColor) : sizeof(TSMesh::__TSMeshVertexBase);
   mVertexFormat.clear();
  
   mVertexFormat.addElement( GFXSemantic::POSITION, GFXDeclType_Float3 );
   mVertexFormat.addElement( GFXSemantic::TANGENTW, GFXDeclType_Float, 3 );
   mVertexFormat.addElement( GFXSemantic::NORMAL, GFXDeclType_Float3 );
   mVertexFormat.addElement( GFXSemantic::TANGENT, GFXDeclType_Float3 );

   mVertexFormat.addElement( GFXSemantic::TEXCOORD, GFXDeclType_Float2, 0 );

   if(hasTexcoord2 || hasColors)
   {
      mVertexFormat.addElement( GFXSemantic::TEXCOORD, GFXDeclType_Float2, 1 );
      mVertexFormat.addElement( GFXSemantic::COLOR, GFXDeclType_Color );
      mVertexFormat.addElement( GFXSemantic::TEXCOORD, GFXDeclType_Float, 2 );
   }

   // Go fix up meshes to include defaults for optional features
   // and initialize them if they're not a skin mesh.
   iter = mMeshes.begin();
   for ( ; iter != mMeshes.end(); iter++ )
   {
      TSMesh *mesh = *iter;
      if (  !mesh ||
            (  mesh->getMeshType() != TSMesh::StandardMeshType &&
               mesh->getMeshType() != TSMesh::SkinMeshType ) )
         continue;

      // Set the flags.
      mesh->mVertexFormat = &mVertexFormat;
      mesh->mVertSize = mVertSize;

      // Create and fill aligned data structure
      mesh->convertToAlignedMeshData();

      // Init the vertex buffer.
      if ( mesh->getMeshType() == TSMesh::StandardMeshType )
         mesh->createVBIB();
   }
}

void TSShape::setupBillboardDetails( const String &cachePath )
{
   // set up billboard details -- only do this once, meaning that
   // if we add a sequence to the shape we don't redo the billboard
   // details...
   if ( !mBillboardDetails.empty() )
      return;

   for ( U32 i=0; i < mDetails.size(); i++ )
   {
      const Detail &det = mDetails[i];

      if ( det.subShapeNum >= 0 )
         continue; // not a billboard detail

      while (mBillboardDetails.size() <= i )
         mBillboardDetails.push_back(NULL);

      mBillboardDetails[i] = new TSLastDetail(   this,
                                                cachePath,
                                                det.bbEquatorSteps,
                                                det.bbPolarSteps,
                                                det.bbPolarAngle,
                                                det.bbIncludePoles,
                                                det.bbDetailLevel,
                                                det.bbDimension );

      mBillboardDetails[i]->update();
   }
}

void TSShape::initMaterialList()
{
   S32 numSubShapes = mSubShapeFirstObject.size();
   #if defined(TORQUE_MAX_LIB)
   mSubShapeFirstTranslucentObject.setSize(numSubShapes);
   #endif

   mHasSkinMesh = false;

   S32 i,j,k;
   // for each subshape, find the first translucent object
   // also, while we're at it, set mHasTranslucency
   for (S32 ss = 0; ss<numSubShapes; ss++)
   {
      S32 start = mSubShapeFirstObject[ss];
      S32 end = mSubShapeNumObjects[ss];
      mSubShapeFirstTranslucentObject[ss] = end;
      for (i=start; i<end; i++)
      {
         // check to see if this object has translucency
         Object & obj = mObjects[i];
         for (j=0; j<obj.numMeshes; j++)
         {
            TSMesh * mesh = mMeshes[obj.startMeshIndex+j];
            if (!mesh)
               continue;

            mHasSkinMesh |= mesh->getMeshType() == TSMesh::SkinMeshType;

            for (k=0; k<mesh->mPrimitives.size(); k++)
            {
               if (mesh->mPrimitives[k].matIndex & TSDrawPrimitive::NoMaterial)
                  continue;
               S32 flags = mMaterialList->getFlags(mesh->mPrimitives[k].matIndex & TSDrawPrimitive::MaterialMask);
               if (flags & TSMaterialList::AuxiliaryMap)
                  continue;
               if (flags & TSMaterialList::Translucent)
               {
                  mFlags |= HasTranslucency;
                  mSubShapeFirstTranslucentObject[ss] = i;
                  break;
               }
            }
            if (k!=mesh->mPrimitives.size())
               break;
         }
         if (j!=obj.numMeshes)
            break;
      }
      if (i!=end)
         break;
   }

}

bool TSShape::preloadMaterialList(const Torque::Path &path)
{
   if (mMaterialList)
      mMaterialList->setTextureLookupPath(path.getPath());
   return true;
}

bool TSShape::buildConvexHull(S32 dl) const
{
   AssertFatal(dl>=0 && dl<mDetails.size(),"TSShape::buildConvexHull: detail out of range");

   bool ok = true;

   const Detail & detail = mDetails[dl];
   S32 ss = detail.subShapeNum;
   S32 od = detail.objectDetailNum;

   S32 start = mSubShapeFirstObject[ss];
   S32 end   = mSubShapeNumObjects[ss];
   for (S32 i=start; i<end; i++)
   {
      TSMesh * mesh = mMeshes[mObjects[i].startMeshIndex+od];
      if (!mesh)
         continue;
      ok &= mesh->buildConvexHull();
   }
   return ok;
}

Vector<MatrixF> gTempNodeTransforms(__FILE__, __LINE__);

void TSShape::computeBounds(S32 dl, Box3F & bounds) const
{
   // if dl==-1, nothing to do
   if (dl==-1)
      return;

   AssertFatal(dl>=0 && dl<mDetails.size(),"TSShapeInstance::computeBounds");

   // get subshape and object detail
   const TSDetail * detail = &mDetails[dl];
   S32 ss = detail->subShapeNum;
   S32 od = detail->objectDetailNum;

   // If we have no subshapes then there is
   // no valid bounds for this detail level.
   if ( ss < 0 )
      return;

   // set up temporary storage for non-local transforms...
   S32 i;
   S32 start = mSubShapeFirstNode[ss];
   S32 end   = mSubShapeNumNodes[ss] + start;
   gTempNodeTransforms.setSize(end-start);
   for (i=start; i<end; i++)
   {
      MatrixF mat;
      QuatF q;
      TSTransform::setMatrix(mDefaultRotations[i].getQuatF(&q),mDefaultTranslations[i],&mat);
      if (mNodes[i].parentIndex>=0)
         gTempNodeTransforms[i-start].mul(gTempNodeTransforms[mNodes[i].parentIndex-start],mat);
      else
         gTempNodeTransforms[i-start] = mat;
   }

   // run through objects and updating bounds as we go
   bounds.minExtents.set( 10E30f, 10E30f, 10E30f);
   bounds.maxExtents.set(-10E30f,-10E30f,-10E30f);
   Box3F box;
   start = mSubShapeFirstObject[ss];
   end   = mSubShapeNumObjects[ss] + start;
   for (i=start; i<end; i++)
   {
      const Object * object = &mObjects[i];
      TSMesh * mesh = od<object->numMeshes ? mMeshes[object->startMeshIndex+od] : NULL;
      if (mesh)
      {
         static MatrixF idMat(true);
         if (object->nodeIndex<0)
            mesh->computeBounds(idMat,box);
         else
            mesh->computeBounds(gTempNodeTransforms[object->nodeIndex-start],box);
         bounds.minExtents.setMin(box.minExtents);
         bounds.maxExtents.setMax(box.maxExtents);
      }
   }
}

TSShapeAlloc TSShape::smTSAlloc;

#define tsalloc TSShape::smTSAlloc


// messy stuff: check to see if we should "skip" meshNum
// this assumes that meshes for a given object are in a row
// skipDL is the lowest detail number we keep (i.e., the # of details we skip)
bool TSShape::checkSkip(S32 meshNum, S32 & curObject, S32 skipDL)
{
   if (skipDL==0)
      // easy out...
      return false;

   // skip detail level exists on this subShape
   S32 skipSS = mDetails[skipDL].subShapeNum;

   if (curObject<mObjects.size())
   {
      S32 start = mObjects[curObject].startMeshIndex;
      if (meshNum>=start)
      {
         // we are either from this object, the next object, or a decal
         if (meshNum < start + mObjects[curObject].numMeshes)
         {
            // this object...
            if (mSubShapeFirstObject[skipSS]>curObject)
               // haven't reached this subshape yet
               return true;
            if (skipSS+1==mSubShapeFirstObject.size() || curObject<mSubShapeFirstObject[skipSS+1])
               // curObject is on subshape of skip detail...make sure it's after skipDL
               return (meshNum-start<mDetails[skipDL].objectDetailNum);
            // if we get here, then curObject occurs on subShape after skip detail (so keep it)
            return false;
         }
         else
            // advance object, try again
            return checkSkip(meshNum,++curObject,skipDL);
      }
   }

   AssertFatal(0,"TSShape::checkSkip: assertion failed");
   return false;
}

void TSShape::assembleShape()
{
   S32 i,j;

   // get counts...
   S32 numNodes = tsalloc.get32();
   S32 numObjects = tsalloc.get32();
   S32 numDecals = tsalloc.get32();
   S32 numSubShapes = tsalloc.get32();
   S32 numIflMaterials = tsalloc.get32();
   S32 numNodeRots;
   S32 numNodeTrans;
   S32 numNodeUniformScales;
   S32 numNodeAlignedScales;
   S32 numNodeArbitraryScales;
   if (smReadVersion<22)
   {
      numNodeRots = numNodeTrans = tsalloc.get32() - numNodes;
      numNodeUniformScales = numNodeAlignedScales = numNodeArbitraryScales = 0;
   }
   else
   {
      numNodeRots = tsalloc.get32();
      numNodeTrans = tsalloc.get32();
      numNodeUniformScales = tsalloc.get32();
      numNodeAlignedScales = tsalloc.get32();
      numNodeArbitraryScales = tsalloc.get32();
   }
   S32 numGroundFrames = 0;
   if (smReadVersion>23)
      numGroundFrames = tsalloc.get32();
   S32 numObjectStates = tsalloc.get32();
   S32 numDecalStates = tsalloc.get32();
   S32 numTriggers = tsalloc.get32();
   S32 numDetails = tsalloc.get32();
   S32 numMeshes = tsalloc.get32();
   S32 numSkins = 0;
   if (smReadVersion<23)
      // in later versions, skins are kept with other meshes
      numSkins = tsalloc.get32();
   S32 numNames = tsalloc.get32();

   // Note that we are recalculating these values later on for safety.
   mSmallestVisibleSize = (F32)tsalloc.get32();
   mSmallestVisibleDL   = tsalloc.get32();
   
   tsalloc.checkGuard();

   // get bounds...
   tsalloc.get32((S32*)&mRadius,1);
   tsalloc.get32((S32*)&mTubeRadius,1);
   tsalloc.get32((S32*)&mCenter,3);
   tsalloc.get32((S32*)&mBounds,6);

   tsalloc.checkGuard();

   // copy various vectors...
   S32 * ptr32 = tsalloc.copyToShape32(numNodes*5);
   mNodes.set(ptr32,numNodes);

   tsalloc.checkGuard();

   ptr32 = tsalloc.copyToShape32(numObjects*6,true);
   if (!ptr32)
      ptr32 = tsalloc.allocShape32(numSkins*6); // pre v23 shapes store skins and meshes separately...no longer
   else
      tsalloc.allocShape32(numSkins*6);
   mObjects.set(ptr32,numObjects);

   tsalloc.checkGuard();

   // DEPRECATED decals
   ptr32 = tsalloc.getPointer32(numDecals*5);

   tsalloc.checkGuard();

   // DEPRECATED ifl materials
   ptr32 = tsalloc.copyToShape32(numIflMaterials*5);

   tsalloc.checkGuard();

   ptr32 = tsalloc.copyToShape32(numSubShapes,true);
   mSubShapeFirstNode.set(ptr32,numSubShapes);
   ptr32 = tsalloc.copyToShape32(numSubShapes,true);
   mSubShapeFirstObject.set(ptr32,numSubShapes);
   // DEPRECATED subShapeFirstDecal
   ptr32 = tsalloc.getPointer32(numSubShapes);

   tsalloc.checkGuard();

   ptr32 = tsalloc.copyToShape32(numSubShapes);
   mSubShapeNumNodes.set(ptr32,numSubShapes);
   ptr32 = tsalloc.copyToShape32(numSubShapes);
   mSubShapeNumObjects.set(ptr32,numSubShapes);
   // DEPRECATED subShapeNumDecals
   ptr32 = tsalloc.getPointer32(numSubShapes);

   tsalloc.checkGuard();

   ptr32 = tsalloc.allocShape32(numSubShapes);
   mSubShapeFirstTranslucentObject.set(ptr32,numSubShapes);

   // get default translation and rotation
   S16 * ptr16 = tsalloc.allocShape16(0);
   for (i=0;i<numNodes;i++)
      tsalloc.copyToShape16(4);
   mDefaultRotations.set(ptr16,numNodes);
   tsalloc.align32();
   ptr32 = tsalloc.allocShape32(0);
   for (i=0;i<numNodes;i++)
   {
      tsalloc.copyToShape32(3);
      tsalloc.copyToShape32(sizeof(Point3F)-12); // handle alignment issues w/ point3f
   }
   mDefaultTranslations.set(ptr32,numNodes);

   // get any node sequence data stored in shape
   mNodeTranslations.setSize(numNodeTrans);
   for (i=0;i<numNodeTrans;i++)
      tsalloc.get32((S32*)&mNodeTranslations[i],3);
   mNodeRotations.setSize(numNodeRots);
   for (i=0;i<numNodeRots;i++)
      tsalloc.get16((S16*)&mNodeRotations[i],4);
   tsalloc.align32();

   tsalloc.checkGuard();

   if (smReadVersion>21)
   {
      // more node sequence data...scale
      mNodeUniformScales.setSize(numNodeUniformScales);
      for (i=0;i<numNodeUniformScales;i++)
         tsalloc.get32((S32*)&mNodeUniformScales[i],1);
      mNodeAlignedScales.setSize(numNodeAlignedScales);
      for (i=0;i<numNodeAlignedScales;i++)
         tsalloc.get32((S32*)&mNodeAlignedScales[i],3);
      mNodeArbitraryScaleFactors.setSize(numNodeArbitraryScales);
      for (i=0;i<numNodeArbitraryScales;i++)
         tsalloc.get32((S32*)&mNodeArbitraryScaleFactors[i],3);
      mNodeArbitraryScaleRots.setSize(numNodeArbitraryScales);
      for (i=0;i<numNodeArbitraryScales;i++)
         tsalloc.get16((S16*)&mNodeArbitraryScaleRots[i],4);
      tsalloc.align32();

      tsalloc.checkGuard();
   }

   // old shapes need ground transforms moved to ground arrays...but only do it once
   if (smReadVersion<22 && tsalloc.allocShape32(0))
   {
      for (i=0; i<mSequences.size(); i++)
      {
         // move ground transform data to ground vectors
         Sequence & seq = mSequences[i];
         S32 oldSz = mGroundTranslations.size();
         mGroundTranslations.setSize(oldSz+seq.numGroundFrames);
         mGroundRotations.setSize(oldSz+seq.numGroundFrames);
         for (S32 j=0;j<seq.numGroundFrames;j++)
         {
            mGroundTranslations[j+oldSz] = mNodeTranslations[seq.firstGroundFrame+j-numNodes];
            mGroundRotations[j+oldSz] = mNodeRotations[seq.firstGroundFrame+j-numNodes];
         }
         seq.firstGroundFrame = oldSz;
         seq.baseTranslation -= numNodes;
         seq.baseRotation -= numNodes;
         seq.baseScale = 0; // not used on older shapes...but keep it clean
      }
   }

   // version 22 & 23 shapes accidentally had no ground transforms, and ground for
   // earlier shapes is handled just above, so...
   if (smReadVersion>23)
   {
      mGroundTranslations.setSize(numGroundFrames);
      for (i=0;i<numGroundFrames;i++)
         tsalloc.get32((S32*)&mGroundTranslations[i],3);
      mGroundRotations.setSize(numGroundFrames);
      for (i=0;i<numGroundFrames;i++)
         tsalloc.get16((S16*)&mGroundRotations[i],4);
      tsalloc.align32();

      tsalloc.checkGuard();
   }

   // object states
   ptr32 = tsalloc.copyToShape32(numObjectStates*3);
   mObjectStates.set(ptr32,numObjectStates);
   tsalloc.allocShape32(numSkins*3); // provide buffer after objectStates for older shapes

   tsalloc.checkGuard();

   // DEPRECATED decal states
   ptr32 = tsalloc.getPointer32(numDecalStates);

   tsalloc.checkGuard();

   // frame triggers
   ptr32 = tsalloc.getPointer32(numTriggers*2);
   mTriggers.setSize(numTriggers);
   dMemcpy(mTriggers.address(),ptr32,sizeof(S32)*numTriggers*2);

   tsalloc.checkGuard();

   // details
   if ( smReadVersion >= 26 )
   {
      U32 alignedSize32 = sizeof( Detail ) / 4;
      ptr32 = tsalloc.copyToShape32( numDetails * alignedSize32, true );
      mDetails.set( ptr32, numDetails );
   }
   else
   {
      // Previous to version 26 the Detail structure
      // only contained the first 7 values...
      //
      //    struct Detail
      //    {
      //       S32 nameIndex;
      //       S32 subShapeNum;
      //       S32 objectDetailNum;
      //       F32 size;
      //       F32 averageError;
      //       F32 maxError;
      //       S32 polyCount;
      //    };
      //
      // In the code below we're reading just these 7 values and
      // copying them to the new larger structure.

      ptr32 = tsalloc.copyToShape32( numDetails * 7, true );

      mDetails.setSize( numDetails );
      for ( U32 i = 0; i < mDetails.size(); i++, ptr32 += 7 )
      {
         Detail *det = &(mDetails[i]);

         // Clear the struct... we don't want to leave 
         // garbage in the parts that are unfilled.
         U32 alignedSize32 = sizeof( Detail );
         dMemset( det, 0, alignedSize32 );

         // Copy the old struct values over.
         dMemcpy( det, ptr32, 7 * 4 );

         // If this is an autobillboard then we need to
         // fill in the new part of the struct.
         if ( det->subShapeNum >= 0 )
            continue; 

         S32 lastDetailOpts = det->objectDetailNum;
         det->bbEquatorSteps = lastDetailOpts & 0x7F; // bits 0..6
         det->bbPolarSteps = (lastDetailOpts >> 7) & 0x3F; // bits 7..12
         det->bbPolarAngle = 0.5f * M_PI_F * (1.0f/64.0f) * (F32) (( lastDetailOpts >>13 ) & 0x3F); // bits 13..18
         det->bbDetailLevel = (lastDetailOpts >> 19) & 0x0F;  // 19..22
         det->bbDimension = (lastDetailOpts >> 23) & 0xFF; // 23..30
         det->bbIncludePoles = (lastDetailOpts & 0x80000000)!=0; // bit 31
      }
   }

   // Some DTS exporters (MAX - I'm looking at you!) write garbage into the
   // averageError and maxError values which stops LOD from working correctly.
   // Try to detect and fix it
   for ( U32 i = 0; i < mDetails.size(); i++ )
   {
      if ( ( mDetails[i].averageError == 0 ) || ( mDetails[i].averageError > 10000 ) ||
           ( mDetails[i].maxError == 0 ) || ( mDetails[i].maxError > 10000 ) )
      {
         mDetails[i].averageError = mDetails[i].maxError = -1.0f;
      }
   }

   // We don't trust the value of mSmallestVisibleDL loaded from the dts
   // since some legacy meshes seem to have the wrong value. Recalculate it
   // now that we have the details loaded.
   updateSmallestVisibleDL();

   S32 skipDL = getMin(mSmallestVisibleDL,smNumSkipLoadDetails);
   if (skipDL < 0)
      skipDL = 0;


   tsalloc.checkGuard();

   // about to read in the meshes...first must allocate some scratch space
   S32 scratchSize = getMax(numSkins,numMeshes);
   TSMesh::smVertsList.setSize(scratchSize);
   TSMesh::smTVertsList.setSize(scratchSize);

   if ( smReadVersion >= 26 )
   {
      TSMesh::smTVerts2List.setSize(scratchSize);
      TSMesh::smColorsList.setSize(scratchSize);
   }

   TSMesh::smNormsList.setSize(scratchSize);
   TSMesh::smEncodedNormsList.setSize(scratchSize);
   TSMesh::smDataCopied.setSize(scratchSize);
   TSSkinMesh::smInitTransformList.setSize(scratchSize);
   TSSkinMesh::smVertexIndexList.setSize(scratchSize);
   TSSkinMesh::smBoneIndexList.setSize(scratchSize);
   TSSkinMesh::smWeightList.setSize(scratchSize);
   TSSkinMesh::smNodeIndexList.setSize(scratchSize);
   for (i=0; i<numMeshes; i++)
   {
      TSMesh::smVertsList[i]=NULL;
      TSMesh::smTVertsList[i]=NULL;
      
      if ( smReadVersion >= 26 )
      {
         TSMesh::smTVerts2List[i] = NULL;
         TSMesh::smColorsList[i] = NULL;
      }
      
      TSMesh::smNormsList[i]=NULL;
      TSMesh::smEncodedNormsList[i]=NULL;
      TSMesh::smDataCopied[i]=false;
      TSSkinMesh::smInitTransformList[i] = NULL;
      TSSkinMesh::smVertexIndexList[i] = NULL;
      TSSkinMesh::smBoneIndexList[i] = NULL;
      TSSkinMesh::smWeightList[i] = NULL;
      TSSkinMesh::smNodeIndexList[i] = NULL;
   }

   // read in the meshes (sans skins)...straightforward read one at a time
   ptr32 = tsalloc.allocShape32(numMeshes + numSkins*numDetails); // leave room for skins on old shapes
   S32 curObject = 0; // for tracking skipped meshes
   for (i=0; i<numMeshes; i++)
   {
      bool skip = checkSkip(i,curObject,skipDL); // skip this mesh?
      S32 meshType = tsalloc.get32();
      if (meshType == TSMesh::DecalMeshType)
         // decal mesh deprecated
         skip = true;
      TSMesh * mesh = TSMesh::assembleMesh(meshType,skip);
      if (ptr32)
         ptr32[i] = skip ?  0 : (S32)mesh;

      // fill in location of verts, tverts, and normals for detail levels
      if (mesh && meshType!=TSMesh::DecalMeshType)
      {
         TSMesh::smVertsList[i]  = mesh->mVerts.address();
         TSMesh::smTVertsList[i] = mesh->mTVerts.address();
         if (smReadVersion >= 26)
         {
            TSMesh::smTVerts2List[i] = mesh->mTVerts2.address();
            TSMesh::smColorsList[i] = mesh->mColors.address();
         }
         TSMesh::smNormsList[i]  = mesh->mNorms.address();
         TSMesh::smEncodedNormsList[i] = mesh->mEncodedNorms.address();
         TSMesh::smDataCopied[i] = !skip; // as long as we didn't skip this mesh, the data should be in shape now
         if (meshType==TSMesh::SkinMeshType)
         {
            TSSkinMesh * skin = (TSSkinMesh*)mesh;
            TSMesh::smVertsList[i]  = skin->mBatchData.initialVerts.address();
            TSMesh::smNormsList[i]  = skin->mBatchData.initialNorms.address();
            TSSkinMesh::smInitTransformList[i] = skin->mBatchData.initialTransforms.address();
            TSSkinMesh::smVertexIndexList[i] = skin->mVertexIndex.address();
            TSSkinMesh::smBoneIndexList[i] = skin->mBoneIndex.address();
            TSSkinMesh::smWeightList[i] = skin->mWeight.address();
            TSSkinMesh::smNodeIndexList[i] = skin->mBatchData.nodeIndex.address();
         }
      }
   }
   mMeshes.set(ptr32,numMeshes);

   tsalloc.checkGuard();

   // names
   char * nameBufferStart = (char*)tsalloc.getPointer8(0);
   char * name = nameBufferStart;
   S32 nameBufferSize = 0;
   mNames.setSize(numNames);
   for (i=0; i<numNames; i++)
   {
      for (j=0; name[j]; j++)
         ;

      mNames[i] = name;
      nameBufferSize += j + 1;
      name += j + 1;
   }

   tsalloc.getPointer8(nameBufferSize);
   tsalloc.align32();

   tsalloc.checkGuard();

   if (smReadVersion<23)
   {
      // get detail information about skins...
      S32 * detailFirstSkin = tsalloc.getPointer32(numDetails);
      S32 * detailNumSkins = tsalloc.getPointer32(numDetails);

      tsalloc.checkGuard();

      // about to read in skins...clear out scratch space...
      if (numSkins)
      {
         TSSkinMesh::smInitTransformList.setSize(numSkins);
         TSSkinMesh::smVertexIndexList.setSize(numSkins);
         TSSkinMesh::smBoneIndexList.setSize(numSkins);
         TSSkinMesh::smWeightList.setSize(numSkins);
         TSSkinMesh::smNodeIndexList.setSize(numSkins);
      }
      for (i=0; i<numSkins; i++)
      {
         TSMesh::smVertsList[i]=NULL;
         TSMesh::smTVertsList[i]=NULL;
         TSMesh::smNormsList[i]=NULL;
         TSMesh::smEncodedNormsList[i]=NULL;
         TSMesh::smDataCopied[i]=false;
         TSSkinMesh::smInitTransformList[i] = NULL;
         TSSkinMesh::smVertexIndexList[i] = NULL;
         TSSkinMesh::smBoneIndexList[i] = NULL;
         TSSkinMesh::smWeightList[i] = NULL;
         TSSkinMesh::smNodeIndexList[i] = NULL;
      }

      // skins
      ptr32 = tsalloc.allocShape32(numSkins);
      for (i=0; i<numSkins; i++)
      {
         bool skip = i<detailFirstSkin[skipDL];
         TSSkinMesh * skin = (TSSkinMesh*)TSMesh::assembleMesh(TSMesh::SkinMeshType,skip);
         if (mMeshes.address())
         {
            // add pointer to skin in shapes list of meshes
            // we reserved room for this above...
            mMeshes.set(mMeshes.address(),mMeshes.size()+1);
            mMeshes[mMeshes.size()-1] = skip ? NULL : skin;
         }

         // fill in location of verts, tverts, and normals for shared detail levels
         if (skin)
         {
            TSMesh::smVertsList[i]  = skin->mBatchData.initialVerts.address();
            TSMesh::smTVertsList[i] = skin->mTVerts.address();
            TSMesh::smNormsList[i]  = skin->mBatchData.initialNorms.address();
            TSMesh::smEncodedNormsList[i]  = skin->mEncodedNorms.address();
            TSMesh::smDataCopied[i] = !skip; // as long as we didn't skip this mesh, the data should be in shape now
            TSSkinMesh::smInitTransformList[i] = skin->mBatchData.initialTransforms.address();
            TSSkinMesh::smVertexIndexList[i] = skin->mVertexIndex.address();
            TSSkinMesh::smBoneIndexList[i] = skin->mBoneIndex.address();
            TSSkinMesh::smWeightList[i] = skin->mWeight.address();
            TSSkinMesh::smNodeIndexList[i] = skin->mBatchData.nodeIndex.address();
         }
      }

      tsalloc.checkGuard();

      // we now have skins in mesh list...add skin objects to object list and patch things up
      fixupOldSkins(numMeshes,numSkins,numDetails,detailFirstSkin,detailNumSkins);
   }

   // allocate storage space for some arrays (filled in during Shape::init)...
   ptr32 = tsalloc.allocShape32(numDetails);
   mAlphaIn.set(ptr32,numDetails);
   ptr32 = tsalloc.allocShape32(numDetails);
   mAlphaOut.set(ptr32,numDetails);
}

void TSShape::disassembleShape()
{
   S32 i;

   // set counts...
   S32 numNodes = tsalloc.set32(mNodes.size());
   S32 numObjects = tsalloc.set32(mObjects.size());
   tsalloc.set32(0); // DEPRECATED decals
   S32 numSubShapes = tsalloc.set32(mSubShapeFirstNode.size());
   tsalloc.set32(0); // DEPRECATED ifl materials
   S32 numNodeRotations = tsalloc.set32(mNodeRotations.size());
   S32 numNodeTranslations = tsalloc.set32(mNodeTranslations.size());
   S32 numNodeUniformScales = tsalloc.set32(mNodeUniformScales.size());
   S32 numNodeAlignedScales = tsalloc.set32(mNodeAlignedScales.size());
   S32 numNodeArbitraryScales = tsalloc.set32(mNodeArbitraryScaleFactors.size());
   S32 numGroundFrames = tsalloc.set32(mGroundTranslations.size());
   S32 numObjectStates = tsalloc.set32(mObjectStates.size());
   tsalloc.set32(0); // DEPRECATED decals
   S32 numTriggers = tsalloc.set32(mTriggers.size());
   S32 numDetails = tsalloc.set32(mDetails.size());
   S32 numMeshes = tsalloc.set32(mMeshes.size());
   S32 numNames = tsalloc.set32(mNames.size());
   tsalloc.set32((S32)mSmallestVisibleSize);
   tsalloc.set32(mSmallestVisibleDL);

   tsalloc.setGuard();

   // get bounds...
   tsalloc.copyToBuffer32((S32*)&mRadius,1);
   tsalloc.copyToBuffer32((S32*)&mTubeRadius,1);
   tsalloc.copyToBuffer32((S32*)&mCenter,3);
   tsalloc.copyToBuffer32((S32*)&mBounds,6);

   tsalloc.setGuard();

   // copy various vectors...
   tsalloc.copyToBuffer32((S32*)mNodes.address(),numNodes*5);
   tsalloc.setGuard();
   tsalloc.copyToBuffer32((S32*)mObjects.address(),numObjects*6);
   tsalloc.setGuard();
   // DEPRECATED: no copy decals
   tsalloc.setGuard();
   tsalloc.copyToBuffer32(0,0); // DEPRECATED: ifl materials!
   tsalloc.setGuard();
   tsalloc.copyToBuffer32((S32*)mSubShapeFirstNode.address(),numSubShapes);
   tsalloc.copyToBuffer32((S32*)mSubShapeFirstObject.address(),numSubShapes);
   tsalloc.copyToBuffer32(0, numSubShapes); // DEPRECATED: no copy subShapeFirstDecal
   tsalloc.setGuard();
   tsalloc.copyToBuffer32((S32*)mSubShapeNumNodes.address(),numSubShapes);
   tsalloc.copyToBuffer32((S32*)mSubShapeNumObjects.address(),numSubShapes);
   tsalloc.copyToBuffer32(0, numSubShapes); // DEPRECATED: no copy subShapeNumDecals
   tsalloc.setGuard();

   // default transforms...
   tsalloc.copyToBuffer16((S16*)mDefaultRotations.address(),numNodes*4);
   tsalloc.copyToBuffer32((S32*)mDefaultTranslations.address(),numNodes*3);

   // animated transforms...
   tsalloc.copyToBuffer16((S16*)mNodeRotations.address(),numNodeRotations*4);
   tsalloc.copyToBuffer32((S32*)mNodeTranslations.address(),numNodeTranslations*3);

   tsalloc.setGuard();

   // ...with scale
   tsalloc.copyToBuffer32((S32*)mNodeUniformScales.address(),numNodeUniformScales);
   tsalloc.copyToBuffer32((S32*)mNodeAlignedScales.address(),numNodeAlignedScales*3);
   tsalloc.copyToBuffer32((S32*)mNodeArbitraryScaleFactors.address(),numNodeArbitraryScales*3);
   tsalloc.copyToBuffer16((S16*)mNodeArbitraryScaleRots.address(),numNodeArbitraryScales*4);

   tsalloc.setGuard();

   tsalloc.copyToBuffer32((S32*)mGroundTranslations.address(),3*numGroundFrames);
   tsalloc.copyToBuffer16((S16*)mGroundRotations.address(),4*numGroundFrames);

   tsalloc.setGuard();

   // object states..
   tsalloc.copyToBuffer32((S32*)mObjectStates.address(),numObjectStates*3);
   tsalloc.setGuard();

   // decal states...
   // DEPRECATED (numDecalStates = 0)
   tsalloc.setGuard();

   // frame triggers
   tsalloc.copyToBuffer32((S32*)mTriggers.address(),numTriggers*2);
   tsalloc.setGuard();

   // details
   if (TSShape::smVersion > 25)
   {
      U32 alignedSize32 = sizeof( Detail ) / 4;
      tsalloc.copyToBuffer32((S32*)mDetails.address(),numDetails * alignedSize32 );
   }
   else
   {
      // Legacy details => no explicit autobillboard parameters
      U32 legacyDetailSize32 = 7;   // only store the first 7 4-byte values of each detail
      for ( S32 i = 0; i < mDetails.size(); i++ )
         tsalloc.copyToBuffer32( (S32*)&mDetails[i], legacyDetailSize32 );
   }
   tsalloc.setGuard();

   // read in the meshes (sans skins)...
   bool * isMesh = new bool[numMeshes]; // funny business because decals are pretend meshes (legacy issue)
   for (i=0;i<numMeshes;i++)
      isMesh[i]=false;
   for (i=0; i<mObjects.size(); i++)
   {
      for (S32 j=0; j<mObjects[i].numMeshes; j++)
         // even if an empty mesh, it's a mesh...
         isMesh[mObjects[i].startMeshIndex+j]=true;
   }
   for (i=0; i<numMeshes; i++)
   {
      TSMesh * mesh = NULL;
      // decal mesh deprecated
      if (isMesh[i])
         mesh = mMeshes[i];
      tsalloc.set32( (mesh && mesh->getMeshType() != TSMesh::DecalMeshType) ? mesh->getMeshType() : TSMesh::NullMeshType);
      if (mesh)
         mesh->disassemble();
   }
   delete [] isMesh;
   tsalloc.setGuard();

   // names
   for (i=0; i<numNames; i++)
      tsalloc.copyToBuffer8((S8 *)(mNames[i].c_str()),mNames[i].length()+1);

   tsalloc.setGuard();
}

//-------------------------------------------------
// write whole shape
//-------------------------------------------------
/** Determine whether we can write this shape in TSTPRO compatible format */
bool TSShape::canWriteOldFormat() const
{
   // Cannot use old format if using autobillboard details
   for (S32 i = 0; i < mDetails.size(); i++)
   {
      if (mDetails[i].subShapeNum < 0)
         return false;
   }

   for (S32 i = 0; i < mMeshes.size(); i++)
   {
      if (!mMeshes[i])
         continue;

      // Cannot use old format if using the new functionality (COLORs, 2nd UV set)
      if (mMeshes[i]->mTVerts2.size() || mMeshes[i]->mColors.size())
         return false;

      // Cannot use old format if any primitive has too many triangles
      // (ie. cannot fit in a S16)
      for (S32 j = 0; j < mMeshes[i]->mPrimitives.size(); j++)
      {
         if ((mMeshes[i]->mPrimitives[j].start +
               mMeshes[i]->mPrimitives[j].numElements) >= (1 << 15))
         {
            return false;
         }
      }
   }

   return true;
}

void TSShape::write(Stream * s, bool saveOldFormat)
{
   S32 currentVersion = smVersion;
   if (saveOldFormat)
      smVersion = 24;

   // write version
   s->write(smVersion | (mExporterVersion<<16));

   tsalloc.setWrite();
   disassembleShape();

   S32     * buffer32 = tsalloc.getBuffer32();
   S16     * buffer16 = tsalloc.getBuffer16();
   S8      * buffer8  = tsalloc.getBuffer8();

   S32 size32 = tsalloc.getBufferSize32();
   S32 size16 = tsalloc.getBufferSize16();
   S32 size8  = tsalloc.getBufferSize8();

   // convert sizes to dwords...
   if (size16 & 1)
      size16 += 2;
   size16 >>= 1;
   if (size8 & 3)
      size8 += 4;
   size8 >>= 2;

   S32 sizeMemBuffer, start16, start8;
   sizeMemBuffer = size32 + size16 + size8;
   start16 = size32;
   start8 = start16+size16;

   // in dwords -- write will properly endian-flip.
   s->write(sizeMemBuffer);
   s->write(start16);
   s->write(start8);

	// endian-flip the entire write buffers.
   fixEndian(buffer32,buffer16,buffer8,size32,size16,size8);

   // now write buffers
   s->write(size32*4,buffer32);
   s->write(size16*4,buffer16);
   s->write(size8 *4,buffer8);

   // write sequences - write will properly endian-flip.
   s->write(mSequences.size());
   for (S32 i=0; i<mSequences.size(); i++)
      mSequences[i].write(s);

   // write material list - write will properly endian-flip.
   mMaterialList->write(*s);

   delete [] buffer32;
   delete [] buffer16;
   delete [] buffer8;

   smVersion = currentVersion;
}

//-------------------------------------------------
// read whole shape
//-------------------------------------------------

bool TSShape::read(Stream * s)
{
   // read version - read handles endian-flip
   s->read(&smReadVersion);
   mExporterVersion = smReadVersion >> 16;
   smReadVersion &= 0xFF;
   if (smReadVersion>smVersion)
   {
      // error -- don't support future versions yet :>
      Con::errorf(ConsoleLogEntry::General,
                  "Error: attempt to load a version %i dts-shape, can currently only load version %i and before.",
                   smReadVersion,smVersion);
      return false;
   }
   mReadVersion = smReadVersion;

   S32 * memBuffer32;
   S16 * memBuffer16;
   S8 * memBuffer8;
   S32 count32, count16, count8;
   if (mReadVersion<19)
   {
      Con::errorf("... Shape with old version.");
      return false;
   }
   else
   {
      S32 i;
      U32 sizeMemBuffer, startU16, startU8;

      // in dwords. - read handles endian-flip
      s->read(&sizeMemBuffer);
      s->read(&startU16);
      s->read(&startU8);

      if (s->getStatus()!=Stream::Ok)
      {
         Con::errorf(ConsoleLogEntry::General, "Error: bad shape file.");
         return false;
      }

      S32 * tmp = new S32[sizeMemBuffer];
      s->read(sizeof(S32)*sizeMemBuffer,(U8*)tmp);
      memBuffer32 = tmp;
      memBuffer16 = (S16*)(tmp+startU16);
      memBuffer8  = (S8*)(tmp+startU8);

      count32 = startU16;
      count16 = startU8-startU16;
      count8  = sizeMemBuffer-startU8;

      // read sequences
      S32 numSequences;
      s->read(&numSequences);
      mSequences.setSize(numSequences);
      for (i=0; i<numSequences; i++)
      {
         mSequences[i].read(s);

         // Store initial (empty) source data
         mSequences[i].sourceData.total = mSequences[i].numKeyframes;
         mSequences[i].sourceData.end = mSequences[i].sourceData.total - 1;
      }

      // read material list
      delete mMaterialList; // just in case...
      mMaterialList = new TSMaterialList;
      mMaterialList->read(*s);
   }

	// since we read in the buffers, we need to endian-flip their entire contents...
   fixEndian(memBuffer32,memBuffer16,memBuffer8,count32,count16,count8);

   tsalloc.setRead(memBuffer32,memBuffer16,memBuffer8,true);
   assembleShape(); // determine size of buffer needed
   mShapeDataSize = tsalloc.getSize();
   tsalloc.doAlloc();
   mShapeData = tsalloc.getBuffer();
   tsalloc.setRead(memBuffer32,memBuffer16,memBuffer8,false);
   assembleShape(); // copy to buffer
   AssertFatal(tsalloc.getSize()==mShapeDataSize,"TSShape::read: shape data buffer size mis-calculated");

   delete [] memBuffer32;

   if (smInitOnRead)
      init();

   //if (names.size() == 3 && dStricmp(names[2], "Box") == 0)
   //{
   //   Con::errorf("\nnodes.set(dMalloc(%d * sizeof(Node)), %d);", nodes.size(), nodes.size());
   //   for (U32 i = 0; i < nodes.size(); i++)
   //   {
   //      Node& obj = nodes[i];

   //      Con::errorf("   nodes[%d].nameIndex = %d;", i, obj.nameIndex);
   //      Con::errorf("   nodes[%d].parentIndex = %d;", i, obj.parentIndex);
   //      Con::errorf("   nodes[%d].firstObject = %d;", i, obj.firstObject);
   //      Con::errorf("   nodes[%d].firstChild = %d;", i, obj.firstChild);
   //      Con::errorf("   nodes[%d].nextSibling = %d;", i, obj.nextSibling);
   //   }

   //   Con::errorf("\nobjects.set(dMalloc(%d * sizeof(Object)), %d);", objects.size(), objects.size());
   //   for (U32 i = 0; i < objects.size(); i++)
   //   {
   //      Object& obj = objects[i];

   //      Con::errorf("   objects[%d].nameIndex = %d;", i, obj.nameIndex);
   //      Con::errorf("   objects[%d].numMeshes = %d;", i, obj.numMeshes);
   //      Con::errorf("   objects[%d].startMeshIndex = %d;", i, obj.startMeshIndex);
   //      Con::errorf("   objects[%d].nodeIndex = %d;", i, obj.nodeIndex);
   //      Con::errorf("   objects[%d].nextSibling = %d;", i, obj.nextSibling);
   //      Con::errorf("   objects[%d].firstDecal = %d;", i, obj.firstDecal);
   //   }

   //   Con::errorf("\nobjectStates.set(dMalloc(%d * sizeof(ObjectState)), %d);", objectStates.size(), objectStates.size());
   //   for (U32 i = 0; i < objectStates.size(); i++)
   //   {
   //      ObjectState& obj = objectStates[i];

   //      Con::errorf("   objectStates[%d].vis = %g;", i, obj.vis);
   //      Con::errorf("   objectStates[%d].frameIndex = %d;", i, obj.frameIndex);
   //      Con::errorf("   objectStates[%d].matFrameIndex = %d;", i, obj.matFrameIndex);
   //   }
   //   Con::errorf("\nsubShapeFirstNode.set(dMalloc(%d * sizeof(S32)), %d);", subShapeFirstNode.size(), subShapeFirstNode.size());
   //   for (U32 i = 0; i < subShapeFirstNode.size(); i++)
   //      Con::errorf("   subShapeFirstNode[%d] = %d;", i, subShapeFirstNode[i]);

   //   Con::errorf("\nsubShapeFirstObject.set(dMalloc(%d * sizeof(S32)), %d);", subShapeFirstObject.size(), subShapeFirstObject.size());
   //   for (U32 i = 0; i < subShapeFirstObject.size(); i++)
   //      Con::errorf("   subShapeFirstObject[%d] = %d;", i, subShapeFirstObject[i]);

   //   //Con::errorf("numDetailFirstSkins = %d", detailFirstSkin.size());
   //   Con::errorf("\nsubShapeNumNodes.set(dMalloc(%d * sizeof(S32)), %d);", subShapeNumNodes.size(), subShapeNumNodes.size());
   //   for (U32 i = 0; i < subShapeNumNodes.size(); i++)
   //      Con::errorf("   subShapeNumNodes[%d] = %d;", i, subShapeNumNodes[i]);

   //   Con::errorf("\nsubShapeNumObjects.set(dMalloc(%d * sizeof(S32)), %d);", subShapeNumObjects.size(), subShapeNumObjects.size());
   //   for (U32 i = 0; i < subShapeNumObjects.size(); i++)
   //      Con::errorf("   subShapeNumObjects[%d] = %d;", i, subShapeNumObjects[i]);

   //   Con::errorf("\ndetails.set(dMalloc(%d * sizeof(Detail)), %d);", details.size(), details.size());
   //   for (U32 i = 0; i < details.size(); i++)
   //   {
   //      Detail& obj = details[i];

   //      Con::errorf("   details[%d].nameIndex = %d;", i, obj.nameIndex);
   //      Con::errorf("   details[%d].subShapeNum = %d;", i, obj.subShapeNum);
   //      Con::errorf("   details[%d].objectDetailNum = %d;", i, obj.objectDetailNum);
   //      Con::errorf("   details[%d].size = %g;", i, obj.size);
   //      Con::errorf("   details[%d].averageError = %g;", i, obj.averageError);
   //      Con::errorf("   details[%d].maxError = %g;", i, obj.maxError);
   //      Con::errorf("   details[%d].polyCount = %d;", i, obj.polyCount);
   //   }

   //   Con::errorf("\ndefaultRotations.set(dMalloc(%d * sizeof(Quat16)), %d);", defaultRotations.size(), defaultRotations.size());
   //   for (U32 i = 0; i < defaultRotations.size(); i++)
   //   {
   //      Con::errorf("   defaultRotations[%d].x = %g;", i, defaultRotations[i].x);
   //      Con::errorf("   defaultRotations[%d].y = %g;", i, defaultRotations[i].y);
   //      Con::errorf("   defaultRotations[%d].z = %g;", i, defaultRotations[i].z);
   //      Con::errorf("   defaultRotations[%d].w = %g;", i, defaultRotations[i].w);
   //   }

   //   Con::errorf("\ndefaultTranslations.set(dMalloc(%d * sizeof(Point3F)), %d);", defaultTranslations.size(), defaultTranslations.size());
   //   for (U32 i = 0; i < defaultTranslations.size(); i++)
   //      Con::errorf("   defaultTranslations[%d].set(%g, %g, %g);", i, defaultTranslations[i].x, defaultTranslations[i].y, defaultTranslations[i].z);

   //   Con::errorf("\nsubShapeFirstTranslucentObject.set(dMalloc(%d * sizeof(S32)), %d);", subShapeFirstTranslucentObject.size(), subShapeFirstTranslucentObject.size());
   //   for (U32 i = 0; i < subShapeFirstTranslucentObject.size(); i++)
   //      Con::errorf("   subShapeFirstTranslucentObject[%d] = %d;", i, subShapeFirstTranslucentObject[i]);

   //   Con::errorf("\nmeshes.set(dMalloc(%d * sizeof(TSMesh)), %d);", meshes.size(), meshes.size());
   //   for (U32 i = 0; i < meshes.size(); i++)
   //   {
   //      TSMesh* obj = meshes[i];

   //      if (obj)
   //      {
   //         Con::errorf("   meshes[%d]->meshType = %d;", i, obj->meshType);
   //         Con::errorf("   meshes[%d]->mBounds.minExtents.set(%g, %g, %g);", i, obj->mBounds.minExtents.x, obj->mBounds.minExtents.y, obj->mBounds.minExtents.z);
   //         Con::errorf("   meshes[%d]->mBounds.maxExtents.set(%g, %g, %g);", i, obj->mBounds.maxExtents.x, obj->mBounds.maxExtents.y, obj->mBounds.maxExtents.z);
   //         Con::errorf("   meshes[%d]->mCenter.set(%g, %g, %g);", i, obj->mCenter.x, obj->mCenter.y, obj->mCenter.z);
   //         Con::errorf("   meshes[%d]->mRadius = %g;", i, obj->mRadius);
   //         Con::errorf("   meshes[%d]->mVisibility = %g;", i, obj->mVisibility);
   //         Con::errorf("   meshes[%d]->mDynamic = %d;", i, obj->mDynamic);
   //         Con::errorf("   meshes[%d]->parentMesh = %d;", i, obj->parentMesh);
   //         Con::errorf("   meshes[%d]->numFrames = %d;", i, obj->numFrames);
   //         Con::errorf("   meshes[%d]->numMatFrames = %d;", i, obj->numMatFrames);
   //         Con::errorf("   meshes[%d]->vertsPerFrame = %d;", i, obj->vertsPerFrame);

   //         Con::errorf("\n   meshes[%d]->verts.set(dMalloc(%d * sizeof(Point3F)), %d);", obj->verts.size(), obj->verts.size());
   //         for (U32 j = 0; j < obj->verts.size(); j++)
   //            Con::errorf("   meshes[%d]->verts[%d].set(%g, %g, %g);", i, j, obj->verts[j].x, obj->verts[j].y, obj->verts[j].z);

   //         Con::errorf("\n   meshes[%d]->norms.set(dMalloc(%d * sizeof(Point3F)), %d);", obj->norms.size(), obj->norms.size());
   //         for (U32 j = 0; j < obj->norms.size(); j++)
   //            Con::errorf("   meshes[%d]->norms[%d].set(%g, %g, %g);", i, j, obj->norms[j].x, obj->norms[j].y, obj->norms[j].z);

   //         Con::errorf("\n   meshes[%d]->tverts.set(dMalloc(%d * sizeof(Point2F)), %d);", obj->tverts.size(), obj->tverts.size());
   //         for (U32 j = 0; j < obj->tverts.size(); j++)
   //            Con::errorf("   meshes[%d]->tverts[%d].set(%g, %g);", i, j, obj->tverts[j].x, obj->tverts[j].y);

   //         Con::errorf("\n   meshes[%d]->primitives.set(dMalloc(%d * sizeof(TSDrawPrimitive)), %d);", obj->primitives.size(), obj->primitives.size());
   //         for (U32 j = 0; j < obj->primitives.size(); j++)
   //         {
   //            TSDrawPrimitive& prim = obj->primitives[j];

   //            Con::errorf("   meshes[%d]->primitives[%d].start = %d;", i, j, prim.start);
   //            Con::errorf("   meshes[%d]->primitives[%d].numElements = %d;", i, j, prim.numElements);
   //            Con::errorf("   meshes[%d]->primitives[%d].matIndex = %d;", i, j, prim.matIndex);
   //         }

   //         Con::errorf("\n   meshes[%d]->encodedNorms.set(dMalloc(%d * sizeof(U8)), %d);", obj->encodedNorms.size(), obj->encodedNorms.size());
   //         for (U32 j = 0; j < obj->encodedNorms.size(); j++)
   //            Con::errorf("   meshes[%d]->encodedNorms[%d] = %c;", i, j, obj->encodedNorms[j]);

   //         Con::errorf("\n   meshes[%d]->indices.set(dMalloc(%d * sizeof(U16)), %d);", obj->indices.size(), obj->indices.size());
   //         for (U32 j = 0; j < obj->indices.size(); j++)
   //            Con::errorf("   meshes[%d]->indices[%d] = %d;", i, j, obj->indices[j]);

   //         Con::errorf("\n   meshes[%d]->initialTangents.set(dMalloc(%d * sizeof(Point3F)), %d);", obj->initialTangents.size(), obj->initialTangents.size());
   //         for (U32 j = 0; j < obj->initialTangents.size(); j++)
   //            Con::errorf("   meshes[%d]->initialTangents[%d].set(%g, %g, %g);", i, j, obj->initialTangents[j].x, obj->initialTangents[j].y, obj->initialTangents[j].z);

   //         Con::errorf("\n   meshes[%d]->tangents.set(dMalloc(%d * sizeof(Point4F)), %d);", obj->tangents.size(), obj->tangents.size());
   //         for (U32 j = 0; j < obj->tangents.size(); j++)
   //            Con::errorf("   meshes[%d]->tangents[%d].set(%g, %g, %g, %g);", i, j, obj->tangents[j].x, obj->tangents[j].y, obj->tangents[j].z, obj->tangents[j].w);

   //         Con::errorf("   meshes[%d]->billboardAxis.set(%g, %g, %g);", i, obj->billboardAxis.x, obj->billboardAxis.y, obj->billboardAxis.z);

   //         Con::errorf("\n   meshes[%d]->planeNormals.set(dMalloc(%d * sizeof(Point3F)), %d);", obj->planeNormals.size(), obj->planeNormals.size());
   //         for (U32 j = 0; j < obj->planeNormals.size(); j++)
   //            Con::errorf("   meshes[%d]->planeNormals[%d].set(%g, %g, %g);", i, j, obj->planeNormals[j].x, obj->planeNormals[j].y, obj->planeNormals[j].z);

   //         Con::errorf("\n   meshes[%d]->planeConstants.set(dMalloc(%d * sizeof(F32)), %d);", obj->planeConstants.size(), obj->planeConstants.size());
   //         for (U32 j = 0; j < obj->planeConstants.size(); j++)
   //            Con::errorf("   meshes[%d]->planeConstants[%d] = %g;", i, j, obj->planeConstants[j]);

   //         Con::errorf("\n   meshes[%d]->planeMaterials.set(dMalloc(%d * sizeof(U32)), %d);", obj->planeMaterials.size(), obj->planeMaterials.size());
   //         for (U32 j = 0; j < obj->planeMaterials.size(); j++)
   //            Con::errorf("   meshes[%d]->planeMaterials[%d] = %d;", i, j, obj->planeMaterials[j]);

   //         Con::errorf("   meshes[%d]->planesPerFrame = %d;", i, obj->planesPerFrame);
   //         Con::errorf("   meshes[%d]->mergeBufferStart = %d;", i, obj->mergeBufferStart);
   //      }
   //   }

   //   Con::errorf("\nalphaIn.set(dMalloc(%d * sizeof(F32)), %d);", alphaIn.size(), alphaIn.size());
   //   for (U32 i = 0; i < alphaIn.size(); i++)
   //      Con::errorf("   alphaIn[%d] = %g;", i, alphaIn[i]);

   //   Con::errorf("\nalphaOut.set(dMalloc(%d * sizeof(F32)), %d);", alphaOut.size(), alphaOut.size());
   //   for (U32 i = 0; i < alphaOut.size(); i++)
   //      Con::errorf("   alphaOut[%d] = %g;", i, alphaOut[i]);

   //   //Con::errorf("numSequences = %d", sequences.size());
   //   //Con::errorf("numNodeRotations = %d", nodeRotations.size());
   //   //Con::errorf("numNodeTranslations = %d", nodeTranslations.size());
   //   //Con::errorf("numNodeUniformScales = %d", nodeUniformScales.size());
   //   //Con::errorf("numNodeAlignedScales = %d", nodeAlignedScales.size());
   //   //Con::errorf("numNodeArbitraryScaleRots = %d", nodeArbitraryScaleRots.size());
   //   //Con::errorf("numNodeArbitraryScaleFactors = %d", nodeArbitraryScaleFactors.size());
   //   //Con::errorf("numGroundRotations = %d", groundRotations.size());
   //   //Con::errorf("numGroundTranslations = %d", groundTranslations.size());
   //   //Con::errorf("numTriggers = %d", triggers.size());
   //   //Con::errorf("numBillboardDetails = %d", billboardDetails.size());

   //   //Con::errorf("\nnumDetailCollisionAccelerators = %d", detailCollisionAccelerators.size());
   //   //for (U32 i = 0; i < detailCollisionAccelerators.size(); i++)
   //   //{
   //   //   ConvexHullAccelerator* obj = detailCollisionAccelerators[i];

   //   //   if (obj)
   //   //   {
   //   //      Con::errorf("   detailCollisionAccelerators[%d].numVerts = %d", i, obj->numVerts);

   //   //      for (U32 j = 0; j < obj->numVerts; j++)
   //   //      {
   //   //         Con::errorf("      verts[%d](%g, %g, %g)", j, obj->vertexList[j].x, obj->vertexList[j].y, obj->vertexList[j].z);
   //   //         Con::errorf("      norms[%d](%g, %g, %g)", j, obj->normalList[j].x, obj->normalList[j].y, obj->normalList[j].z);
   //   //         //U8**     emitStrings;
   //   //      }
   //   //   }
   //   //}

   //   Con::errorf("\nnames.setSize(%d);", names.size());
   //   for (U32 i = 0; i < names.size(); i++)
   //      Con::errorf("   names[%d] = StringTable->insert(\"%s\");", i, names[i]);

   //   //TSMaterialList * materialList;

   //   Con::errorf("\nradius = %g;", radius);
   //   Con::errorf("tubeRadius = %g;", tubeRadius);
   //   Con::errorf("center.set(%g, %g, %g);", center.x, center.y, center.z);
   //   Con::errorf("bounds.minExtents.set(%g, %g, %g);", bounds.minExtents.x, bounds.minExtents.y, bounds.minExtents.z);
   //   Con::errorf("bounds.maxExtents.set(%g, %g, %g);", bounds.maxExtents.x, bounds.maxExtents.y, bounds.maxExtents.z);

   //   Con::errorf("\nmExporterVersion = %d;", mExporterVersion);
   //   Con::errorf("mSmallestVisibleSize = %g;", mSmallestVisibleSize);
   //   Con::errorf("mSmallestVisibleDL = %d;", mSmallestVisibleDL);
   //   Con::errorf("mReadVersion = %d;", mReadVersion);
   //   Con::errorf("mFlags = %d;", mFlags);
   //   //Con::errorf("data = %d", data);
   //   Con::errorf("mSequencesConstructed = %d;", mSequencesConstructed);
   //}

   return true;
}

void TSShape::createEmptyShape()
{
   mNodes.set(dMalloc(1 * sizeof(Node)), 1);
      mNodes[0].nameIndex = 1;
      mNodes[0].parentIndex = -1;
      mNodes[0].firstObject = 0;
      mNodes[0].firstChild = -1;
      mNodes[0].nextSibling = -1;

   mObjects.set(dMalloc(1 * sizeof(Object)), 1);
      mObjects[0].nameIndex = 2;
      mObjects[0].numMeshes = 1;
      mObjects[0].startMeshIndex = 0;
      mObjects[0].nodeIndex = 0;
      mObjects[0].nextSibling = -1;
      mObjects[0].firstDecal = -1;

   mObjectStates.set(dMalloc(1 * sizeof(ObjectState)), 1);
      mObjectStates[0].vis = 1;
      mObjectStates[0].frameIndex = 0;
      mObjectStates[0].matFrameIndex = 0;

   mSubShapeFirstNode.set(dMalloc(1 * sizeof(S32)), 1);
      mSubShapeFirstNode[0] = 0;

   mSubShapeFirstObject.set(dMalloc(1 * sizeof(S32)), 1);
      mSubShapeFirstObject[0] = 0;

   mDetailFirstSkin.set(NULL, 0);

   mSubShapeNumNodes.set(dMalloc(1 * sizeof(S32)), 1);
      mSubShapeNumNodes[0] = 1;

   mSubShapeNumObjects.set(dMalloc(1 * sizeof(S32)), 1);
      mSubShapeNumObjects[0] = 1;

   mDetails.set(dMalloc(1 * sizeof(Detail)), 1);
      mDetails[0].nameIndex = 0;
      mDetails[0].subShapeNum = 0;
      mDetails[0].objectDetailNum = 0;
      mDetails[0].size = 2.0f;
      mDetails[0].averageError = -1.0f;
      mDetails[0].maxError = -1.0f;
      mDetails[0].polyCount = 0;

   mDefaultRotations.set(dMalloc(1 * sizeof(Quat16)), 1);
      mDefaultRotations[0].x = 0.0f;
      mDefaultRotations[0].y = 0.0f;
      mDefaultRotations[0].z = 0.0f;
      mDefaultRotations[0].w = 0.0f;

   mDefaultTranslations.set(dMalloc(1 * sizeof(Point3F)), 1);
      mDefaultTranslations[0].set(0.0f, 0.0f, 0.0f);

   mSubShapeFirstTranslucentObject.set(dMalloc(1 * sizeof(S32)), 1);
      mSubShapeFirstTranslucentObject[0] = 1;

   mAlphaIn.set(dMalloc(1 * sizeof(F32)), 1);
      mAlphaIn[0] = 0;

   mAlphaOut.set(dMalloc(1 * sizeof(F32)), 1);
      mAlphaOut[0] = -1;

   mSequences.set(NULL, 0);
   mNodeRotations.set(NULL, 0);
   mNodeTranslations.set(NULL, 0);
   mNodeUniformScales.set(NULL, 0);
   mNodeAlignedScales.set(NULL, 0);
   mNodeArbitraryScaleRots.set(NULL, 0);
   mNodeArbitraryScaleFactors.set(NULL, 0);
   mGroundRotations.set(NULL, 0);
   mGroundTranslations.set(NULL, 0);
   mTriggers.set(NULL, 0);
   mBillboardDetails.set(NULL, 0);

   mNames.setSize(3);
      mNames[0] = StringTable->insert("Detail2");
      mNames[1] = StringTable->insert("Mesh2");
      mNames[2] = StringTable->insert("Mesh");

   mRadius = 0.866025f;
   mTubeRadius = 0.707107f;
   mCenter.set(0.0f, 0.5f, 0.0f);
   mBounds.minExtents.set(-0.5f, 0.0f, -0.5f);
   mBounds.maxExtents.set(0.5f, 1.0f, 0.5f);

   mExporterVersion = 124;
   mSmallestVisibleSize = 2;
   mSmallestVisibleDL = 0;
   mReadVersion = 24;
   mFlags = 0;
   mSequencesConstructed = 0;

   mUseDetailFromScreenError = false;

   mDetailLevelLookup.setSize( 1 );
   mDetailLevelLookup[0].set( -1, 0 );

   // Init the collision accelerator array.  Note that we don't compute the
   //  accelerators until the app requests them
   mDetailCollisionAccelerators.setSize(mDetails.size());
   for (U32 i = 0; i < mDetailCollisionAccelerators.size(); i++)
      mDetailCollisionAccelerators[i] = NULL;
}

void TSShape::fixEndian(S32 * buff32, S16 * buff16, S8 *, S32 count32, S32 count16, S32)
{
	// if endian-ness isn't the same, need to flip the buffer contents.
   if (0x12345678!=convertLEndianToHost(0x12345678))
   {
      for (S32 i=0; i<count32; i++)
         buff32[i]=convertLEndianToHost(buff32[i]);
      for (S32 i=0; i<count16*2; i++)
         buff16[i]=convertLEndianToHost(buff16[i]);
   }
}

template<> void *Resource<TSShape>::create(const Torque::Path &path)
{
   // Execute the shape script if it exists
   Torque::Path scriptPath(path);
   scriptPath.setExtension("cs");

   // Don't execute the script if we're already doing so!
   StringTableEntry currentScript = Platform::stripBasePath(CodeBlock::getCurrentCodeBlockFullPath());
   if (!scriptPath.getFullPath().equal(currentScript))
   {
      Torque::Path scriptPathDSO(scriptPath);
      scriptPathDSO.setExtension("cs.dso");

      if (Torque::FS::IsFile(scriptPathDSO) || Torque::FS::IsFile(scriptPath))
      {
         String evalCmd = "exec(\"" + scriptPath + "\");";

         String instantGroup = Con::getVariable("InstantGroup");
         Con::setIntVariable("InstantGroup", RootGroupId);
         Con::evaluate((const char*)evalCmd.c_str(), false, scriptPath.getFullPath());
         Con::setVariable("InstantGroup", instantGroup.c_str());
      }
   }

   // Attempt to load the shape
   TSShape * ret = 0;
   bool readSuccess = false;
   const String extension = path.getExtension();

   if ( extension.equal( "dts", String::NoCase ) )
   {
      FileStream stream;
      stream.open( path.getFullPath(), Torque::FS::File::Read );
      if ( stream.getStatus() != Stream::Ok )
      {
         Con::errorf( "Resource<TSShape>::create - Could not open '%s'", path.getFullPath().c_str() );
         return NULL;
      }

      ret = new TSShape;
      readSuccess = ret->read(&stream);
   }
   else if ( extension.equal( "dae", String::NoCase ) || extension.equal( "kmz", String::NoCase ) )
   {
#ifdef TORQUE_COLLADA
      // Attempt to load the DAE file
      ret = loadColladaShape(path);
      readSuccess = (ret != NULL);
#else
      // No COLLADA support => attempt to load the cached DTS file instead
      Torque::Path cachedPath = path;
      cachedPath.setExtension("cached.dts");
       
      FileStream stream;
      stream.open( cachedPath.getFullPath(), Torque::FS::File::Read );
      if ( stream.getStatus() != Stream::Ok )
      {
         Con::errorf( "Resource<TSShape>::create - Could not open '%s'", cachedPath.getFullPath().c_str() );
         return NULL;
      }
      ret = new TSShape;
      readSuccess = ret->read(&stream);
#endif
   }
   else
   {
      Con::errorf( "Resource<TSShape>::create - '%s' has an unknown file format", path.getFullPath().c_str() );
      delete ret;
      return NULL;
   }

   if( !readSuccess )
   {
      Con::errorf( "Resource<TSShape>::create - Error reading '%s'", path.getFullPath().c_str() );
      delete ret;
      ret = NULL;
   }

   return ret;
}

template<> ResourceBase::Signature  Resource<TSShape>::signature()
{
   return MakeFourCC('t','s','s','h');
}

TSShape::ConvexHullAccelerator* TSShape::getAccelerator(S32 dl)
{
   AssertFatal(dl < mDetails.size(), "Error, bad detail level!");
   if (dl == -1)
      return NULL;

   AssertFatal( mDetailCollisionAccelerators.size() == mDetails.size(), 
      "TSShape::getAccelerator() - mismatched array sizes!" );

   if (mDetailCollisionAccelerators[dl] == NULL)
      computeAccelerator(dl);

   AssertFatal(mDetailCollisionAccelerators[dl] != NULL, "This should be non-null after computing it!");
   return mDetailCollisionAccelerators[dl];
}


void TSShape::computeAccelerator(S32 dl)
{
   AssertFatal(dl < mDetails.size(), "Error, bad detail level!");

   // Have we already computed this?
   if (mDetailCollisionAccelerators[dl] != NULL)
      return;

   // Create a bogus features list...
   ConvexFeature cf;
   MatrixF mat(true);
   Point3F n(0, 0, 1);

   const TSDetail* detail = &mDetails[dl];
   S32 ss = detail->subShapeNum;
   S32 od = detail->objectDetailNum;

   S32 start = mSubShapeFirstObject[ss];
   S32 end   = mSubShapeNumObjects[ss] + start;
   if (start < end)
   {
      // run through objects and collide
      // DMMNOTE: This assumes that the transform of the collision hulls is
      //  identity...
      U32 surfaceKey = 0;
      for (S32 i = start; i < end; i++)
      {
         const TSObject* obj = &mObjects[i];

         if (obj->numMeshes && od < obj->numMeshes) {
            TSMesh* mesh = mMeshes[obj->startMeshIndex + od];
            if (mesh)
               mesh->getFeatures(0, mat, n, &cf, surfaceKey);
         }
      }
   }

   Vector<Point3F> fixedVerts;
   VECTOR_SET_ASSOCIATION(fixedVerts);
   S32 i;
   for (i = 0; i < cf.mVertexList.size(); i++) {
      S32 j;
      bool found = false;
      for (j = 0; j < cf.mFaceList.size(); j++) {
         if (cf.mFaceList[j].vertex[0] == i ||
             cf.mFaceList[j].vertex[1] == i ||
             cf.mFaceList[j].vertex[2] == i) {
            found = true;
            break;
         }
      }
      if (!found)
         continue;

      found = false;
      for (j = 0; j < fixedVerts.size(); j++) {
         if (fixedVerts[j] == cf.mVertexList[i]) {
            found = true;
            break;
         }
      }
      if (found == true) {
         // Ok, need to replace any references to vertex i in the facelists with
         //  a reference to vertex j in the fixed list
         for (S32 k = 0; k < cf.mFaceList.size(); k++) {
            for (S32 l = 0; l < 3; l++) {
               if (cf.mFaceList[k].vertex[l] == i)
                  cf.mFaceList[k].vertex[l] = j;
            }
         }
      } else {
         for (S32 k = 0; k < cf.mFaceList.size(); k++) {
            for (S32 l = 0; l < 3; l++) {
               if (cf.mFaceList[k].vertex[l] == i)
                  cf.mFaceList[k].vertex[l] = fixedVerts.size();
            }
         }
         fixedVerts.push_back(cf.mVertexList[i]);
      }
   }
   cf.mVertexList.setSize(0);
   cf.mVertexList = fixedVerts;

   // Ok, so now we have a vertex list.  Lets copy that out...
   ConvexHullAccelerator* accel = new ConvexHullAccelerator;
   mDetailCollisionAccelerators[dl] = accel;
   accel->numVerts    = cf.mVertexList.size();
   accel->vertexList  = new Point3F[accel->numVerts];
   dMemcpy(accel->vertexList, cf.mVertexList.address(), sizeof(Point3F) * accel->numVerts);

   accel->normalList = new Point3F[cf.mFaceList.size()];
   for (i = 0; i < cf.mFaceList.size(); i++)
      accel->normalList[i] = cf.mFaceList[i].normal;

   accel->emitStrings = new U8*[accel->numVerts];
   dMemset(accel->emitStrings, 0, sizeof(U8*) * accel->numVerts);

   for (i = 0; i < accel->numVerts; i++) {
      S32 j;

      Vector<U32> faces;
      VECTOR_SET_ASSOCIATION(faces);
      for (j = 0; j < cf.mFaceList.size(); j++) {
         if (cf.mFaceList[j].vertex[0] == i ||
             cf.mFaceList[j].vertex[1] == i ||
             cf.mFaceList[j].vertex[2] == i) {
            faces.push_back(j);
         }
      }
      AssertFatal(faces.size() != 0, "Huh?  Vertex unreferenced by any faces");

      // Insert all faces that didn't make the first cut, but share a plane with
      //  a face that's on the short list.
      for (j = 0; j < cf.mFaceList.size(); j++) {
         bool found = false;
         S32 k;
         for (k = 0; k < faces.size(); k++) {
            if (faces[k] == j)
               found = true;
         }
         if (found)
            continue;

         found = false;
         for (k = 0; k < faces.size(); k++) {
            if (mDot(accel->normalList[faces[k]], accel->normalList[j]) > 0.999) {
               found = true;
               break;
            }
         }
         if (found)
            faces.push_back(j);
      }

      Vector<U32> vertRemaps;
      VECTOR_SET_ASSOCIATION(vertRemaps);
      for (j = 0; j < faces.size(); j++) {
         for (U32 k = 0; k < 3; k++) {
            U32 insert = cf.mFaceList[faces[j]].vertex[k];
            bool found = false;
            for (S32 l = 0; l < vertRemaps.size(); l++) {
               if (insert == vertRemaps[l]) {
                  found = true;
                  break;
               }
            }
            if (!found)
               vertRemaps.push_back(insert);
         }
      }

      Vector<Point2I> edges;
      VECTOR_SET_ASSOCIATION(edges);
      for (j = 0; j < faces.size(); j++) {
         for (U32 k = 0; k < 3; k++) {
            U32 edgeStart = cf.mFaceList[faces[j]].vertex[(k + 0) % 3];
            U32 edgeEnd   = cf.mFaceList[faces[j]].vertex[(k + 1) % 3];

            U32 e0 = getMin(edgeStart, edgeEnd);
            U32 e1 = getMax(edgeStart, edgeEnd);

            bool found = false;
            for (S32 l = 0; l < edges.size(); l++) {
               if (edges[l].x == e0 && edges[l].y == e1) {
                  found = true;
                  break;
               }
            }
            if (!found)
               edges.push_back(Point2I(e0, e1));
         }
      }

      //AssertFatal(vertRemaps.size() < 256 && faces.size() < 256 && edges.size() < 256,
      //            "Error, ran over the shapebase assumptions about convex hulls.");

      U32 emitStringLen = 1 + vertRemaps.size()  +
                          1 + (edges.size() * 2) +
                          1 + (faces.size() * 4);
      accel->emitStrings[i] = new U8[emitStringLen];

      U32 currPos = 0;

      accel->emitStrings[i][currPos++] = vertRemaps.size();
      for (j = 0; j < vertRemaps.size(); j++)
         accel->emitStrings[i][currPos++] = vertRemaps[j];

      accel->emitStrings[i][currPos++] = edges.size();
      for (j = 0; j < edges.size(); j++) {
         S32 l;
         U32 old = edges[j].x;
         bool found = false;
         for (l = 0; l < vertRemaps.size(); l++) {
            if (vertRemaps[l] == old) {
               found = true;
               accel->emitStrings[i][currPos++] = l;
               break;
            }
         }
         AssertFatal(found, "Error, couldn't find the remap!");

         old = edges[j].y;
         found = false;
         for (l = 0; l < vertRemaps.size(); l++) {
            if (vertRemaps[l] == old) {
               found = true;
               accel->emitStrings[i][currPos++] = l;
               break;
            }
         }
         AssertFatal(found, "Error, couldn't find the remap!");
      }

      accel->emitStrings[i][currPos++] = faces.size();
      for (j = 0; j < faces.size(); j++) {
         accel->emitStrings[i][currPos++] = faces[j];
         for (U32 k = 0; k < 3; k++) {
            U32 old = cf.mFaceList[faces[j]].vertex[k];
            bool found = false;
            for (S32 l = 0; l < vertRemaps.size(); l++) {
               if (vertRemaps[l] == old) {
                  found = true;
                  accel->emitStrings[i][currPos++] = l;
                  break;
               }
            }
            AssertFatal(found, "Error, couldn't find the remap!");
         }
      }
      AssertFatal(currPos == emitStringLen, "Error, over/underflowed the emission string!");
   }
}
