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

#ifndef _TIMESOURCE_H_
#define _TIMESOURCE_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

#ifndef _TSTREAM_H_
#include "core/stream/tStream.h"
#endif

#ifndef _SIM_H_
#include "console/sim.h"
#endif


/// Timer that queries the real-time ticker.
struct RealMSTimer
{
   typedef U32 TickType;
   static TickType getTick()
   {
      return Platform::getRealMilliseconds();
   }
};

/// Timer that queries the simulation-time ticker.
struct VirtualMSTimer
{
   typedef U32 TickType;
   static TickType getTick()
   {
      return Platform::getVirtualMilliseconds();
   }
};

/// Timer that queries Sim::getCurrentTime().
struct SimMSTimer
{
   typedef U32 TickType;
   static TickType getTick()
   {
      return Sim::getCurrentTime();
   }
};


///
template< class Timer = RealMSTimer, typename Tick = typename Timer::TickType >
class GenericTimeSource : public IPositionable< Tick >,
                          public IProcess,
                          public IResettable
{
   public:

      typedef IPositionable< Tick > Parent;
      typedef Tick TickType;

   protected:

      ///
      TickType mStartTime;

      ///
      TickType mPauseTime;

      ///
      Timer mTimer;

   public:

      GenericTimeSource()
         : mStartTime( TypeTraits< TickType >::MAX ),
           mPauseTime( TypeTraits< TickType >::MAX ) {}

      bool isStarted() const
      {
         return ( mStartTime != TypeTraits< TickType >::MAX );
      }
      bool isPaused() const
      {
         return ( mPauseTime != TypeTraits< TickType >::MAX );
      }

      /// Return the number of ticks since the time source
      /// has been started.
      TickType getPosition() const
      {
         if( !isStarted() )
            return TypeTraits< TickType >::ZERO;
         else if( isPaused() )
            return ( mPauseTime - mStartTime );
         else
            return ( mTimer.getTick() - mStartTime );
      }

      ///
      void setPosition( TickType pos )
      {
         if( !isStarted() )
            mStartTime = pos;
         else
         {
            TickType savedStartTime = mStartTime;

            mStartTime = ( mTimer.getTick() - pos );
            if( isPaused() )
               mPauseTime = ( mStartTime + ( mPauseTime - savedStartTime ) );
         }
      }

      // IResettable.
      virtual void reset()
      {
         mStartTime = TypeTraits< TickType >::MAX;
         mPauseTime = TypeTraits< TickType >::MAX;
      }

      // IProcess.
      virtual void start()
      {
         if( !isStarted() )
         {
            TickType now = mTimer.getTick();

            if( isPaused() )
            {
               mStartTime += now - mPauseTime;
               mPauseTime = TypeTraits< TickType >::MAX;
            }
            else
               mStartTime = now;
         }
      }
      virtual void stop()
      {
         reset();
      }
      virtual void pause()
      {
         if( !isPaused() )
            mPauseTime = mTimer.getTick();
      }
};

#endif // _TIMESOURCE_H_
