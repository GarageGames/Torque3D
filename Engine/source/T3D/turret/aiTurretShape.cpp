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

#include "T3D/turret/aiTurretShape.h"

#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "core/stream/bitStream.h"
#include "math/mMath.h"
#include "math/mathIO.h"
#include "math/mathUtils.h"
#include "gfx/gfxDrawUtil.h"
#include "ts/tsShapeInstance.h"
#include "math/mRandom.h"

static U32 sScanTypeMask =       PlayerObjectType     |
                                 VehicleObjectType;

static U32 sAimTypeMask =        TerrainObjectType       |
                                 WaterObjectType         | 
                                 PlayerObjectType        |
                                 StaticShapeObjectType   | 
                                 VehicleObjectType       |
                                 ItemObjectType;

//----------------------------------------------------------------------------

AITurretShapeData::StateData::StateData()
{
   name = 0;
   transition.rest[0] = transition.rest[1] = -1;
   transition.target[0] = transition.target[1] = -1;
   transition.activated[0] = transition.activated[1] = -1;
   transition.timeout = -1;
   waitForTimeout = true;
   timeoutValue = 0;
   fire = false;
   scan = false;
   script = 0;
   
   scaleAnimation = false;
   direction = false;

   sequence = -1;
}

static AITurretShapeData::StateData gDefaultStateData;

//----------------------------------------------------------------------------

IMPLEMENT_CO_DATABLOCK_V1(AITurretShapeData);

ConsoleDocClass( AITurretShapeData,
   "@brief Defines properties for an AITurretShape object.\n\n"
   "@see AITurretShape\n"
   "@see TurretShapeData\n"
   "@ingroup gameObjects\n"
);

AITurretShapeData::AITurretShapeData()
{
   maxScanHeading = 90;
   maxScanPitch = 90;
   maxScanDistance = 20;

   // Do a full scan every three ticks
   scanTickFrequency = 3;

   // Randomly add 0 to 1 ticks to the scan frequency as
   // chosen every scan period.
   scanTickFrequencyVariance = 1;

   trackLostTargetTime = 0;

   scanNode = -1;
   aimNode = -1;

   maxWeaponRange = 100;

   weaponLeadVelocity = 0;

   for (S32 i = 0; i < MaxStates; i++) {
      stateName[i] = 0;
      stateTransitionAtRest[i] = 0;
      stateTransitionNotAtRest[i] = 0;
      stateTransitionTarget[i] = 0;
      stateTransitionNoTarget[i] = 0;
      stateTransitionActivated[i] = 0;
      stateTransitionDeactivated[i] = 0;
      stateTransitionTimeout[i] = 0;
      stateWaitForTimeout[i] = true;
      stateTimeoutValue[i] = 0;
      stateFire[i] = false;
      stateScan[i] = false;
      stateScaleAnimation[i] = true;
      stateDirection[i] = true;
      stateSequence[i] = 0;
      stateScript[i] = 0;
   }
   isAnimated = false;
   statesLoaded = false;
}

void AITurretShapeData::initPersistFields()
{
   addField("maxScanHeading",       TypeF32,       Offset(maxScanHeading,        AITurretShapeData),
      "@brief Maximum number of degrees to scan left and right.\n\n"
      "@note Maximum scan heading is 90 degrees.\n");
   addField("maxScanPitch",         TypeF32,       Offset(maxScanPitch,          AITurretShapeData),
      "@brief Maximum number of degrees to scan up and down.\n\n"
      "@note Maximum scan pitch is 90 degrees.\n");
   addField("maxScanDistance",      TypeF32,       Offset(maxScanDistance,       AITurretShapeData),
      "@brief Maximum distance to scan.\n\n"
      "When combined with maxScanHeading and maxScanPitch this forms a 3D scanning wedge used to initially "
      "locate a target.\n");

   addField("scanTickFrequency",          TypeS32,       Offset(scanTickFrequency,       AITurretShapeData),
      "@brief How often should we perform a full scan when looking for a target.\n\n"
      "Expressed as the number of ticks between full scans, but no less than 1.\n");
   addField("scanTickFrequencyVariance",  TypeS32,       Offset(scanTickFrequencyVariance,       AITurretShapeData),
      "@brief Random amount that should be added to the scan tick frequency each scan period.\n\n"
      "Expressed as the number of ticks to randomly add, but no less than zero.\n");

   addField("trackLostTargetTime",  TypeF32,       Offset(trackLostTargetTime,       AITurretShapeData),
      "@brief How long after the turret has lost the target should it still track it.\n\n"
      "Expressed in seconds.\n");

   addField("maxWeaponRange",       TypeF32,       Offset(maxWeaponRange,       AITurretShapeData),
      "@brief Maximum distance that the weapon will fire upon a target.\n\n");

   addField("weaponLeadVelocity",   TypeF32,       Offset(weaponLeadVelocity,   AITurretShapeData),
      "@brief Velocity used to lead target.\n\n"
      "If value <= 0, don't lead target.\n");

   // State arrays
   addArray( "States", MaxStates );

      addField( "stateName", TypeCaseString, Offset(stateName, AITurretShapeData), MaxStates,
         "Name of this state." );
      addField( "stateTransitionOnAtRest", TypeString, Offset(stateTransitionAtRest, AITurretShapeData), MaxStates,
         "Name of the state to transition to when the turret is at rest (static).");
      addField( "stateTransitionOnNotAtRest", TypeString, Offset(stateTransitionNotAtRest, AITurretShapeData), MaxStates,
         "Name of the state to transition to when the turret is not at rest (not static).");
      addField( "stateTransitionOnTarget", TypeString, Offset(stateTransitionTarget, AITurretShapeData), MaxStates,
         "Name of the state to transition to when the turret gains a target." );
      addField( "stateTransitionOnNoTarget", TypeString, Offset(stateTransitionNoTarget, AITurretShapeData), MaxStates,
         "Name of the state to transition to when the turret loses a target." );
      addField( "stateTransitionOnActivated", TypeString, Offset(stateTransitionActivated, AITurretShapeData), MaxStates,
         "Name of the state to transition to when the turret goes from deactivated to activated.");
      addField( "stateTransitionOnDeactivated", TypeString, Offset(stateTransitionDeactivated, AITurretShapeData), MaxStates,
         "Name of the state to transition to when the turret goes from activated to deactivated");
      addField( "stateTransitionOnTimeout", TypeString, Offset(stateTransitionTimeout, AITurretShapeData), MaxStates,
         "Name of the state to transition to when we have been in this state "
         "for stateTimeoutValue seconds." );
      addField( "stateTimeoutValue", TypeF32, Offset(stateTimeoutValue, AITurretShapeData), MaxStates,
         "Time in seconds to wait before transitioning to stateTransitionOnTimeout." );
      addField( "stateWaitForTimeout", TypeBool, Offset(stateWaitForTimeout, AITurretShapeData), MaxStates,
         "If false, this state ignores stateTimeoutValue and transitions "
         "immediately if other transition conditions are met." );
      addField( "stateFire", TypeBool, Offset(stateFire, AITurretShapeData), MaxStates,
         "The first state with this set to true is the state entered by the "
         "client when it receives the 'fire' event." );
      addField( "stateScan", TypeBool, Offset(stateScan, AITurretShapeData), MaxStates,
         "Indicates the turret should perform a continuous scan looking for targets." );
      addField( "stateDirection", TypeBool, Offset(stateDirection, AITurretShapeData), MaxStates,
         "@brief Direction of the animation to play in this state.\n\n"
         "True is forward, false is backward." );
      addField( "stateSequence", TypeString, Offset(stateSequence, AITurretShapeData), MaxStates,
         "Name of the sequence to play on entry to this state." );
      addField( "stateScaleAnimation", TypeBool, Offset(stateScaleAnimation, AITurretShapeData), MaxStates,
         "If true, the timeScale of the stateSequence animation will be adjusted "
         "such that the sequence plays for stateTimeoutValue seconds. " );
      addField( "stateScript", TypeCaseString, Offset(stateScript, AITurretShapeData), MaxStates,
         "@brief Method to execute on entering this state.\n\n"
         "Scoped to AITurretShapeData.");

   endArray( "States" );

   Parent::initPersistFields();
}

bool AITurretShapeData::onAdd()
{
   if (!Parent::onAdd())
      return false;

   // Copy state data from the scripting arrays into the
   // state structure array. If we have state data already,
   // we are on the client and need to leave it alone.
   for (U32 i = 0; i < MaxStates; i++) {
      StateData& s = state[i];
      if (statesLoaded == false) {
         s.name = stateName[i];
         s.transition.rest[0] = lookupState(stateTransitionNotAtRest[i]);
         s.transition.rest[1] = lookupState(stateTransitionAtRest[i]);
         s.transition.target[0] = lookupState(stateTransitionNoTarget[i]);
         s.transition.target[1] = lookupState(stateTransitionTarget[i]);
         s.transition.activated[0] = lookupState(stateTransitionDeactivated[i]);
         s.transition.activated[1] = lookupState(stateTransitionActivated[i]);
         s.transition.timeout = lookupState(stateTransitionTimeout[i]);
         s.waitForTimeout = stateWaitForTimeout[i];
         s.timeoutValue = stateTimeoutValue[i];
         s.fire = stateFire[i];
         s.scan = stateScan[i];
         s.scaleAnimation = stateScaleAnimation[i];
         s.direction = stateDirection[i];
         s.script = stateScript[i];

         // Resolved at load time
         s.sequence = -1;
      }

      // The first state marked as "fire" is the state entered on the
      // client when it recieves a fire event.
      if (s.fire && fireState == -1)
         fireState = i;
   }

   // Always preload images, this is needed to avoid problems with
   // resolving sequences before transmission to a client.
   return true;
}

bool AITurretShapeData::preload(bool server, String &errorStr)
{
   if (!Parent::preload(server, errorStr))
      return false;

   // We have mShape at this point.  Resolve nodes.
   scanNode = mShape->findNode("scanPoint");
   aimNode = mShape->findNode("aimPoint");
   if (aimNode == -1)
   {
      aimNode = pitchNode;
   }
   if (aimNode == -1)
   {
      aimNode = headingNode;
   }

   // Resolve state sequence names & emitter nodes
   isAnimated = false;
   for (U32 j = 0; j < MaxStates; j++) {
      StateData& s = state[j];
      if (stateSequence[j] && stateSequence[j][0])
         s.sequence = mShape->findSequence(stateSequence[j]);
      if (s.sequence != -1)
      {
         // This state has an animation sequence
         isAnimated = true;
      }
   }

   return true;
}

void AITurretShapeData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->write(maxScanHeading);
   stream->write(maxScanPitch);
   stream->write(maxScanDistance);
   stream->write(maxWeaponRange);
   stream->write(weaponLeadVelocity);

   for (U32 i = 0; i < MaxStates; i++)
      if (stream->writeFlag(state[i].name && state[i].name[0])) {
         StateData& s = state[i];
         // States info not needed on the client:
         //    s.scriptNames
         // Transitions are inc. one to account for -1 values
         stream->writeString(state[i].name);

         stream->writeInt(s.transition.rest[0]+1,NumStateBits);
         stream->writeInt(s.transition.rest[1]+1,NumStateBits);
         stream->writeInt(s.transition.target[0]+1,NumStateBits);
         stream->writeInt(s.transition.target[1]+1,NumStateBits);
         stream->writeInt(s.transition.activated[0]+1,NumStateBits);
         stream->writeInt(s.transition.activated[1]+1,NumStateBits);
         stream->writeInt(s.transition.timeout+1,NumStateBits);

         if(stream->writeFlag(s.timeoutValue != gDefaultStateData.timeoutValue))
            stream->write(s.timeoutValue);

         stream->writeFlag(s.waitForTimeout);
         stream->writeFlag(s.fire);
         stream->writeFlag(s.scan);
         stream->writeFlag(s.scaleAnimation);
         stream->writeFlag(s.direction);

         if(stream->writeFlag(s.sequence != gDefaultStateData.sequence))
            stream->writeSignedInt(s.sequence, 16);
      }
}

void AITurretShapeData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   stream->read(&maxScanHeading);
   stream->read(&maxScanPitch);
   stream->read(&maxScanDistance);
   stream->read(&maxWeaponRange);
   stream->read(&weaponLeadVelocity);

   for (U32 i = 0; i < MaxStates; i++) {
      if (stream->readFlag()) {
         StateData& s = state[i];
         // States info not needed on the client:
         //    s.scriptNames
         // Transitions are dec. one to restore -1 values
         s.name = stream->readSTString();

         s.transition.rest[0] = stream->readInt(NumStateBits) - 1;
         s.transition.rest[1] = stream->readInt(NumStateBits) - 1;
         s.transition.target[0] = stream->readInt(NumStateBits) - 1;
         s.transition.target[1] = stream->readInt(NumStateBits) - 1;
         s.transition.activated[0] = stream->readInt(NumStateBits) - 1;
         s.transition.activated[1] = stream->readInt(NumStateBits) - 1;
         s.transition.timeout = stream->readInt(NumStateBits) - 1;
         if(stream->readFlag())
            stream->read(&s.timeoutValue);
         else
            s.timeoutValue = gDefaultStateData.timeoutValue;

         s.waitForTimeout = stream->readFlag();
         s.fire = stream->readFlag();
         s.scan = stream->readFlag();
         s.scaleAnimation = stream->readFlag();
         s.direction = stream->readFlag();

         if(stream->readFlag())
            s.sequence = stream->readSignedInt(16);
      }
   }

   statesLoaded = true;
}

S32 AITurretShapeData::lookupState(const char* name)
{
   if (!name || !name[0])
      return -1;
   for (U32 i = 0; i < MaxStates; i++)
      if (stateName[i] && !dStricmp(name,stateName[i]))
         return i;
   Con::errorf(ConsoleLogEntry::General,"AITurretShapeData:: Could not resolve state \"%s\" for image \"%s\"",name,getName());
   return 0;
}


//----------------------------------------------------------------------------

// Used to build potential target list
static void _scanCallback( SceneObject* object, void* data )
{
   AITurretShape* turret = (AITurretShape*)data;

   ShapeBase* shape = dynamic_cast<ShapeBase*>(object);
   if (shape && shape->getDamageState() == ShapeBase::Enabled)
   {
      Point3F targetPos = shape->getBoxCenter();

      // Put target position into the scan node's space
      turret->mScanWorkspaceScanWorldMat.mulP(targetPos);

      // Is the target within scanning distance
      if (targetPos.lenSquared() > turret->getMaxScanDistanceSquared())
         return;

      // Make sure the target is in front and within the maximum
      // heading range
      Point2F targetXY(targetPos.x, targetPos.y);
      targetXY.normalizeSafe();
      F32 headingDot = mDot(Point2F(0, 1), targetXY);
      F32 heading = mAcos(headingDot);
      if (headingDot < 0 || heading > turret->getMaxScanHeading())
         return;

      // Make sure the target is in front and within the maximum
      // pitch range
      Point2F targetZY(targetPos.z, targetPos.y);
      targetZY.normalizeSafe();
      F32 pitchDot = mDot(Point2F(0, 1), targetZY);
      F32 pitch = mAcos(pitchDot);
      if (pitchDot < 0 || pitch > turret->getMaxScanPitch())
         return;

      turret->addPotentialTarget(shape);
   }
}

// Used to sort potential target list based on distance
static Point3F comparePoint;
static S32 QSORT_CALLBACK _sortCallback(const void* a,const void* b)
{
   const ShapeBase* s1 = (*reinterpret_cast<const ShapeBase* const*>(a));
   const ShapeBase* s2 = (*reinterpret_cast<const ShapeBase* const*>(b));

   F32 s1Len = (s1->getPosition() - comparePoint).lenSquared();
   F32 s2Len = (s2->getPosition() - comparePoint).lenSquared();

   return s1Len - s2Len;
}

IMPLEMENT_CO_NETOBJECT_V1(AITurretShape);

ConsoleDocClass( AITurretShape,
   "@ingroup gameObjects"
);

AITurretShape::AITurretShape()
{
   mTypeMask |= VehicleObjectType | DynamicShapeObjectType;
   mDataBlock = 0;

   mScanHeading = 0;
   mScanPitch = 0;
   mScanDistance = 0;
   mScanDistanceSquared = 0;
   mScanBox = Box3F::Zero;

   mScanTickFrequency = 1;
   mScanTickFrequencyVariance = 0;
   mTicksToNextScan = mScanTickFrequency;

   mWeaponRangeSquared = 0;
   mWeaponLeadVelocitySquared = 0;

   mScanForTargets = false;
   mTrackTarget = false;

   mState = NULL;
   mStateDelayTime = 0;
   mStateActive = true;
   mStateAnimThread = NULL;

   // For the TurretShape class
   mSubclassTurretShapeHandlesScene = true;
}

AITurretShape::~AITurretShape()
{
   _cleanupPotentialTargets();
}

//----------------------------------------------------------------------------

void AITurretShape::initPersistFields()
{
   Parent::initPersistFields();
}

bool AITurretShape::onAdd()
{
   if( !Parent::onAdd() )
      return false;

   // Add this object to the scene
   addToScene();

   _setScanBox();

   if (isServerObject())
      _initState();

   if (isServerObject())
      scriptOnAdd();

   return true;
}

void AITurretShape::onRemove()
{
   Parent::onRemove();

   scriptOnRemove();

   mIgnoreObjects.clear();

   // Remove this object from the scene
   removeFromScene();
}

bool AITurretShape::onNewDataBlock(GameBaseData* dptr, bool reload)
{
   mDataBlock = dynamic_cast<AITurretShapeData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr, reload))
      return false;

   mScanHeading = mDegToRad(mDataBlock->maxScanHeading);
   if (mIsZero(mScanHeading))
      mScanHeading = M_PI_F;
   mScanPitch = mDegToRad(mDataBlock->maxScanPitch);
   if (mIsZero(mScanPitch))
      mScanPitch = M_PI_F;

   mScanDistance = mDataBlock->maxScanDistance;
   mScanDistanceSquared = mScanDistance * mScanDistance;

   mWeaponRangeSquared = mDataBlock->maxWeaponRange * mDataBlock->maxWeaponRange;
   mWeaponLeadVelocitySquared = (mDataBlock->weaponLeadVelocity > 0) ? (mDataBlock->weaponLeadVelocity * mDataBlock->weaponLeadVelocity) : 0;

   // The scan box is built such that the scanning origin is at (0,0,0) and the scanning distance
   // is out along the y axis.  When this is transformed to the turret's location is provides a
   // scanning volume in front of the turret.
   F32 scanX = mScanDistance*mSin(mScanHeading);
   F32 scanY = mScanDistance;
   F32 scanZ = mScanDistance*mSin(mScanPitch);
   mScanBox.set(-scanX, 0, -scanZ, scanX, scanY, scanZ);

   mScanTickFrequency = mDataBlock->scanTickFrequency;
   if (mScanTickFrequency < 1)
      mScanTickFrequency = 1;

   mScanTickFrequencyVariance = mDataBlock->scanTickFrequencyVariance;
   if (mScanTickFrequencyVariance < 0)
      mScanTickFrequencyVariance = 0;

   mTicksToNextScan = mScanTickFrequency;

   // For states
   mStateAnimThread = 0;
   if (mDataBlock->isAnimated)
   {
      mStateAnimThread = mShapeInstance->addThread();
      mShapeInstance->setTimeScale(mStateAnimThread,0);
   }

   scriptOnNewDataBlock();
   return true;
}

//----------------------------------------------------------------------------

void AITurretShape::addToIgnoreList(ShapeBase* obj)
{
   mIgnoreObjects.addObject(obj);
}

void AITurretShape::removeFromIgnoreList(ShapeBase* obj)
{
   mIgnoreObjects.removeObject(obj);
}

//----------------------------------------------------------------------------

void AITurretShape::_initState()
{
   // Set the turret to its starting state.
   setTurretState(0, true);
}

void AITurretShape::setTurretStateName(const char* newState, bool force)
{
   S32 stateVal = mDataBlock->lookupState(newState);
   if (stateVal >= 0)
   {
      setTurretState(stateVal, force);
   }
}

void AITurretShape::setTurretState(U32 newState, bool force)
{
   setMaskBits(TurretStateMask);

   // If going back into the same state, just reset the timer
   // and invoke the script callback
   if (!force && mState == &mDataBlock->state[newState])
   {
      mStateDelayTime = mState->timeoutValue;
      if (mState->script && !isGhost())
         _scriptCallback(mState->script);

      return;
   }

   mState = &mDataBlock->state[newState];

   // Reset cyclic sequences back to the first frame to turn it off
   // (the first key frame should be it's off state).
   if (mStateAnimThread && mStateAnimThread->getSequence()->isCyclic())
   {
      mShapeInstance->setPos(mStateAnimThread,0);
      mShapeInstance->setTimeScale(mStateAnimThread,0);
   }

   AITurretShapeData::StateData& stateData = *mState;

   // Check for immediate transitions
   S32 ns;
   if ((ns = stateData.transition.rest[mAtRest]) != -1)
   {
      setTurretState(ns);
      return;
   }
   if ((ns = stateData.transition.target[mTarget.isValid()]) != -1)
   {
      setTurretState(ns);
      return;
   }
   if ((ns = stateData.transition.activated[mStateActive]) != -1)
   {
      setTurretState(ns);
      return;
   }

   //
   // Initialize the new state...
   //
   mStateDelayTime = stateData.timeoutValue;

   // Play animation
   if (mStateAnimThread && stateData.sequence != -1) 
   {
      mShapeInstance->setSequence(mStateAnimThread,stateData.sequence, stateData.direction ? 0.0f : 1.0f);
      F32 timeScale = (stateData.scaleAnimation && stateData.timeoutValue) ?
         mShapeInstance->getDuration(mStateAnimThread) / stateData.timeoutValue : 1.0f;
      mShapeInstance->setTimeScale(mStateAnimThread, stateData.direction ? timeScale : -timeScale);
   }

   // Script callback on server
   if (stateData.script && stateData.script[0] && !isGhost())
      _scriptCallback(stateData.script);

   // If there is a zero timeout, and a timeout transition, then
   // go ahead and transition imediately.
   if (!mStateDelayTime)
   {
      if ((ns = stateData.transition.timeout) != -1)
      {
         setTurretState(ns);
         return;
      }
   }
}

void AITurretShape::_updateTurretState(F32 dt)
{
   AITurretShapeData::StateData& stateData = *mState;

   mStateDelayTime -= dt;

   // Check for transitions. On some states we must wait for the
   // full timeout value before moving on.
   if (mStateDelayTime <= 0 || !stateData.waitForTimeout) 
   {
      S32 ns;

      if ((ns = stateData.transition.rest[mAtRest]) != -1) 
         setTurretState(ns);
      else if ((ns = stateData.transition.target[mTarget.isValid()]) != -1) 
         setTurretState(ns);
      else if ((ns = stateData.transition.activated[mStateActive]) != -1)
         setTurretState(ns);
      else if (mStateDelayTime <= 0 && (ns = stateData.transition.timeout) != -1) 
         setTurretState(ns);
   }
}

void AITurretShape::_scriptCallback(const char* function)
{
   Con::executef( mDataBlock, function, getIdString() );
}

//----------------------------------------------------------------------------

void AITurretShape::_cleanupPotentialTargets()
{
   mPotentialTargets.clear();
}

void AITurretShape::_performScan()
{
   // Only on server
   if (isClientObject())
      return;

   // Are we ready for a scan?
   --mTicksToNextScan;
   if (mTicksToNextScan > 0)
      return;

   _cleanupPotentialTargets();

   _setScanBox();

   // Set up for the scan
   getScanTransform(mScanWorkspaceScanMat);
   mScanWorkspaceScanWorldMat = mScanWorkspaceScanMat;
   mScanWorkspaceScanWorldMat.affineInverse();

   disableCollision();
   for ( SimSetIterator iter(&mIgnoreObjects); *iter; ++iter )
   {
      ShapeBase* obj = static_cast<ShapeBase*>( *iter );
      obj->disableCollision();
   }

   gServerContainer.findObjects( mTransformedScanBox, sScanTypeMask, _scanCallback, (void*)this );

   for ( SimSetIterator iter(&mIgnoreObjects); *iter; ++iter )
   {
      ShapeBase* obj = static_cast<ShapeBase*>( *iter );
      obj->enableCollision();
   }
   enableCollision();

   if (mPotentialTargets.size() == 0)
   {
      // No targets in range.  Clear out our current target, if necessary.
      _lostTarget();
   }
   else
   {
      // Sort the targets
      comparePoint = getPosition();
      dQsort(mPotentialTargets.address(),mPotentialTargets.size(),sizeof(SimObjectList::value_type),_sortCallback);

      // Go through the targets in order to find one that is not blocked from view
      Point3F start;
      mScanWorkspaceScanMat.getColumn(3, &start);
      S32 index = 0;
      bool los = false;

      disableCollision();
      for (index=0; index < mPotentialTargets.size(); ++index)
      {
         ShapeBase* shape = (ShapeBase*)mPotentialTargets[index];

         Point3F sightPoint;
         los = _testTargetLineOfSight(start, shape, sightPoint);

         // Check if we have a clear line of sight
         if (los)
            break;
      }
      enableCollision();

      // If we found a valid, visible target (no hits between here and there), latch on to it
      if (los)
      {
         _gainedTarget((ShapeBase*)mPotentialTargets[index]);
      }
   }

   // Prepare for next scan period
   mTicksToNextScan = mScanTickFrequency;
   if (mScanTickFrequencyVariance > 0)
   {
      mTicksToNextScan += gRandGen.randI(0, mScanTickFrequencyVariance);
   }
}

void AITurretShape::addPotentialTarget(ShapeBase* shape)
{
   mPotentialTargets.push_back(shape);
}

void AITurretShape::_lostTarget()
{
   mTarget.target = NULL;
}

void AITurretShape::_gainedTarget(ShapeBase* target)
{
   mTarget.target = target;
   if (target)
   {
      mTarget.hadValidTarget = true;
   }
}

void AITurretShape::_trackTarget(F32 dt)
{
   // Only on server
   if (isClientObject())
      return;

   // We can only track a target if we have one
   if (!mTarget.isValid())
      return;

   Point3F targetPos = mTarget.target->getBoxCenter();

   // Can we see the target?
   MatrixF aimMat;
   getAimTransform(aimMat);
   Point3F start;
   aimMat.getColumn(3, &start);
   RayInfo ri;

   Point3F sightPoint;

   disableCollision();
   bool los = _testTargetLineOfSight(start, mTarget.target, sightPoint);
   enableCollision();

   if (!los)
   {
      // Target is blocked.  Should we try to track from its last
      // known position and velocity?
      SimTime curTime = Sim::getCurrentTime();
      if ( (curTime - mTarget.lastSightTime) > (mDataBlock->trackLostTargetTime * 1000.0f) )
      {
         // Time's up.  Stop tracking.
         _cleanupTargetAndTurret();
         return;
      }
      
      // Use last known information to attempt to
      // continue to track target for a while.
      targetPos = mTarget.lastPos + mTarget.lastVel * F32(curTime - mTarget.lastSightTime) / 1000.0f;
   }
   else
   {
      // Target is visible

      // We only track targets that are alive
      if (mTarget.target->getDamageState() != Enabled)
      {
         // We can't track any more
         _cleanupTargetAndTurret();
         return;
      }

      targetPos = sightPoint;

      // Store latest target info
      mTarget.lastPos = targetPos;
      mTarget.lastVel = mTarget.target->getVelocity();
      mTarget.lastSightTime = Sim::getCurrentTime();
   }

   // Calculate angles to face the target, specifically the part that we can see
   VectorF toTarget;
   MatrixF mat;
   S32 node = mDataBlock->aimNode;
   if (node != -1)
   {
      // Get the current position of our node
      MatrixF* nodeTrans = &mShapeInstance->mNodeTransforms[node];
      Point3F currentPos;
      nodeTrans->getColumn(3, &currentPos);

      // Turn this into a matrix we can use to put the target
      // position into our space.
      MatrixF nodeMat(true);
      nodeMat.setColumn(3, currentPos);
      mat.mul(mObjToWorld, nodeMat);
      mat.affineInverse();
   }
   else
   {
      mat = mWorldToObj;
   }
   mat.mulP(targetPos, &toTarget);

   // lead the target
   F32 timeToTargetSquared = (mWeaponLeadVelocitySquared > 0) ? toTarget.lenSquared() / mWeaponLeadVelocitySquared : 0;
   if (timeToTargetSquared > 1.0)
   {
      targetPos = targetPos + (mTarget.lastVel * mSqrt(timeToTargetSquared));
      mat.mulP(targetPos, &toTarget);
   }

   F32 yaw, pitch;
   MathUtils::getAnglesFromVector(toTarget, yaw, pitch);
   if (yaw > M_PI_F)
      yaw = yaw - M_2PI_F;
   //if (pitch > M_PI_F)
   //   pitch = -(pitch - M_2PI_F);

   Point3F rot(-pitch, 0.0f, yaw);

   // If we have a rotation rate make sure we follow it
   if (mHeadingRate > 0)
   {
      F32 rate = mHeadingRate * dt;
      F32 rateCheck = mFabs(rot.z - mRot.z);
      if (rateCheck > rate)
      {
         // This will clamp the new value to the rate regardless if it
         // is increasing or decreasing.
         rot.z = mClampF(rot.z, mRot.z-rate, mRot.z+rate);
      }
   }
   if (mPitchRate > 0)
   {
      F32 rate = mPitchRate * dt;
      F32 rateCheck = mFabs(rot.x - mRot.x);
      if (rateCheck > rate)
      {
         // This will clamp the new value to the rate regardless if it
         // is increasing or decreasing.
         rot.x = mClampF(rot.x, mRot.x-rate, mRot.x+rate);
      }
   }

   // Test if the rotation to the target is outside of our limits
   if (_outsideLimits(rot))
   {
      // We can't track any more
      _cleanupTargetAndTurret();
      return;
   }

   // Test if the target is out of weapons range
   if (toTarget.lenSquared() > mWeaponRangeSquared)
   {
      // We can't track any more
      _cleanupTargetAndTurret();
      return;
   }

   mRot = rot;

   _setRotation( mRot );
   setMaskBits(TurretUpdateMask);
}

void AITurretShape::_cleanupTargetAndTurret()
{
   _lostTarget();
   resetTarget();
}

bool AITurretShape::_testTargetLineOfSight(Point3F& aimPoint, ShapeBase* target, Point3F& sightPoint)
{
   Point3F targetCenter = target->getBoxCenter();
   RayInfo ri;
   bool hit = false;

   target->disableCollision();
   
   // First check for a clear line of sight to the target's center
   Point3F testPoint =  targetCenter;
   hit = gServerContainer.castRay(aimPoint, testPoint, sAimTypeMask, &ri);
   if (hit)
   {
      // No clear line of sight to center, so try to the target's right.  Players holding
      // a gun in their right hand will tend to stick their right shoulder out first if
      // they're peering around some cover to shoot, like a wall.
      Box3F targetBounds = target->getObjBox();
      F32 radius = targetBounds.len_x() > targetBounds.len_y() ? targetBounds.len_x() : targetBounds.len_y();
      radius *= 0.5;

      VectorF toTurret = aimPoint - targetCenter;
      toTurret.normalizeSafe();
      VectorF toTurretRight = mCross(toTurret, Point3F::UnitZ);

      testPoint = targetCenter + toTurretRight * radius;

      hit = gServerContainer.castRay(aimPoint, testPoint, sAimTypeMask, &ri);

      if (hit)
      {
         // No clear line of sight to right, so try the target's left
         VectorF toTurretLeft = toTurretRight * -1.0f;
         testPoint = targetCenter + toTurretLeft * radius;
         hit = gServerContainer.castRay(aimPoint, testPoint, sAimTypeMask, &ri);
      }

      if (hit)
      {
         // No clear line of sight to left, so try the target's top
         testPoint = targetCenter;
         testPoint.z += targetBounds.len_z() * 0.5f;
         hit = gServerContainer.castRay(aimPoint, testPoint, sAimTypeMask, &ri);
      }

      if (hit)
      {
         // No clear line of sight to top, so try the target's bottom
         testPoint = targetCenter;
         testPoint.z -= targetBounds.len_z() * 0.5f;
         hit = gServerContainer.castRay(aimPoint, testPoint, sAimTypeMask, &ri);
      }
   }
   
   target->enableCollision();

   if (!hit)
   {
      // Line of sight point is that last one  we tested
      sightPoint = testPoint;
   }

   return !hit;
}

//----------------------------------------------------------------------------

void AITurretShape::setAllGunsFiring(bool fire)
{
   setImageTriggerState(0,fire);
   setImageTriggerState(1,fire);
   setImageTriggerState(2,fire);
   setImageTriggerState(3,fire);
}

void AITurretShape::setGunSlotFiring(S32 slot, bool fire)
{
   if (slot < 0 || slot > 3)
      return;

   setImageTriggerState(slot, fire);
}

//----------------------------------------------------------------------------

void AITurretShape::setTransform(const MatrixF& mat)
{
   Parent::setTransform(mat);

   // Set the scanning box
   _setScanBox();
}

void AITurretShape::recenterTurret()
{
   mRot.set(0,0,0);
   _setRotation( mRot );
   setMaskBits(TurretUpdateMask);
}

void AITurretShape::_setScanBox()
{
   mTransformedScanBox = mScanBox;
   MatrixF mat;
   getScanTransform(mat);
   mat.mul(mTransformedScanBox);
}

void AITurretShape::getScanTransform(MatrixF& mat)
{
   if (mDataBlock && mDataBlock->scanNode != -1)
   {
      if (getNodeTransform(mDataBlock->scanNode, mat))
      {
         return;
      }
   }

   mat = mObjToWorld;
}

void AITurretShape::getAimTransform(MatrixF& mat)
{
   if (mDataBlock && mDataBlock->aimNode != -1)
   {
      if (getNodeTransform(mDataBlock->aimNode, mat))
      {
         return;
      }
   }

   mat = mObjToWorld;
}

//----------------------------------------------------------------------------

void AITurretShape::processTick(const Move* move)
{
   Parent::processTick(move);

   if (isServerObject() && mDamageState == Enabled)
   {
      _updateTurretState(TickSec);

      if (mScanForTargets)
      {
         // Perform a scan for targets
         _performScan();

         // If we found one, turn off the scan
         if (mTarget.isValid())
         {
            mScanForTargets = false;
         }
      }

      if (mTrackTarget)
      {
         _trackTarget(TickSec);

         // If the target is lost, no longer track it
         if (!mTarget.isValid())
         {
            mTrackTarget = false;
         }
      }
   }
}

void AITurretShape::advanceTime(F32 dt)
{
   // If there were any ShapeBase script threads that
   // have played, then we need to update all code
   // controlled nodes.  This is done before the Parent
   // call as script threads may play and be destroyed
   // before our code is called.
   bool updateNodes = false;
   for (U32 i = 0; i < MaxScriptThreads; i++)
   {
      Thread& st = mScriptThread[i];
      if (st.thread)
      {
         updateNodes = true;
         break;
      }
   }

   Parent::advanceTime(dt);

   // Update any state thread
   AITurretShapeData::StateData& stateData = *mState;
   if (mStateAnimThread && stateData.sequence != -1) 
   {
      mShapeInstance->advanceTime(dt,mStateAnimThread); 

      updateNodes = true;
   }

   if (updateNodes)
   {
      // Update all code controlled nodes
      _updateNodes(mRot);
   }
}

//----------------------------------------------------------------------------

U32 AITurretShape::packUpdate(NetConnection *connection, U32 mask, BitStream *bstream)
{
   U32 retMask = Parent::packUpdate(connection,mask,bstream);

   // Indicate that the transform has changed to update the scan box
   bstream->writeFlag(mask & (PositionMask | ExtendedInfoMask));

   // Handle any state changes that need to be passed along
   if (bstream->writeFlag(mask & TurretStateMask))
   {
      bstream->write(mDataBlock->lookupState(mState->name));
   }

   return retMask;
}

void AITurretShape::unpackUpdate(NetConnection *connection, BitStream *bstream)
{
   Parent::unpackUpdate(connection,bstream);

   // Transform has changed
   if (bstream->readFlag())
   {
      _setScanBox();
   }

   //TurretStateMask
   if (bstream->readFlag())
   {
      S32 newstate;
      bstream->read( &newstate );
      setTurretState(newstate);
   }
}

//----------------------------------------------------------------------------

void AITurretShape::prepBatchRender( SceneRenderState *state, S32 mountedImageIndex )
{
   Parent::prepBatchRender( state, mountedImageIndex );

   if ( !gShowBoundingBox )
      return;

   ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
   ri->renderDelegate.bind( this, &AITurretShape::_renderScanner );
   ri->type = RenderPassManager::RIT_Editor;
   state->getRenderPass()->addInst( ri );
}

void AITurretShape::_renderScanner( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat )
{
   GFXStateBlockDesc desc;
   desc.setBlend(false, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
   desc.setZReadWrite(false,true);
   desc.fillMode = GFXFillWireframe;

   // Render the scan box
   GFX->getDrawUtil()->drawCube(desc, mTransformedScanBox.getExtents(), mTransformedScanBox.getCenter(), ColorI(255, 0, 0));

   // Render a line from the scan node to the max scan distance
   MatrixF nodeMat;
   if (getNodeTransform(mDataBlock->scanNode, nodeMat))
   {
      Point3F start;
      nodeMat.getColumn(3, &start);

      Point3F end(0.0f, mScanDistance, 0.0f);
      nodeMat.mulP(end);

      GFX->getDrawUtil()->drawLine(start, end, ColorI(255, 255, 0));
   }
}

//----------------------------------------------------------------------------

DefineEngineMethod( AITurretShape, addToIgnoreList, void, (ShapeBase* obj),,
   "@brief Adds object to the turret's ignore list.\n\n"
   "All objects in this list will be ignored by the turret's targeting.\n"
   "@param obj The ShapeBase object to ignore.\n")
{
   object->addToIgnoreList(obj);
}

DefineEngineMethod( AITurretShape, removeFromIgnoreList, void, (ShapeBase* obj),,
   "@brief Removes object from the turret's ignore list.\n\n"
   "All objects in this list will be ignored by the turret's targeting.\n"
   "@param obj The ShapeBase object to once again allow for targeting.\n")
{
   object->removeFromIgnoreList(obj);
}

DefineEngineMethod( AITurretShape, setTurretState, void, (const char* newState, bool force), (false),
   "@brief Set the turret's current state.\n\n"
   "Normally the turret's state comes from updating the state machine but this method "
   "allows you to override this and jump to the requested state immediately.\n"
   "@param newState The name of the new state.\n"
   "@param force Is true then force the full processing of the new state even if it is the "
   "same as the current state.  If false then only the time out value is reset and the state's "
   "script method is called, if any.\n")
{
   object->setTurretStateName(newState, force);
}

DefineEngineMethod( AITurretShape, activateTurret, void, ( ),,
   "@brief Activate a turret from a deactive state.\n\n")
{
   object->activateTurret();
}

DefineEngineMethod( AITurretShape, deactivateTurret, void, ( ),,
   "@brief Deactivate a turret from an active state.\n\n")
{
   object->deactivateTurret();
}

DefineEngineMethod( AITurretShape, startScanForTargets, void, ( ),,
   "@brief Begin scanning for a target.\n\n")
{
   object->startScanForTargets();
}

DefineEngineMethod( AITurretShape, stopScanForTargets, void, ( ),,
   "@brief Stop scanning for targets.\n\n"
   "@note Only impacts the scanning for new targets.  Does not effect a turret's current "
   "target lock.\n")
{
   object->stopScanForTargets();
}

DefineEngineMethod( AITurretShape, startTrackingTarget, void, ( ),,
   "@brief Have the turret track the current target.\n\n")
{
   object->startTrackingTarget();
}

DefineEngineMethod( AITurretShape, stopTrackingTarget, void, ( ),,
   "@brief Stop the turret from tracking the current target.\n\n")
{
   object->stopTrackingTarget();
}

DefineEngineMethod( AITurretShape, hasTarget, bool, (),,
   "@brief Indicates if the turret has a target.\n\n"
   "@returns True if the turret has a target.\n")
{
   return object->hasTarget();
}

DefineEngineMethod( AITurretShape, getTarget, SimObject*, (),,
   "@brief Get the turret's current target.\n\n"
   "@returns The object that is the target's current target, or 0 if no target.\n")
{
   return object->getTarget();
}

DefineEngineMethod( AITurretShape, resetTarget, void, ( ),,
   "@brief Resets the turret's target tracking.\n\n"
   "Only resets the internal target tracking.  Does not modify the turret's facing.\n")
{
   object->resetTarget();
}

DefineEngineMethod( AITurretShape, setWeaponLeadVelocity, void, (F32 velocity),,
   "@brief Set the turret's projectile velocity to help lead the target.\n\n"
   "This value normally comes from AITurretShapeData::weaponLeadVelocity but this method "
   "allows you to override the datablock value.  This can be useful if the turret changes "
   "ammunition, uses a different weapon than the default, is damaged, etc.\n"
   "@note Setting this to 0 will disable target leading.\n")
{
   object->setWeaponLeadVelocity(velocity);
}

DefineEngineMethod( AITurretShape, getWeaponLeadVelocity, F32, (),,
   "@brief Get the turret's defined projectile velocity that helps with target leading.\n\n"
   "@returns The defined weapon projectile speed, or 0 if leading is disabled.\n")
{
   return object->getWeaponLeadVelocity();
}

DefineEngineMethod( AITurretShape, setAllGunsFiring, void, (bool fire),,
   "@brief Set the firing state of the turret's guns.\n\n"
   "@param fire Set to true to activate all guns.  False to deactivate them.\n")
{
   object->setAllGunsFiring(fire);
}

DefineEngineMethod( AITurretShape, setGunSlotFiring, void, (S32 slot, bool fire),,
   "@brief Set the firing state of the given gun slot.\n\n"
   "@param slot The gun to modify.  Valid range is 0-3 that corresponds to the weapon mount point.\n"
   "@param fire Set to true to activate the gun.  False to deactivate it.\n")
{
   object->setGunSlotFiring(slot, fire);
}

DefineEngineMethod( AITurretShape, recenterTurret, void, ( ),,
   "@brief Recenter the turret's weapon.\n\n")
{
   object->recenterTurret();
}
