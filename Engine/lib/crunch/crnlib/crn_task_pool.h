// File: crn_task_pool.h
// See Copyright Notice and license at the end of inc/crnlib.h
#pragma once
#include "crn_condition_var.h"
#include <deque>

namespace crnlib
{
   class task_pool
   {
   public:
      task_pool();
      task_pool(uint num_threads);
      ~task_pool();
      
      enum { cMaxThreads = 16 };
      bool init(uint num_threads);
      void deinit();
      
      uint get_num_threads() const;
      
      // C-style task callback
      typedef void (*task_callback_func)(uint64 data, void* pData_ptr);
      void queue_task(task_callback_func pFunc, uint64 data = 0, void* pData_ptr = NULL);
            
      class executable_task
      {
      public:
         virtual void execute_task(uint64 data, void* pData_ptr) = 0;
      };
      
      // It's the caller's responsibility to crnlib_delete pObj within the execute_task() method, if needed!
      void queue_task(executable_task* pObj, uint64 data = 0, void* pData_ptr = NULL);
      
      template<typename S, typename T>
      inline void queue_object_task(S* pObject, T pObject_method, uint64 data = 0, void* pData_ptr = NULL);
            
      void join();

   private:
      uint m_num_threads;
      
      uint m_num_outstanding_tasks;
            
      void* m_threads[cMaxThreads];
      
      bool m_exit_flag;
                  
      condition_var m_task_condition_var;
      
      enum task_flags
      {
         cTaskFlagObject = 1
      };

      struct task
      {
         uint64               m_data;
         void*                m_pData_ptr;
         
         union
         {
            task_callback_func   m_callback;
            executable_task*     m_pObj;
         };
         
         uint                 m_flags;
      };      
      
      std::deque<task> m_tasks;

      void process_task(task& tsk);
      
      static bool join_condition_func(void* pCallback_data_ptr, uint64 callback_data);
      static bool wait_condition_func(void* pCallback_data_ptr, uint64 callback_data);
      static unsigned __stdcall thread_func(void* pContext);
   };
   
   enum object_task_flags
   {
      cObjectTaskFlagDefault = 0,
      cObjectTaskFlagDeleteAfterExecution = 1
   };
   
   template<typename T>
   class object_task : public task_pool::executable_task
   {
   public:
      object_task(uint flags = cObjectTaskFlagDefault) :
         m_pObject(NULL),
         m_pMethod(NULL),
         m_flags(flags)
      {
      }
      
      typedef void (T::*object_method_ptr)(uint64 data, void* pData_ptr);
                  
      object_task(T* pObject, object_method_ptr pMethod, uint flags = cObjectTaskFlagDefault) :
         m_pObject(pObject),
         m_pMethod(pMethod),
         m_flags(flags)
      {
         CRNLIB_ASSERT(pObject && pMethod);
      }
      
      void init(T* pObject, object_method_ptr pMethod, uint flags = cObjectTaskFlagDefault)
      {
         CRNLIB_ASSERT(pObject && pMethod);
         
         m_pObject = pObject;
         m_pMethod = pMethod;
         m_flags = flags;
      }
      
      T* get_object() const { return m_pObject; }
      object_method_ptr get_method() const { return m_pMethod; }
            
      virtual void execute_task(uint64 data, void* pData_ptr)
      {
         (m_pObject->*m_pMethod)(data, pData_ptr);
         
         if (m_flags & cObjectTaskFlagDeleteAfterExecution)
            crnlib_delete(this);
      }
   
   protected:
      T* m_pObject;        
      
      object_method_ptr m_pMethod;
      
      uint m_flags;
   };
   
   template<typename S, typename T>
   inline void task_pool::queue_object_task(S* pObject, T pObject_method, uint64 data, void* pData_ptr)
   {
      queue_task(crnlib_new< object_task<S> >(pObject, pObject_method, cObjectTaskFlagDeleteAfterExecution), data, pData_ptr);
   }
 
} // namespace crnlib
