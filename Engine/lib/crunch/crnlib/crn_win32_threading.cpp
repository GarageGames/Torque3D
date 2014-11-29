// File: crn_win32_threading.cpp
// See Copyright Notice and license at the end of inc/crnlib.h
#include "crn_core.h"
#include "crn_win32_threading.h"
#include "crn_winhdr.h"

namespace crnlib
{
   uint g_number_of_processors = 1;

   int32 interlocked_compare_exchange32(int32 volatile *Destination, int32 Exchange, int32 Comperand)
   {
      CRNLIB_ASSUME(sizeof(LONG) == sizeof(int32));
      return InterlockedCompareExchange((volatile LONG*)Destination, Exchange, Comperand);
   }

   int32 interlocked_increment32(int32 volatile *lpAddend)
   {
      return InterlockedIncrement((volatile LONG*)lpAddend);
   }

   int32 interlocked_exchange_add32(int32 volatile *Addend, int32 Value)
   {
      return InterlockedExchangeAdd((volatile LONG*)Addend, Value);
   }

   int32 interlocked_exchange32(int32 volatile *Target, int32 Value)
   {
      return InterlockedExchange((volatile LONG*)Target, Value);
   }

   uint32 get_current_thread_id()
   {
      return GetCurrentThreadId();
   }
}
