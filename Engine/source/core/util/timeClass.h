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

#ifndef _TIMECLASS_H_
#define _TIMECLASS_H_

#ifndef _TORQUE_TYPES_H_
#include "platform/types.h"
#endif


#if defined(TORQUE_COMPILER_VISUALC)
   #define TORQUE_CONSTANT_S64(a) (a##I64)
   #define TORQUE_CONSTANT_U64(a) (a##UI64)
#else
   #define TORQUE_CONSTANT_S64(a) (a##LL)      ///< Used to declare signed 64 bit constants  @hideinitializer
   #define TORQUE_CONSTANT_U64(a) (a##ULL)     ///< Used to declare unsigned 64 bit constants @hideinitializer
#endif

namespace Torque
{

//-----------------------------------------------------------------------------
/// 64 bit time representation with ten microsecond resolution.
class Time
{
   class Tester;
   
public:
   struct DateTime
   {
      S32 year;
      S32 month;
      S32 day;
      S32 hour;
      S32 minute;
      S32 second;
      S32 microsecond;
   };
 
   static void getCurrentDateTime(DateTime &dateTime);
   static Time getCurrentTime();

   static const S64 OneDay          = TORQUE_CONSTANT_S64(8640000000);
   static const S64 OneHour         = TORQUE_CONSTANT_S64( 360000000);
   static const S64 OneMinute       = TORQUE_CONSTANT_S64(   6000000);
   static const S64 OneSecond       = TORQUE_CONSTANT_S64(    100000);
   static const S64 OneMillisecond  = TORQUE_CONSTANT_S64(       100);

   Time();
   explicit Time(S64 time);
   Time(S32 year, S32 month, S32 day, S32 hour, S32 minute, S32 second, S32 microsecond);
   Time(const DateTime &dateTime);

   bool set(S32 year, S32 month, S32 day, S32 hour, S32 minute, S32 second, S32 microsecond);
   void get(S32 *year, S32 *month, S32 *day, S32 *hour, S32 *minute, S32 *second, S32 *microsecond) const;

   Time operator+() const;
   Time operator-() const;
   Time operator+(const Time &time) const;
   Time operator-(const Time &time) const;
   S64 operator/(const Time &time) const;
   const Time& operator+=(const Time time);
   const Time& operator-=(const Time time);
   
   template<typename T> Time operator*(T scaler) const { return Time(_time * scaler); }
   template<typename T> Time operator/(T scaler) const { return Time(_time / scaler); }
   template<typename T> friend Time operator*(T scaler,Time t) { return t * scaler; }

   bool operator==(const Time &time) const;
   bool operator!=(const Time &time) const;
   bool operator<(const Time &time) const;
   bool operator>(const Time &time) const;
   bool operator<=(const Time &time) const;
   bool operator>=(const Time &time) const;

   operator Tester*() const
   {
      static Tester test;
      return (_time == 0)? 0: &test;
   }
   bool operator!() const;

   S64 getSeconds() const;
   S64 getMilliseconds() const;
   S64 getMicroseconds() const;
   S64 getInternalRepresentation() const;

private:
   class Tester
   {
      void operator delete(void*) {}
   };

   S64 _time;

   bool _isLeapYear(S32 year) const;
   S32  _daysInMonth(S32 month, S32 year) const;
};

namespace TimeConstant
{
   const Time OneDay          (Time::OneDay);
   const Time OneHour         (Time::OneHour);
   const Time OneMinute       (Time::OneMinute);
   const Time OneSecond       (Time::OneSecond);
   const Time OneMillisecond  (Time::OneMillisecond);
}

//-----------------------------------------------------------------------------

inline Time::Time()
{
   _time = 0;
}

inline Time::Time(S64 time)
{
   _time = time;
}

inline Time::Time(S32 year, S32 month, S32 day, S32 hour, S32 minute, S32 second, S32 microsecond)
{
   set(year, month, day, hour, minute, second, microsecond);
}

inline Time::Time(const Time::DateTime &dateTime)
{
   set(dateTime.year, dateTime.month, dateTime.day, dateTime.hour, dateTime.minute, dateTime.second, dateTime.microsecond);
}

inline Time Time::operator+() const
{
   return Time(_time);
}

inline Time Time::operator-() const
{
   return Time(-_time);
}

inline Time Time::operator+(const Time &time) const
{
   return Time(_time + time._time);
}

inline Time Time::operator-(const Time &time) const
{
   return Time(_time - time._time);
}

inline S64 Time::operator/(const Time &time) const
{
   return S64(_time / time._time);
}

inline const Time& Time::operator+=(const Time time)
{
   _time += time._time;
   return *this;
}

inline const Time& Time::operator-=(const Time time)
{
   _time -= time._time;
   return *this;
}

inline bool Time::operator==(const Time &time) const
{
   return (_time == time._time);
}

inline bool Time::operator!=(const Time &time) const
{
   return (_time != time._time);
}

inline bool Time::operator<(const Time &time) const
{
   return (_time < time._time);
}

inline bool Time::operator>(const Time &time) const
{
   return (_time > time._time);
}

inline bool Time::operator<=(const Time &time) const
{
   return (_time <= time._time);
}

inline bool Time::operator>=(const Time &time) const
{
   return (_time >= time._time);
}

inline bool Time::operator !() const
{
   return _time == 0;
}

inline S64 Time::getSeconds() const
{
   return _time / TimeConstant::OneSecond._time;
}

inline S64 Time::getMilliseconds() const
{
   return _time / TimeConstant::OneMillisecond._time;
}

inline S64 Time::getMicroseconds() const
{
   return _time * 10;
}

inline S64 Time::getInternalRepresentation() const
{
   return _time;
}

//-----------------------------------------------------------------------------
// time i/o time functions

template<class S> inline bool read(S &stream, Time *theTime)
{
   S64 time;
   bool ret = read(stream, &time);
   *theTime = Time(time);
   return ret;
}

template<class S> inline bool write(S &stream, const Time &theTime)
{
   S64 time = theTime.getInternalRepresentation();
   return write(stream, time);
}

//-----------------------------------------------------------------------------

inline Time UnixTimeToTime(U32 t)
{
   // Converts "unix" time, seconds since 00:00:00 UTC, January 1, 1970
   return Time(((S64)(t)) * 100000 + TORQUE_CONSTANT_S64(6213568320000000));
}

inline Time Win32FileTimeToTime(U32 low,U32 high)
{
   // Converts Win32 "file" time, 100 nanosecond intervals since 00:00:00 UTC, January 1, 1601
   S64 t = (((S64)high) << 32) + low;
   return Time(t / 100 + TORQUE_CONSTANT_S64(5049120960000000));
}


} // Namespace

#endif
