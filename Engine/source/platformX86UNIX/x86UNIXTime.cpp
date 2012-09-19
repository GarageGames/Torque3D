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

#include "platformX86UNIX/platformX86UNIX.h"
#include "platform/platformTimer.h"
#include "time.h"
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

static U32 sgCurrentTime = 0;

U32 x86UNIXGetTickCount();

//--------------------------------------
void Platform::getLocalTime(LocalTime &lt)
{
   struct tm *systime;
   time_t long_time;

   time( &long_time );                // Get time as long integer.
   systime = localtime( &long_time ); // Convert to local time.
   
   lt.sec      = systime->tm_sec;      
   lt.min      = systime->tm_min;      
   lt.hour     = systime->tm_hour;     
   lt.month    = systime->tm_mon;    
   lt.monthday = systime->tm_mday; 
   lt.weekday  = systime->tm_wday;  
   lt.year     = systime->tm_year;     
   lt.yearday  = systime->tm_yday;  
   lt.isdst    = systime->tm_isdst;    
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

U32 Platform::getTime()
{
   time_t long_time;
   time( &long_time );
   return long_time;
}   

U32 Platform::getRealMilliseconds()
{
//   struct rusage usageStats;
//   getrusage(RUSAGE_SELF, &usageStats);
//   return usageStats.ru_utime.tv_usec;
   return x86UNIXGetTickCount();
}

U32 Platform::getVirtualMilliseconds()
{
   return sgCurrentTime;
}   

void Platform::advanceTime(U32 delta)
{
   sgCurrentTime += delta;
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

PlatformTimer *PlatformTimer::create()
{
   return new DefaultPlatformTimer();
}
//------------------------------------------------------------------------------
//-------------------------------------- x86UNIX Implementation
//
//
static bool   sg_initialized = false;
static U32 sg_secsOffset  = 0;
//--------------------------------------
U32 x86UNIXGetTickCount()
{
   // TODO: What happens when crossing a day boundary?
   //
   timeval t;

   if (sg_initialized == false) {
      sg_initialized = true;

      gettimeofday(&t, NULL);
      sg_secsOffset = t.tv_sec;
   }

   gettimeofday(&t, NULL);

   U32 secs  = t.tv_sec - sg_secsOffset;
   U32 uSecs = t.tv_usec;

   // Make granularity 1 ms
   return (secs * 1000) + (uSecs / 1000);
}


void Platform::sleep(U32 ms)
{
	// note: this will overflow if you want to sleep for more than 49 days. just so ye know.
	usleep( ms * 1000 );
}
	    
