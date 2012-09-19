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

#include "interior/pathedInterior.h"
#include "core/stream/stream.h"
#include "console/consoleTypes.h"
#include "scene/sceneRenderState.h"
#include "math/mathIO.h"
#include "core/stream/bitStream.h"
#include "interior/interior.h"
#include "scene/simPath.h"
#include "scene/pathManager.h"
#include "core/frameAllocator.h"
#include "scene/sceneManager.h"
#include "sfx/sfxSystem.h"
#include "sfx/sfxProfile.h"
#include "sfx/sfxSource.h"
#include "core/resourceManager.h"

IMPLEMENT_CO_NETOBJECT_V1(PathedInterior);
IMPLEMENT_CO_DATABLOCK_V1(PathedInteriorData);

ConsoleDocClass( PathedInterior,
				"@brief Legacy interior related class, soon to be deprecated.\n\n"
				"Not intended for game development, for editors or internal use only.\n\n "
				"@internal");

ConsoleDocClass( PathedInteriorData,
				"@brief Legacy interior related class, soon to be deprecated.\n\n"
				"Not intended for game development, for editors or internal use only.\n\n "
				"@internal");

//--------------------------------------------------------------------------

PathedInteriorData::PathedInteriorData()
{
   for(U32 i = 0; i < MaxSounds; i++)
      sound[i] = NULL;
}

void PathedInteriorData::initPersistFields()
{
   addField("StartSound", TYPEID< SFXProfile >(), Offset(sound[StartSound], PathedInteriorData));
   addField("SustainSound", TYPEID< SFXProfile >(), Offset(sound[SustainSound], PathedInteriorData));
   addField("StopSound", TYPEID< SFXProfile >(), Offset(sound[StopSound], PathedInteriorData));

   Parent::initPersistFields();
}

void PathedInteriorData::packData(BitStream *stream)
{
   for (S32 i = 0; i < MaxSounds; i++)
   {
      if (stream->writeFlag(sound[i]))
         stream->writeRangedU32(packed? SimObjectId(sound[i]):
                                sound[i]->getId(),DataBlockObjectIdFirst,DataBlockObjectIdLast);
   }
   Parent::packData(stream);
}

void PathedInteriorData::unpackData(BitStream* stream)
{
   for (S32 i = 0; i < MaxSounds; i++) {
      sound[i] = NULL;
      if (stream->readFlag())
         sound[i] = (SFXProfile*)stream->readRangedU32(DataBlockObjectIdFirst,
                                                         DataBlockObjectIdLast);
   }
   Parent::unpackData(stream);
}

bool PathedInteriorData::preload(bool server, String &errorStr)
{
   if(!Parent::preload(server, errorStr))
      return false;
      
   // Resolve objects transmitted from server
   if (!server)
   {
      for (S32 i = 0; i < MaxSounds; i++)
         if (sound[i])
            Sim::findObject(SimObjectId(sound[i]),sound[i]);
   }
   return true;
}

PathedInterior::PathedInterior()
{
   mNetFlags.set(Ghostable);
   mTypeMask = InteriorObjectType;

   mCurrentPosition = 0;
   mTargetPosition = 0;
   mPathKey = 0xFFFFFFFF;
   mStopped = false;

   mSustainSound = NULL;
}

PathedInterior::~PathedInterior()
{
   //
}

PathedInterior *PathedInterior::mClientPathedInteriors = NULL;
//--------------------------------------------------------------------------
void PathedInterior::initPersistFields()
{
   addField("interiorResource", TypeFilename,       Offset(mInteriorResName,  PathedInterior));
   addField("interiorIndex",    TypeS32,            Offset(mInteriorResIndex, PathedInterior));
   addField("basePosition",     TypeMatrixPosition, Offset(mBaseTransform,    PathedInterior));
   addField("baseRotation",     TypeMatrixRotation, Offset(mBaseTransform,    PathedInterior));
   addField("baseScale",        TypePoint3F,        Offset(mBaseScale,        PathedInterior));

   Parent::initPersistFields();
}


//--------------------------------------------------------------------------
bool PathedInterior::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   // Load the interior resource and extract the interior that is us.
   mInteriorRes = ResourceManager::get().load(mInteriorResName);
   if (bool(mInteriorRes) == false)
      return false;
   mInterior = mInteriorRes->getSubObject(mInteriorResIndex);
   if (mInterior == NULL)
      return false;

   // Setup bounding information
   mObjBox = mInterior->getBoundingBox();
   resetWorldBox();

   setScale(mBaseScale);
   setTransform(mBaseTransform);

   if (isClientObject()) {
      mNextClientPI = mClientPathedInteriors;
      mClientPathedInteriors = this;
      mInterior->prepForRendering(mInteriorRes.getPath().getFullPath().c_str());
//      gInteriorLMManager.addInstance(mInterior->getLMHandle(), mLMHandle, NULL, this);
   }

   if(isClientObject())
   {
      Point3F initialPos( 0.0, 0.0, 0.0 );
      mBaseTransform.getColumn(3, &initialPos);
      Point3F pathPos( 0.0, 0.0, 0.0 );
      //gClientPathManager->getPathPosition(mPathKey, 0, pathPos);
      mOffset = initialPos - pathPos;
      //gClientPathManager->getPathPosition(mPathKey, mCurrentPosition, pathPos);
      MatrixF mat = getTransform();
      mat.setColumn(3, pathPos + mOffset);
      setTransform(mat);
   }

   addToScene();

   return true;
}

bool PathedInterior::onNewDataBlock( GameBaseData *dptr, bool reload )
{
   mDataBlock = dynamic_cast<PathedInteriorData*>(dptr);

   if ( isClientObject() )
   {
      SFX_DELETE( mSustainSound );

      if ( mDataBlock->sound[PathedInteriorData::SustainSound] )
         mSustainSound = SFX->createSource( mDataBlock->sound[PathedInteriorData::SustainSound], &getTransform() );

      if ( mSustainSound )
         mSustainSound->play();
   }

   return Parent::onNewDataBlock(dptr,reload);
}

void PathedInterior::onRemove()
{
   if(isClientObject())
   {
      SFX_DELETE( mSustainSound );

      PathedInterior **walk = &mClientPathedInteriors;
      while(*walk)
      {
         if(*walk == this)
         {
            *walk = mNextClientPI;
            break;
         }
         walk = &((*walk)->mNextClientPI);
      }
/*      if(bool(mInteriorRes) && mLMHandle != 0xFFFFFFFF)
      {
         if (mInterior->getLMHandle() != 0xFFFFFFFF)
            gInteriorLMManager.removeInstance(mInterior->getLMHandle(), mLMHandle);
      }*/
   }
   removeFromScene();
   Parent::onRemove();
}

//------------------------------------------------------------------------------
bool PathedInterior::buildPolyList(AbstractPolyList* list, const Box3F& wsBox, const SphereF&)
{
   if (bool(mInteriorRes) == false)
      return false;

   // Setup collision state data
   list->setTransform(&getTransform(), getScale());
   list->setObject(this);

   return mInterior->buildPolyList(list, wsBox, mWorldToObj, getScale());
}


//--------------------------------------------------------------------------
void PathedInterior::prepRenderImage( SceneRenderState* state )
{
   if (mPathKey == SimPath::Path::NoPathIndex)
      return;

/*
      SceneRenderImage* image = new SceneRenderImage;
      image->obj = this;
      state->insertRenderImage(image);
*/
}

extern ColorF gInteriorFogColor;

void PathedInterior::renderObject(SceneRenderState* state)
{
}

void PathedInterior::resolvePathKey()
{
   if(mPathKey == 0xFFFFFFFF && !isGhost())
   {
      mPathKey = getPathKey();
      Point3F pathPos( 0.0, 0.0, 0.0 );
      Point3F initialPos( 0.0, 0.0, 0.0 );
      mBaseTransform.getColumn(3, &initialPos);
      //gServerPathManager->getPathPosition(mPathKey, 0, pathPos);
      mOffset = initialPos - pathPos;
   }
}


//--------------------------------------------------------------------------
U32 PathedInterior::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);
   resolvePathKey();

   if (stream->writeFlag(mask & InitialUpdateMask))
   {
      // Inital update...
      stream->writeString(mInteriorResName);
      stream->write(mInteriorResIndex);

      stream->writeAffineTransform(mBaseTransform);
      mathWrite(*stream, mBaseScale);

      stream->write(mPathKey);
   }
   if(stream->writeFlag((mask & NewPositionMask) && mPathKey != SimPath::Path::NoPathIndex))
      stream->writeInt(S32(mCurrentPosition), gServerPathManager->getPathTimeBits(mPathKey));
   if(stream->writeFlag((mask & NewTargetMask) && mPathKey != SimPath::Path::NoPathIndex))
   {
      if(stream->writeFlag(mTargetPosition < 0))
      {
         stream->writeFlag(mTargetPosition == -1);
      }
      else
         stream->writeInt(S32(mTargetPosition), gServerPathManager->getPathTimeBits(mPathKey));
   }
   return retMask;
}

void PathedInterior::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   MatrixF tempXForm;
   Point3F tempScale;

   if (stream->readFlag())
   {
      // Initial
      mInteriorResName = stream->readSTString();
      stream->read(&mInteriorResIndex);

      stream->readAffineTransform(&tempXForm);
      mathRead(*stream, &tempScale);
      mBaseTransform = tempXForm;
      mBaseScale     = tempScale;

      stream->read(&mPathKey);
   }
   if(stream->readFlag())
   {
      Point3F pathPos(0.0f, 0.0f, 0.0f);
      mCurrentPosition = stream->readInt(gClientPathManager->getPathTimeBits(mPathKey));
      if(isProperlyAdded())
      {
         //gClientPathManager->getPathPosition(mPathKey, mCurrentPosition, pathPos);
         MatrixF mat = getTransform();
         mat.setColumn(3, pathPos + mOffset);
         setTransform(mat);
      }
   }
   if(stream->readFlag())
   {
      if(stream->readFlag())
      {
         mTargetPosition = stream->readFlag() ? -1 : -2;
      }
      else
         mTargetPosition = stream->readInt(gClientPathManager->getPathTimeBits(mPathKey));
   }
}

void PathedInterior::processTick(const Move* move)
{
   if(isServerObject())
   {
      S32 timeMs = 32;   
      if(mCurrentPosition != mTargetPosition)
      {
         S32 delta;
         if(mTargetPosition == -1)
            delta = timeMs;
         else if(mTargetPosition == -2)
            delta = -timeMs;
         else
         {
            delta = mTargetPosition - (S32)mCurrentPosition;
            if(delta < -timeMs)
               delta = -timeMs;
            else if(delta > timeMs)
               delta = timeMs;
         }
         mCurrentPosition += delta;
         U32 totalTime = gClientPathManager->getPathTotalTime(mPathKey);
         while(mCurrentPosition > totalTime)
            mCurrentPosition -= totalTime;
         while(mCurrentPosition < 0)
            mCurrentPosition += totalTime;
      }
   }
}

void PathedInterior::computeNextPathStep(U32 timeDelta)
{
   S32 timeMs = timeDelta;
   mStopped = false;

   if(mCurrentPosition == mTargetPosition)
   {
      mExtrudedBox = getWorldBox();
      mCurrentVelocity.set(0,0,0);
   }
   else
   {
      S32 delta = 0;
      if(mTargetPosition < 0)
      {
         if(mTargetPosition == -1)
            delta = timeMs;
         else if(mTargetPosition == -2)
            delta = -timeMs;
         
         mCurrentPosition += delta;

         U32 totalTime = gClientPathManager->getPathTotalTime(mPathKey);
         
         while(mCurrentPosition >= totalTime)
            mCurrentPosition -= totalTime;
         
         while(mCurrentPosition < 0)
            mCurrentPosition += totalTime;
      }
      else
      {
         delta = mTargetPosition - (S32)mCurrentPosition;
         if(delta < -timeMs)
            delta = -timeMs;
         else if(delta > timeMs)
            delta = timeMs;
         mCurrentPosition += delta;
      }

      Point3F curPoint;
      Point3F newPoint( 0.0, 0.0, 0.0 );
      MatrixF mat = getTransform();
      mat.getColumn(3, &curPoint);
      //gClientPathManager->getPathPosition(mPathKey, mCurrentPosition, newPoint);
      newPoint += mOffset;

      Point3F displaceDelta = newPoint - curPoint;
      mExtrudedBox = getWorldBox();

      if(displaceDelta.x < 0)
         mExtrudedBox.minExtents.x += displaceDelta.x;
      else
         mExtrudedBox.maxExtents.x += displaceDelta.x;
      if(displaceDelta.y < 0)
         mExtrudedBox.minExtents.y += displaceDelta.y;
      else
         mExtrudedBox.maxExtents.y += displaceDelta.y;
      if(displaceDelta.z < 0)
         mExtrudedBox.minExtents.z += displaceDelta.z;
      else
         mExtrudedBox.maxExtents.z += displaceDelta.z;

      mCurrentVelocity = displaceDelta * 1000 / F32(timeDelta);
   }
   Point3F pos;
   mExtrudedBox.getCenter(&pos);
   MatrixF mat = getTransform();
   mat.setColumn(3, pos);

   if ( mSustainSound )
   {
      mSustainSound->setTransform( mat );
      mSustainSound->setVelocity( getVelocity() );
   }
}

Point3F PathedInterior::getVelocity()
{
   return mCurrentVelocity;
}

void PathedInterior::advance(F64 timeDelta)
{
   if(mStopped)
      return;

   if(mCurrentVelocity.len() == 0)
   {
//      if(mSustainHandle)
//      {
//         alxStop(mSustainHandle);
//         mSustainHandle = 0;
//      }
      return;
   }
   MatrixF mat = getTransform();
   Point3F newPoint;
   mat.getColumn(3, &newPoint);
   newPoint += mCurrentVelocity * timeDelta / 1000.0f;
   //gClientPathManager->getPathPosition(mPathKey, mCurrentPosition, newPoint);
   mat.setColumn(3, newPoint);// + mOffset);
   setTransform(mat);
   setRenderTransform(mat);
}

U32 PathedInterior::getPathKey()
{
   AssertFatal(isServerObject(), "Error, must be a server object to call this...");

   SimGroup* myGroup = getGroup();
   AssertFatal(myGroup != NULL, "No group for this object?");

   for (SimGroup::iterator itr = myGroup->begin(); itr != myGroup->end(); itr++) {
      SimPath::Path* pPath = dynamic_cast<SimPath::Path*>(*itr);
      if (pPath != NULL) {
         U32 pathKey = pPath->getPathIndex();
         AssertFatal(pathKey != SimPath::Path::NoPathIndex, "Error, path must have event over at this point...");
         return pathKey;
      }
   }

   return SimPath::Path::NoPathIndex;
}

void PathedInterior::setPathPosition(S32 newPosition)
{
   resolvePathKey();
   if(newPosition < 0)
      newPosition = 0;
   if(newPosition > S32(gServerPathManager->getPathTotalTime(mPathKey)))
      newPosition = S32(gServerPathManager->getPathTotalTime(mPathKey));
   mCurrentPosition = mTargetPosition = newPosition;
   setMaskBits(NewPositionMask | NewTargetMask);
}

void PathedInterior::setTargetPosition(S32 newPosition)
{
   resolvePathKey();
   if(newPosition < -2)
      newPosition = 0;
   if(newPosition > S32(gServerPathManager->getPathTotalTime(mPathKey)))
      newPosition = gServerPathManager->getPathTotalTime(mPathKey);
   if(mTargetPosition != newPosition)
   {
      mTargetPosition = newPosition;
      setMaskBits(NewTargetMask);
   }
}

ConsoleMethod(PathedInterior, setPathPosition, void, 3, 3, "")
{
   ((PathedInterior *) object)->setPathPosition(dAtoi(argv[2]));
}

ConsoleMethod(PathedInterior, setTargetPosition, void, 3, 3, "")
{
   ((PathedInterior *) object)->setTargetPosition(dAtoi(argv[2]));
}


//--------------------------------------------------------------------------
bool PathedInterior::readPI(Stream& stream)
{
   mName            = stream.readSTString();
   mInteriorResName = stream.readSTString();
   stream.read(&mInteriorResIndex);
   stream.read(&mPathIndex);
   mathRead(stream, &mOffset);

   U32 numTriggers;
   stream.read(&numTriggers);
   mTriggers.setSize(numTriggers);
   for (S32 i = 0; i < mTriggers.size(); i++)
      mTriggers[i] = stream.readSTString();

   return (stream.getStatus() == Stream::Ok);
}

bool PathedInterior::writePI(Stream& stream) const
{
   stream.writeString(mName);
   stream.writeString(mInteriorResName);
   stream.write(mInteriorResIndex);
   stream.write(mPathIndex);
   mathWrite(stream, mOffset);

   stream.write(mTriggers.size());
   for (S32 i = 0; i < mTriggers.size(); i++)
      stream.writeString(mTriggers[i]);

   return (stream.getStatus() == Stream::Ok);
}

PathedInterior* PathedInterior::clone() const
{
   PathedInterior* pClone = new PathedInterior;
   
   pClone->mName             = mName;
   pClone->mInteriorResName  = mInteriorResName;
   pClone->mInteriorResIndex = mInteriorResIndex;
   pClone->mPathIndex        = mPathIndex;
   pClone->mOffset           = mOffset;

   return pClone;
}


