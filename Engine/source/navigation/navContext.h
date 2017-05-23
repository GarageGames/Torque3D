//-----------------------------------------------------------------------------
// Copyright (c) 2014 Daniel Buckmaster
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

#ifndef _NAV_CONTEXT_H_
#define _NAV_CONTEXT_H_

#include "torqueRecast.h"
#include <Recast.h>

/// @brief Implements the rcContext interface in Torque.
class NavContext: public rcContext {
public:
   /// Default constructor.
   NavContext() : rcContext(true) { doResetTimers(); }

   void log(const rcLogCategory category, const String &msg);

protected:
   /// Clears all log entries.
   virtual void doResetLog();

   /// Logs a message.
   /// @param[in] category The category of the message.
   /// @param[in] msg      The formatted message.
   /// @param[in] len      The length of the formatted message.
   virtual void doLog(const rcLogCategory category, const char* msg, const int len);

   /// Clears all timers. (Resets all to unused.)
   virtual void doResetTimers();

   /// Starts the specified performance timer.
   /// @param[in] label The category of timer.
   virtual void doStartTimer(const rcTimerLabel label);

   /// Stops the specified performance timer.
   /// @param[in] label The category of the timer.
   virtual void doStopTimer(const rcTimerLabel label);

   /// Returns the total accumulated time of the specified performance timer.
   /// @param[in] label The category of the timer.
   /// @return The accumulated time of the timer, or -1 if timers are disabled or the timer has never been started.
   virtual int doGetAccumulatedTime(const rcTimerLabel label) const;

private:
   /// Store start time and final time for each timer.
   S32 mTimers[RC_MAX_TIMERS][2];
};

#endif
