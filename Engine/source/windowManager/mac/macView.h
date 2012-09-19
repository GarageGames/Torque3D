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

#ifndef _MACVIEW_H_
#define _MACVIEW_H_

#import <Cocoa/Cocoa.h>
#include "windowManager/mac/macWindow.h"

/// GGMacView handles displaying content and responding to user input.
@interface GGMacView : NSOpenGLView
{
   MacWindow* mTorqueWindow;
   U32 mLastMods;
   bool mHandledAsCharEvent;
}
- (void)setTorqueWindow:(MacWindow*)theWindow;
- (MacWindow*)torqueWindow;

/// @name Inherited Mouse Input methods
/// @{
- (void)mouseDown:(NSEvent *)theEvent;
- (void)rightMouseDown:(NSEvent *)theEvent;
- (void)mouseDragged:(NSEvent *)theEvent;
- (void)rightMouseDragged:(NSEvent *)theEvent;
- (void)mouseUp:(NSEvent *)theEvent;
- (void)rightMouseUp:(NSEvent *)theEvent;
- (void)mouseMoved:(NSEvent *)theEvent;
- (void)scrollWheel:(NSEvent *)theEvent;
/// @}

/// @name Inherited Keyboard Input methods
/// @{
- (void)keyDown:(NSEvent *)theEvent;
- (void)keyUp:(NSEvent *)theEvent;
/// @}

/// @name Keyboard Input Common Code
/// @{
- (void)rawKeyUpDown:(NSEvent *)theEvent keyDown:(BOOL)isKeyDown;
/// @}

/// @name Mouse Input Common Code
/// @{
- (void)mouseUpDown:(NSEvent *)theEvent mouseDown:(BOOL)isMouseDown;
- (void)mouseMotion:(NSEvent *)theEvent;
/// @}

- (BOOL)acceptsFirstResponder;
- (BOOL)becomeFirstResponder;
- (BOOL)resignFirstResponder;


@end

#endif
