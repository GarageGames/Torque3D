// File: crn_platform.h
// See Copyright Notice and license at the end of inc/crnlib.h
#pragma once

#ifdef CRNLIB_PLATFORM_PC
   const bool c_crnlib_little_endian_platform = true;
#else
   const bool c_crnlib_little_endian_platform = false;
#endif   

const bool c_crnlib_big_endian_platform = !c_crnlib_little_endian_platform;

inline bool crnlib_is_little_endian() { return c_crnlib_little_endian_platform; }
inline bool crnlib_is_big_endian() { return c_crnlib_big_endian_platform; }

inline bool crnlib_is_pc() 
{
#ifdef CRNLIB_PLATFORM_PC
   return true;
#else
   return false;
#endif
}

inline bool crnlib_is_x86() 
{
#ifdef CRNLIB_PLATFORM_PC_X86
   return true;
#else
   return false;
#endif
}

inline bool crnlib_is_x64() 
{
#ifdef CRNLIB_PLATFORM_PC_X64
   return true;
#else
   return false;
#endif
}

bool crnlib_is_debugger_present(void);
void crnlib_debug_break(void);
void crnlib_output_debug_string(const char* p);

// actually in crnlib_assert.cpp
void crnlib_assert(const char* pExp, const char* pFile, unsigned line);
void crnlib_fail(const char* pExp, const char* pFile, unsigned line);
