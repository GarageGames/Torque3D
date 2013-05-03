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

#ifndef _RAZERHYDRADEVICE_H_
#define _RAZERHYDRADEVICE_H_

#include "platform/input/razerHydra/razerHydraConstants.h"
#include "platform/platformDlibrary.h"
#include "platform/input/IInputDevice.h"
#include "platform/input/event.h"
#include "core/util/tSingleton.h"
#include "math/mQuat.h"
#include "sixense.h"

#define HYDRA_ACTIVE_CHECK_FREQ 1000

#define FN_HYDRA __cdecl

// Library function typedefs
typedef int (FN_HYDRA* FN_SixenseInit)();
typedef int (FN_HYDRA* FN_SixenseExit)();

typedef int (FN_HYDRA* FN_SixenseGetMaxBases)();
typedef int (FN_HYDRA* FN_SixenseSetActiveBase)(int base_num);
typedef int (FN_HYDRA* FN_SixenseIsBaseConnected)(int base_num);

typedef int (FN_HYDRA* FN_SixenseGetMaxControllers)();
typedef int (FN_HYDRA* FN_SixenseIsControllerEnabled)(int which);
typedef int (FN_HYDRA* FN_SixenseGetNumActiveControllers)();

typedef int (FN_HYDRA* FN_SixenseGetHistorySize)();

typedef int (FN_HYDRA* FN_SixenseGetData)(int which, int index_back, sixenseControllerData *);
typedef int (FN_HYDRA* FN_SixenseGetAllData)(int index_back, sixenseAllControllerData *);
typedef int (FN_HYDRA* FN_SixenseGetNewestData)(int which, sixenseControllerData *);
typedef int (FN_HYDRA* FN_SixenseGetAllNewestData)(sixenseAllControllerData *);

typedef int (FN_HYDRA* FN_SixenseSetHemisphereTrackingMode)(int which_controller, int state);
typedef int (FN_HYDRA* FN_SixenseGetHemisphereTrackingMode)(int which_controller, int *state);

typedef int (FN_HYDRA* FN_SixenseAutoEnableHemisphereTracking)(int which_controller);

typedef int (FN_HYDRA* FN_SixenseSetHighPriorityBindingEnabled)(int on_or_off);
typedef int (FN_HYDRA* FN_SixenseGetHighPriorityBindingEnabled)(int *on_or_off);

typedef int (FN_HYDRA* FN_SixenseTriggerVibration)(int controller_id, int duration_100ms, int pattern_id);

typedef int (FN_HYDRA* FN_SixenseSetFilterEnabled)(int on_or_off);
typedef int (FN_HYDRA* FN_SixenseGetFilterEnabled)(int *on_or_off);

typedef int (FN_HYDRA* FN_SixenseSetFilterParams)(float near_range, float near_val, float far_range, float far_val);
typedef int (FN_HYDRA* FN_SixenseGetFilterParams)(float *near_range, float *near_val, float *far_range, float *far_val);

typedef int (FN_HYDRA* FN_SixenseSetBaseColor)(unsigned char red, unsigned char green, unsigned char blue);
typedef int (FN_HYDRA* FN_SixenseGetBaseColor)(unsigned char *red, unsigned char *green, unsigned char *blue);

struct RazerHyrdaControllerData;

class RazerHydraDevice : public IInputDevice
{
public:
   static bool smEnableDevice;

   // Should events be sent when a controller is docked
   static bool smProcessWhenDocked;

   // The type of position events to broadcast
   static bool smSeparatePositionEvents;
   static bool smCombinedPositionEvents;

   // Broadcast controller rotation as axis
   static bool smRotationAsAxisEvents;

   // The maximum controller angle when used as an axis event
   // as measured from a vector pointing straight up (in degrees)
   static F32 smMaximumAxisAngle;

   // Indicates that a whole frame event should be generated and frames
   // should be buffered.
   static bool smGenerateWholeFrameEvents;

   // Controller Action Codes
   static U32 RH_DOCKED[RazerHydraConstants::MaxControllers];     // SI_BUTTON

   static U32 RH_POSX[RazerHydraConstants::MaxControllers];       // SI_FLOAT
   static U32 RH_POSY[RazerHydraConstants::MaxControllers];
   static U32 RH_POSZ[RazerHydraConstants::MaxControllers];

   static U32 RH_POS[RazerHydraConstants::MaxControllers];        // SI_POS

   static U32 RH_ROT[RazerHydraConstants::MaxControllers];        // SI_ROT

   static U32 RH_ROTAXISX[RazerHydraConstants::MaxControllers];   // SI_AXIS
   static U32 RH_ROTAXISY[RazerHydraConstants::MaxControllers];

   static U32 RH_THUMBX[RazerHydraConstants::MaxControllers];     // SI_AXIS
   static U32 RH_THUMBY[RazerHydraConstants::MaxControllers];

   static U32 RH_TRIGGER[RazerHydraConstants::MaxControllers];    // SI_AXIS

   static U32 RH_SHOULDER[RazerHydraConstants::MaxControllers];   // SI_BUTTON
   static U32 RH_THUMB[RazerHydraConstants::MaxControllers];      // SI_BUTTON
   static U32 RH_START[RazerHydraConstants::MaxControllers];      // SI_BUTTON
   static U32 RH_1[RazerHydraConstants::MaxControllers];          // SI_BUTTON
   static U32 RH_2[RazerHydraConstants::MaxControllers];          // SI_BUTTON
   static U32 RH_3[RazerHydraConstants::MaxControllers];          // SI_BUTTON
   static U32 RH_4[RazerHydraConstants::MaxControllers];          // SI_BUTTON

   // Whole frame
   static U32 RH_FRAME;       // SI_INT

public:
   RazerHydraDevice();
   ~RazerHydraDevice();

   static void staticInit();

   bool enable();

   void disable();

   bool process();

   bool isActive() { return mActive; }

   bool isControllerDocked(U32 controller);

   const Point3F& getControllerPosition(U32 controller);

   const QuatF& getControllerRotation(U32 controller);

protected:
   DLibraryRef mRazerHydraLib;

   FN_SixenseInit                      mfnSixenseInit;
   FN_SixenseExit                      mfnSixenseExit;

   FN_SixenseGetMaxBases               mfnSixenseGetMaxBases;
   FN_SixenseSetActiveBase             mfnSixenseSetActiveBase;
   FN_SixenseIsBaseConnected           mfnSixenseIsBaseConnected;

   FN_SixenseGetMaxControllers         mfnSixenseGetMaxControllers;
   FN_SixenseIsControllerEnabled       mfnSixenseIsControllerEnabled;
   FN_SixenseGetNumActiveControllers   mfnSixenseGetNumActiveControllers;

   FN_SixenseGetHistorySize            mfnSixenseGetHistorySize;

   FN_SixenseGetData                   mfnSixenseGetData;
   FN_SixenseGetAllData                mfnSixenseGetAllData;
   FN_SixenseGetNewestData             mfnSixenseGetNewestData;
   FN_SixenseGetAllNewestData          mfnSixenseGetAllNewestData;

   FN_SixenseSetHemisphereTrackingMode mfnSixenseSetHemisphereTrackingMode;
   FN_SixenseGetHemisphereTrackingMode mfnSixenseGetHemisphereTrackingMode;

   FN_SixenseAutoEnableHemisphereTracking mfnSixenseAutoEnableHemisphereTracking;

   FN_SixenseSetHighPriorityBindingEnabled   mfnSixenseSetHighPriorityBindingEnabled;
   FN_SixenseGetHighPriorityBindingEnabled   mfnSixenseGetHighPriorityBindingEnabled;

   FN_SixenseTriggerVibration          mfnSixenseTriggerVibration;

   FN_SixenseSetFilterEnabled          mfnSixenseSetFilterEnabled;
   FN_SixenseGetFilterEnabled          mfnSixenseGetFilterEnabled;

   FN_SixenseSetFilterParams           mfnSixenseSetFilterParams;
   FN_SixenseGetFilterParams           mfnSixenseGetFilterParams;

   FN_SixenseSetBaseColor              mfnSixenseSetBaseColor;
   FN_SixenseGetBaseColor              mfnSixenseGetBaseColor;

   S32 mMaximumBases;
   S32 mMaximumControllers;

   S32 mNumberActiveControllers;

   /// Is the Razer Hydra active (enabled means the library is loaded)
   bool mActive;

   /// When was the last check for an active Hydra
   S32 mLastActiveCheck;

   /// Buffers to store data for the controllers
   RazerHyrdaControllerData*  mDataBuffer[RazerHydraConstants::MaxControllers][2];

   /// Points to the buffers that holds the previously collected data
   /// for each controller
   RazerHyrdaControllerData*  mPrevData[RazerHydraConstants::MaxControllers];

protected:
   /// Build out the codes used for controller actions with the
   /// Input Event Manager
   void buildCodeTable();

   /// Checks if there are active controllers
   bool checkControllers();

public:
   // For ManagedSingleton.
   static const char* getSingletonName() { return "RazerHydraDevice"; }   
};

/// Returns the RazerHydraDevice singleton.
#define RAZERHYDRADEV ManagedSingleton<RazerHydraDevice>::instance()

#endif   // _RAZERHYDRADEVICE_H_
