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

#include "platform/platformTimer.h"
#include "core/util/journal/process.h"
#include "console/engineAPI.h"

void TimeManager::_updateTime()
{
   // Calculate & filter time delta since last event.

   // How long since last update?
   S32 delta = mTimer->getElapsedMs();

   // Now - we want to try to sleep until the time threshold will hit.
   S32 msTillThresh = (mBackground ? mBackgroundThreshold : mForegroundThreshold) - delta;

   if(msTillThresh > 0)
   {
      // There's some time to go, so let's sleep.
      Platform::sleep( msTillThresh );
   }

   // Ok - let's grab the new elapsed and send that out.
   S32 finalDelta = mTimer->getElapsedMs();
   mTimer->reset();

   timeEvent.trigger(finalDelta);
}

TimeManager::TimeManager()
{
   mBackground = false;
   mTimer = PlatformTimer::create();
   Process::notify(this, &TimeManager::_updateTime, PROCESS_TIME_ORDER);
   
   mForegroundThreshold = 5;
   mBackgroundThreshold = 10;
}

TimeManager::~TimeManager()
{
   Process::remove(this, &TimeManager::_updateTime);
   delete mTimer;
}

void TimeManager::setForegroundThreshold(const S32 msInterval)
{
   AssertFatal(msInterval > 0, "TimeManager::setForegroundThreshold - should have at least 1 ms between time events to avoid math problems!");
   mForegroundThreshold = msInterval;
}

const S32 TimeManager::getForegroundThreshold() const
{
   return mForegroundThreshold;
}

void TimeManager::setBackgroundThreshold(const S32 msInterval)
{
   AssertFatal(msInterval > 0, "TimeManager::setBackgroundThreshold - should have at least 1 ms between time events to avoid math problems!");
   mBackgroundThreshold = msInterval;
}

const S32 TimeManager::getBackgroundThreshold() const
{
   return mBackgroundThreshold;
}

//----------------------------------------------------------------------------------
PlatformTimer::PlatformTimer()
{
}

PlatformTimer::~PlatformTimer()
{
}


// Exposes PlatformTimer to script for when high precision is needed.

#include "core/util/tDictionary.h"
#include "console/console.h"

class ScriptTimerMan
{
public:

   ScriptTimerMan();
   ~ScriptTimerMan();

   S32 startTimer();
   S32 stopTimer( S32 id );

protected:

   static S32 smNextId;

   typedef Map<S32,PlatformTimer*> TimerMap;

   TimerMap mTimers;
};

S32 ScriptTimerMan::smNextId = 1;

ScriptTimerMan::ScriptTimerMan()
{
}

ScriptTimerMan::~ScriptTimerMan()
{
   TimerMap::Iterator itr = mTimers.begin();

   for ( ; itr != mTimers.end(); itr++ )   
      delete itr->value;
   
   mTimers.clear();
}

S32 ScriptTimerMan::startTimer()
{
   PlatformTimer *timer = PlatformTimer::create();
   mTimers.insert( smNextId, timer );
   smNextId++;
   return ( smNextId - 1 );
}

S32 ScriptTimerMan::stopTimer( S32 id )
{
   TimerMap::Iterator itr = mTimers.find( id );
   if ( itr == mTimers.end() )
      return -1;

   PlatformTimer *timer = itr->value;
   S32 elapsed = timer->getElapsedMs();

   mTimers.erase( itr );
   delete timer;

   return elapsed;
}

ScriptTimerMan gScriptTimerMan;

DefineConsoleFunction( startPrecisionTimer, S32, (), , "startPrecisionTimer() - Create and start a high resolution platform timer. Returns the timer id." )
{
   return gScriptTimerMan.startTimer();
}

DefineConsoleFunction( stopPrecisionTimer, S32, ( S32 id), , "stopPrecisionTimer( S32 id ) - Stop and destroy timer with the passed id.  Returns the elapsed milliseconds." )
{
   return gScriptTimerMan.stopTimer( id );
}