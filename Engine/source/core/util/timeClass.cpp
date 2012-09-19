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

#include <time.h>

#include "core/util/timeClass.h"

namespace Torque
{

//Micro   0.000001   10-6
//Milli   0.001      10-3

static S8  _DaysInMonth[13]    = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static S8  _DaysInMonthLeap[13]= {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static S32 _DayNumber[13]      = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
static S32 _DayNumberLeap[13]  = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366};


/**----------------------------------------------------------------------------
* PRIVATE Test to see if a year is a leap year.
*
* @param year Year to test for leap year
* @return true if year is a leap year
*/
inline bool Time::_isLeapYear(S32 year) const
{
   return ((year & 3) == 0) && ( ((year % 100) != 0) || ((year % 400) == 0) );
}


/**----------------------------------------------------------------------------
* PRIVATE Determine the number of days in any given month/year
*
* @param month Month to be tested
* @param year Year to be tested
* @return Number of days in month/year
*/
S32 Time::_daysInMonth(S32 month, S32 year) const
{
   if (_isLeapYear(year))
      return _DaysInMonthLeap[month];
   else
      return _DaysInMonth[month];
}


//-----------------------------------------------------------------------------
void Time::getCurrentDateTime(DateTime &dateTime)
{
   time_t      long_time;

   time( &long_time );

   struct tm   *systime = localtime( &long_time );

   dateTime.year        = systime->tm_year;
   dateTime.month       = systime->tm_mon;
   dateTime.day         = systime->tm_mday;
   dateTime.hour        = systime->tm_hour;
   dateTime.minute      = systime->tm_min;
   dateTime.second      = systime->tm_sec;
   dateTime.microsecond = 0;
}

Time Time::getCurrentTime()
{
   return Torque::UnixTimeToTime( time( NULL ) );
}

bool Time::set(S32 year, S32 month, S32 day, S32 hour, S32 minute, S32 second, S32 microsecond)
{
   second += microsecond / 100000;
   microsecond %= 100000;
   minute += second / 60;
   second %= 60;
   hour += minute / 60;
   minute %= 60;
   S32 carryDays = hour / 24;
   hour %= 24;

   bool leapYear = _isLeapYear(year);

   year -= 1;     // all the next operations need (year-1) so do it ahead of time
   S32 gregorian = 365 * year             // number of days since the epoch
                   + (year/4)             // add Julian leap year days
                   - (year/100)           // subtract century leap years
                   + (year/400)           // add gregorian 400 year leap adjustment
                   + ((367*month-362)/12) // days in prior months
                   + day                  // add days
                   + carryDays;           // add days from time overflow/underflow

   // make days in this year adjustment if leap year
   if (leapYear)
   {
      if (month > 2)
         gregorian -= 1;
   }
   else
   {
      if (month > 2)
         gregorian -= 2;
   }

   _time  = S64(gregorian) * OneDay;
   _time += S64((hour * OneHour) +
                (minute * OneMinute) +
                (second * OneSecond) +
                microsecond);

   return true;
}


//-----------------------------------------------------------------------------
void Time::get(S32 *pyear, S32 *pmonth, S32 *pday, S32 *phour, S32 *pminute, S32 *psecond, S32 *pmicrosecond) const
{
   // extract date if requested
   if (pyear || pmonth || pday)
   {
      S32 gregorian = (S32)(_time / OneDay);

      S32 prior = gregorian - 1;           // prior days
      S32 years400 = prior / 146097L;      // number of 400 year cycles
      S32 days400 = prior % 146097L;       // days NOT in years400
      S32 years100 = days400 / 36524L;     // number 100 year cycles not checked
      S32 days100 =  days400 % 36524L;     // days NOT already included
      S32 years4 = days100 / 1461L;        // number 4 year cycles not checked
      S32 days4 = days100 % 1461L;         // days NOT already included
      S32 year1 = days4 / 365L;            // number years not already checked
      S32 day1  = days4 % 365L;            // days NOT already included
      S32 day;
      S32 year = (400 * years400) + (100 * years100) + (4 * years4) + year1;

      // December 31 of leap year
      if (years100 == 4 || year1 == 4)
      {
          day = 366;
      }
      else
      {
          year += 1;
          day = day1 + 1;
      }

      const S32 *dayNumber = _isLeapYear(year) ? _DayNumberLeap : _DayNumber;

      // find month and day in month given computed year and day number,
      S32 month = 1;
      while(day >= dayNumber[month])
         month++;

      day -= dayNumber[month-1];

      if(pyear)
         *pyear  = year;
      if(pmonth)
         *pmonth = month;
      if(pday)
         *pday   = day;
   }

   // extract time
   if (phour)
      *phour = (S32)((_time % OneDay) / OneHour);

   S32 time = (S32)(_time % OneHour);

   if (pminute)
      *pminute = time / (S32)OneMinute;
   time %= OneMinute;

   if (psecond)
      *psecond = time / (S32)OneSecond;
   if (pmicrosecond)
      *pmicrosecond = time % OneSecond;
}

} // Namespace
