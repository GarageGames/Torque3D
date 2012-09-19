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
#include "platformWin32/platformWin32.h"

#include "time.h"

void Platform::sleep(U32 ms)
{
   Sleep(ms);
}

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
   // Converting a LocalTime to SYSTEMTIME
   // requires a few annoying adjustments.
   SYSTEMTIME st;
   st.wMilliseconds  = 0;
   st.wSecond        = lt.sec;
   st.wMinute        = lt.min;
   st.wHour          = lt.hour;
   st.wDay           = lt.monthday;
   st.wDayOfWeek     = lt.weekday;
   st.wMonth         = lt.month + 1;
   st.wYear          = lt.year + 1900;   
   
   TCHAR buffer[1024] = {0};

   int result = 0;

   String outStr;

   // Note: The 'Ex' version of GetDateFormat and GetTimeFormat are preferred 
   // and have better support for supplemental locales but are not supported 
   // for version of windows prior to Vista. 
   //
   // Would be nice if Torque was more aware of the OS version and 
   // take full advantage of it.

   result = GetDateFormat( LOCALE_USER_DEFAULT,
                           DATE_SHORTDATE,
                           &st,
                           NULL,
                           (LPTSTR)buffer,
                           1024 );

   // Also would be nice to have a standard system for torque to
   // retrieve and display windows level errors using GetLastError and
   // FormatMessage...
   AssertWarn( result != 0, "Platform::getLocalTime" );

   outStr += buffer;
   outStr += "\t";

   result = GetTimeFormat( LOCALE_USER_DEFAULT,
                           0,
                           &st,
                           NULL,
                           (LPTSTR)buffer,
                           1024 );

   AssertWarn( result != 0, "Platform::localTimeToString, error occured!" );
   
   outStr += buffer;
   
   return outStr;
}

U32 Platform::getTime()
{
   time_t long_time;
   time( &long_time );
   return long_time;
}

void Platform::fileToLocalTime(const FileTime & ft, LocalTime * lt)
{
   if(!lt)
      return;

   dMemset(lt, 0, sizeof(LocalTime));

   FILETIME winFileTime;
   winFileTime.dwLowDateTime = ft.v1;
   winFileTime.dwHighDateTime = ft.v2;

   SYSTEMTIME winSystemTime;

   // convert the filetime to local time
   FILETIME convertedFileTime;
   if(::FileTimeToLocalFileTime(&winFileTime, &convertedFileTime))
   {
      // get the time into system time struct
      if(::FileTimeToSystemTime((const FILETIME *)&convertedFileTime, &winSystemTime))
      {
         SYSTEMTIME * time = &winSystemTime;

         // fill it in...
         lt->sec = time->wSecond;
         lt->min = time->wMinute;
         lt->hour = time->wHour;
         lt->month = time->wMonth;
         lt->monthday = time->wDay;
         lt->weekday = time->wDayOfWeek;
         lt->year = (time->wYear < 1900) ? 1900 : (time->wYear - 1900);

         // not calculated
         lt->yearday = 0;
         lt->isdst = false;
      }
   }
}

U32 Platform::getRealMilliseconds()
{
   return GetTickCount();
}

U32 Platform::getVirtualMilliseconds()
{
   return winState.currentTime;
}

void Platform::advanceTime(U32 delta)
{
   winState.currentTime += delta;
}

