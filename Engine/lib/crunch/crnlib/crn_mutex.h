// File: crn_mutex.h
// See Copyright Notice and license at the end of inc/crnlib.h
#pragma once

namespace crnlib
{
   class mutex
   {
      mutex(const mutex&);
      mutex& operator= (const mutex&);
      
   public:
      mutex(unsigned int spin_count = 0);
      ~mutex();
      void lock();
      void unlock();
      void set_spin_count(unsigned int count);
   
   private:
      int m_buf[12];
      
#ifdef CRNLIB_BUILD_DEBUG
      unsigned int m_lock_count;
#endif      
   };
   
   class scoped_mutex
   {
      scoped_mutex(const scoped_mutex&);
      scoped_mutex& operator= (const scoped_mutex&);

   public:
      inline scoped_mutex(mutex& m) : m_mutex(m) { m_mutex.lock(); }
      inline ~scoped_mutex() { m_mutex.unlock(); }

   private:
      mutex& m_mutex;
   };

} // namespace crnlib   
