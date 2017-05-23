//-----------------------------------------------------------------------------
// Copyright (c) 2014 GarageGames, LLC
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

#include "console/consoleTypes.h"
#include "console/typeValidators.h"
#include "core/stream/bitStream.h"
#include "T3D/shapeBase.h"
#include "ts/tsShapeInstance.h"
#include "T3D/fx/ribbon.h"
#include "math/mathUtils.h"
#include "math/mathIO.h"
#include "sim/netConnection.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxDrawUtil.h"
#include "materials/sceneData.h"
#include "materials/matInstance.h"
#include "gui/3d/guiTSControl.h"
#include "materials/materialManager.h"
#include "materials/processedShaderMaterial.h"
#include "gfx/gfxTransformSaver.h"


IMPLEMENT_CO_DATABLOCK_V1(RibbonData);
IMPLEMENT_CO_NETOBJECT_V1(Ribbon);


//--------------------------------------------------------------------------
//
RibbonData::RibbonData()
{
   for (U8 i = 0; i < NumFields; i++) {
      mSizes[i] = 0.0f;
      mColours[i].set(0.0f, 0.0f, 0.0f, 1.0f);
      mTimes[i] = -1.0f;
   }

   mRibbonLength = 0;
   mUseFadeOut = false;
   mFadeAwayStep = 0.032f;
   segmentsPerUpdate = 1;
   mMatName = StringTable->EmptyString();
   mTileScale = 1.0f;
   mFixedTexcoords = false;
   mSegmentSkipAmount = 0;
   mTexcoordsRelativeToDistance = false;
}

//--------------------------------------------------------------------------

void RibbonData::initPersistFields()
{
   Parent::initPersistFields();

   addGroup("Ribbon");

   addField("size", TypeF32, Offset(mSizes, RibbonData), NumFields,
      "The size of the ribbon at the specified keyframe.");
   addField("color", TypeColorF, Offset(mColours, RibbonData), NumFields,
      "The colour of the ribbon at the specified keyframe.");
   addField("position", TypeF32, Offset(mTimes, RibbonData), NumFields,
      "The position of the keyframe along the lifetime of the ribbon.");

   addField("ribbonLength", TypeS32, Offset(mRibbonLength, RibbonData),
      "The amount of segments the Ribbon can maximally have in length.");
   addField("segmentsPerUpdate", TypeS32, Offset(segmentsPerUpdate, RibbonData),
      "How many segments to add each update.");
   addField("skipAmount", TypeS32, Offset(mSegmentSkipAmount, RibbonData),
      "The amount of segments to skip each update.");

   addField("useFadeOut", TypeBool, Offset(mUseFadeOut, RibbonData),
      "If true, the ribbon will fade away after deletion.");
   addField("fadeAwayStep", TypeF32, Offset(mFadeAwayStep, RibbonData),
      "How much to fade the ribbon with each update, after deletion.");
   addField("ribbonMaterial", TypeString, Offset(mMatName, RibbonData),
      "The material the ribbon uses for rendering.");
   addField("tileScale", TypeF32, Offset(mTileScale, RibbonData),
      "How much to scale each 'tile' with, where 1 means the material is stretched"
      "across the whole ribbon. (If TexcoordsRelativeToDistance is true, this is in meters.)");
   addField("fixedTexcoords", TypeBool, Offset(mFixedTexcoords, RibbonData),
      "If true, this prevents 'floating' texture coordinates.");
   addField("texcoordsRelativeToDistance", TypeBool, Offset(mTexcoordsRelativeToDistance, RibbonData),
      "If true, texture coordinates are scaled relative to distance, this prevents"
      "'stretched' textures.");

   endGroup("Ribbon");
}


//--------------------------------------------------------------------------
bool RibbonData::onAdd()
{
   if(!Parent::onAdd())
      return false;

   return true;
}


bool RibbonData::preload(bool server, String &errorBuffer)
{
   if (Parent::preload(server, errorBuffer) == false)
      return false;

   return true;
}

//--------------------------------------------------------------------------
void RibbonData::packData(BitStream* stream)
{
   Parent::packData(stream);

   for (U8 i = 0; i < NumFields; i++) {
      stream->write(mSizes[i]);
      stream->write(mColours[i]);
      stream->write(mTimes[i]);
   }

   stream->write(mRibbonLength);
   stream->writeString(mMatName);
   stream->writeFlag(mUseFadeOut);
   stream->write(mFadeAwayStep);
   stream->write(segmentsPerUpdate);
   stream->write(mTileScale);
   stream->writeFlag(mFixedTexcoords);
   stream->writeFlag(mTexcoordsRelativeToDistance);
}

void RibbonData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   for (U8 i = 0; i < NumFields; i++) {
      stream->read(&mSizes[i]);
      stream->read(&mColours[i]);
      stream->read(&mTimes[i]);
   }

   stream->read(&mRibbonLength);
   mMatName = StringTable->insert(stream->readSTString());
   mUseFadeOut = stream->readFlag();
   stream->read(&mFadeAwayStep);
   stream->read(&segmentsPerUpdate);
   stream->read(&mTileScale);
   mFixedTexcoords = stream->readFlag();
   mTexcoordsRelativeToDistance = stream->readFlag();
}


//--------------------------------------------------------------------------
//--------------------------------------
//
Ribbon::Ribbon()
{
   mTypeMask |= StaticObjectType;

   VECTOR_SET_ASSOCIATION(mSegmentPoints);
   mSegmentPoints.clear();

   mRibbonMat = NULL;

   mUpdateBuffers = true;
   mDeleteOnEnd = false;
   mUseFadeOut = false;
   mFadeAwayStep = 1.0f;
   mFadeOut = 1.0f;

   mNetFlags.clear(Ghostable);
   mNetFlags.set(IsGhost);

   mRadiusSC = NULL;
   mRibbonProjSC = NULL;

   mSegmentOffset = 0;
   mSegmentIdx = 0;

   mTravelledDistance = 0;
}

Ribbon::~Ribbon()
{
   //Make sure we cleanup
   SAFE_DELETE(mRibbonMat);
}

//--------------------------------------------------------------------------
void Ribbon::initPersistFields()
{
   Parent::initPersistFields();
}

bool Ribbon::onAdd()
{
   if(!Parent::onAdd())
      return false;

   // add to client side mission cleanup
   SimGroup *cleanup = dynamic_cast<SimGroup *>( Sim::findObject( "ClientMissionCleanup") );
   if( cleanup != NULL )
   {
      cleanup->addObject( this );
   }
   else
   {
      AssertFatal( false, "Error, could not find ClientMissionCleanup group" );
      return false;
   }

   if (!isServerObject()) {

      if(GFX->getPixelShaderVersion() >= 1.1 && dStrlen(mDataBlock->mMatName) > 0 )
      {
         mRibbonMat = MATMGR->createMatInstance( mDataBlock->mMatName );
         GFXStateBlockDesc desc;
         desc.setZReadWrite( true, false );
         desc.cullDefined = true;
         desc.cullMode = GFXCullNone;
         desc.setBlend(true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);

         desc.samplersDefined = true;

         GFXSamplerStateDesc sDesc(GFXSamplerStateDesc::getClampLinear());
         sDesc.addressModeV = GFXAddressWrap;

         desc.samplers[0] = sDesc;

         mRibbonMat->addStateBlockDesc( desc );
         mRibbonMat->init(MATMGR->getDefaultFeatures(), getGFXVertexFormat<GFXVertexPCNTT>());

         mRadiusSC = mRibbonMat->getMaterialParameterHandle( "$radius" );
         mRibbonProjSC = mRibbonMat->getMaterialParameterHandle( "$ribbonProj" );

      } else {
         Con::warnf( "Invalid Material name: %s: for Ribbon", mDataBlock->mMatName );
#ifdef TORQUE_DEBUG
         Con::warnf( "- This could be caused by having the shader data datablocks in server-only code." );
#endif
         mRibbonMat = NULL;
      }
   }

   mObjBox.minExtents.set( 1.0f, 1.0f, 1.0f );
   mObjBox.maxExtents.set(  2.0f, 2.0f,  2.0f );
   // Reset the World Box.
   resetWorldBox();
   // Set the Render Transform.
   setRenderTransform(mObjToWorld);

   addToScene();

   return true;
}


void Ribbon::onRemove()
{

   removeFromScene();
   SAFE_DELETE(mRibbonMat);

   Parent::onRemove();
}


bool Ribbon::onNewDataBlock(GameBaseData* dptr, bool reload)
{
   mDataBlock = dynamic_cast<RibbonData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr, reload))
      return false;

   return true;
}

void Ribbon::processTick(const Move* move)
{
   Parent::processTick(move);

   if (mDeleteOnEnd) {

      if (mUseFadeOut) {

         if (mFadeOut <= 0.0f) {
            mFadeOut = 0.0f;
            //delete this class
            mDeleteOnEnd = false;
            safeDeleteObject();
            return;
         }
         mFadeOut -= mFadeAwayStep;
         if (mFadeOut < 0.0f) {
            mFadeOut = 0.0f;
         }

         mUpdateBuffers = true;

      } else {
         //if (mSegmentPoints.size() == 0) {
         //delete this class
         mDeleteOnEnd = false;
         safeDeleteObject();
         return;
         //}
         //mSegmentPoints.pop_back();  
      }


   }
}

void Ribbon::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);
}

void Ribbon::interpolateTick(F32 delta)
{
   Parent::interpolateTick(delta);
}

void Ribbon::addSegmentPoint(Point3F &point, MatrixF &mat) {

   //update our position
   setRenderTransform(mat);
   MatrixF xform(true);
   xform.setColumn(3, point);
   setTransform(xform);

   if(mSegmentIdx < mDataBlock->mSegmentSkipAmount)
   {
      mSegmentIdx++;
      return;
   }

   mSegmentIdx = 0;

   U32 segmentsToDelete = checkRibbonDistance(mDataBlock->segmentsPerUpdate);

   for (U32 i = 0; i < segmentsToDelete; i++) {
      S32 last = mSegmentPoints.size() - 1;
      if (last < 0)
         break;
      mTravelledDistance += last ? (mSegmentPoints[last] - mSegmentPoints[last-1]).len() : 0;
      mSegmentPoints.pop_back();
      mUpdateBuffers = true;
      mSegmentOffset++;
   }

   //If there is no other points, just add a new one.
   if (mSegmentPoints.size() == 0) {

      mSegmentPoints.push_front(point);
      mUpdateBuffers = true;
      return;
   }

   Point3F startPoint = mSegmentPoints[0];

   //add X points based on how many segments Per Update from last point to current point
   for (U32 i = 0; i < mDataBlock->segmentsPerUpdate; i++) {

      F32 interp = (F32(i+1) / (F32)mDataBlock->segmentsPerUpdate);
      //(end - start) * percentage) + start
      Point3F derivedPoint = ((point - startPoint) * interp) + startPoint;

      mSegmentPoints.push_front(derivedPoint);
      mUpdateBuffers = true;
   }

   if (mSegmentPoints.size() > 1) {

      Point3F pointA = mSegmentPoints[mSegmentPoints.size()-1];
      Point3F pointB = mSegmentPoints[0];

      Point3F diffSize = pointA - pointB;

      if (diffSize.x == 0.0f)
         diffSize.x = 1.0f;

      if (diffSize.y == 0.0f)
         diffSize.y = 1.0f;

      if (diffSize.z == 0.0f)
         diffSize.z = 1.0f;

      Box3F objBox;
      objBox.minExtents.set( diffSize * -1 );
      objBox.maxExtents.set( diffSize  );

      if (objBox.minExtents.x > objBox.maxExtents.x) {
         F32 tmp = objBox.minExtents.x;
         objBox.minExtents.x = objBox.maxExtents.x;
         objBox.maxExtents.x = tmp;
      }
      if (objBox.minExtents.y > objBox.maxExtents.y) {
         F32 tmp = objBox.minExtents.y;
         objBox.minExtents.y = objBox.maxExtents.y;
         objBox.maxExtents.y = tmp;
      }
      if (objBox.minExtents.z > objBox.maxExtents.z) {
         F32 tmp = objBox.minExtents.z;
         objBox.minExtents.z = objBox.maxExtents.z;
         objBox.maxExtents.z = tmp;
      }



      if (objBox.isValidBox()) {
         mObjBox = objBox;
         // Reset the World Box.
         resetWorldBox();
      }
   }

}

void Ribbon::deleteOnEnd() {

   mDeleteOnEnd = true;
   mUseFadeOut = mDataBlock->mUseFadeOut;
   mFadeAwayStep = mDataBlock->mFadeAwayStep;

}

U32 Ribbon::checkRibbonDistance(S32 segments) {

   S32 len = mSegmentPoints.size();
   S32 difference = (mDataBlock->mRibbonLength/(mDataBlock->mSegmentSkipAmount+1)) - len;

   if (difference < 0)
      return mAbs(difference);

   return 0;  //do not delete any points
}

void Ribbon::setShaderParams() {

   F32 numSegments = (F32)mSegmentPoints.size();
   F32 length = (F32)mDataBlock->mRibbonLength;
   Point3F radius(numSegments / length, numSegments, length);
   MaterialParameters* matParams = mRibbonMat->getMaterialParameters();
   matParams->setSafe( mRadiusSC, radius );  
}

//--------------------------------------------------------------------------
//U32 Ribbon::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
//{
//   U32 retMask = Parent::packUpdate(con, mask, stream);
//   return retMask;
//}
//
//void Ribbon::unpackUpdate(NetConnection* con, BitStream* stream)
//{
//   Parent::unpackUpdate(con, stream);
//}

//--------------------------------------------------------------------------
void Ribbon::prepRenderImage(SceneRenderState *state)
{
   if (mFadeOut == 0.0f)
      return;

   if(!mRibbonMat)
      return;

   if (mDeleteOnEnd == true && mUseFadeOut == false) {
      return;
   }

   // We only render during the normal diffuse render pass.
   if( !state->isDiffusePass() )
      return;

   U32 segments = mSegmentPoints.size();
   if (segments < 2)
      return;

   MeshRenderInst *ri = state->getRenderPass()->allocInst<MeshRenderInst>();
   ri->type = RenderPassManager::RIT_Translucent;
   ri->translucentSort = true;
   ri->sortDistSq = ( mSegmentPoints[0] - state->getCameraPosition() ).lenSquared();

   RenderPassManager *renderPass = state->getRenderPass();
   MatrixF *proj = renderPass->allocUniqueXform(MatrixF( true ));
   proj->mul(GFX->getProjectionMatrix());
   proj->mul(GFX->getWorldMatrix());
   ri->objectToWorld = &MatrixF::Identity;
   ri->worldToCamera = &MatrixF::Identity;
   ri->projection    = proj;
   ri->matInst = mRibbonMat;

   // Set up our vertex buffer and primitive buffer
   if(mUpdateBuffers)
      createBuffers(state, verts, primBuffer, segments);

   ri->vertBuff = &verts;
   ri->primBuff = &primBuffer;
   ri->visibility = 1.0f;

   ri->prim = renderPass->allocPrim();
   ri->prim->type = GFXTriangleList;
   ri->prim->minIndex = 0;
   ri->prim->startIndex = 0;
   ri->prim->numPrimitives = (segments-1) * 2;
   ri->prim->startVertex = 0;
   ri->prim->numVertices = segments * 2;

   if (mRibbonMat) {
      ri->defaultKey = mRibbonMat->getStateHint();
   } else {
      ri->defaultKey = 1;
   }
   ri->defaultKey2 = (uintptr_t)ri->vertBuff; // Not 64bit safe!

   state->getRenderPass()->addInst(ri);
}

void Ribbon::createBuffers(SceneRenderState *state, GFXVertexBufferHandle<GFXVertexPCNTT> &verts, GFXPrimitiveBufferHandle &pb, U32 segments) {
   PROFILE_SCOPE( Ribbon_createBuffers );
   Point3F cameraPos = state->getCameraPosition();
   U32 count = 0;
   U32 indexCount = 0;
   verts.set(GFX, (segments*2), GFXBufferTypeDynamic);

   // create index buffer based on that size
   U32 indexListSize = (segments-1) * 6;
   pb.set( GFX, indexListSize, 0, GFXBufferTypeDynamic );
   U16 *indices = NULL;

   verts.lock();
   pb.lock( &indices );
   F32 totalLength = 0;
   if(mDataBlock->mTexcoordsRelativeToDistance)
   {
      for (U32 i = 0; i < segments; i++)
         if (i != 0)
            totalLength += (mSegmentPoints[i] - mSegmentPoints[i-1]).len();
   }

   U8 fixedAppend = 0;
   F32 curLength = 0;
   for (U32 i = 0; i < segments; i++) {

      F32 interpol = ((F32)i / (F32)(segments-1));
      Point3F leftvert = mSegmentPoints[i];
      Point3F rightvert = mSegmentPoints[i];
      F32 tRadius = mDataBlock->mSizes[0];
      ColorF tColor = mDataBlock->mColours[0];

      for (U8 j = 0; j < RibbonData::NumFields-1; j++) {

         F32 curPosition = mDataBlock->mTimes[j];
         F32 curRadius = mDataBlock->mSizes[j];
         ColorF curColor = mDataBlock->mColours[j];
         F32 nextPosition = mDataBlock->mTimes[j+1];
         F32 nextRadius = mDataBlock->mSizes[j+1];
         ColorF nextColor = mDataBlock->mColours[j+1];

         if (  curPosition < 0
            || curPosition > interpol )
            break;
         F32 positionDiff = (interpol - curPosition) / (nextPosition - curPosition);

         tRadius = curRadius + (nextRadius - curRadius) * positionDiff;
         tColor.interpolate(curColor, nextColor, positionDiff);
      }

      Point3F diff;
      F32 length;
      if (i == 0) {
         diff = mSegmentPoints[i+1] - mSegmentPoints[i];
         length = 0;
      } else if (i == segments-1) {
         diff = mSegmentPoints[i] - mSegmentPoints[i-1];
         length = diff.len();
      } else {
         diff = mSegmentPoints[i+1] - mSegmentPoints[i-1];
         length = (mSegmentPoints[i] - mSegmentPoints[i-1]).len();
      }

      //left point
      Point3F eyeMinPos = cameraPos - leftvert;
      Point3F perpendicular = mCross(diff, eyeMinPos);
      perpendicular.normalize();
      perpendicular = perpendicular * tRadius * -1.0f;
      perpendicular += mSegmentPoints[i];

      verts[count].point.set(perpendicular);
      ColorF color = tColor;

      if (mDataBlock->mUseFadeOut)
         color.alpha *= mFadeOut;

      F32 texCoords;
      if(mDataBlock->mFixedTexcoords && !mDataBlock->mTexcoordsRelativeToDistance)
      {
         U32 fixedIdx = (i+mDataBlock->mRibbonLength-mSegmentOffset)%mDataBlock->mRibbonLength;
         if(fixedIdx == 0 && i > 0)
            fixedAppend++;
         F32 fixedInterpol = (F32)fixedIdx / (F32)(mDataBlock->mRibbonLength);
         fixedInterpol += fixedAppend;
         texCoords = (1.0f - fixedInterpol)*mDataBlock->mTileScale;
      }
      else if(mDataBlock->mTexcoordsRelativeToDistance)
         texCoords = (mTravelledDistance + (totalLength - (curLength + length)))*mDataBlock->mTileScale;
      else
         texCoords = (1.0f - interpol)*mDataBlock->mTileScale;

      verts[count].color = color;
      verts[count].texCoord[1] = Point2F(interpol, 0);
      verts[count].texCoord[0] = Point2F(0.0f, texCoords);
      verts[count].normal.set(diff);

      //Triangle strip style indexing, so grab last 2
      if (count > 1) {
         indices[indexCount] = count-2;
         indexCount++;
         indices[indexCount] = count;
         indexCount++;
         indices[indexCount] = count-1;
         indexCount++;
      }
      count++;

      eyeMinPos = cameraPos - rightvert;
      perpendicular = mCross(diff, eyeMinPos);
      perpendicular.normalize();
      perpendicular = perpendicular * tRadius;
      perpendicular += mSegmentPoints[i];

      verts[count].point.set(perpendicular);
      color = tColor;

      if (mDataBlock->mUseFadeOut)
         color.alpha *= mFadeOut;

      verts[count].color = color;
      verts[count].texCoord[1] = Point2F(interpol, 1);
      verts[count].texCoord[0] = Point2F(1.0f, texCoords);
      verts[count].normal.set(diff);

      //Triangle strip style indexing, so grab last 2
      if (count > 1) {
         indices[indexCount] = count-2;
         indexCount++;
         indices[indexCount] = count-1;
         indexCount++;
         indices[indexCount] = count;
         indexCount++;
      }
      count++;
      curLength += length;
   }

   Point3F pointA = verts[count-1].point;
   Point3F pointB = verts[0].point;

   pb.unlock();
   verts.unlock();
 
   Point3F diffSize = pointA - pointB;

   Box3F objBox;
   objBox.minExtents.set( diffSize * -1 );
   objBox.maxExtents.set( diffSize  );

   if (objBox.minExtents.x > objBox.maxExtents.x) {
      F32 tmp = objBox.minExtents.x;
      objBox.minExtents.x = objBox.maxExtents.x;
      objBox.maxExtents.x = tmp;
   }
   if (objBox.minExtents.y > objBox.maxExtents.y) {
      F32 tmp = objBox.minExtents.y;
      objBox.minExtents.y = objBox.maxExtents.y;
      objBox.maxExtents.y = tmp;
   }
   if (objBox.minExtents.z > objBox.maxExtents.z) {
      F32 tmp = objBox.minExtents.z;
      objBox.minExtents.z = objBox.maxExtents.z;
      objBox.maxExtents.z = tmp;
   }

   if (objBox.isValidBox()) {
      mObjBox = objBox;
      // Reset the World Box.
      resetWorldBox();
   }

   mUpdateBuffers = false;
}
