#include "T3D/systems/updateSystem.h"

void UpdateSystem::processTick()
{
   for (U32 i = 0; i < UpdateSystemInterface::all.size(); i++)
   {
      if (UpdateSystemInterface::all[i]->mIsEnabled)
      {
         //do work
      }
   }
}

void UpdateSystem::advanceTime(U32 _tickMS)
{
   for (U32 i = 0; i < UpdateSystemInterface::all.size(); i++)
   {
      if (UpdateSystemInterface::all[i]->mIsEnabled)
      {
         //do work
      }
   }
}

void UpdateSystem::interpolateTick(U32 _deltaMS)
{
   for (U32 i = 0; i < UpdateSystemInterface::all.size(); i++)
   {
      if (UpdateSystemInterface::all[i]->mIsEnabled)
      {
         //do work
      }
   }
}