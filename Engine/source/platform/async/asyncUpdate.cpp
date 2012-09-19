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

#include "platform/async/asyncUpdate.h"
#include "core/stream/tStream.h"


//-----------------------------------------------------------------------------
//    AsyncUpdateList implementation.
//-----------------------------------------------------------------------------

void AsyncUpdateList::process( S32 timeOut )
{
   U32 endTime = 0;
   if( timeOut != -1 )
      endTime = Platform::getRealMilliseconds() + timeOut;

   // Flush the process list.

   IPolled* ptr;
   IPolled* firstProcessedPtr = 0;

   while( mUpdateList.tryPopFront( ptr ) )
   {
      if( ptr == firstProcessedPtr )
      {
         // We've wrapped around.  Stop.

         mUpdateList.pushFront( ptr );
         break;
      }

      if( ptr->update() )
      {
         mUpdateList.pushBack( ptr );

         if( !firstProcessedPtr )
            firstProcessedPtr = ptr;
      }

      // Stop if we have exceeded our processing time budget.

      if( timeOut != -1
          && Platform::getRealMilliseconds() >= endTime )
         break;
   }
}

//--------------------------------------------------------------------------
//    AsyncUpdateThread implementation.
//--------------------------------------------------------------------------

void AsyncUpdateThread::run( void* )
{
   _setName( getName() );

   while( !checkForStop() )
   {
      _waitForEventAndReset();
      
      if( !checkForStop() )
         mUpdateList->process();
   }
}
