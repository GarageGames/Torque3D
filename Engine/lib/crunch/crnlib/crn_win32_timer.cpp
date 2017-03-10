// File: crn_win32_timer.cpp
// See Copyright Notice and license at the end of inc/crnlib.h
#include "crn_core.h"
#include "crn_win32_timer.h"
#include "crn_winhdr.h"

namespace crnlib
{
   uint64 timer::g_init_ticks;
   uint64 timer::g_freq;
   double timer::g_inv_freq;

   timer::timer() :
      m_start_time(0),
      m_stop_time(0),
      m_started(false),
      m_stopped(false)
   {
      if (!g_inv_freq) init();
   }
   
   timer::timer(timer_ticks start_ticks)
   {
      if (!g_inv_freq) init();
      
      m_start_time = start_ticks;
      
      m_started = true;
      m_stopped = false;
   }
   
   void timer::start(timer_ticks start_ticks)
   {
      m_start_time = start_ticks;
      
      m_started = true;
      m_stopped = false;
   }

   void timer::start()
   {
      QueryPerformanceCounter((LARGE_INTEGER*)&m_start_time);
      
      m_started = true;
      m_stopped = false;
   }
   
   void timer::stop()
   {
      CRNLIB_ASSERT(m_started);
                  
      QueryPerformanceCounter((LARGE_INTEGER*)&m_stop_time);
      
      m_stopped = true;
   }

   double timer::get_elapsed_secs() const
   {
      CRNLIB_ASSERT(m_started);
      if (!m_started)
         return 0;
      
      uint64 stop_time = m_stop_time;
      if (!m_stopped)
         QueryPerformanceCounter((LARGE_INTEGER*)&stop_time);
         
      uint64 delta = stop_time - m_start_time;
      return delta * g_inv_freq;
   }
   
   uint64 timer::get_elapsed_us() const
   {
      CRNLIB_ASSERT(m_started);
      if (!m_started)
         return 0;
         
      uint64 stop_time = m_stop_time;
      if (!m_stopped)
         QueryPerformanceCounter((LARGE_INTEGER*)&stop_time);
      
      uint64 delta = stop_time - m_start_time;
      return (delta * 1000000ULL + (g_freq >> 1U)) / g_freq;      
   }
   
   void timer::init()
   {
      if (!g_inv_freq)
      {
         QueryPerformanceFrequency((LARGE_INTEGER*)&g_freq);
         g_inv_freq = 1.0f / g_freq;
         
         QueryPerformanceCounter((LARGE_INTEGER*)&g_init_ticks);
      }
   }
   
   timer_ticks timer::get_init_ticks()
   {
      if (!g_inv_freq) init();
      
      return g_init_ticks;
   }
   
   timer_ticks timer::get_ticks()
   {
      if (!g_inv_freq) init();
      
      timer_ticks ticks;
      QueryPerformanceCounter((LARGE_INTEGER*)&ticks);
      return ticks;
   }
   
   double timer::ticks_to_secs(timer_ticks ticks)
   {
      if (!g_inv_freq) init();
      
      return ticks * g_inv_freq;
   }

} // namespace crnlib
