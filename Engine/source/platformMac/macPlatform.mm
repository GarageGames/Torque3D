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

#import <Cocoa/Cocoa.h>
#import <unistd.h>
#import "platform/platform.h"
#import "console/console.h"
#import "core/stringTable.h"
#import "core/util/str.h"
#import "platform/platformInput.h"
#import "platform/threads/thread.h"
#import "core/util/journal/process.h"

//-----------------------------------------------------------------------------
// Completely closes and restarts the simulation
void Platform::restartInstance()
{
   // Returns the NSBundle that corresponds to the directory where the current app executable is located.
   NSBundle* mainAppBundle = [NSBundle mainBundle];
   
   // Returns the file URL of the receiver's executable file.
   // Not currently used, but left here for reference
   //NSURL* execURL = [mainAppBundle executableURL];
   
   // Returns the full pathname of the receiver's executable file.
   NSString* execString = [mainAppBundle executablePath];
   
   // Create a mutable string we can build into an executable command
   NSMutableString* mut = [[[NSMutableString alloc] init] autorelease];
   
   // Base string is the executable path
   [mut appendString:execString];
   
   // append ampersand so that we can launch without blocking.
   // encase in quotes so that spaces in the path are accepted.
   [mut insertString:@"\"" atIndex:0];
   [mut appendString:@"\" & "];
   [mut appendString:@"\\0"];
   
   // Convert to a C string
   const char* execCString = [mut UTF8String];
   
   // Echo the command before we run it
   Con::printf("---- %s -----", execCString);
   
   // Run the restart command and hope for the best
   system(execCString);
}

void Platform::postQuitMessage(const S32 in_quitVal)
{
   Process::requestShutdown();
}

void Platform::forceShutdown(S32 returnValue)
{
   //exit(returnValue);
   [NSApp performSelector:@selector(terminate:) withObject:nil afterDelay:0.0];
}

//-----------------------------------------------------------------------------
void Platform::debugBreak()
{
   raise(SIGTRAP);
}

#pragma mark ---- Various Directories ----
//-----------------------------------------------------------------------------
const char* Platform::getUserDataDirectory() 
{
   // application support directory is most in line with the current usages of this function.
   // this may change with later usage
   // perhaps the user data directory should be pref-controlled?
   NSString *nsDataDir = [@"~/Library/Application Support/" stringByStandardizingPath];
   return StringTable->insert([nsDataDir UTF8String]);
}

//-----------------------------------------------------------------------------
const char* Platform::getUserHomeDirectory() 
{
   return StringTable->insert([[@"~/" stringByStandardizingPath] UTF8String]);
}

//-----------------------------------------------------------------------------
StringTableEntry osGetTemporaryDirectory()
{
   NSString *tdir = NSTemporaryDirectory();
   const char *path = [tdir UTF8String];
   return StringTable->insert(path);
}

#pragma mark ---- Platform utility funcs ----
//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
bool Platform::openWebBrowser( const char* webAddress )
{
   OSStatus err;
   CFURLRef url = CFURLCreateWithBytes(NULL,(UInt8*)webAddress,dStrlen(webAddress),kCFStringEncodingASCII,NULL);
   err = LSOpenCFURLRef(url,NULL);
   CFRelease(url);
   
   return(err==noErr);
}

#pragma mark ---- Administrator ----
//-----------------------------------------------------------------------------
bool Platform::getUserIsAdministrator()
{
   // if we can write to /Library, we're probably an admin
   // HACK: this is not really very good, because people can chmod Library.
   return (access("/Library", W_OK) == 0);
}
