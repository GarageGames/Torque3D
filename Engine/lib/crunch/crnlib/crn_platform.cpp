// File: crn_platform.cpp
// See Copyright Notice and license at the end of inc/crnlib.h
#include "crn_core.h"
#include "crn_winhdr.h"

bool crnlib_is_debugger_present(void)
{
   return IsDebuggerPresent() != 0;
}

void crnlib_debug_break(void)
{
   DebugBreak();
}

void crnlib_output_debug_string(const char* p)
{
   OutputDebugStringA(p);
}
