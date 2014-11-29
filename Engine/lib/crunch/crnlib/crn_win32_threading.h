// File: crn_win32_threading.h
// See Copyright Notice and license at the end of inc/crnlib.h
#pragma once

namespace crnlib
{
   // g_number_of_processors defaults to 1. Will be higher on multicore machines.
   extern uint g_number_of_processors;

   int32 interlocked_compare_exchange32(int32 volatile *Destination, int32 Exchange, int32 Comperand);
   int32 interlocked_increment32(int32 volatile *lpAddend);
   int32 interlocked_exchange_add32(int32 volatile *Addend, int32 Value);
   int32 interlocked_exchange32(int32 volatile *Target, int32 Value);
   uint32 get_current_thread_id();

} // namespace crnlib


