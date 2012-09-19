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

#include "platform/platform.h"
#include "platform/platformTimer.h"
#include "core/util/journal/journaledSignal.h"
#include "core/util/journal/process.h"
#include "math/mMath.h"
#include "console/console.h"

#include "unit/test.h"
using namespace UnitTesting;

CreateUnitTest(Check_advanceTime, "Platform/Time/advanceTime")
{
   void run()
   {
      U32 time = Platform::getVirtualMilliseconds();
      Platform::advanceTime(10);
      U32 newTime = Platform::getVirtualMilliseconds();
      
      test(newTime - time == 10, "Platform::advanceTime is borked, we advanced 10ms but didn't get a 10ms delta!");
   }
};

CreateUnitTest(Check_platformSleep, "Platform/Time/Sleep")
{
   const static S32 sleepTimeMs = 500;
   void run()
   {
      U32 start = Platform::getRealMilliseconds();
      Platform::sleep(sleepTimeMs);
      U32 end = Platform::getRealMilliseconds();
      
      test(end - start >= sleepTimeMs, "We didn't sleep at least as long as we requested!");
   }
};

CreateUnitTest(Check_timeManager, "Platform/Time/Manager")
{
   void handleTimeEvent(S32 timeDelta)
   {
      mElapsedTime += timeDelta;
      mNumberCalls++;
      
      if(mElapsedTime >= 1000)
         Process::requestShutdown();
   }
   
   S32 mElapsedTime;
   S32 mNumberCalls;
   
   void run()
   {
      mElapsedTime = mNumberCalls = 0;
      
      // Initialize the time manager...
      TimeManager time;
      time.timeEvent.notify(this, &Check_timeManager::handleTimeEvent);
      
      // Event loop till at least one second has passed.
      const U32 start = Platform::getRealMilliseconds();

      while(Process::processEvents())
      {
         // If we go too long, kill it off...
         if(Platform::getRealMilliseconds() - start > 30*1000)
         {
            test(false, "Terminated process loop due to watchdog, not due to time manager event, after 30 seconds.");
            Process::requestShutdown();
         }
      }
      
      const U32 end = Platform::getRealMilliseconds();
      
      // Now, confirm we have approximately similar elapsed times.
      S32 elapsedRealTime = end - start;
      test(mAbs(elapsedRealTime - mElapsedTime) < 50, "Failed to elapse time to within the desired tolerance.");
      
      test(mNumberCalls > 0, "Somehow got no event callbacks from TimeManager?");
      
      Con::printf("   Got %d time events, and elapsed %dms from TimeManager, "
                  "%dms according to Platform::getRealMilliseconds()",
                  mNumberCalls, mElapsedTime, elapsedRealTime);
   }
};