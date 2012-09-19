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

#include "platform/platformTLS.h"
#include "platformWin32/platformWin32.h"
#include "core/util/safeDelete.h"

#define TORQUE_ALLOC_STORAGE(member, cls, data) \
   AssertFatal(sizeof(cls) <= sizeof(data), avar("Error, storage for %s must be %d bytes.", #cls, sizeof(cls))); \
   member = (cls *) data; \
   constructInPlace(member)

//-----------------------------------------------------------------------------

struct PlatformThreadStorage
{
   DWORD mTlsIndex;
};

//-----------------------------------------------------------------------------

ThreadStorage::ThreadStorage()
{
   TORQUE_ALLOC_STORAGE(mThreadStorage, PlatformThreadStorage, mStorage);
   mThreadStorage->mTlsIndex = TlsAlloc();
}

ThreadStorage::~ThreadStorage()
{
   TlsFree(mThreadStorage->mTlsIndex);
   destructInPlace(mThreadStorage);
}

void *ThreadStorage::get()
{
   return TlsGetValue(mThreadStorage->mTlsIndex);
}

void ThreadStorage::set(void *value)
{
   TlsSetValue(mThreadStorage->mTlsIndex, value);
}

/* POSIX IMPLEMENTATION LOOKS LIKE THIS:

class PlatformThreadStorage
{
pthread_key_t mThreadKey;
};

ThreadStorage::ThreadStorage()
{
TORQUE_ALLOC_STORAGE(mThreadStorage, PlatformThreadStorage, mStorage);
pthread_key_create(&mThreadStorage->mThreadKey, NULL);
}

ThreadStorage::~ThreadStorage()
{
pthread_key_delete(mThreadStorage->mThreadKey);
}

void *ThreadStorage::get()
{
return pthread_getspecific(mThreadStorage->mThreadKey);
}

void ThreadStorage::set(void *value)
{
pthread_setspecific(mThreadStorage->mThreadKey, value);
}
*/