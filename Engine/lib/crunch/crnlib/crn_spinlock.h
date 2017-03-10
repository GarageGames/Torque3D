// File: crn_spinlock.h
// See Copyright Notice and license at the end of inc/crnlib.h
#pragma once

namespace crnlib
{
   // Simple non-recursive spinlock.   
   class spinlock
   {
   public:
      inline spinlock() : m_flag(0) { }
            
      void lock(uint32 max_spins = 4096, bool yielding = true, bool memoryBarrier = true);
      
      inline void lock_no_barrier(uint32 max_spins = 4096, bool yielding = true) { lock(max_spins, yielding, false); }

      void unlock();
            
      inline void unlock_no_barrier() { m_flag = CRNLIB_FALSE; }

   private:
      volatile int32 m_flag;
   };

   class scoped_spinlock
   {
      scoped_spinlock(const scoped_spinlock&);
      scoped_spinlock& operator= (const scoped_spinlock&);

   public:
      inline scoped_spinlock(spinlock& lock) : m_lock(lock) { m_lock.lock(); }
      inline ~scoped_spinlock() { m_lock.unlock(); }

   private:
      spinlock& m_lock;
   };
   
} // namespace crnlib
