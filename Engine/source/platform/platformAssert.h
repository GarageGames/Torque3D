//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef _PLATFORMASSERT_H_
#define _PLATFORMASSERT_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

class PlatformAssert
{
public:
   enum Type 
   {
      Warning   = 3,
      Fatal     = 2,
      Fatal_ISV = 1
   };

private:
   static PlatformAssert *platformAssert;
   bool processing;

   virtual bool displayMessageBox(const char *title, const char *message, bool retry);
   virtual bool process(Type         assertType,
                const char*  filename,
                U32          lineNumber,
                const char*  message);

   PlatformAssert();
   virtual ~PlatformAssert();

public:
   static void create( PlatformAssert* newAssertClass = NULL );
   static void destroy();
   static bool processAssert(Type         assertType,
                             const char*  filename,
                             U32          lineNumber,
                             const char*  message);
   static char *message(const char *message, ...);
   static bool processingAssert();
};


#ifdef TORQUE_ENABLE_ASSERTS
   /*!
      Assert that the statement x is true, and continue processing.

      If the statment x is true, continue processing.

      If the statement x is false, log the file and line where the assert occured,
      the message y and continue processing.

      These asserts are only present in DEBUG builds.
    */
   #define AssertWarn(x, y)      \
         { if ((x)==0) \
            ::PlatformAssert::processAssert(::PlatformAssert::Warning, __FILE__, __LINE__,  y); }

   /*!
      Assert that the statement x is true, otherwise halt.

      If the statement x is true, continue processing.

      If the statement x is false, log the file and line where the assert occured,
      the message y and displaying a dialog containing the message y. The user then
      has the option to halt or continue causing the debugger to break.

      These asserts are only present in DEBUG builds.

      This assert is very useful for verifying data as well as function entry and
      exit conditions.
    */
   #define AssertFatal(x, y)         \
         { if (((bool)(x))==(bool)0) \
            { if ( ::PlatformAssert::processAssert(::PlatformAssert::Fatal, __FILE__, __LINE__,  y) ) { ::Platform::debugBreak(); } } }

#else
   #define AssertFatal(x, y)   { (void)sizeof(x); (void)sizeof(y); }
   #define AssertWarn(x, y)    { (void)sizeof(x); (void)sizeof(y); }
#endif

/*!
   Assert (In Shipping Version) that the statement x is true, otherwise halt.

   If the statement x is true, continue processing.

   If the statement x is false, log the file and line where the assert occurred,
   the message y and exit the program displaying a dialog containing the message y.
   These asserts are present in both OPTIMIZED and DEBUG builds.

   This assert should only be used for rare conditions where the application cannot continue
   execution without seg-faulting and you want to display a nice exit message.
 */
#define AssertISV(x, y)  \
   { if ((x)==0)         \
{ if ( ::PlatformAssert::processAssert(::PlatformAssert::Fatal_ISV, __FILE__, __LINE__,  y) ) { ::Platform::debugBreak(); } } }


/*!
   Sprintf style string formating into a fixed temporary buffer.
   @param   in_msg sprintf style format string
   @returns pointer to fixed buffer containing formatted string

   \b Example:
   \code
   U8 a = 5;
   S16 b = -10;
   char *output = avar("hello %s! a=%u, b=%d", "world");
   ouput = "hello world! a=5, b=-10"
   \endcode

   @warning avar uses a static fixed buffer.  Treat the buffer as volatile data
   and use it immediately.  Other functions my use avar too and clobber the buffer.
 */
const char* avar(const char *in_msg, ...);



#endif // _PLATFORM_ASSERT_H_

