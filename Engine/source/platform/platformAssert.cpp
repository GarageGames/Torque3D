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

#include <stdarg.h>

#include "core/strings/stringFunctions.h"
#include "console/console.h"


//-------------------------------------- STATIC Declaration
PlatformAssert *PlatformAssert::platformAssert = NULL;

//--------------------------------------
PlatformAssert::PlatformAssert()
{
   processing = false;
   ignoreAll = false;
}

//--------------------------------------
PlatformAssert::~PlatformAssert()
{
}

//--------------------------------------
void PlatformAssert::create( PlatformAssert* newAssertClass )
{
   if (!platformAssert)
      platformAssert = newAssertClass ? newAssertClass : new PlatformAssert;
}


//--------------------------------------
void PlatformAssert::destroy()
{
   if (platformAssert)
      delete platformAssert;
   platformAssert = NULL;
}


//--------------------------------------
bool PlatformAssert::displayMessageBox(const char *title, const char *message, bool retry)
{
   if (retry)
      return Platform::AlertRetry(title, message);

   Platform::AlertOK(title, message);
   return false;
}

static const char *typeName[] = { "Unknown", "Fatal-ISV", "Fatal", "Warning" };

//--------------------------------------

bool PlatformAssert::process(Type         assertType,
                             const char  *filename,
                             U32          lineNumber,
                             const char  *message)
{
    // If we're somehow recursing, just die.
    if(processing)
        Platform::debugBreak();
    
    processing = true;
    bool ret = false;
    
    // always dump to the Assert to the Console
    if (Con::isActive())
    {
        if (assertType == Warning)
            Con::warnf(ConsoleLogEntry::Assert, "%s(%ld,0): {%s} - %s", filename, lineNumber, typeName[assertType], message);
        else
            Con::errorf(ConsoleLogEntry::Assert, "%s(%ld,0): {%s} - %s", filename, lineNumber, typeName[assertType], message);
    }
    
    // if not a WARNING pop-up a dialog box
    if (assertType != Warning)
    {
        // used for processing navGraphs (an assert won't botch the whole build)
        if(Con::getBoolVariable("$FP::DisableAsserts", false) == true)
            Platform::forceShutdown(1);
        
        char buffer[2048];
        dSprintf(buffer, 2048, "%s: (%s @ %ld)", typeName[assertType], filename, lineNumber);
        if( !ignoreAll )
        {
            // Display message box with Debug, Ignore, Ignore All, and Exit options
            switch( Platform::AlertAssert(buffer, message) )
            {
                case Platform::ALERT_ASSERT_DEBUG:				
                    ret = true;
                    break;
                case Platform::ALERT_ASSERT_IGNORE:
                    ret = false;
                    break;
                case Platform::ALERT_ASSERT_IGNORE_ALL:
                    ignoreAll = true;
                    ret = false;
                    break;
                default:
                case Platform::ALERT_ASSERT_EXIT:
                    Platform::forceShutdown(1);
                    break;
            }
        }
    }
    
    processing = false;
    
    return ret;
}

bool PlatformAssert::processingAssert()
{
   return platformAssert ? platformAssert->processing : false;
}

//--------------------------------------
bool PlatformAssert::processAssert(Type        assertType,
                                   const char  *filename,
                                   U32         lineNumber,
                                   const char  *message)
{
   if (platformAssert)
      return platformAssert->process(assertType, filename, lineNumber, message);
   else // when platAssert NULL (during _start/_exit) try direct output...
      dPrintf("\n%s: (%s @ %ld) %s\n", typeName[assertType], filename, lineNumber, message);

   // this could also be platform-specific: OutputDebugString on PC, DebugStr on Mac.
   // Will raw printfs do the job?  In the worst case, it's a break-pointable line of code.
   // would have preferred Con but due to race conditions, it might not be around...
   // Con::errorf(ConsoleLogEntry::Assert, "%s: (%s @ %ld) %s", typeName[assertType], filename, lineNumber, message);

   return true;
}

//--------------------------------------
const char* avar(const char *message, ...)
{
   static char buffer[4096];
   va_list args;
   va_start(args, message);
   dVsprintf(buffer, sizeof(buffer), message, args);
   va_end(args);
   return( buffer );
}

//-----------------------------------------------------------------------------

ConsoleFunction( Assert, void, 3, 3, "(condition, message) - Fatal Script Assertion" )
{
    // Process Assertion.
    AssertISV( dAtob(argv[1]), argv[2] );
}
