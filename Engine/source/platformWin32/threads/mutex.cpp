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

#include "platform/threads/mutex.h"
#include "platformWin32/platformWin32.h"
#include "core/util/safeDelete.h"

//-----------------------------------------------------------------------------
// Mutex Data
//-----------------------------------------------------------------------------

struct PlatformMutexData
{
   CRITICAL_SECTION mCriticalSection;
};

//-----------------------------------------------------------------------------
// Constructor/Destructor
//-----------------------------------------------------------------------------

Mutex::Mutex()
{
   mData = new PlatformMutexData;
   InitializeCriticalSection( &mData->mCriticalSection );
}

Mutex::~Mutex()
{
   AssertFatal( TryEnterCriticalSection( &mData->mCriticalSection ), "Mutex::~Mutex - Critical section is locked!" );
   DeleteCriticalSection( &mData->mCriticalSection );
   SAFE_DELETE( mData );
}

//-----------------------------------------------------------------------------
// Public Methods
//-----------------------------------------------------------------------------

bool Mutex::lock( bool block )
{
   AssertFatal( mData, "Mutex::lock - No data!" );

   if( !block )
      return TryEnterCriticalSection( &mData->mCriticalSection );
   else
   {
      EnterCriticalSection( &mData->mCriticalSection );
      return true;
   }
}

void Mutex::unlock()
{
   AssertFatal( mData, "Mutex::unlock - No data!" );
   LeaveCriticalSection( &mData->mCriticalSection );
}
