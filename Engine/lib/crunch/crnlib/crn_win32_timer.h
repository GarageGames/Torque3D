// File: crn_win32_timer.h
// See Copyright Notice and license at the end of inc/crnlib.h
#pragma once

namespace crnlib
{
   typedef uint64 timer_ticks;

   class timer
   {
   public:
      timer();
      timer(timer_ticks start_ticks);

      void start();
      void start(timer_ticks start_ticks);

      void stop();

      double get_elapsed_secs() const;
      uint64 get_elapsed_us() const;

      static void init();
      static timer_ticks get_init_ticks();
      static timer_ticks get_ticks();
      static double ticks_to_secs(timer_ticks ticks);

   private:
      static uint64 g_init_ticks;
      static uint64 g_freq;
      static double g_inv_freq;

      uint64 m_start_time;
      uint64 m_stop_time;

      bool m_started : 1;
      bool m_stopped : 1;
   };

} // namespace crnlib
