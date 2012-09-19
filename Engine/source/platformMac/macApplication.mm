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

#include "macApplication.h"
#include "windowManager/mac/macWindow.h"
#include "windowManager/mac/macView.h"
#include "console/console.h"

@implementation macApplication

- (void)sendEvent:(NSEvent*)theEvent
{
   if([theEvent type] == NSKeyUp)
   {
      if([theEvent modifierFlags] & NSCommandKeyMask)
      {
         // These will normally be blocked, but we wants them!
         [[self delegate] keyUp:theEvent];
         return;
      }
   }
   
   MacWindow* window = [(GGMacView*)[self delegate] torqueWindow];
   if(window && window->isFullscreen())
   {
      switch([theEvent type])
      {
      case NSLeftMouseDown:
         [[self delegate] mouseDown:theEvent];
         return;
      case NSRightMouseDown:
         [[self delegate] rightMouseDown:theEvent];
         return;
      case NSLeftMouseUp:
         [[self delegate] mouseUp:theEvent];
         return;
      case NSRightMouseUp:
         [[self delegate] rightMouseUp:theEvent];
         return;
      case NSMouseMoved:
         [[self delegate] mouseMoved:theEvent];
         return;
      case NSLeftMouseDragged:
         [[self delegate] mouseDragged:theEvent];
         return;
      case NSRightMouseDragged:
         [[self delegate] rightMouseDragged:theEvent];
         return;
      case NSScrollWheel:
         [[self delegate] scrollWheel:theEvent];
         return;
      default:
         break;
      }
   }
   
   [super sendEvent:theEvent];
}

@end
