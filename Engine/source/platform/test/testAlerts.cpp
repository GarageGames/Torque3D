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

#include "platform/platform.h"
#include "unit/test.h"

using namespace UnitTesting;

CreateInteractiveTest(CheckPlatformAlerts, "Platform/Alerts")
{
   void run()
   {
      // Run through all the alert types.
      Platform::AlertOK("Test #1 - AlertOK", "This is a test of Platform::AlertOK. I am a blocking dialog with an OK button. Please hit OK to continue.");
      test(true, "AlertOK should return when the user clicks on it..."); // <-- gratuitous test point.
      
      bool res;
      
      res = Platform::AlertOKCancel("Test #2 - AlertOKCancel", "This is a test of Platform::alertOKCancel. I am a blocking dialog with an OK and a Cancel button. Please hit Cancel to continue.");
      test(res==false,"AlertOKCancel - Didn't get cancel. User error, or just bad code?");
      
      res = Platform::AlertOKCancel("Test #3 - AlertOKCancel", "This is a test of Platform::alertOKCancel. I am a blocking dialog with an OK and a Cancel button. Please hit OK to continue.");
      test(res==true,"AlertOKCancel - Didn't get ok. User error, or just bad code?");
      
      res = Platform::AlertRetry("Test #4 - AlertRetry", "This is a test of Platform::AlertRetry. I am a blocking dialog with an Retry and a Cancel button. Please hit Retry to continue.");
      test(res==true,"AlertRetry - Didn't get retry. User error, or just bad code?");

      res = Platform::AlertRetry("Test #5 - AlertRetry", "This is a test of Platform::AlertRetry. I am a blocking dialog with an Retry and a Cancel button. Please hit Cancel to continue.");
      test(res==false,"AlertRetry - Didn't get cancel. User error, or just bad code?");
   }
};