// File: crn_condition_var.h
// See Copyright Notice and license at the end of inc/crnlib.h
// Inspired by the "monitor" class in "Win32 Multithreaded Programming" by Cohen and Woodring.
// Also see http://en.wikipedia.org/wiki/Monitor_(synchronization)
#pragma once

#include "crn_mutex.h"
#include "crn_event.h"
#include "crn_semaphore.h"

namespace crnlib
{
   class condition_var
   {
      CRNLIB_NO_COPY_OR_ASSIGNMENT_OP(condition_var);
            
   public:
      condition_var(uint spin_count = 4096U);
      ~condition_var();

      // Locks the condition_var. 
      // Recursive locks are supported.
      void lock();
      
      // Returns TRUE if the thread owning this condition function should stop waiting. 
      // This function will always be called from within the condition_var, but it may be called from several different threads!
      typedef bool (*pCondition_func)(void* pCallback_data_ptr, uint64 callback_data);
            
      // Temporarily leaves the lock and waits for a condition to be satisfied.
      // If pCallback is NULL, this method will return after another thread enters and exits the lock (like a Vista-style condition variable).
      // Otherwise, this method will only return when the specified condition function returns TRUE when another thread exits the lock.
      // When this method returns, the calling thread will be inside the lock.
      // Returns -1 on timeout or error, 0 if the wait was satisfied, or 1 or higher if one of the extra wait handles became signaled.
      // It is highly recommended you use a non-null condition callback. If you don't be sure to check for race conditions!
      int wait(pCondition_func pCallback = NULL, void* pCallback_data_ptr = NULL, uint64 callback_data = 0, 
               uint num_wait_handles = 0, const void** pWait_handles = NULL, uint32 max_time_to_wait = UINT32_MAX);
      
      // Unlocks the condition_var. Another thread may be woken up if its condition function has become satisfied.
      void unlock();
      
      uint32 get_cur_lock_count() const;

   private:
      enum { cMaxWaitingThreads = 16, cMaxWaitingThreadsMask = cMaxWaitingThreads - 1 };

      semaphore      m_condition_var_lock;
      mutex          m_waiters_array_lock;
      uint32         m_tls;
      uint           m_cur_age;
      
      struct waiting_thread
      {
         uint64            m_callback_data;
         void*             m_pCallback_ptr;
         pCondition_func   m_callback_func;
         uint              m_age;
         bool              m_satisfied;
         bool              m_occupied;

         event             m_event;
         
         void clear()
         {
            m_callback_data = 0;
            m_pCallback_ptr = NULL;
            m_callback_func = NULL;
            m_age = 0;
            m_satisfied = false;
            m_occupied = false;
         }
      };
      waiting_thread m_waiters[cMaxWaitingThreads];

      int m_max_waiter_array_index;
            
      void set_cur_lock_count(uint32 newCount);
      
      void leave_and_scan(int index_to_ignore = -1);
   };
   
   class scoped_condition_var
   {
      CRNLIB_NO_COPY_OR_ASSIGNMENT_OP(scoped_condition_var);
   public:
      inline scoped_condition_var(condition_var& m) : m_condition_var(m) { m_condition_var.lock(); }
      inline ~scoped_condition_var() { m_condition_var.unlock(); }
   private:
      condition_var& m_condition_var;
   };

} // namespace crnlib
