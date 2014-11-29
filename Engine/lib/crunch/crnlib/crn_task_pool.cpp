// File: crn_task_pool.cpp
// See Copyright Notice and license at the end of inc/crnlib.h
#include "crn_core.h"
#include "crn_task_pool.h"
#include <process.h>

#include "crn_winhdr.h"

namespace crnlib
{
   task_pool::task_pool() :
      m_num_threads(0),
      m_num_outstanding_tasks(0),
      m_exit_flag(false)
   {
      utils::zero_object(m_threads);
   }

   task_pool::task_pool(uint num_threads) :
      m_num_threads(0),
      m_num_outstanding_tasks(0),
      m_exit_flag(false)
   {
      utils::zero_object(m_threads);
      bool status = init(num_threads);
      CRNLIB_VERIFY(status);
   }

   task_pool::~task_pool()
   {
      deinit();
   }

   bool task_pool::init(uint num_threads)
   {
      CRNLIB_ASSERT(num_threads <= cMaxThreads);
      num_threads = math::minimum<uint>(num_threads, cMaxThreads);

      deinit();

      m_task_condition_var.lock();

      m_num_threads = num_threads;

      bool succeeded = true;
      for (uint i = 0; i < num_threads; i++)
      {
         m_threads[i] = (HANDLE)_beginthreadex(NULL, 32768, thread_func, this, 0, NULL);

         CRNLIB_ASSERT(m_threads[i] != 0);
         if (!m_threads[i])
         {
            succeeded = false;
            break;
         }
      }

      m_task_condition_var.unlock();

      if (!succeeded)
      {
         deinit();
         return false;
      }
      return true;
   }

   void task_pool::deinit()
   {
      if (m_num_threads)
      {
         m_task_condition_var.lock();

         m_exit_flag = true;

         m_task_condition_var.unlock();

         for (uint i = 0; i < m_num_threads; i++)
         {
            if (m_threads[i])
            {
               for ( ; ; )
               {
                  uint32 result = WaitForSingleObject(m_threads[i], 1000);
                  if (result == WAIT_OBJECT_0)
                     break;
               }

               CloseHandle(m_threads[i]);

               m_threads[i] = NULL;
            }
         }

         m_num_threads = 0;

         m_exit_flag = false;
      }

      m_tasks.clear();
      m_num_outstanding_tasks = 0;
   }

   uint task_pool::get_num_threads() const
   {
      return m_num_threads;
   }

   void task_pool::queue_task(task_callback_func pFunc, uint64 data, void* pData_ptr)
   {
      CRNLIB_ASSERT(pFunc);

      m_task_condition_var.lock();

      task tsk;
      tsk.m_callback = pFunc;
      tsk.m_data = data;
      tsk.m_pData_ptr = pData_ptr;
      tsk.m_flags = 0;
      m_tasks.push_back(tsk);

      m_num_outstanding_tasks++;

      m_task_condition_var.unlock();
   }

   // It's the object's responsibility to crnlib_delete pObj within the execute_task() method, if needed!
   void task_pool::queue_task(executable_task* pObj, uint64 data, void* pData_ptr)
   {
      CRNLIB_ASSERT(pObj);

      m_task_condition_var.lock();

      task tsk;
      tsk.m_pObj = pObj;
      tsk.m_data = data;
      tsk.m_pData_ptr = pData_ptr;
      tsk.m_flags = cTaskFlagObject;
      m_tasks.push_back(tsk);

      m_num_outstanding_tasks++;

      m_task_condition_var.unlock();
   }

   bool task_pool::join_condition_func(void* pCallback_data_ptr, uint64 callback_data)
   {
      callback_data;

      task_pool* pPool = static_cast<task_pool*>(pCallback_data_ptr);

      return (!pPool->m_num_outstanding_tasks) || pPool->m_exit_flag;
   }

   void task_pool::process_task(task& tsk)
   {
      if (tsk.m_flags & cTaskFlagObject)
         tsk.m_pObj->execute_task(tsk.m_data, tsk.m_pData_ptr);
      else
         tsk.m_callback(tsk.m_data, tsk.m_pData_ptr);

      m_task_condition_var.lock();

      m_num_outstanding_tasks--;

      m_task_condition_var.unlock();
   }

   void task_pool::join()
   {
      for ( ; ; )
      {
         m_task_condition_var.lock();

         if (!m_tasks.empty())
         {
            task tsk(m_tasks.front());
            m_tasks.pop_front();

            m_task_condition_var.unlock();

            process_task(tsk);
         }
         else
         {
            int result = m_task_condition_var.wait(join_condition_func, this);
            result;
            CRNLIB_ASSERT(result >= 0);

            m_task_condition_var.unlock();

            break;
         }
      }
   }

   bool task_pool::wait_condition_func(void* pCallback_data_ptr, uint64 callback_data)
   {
      callback_data;

      task_pool* pPool = static_cast<task_pool*>(pCallback_data_ptr);

      return (!pPool->m_tasks.empty()) || pPool->m_exit_flag;
   }

   unsigned __stdcall task_pool::thread_func(void* pContext)
   {
      //set_thread_name(GetCurrentThreadId(), "taskpoolhelper");

      task_pool* pPool = static_cast<task_pool*>(pContext);

      for ( ; ; )
      {
         pPool->m_task_condition_var.lock();

         int result = pPool->m_task_condition_var.wait(wait_condition_func, pPool);

         CRNLIB_ASSERT(result >= 0);

         if ((result < 0) || (pPool->m_exit_flag))
         {
            pPool->m_task_condition_var.unlock();
            break;
         }

         if (pPool->m_tasks.empty())
            pPool->m_task_condition_var.unlock();
         else
         {
            task tsk(pPool->m_tasks.front());
            pPool->m_tasks.pop_front();

            pPool->m_task_condition_var.unlock();

            pPool->process_task(tsk);
         }
      }

      _endthreadex(0);
      return 0;
   }

} // namespace crnlib
