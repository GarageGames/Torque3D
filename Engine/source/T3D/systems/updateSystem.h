#pragma once
#include "componentSystem.h"

class UpdateSystemInterface : public SystemInterface<UpdateSystemInterface>
{
public:
   bool mIsEnabled;
};

class UpdateSystem
{
public:
   static void processTick();
   static void advanceTime(U32 _tickMS);
   static void interpolateTick(U32 _deltaMS);
};