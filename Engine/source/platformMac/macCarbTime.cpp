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

#include <CoreServices/CoreServices.h>
#include "platform/platformTimer.h"
#include <time.h>
#include <unistd.h>

//--------------------------------------

static U32 sgCurrentTime = 0;

//--------------------------------------
void Platform::getLocalTime(LocalTime &lt)
{
   struct tm systime;
   time_t long_time;

   /// Get time as long integer.
   time( &long_time );
   /// Convert to local time, thread safe.
   localtime_r( &long_time, &systime );
   
   /// Fill the return struct
   lt.sec      = systime.tm_sec;
   lt.min      = systime.tm_min;
   lt.hour     = systime.tm_hour;
   lt.month    = systime.tm_mon;
   lt.monthday = systime.tm_mday;
   lt.weekday  = systime.tm_wday;
   lt.year     = systime.tm_year;
   lt.yearday  = systime.tm_yday;
   lt.isdst    = systime.tm_isdst;
}   

String Platform::localTimeToString( const LocalTime &lt )
{
   tm systime;
   
   systime.tm_sec    = lt.sec;
   systime.tm_min    = lt.min;
   systime.tm_hour   = lt.hour;
   systime.tm_mon    = lt.month;
   systime.tm_mday   = lt.monthday;
   systime.tm_wday   = lt.weekday;
   systime.tm_year   = lt.year;
   systime.tm_yday   = lt.yearday;
   systime.tm_isdst  = lt.isdst;

   return asctime( &systime );
}

/// Gets the time in seconds since the Epoch
U32 Platform::getTime()
{
   time_t epoch_time;
   time( &epoch_time );
   return epoch_time;
}   

/// Gets the time in milliseconds since some epoch. In this case, system start time.
/// Storing milliseconds in a U32 overflows every 49.71 days
U32 Platform::getRealMilliseconds()
{
   // Duration is a S32 value.
   // if negative, it is in microseconds.
   // if positive, it is in milliseconds.
   Duration durTime = AbsoluteToDuration(UpTime());
   U32 ret;
   if( durTime < 0 )
      ret = durTime / -1000;
   else 
      ret = durTime;

   return ret;
}   

U32 Platform::getVirtualMilliseconds()
{
   return sgCurrentTime;   
}   

void Platform::advanceTime(U32 delta)
{
   sgCurrentTime += delta;
}   

/// Asks the operating system to put the process to sleep for at least ms milliseconds
void Platform::sleep(U32 ms)
{
    // note: this will overflow if you want to sleep for more than 49 days. just so ye know.
    usleep( ms * 1000 );
}

//----------------------------------------------------------------------------------
PlatformTimer* PlatformTimer::create()
{
   return new DefaultPlatformTimer;
}

void Platform::fileToLocalTime(const FileTime & ft, LocalTime * lt)
{
   if(!lt)
      return;
      
   time_t long_time = ft;

   struct tm systime;

   /// Convert to local time, thread safe.
   localtime_r( &long_time, &systime );
   
   /// Fill the return struct
   lt->sec      = systime.tm_sec;
   lt->min      = systime.tm_min;
   lt->hour     = systime.tm_hour;
   lt->month    = systime.tm_mon;
   lt->monthday = systime.tm_mday;
   lt->weekday  = systime.tm_wday;
   lt->year     = systime.tm_year;
   lt->yearday  = systime.tm_yday;
   lt->isdst    = systime.tm_isdst;
}

