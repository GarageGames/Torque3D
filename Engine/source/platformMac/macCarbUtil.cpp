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

#include <ApplicationServices/ApplicationServices.h>
#include <CoreServices/CoreServices.h>

#include "platform/platform.h"
#include "core/strings/stringFunctions.h"

void Platform::outputDebugString( const char *string, ... )
{
#ifdef TORQUE_DEBUG
   char buffer[ 2048 ];
   
   va_list args;
   va_start( args, string );
   
   dVsprintf( buffer, sizeof( buffer ), string, args );
   va_end( args );

   U32 length = strlen( buffer );
   if( length == ( sizeof( buffer ) - 1 ) )
      length --;

   buffer[ length ]     = '\n';
   buffer[ length + 1 ] = '\0';

   fputs( buffer, stderr );
   fflush(stderr);
#endif
}

#pragma mark ---- Platform utility funcs ----
//--------------------------------------
// Web browser function:
//--------------------------------------
bool Platform::openWebBrowser( const char* webAddress )
{
   OSStatus err;
   CFURLRef url = CFURLCreateWithBytes(NULL,(UInt8*)webAddress,dStrlen(webAddress),kCFStringEncodingASCII,NULL);
   err = LSOpenCFURLRef(url,NULL);
   CFRelease(url);

   return(err==noErr);
}

