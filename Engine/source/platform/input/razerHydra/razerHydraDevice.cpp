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

#include "platform/input/razerHydra/razerHydraDevice.h"
#include "platform/input/razerHydra/razerHydraData.h"
#include "platform/input/razerHydra/razerHydraConstants.h"
#include "platform/input/razerHydra/razerHydraFrameStore.h"
#include "platform/platformInput.h"
#include "core/module.h"
#include "console/engineAPI.h"
#include "math/mAngAxis.h"
#include "math/mTransform.h"

MODULE_BEGIN( RazerHydraDevice )

   MODULE_INIT_AFTER( InputEventManager )
   MODULE_SHUTDOWN_BEFORE( InputEventManager )

   MODULE_INIT
   {
      RazerHydraDevice::staticInit();
      ManagedSingleton< RazerHydraDevice >::createSingleton();
      if(RazerHydraDevice::smEnableDevice)
      {
         RAZERHYDRADEV->enable();
      }

      // Register the device with the Input Event Manager
      INPUTMGR->registerDevice(RAZERHYDRADEV);
   }
   
   MODULE_SHUTDOWN
   {
      INPUTMGR->unregisterDevice(RAZERHYDRADEV);
      ManagedSingleton< RazerHydraDevice >::deleteSingleton();
   }

MODULE_END;

bool RazerHydraDevice::smEnableDevice = true;

bool RazerHydraDevice::smProcessWhenDocked = false;

bool RazerHydraDevice::smSeparatePositionEvents = true;
bool RazerHydraDevice::smCombinedPositionEvents = false;
bool RazerHydraDevice::smRotationAsAxisEvents = false;

F32 RazerHydraDevice::smMaximumAxisAngle = 25.0;

bool RazerHydraDevice::smGenerateWholeFrameEvents = false;

U32 RazerHydraDevice::RH_DOCKED[RazerHydraConstants::MaxControllers] = {0};
U32 RazerHydraDevice::RH_POSX[RazerHydraConstants::MaxControllers] = {0};
U32 RazerHydraDevice::RH_POSY[RazerHydraConstants::MaxControllers] = {0};
U32 RazerHydraDevice::RH_POSZ[RazerHydraConstants::MaxControllers] = {0};
U32 RazerHydraDevice::RH_POS[RazerHydraConstants::MaxControllers] = {0};
U32 RazerHydraDevice::RH_ROT[RazerHydraConstants::MaxControllers] = {0};
U32 RazerHydraDevice::RH_ROTAXISX[RazerHydraConstants::MaxControllers] = {0};
U32 RazerHydraDevice::RH_ROTAXISY[RazerHydraConstants::MaxControllers] = {0};
U32 RazerHydraDevice::RH_THUMBX[RazerHydraConstants::MaxControllers] = {0};
U32 RazerHydraDevice::RH_THUMBY[RazerHydraConstants::MaxControllers] = {0};
U32 RazerHydraDevice::RH_TRIGGER[RazerHydraConstants::MaxControllers] = {0};
U32 RazerHydraDevice::RH_SHOULDER[RazerHydraConstants::MaxControllers] = {0};
U32 RazerHydraDevice::RH_THUMB[RazerHydraConstants::MaxControllers] = {0};
U32 RazerHydraDevice::RH_START[RazerHydraConstants::MaxControllers] = {0};
U32 RazerHydraDevice::RH_1[RazerHydraConstants::MaxControllers] = {0};
U32 RazerHydraDevice::RH_2[RazerHydraConstants::MaxControllers] = {0};
U32 RazerHydraDevice::RH_3[RazerHydraConstants::MaxControllers] = {0};
U32 RazerHydraDevice::RH_4[RazerHydraConstants::MaxControllers] = {0};
U32 RazerHydraDevice::RH_FRAME = 0;

RazerHydraDevice::RazerHydraDevice()
{
   // From IInputDevice
   dStrcpy(mName, "razerhydra");
   mDeviceType = INPUTMGR->getNextDeviceType();

   //
   mRazerHydraLib = NULL;
   mEnabled = false;
   mActive = false;
   mNumberActiveControllers = 0;
   mLastActiveCheck = 0;

   for(U32 i=0; i<RazerHydraConstants::MaxControllers; ++i)
   {
      for(U32 j=0; j<2; ++j)
      {
         mDataBuffer[i][j] = new RazerHyrdaControllerData();
      }

      mPrevData[i] = mDataBuffer[i][0];
   }

   buildCodeTable();
}

RazerHydraDevice::~RazerHydraDevice()
{
   mRazerHydraLib = NULL;

   // Delete the controller data buffers
   for(U32 i=0; i<RazerHydraConstants::MaxControllers; ++i)
   {
      for(U32 j=0; j<2; ++j)
      {
         delete mDataBuffer[i][j];
         mDataBuffer[i][j] = NULL;
         mPrevData[i] = NULL;
      }
   }
}

void RazerHydraDevice::staticInit()
{
   Con::addVariable("pref::RazerHydra::EnableDevice", TypeBool, &smEnableDevice, 
      "@brief If true, the Razer Hydra device will be enabled, if present.\n\n"
	   "@ingroup Game");

   Con::addVariable("RazerHydra::ProcessWhenDocked", TypeBool, &smProcessWhenDocked, 
      "@brief If true, events will still be sent when a controller is docked.\n\n"
	   "@ingroup Game");

   Con::addVariable("RazerHydra::SeparatePositionEvents", TypeBool, &smSeparatePositionEvents, 
      "@brief If true, separate position events will be sent for each component.\n\n"
	   "@ingroup Game");
   Con::addVariable("RazerHydra::CombinedPositionEvents", TypeBool, &smCombinedPositionEvents, 
      "@brief If true, one position event will be sent that includes one component per argument.\n\n"
	   "@ingroup Game");

   Con::addVariable("RazerHydra::RotationAsAxisEvents", TypeBool, &smRotationAsAxisEvents, 
      "@brief If true, broadcast controller rotation as axis events.\n\n"
	   "@ingroup Game");
   Con::addVariable("RazerHydra::MaximumAxisAngle", TypeF32, &smMaximumAxisAngle, 
      "@brief The maximum controller angle when used as an axis event as measured from a vector pointing straight up (in degrees).\n\n"
      "Shoud range from 0 to 90 degrees.\n\n"
	   "@ingroup Game");

   Con::addVariable("RazerHydra::GenerateWholeFrameEvents", TypeBool, &smGenerateWholeFrameEvents, 
      "@brief Indicates that a whole frame event should be generated and frames should be buffered.\n\n"
	   "@ingroup Game");
}

void RazerHydraDevice::buildCodeTable()
{
   // Obtain all of the device codes
   for(U32 i=0; i<RazerHydraConstants::MaxControllers; ++i)
   {
      RH_DOCKED[i] = INPUTMGR->getNextDeviceCode();

      RH_POSX[i] = INPUTMGR->getNextDeviceCode();
      RH_POSY[i] = INPUTMGR->getNextDeviceCode();
      RH_POSZ[i] = INPUTMGR->getNextDeviceCode();

      RH_POS[i] = INPUTMGR->getNextDeviceCode();

      RH_ROT[i] = INPUTMGR->getNextDeviceCode();

      RH_ROTAXISX[i] = INPUTMGR->getNextDeviceCode();
      RH_ROTAXISY[i] = INPUTMGR->getNextDeviceCode();

      RH_THUMBX[i] = INPUTMGR->getNextDeviceCode();
      RH_THUMBY[i] = INPUTMGR->getNextDeviceCode();

      RH_TRIGGER[i] = INPUTMGR->getNextDeviceCode();

      RH_SHOULDER[i] = INPUTMGR->getNextDeviceCode();
      RH_THUMB[i] = INPUTMGR->getNextDeviceCode();
      RH_START[i] = INPUTMGR->getNextDeviceCode();
      RH_1[i] = INPUTMGR->getNextDeviceCode();
      RH_2[i] = INPUTMGR->getNextDeviceCode();
      RH_3[i] = INPUTMGR->getNextDeviceCode();
      RH_4[i] = INPUTMGR->getNextDeviceCode();
   }

   RH_FRAME = INPUTMGR->getNextDeviceCode();

   // Build out the virtual map
   char buffer[64];
   for(U32 i=0; i<RazerHydraConstants::MaxControllers; ++i)
   {
      dSprintf(buffer, 64, "rh_docked%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_BUTTON, RH_DOCKED[i] );

      dSprintf(buffer, 64, "rh_posx%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_FLOAT, RH_POSX[i] );
      dSprintf(buffer, 64, "rh_posy%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_FLOAT, RH_POSY[i] );
      dSprintf(buffer, 64, "rh_posz%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_FLOAT, RH_POSZ[i] );

      dSprintf(buffer, 64, "rh_pos%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_POS, RH_POS[i] );

      dSprintf(buffer, 64, "rh_rot%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_ROT, RH_ROT[i] );

      dSprintf(buffer, 64, "rh_rotaxisx%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_AXIS, RH_ROTAXISX[i] );
      dSprintf(buffer, 64, "rh_rotaxisy%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_AXIS, RH_ROTAXISY[i] );

      dSprintf(buffer, 64, "rh_thumbx%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_AXIS, RH_THUMBX[i] );
      dSprintf(buffer, 64, "rh_thumby%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_AXIS, RH_THUMBY[i] );

      dSprintf(buffer, 64, "rh_trigger%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_AXIS, RH_TRIGGER[i] );

      dSprintf(buffer, 64, "rh_shoulder%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_BUTTON, RH_SHOULDER[i] );

      dSprintf(buffer, 64, "rh_thumb%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_BUTTON, RH_THUMB[i] );

      dSprintf(buffer, 64, "rh_start%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_BUTTON, RH_START[i] );

      dSprintf(buffer, 64, "rh_1button%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_BUTTON, RH_1[i] );
      dSprintf(buffer, 64, "rh_2button%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_BUTTON, RH_2[i] );
      dSprintf(buffer, 64, "rh_3button%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_BUTTON, RH_3[i] );
      dSprintf(buffer, 64, "rh_4button%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_BUTTON, RH_4[i] );
   }

   AddInputVirtualMap(  rh_frame,      SI_INT,     RH_FRAME );
}

bool RazerHydraDevice::enable()
{
   // Start off with disabling the device if it is already enabled
   disable();

   // Dynamically load the Razer Hydra library and set up function pointers
#ifdef LOG_INPUT
   Input::log( "Enabling Razer Hydra...\n" );
#endif

   const char* dllName;
#ifdef TORQUE_OS_WIN32
   #ifdef TORQUE_DEBUG
      dllName = "sixensed.dll";
   #else
      dllName = "sixense.dll";
   #endif
#else
   #ifdef LOG_INPUT
      Input::log( "...platform not supported for Razer Hydra\n" );
   #endif
      return;
#endif

   mRazerHydraLib = OsLoadLibrary( dllName );
   if(mRazerHydraLib)
   {
#ifdef LOG_INPUT
      Input::log( "Razer Hydra library loaded.\n" );
#endif
      Con::printf("Razer Hydra Init:");

      // Obtain library function pointers
      mfnSixenseInit = (FN_SixenseInit) mRazerHydraLib->bind( "sixenseInit" );
      mfnSixenseExit = (FN_SixenseExit) mRazerHydraLib->bind( "sixenseExit" );

      mfnSixenseGetMaxBases = (FN_SixenseGetMaxBases) mRazerHydraLib->bind( "sixenseGetMaxBases" );
      mfnSixenseSetActiveBase = (FN_SixenseSetActiveBase) mRazerHydraLib->bind( "sixenseSetActiveBase" );
      mfnSixenseIsBaseConnected = (FN_SixenseIsBaseConnected) mRazerHydraLib->bind( "sixenseIsBaseConnected" );

      mfnSixenseGetMaxControllers = (FN_SixenseGetMaxControllers) mRazerHydraLib->bind( "sixenseGetMaxControllers" );
      mfnSixenseIsControllerEnabled = (FN_SixenseIsControllerEnabled) mRazerHydraLib->bind( "sixenseIsControllerEnabled" );
      mfnSixenseGetNumActiveControllers = (FN_SixenseGetNumActiveControllers) mRazerHydraLib->bind( "sixenseGetNumActiveControllers" );

      mfnSixenseGetHistorySize = (FN_SixenseGetHistorySize) mRazerHydraLib->bind( "sixenseGetHistorySize" );

      mfnSixenseGetData = (FN_SixenseGetData) mRazerHydraLib->bind( "sixenseGetData" );
      mfnSixenseGetAllData = (FN_SixenseGetAllData) mRazerHydraLib->bind( "sixenseGetAllData" );
      mfnSixenseGetNewestData = (FN_SixenseGetNewestData) mRazerHydraLib->bind( "sixenseGetNewestData" );
      mfnSixenseGetAllNewestData = (FN_SixenseGetAllNewestData) mRazerHydraLib->bind( "sixenseGetAllNewestData" );

      mfnSixenseSetHemisphereTrackingMode = (FN_SixenseSetHemisphereTrackingMode) mRazerHydraLib->bind( "sixenseSetHemisphereTrackingMode" );
      mfnSixenseGetHemisphereTrackingMode = (FN_SixenseGetHemisphereTrackingMode) mRazerHydraLib->bind( "sixenseGetHemisphereTrackingMode" );

      mfnSixenseAutoEnableHemisphereTracking = (FN_SixenseAutoEnableHemisphereTracking) mRazerHydraLib->bind( "sixenseAutoEnableHemisphereTracking" );

      mfnSixenseSetHighPriorityBindingEnabled = (FN_SixenseSetHighPriorityBindingEnabled) mRazerHydraLib->bind( "sixenseSetHighPriorityBindingEnabled" );
      mfnSixenseGetHighPriorityBindingEnabled = (FN_SixenseGetHighPriorityBindingEnabled) mRazerHydraLib->bind( "sixenseGetHighPriorityBindingEnabled" );

      mfnSixenseTriggerVibration = (FN_SixenseTriggerVibration) mRazerHydraLib->bind( "sixenseTriggerVibration" );

      mfnSixenseSetFilterEnabled = (FN_SixenseSetFilterEnabled) mRazerHydraLib->bind( "sixenseSetFilterEnabled" );
      mfnSixenseGetFilterEnabled = (FN_SixenseGetFilterEnabled) mRazerHydraLib->bind( "sixenseGetFilterEnabled" );

      mfnSixenseSetFilterParams = (FN_SixenseSetFilterParams) mRazerHydraLib->bind( "sixenseSetFilterParams" );
      mfnSixenseGetFilterParams = (FN_SixenseGetFilterParams) mRazerHydraLib->bind( "sixenseGetFilterParams" );

      mfnSixenseSetBaseColor = (FN_SixenseSetBaseColor) mRazerHydraLib->bind( "sixenseSetBaseColor" );
      mfnSixenseGetBaseColor = (FN_SixenseGetBaseColor) mRazerHydraLib->bind( "sixenseGetBaseColor" );

      // Init the sixense library
      S32 result = mfnSixenseInit();
      if(result == SIXENSE_FAILURE)
      {
         // Problem with starting the library
         Con::printf("   Sixense library startup failure");
         mRazerHydraLib = NULL;

         mEnabled = false;
         mActive = false;

         return false;
      }

      // Retrieve some information about the Hydra
      mMaximumBases = mfnSixenseGetMaxBases();
      Con::printf("   Max bases: %d", mMaximumBases);
      mMaximumControllers = mfnSixenseGetMaxControllers();
      Con::printf("   Max controllers: %d", mMaximumControllers);

      mEnabled = true;

      if(checkControllers())
      {
         Con::printf("   Active controllers: %d", mNumberActiveControllers);
         mActive = true;
      }
      else
      {
         Con::printf("   Controllers not yet found.  Starting to poll.");
         mLastActiveCheck = Platform::getRealMilliseconds();
         mActive = false;
      }

      Con::printf("");

      return true;
   }
   else
   {
#ifdef LOG_INPUT
      Input::log( "Razer Hydra library was not found.\n" );
#endif
      Con::errorf("Razer Hydra library was not found.");

      mEnabled = false;
      mActive = false;
   }

   return false;
}

void RazerHydraDevice::disable()
{
   if(mRazerHydraLib)
   {
      // Shutdown the sixense library
      mfnSixenseExit();

      mRazerHydraLib = NULL;
   }

   mEnabled = false;
}

bool RazerHydraDevice::process()
{
   if(!mEnabled)
      return false;

   //Con::printf("RazerHydraDevice::process()");

   if(!mActive)
   {
      // Only perform a check on a periodic basis
      S32 time = Platform::getRealMilliseconds();
      if((time - mLastActiveCheck) < RazerHydraConstants::HydraActiveCheckFreq)
         return false;

      mLastActiveCheck = time;

      if(checkControllers())
      {
         Con::printf("Razer Hydra now has %d active controllers", mNumberActiveControllers);
         mActive = true;
      }
      else
      {
         return false;
      }
   }
   else if(!checkControllers())
   {
      // We no longer have active controllers
      Con::printf("Razer Hydra now has NO active controllers");
      mLastActiveCheck = Platform::getRealMilliseconds();
      mActive = false;
      return false;
   }

   //Build the maximum axis angle to be passed into the RazerHyrdaControllerData::setData()
   F32 maxAxisRadius = mSin(mDegToRad(smMaximumAxisAngle));

   // Get the controller data
   mfnSixenseSetActiveBase(RazerHydraConstants::DefaultHydraBase);
   sixenseAllControllerData acd;
   mfnSixenseGetAllNewestData(&acd);

   // Store the current data from each controller and compare with previous data
   U32 diff[RazerHydraConstants::MaxControllers];
   U32 metaDiff[RazerHydraConstants::MaxControllers];
   RazerHyrdaControllerData* currentBuffer[RazerHydraConstants::MaxControllers];
   for(U32 i=0; i<RazerHydraConstants::MaxControllers; ++i)
   {
      currentBuffer[i] = (mPrevData[i] == mDataBuffer[i][0]) ? mDataBuffer[i][1] : mDataBuffer[i][0];
      currentBuffer[i]->setData(acd.controllers[i], maxAxisRadius);
      diff[i] = mPrevData[i]->compare(currentBuffer[i]);
      metaDiff[i] = mPrevData[i]->compareMeta(currentBuffer[i]);
   }

   // Update the previous data pointers.  We do this here in case someone calls our
   // console functions during one of the input events below.
   mPrevData[0] = currentBuffer[0];
   mPrevData[1] = currentBuffer[1];

   // Process each controller's meta data.
   for(U32 i=0; i<RazerHydraConstants::MaxControllers; ++i)
   {
      // Docked
      if(metaDiff[i] != RazerHyrdaControllerData::METADIFF_NONE)
      {
         INPUTMGR->buildInputEvent(mDeviceType, RazerHydraConstants::DefaultHydraBase, SI_BUTTON, RH_DOCKED[i], currentBuffer[i]->mIsDocked ? SI_MAKE : SI_BREAK, currentBuffer[i]->mIsDocked ? 1.0f : 0.0f);
      }

      // Position, rotation and buttons
      if(diff[i] != RazerHyrdaControllerData::DIFF_NONE && (!currentBuffer[i]->mIsDocked || smProcessWhenDocked && currentBuffer[i]->mIsDocked))
      {
         // Position
         if(smSeparatePositionEvents)
         {
            if(diff[i] & RazerHyrdaControllerData::DIFF_POSX)
               INPUTMGR->buildInputEvent(mDeviceType, RazerHydraConstants::DefaultHydraBase, SI_FLOAT, RH_POSX[i], SI_MOVE, currentBuffer[i]->mPos[0]);
            if(diff[i] & RazerHyrdaControllerData::DIFF_POSY)
               INPUTMGR->buildInputEvent(mDeviceType, RazerHydraConstants::DefaultHydraBase, SI_FLOAT, RH_POSY[i], SI_MOVE, currentBuffer[i]->mPos[1]);
            if(diff[i] & RazerHyrdaControllerData::DIFF_POSZ)
               INPUTMGR->buildInputEvent(mDeviceType, RazerHydraConstants::DefaultHydraBase, SI_FLOAT, RH_POSZ[i], SI_MOVE, currentBuffer[i]->mPos[2]);
         }
         if(smCombinedPositionEvents)
         {
            if(diff[i] & RazerHyrdaControllerData::DIFF_POSX || diff[i] & RazerHyrdaControllerData::DIFF_POSY || diff[i] & RazerHyrdaControllerData::DIFF_POSZ)
            {
               INPUTMGR->buildInputEvent(mDeviceType, RazerHydraConstants::DefaultHydraBase, SI_POS, RH_POS[i], SI_MOVE, currentBuffer[i]->mPosPoint);
            }
         }

         // Rotation
         if(diff[i] & RazerHyrdaControllerData::DIFF_ROT)
            INPUTMGR->buildInputEvent(mDeviceType, RazerHydraConstants::DefaultHydraBase, SI_ROT, RH_ROT[i], SI_MOVE, currentBuffer[i]->mRotQuat);

         // Thumb stick
         if(diff[i] & RazerHyrdaControllerData::DIFF_AXISX)
            INPUTMGR->buildInputEvent(mDeviceType, RazerHydraConstants::DefaultHydraBase, SI_AXIS, RH_THUMBX[i], SI_MOVE, currentBuffer[i]->mThumbStick[0]);
         if(diff[i] & RazerHyrdaControllerData::DIFF_AXISY)
            INPUTMGR->buildInputEvent(mDeviceType, RazerHydraConstants::DefaultHydraBase, SI_AXIS, RH_THUMBY[i], SI_MOVE, currentBuffer[i]->mThumbStick[1]);

         // Trigger
         if(diff[i] & RazerHyrdaControllerData::DIFF_TRIGGER)
            INPUTMGR->buildInputEvent(mDeviceType, RazerHydraConstants::DefaultHydraBase, SI_AXIS, RH_TRIGGER[i], SI_MOVE, currentBuffer[i]->mTrigger);

         // Shoulder button
         if(diff[i] & RazerHyrdaControllerData::DIFF_BUTTON_SHOULDER)
            INPUTMGR->buildInputEvent(mDeviceType, RazerHydraConstants::DefaultHydraBase, SI_BUTTON, RH_SHOULDER[i], currentBuffer[i]->mShoulder ? SI_MAKE : SI_BREAK, currentBuffer[i]->mShoulder ? 1.0f : 0.0f);

         // Thumb button
         if(diff[i] & RazerHyrdaControllerData::DIFF_BUTTON_THUMB)
            INPUTMGR->buildInputEvent(mDeviceType, RazerHydraConstants::DefaultHydraBase, SI_BUTTON, RH_THUMB[i], currentBuffer[i]->mThumb ? SI_MAKE : SI_BREAK, currentBuffer[i]->mThumb ? 1.0f : 0.0f);

         // Start button
         if(diff[i] & RazerHyrdaControllerData::DIFF_BUTTON_START)
            INPUTMGR->buildInputEvent(mDeviceType, RazerHydraConstants::DefaultHydraBase, SI_BUTTON, RH_START[i], currentBuffer[i]->mStart ? SI_MAKE : SI_BREAK, currentBuffer[i]->mStart ? 1.0f : 0.0f);

         // Button 1
         if(diff[i] & RazerHyrdaControllerData::DIFF_BUTTON1)
            INPUTMGR->buildInputEvent(mDeviceType, RazerHydraConstants::DefaultHydraBase, SI_BUTTON, RH_1[i], currentBuffer[i]->mButton1 ? SI_MAKE : SI_BREAK, currentBuffer[i]->mButton1 ? 1.0f : 0.0f);

         // Button 2
         if(diff[i] & RazerHyrdaControllerData::DIFF_BUTTON2)
            INPUTMGR->buildInputEvent(mDeviceType, RazerHydraConstants::DefaultHydraBase, SI_BUTTON, RH_2[i], currentBuffer[i]->mButton2 ? SI_MAKE : SI_BREAK, currentBuffer[i]->mButton2 ? 1.0f : 0.0f);

         // Button 3
         if(diff[i] & RazerHyrdaControllerData::DIFF_BUTTON3)
            INPUTMGR->buildInputEvent(mDeviceType, RazerHydraConstants::DefaultHydraBase, SI_BUTTON, RH_3[i], currentBuffer[i]->mButton3 ? SI_MAKE : SI_BREAK, currentBuffer[i]->mButton3 ? 1.0f : 0.0f);

         // Button 4
         if(diff[i] & RazerHyrdaControllerData::DIFF_BUTTON4)
            INPUTMGR->buildInputEvent(mDeviceType, RazerHydraConstants::DefaultHydraBase, SI_BUTTON, RH_4[i], currentBuffer[i]->mButton4 ? SI_MAKE : SI_BREAK, currentBuffer[i]->mButton4 ? 1.0f : 0.0f);
      }

      // Left rotation as axis.  Done here as we still need to send events even when docked.
      if(smRotationAsAxisEvents && diff[i] & RazerHyrdaControllerData::DIFF_ROTAXIS)
      {
         if(diff[i] & RazerHyrdaControllerData::DIFF_ROTAXISX)
            INPUTMGR->buildInputEvent(mDeviceType, RazerHydraConstants::DefaultHydraBase, SI_AXIS, RH_ROTAXISX[i], SI_MOVE, currentBuffer[i]->mRotAxis.x);
         if(diff[i] & RazerHyrdaControllerData::DIFF_ROTAXISY)
            INPUTMGR->buildInputEvent(mDeviceType, RazerHydraConstants::DefaultHydraBase, SI_AXIS, RH_ROTAXISY[i], SI_MOVE, currentBuffer[i]->mRotAxis.y);
      }
   }

   // Send out whole frame event, but only if the special frame group is defined
   if(smGenerateWholeFrameEvents && RazerHydraFrameStore::isFrameGroupDefined())
   {
      S32 id = RAZERHYDRAFS->generateNewFrame(acd, maxAxisRadius);
      if(id != 0)
      {
         INPUTMGR->buildInputEvent(mDeviceType, RazerHydraConstants::DefaultHydraBase, SI_INT, RH_FRAME, SI_VALUE, id);
      }
   }

   return true;
}

bool RazerHydraDevice::checkControllers()
{
   if(!mEnabled)
      return false;

   bool hasBase = (mfnSixenseIsBaseConnected(RazerHydraConstants::DefaultHydraBase) == 1);
   if(!hasBase)
   {
      mNumberActiveControllers = 0;
      return false;
   }

   mfnSixenseSetActiveBase(RazerHydraConstants::DefaultHydraBase);
   mNumberActiveControllers = mfnSixenseGetNumActiveControllers();
   if(mNumberActiveControllers < 1)
      return false;

   return true;
}

bool RazerHydraDevice::isControllerDocked(U32 controller)
{
   if(!mEnabled || !mActive)
      return true;

   if(controller >= RazerHydraConstants::MaxControllers)
      return true;

   // Results are based on the last retrieved data from the device
   return mPrevData[controller]->mIsDocked || !mPrevData[controller]->mDataSet;
}

const Point3F& RazerHydraDevice::getControllerPosition(U32 controller)
{
   if(!mEnabled || !mActive)
      return Point3F::Zero;

   if(controller >= RazerHydraConstants::MaxControllers)
      return Point3F::Zero;

   // Results are based on the last retrieved data from the device
   return mPrevData[controller]->mPosPoint;
}

const QuatF& RazerHydraDevice::getControllerRotation(U32 controller)
{
   if(!mEnabled || !mActive)
      return QuatF::Identity;

   if(controller >= RazerHydraConstants::MaxControllers)
      return QuatF::Identity;

   // Results are based on the last retrieved data from the device
   return mPrevData[controller]->mRotQuat;
}

//-----------------------------------------------------------------------------

DefineEngineFunction(isRazerHydraActive, bool, (),,
   "@brief Used to determine if the Razer Hydra input device active\n\n"

   "The Razer Hydra input device is considered active when the support library has been "
   "loaded and the controller has been found.\n\n"

   "@return True if the Razer Hydra input device is active.\n"

   "@ingroup Game")
{
   if(!ManagedSingleton<RazerHydraDevice>::instanceOrNull())
   {
      return false;
   }

   return RAZERHYDRADEV->isActive();
}

DefineEngineFunction(isRazerHydraControllerDocked, bool, (S32 controller),,
   "@brief Used to determine if the given Razer Hydra controller is docked\n\n"

   "@param controller Controller number to check.\n"

   "@return True if the given Razer Hydra controller is docked.  Also returns true if "
   "the input device is not found or active.\n"

   "@ingroup Game")
{
   if(!ManagedSingleton<RazerHydraDevice>::instanceOrNull())
   {
      return true;
   }

   return RAZERHYDRADEV->isControllerDocked(controller);
}

DefineEngineFunction(getRazerHydraControllerPos, Point3F, (S32 controller),,
   "@brief Get the given Razer Hydra controller's last position\n\n"

   "@param controller Controller number to check.\n"

   "@return A Point3F containing the last known position.\n"

   "@ingroup Game")
{
   if(!ManagedSingleton<RazerHydraDevice>::instanceOrNull())
   {
      return Point3F::Zero;
   }

   return RAZERHYDRADEV->getControllerPosition(controller);
}

DefineEngineFunction(getRazerHydraControllerRot, AngAxisF, (S32 controller),,
   "@brief Get the given Razer Hydra controller's last rotation\n\n"

   "@param controller Controller number to check.\n"

   "@return A AngAxisF containing the last known rotation.\n"

   "@ingroup Game")
{
   AngAxisF aa(Point3F(0, 0, 1), 0);

   if(!ManagedSingleton<RazerHydraDevice>::instanceOrNull())
   {
      return aa;
   }

   const QuatF& qa = RAZERHYDRADEV->getControllerRotation(controller);
   aa.set(qa);
   aa.axis.normalize();

   return aa;
}

DefineEngineFunction(getRazerHydraControllerTransform, TransformF, (S32 controller),,
   "@brief Get the given Razer Hydra controller's last transform\n\n"

   "@param controller Controller number to check.\n"

   "@return A TransformF containing the last known transform.\n"

   "@ingroup Game")
{
   TransformF trans;

   if(!ManagedSingleton<RazerHydraDevice>::instanceOrNull())
   {
      return trans;
   }

   const Point3F& pos = RAZERHYDRADEV->getControllerPosition(controller);
   const QuatF& qa = RAZERHYDRADEV->getControllerRotation(controller);

   AngAxisF aa(qa);
   aa.axis.normalize();
   trans.set(pos, aa);

   return trans;
}
