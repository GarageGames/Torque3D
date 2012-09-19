#ifndef NV_THREAD_CONFIG_H

#define NV_THREAD_CONFIG_H

#include "NvUserMemAlloc.h"

/*

NvThreadConfig.h : A simple wrapper class to define threading and mutex locks.

*/


/*!
**
** Copyright (c) 2009 by John W. Ratcliff mailto:jratcliffscarab@gmail.com
**
** Portions of this source has been released with the PhysXViewer application, as well as
** Rocket, CreateDynamics, ODF, and as a number of sample code snippets.
**
** If you find this code useful or you are feeling particularily generous I would
** ask that you please go to http://www.amillionpixels.us and make a donation
** to Troy DeMolay.
**
** DeMolay is a youth group for young men between the ages of 12 and 21.
** It teaches strong moral principles, as well as leadership skills and
** public speaking.  The donations page uses the 'pay for pixels' paradigm
** where, in this case, a pixel is only a single penny.  Donations can be
** made for as small as $4 or as high as a $100 block.  Each person who donates
** will get a link to their own site as well as acknowledgement on the
** donations blog located here http://www.amillionpixels.blogspot.com/
**
** If you wish to contact me you can use the following methods:
**
** Skype ID: jratcliff63367
** Yahoo: jratcliff63367
** AOL: jratcliff1961
** email: jratcliffscarab@gmail.com
**
**
** The MIT license:
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is furnished
** to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.

** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
** WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#ifdef _MSC_VER
typedef __int64 int64_t;
#else
#include <stdint.h>
#endif

namespace CONVEX_DECOMPOSITION
{

NxU32 tc_timeGetTime(void);
void     tc_sleep(NxU32 ms);

void     tc_spinloop();
void     tc_interlockedExchange(void *dest, const int64_t exchange);
NxI32      tc_interlockedCompareExchange(void *dest, NxI32 exchange, NxI32 compare);
NxI32      tc_interlockedCompareExchange(void *dest, const NxI32 exchange1, const NxI32 exchange2, const NxI32 compare1, const NxI32 compare2);

class ThreadMutex
{
public:
  virtual void lock(void) = 0;
  virtual void unlock(void) = 0;
  virtual bool tryLock(void) = 0;
};


ThreadMutex * tc_createThreadMutex(void);
void          tc_releaseThreadMutex(ThreadMutex *tm);

class ThreadInterface
{
public:
  virtual void threadMain(void) = 0;
};

class Thread
{
public:
};

Thread      * tc_createThread(ThreadInterface *tinterface);
void          tc_releaseThread(Thread *t);

class ThreadEvent
{
public:
  virtual void setEvent(void) = 0; // signal the event
  virtual void resetEvent(void) = 0;
  virtual void waitForSingleObject(NxU32 ms) = 0;
};

ThreadEvent * tc_createThreadEvent(void);
void          tc_releaseThreadEvent(ThreadEvent *t);

}; // end of namespace


#endif
