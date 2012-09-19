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

#ifndef _PROCESSLIST_H_
#define _PROCESSLIST_H_

#ifndef _SIM_H_
#include "console/sim.h"
#endif
#ifndef _TSIGNAL_H_
#include "core/util/tSignal.h"
#endif

//----------------------------------------------------------------------------

#define TickMs      32
#define TickSec     (F32(TickMs) / 1000.0f)

//----------------------------------------------------------------------------

class GameConnection;
struct Move;


class ProcessObject
{
   
public:

   ProcessObject();
   virtual ~ProcessObject() { removeFromProcessList(); }

   /// Removes this object from the tick-processing list
   void removeFromProcessList() { plUnlink(); }   

   /// Set the status of tick processing.
   ///
   /// Set true to receive processTick, advanceTime, and interpolateTick calls.
   ///
   /// @see processTick
   /// @param   t   If true, tick processing is enabled.
   virtual void setProcessTick( bool t ) { mProcessTick = t; }

   /// Returns true if this object processes ticks.
   bool isTicking() const { return mProcessTick; }

   /// This is really implemented in GameBase and is only here to avoid
   /// casts within ProcessList.
   virtual GameConnection* getControllingClient() { return NULL; }   

   /// This is really implemented in GameBase and is only here to avoid
   /// casts within ProcessList.
   virtual U32 getPacketDataChecksum( GameConnection *conn ) { return -1; }

   /// Force this object to process after some other object.
   ///
   /// For example, a player mounted to a vehicle would want to process after 
   /// the vehicle to prevent a visible 'lagging' from occurring when the 
   /// vehicle moves. So the player would be set to processAfter(theVehicle).
   ///
   /// @param   obj   Object to process after
   virtual void processAfter( ProcessObject *obj ) {}
  
   /// Clears the effects of a call to processAfter()
   virtual void clearProcessAfter() {}

   /// Returns the object that this processes after.
   ///
   /// @see processAfter
   virtual ProcessObject* getAfterObject() const { return NULL; }

   /// Processes a move event and updates object state once every 32 milliseconds.
   ///
   /// This takes place both on the client and server, every 32 milliseconds (1 tick).
   ///
   /// @see    ProcessList
   /// @param  move   Move event corresponding to this tick, or NULL.
   virtual void processTick( const Move *move ) {}

   /// Interpolates between tick events.  This takes place on the CLIENT ONLY.
   ///
   /// @param   delta   Time since last call to interpolate
   virtual void interpolateTick( F32 delta ) {}

   /// Advances simulation time for animations. This is called every frame.
   ///
   /// @param   dt   Time since last advance call
   virtual void advanceTime( F32 dt ) {}
   
   /// Allow object to modify the Move before it is ticked or sent to the server.
   /// This is only called for the control object on the client-side.
   virtual void preprocessMove( Move *move ) {}

//protected:

   struct Link
   {
      ProcessObject *next;
      ProcessObject *prev;
   };

   // Processing interface
   void plUnlink();
   void plLinkAfter(ProcessObject*);
   void plLinkBefore(ProcessObject*);
   void plJoin(ProcessObject*);

   U32 mProcessTag;                       // Tag used during sort
   U32 mOrderGUID;                        // UID for keeping order synced (e.g., across network or runs of sim)
   Link mProcessLink;                     // Ordered process queue

   bool mProcessTick;

   bool mIsGameBase;
};

//----------------------------------------------------------------------------

typedef Signal<void()> PreTickSignal;
typedef Signal<void(SimTime)> PostTickSignal;
class GameBase;

/// List of ProcessObjects.
class ProcessList
{
public:

   ProcessList();
   virtual ~ProcessList() {}

   void markDirty()  { mDirty = true; }
   bool isDirty()  { return mDirty; }   

   SimTime getLastTime() { return mLastTime; }
   F32 getLastDelta() { return mLastDelta; }
   F32 getLastInterpDelta() { return mLastDelta / F32(TickMs); }
   U32 getTotalTicks() { return mTotalTicks; }
   void dumpToConsole();

   PreTickSignal& preTickSignal() { return mPreTick; }
   PostTickSignal& postTickSignal() { return mPostTick; }
   
   virtual void addObject( ProcessObject *obj );
   
   /// Returns true if a tick was processed.
   virtual bool advanceTime( SimTime timeDelta );

protected:
 
   void orderList();
   GameBase* getGameBase( ProcessObject *obj );

   virtual void advanceObjects();
   virtual void onAdvanceObjects() { advanceObjects(); }
   virtual void onPreTickObject( ProcessObject* ) {}
   virtual void onTickObject( ProcessObject* ) {}   

protected:

   ProcessObject mHead;

   U32 mCurrentTag;
   bool mDirty;

   U32 mTotalTicks;
   SimTime mLastTick;
   SimTime mLastTime;
   F32 mLastDelta;

   PreTickSignal mPreTick;
   PostTickSignal mPostTick;
};

#endif // _PROCESSLIST_H_