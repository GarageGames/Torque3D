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
#include "platform/nativeDialogs/msgBox.h"
#include "console/console.h"

void Platform::AlertOK(const char *windowTitle, const char *message)
{
   Platform::messageBox(windowTitle, message, MBOk, MIInformation);
}

//--------------------------------------
bool Platform::AlertOKCancel(const char *windowTitle, const char *message)
{
   return ( Platform::messageBox(windowTitle, message, MBOkCancel, MIInformation) == MROk );
}

//--------------------------------------
bool Platform::AlertRetry(const char *windowTitle, const char *message)
{
   return ( Platform::messageBox(windowTitle, message, MBRetryCancel, MIInformation) == MRRetry );
}

namespace MsgBoxMac
{
   struct _NSStringMap
   {
      S32 num;
      NSString* ok;
      NSString* cancel;
      NSString* third;
   };

   static _NSStringMap sgButtonTextMap[] =
   {
      { MBOk,                 @"OK",    nil,        nil },
      { MBOkCancel,           @"OK",    @"Cancel",  nil },
      { MBRetryCancel,        @"Retry", @"Cancel",  nil },
      { MBSaveDontSave,       @"Yes",  @"No", nil },
      { MBSaveDontSaveCancel, @"Yes",  @"No",  @"Cancel" },
      { -1, nil, nil, nil }
   };

   struct _NSAlertResultMap
   {
      S32 num;
      S32 ok;
      S32 cancel;
      S32 third;
   };

   static _NSAlertResultMap sgAlertResultMap[] = 
   {
      { MBOk,                 MROk,    0,          0 },
      { MBOkCancel,           MROk,    MRCancel,   0 },
      { MBRetryCancel,        MRRetry, MRCancel,   0 },
      { MBSaveDontSave,       MROk,    MRDontSave, 0 },
      { MBSaveDontSaveCancel, MROk,    MRDontSave,   MRCancel },
      { -1, nil, nil, nil }
   };
} // end MsgBoxMac namespace

//-----------------------------------------------------------------------------
S32 Platform::messageBox(const UTF8 *title, const UTF8 *message, MBButtons buttons, MBIcons icon)
{
// TODO: put this on the main thread

   // determine the button text
   NSString *okBtn      = nil;
   NSString *cancelBtn  = nil;
   NSString *thirdBtn   = nil;
   U32 i;
   for(i = 0; MsgBoxMac::sgButtonTextMap[i].num != -1; i++)
   {
      if(MsgBoxMac::sgButtonTextMap[i].num != buttons)
         continue;

      okBtn = MsgBoxMac::sgButtonTextMap[i].ok;
      cancelBtn = MsgBoxMac::sgButtonTextMap[i].cancel;
      thirdBtn = MsgBoxMac::sgButtonTextMap[i].third;
      break;
   }
   if(MsgBoxMac::sgButtonTextMap[i].num == -1)
      Con::errorf("Unknown message box button set requested. Mac Platform::messageBox() probably needs to be updated.");
   
   // convert title and message to NSStrings
   NSString *nsTitle = [NSString stringWithUTF8String:title];
   NSString *nsMessage = [NSString stringWithUTF8String:message];
   // TODO: ensure that the cursor is the expected shape
   // show the alert
   S32 result = -2;
   
   NSAlert *alert = [NSAlert alertWithMessageText:nsTitle
                                    defaultButton:okBtn
                                  alternateButton:thirdBtn
                                      otherButton:cancelBtn
                        informativeTextWithFormat:nsMessage];
   
   switch(icon)
   {
      // TODO:
      // Currently, NSAlert only provides two alert icon options.  
      // NSWarningAlertStyle and NSInformationalAlertStyle are identical and
      // display the application icon, while NSCriticalAlertStyle displays
      // a shrunken app icon badge on a yellow-triangle-with-a-bang icon.
      // If custom icons were created, they could be used here with the
      // message [alert setIcon:foo]
      case MIWarning:     // MIWarning = 0
         
      case MIQuestion:    // MIquestion = 3
         [alert setAlertStyle:NSWarningAlertStyle];
         break;
         
      case MIInformation: // MIInformation = 1
         [alert setAlertStyle:NSInformationalAlertStyle];
         break;
         
      case MIStop:        // MIStop = 3
         [alert setAlertStyle:NSCriticalAlertStyle];
         break;
         
      default:
         Con::errorf("Unknown message box icon requested. Mac Platform::messageBox() probably needs to be updated.");
   }
   
   id appDelegate = [NSApp delegate];
   [NSApp setDelegate: nil];
   
   U32 cursorDepth = 0;
   
   while(!CGCursorIsVisible())
   {
      CGDisplayShowCursor(kCGDirectMainDisplay);
      cursorDepth++;
   }
   
   CGAssociateMouseAndMouseCursorPosition(true);
   result = [alert runModal];

   [NSApp setDelegate: appDelegate];
   
   S32 ret = 0;
   for(U32 i = 0; MsgBoxMac::sgAlertResultMap[i].num != -1; i++)
   {
      if(MsgBoxMac::sgAlertResultMap[i].num != buttons)
         continue;
      
      switch(result)
      {
         case NSAlertDefaultReturn:
            ret = MsgBoxMac::sgAlertResultMap[i].ok;     break;
         case NSAlertOtherReturn:
            ret = MsgBoxMac::sgAlertResultMap[i].cancel; break;
         case NSAlertAlternateReturn:
            ret = MsgBoxMac::sgAlertResultMap[i].third;  break;
      }
   }
   
   return ret;
}