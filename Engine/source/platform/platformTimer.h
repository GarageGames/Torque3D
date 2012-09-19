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

#ifndef _PLATFORM_PLATFORMTIMER_H_
#define _PLATFORM_PLATFORMTIMER_H_

#include "platform/platform.h"
#include "core/util/journal/journaledSignal.h"

/// Platform-specific timer class.
///
/// This exists primarily as support for the TimeManager, but may be useful
/// elsewhere.
class PlatformTimer
{
protected:
   PlatformTimer();
public:
   virtual ~PlatformTimer();
   
   /// Get the number of MS that have elapsed since creation or the last
   /// reset call.
   virtual const S32 getElapsedMs()=0;
   
   /// Reset elapsed ms back to zero.
   virtual void reset()=0;
   
   /// Create a new PlatformTimer.
   static PlatformTimer *create();
};

/// Utility class to fire journalled time-delta events at regular intervals.
///
/// Most games and simulations need the ability to update their state based on
/// a time-delta. However, tracking time accurately and sending out well-conditioned
/// events (for instance, allowing no events with delta=0) tends to be platform
/// specific. This class provides an abstraction around this platform mojo.
///
/// In addition, a well behaved application may want to alter how frequently
/// it processes time advancement depending on its execution state. For instance,
/// a game running in the background can significantly reduce CPU usage
/// by only updating every 100ms, instead of trying to maintain a 1ms update
/// update rate.
class TimeManager
{
   PlatformTimer *mTimer;
   S32 mForegroundThreshold, mBackgroundThreshold;
   bool mBackground;
   
   void _updateTime();

public:

   TimeManagerEvent timeEvent;
   
   TimeManager();   
   ~TimeManager();
   
   void setForegroundThreshold(const S32 msInterval);
   const S32 getForegroundThreshold() const;
   
   void setBackgroundThreshold(const S32 msInterval);
   const S32 getBackgroundThreshold() const;

   void setBackground(const bool isBackground) { mBackground = isBackground; };
   const bool getBackground() const { return mBackground; };

};

class DefaultPlatformTimer : public PlatformTimer
{
   S32 mLastTime, mNextTime;
   
public:
   DefaultPlatformTimer()
   {
      mLastTime = mNextTime = Platform::getRealMilliseconds();
   }
   
   const S32 getElapsedMs()
   {
      mNextTime = Platform::getRealMilliseconds();
      return (mNextTime - mLastTime);
   }
   
   void reset()
   {
      mLastTime = mNextTime;
   }
};

#endif