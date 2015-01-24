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

#include "platformWin32/platformWin32.h"
#include "core/strings/stringFunctions.h"
#include "core/util/journal/process.h"

void Platform::postQuitMessage(const S32 in_quitVal)
{
   if (!Platform::getWebDeployment())
      Process::requestShutdown();
}

void Platform::debugBreak()
{
    DebugBreak();
}

void Platform::forceShutdown(S32 returnValue)
{
   // Don't do an ExitProcess here or you'll wreak havoc in a multithreaded
   // environment.

   exit( returnValue );
}

void Platform::outputDebugString( const char *string, ... )
{
   // Expand string.

   char buffer[ 2048 ];
   
   va_list args;
   va_start( args, string );
   
   dVsprintf( buffer, sizeof( buffer ), string, args );
   va_end( args );

   // Append a newline to buffer.  This is better than calling OutputDebugStringA
   // twice as in a multi-threaded environment, some other thread may output some
   // stuff in between the two calls.

   U32 length = strlen( buffer );
   if( length == ( sizeof( buffer ) - 1 ) )
      length --;

   buffer[ length ]     = '\n';
   buffer[ length + 1 ] = '\0';

   OutputDebugStringA( buffer );
}

