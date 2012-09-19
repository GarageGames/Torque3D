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

// For VS2005.
#define _WIN32_WINNT 0x501
#ifndef TORQUE_OS_XENON
#include "platformWin32/platformWin32.h"
#endif
#include "platform/async/asyncUpdate.h"


AsyncUpdateThread::AsyncUpdateThread( String name, AsyncUpdateList* updateList )
   : Parent( 0, 0, false, false ),
     mUpdateList( updateList ),
     mName( name )
{
   // Create an auto-reset event in non-signaled state.
   mUpdateEvent = CreateEvent( NULL, false, false, NULL );
}

AsyncUpdateThread::~AsyncUpdateThread()
{
   CloseHandle( ( HANDLE ) mUpdateEvent );
}

void AsyncUpdateThread::_waitForEventAndReset()
{
   WaitForSingleObject( ( HANDLE ) mUpdateEvent, INFINITE );
}

void AsyncUpdateThread::triggerUpdate()
{
   SetEvent( ( HANDLE ) mUpdateEvent );
}

AsyncPeriodicUpdateThread::AsyncPeriodicUpdateThread( String name,
                                                      AsyncUpdateList* updateList,
                                                      U32 intervalMS )
   : Parent( name, updateList )
{
   mUpdateTimer = CreateWaitableTimer( NULL, FALSE, NULL );

   // This is a bit contrived.  The 'dueTime' is in 100 nanosecond intervals
   // and relative if it is negative.  The period is in milliseconds.

   LARGE_INTEGER deltaTime;
   deltaTime.QuadPart = - LONGLONG( intervalMS * 10 /* micro */ * 1000 /* milli */ );

   SetWaitableTimer( ( HANDLE ) mUpdateTimer, &deltaTime, intervalMS, NULL, NULL, FALSE );
}

AsyncPeriodicUpdateThread::~AsyncPeriodicUpdateThread()
{
   CloseHandle( ( HANDLE ) mUpdateTimer );
}

void AsyncPeriodicUpdateThread::_waitForEventAndReset()
{
   HANDLE handles[ 2 ];
   
   handles[ 0 ] = ( HANDLE ) mUpdateEvent;
   handles[ 1 ] = ( HANDLE ) mUpdateTimer;

   WaitForMultipleObjects( 2, handles, FALSE, INFINITE );
}
