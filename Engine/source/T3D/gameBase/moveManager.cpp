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

#include "T3D/gameBase/moveManager.h"
#include "core/stream/bitStream.h"
#include "core/module.h"
#include "console/consoleTypes.h"
#include "core/strings/stringFunctions.h"
#include "math/mConstants.h"


MODULE_BEGIN( MoveManager )

   MODULE_INIT
   {
      MoveManager::init();
   }

MODULE_END;


bool MoveManager::mDeviceIsKeyboardMouse = false;
F32 MoveManager::mForwardAction = 0;
F32 MoveManager::mBackwardAction = 0;
F32 MoveManager::mUpAction = 0;
F32 MoveManager::mDownAction = 0;
F32 MoveManager::mLeftAction = 0;
F32 MoveManager::mRightAction = 0;

bool MoveManager::mFreeLook = false;
F32 MoveManager::mPitch = 0;
F32 MoveManager::mYaw = 0;
F32 MoveManager::mRoll = 0;

F32 MoveManager::mPitchUpSpeed = 0;
F32 MoveManager::mPitchDownSpeed = 0;
F32 MoveManager::mYawLeftSpeed = 0;
F32 MoveManager::mYawRightSpeed = 0;
F32 MoveManager::mRollLeftSpeed = 0;
F32 MoveManager::mRollRightSpeed = 0;

F32 MoveManager::mXAxis_L = 0;
F32 MoveManager::mYAxis_L = 0;
F32 MoveManager::mXAxis_R = 0;
F32 MoveManager::mYAxis_R = 0;

U32 MoveManager::mTriggerCount[MaxTriggerKeys] = { 0, };
U32 MoveManager::mPrevTriggerCount[MaxTriggerKeys] = { 0, };

const Move NullMove;

void MoveManager::init()
{
   Con::addVariable("mvForwardAction", TypeF32, &mForwardAction, 
      "Forwards movement speed for the active player.\n"
	   "@ingroup Game");
   Con::addVariable("mvBackwardAction", TypeF32, &mBackwardAction, 
      "Backwards movement speed for the active player.\n"
	   "@ingroup Game");
   Con::addVariable("mvUpAction", TypeF32, &mUpAction, 
      "Upwards movement speed for the active player.\n"
	   "@ingroup Game");
   Con::addVariable("mvDownAction", TypeF32, &mDownAction, 
      "Downwards movement speed for the active player.\n"
	   "@ingroup Game");
   Con::addVariable("mvLeftAction", TypeF32, &mLeftAction, 
      "Left movement speed for the active player.\n"
	   "@ingroup Game");
   Con::addVariable("mvRightAction", TypeF32, &mRightAction, 
      "Right movement speed for the active player.\n"
	   "@ingroup Game");

   Con::addVariable("mvFreeLook", TypeBool, &mFreeLook, 
      "Boolean state for if freelook is active or not.\n"
	   "@ingroup Game");
   Con::addVariable("mvDeviceIsKeyboardMouse", TypeBool, &mDeviceIsKeyboardMouse, 
      "Boolean state for it the system is using a keyboard and mouse or not.\n"
	   "@ingroup Game");
   Con::addVariable("mvPitch", TypeF32, &mPitch, 
      "Current pitch value, typically applied through input devices, such as a mouse.\n"
	   "@ingroup Game");
   Con::addVariable("mvYaw", TypeF32, &mYaw, 
      "Current yaw value, typically applied through input devices, such as a mouse.\n"
	   "@ingroup Game");
   Con::addVariable("mvRoll", TypeF32, &mRoll, 
      "Current roll value, typically applied through input devices, such as a mouse.\n"
	   "@ingroup Game");
   Con::addVariable("mvPitchUpSpeed", TypeF32, &mPitchUpSpeed, 
      "Upwards pitch speed.\n"
	   "@ingroup Game");
   Con::addVariable("mvPitchDownSpeed", TypeF32, &mPitchDownSpeed, 
      "Downwards pitch speed.\n"
	   "@ingroup Game");
   Con::addVariable("mvYawLeftSpeed", TypeF32, &mYawLeftSpeed, 
      "Left Yaw speed.\n"
	   "@ingroup Game");
   Con::addVariable("mvYawRightSpeed", TypeF32, &mYawRightSpeed, 
      "Right Yaw speed.\n"
	   "@ingroup Game");
   Con::addVariable("mvRollLeftSpeed", TypeF32, &mRollLeftSpeed, 
      "Left roll speed.\n"
	   "@ingroup Game");
   Con::addVariable("mvRollRightSpeed", TypeF32, &mRollRightSpeed, 
      "Right roll speed.\n"
	   "@ingroup Game");

   // Dual-analog
   Con::addVariable( "mvXAxis_L", TypeF32, &mXAxis_L, 
      "Left thumbstick X axis position on a dual-analog gamepad.\n"
	   "@ingroup Game" );
   Con::addVariable( "mvYAxis_L", TypeF32, &mYAxis_L, 
      "Left thumbstick Y axis position on a dual-analog gamepad.\n"
	   "@ingroup Game" );

   Con::addVariable( "mvXAxis_R", TypeF32, &mXAxis_R, 
      "Right thumbstick X axis position on a dual-analog gamepad.\n"
	   "@ingroup Game" );
   Con::addVariable( "mvYAxis_R", TypeF32, &mYAxis_R, 
      "Right thumbstick Y axis position on a dual-analog gamepad.\n"
	   "@ingroup Game");

   for(U32 i = 0; i < MaxTriggerKeys; i++)
   {
      char varName[256];
      dSprintf(varName, sizeof(varName), "mvTriggerCount%d", i);
      Con::addVariable(varName, TypeS32, &mTriggerCount[i], 
         "Used to determine the trigger counts of buttons. Namely used for input actions such as jumping and weapons firing.\n"
	      "@ingroup Game");
   }
}

Move::Move()
{
   px=16; py=16; pz=16;
   pyaw=0; ppitch=0; proll=0;
   x=0; y=0; z=0;
   yaw=0; pitch=0; roll=0;
   id=0;
   sendCount=0;

   checksum = false;
   deviceIsKeyboardMouse = false;
   freeLook = false;
   for (S32 i = 0; i< MaxTriggerKeys; i++)
      trigger[i] = false;
}

static inline F32 clampFloatWrap(F32 val)
{
   return val - F32(S32(val));
}

static inline S32 clampRangeClamp(F32 val)
{
   if(val < -1)
      return 0;
   if(val > 1)
      return 32;
            
   // 0.5 / 16 = 0.03125 ... this forces a round up to
   // make the precision near zero equal in the negative
   // and positive directions.  See...
   //
   // http://www.garagegames.com/community/forums/viewthread/49714
   
   return (S32)((val + 1.03125) * 16);
}


#define FANG2IANG(x)   ((U32)((S16)((F32(0x10000) / M_2PI) * x)) & 0xFFFF)
#define IANG2FANG(x)   (F32)((M_2PI / F32(0x10000)) * (F32)((S16)x))

void Move::unclamp()
{
   yaw = IANG2FANG(pyaw);
   pitch = IANG2FANG(ppitch);
   roll = IANG2FANG(proll);

   x = (px - 16) / F32(16);
   y = (py - 16) / F32(16);
   z = (pz - 16) / F32(16);
}

static inline F32 clampAngleClamp( F32 angle )
{
  const F32 limit = ( M_PI_F / 180.0f ) * 179.999f;
  if ( angle < -limit )
     return -limit;
  if ( angle > limit )
     return limit;

  return angle;
}

void Move::clamp()
{
   // If yaw/pitch/roll goes equal or greater than -PI/+PI it 
   // flips the direction of the rotation... we protect against
   // that by clamping before the conversion.
            
   yaw   = clampAngleClamp( yaw );
   pitch = clampAngleClamp( pitch );
   roll  = clampAngleClamp( roll );

   // angles are all 16 bit.
   pyaw = FANG2IANG(yaw);
   ppitch = FANG2IANG(pitch);
   proll = FANG2IANG(roll);

   px = clampRangeClamp(x);
   py = clampRangeClamp(y);
   pz = clampRangeClamp(z);
   unclamp();
}

void Move::pack(BitStream *stream, const Move * basemove)
{
   bool alwaysWriteAll = basemove!=NULL;
   if (!basemove)
      basemove = &NullMove;

   packMove(stream, basemove, alwaysWriteAll);
}

bool Move::packMove(BitStream *stream, const Move* basemove, bool alwaysWriteAll)
{
   S32 i;
   bool triggerDifferent = false;
   for (i=0; i < MaxTriggerKeys; i++)
      if (trigger[i] != basemove->trigger[i])
         triggerDifferent = true;
   bool somethingDifferent = (pyaw!=basemove->pyaw)     ||
                             (ppitch!=basemove->ppitch) ||
                             (proll!=basemove->proll)   ||
                             (px!=basemove->px)         ||
                             (py!=basemove->py)         ||
                             (pz!=basemove->pz)         ||
                             (deviceIsKeyboardMouse!=basemove->deviceIsKeyboardMouse) ||
                             (freeLook!=basemove->freeLook) ||
                             triggerDifferent;
   
   if (alwaysWriteAll || stream->writeFlag(somethingDifferent))
   {
      if(stream->writeFlag(pyaw != basemove->pyaw))
      stream->writeInt(pyaw, 16);
      if(stream->writeFlag(ppitch != basemove->ppitch))
      stream->writeInt(ppitch, 16);
      if(stream->writeFlag(proll != basemove->proll))
      stream->writeInt(proll, 16);

      if (stream->writeFlag(px != basemove->px))
         stream->writeInt(px, 6);
      if (stream->writeFlag(py != basemove->py))
         stream->writeInt(py, 6);
      if (stream->writeFlag(pz != basemove->pz))
         stream->writeInt(pz, 6);
      stream->writeFlag(freeLook);
      stream->writeFlag(deviceIsKeyboardMouse);

      if (stream->writeFlag(triggerDifferent))
         for(i = 0; i < MaxTriggerKeys; i++)
      stream->writeFlag(trigger[i]);
   }

   return (triggerDifferent || somethingDifferent);
}

void Move::unpack(BitStream *stream, const Move * basemove)
{
   bool alwaysReadAll = basemove!=NULL;
   if (!basemove)
      basemove=&NullMove;

   bool readMove = unpackMove(stream, basemove, alwaysReadAll);
   if(!readMove)
      *this = *basemove;
}

bool Move::unpackMove(BitStream *stream, const Move* basemove, bool alwaysReadAll)
{
   bool readMove = alwaysReadAll;
   if(!readMove)
   {
      readMove = stream->readFlag();
   }

   if (readMove)
   {
      pyaw = stream->readFlag() ? stream->readInt(16) : basemove->pyaw;
      ppitch = stream->readFlag() ? stream->readInt(16) : basemove->ppitch;
      proll = stream->readFlag() ? stream->readInt(16) : basemove->proll;

      px = stream->readFlag() ? stream->readInt(6) : basemove->px;
      py = stream->readFlag() ? stream->readInt(6) : basemove->py;
      pz = stream->readFlag() ? stream->readInt(6) : basemove->pz;
      freeLook = stream->readFlag();
      deviceIsKeyboardMouse = stream->readFlag();

      bool triggersDiffer = stream->readFlag();
      for (S32 i = 0; i< MaxTriggerKeys; i++)
         trigger[i] = triggersDiffer ? stream->readFlag() : basemove->trigger[i];
      unclamp();
   }

   return readMove;
}
