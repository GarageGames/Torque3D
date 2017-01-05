//-----------------------------------------------------------------------------
// Copyright (c) 2014 GarageGames, LLC
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

#ifdef TORQUE_TESTS_ENABLED
#include "testing/unitTesting.h"
#include "platform/platformTimer.h"
#include "core/util/journal/process.h"
#include "math/mMath.h"

TEST(Platform, AdvanceTime)
{
   U32 time = Platform::getVirtualMilliseconds();
   Platform::advanceTime(10);
   U32 newTime = Platform::getVirtualMilliseconds();
   EXPECT_EQ(10, newTime - time)
      << "We advanced 10ms but didn't get a 10ms delta!";
}

TEST(Platform, Sleep)
{
   U32 start = Platform::getRealMilliseconds();
   Platform::sleep(500);
   U32 end = Platform::getRealMilliseconds();
   EXPECT_GE(end - start, 500-10) // account for clock resolution
      << "We didn't sleep at least as long as we requested!";
};

struct handle
{
   S32 mElapsedTime;
   S32 mNumberCalls;

   handle() : mElapsedTime(0), mNumberCalls(0) {}

   void timeEvent(S32 timeDelta)
   {
      mElapsedTime += timeDelta;
      mNumberCalls++;
      
      if(mElapsedTime >= 1000)
         Process::requestShutdown();
   }
};

TEST(TimeManager, BasicAPI)
{
   handle handler;

   // Initialize the time manager...
   TimeManager time;
   time.timeEvent.notify(&handler, &handle::timeEvent);

   // Event loop till at least one second has passed.
   const U32 start = Platform::getRealMilliseconds();

   while(Process::processEvents())
   {
      // If we go too long, kill it off...
      if(Platform::getRealMilliseconds() - start > 30*1000)
      {
         EXPECT_TRUE(false)
            << "Terminated process loop due to watchdog, not due to time manager event, after 30 seconds.";
         Process::requestShutdown();
      }
   }
   const U32 end = Platform::getRealMilliseconds();

   // Now, confirm we have approximately similar elapsed times.
   S32 elapsedRealTime = end - start;
   EXPECT_LT(mAbs(elapsedRealTime - handler.mElapsedTime), 50)
      << "Failed to elapse time to within the desired tolerance.";
   EXPECT_GT(handler.mNumberCalls, 0)
      << "Somehow got no event callbacks from TimeManager?";
};

#endif