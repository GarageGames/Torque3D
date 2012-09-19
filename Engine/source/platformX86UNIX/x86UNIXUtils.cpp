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

#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <sys/utsname.h>

// for UnixCommandExecutor
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "platformX86UNIX/platformX86UNIX.h"
#include "platformX86UNIX/x86UNIXUtils.h"

UnixUtils *UUtils = NULL;
UnixUtils utils; 

UnixUtils::UnixUtils()
{
   UUtils = this;

   mUnameInfo = (struct utsname*)dRealMalloc(sizeof(struct utsname));;
   if (uname(mUnameInfo) == -1)
   {
      // oh well
      dRealFree(mUnameInfo);
      mUnameInfo = NULL;
   }
}

UnixUtils::~UnixUtils()
{
   if (mUnameInfo != NULL)
   {
      dRealFree(mUnameInfo);
      mUnameInfo = NULL;
   }
}

const char* UnixUtils::getOSName()
{
   if (mUnameInfo == NULL)
      return "";

   return mUnameInfo->sysname;  
}

bool UnixUtils::inBackground()
{
   int terminalGroupId = tcgetpgrp(fileno(stdin));
   int myPid = getpid();
   if (terminalGroupId != myPid)
      return true;
   else
      return false;
}

//-----------------------------------------------------------------------------
// UnixCommandExecutor 
void UnixCommandExecutor::clearFields()
{
   mRet = -1;
   mStdoutSave = -1;
   mStderrSave = -1;
   mPipeFiledes[0] = -1;
   mPipeFiledes[1] = -1;
   mChildPID = -1;
   mBytesRead = 0;
   mStdoutClosed = false;
   mStderrClosed = false;
   mChildExited = false;
}

UnixCommandExecutor::UnixCommandExecutor()
{
   clearFields();
}

UnixCommandExecutor::~UnixCommandExecutor()
{
   cleanup();
}

int UnixCommandExecutor::exec(char* args[], 
                              char* stdoutCapture, int stdoutCaptureSize)
{
   // check for shitty parameters
   if (args == NULL || stdoutCapture == NULL || 
       stdoutCaptureSize <= 0)
      return -666;

   // we're going to be redirecting stdout, so save it so that we can 
   // restore it
   mRet = dup(1);
   if (mRet == -1)
   {
      cleanup();
      return mRet;
   }
   mStdoutSave = mRet;

   // save stderr
   mRet = dup(2);
   if (mRet == -1)
   {
      cleanup();
      return mRet;
   }
   mStderrSave = mRet;

   // we'll need some pipe action for communicating with subprocess
   mRet = pipe(mPipeFiledes);
   if (mRet == -1)
   {
      cleanup();
      return mRet;
   }

   // close stdout
   mRet = close(1);
   if (mRet == -1)
   {
      cleanup();
      return mRet;
   }
   mStdoutClosed = true;

   // stderr just gets closed and the output discarded
   mRet = close(2);
   if (mRet == -1)
   {
      cleanup();
      return mRet;
   }
   mStderrClosed = true;

   // dup the pipe output into stdout
   mRet = dup2(mPipeFiledes[1], 1);
   if (mRet == -1)
   {
      cleanup();
      return mRet;
   }

   // fork
   mRet = fork();
   if (mRet == -1)
   {
      cleanup();
      return mRet;
   }
         
   if (mRet == 0)
   {
      // child process

      //close(mPipeFiledes[0]);
      mRet = execvp(args[0], args);
      // if exec returns, some bad shit went down, so just
      // get outta here
      exit(mRet);
   }

   // parent process
   mChildPID = mRet;

   // need to suck in data from pipe while child is running, 
   // otherwise child will eventually block on write and we'll
   // wait forever
   memset(stdoutCapture, 0, stdoutCaptureSize);

   // set input to be non blocking so that we don't block on read
   mRet = fcntl(mPipeFiledes[0], F_SETFL, O_NONBLOCK);
   if (mRet == -1)
   {
      cleanup();
      return mRet;
   }

   // check to see if child has exited
   mRet = waitpid(mChildPID, NULL, WNOHANG);
   while (mRet == 0)
   {
      // not exited, read some data
      mRet = read(mPipeFiledes[0], stdoutCapture + mBytesRead, 
                  stdoutCaptureSize - mBytesRead);
      // any error that isn't EAGAIN means we should exit
      if (mRet == -1 && errno != EAGAIN)
      {
         cleanup();
         return mRet;
      }

      // if the read was ok, increment bytes read
      if (mRet != -1)
         mBytesRead += mRet;

      // check again for child exit
      mRet = waitpid(mChildPID, NULL, WNOHANG);    
   }

   // check for error from waitpid
   if (mRet == -1 && errno != ECHILD)
   {
      cleanup();
      return mRet;
   }

   // if we get here, the child exited
   mChildExited = true;

   // read final bit of data
   mRet = read(mPipeFiledes[0], stdoutCapture + mBytesRead, 
               stdoutCaptureSize - mBytesRead);
   if (mRet == -1 && errno != EAGAIN)
   {
      cleanup();
      return mRet;
   }

   if (mRet != -1)
      mBytesRead += mRet;

   // done...cleanup
   cleanup();

   return 0;
}

void UnixCommandExecutor::cleanup()
{
   // if child spawned and not exited, wait
   if (mChildPID > 0 && !mChildExited)
      waitpid(mChildPID, NULL, 0);
   // close pipe descriptors
   if (mPipeFiledes[0] != -1)
      close(mPipeFiledes[0]);
   if (mPipeFiledes[1] != -1)
      close(mPipeFiledes[1]);
   // if stdout is redirected, restore it
   if (mStdoutClosed && mStdoutSave != -1)
      dup2(mStdoutSave, 1);
   // close stdout save descriptor
   if (mStdoutSave != -1)
      close(mStdoutSave);
   // if stderr is redirected, restore it
   if (mStderrClosed && mStderrSave != -1)
      dup2(mStderrSave, 2);
   // close stderr save descriptor
   if (mStderrSave != -1)
      close(mStderrSave);

   clearFields();
}

/* Usage:
   UnixCommandExecutor exec;
   char* args[] = { "ps", "-aux", NULL };
   char data[20000];
   int ret = exec.exec(args, data, sizeof(data));
   printf("%s", data);
*/
