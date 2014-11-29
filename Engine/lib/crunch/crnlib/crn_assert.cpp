// File: crn_assert.cpp
// See Copyright Notice and license at the end of inc/crnlib.h
#include "crn_core.h"
#include "crn_winhdr.h"
#include <stdio.h>

static bool g_fail_exceptions;
static bool g_exit_on_failure = true;

void crnlib_enable_fail_exceptions(bool enabled)
{
   g_fail_exceptions = enabled;
}

void crnlib_assert(const char* pExp, const char* pFile, unsigned line)
{
   char buf[512];

#if defined(WIN32) && defined(_MSC_VER)
   sprintf_s(buf, sizeof(buf), "%s(%u): Assertion failed: \"%s\"\n", pFile, line, pExp);
#else
   sprintf(buf, "%s(%u): Assertion failed: \"%s\"\n", pFile, line, pExp);
#endif

   crnlib_output_debug_string(buf);

   printf(buf);

   if (crnlib_is_debugger_present())
      crnlib_debug_break();
}

void crnlib_fail(const char* pExp, const char* pFile, unsigned line)
{
   char buf[512];

#if defined(WIN32) && defined(_MSC_VER)
   sprintf_s(buf, sizeof(buf), "%s(%u): Failure: \"%s\"\n", pFile, line, pExp);
#else
   sprintf(buf, "%s(%u): Failure: \"%s\"\n", pFile, line, pExp);
#endif

   crnlib_output_debug_string(buf);

   printf(buf);

   if (crnlib_is_debugger_present())
      crnlib_debug_break();

   if (g_fail_exceptions)
      RaiseException(CRNLIB_FAIL_EXCEPTION_CODE, 0, 0, NULL);
   else if (g_exit_on_failure)
      exit(EXIT_FAILURE);
}

void trace(const char* pFmt, va_list args)
{
   if (crnlib_is_debugger_present())
   {
      char buf[512];
#if defined(WIN32) && defined(_MSC_VER)
      vsprintf_s(buf, sizeof(buf), pFmt, args);
#else
      vsprintf(buf, pFmt, args);
#endif

      crnlib_output_debug_string(buf);
   }
};

void trace(const char* pFmt, ...)
{
   va_list args;
   va_start(args, pFmt);
   trace(pFmt, args);
   va_end(args);
};
