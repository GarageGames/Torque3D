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

#ifndef _X86UNIXUTILS_H_
#define _X86UNIXUTILS_H_

struct utsname;

class UnixUtils
{
public:
   UnixUtils();
   virtual ~UnixUtils();

   /**
      Returns true if we're running in the background, false otherwise.
      There's no "standard" way to determine this in unix, but 
      modern job control unices should support the method described
      here:

      http://www.faqs.org/faqs/unix-faq/faq/part3/

      (question 3.7)
    */
   bool inBackground();

   /**
      Returns the name of the OS, as reported by uname.
    */
   const char* getOSName();

private:
   struct utsname* mUnameInfo;
};

extern UnixUtils *UUtils;

// utility class for running a unix command and capturing its output
class UnixCommandExecutor
{
   private:
      int mRet;
      int mStdoutSave;
      int mStderrSave;
      int mPipeFiledes[2];
      int mChildPID;
      int mBytesRead;
      bool mStdoutClosed;
      bool mStderrClosed;
      bool mChildExited;

      void clearFields();
      void cleanup();

   public:
      UnixCommandExecutor();
      ~UnixCommandExecutor();

      // Runs the specified command.  
      // - args is a null terminated list of the command and its arguments,
      // e.g: "ps", "-aux", NULL
      // - stdoutCapture is the buffer where stdout data will be stored
      // - stdoutCaptureSize is the size of the buffer
      // None of these parameters may be null.  stdoutCaptureSize must be > 0
      //
      // returns -2 if the parameters are bad.  returns -1 if some other
      // error occurs, check errno for the exact error.
      int exec(char* args[], char* stdoutCapture, int stdoutCaptureSize);
};

#endif
