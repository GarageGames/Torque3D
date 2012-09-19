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
#import <Carbon/Carbon.h>
#include <unistd.h>
#include "platform/platform.h"
#include "console/console.h"
#include "core/stringTable.h"
#include "platform/platformInput.h"
#include "platform/threads/thread.h"

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

#pragma mark ---- Administrator ----
//-----------------------------------------------------------------------------
bool Platform::getUserIsAdministrator()
{
   // if we can write to /Library, we're probably an admin
   // HACK: this is not really very good, because people can chmod Library.
   return (access("/Library", W_OK) == 0);
}

#pragma mark ---- Cosmetic ----
//-----------------------------------------------------------------------------
bool Platform::displaySplashWindow()
{
    return false;
}

#pragma mark ---- File IO ----
//-----------------------------------------------------------------------------
bool dPathCopy(const char* source, const char* dest, bool nooverwrite)
{
   NSFileManager *manager = [NSFileManager defaultManager];
   NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
   
   NSString *nsource = [[NSString stringWithUTF8String:source] stringByStandardizingPath];
   NSString *ndest   = [[NSString stringWithUTF8String:dest] stringByStandardizingPath];
   NSString *ndestFolder = [ndest stringByDeletingLastPathComponent];
   
   if(! [manager fileExistsAtPath:nsource])
   {
      Con::errorf("dPathCopy: no file exists at %s",source);
      return false;
   }
   
   if( [manager fileExistsAtPath:ndest] )
   {
      if(nooverwrite)
      {
         Con::errorf("dPathCopy: file already exists at %s",dest);
         return false;
      }
      Con::warnf("Deleting files at path: %s", dest);
      bool deleted = [manager removeFileAtPath:ndest handler:nil];
      if(!deleted)
      {
         Con::errorf("Copy failed! Could not delete files at path: %s", dest);
         return false;
      }
   }
   
   if([manager fileExistsAtPath:ndestFolder] == NO)
   {
      ndestFolder = [ndestFolder stringByAppendingString:@"/"]; // createpath requires a trailing slash
      Platform::createPath([ndestFolder UTF8String]);
   }
   
   bool ret = [manager copyPath:nsource toPath:ndest handler:nil];
   
   [pool release];
   return ret;
   
}

//-----------------------------------------------------------------------------
bool dFileRename(const char *source, const char *dest)
{
   if(source == NULL || dest == NULL)
      return false;
      
   NSFileManager *manager = [NSFileManager defaultManager];
   
   NSString *nsource = [manager stringWithFileSystemRepresentation:source length:dStrlen(source)];
   NSString *ndest   = [manager stringWithFileSystemRepresentation:dest length:dStrlen(dest)];
   
   if(! [manager fileExistsAtPath:nsource])
   {
      Con::errorf("dFileRename: no file exists at %s",source);
      return false;
   }
   
   if( [manager fileExistsAtPath:ndest] )
   {
      Con::warnf("dFileRename: Deleting files at path: %s", dest);
   }
   
   bool ret = [manager movePath:nsource toPath:ndest handler:nil];
  
   return ret;
}

#pragma mark -
#pragma mark ---- ShellExecute ----
class ExecuteThread : public Thread
{
   const char* zargs;
   const char* directory;
   const char* executable;
public:
   ExecuteThread(const char *_executable, const char *_args /* = NULL */, const char *_directory /* = NULL */) : Thread(0, NULL, false, true)
   {
      zargs = dStrdup(_args);
      directory = dStrdup(_directory);
      executable = dStrdup(_executable);
      start();
   }
   
   virtual void run(void* arg);
};

static char* _unDoubleQuote(char* arg)
{
   U32 len = dStrlen(arg);
   if(!len)
      return arg;
   
   if(arg[0] == '"' && arg[len-1] == '"')
   {
      arg[len - 1] = '\0';
      return arg + 1;
   }
   return arg;
}

void ExecuteThread::run(void* arg)
{
// 2k should be enough. if it's not, bail.
//   char buf[2048];
//   U32 len = dSprintf(buf, sizeof(buf), "%s %s -workingDir %s", executable, args, directory);
//   if( len >= sizeof(buf))
//   {
//      Con::errorf("shellExecute(): the command was too long, and won't be run.");
//      return;
//   }
//   // calls sh with the string and blocks until the command returns.
//   system(buf);

   // FIXME: there is absolutely no error checking in here.
   printf("creating nstask\n");
   NSTask *aTask = [[NSTask alloc] init];
   NSMutableArray *array = [NSMutableArray array];

   // scan the args list, breaking it up, space delimited, backslash escaped.
   U32 len = dStrlen(zargs);
   char args[len+1];
   dStrncpy(args, zargs, len+1);
   char *lastarg = args;
   bool escaping = false;
   for(int i = 0; i< len; i++)
   {
      char c = args[i];
      // a backslash escapes the next character
      if(escaping)      
         continue;
      if(c == '\\')
         escaping = true;
      
      if(c == ' ')
      {
         args[i] = '\0';
         if(*lastarg)
            [array addObject:[NSString stringWithUTF8String: _unDoubleQuote(lastarg)]];
         lastarg = args + i + 1;
      }
   }
   if(*lastarg)
      [array addObject:[NSString stringWithUTF8String: _unDoubleQuote(lastarg)]];

   [aTask setArguments: array];
     
   [aTask setCurrentDirectoryPath:[NSString stringWithUTF8String: this->directory]];
   [aTask setLaunchPath:[NSString stringWithUTF8String:executable]];
   [aTask launch];
   [aTask waitUntilExit];
   U32 ret = [aTask terminationStatus];
   Con::executef("onExecuteDone", Con::getIntArg(ret));
   printf("done nstask\n");
}

ConsoleFunction(shellExecute, bool, 2, 4, "(executable, [args], [directory])")
{
   ExecuteThread *et = new ExecuteThread(argv[1], argc > 2 ? argv[2] : NULL, argc > 3 ? argv[3] : NULL);
   TORQUE_UNUSED(et);
   return true; // Bug: BPNC error: need feedback on whether the command was sucessful
}
