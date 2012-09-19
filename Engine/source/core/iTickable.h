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

#ifndef _ITICKABLE_H_
#define _ITICKABLE_H_

#include "core/util/tVector.h"

/// This interface allows you to let any object be ticked. You use it like so:
/// @code
/// class FooClass : public SimObject, public virtual ITickable
/// {
///    // You still mark SimObject as Parent
///    typdef SimObject Parent;
/// private:
/// ...
///
/// protected:
///    // These three methods are the interface for ITickable
///    virtual void interpolateTick( F32 delta );
///    virtual void processTick();
///    virtual void advanceTime( F32 timeDelta );
///
/// public:
/// ...
/// };
/// @endcode
/// Please note the three methods you must implement to use ITickable, but don't
/// worry. If you forget, the compiler will tell you so. Also note that the
/// typedef for Parent should NOT BE SET to ITickable, the compiler will <i>probably</i>
/// also tell you if you forget that. Last, but assuridly not least is that you note
/// the way that the inheretance is done: public <b>virtual</b> ITickable
/// It is very important that you keep the virtual keyword in there, otherwise
/// proper behavior is not guarenteed. You have been warned.
///
/// The point of a tickable object is that the object gets ticks at a fixed rate
/// which is one tick every 32ms. This means, also, that if an object doesn't get
/// updated for 64ms, that the next update it will get two-ticks. Basically it
/// comes down to this. You are assured to get one tick per 32ms of time passing
/// provided that isProcessingTicks returns true when ITickable calls it.
///
/// isProcessingTicks is a virtual method and you can (should you want to)
/// override it and put some extended functionality to decide if you want to
/// recieve tick-notification or not.
///
/// The other half of this is that you get time-notification from advanceTime.
/// advanceTime lets you know when time passes regardless of the return value
/// of isProcessingTicks. The object WILL get the advanceTime call every single
/// update. The argument passed to advanceTime is the time since the last call
/// to advanceTime. Updates are not based on the 32ms tick time. Updates are
/// dependant on framerate. So you may get 200 advanceTime calls in a second, or you
/// may only get 20. There is no way of assuring consistant calls of advanceTime
/// like there is with processTick. Both are useful for different things, and
/// it is important to understand the differences between them.
///
/// Interpolation is the last part of the ITickable interface. It is called
/// every update, as long as isProcessingTicks evaluates to true on the object.
/// This is used to interpolate between 32ms ticks. The argument passed to
/// interpolateTick is the time since the last call to processTick. You can see
/// in the code for ITickable::advanceTime that before a tick occurs it calls
/// interpolateTick(0) on every object. This is to tell objects which do interpolate
/// between ticks to reset their interpolation because they are about to get a
/// new tick.
///
/// This is an extremely powerful interface when used properly. An example of a class
/// that properly uses this interface is GuiTickCtrl. The documentation for that
/// class describes why it was created and why it was important that it use
/// a consistant update frequency for its effects.
/// @see GuiTickCtrl
///
/// @todo Support processBefore/After and move the GameBase processing over to use ITickable
class ITickable
{
private:
   static U32 smLastTick;  ///< Time of the last tick that occurred
   static U32 smLastTime;  ///< Last time value at which advanceTime was called
   static U32 smLastDelta; ///< Last delta value for advanceTime

   static U32 smTickShift; ///< Shift value to control how often Ticks occur
   static U32 smTickMs;    ///< Number of milliseconds per tick, 32 in this case
   static F32 smTickSec;   ///< Fraction of a second per tick
   static U32 smTickMask;

   // This just makes life easy
   typedef Vector<ITickable *>::iterator ProcessListIterator;
   /// Returns a reference to the list of all ITickable objects.
   static Vector<ITickable *>& getProcessList();   
   
   bool mProcessTick; ///< Set to true if this object wants tick processing
protected:
   /// This method is called every frame and lets the control interpolate between
   /// ticks so you can smooth things as long as isProcessingTicks returns true
   /// when it is called on the object
   virtual void interpolateTick( F32 delta ) = 0;

   /// This method is called once every 32ms if isProcessingTicks returns true
   /// when called on the object
   virtual void processTick() = 0;

   /// This method is called once every frame regardless of the return value of
   /// isProcessingTicks and informs the object of the passage of time.
   /// @param timeDelta Time increment in seconds.
   virtual void advanceTime( F32 timeDelta ) = 0;

public:

   /// Constructor
   /// This will add the object to the process list
   ITickable();

   /// Destructor
   /// Remove this object from the process list
   virtual ~ITickable();

   /// Is this object wanting to receive tick notifications
   /// @returns True if object wants tick notifications
   bool isProcessingTicks() const { return mProcessTick; };

   /// Sets this object as either tick processing or not
   /// @param   tick     True if this object should process ticks
   virtual void setProcessTicks( bool tick = true );

   /// Initialise the ITickable system.
   static void init( const U32 tickShift );

   /// Gets the Tick bit-shift.
   static U32 getTickShift() { return smTickShift; }
   /// Gets the Tick (ms)
   static U32 getTickMs() { return smTickMs; }
   /// Gets the Tick (seconds)
   static F32 getTickSec() { return smTickSec; }
   /// Gets the Tick mask.
   static U32 getTickMask() { return smTickMask; }

//------------------------------------------------------------------------------

   /// This is called in clientProcess to advance the time for all ITickable
   /// objects.
   /// @param timeDelta Time increment in milliseconds.
   /// @returns True if any ticks were sent
   /// @see clientProcess
   static bool advanceTime( U32 timeDelta );
};

//------------------------------------------------------------------------------

inline void ITickable::setProcessTicks( bool tick /* = true  */ )
{
   mProcessTick = tick;
}

#endif
