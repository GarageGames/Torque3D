// File: crn_event.h
// See Copyright Notice and license at the end of inc/crnlib.h
#pragma once

namespace crnlib
{
   class event
   {
      CRNLIB_NO_COPY_OR_ASSIGNMENT_OP(event);
      
   public:
      event(bool manual_reset = false, bool initial_state = false, const char* pName = NULL);
      ~event();

      inline void *get_handle(void) const { return m_handle; }   

      void set(void);
      void reset(void);
      void pulse(void);
      bool wait(uint32 milliseconds = UINT32_MAX);
     
   private:
      void *m_handle;
   };

} // namespace crnlib

