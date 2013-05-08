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

#include "platform/input/leapMotion/leapMotionDevice.h"
#include "platform/input/leapMotion/leapMotionData.h"
#include "platform/input/leapMotion/leapMotionFrameStore.h"
#include "platform/platformInput.h"
#include "core/module.h"
#include "platform/threads/mutex.h"
#include "console/engineAPI.h"

MODULE_BEGIN( LeapMotionDevice )

   MODULE_INIT_AFTER( InputEventManager )
   MODULE_SHUTDOWN_BEFORE( InputEventManager )

   MODULE_INIT
   {
      LeapMotionDevice::staticInit();
      ManagedSingleton< LeapMotionDevice >::createSingleton();
      if(LeapMotionDevice::smEnableDevice)
      {
         LEAPMOTIONDEV->enable();
      }

      // Register the device with the Input Event Manager
      INPUTMGR->registerDevice(LEAPMOTIONDEV);
   }
   
   MODULE_SHUTDOWN
   {
      INPUTMGR->unregisterDevice(LEAPMOTIONDEV);
      ManagedSingleton< LeapMotionDevice >::deleteSingleton();
   }

MODULE_END;

//-----------------------------------------------------------------------------
// LeapMotionDevice
//-----------------------------------------------------------------------------

bool LeapMotionDevice::smEnableDevice = true;

bool LeapMotionDevice::smGenerateIndividualEvents = true;
bool LeapMotionDevice::smKeepHandIndexPersistent = false;
bool LeapMotionDevice::smKeepPointableIndexPersistent = false;

bool LeapMotionDevice::smGenerateSingleHandRotationAsAxisEvents = false;

F32 LeapMotionDevice::smMaximumHandAxisAngle = 25.0f;

bool LeapMotionDevice::smGenerateWholeFrameEvents = false;

U32 LeapMotionDevice::LM_FRAMEVALIDDATA = 0;
U32 LeapMotionDevice::LM_HAND[LeapMotionConstants::MaxHands] = {0};
U32 LeapMotionDevice::LM_HANDROT[LeapMotionConstants::MaxHands] = {0};
U32 LeapMotionDevice::LM_HANDAXISX = 0;
U32 LeapMotionDevice::LM_HANDAXISY = 0;
U32 LeapMotionDevice::LM_HANDPOINTABLE[LeapMotionConstants::MaxHands][LeapMotionConstants::MaxPointablesPerHand] = {0};
U32 LeapMotionDevice::LM_HANDPOINTABLEROT[LeapMotionConstants::MaxHands][LeapMotionConstants::MaxPointablesPerHand] = {0};
U32 LeapMotionDevice::LM_FRAME = 0;

LeapMotionDevice::LeapMotionDevice()
{
   // From IInputDevice
   dStrcpy(mName, "leapmotion");
   mDeviceType = INPUTMGR->getNextDeviceType();

   mController = NULL;
   mListener = NULL;
   mActiveMutex = Mutex::createMutex();

   //
   mEnabled = false;
   mActive = false;

   for(U32 i=0; i<2; ++i)
   {
      mDataBuffer[i] = new LeapMotionDeviceData();
   }
   mPrevData = mDataBuffer[0];

   buildCodeTable();
}

LeapMotionDevice::~LeapMotionDevice()
{
   disable();

   Mutex::destroyMutex(mActiveMutex);
}

void LeapMotionDevice::staticInit()
{
   Con::addVariable("pref::LeapMotion::EnableDevice", TypeBool, &smEnableDevice, 
      "@brief If true, the Leap Motion device will be enabled, if present.\n\n"
	   "@ingroup Game");

   Con::addVariable("LeapMotion::GenerateIndividualEvents", TypeBool, &smGenerateIndividualEvents, 
      "@brief Indicates that events for each hand and pointable will be created.\n\n"
	   "@ingroup Game");
   Con::addVariable("LeapMotion::KeepHandIndexPersistent", TypeBool, &smKeepHandIndexPersistent, 
      "@brief Indicates that we track hand IDs and will ensure that the same hand will remain at the same index between frames.\n\n"
	   "@ingroup Game");
   Con::addVariable("LeapMotion::KeepPointableIndexPersistent", TypeBool, &smKeepPointableIndexPersistent, 
      "@brief Indicates that we track pointable IDs and will ensure that the same pointable will remain at the same index between frames.\n\n"
	   "@ingroup Game");

   Con::addVariable("LeapMotion::GenerateSingleHandRotationAsAxisEvents", TypeBool, &smGenerateSingleHandRotationAsAxisEvents, 
      "@brief If true, broadcast single hand rotation as axis events.\n\n"
	   "@ingroup Game");
   Con::addVariable("LeapMotion::MaximumHandAxisAngle", TypeF32, &smMaximumHandAxisAngle, 
      "@brief The maximum hand angle when used as an axis event as measured from a vector pointing straight up (in degrees).\n\n"
      "Shoud range from 0 to 90 degrees.\n\n"
	   "@ingroup Game");

   Con::addVariable("LeapMotion::GenerateWholeFrameEvents", TypeBool, &smGenerateWholeFrameEvents, 
      "@brief Indicates that a whole frame event should be generated and frames should be buffered.\n\n"
	   "@ingroup Game");
}

void LeapMotionDevice::buildCodeTable()
{
   // Obtain all of the device codes
   LM_FRAMEVALIDDATA = INPUTMGR->getNextDeviceCode();

   for(U32 i=0; i<LeapMotionConstants::MaxHands; ++i)
   {
      // Hands
      LM_HAND[i] = INPUTMGR->getNextDeviceCode();
      LM_HANDROT[i] = INPUTMGR->getNextDeviceCode();

      // Pointables per hand
      for(U32 j=0; j<LeapMotionConstants::MaxPointablesPerHand; ++j)
      {
         LM_HANDPOINTABLE[i][j] = INPUTMGR->getNextDeviceCode();
         LM_HANDPOINTABLEROT[i][j] = INPUTMGR->getNextDeviceCode();
      }
   }

   LM_HANDAXISX = INPUTMGR->getNextDeviceCode();
   LM_HANDAXISY = INPUTMGR->getNextDeviceCode();

   LM_FRAME = INPUTMGR->getNextDeviceCode();

   // Build out the virtual map
   AddInputVirtualMap(  lm_framevaliddata,      SI_BUTTON,     LM_FRAMEVALIDDATA );

   char buffer[64];
   for(U32 i=0; i<LeapMotionConstants::MaxHands; ++i)
   {
      // Hands
      dSprintf(buffer, 64, "lm_hand%d", i+1);
      INPUTMGR->addVirtualMap( buffer, SI_POS, LM_HAND[i] );
      dSprintf(buffer, 64, "lm_hand%drot", i+1);
      INPUTMGR->addVirtualMap( buffer, SI_ROT, LM_HANDROT[i] );

      // Pointables per hand
      for(U32 j=0; j<LeapMotionConstants::MaxPointablesPerHand; ++j)
      {
         dSprintf(buffer, 64, "lm_hand%dpoint%d", i+1, j+1);
         INPUTMGR->addVirtualMap( buffer, SI_POS, LM_HANDPOINTABLE[i][j] );
         dSprintf(buffer, 64, "lm_hand%dpoint%drot", i+1, j+1);
         INPUTMGR->addVirtualMap( buffer, SI_POS, LM_HANDPOINTABLEROT[i][j] );
      }
   }

   AddInputVirtualMap(  lm_handaxisx,  SI_AXIS,    LM_HANDAXISX );
   AddInputVirtualMap(  lm_handaxisy,  SI_AXIS,    LM_HANDAXISY );

   AddInputVirtualMap(  lm_frame,      SI_INT,     LM_FRAME );
}

bool LeapMotionDevice::enable()
{
   // Start off with disabling the device if it is already enabled
   disable();

   // Create the controller to talk with the Leap Motion along with the listener
   mListener = new MotionListener();
   mController = new Leap::Controller(*mListener);

   // The device is now enabled but not yet ready to be used
   mEnabled = true;

   return false;
}

void LeapMotionDevice::disable()
{
   if(mController)
   {
      delete mController;
      mController = NULL;

      if(mListener)
      {
         delete mListener;
         mListener = NULL;
      }
   }

   setActive(false);
   mEnabled = false;
}

bool LeapMotionDevice::getActive()
{
   Mutex::lockMutex(mActiveMutex);
   bool active = mActive;
   Mutex::unlockMutex(mActiveMutex);

   return active;
}

void LeapMotionDevice::setActive(bool state)
{
   Mutex::lockMutex(mActiveMutex);
   mActive = state;
   Mutex::unlockMutex(mActiveMutex);
}

bool LeapMotionDevice::process()
{
   if(!mEnabled)
      return false;

   if(!getActive())
      return false;

   //Con::printf("LeapMotionDevice::process()");

   //Build the maximum hand axis angle to be passed into the LeapMotionDeviceData::setData()
   F32 maxHandAxisRadius = mSin(mDegToRad(smMaximumHandAxisAngle));

   // Get a frame of data
   const Leap::Frame frame = mController->frame();

   //const Leap::HandList hands = frame.hands();
   //Con::printf("Frame: %lld  Hands: %d  Fingers: %d  Tools: %d", (long long)frame.id(), hands.count(), frame.fingers().count(), frame.tools().count());

   // Store the current data
   LeapMotionDeviceData* currentBuffer = (mPrevData == mDataBuffer[0]) ? mDataBuffer[1] : mDataBuffer[0];
   currentBuffer->setData(frame, mPrevData, smKeepHandIndexPersistent, smKeepPointableIndexPersistent, maxHandAxisRadius);
   U32 diff = mPrevData->compare(currentBuffer);
   U32 metaDiff = mPrevData->compareMeta(currentBuffer);

   // Update the previous data pointers.  We do this here in case someone calls our
   // console functions during one of the input events below.
   mPrevData = currentBuffer;

   // Send out any meta data
   if(metaDiff & LeapMotionDeviceData::METADIFF_FRAME_VALID_DATA)
   {
      // Frame valid change event
      INPUTMGR->buildInputEvent(mDeviceType, DEFAULT_MOTION_UNIT, SI_BUTTON, LM_FRAMEVALIDDATA, currentBuffer->mHasTrackingData ? SI_MAKE : SI_BREAK, currentBuffer->mHasTrackingData ? 1.0f : 0.0f);
   }

   // Send out any valid data
   if(currentBuffer->mDataSet && currentBuffer->mIsValid)
   {
      // Hands and their pointables
      if(smGenerateIndividualEvents)
      {
         for(U32 i=0; i<LeapMotionConstants::MaxHands; ++i)
         {
            if(currentBuffer->mHandValid[i])
            {
               // Send out position
               INPUTMGR->buildInputEvent(mDeviceType, DEFAULT_MOTION_UNIT, SI_POS, LM_HAND[i], SI_MOVE, currentBuffer->mHandPosPoint[i]);

               // Send out rotation
               INPUTMGR->buildInputEvent(mDeviceType, DEFAULT_MOTION_UNIT, SI_ROT, LM_HANDROT[i], SI_MOVE, currentBuffer->mHandRotQuat[i]);

               // Pointables for hand
               for(U32 j=0; j<LeapMotionConstants::MaxPointablesPerHand; ++j)
               {
                  if(currentBuffer->mPointableValid[i][j])
                  {
                     // Send out position
                     INPUTMGR->buildInputEvent(mDeviceType, DEFAULT_MOTION_UNIT, SI_POS, LM_HANDPOINTABLE[i][j], SI_MOVE, currentBuffer->mPointablePosPoint[i][j]);

                     // Send out rotation
                     INPUTMGR->buildInputEvent(mDeviceType, DEFAULT_MOTION_UNIT, SI_ROT, LM_HANDPOINTABLEROT[i][j], SI_MOVE, currentBuffer->mPointableRotQuat[i][j]);
                  }
               }
            }
         }
      }

      // Single Hand as axis rotation
      if(smGenerateSingleHandRotationAsAxisEvents && diff & LeapMotionDeviceData::DIFF_HANDROTAXIS)
      {
         if(diff & LeapMotionDeviceData::DIFF_HANDROTAXISX)
            INPUTMGR->buildInputEvent(mDeviceType, DEFAULT_MOTION_UNIT, SI_AXIS, LM_HANDAXISX, SI_MOVE, currentBuffer->mHandRotAxis[0]);
         if(diff & LeapMotionDeviceData::DIFF_HANDROTAXISY)
            INPUTMGR->buildInputEvent(mDeviceType, DEFAULT_MOTION_UNIT, SI_AXIS, LM_HANDAXISY, SI_MOVE, currentBuffer->mHandRotAxis[1]);
      }
   }

   // Send out whole frame event, but only if the special frame group is defined
   if(smGenerateWholeFrameEvents && LeapMotionFrameStore::isFrameGroupDefined())
   {
      S32 id = LEAPMOTIONFS->generateNewFrame(frame, maxHandAxisRadius);
      if(id != 0)
      {
         INPUTMGR->buildInputEvent(mDeviceType, DEFAULT_MOTION_UNIT, SI_INT, LM_FRAME, SI_VALUE, id);
      }
   }

   return true;
}

//-----------------------------------------------------------------------------
// LeapMotionDevice::MotionListener
//-----------------------------------------------------------------------------

void LeapMotionDevice::MotionListener::onConnect (const Leap::Controller &controller)
{
   LEAPMOTIONDEV->setActive(true);
}

void LeapMotionDevice::MotionListener::onDisconnect (const Leap::Controller &controller)
{
   LEAPMOTIONDEV->setActive(false);
}
//-----------------------------------------------------------------------------

DefineEngineFunction(isLeapMotionActive, bool, (),,
   "@brief Used to determine if the Leap Motion input device is active\n\n"

   "The Leap Motion input device is considered active when the support library has been "
   "loaded and the device has been found.\n\n"

   "@return True if the Leap Motion input device is active.\n"

   "@ingroup Game")
{
   if(!ManagedSingleton<LeapMotionDevice>::instanceOrNull())
   {
      return false;
   }

   return LEAPMOTIONDEV->getActive();
}
