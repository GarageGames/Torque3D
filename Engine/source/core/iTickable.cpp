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

#include "core/iTickable.h"

// The statics
U32 ITickable::smLastTick = 0;
U32 ITickable::smLastTime = 0;
U32 ITickable::smLastDelta = 0;

U32 ITickable::smTickShift = 5;
U32 ITickable::smTickMs = ( 1 << smTickShift );
F32 ITickable::smTickSec = ( F32( ITickable::smTickMs ) / 1000.f );
U32 ITickable::smTickMask = ( smTickMs - 1 );

//------------------------------------------------------------------------------

ITickable::ITickable() : mProcessTick( true )
{
   getProcessList().push_back( this );
}

//------------------------------------------------------------------------------

ITickable::~ITickable()
{
   for( ProcessListIterator i = getProcessList().begin(); i != getProcessList().end(); i++ )
   {
      if( (*i) == this )
      {
         getProcessList().erase( i );
         return;
      }
   }
}

//------------------------------------------------------------------------------

void ITickable::init( const U32 tickShift )
{
    // Sanity!
    AssertFatal( tickShift == 0 || tickShift <= 31, "ITickable::init() - Invalid 'tickShift' parameter!" );

    // Calculate tick constants.
    smTickShift = tickShift;
    smTickMs = ( 1 << smTickShift );
    smTickSec = ( F32( smTickMs ) / 1000.f );
    smTickMask = ( smTickMs - 1 );
}

//------------------------------------------------------------------------------

Vector<ITickable *>& ITickable::getProcessList()
{
   // This helps to avoid the static initialization order fiasco
   static Vector<ITickable *> smProcessList( __FILE__, __LINE__ ); ///< List of tick controls
   return smProcessList;
}

//------------------------------------------------------------------------------

bool ITickable::advanceTime( U32 timeDelta )
{
   U32 targetTime = smLastTime + timeDelta;
   U32 targetTick = ( targetTime + smTickMask ) & ~smTickMask;
   U32 tickCount = ( targetTick - smLastTick ) >> smTickShift;

   // Advance objects
   if( tickCount )
   {
      for( ; smLastTick != targetTick; smLastTick += smTickMs )
      {
         for( U32 i=0; i < getProcessList().size(); )
         {
            ITickable* iTick = getProcessList()[i];
            if( iTick->isProcessingTicks() )
            {
               iTick->processTick();

               // Only advance counter if the tickable hasn't deleted itself
               if( i < getProcessList().size() && iTick == getProcessList()[i] )
                  ++i;
            }
            else
            {
               // Move onto the next tickable
               ++i;
            }
         }
      }
   }

   smLastDelta = ( smTickMs - ( targetTime & smTickMask ) ) & smTickMask;
   F32 dt = smLastDelta / F32( smTickMs );

   // Now interpolate objects that want ticks.  Note that an object should never delete
   // itself during an interpolateTick().
   for( ProcessListIterator i = getProcessList().begin(); i != getProcessList().end(); i++ )
      if( (*i)->isProcessingTicks() )
         (*i)->interpolateTick( dt );


   // Inform ALL objects that time was advanced
   dt = F32( timeDelta ) / 1000.f;
   for( U32 i=0; i < getProcessList().size(); )
   {
      ITickable* iTick = getProcessList()[i];
      iTick->advanceTime( dt );

      // Only advance counter if the tickable hasn't deleted itself
      if( i < getProcessList().size() && iTick == getProcessList()[i] )
         ++i;
   }

   smLastTime = targetTime;

   return tickCount != 0;
}
