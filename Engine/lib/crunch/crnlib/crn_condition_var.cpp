// File: crn_condition_var.cpp
// See Copyright Notice and license at the end of inc/crnlib.h
#include "crn_core.h"
#include "crn_condition_var.h"
#include "crn_spinlock.h"
#include "crn_winhdr.h"

namespace crnlib
{
   void spinlock::lock(uint32 max_spins, bool yielding, bool memoryBarrier)
   {
      if (g_number_of_processors <= 1)
         max_spins = 1;

      uint32 spinCount = 0;
      uint32 yieldCount = 0;

      for ( ; ; )
      {
         CRNLIB_ASSUME(sizeof(long) == sizeof(int32));
         if (!InterlockedExchange((volatile long*)&m_flag, TRUE))
            break;

         YieldProcessor();
         YieldProcessor();
         YieldProcessor();
         YieldProcessor();
         YieldProcessor();
         YieldProcessor();
         YieldProcessor();
         YieldProcessor();

         spinCount++;
         if ((yielding) && (spinCount >= max_spins))
         {
            switch (yieldCount)
            {
               case 0:
               {
                  spinCount = 0;

                  Sleep(0);

                  yieldCount++;
                  break;
               }
               case 1:
               {
                  if (g_number_of_processors <= 1)
                     spinCount = 0;
                  else
                     spinCount = max_spins / 2;

                  Sleep(1);

                  yieldCount++;
                  break;
               }
               case 2:
               {
                  if (g_number_of_processors <= 1)
                     spinCount = 0;
                  else
                     spinCount = max_spins;

                  Sleep(2);
                  break;
               }
            }
         }
      }

      if (memoryBarrier)
      {
#ifdef _MSC_VER
         MemoryBarrier();
#elif defined(__MINGW32__) && defined(__MINGW64__)
         __sync_synchronize();
#endif
      }
   }

   void spinlock::unlock()
   {
#ifdef _MSC_VER
      MemoryBarrier();
#elif defined(__MINGW32__) && defined(__MINGW64__)
         __sync_synchronize();
#endif

      m_flag = FALSE;
   }

   mutex::mutex(unsigned int spin_count)
   {
      CRNLIB_ASSUME(sizeof(mutex) >= sizeof(CRITICAL_SECTION));

      void *p = m_buf;
      CRITICAL_SECTION &m_cs = *static_cast<CRITICAL_SECTION *>(p);

      BOOL status = true;
#ifdef _XBOX
      InitializeCriticalSectionAndSpinCount(&m_cs, spin_count);
#else
      status = InitializeCriticalSectionAndSpinCount(&m_cs, spin_count);
#endif
      if (!status)
         crnlib_fail("mutex::mutex: InitializeCriticalSectionAndSpinCount failed", __FILE__, __LINE__);

#ifdef CRNLIB_BUILD_DEBUG
      m_lock_count = 0;
#endif
   }

   mutex::~mutex()
   {
      void *p = m_buf;
      CRITICAL_SECTION &m_cs = *static_cast<CRITICAL_SECTION *>(p);

#ifdef CRNLIB_BUILD_DEBUG
      if (m_lock_count)
         crnlib_assert("mutex::~mutex: mutex is still locked", __FILE__, __LINE__);
#endif
      DeleteCriticalSection(&m_cs);
   }

   void mutex::lock()
   {
      void *p = m_buf;
      CRITICAL_SECTION &m_cs = *static_cast<CRITICAL_SECTION *>(p);

      EnterCriticalSection(&m_cs);
#ifdef CRNLIB_BUILD_DEBUG
      m_lock_count++;
#endif
   }

   void mutex::unlock()
   {
      void *p = m_buf;
      CRITICAL_SECTION &m_cs = *static_cast<CRITICAL_SECTION *>(p);

#ifdef CRNLIB_BUILD_DEBUG
      if (!m_lock_count)
         crnlib_assert("mutex::unlock: mutex is not locked", __FILE__, __LINE__);
      m_lock_count--;
#endif
      LeaveCriticalSection(&m_cs);
   }

   void mutex::set_spin_count(unsigned int count)
   {
      void *p = m_buf;
      CRITICAL_SECTION &m_cs = *static_cast<CRITICAL_SECTION *>(p);

      SetCriticalSectionSpinCount(&m_cs, count);
   }

   semaphore::semaphore(int32 initialCount, int32 maximumCount, const char* pName)
   {
      m_handle = CreateSemaphoreA(NULL, initialCount, maximumCount, pName);
      if (NULL == m_handle)
      {
         CRNLIB_FAIL("semaphore: CreateSemaphore() failed");
      }
   }

   semaphore::~semaphore()
   {
      if (m_handle)
      {
         CloseHandle(m_handle);
         m_handle = NULL;
      }
   }

   void semaphore::release(int32 releaseCount, int32 *pPreviousCount)
   {
      CRNLIB_ASSUME(sizeof(LONG) == sizeof(int32));
      if (0 == ReleaseSemaphore(m_handle, releaseCount, (LPLONG)pPreviousCount))
      {
         CRNLIB_FAIL("semaphore: ReleaseSemaphore() failed");
      }
   }

   bool semaphore::wait(uint32 milliseconds)
   {
      uint32 result = WaitForSingleObject(m_handle, milliseconds);

      if (WAIT_FAILED == result)
      {
         CRNLIB_FAIL("semaphore: WaitForSingleObject() failed");
      }

      return WAIT_OBJECT_0 == result;
   }

   event::event(bool manual_reset, bool initial_state, const char* pName)
   {
      m_handle = CreateEventA(NULL, manual_reset, initial_state, pName);

      if (NULL == m_handle)
         CRNLIB_FAIL("event: CreateEvent() failed");
   }

   event::~event()
   {
      if (m_handle)
      {
         CloseHandle(m_handle);
         m_handle = NULL;
      }
   }

   void event::set(void)
   {
      SetEvent(m_handle);
   }

   void event::reset(void)
   {
      ResetEvent(m_handle);
   }

   void event::pulse(void)
   {
      PulseEvent(m_handle);
   }

   bool event::wait(uint32 milliseconds)
   {
      uint32 result = WaitForSingleObject(m_handle, milliseconds);

      if (result == WAIT_FAILED)
      {
         CRNLIB_FAIL("event: WaitForSingleObject() failed");
      }

      return (result == WAIT_OBJECT_0);
   }

   condition_var::condition_var(uint spin_count) :
      m_condition_var_lock(1, 1),
      m_tls(TlsAlloc()),
      m_cur_age(0),
      m_max_waiter_array_index(-1)
   {
      CRNLIB_ASSERT(TLS_OUT_OF_INDEXES != m_tls);

      m_waiters_array_lock.set_spin_count(spin_count);

      m_waiters_array_lock.lock();

      for (uint i = 0; i < cMaxWaitingThreads; i++)
         m_waiters[i].clear();

      m_waiters_array_lock.unlock();
   }

   condition_var::~condition_var()
   {
      TlsFree(m_tls);
   }

   void condition_var::lock()
   {
      uint32 cur_count = get_cur_lock_count();
      CRNLIB_ASSERT(cur_count != 0xFFFFFFFF);
      cur_count++;
      set_cur_lock_count(cur_count);

      if (1 == cur_count)
         m_condition_var_lock.wait();
   }

   void condition_var::unlock()
   {
      uint32 cur_count = get_cur_lock_count();
      CRNLIB_ASSERT(cur_count);
      cur_count--;
      set_cur_lock_count(cur_count);

      if (!cur_count)
         leave_and_scan();
   }

   void condition_var::leave_and_scan(int index_to_ignore)
   {
      m_waiters_array_lock.lock();

      uint best_age = 0;
      int best_index = -1;
      for (int i = 0; i <= m_max_waiter_array_index; i++)
      {
         waiting_thread& waiter = m_waiters[i];

         if ((i != index_to_ignore) && (waiter.m_occupied) && (!waiter.m_satisfied))
         {
            uint age = m_cur_age - waiter.m_age;

            if ((age > best_age) || (best_index < 0))
            {
               if ((!waiter.m_callback_func) || (waiter.m_callback_func(waiter.m_pCallback_ptr, waiter.m_callback_data)))
               {
                  best_age = age;
                  best_index = i;
               }
            }
         }
      }

      if (best_index >= 0)
      {
         waiting_thread& waiter = m_waiters[best_index];
         waiter.m_satisfied = true;
         waiter.m_event.set();
         m_waiters_array_lock.unlock();
      }
      else
      {
         m_waiters_array_lock.unlock();
         m_condition_var_lock.release();
      }
   }

   uint32 condition_var::get_cur_lock_count() const
   {
      return (uint32)((intptr_t)TlsGetValue(m_tls));
   }

   int condition_var::wait(
     pCondition_func pCallback, void* pCallback_data_ptr, uint64 callback_data,
     uint num_wait_handles, const void** pWait_handles, uint32 max_time_to_wait)
   {
      CRNLIB_ASSERT(get_cur_lock_count());

      // First, see if the calling thread's condition function is satisfied. If so, there's no need to wait.
      if ((pCallback) && (pCallback(pCallback_data_ptr, callback_data)))
         return 0;

      // Add this thread to the list of waiters.
      m_waiters_array_lock.lock();

      uint i;
      for (i = 0; i < cMaxWaitingThreads; i++)
         if (!m_waiters[i].m_occupied)
            break;

      CRNLIB_VERIFY(i != cMaxWaitingThreads);

      m_max_waiter_array_index = math::maximum<int>(m_max_waiter_array_index, i);

      waiting_thread& waiter = m_waiters[i];

      waiter.m_callback_func     = pCallback;
      waiter.m_pCallback_ptr     = pCallback_data_ptr;
      waiter.m_callback_data     = callback_data;
      waiter.m_satisfied         = false;
      waiter.m_occupied          = true;
      waiter.m_age               = m_cur_age++;
      waiter.m_event.reset();

      m_waiters_array_lock.unlock();

      // Now leave the condition_var and scan to see if there are any satisfied waiters.
      leave_and_scan(i);

      // Let's wait for this thread's condition to be satisfied, or until timeout, or until one of the user supplied handles is signaled.
      int return_index = 0;

      const uint cMaxWaitHandles = 64;
      CRNLIB_ASSERT(num_wait_handles < cMaxWaitHandles);

      HANDLE handles[cMaxWaitHandles];

      handles[0] = waiter.m_event.get_handle();
      uint total_handles = 1;

      if (num_wait_handles)
      {
         CRNLIB_ASSERT(pWait_handles);
         memcpy(handles + total_handles, pWait_handles, sizeof(HANDLE) * num_wait_handles);
         total_handles += num_wait_handles;
      }

      uint32 result;
      if (max_time_to_wait == UINT32_MAX)
      {
         do
         {
            result = WaitForMultipleObjects(total_handles, handles, FALSE, 2000);
         } while (result == WAIT_TIMEOUT);
      }
      else
         result = WaitForMultipleObjects(total_handles, handles, FALSE, max_time_to_wait);

      if ((result == WAIT_ABANDONED) || (result == WAIT_TIMEOUT))
         return_index = -1;
      else
         return_index = result - WAIT_OBJECT_0;

      // See if our condition was satisfied, and remove this thread from the waiter list.
      m_waiters_array_lock.lock();

      const bool was_satisfied = waiter.m_satisfied;

      waiter.m_occupied = false;

      m_waiters_array_lock.unlock();

      if (0 == return_index)
      {
         CRNLIB_ASSERT(was_satisfied);
      }
      else
      {
         // Enter the condition_var if a user supplied handle was signaled. This guarantees that on exit of this function we're still inside the condition_var, no matter
         // what happened during the WaitForMultipleObjects() call.
         if (!was_satisfied)
            m_condition_var_lock.wait();
      }

      return return_index;
   }

   void condition_var::set_cur_lock_count(uint32 newCount)
   {
      TlsSetValue(m_tls, (void*)newCount);
   }

} // namespace crnlib
