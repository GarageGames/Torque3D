/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_COCOA

#include "SDL_assert.h"
#include "SDL_events.h"
#include "SDL_cocoamouse.h"
#include "SDL_cocoamousetap.h"

#include "../../events/SDL_mouse_c.h"

/* #define DEBUG_COCOAMOUSE */

#ifdef DEBUG_COCOAMOUSE
#define DLog(fmt, ...) printf("%s: " fmt "\n", __func__, ##__VA_ARGS__)
#else
#define DLog(...) do { } while (0)
#endif

@implementation NSCursor (InvisibleCursor)
+ (NSCursor *)invisibleCursor
{
    static NSCursor *invisibleCursor = NULL;
    if (!invisibleCursor) {
        /* RAW 16x16 transparent GIF */
        static unsigned char cursorBytes[] = {
            0x47, 0x49, 0x46, 0x38, 0x37, 0x61, 0x10, 0x00, 0x10, 0x00, 0x80,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0xF9, 0x04,
            0x01, 0x00, 0x00, 0x01, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x10,
            0x00, 0x10, 0x00, 0x00, 0x02, 0x0E, 0x8C, 0x8F, 0xA9, 0xCB, 0xED,
            0x0F, 0xA3, 0x9C, 0xB4, 0xDA, 0x8B, 0xB3, 0x3E, 0x05, 0x00, 0x3B
        };

        NSData *cursorData = [NSData dataWithBytesNoCopy:&cursorBytes[0]
                                                  length:sizeof(cursorBytes)
                                            freeWhenDone:NO];
        NSImage *cursorImage = [[[NSImage alloc] initWithData:cursorData] autorelease];
        invisibleCursor = [[NSCursor alloc] initWithImage:cursorImage
                                                  hotSpot:NSZeroPoint];
    }

    return invisibleCursor;
}
@end


static SDL_Cursor *
Cocoa_CreateDefaultCursor()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSCursor *nscursor;
    SDL_Cursor *cursor = NULL;

    nscursor = [NSCursor arrowCursor];

    if (nscursor) {
        cursor = SDL_calloc(1, sizeof(*cursor));
        if (cursor) {
            cursor->driverdata = nscursor;
            [nscursor retain];
        }
    }

    [pool release];

    return cursor;
}

static SDL_Cursor *
Cocoa_CreateCursor(SDL_Surface * surface, int hot_x, int hot_y)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSImage *nsimage;
    NSCursor *nscursor = NULL;
    SDL_Cursor *cursor = NULL;

    nsimage = Cocoa_CreateImage(surface);
    if (nsimage) {
        nscursor = [[NSCursor alloc] initWithImage: nsimage hotSpot: NSMakePoint(hot_x, hot_y)];
    }

    if (nscursor) {
        cursor = SDL_calloc(1, sizeof(*cursor));
        if (cursor) {
            cursor->driverdata = nscursor;
        } else {
            [nscursor release];
        }
    }

    [pool release];

    return cursor;
}

static SDL_Cursor *
Cocoa_CreateSystemCursor(SDL_SystemCursor id)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSCursor *nscursor = NULL;
    SDL_Cursor *cursor = NULL;

    switch(id)
    {
    case SDL_SYSTEM_CURSOR_ARROW:
        nscursor = [NSCursor arrowCursor];
        break;
    case SDL_SYSTEM_CURSOR_IBEAM:
        nscursor = [NSCursor IBeamCursor];
        break;
    case SDL_SYSTEM_CURSOR_WAIT:
        nscursor = [NSCursor arrowCursor];
        break;
    case SDL_SYSTEM_CURSOR_CROSSHAIR:
        nscursor = [NSCursor crosshairCursor];
        break;
    case SDL_SYSTEM_CURSOR_WAITARROW:
        nscursor = [NSCursor arrowCursor];
        break;
    case SDL_SYSTEM_CURSOR_SIZENWSE:
    case SDL_SYSTEM_CURSOR_SIZENESW:
        nscursor = [NSCursor closedHandCursor];
        break;
    case SDL_SYSTEM_CURSOR_SIZEWE:
        nscursor = [NSCursor resizeLeftRightCursor];
        break;
    case SDL_SYSTEM_CURSOR_SIZENS:
        nscursor = [NSCursor resizeUpDownCursor];
        break;
    case SDL_SYSTEM_CURSOR_SIZEALL:
        nscursor = [NSCursor closedHandCursor];
        break;
    case SDL_SYSTEM_CURSOR_NO:
        nscursor = [NSCursor operationNotAllowedCursor];
        break;
    case SDL_SYSTEM_CURSOR_HAND:
        nscursor = [NSCursor pointingHandCursor];
        break;
    default:
        SDL_assert(!"Unknown system cursor");
        return NULL;
    }

    if (nscursor) {
        cursor = SDL_calloc(1, sizeof(*cursor));
        if (cursor) {
            /* We'll free it later, so retain it here */
            [nscursor retain];
            cursor->driverdata = nscursor;
        }
    }

    [pool release];

    return cursor;
}

static void
Cocoa_FreeCursor(SDL_Cursor * cursor)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSCursor *nscursor = (NSCursor *)cursor->driverdata;

    [nscursor release];
    SDL_free(cursor);

    [pool release];
}

static int
Cocoa_ShowCursor(SDL_Cursor * cursor)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    SDL_VideoDevice *device = SDL_GetVideoDevice();
    SDL_Window *window = (device ? device->windows : NULL);
    for (; window != NULL; window = window->next) {
        SDL_WindowData *driverdata = (SDL_WindowData *)window->driverdata;
        if (driverdata) {
            [driverdata->nswindow performSelectorOnMainThread:@selector(invalidateCursorRectsForView:)
                                                   withObject:[driverdata->nswindow contentView]
                                                waitUntilDone:NO];
        }
    }

    [pool release];

    return 0;
}

static void
Cocoa_WarpMouse(SDL_Window * window, int x, int y)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    if ([data->listener isMoving])
    {
        DLog("Postponing warp, window being moved.");
        [data->listener setPendingMoveX:x
                                      Y:y];
        return;
    }

    SDL_Mouse *mouse = SDL_GetMouse();
    CGPoint point = CGPointMake(x + (float)window->x, y + (float)window->y);

    Cocoa_HandleMouseWarp(point.x, point.y);

    /* According to the docs, this was deprecated in 10.6, but it's still
     * around. The substitute requires a CGEventSource, but I'm not entirely
     * sure how we'd procure the right one for this event.
     */
    CGSetLocalEventsSuppressionInterval(0.0);
    CGWarpMouseCursorPosition(point);
    CGSetLocalEventsSuppressionInterval(0.25);

    if (!mouse->relative_mode) {
        /* CGWarpMouseCursorPosition doesn't generate a window event, unlike our
         * other implementations' APIs.
         */
        SDL_SendMouseMotion(mouse->focus, mouse->mouseID, 0, x, y);
    }
}

static int
Cocoa_SetRelativeMouseMode(SDL_bool enabled)
{
    /* We will re-apply the relative mode when the window gets focus, if it
     * doesn't have focus right now.
     */
    SDL_Window *window = SDL_GetMouseFocus();
    if (!window) {
      return 0;
    }

    /* We will re-apply the relative mode when the window finishes being moved,
     * if it is being moved right now.
     */
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    if ([data->listener isMoving]) {
        return 0;
    }

    CGError result;
    if (enabled) {
        DLog("Turning on.");
        result = CGAssociateMouseAndMouseCursorPosition(NO);
    } else {
        DLog("Turning off.");
        result = CGAssociateMouseAndMouseCursorPosition(YES);
    }
    if (result != kCGErrorSuccess) {
        return SDL_SetError("CGAssociateMouseAndMouseCursorPosition() failed");
    }
    return 0;
}

void
Cocoa_InitMouse(_THIS)
{
    SDL_Mouse *mouse = SDL_GetMouse();

    mouse->driverdata = SDL_calloc(1, sizeof(SDL_MouseData));

    mouse->CreateCursor = Cocoa_CreateCursor;
    mouse->CreateSystemCursor = Cocoa_CreateSystemCursor;
    mouse->ShowCursor = Cocoa_ShowCursor;
    mouse->FreeCursor = Cocoa_FreeCursor;
    mouse->WarpMouse = Cocoa_WarpMouse;
    mouse->SetRelativeMouseMode = Cocoa_SetRelativeMouseMode;

    SDL_SetDefaultCursor(Cocoa_CreateDefaultCursor());

    Cocoa_InitMouseEventTap(mouse->driverdata);

    SDL_MouseData *driverdata = (SDL_MouseData*)mouse->driverdata;
    const NSPoint location =  [NSEvent mouseLocation];
    driverdata->lastMoveX = location.x;
    driverdata->lastMoveY = location.y;
}

void
Cocoa_HandleMouseEvent(_THIS, NSEvent *event)
{
    switch ([event type])
    {
        case NSMouseMoved:
        case NSLeftMouseDragged:
        case NSRightMouseDragged:
        case NSOtherMouseDragged:
            break;

        default:
            /* Ignore any other events. */
            return;
    }

    SDL_Mouse *mouse = SDL_GetMouse();

    SDL_MouseData *driverdata = (SDL_MouseData*)mouse->driverdata;
    const SDL_bool seenWarp = driverdata->seenWarp;
    driverdata->seenWarp = NO;

    const NSPoint location =  [NSEvent mouseLocation];
    const CGFloat lastMoveX = driverdata->lastMoveX;
    const CGFloat lastMoveY = driverdata->lastMoveY;
    driverdata->lastMoveX = location.x;
    driverdata->lastMoveY = location.y;
    DLog("Last seen mouse: (%g, %g)", location.x, location.y);

    /* Non-relative movement is handled in -[Cocoa_WindowListener mouseMoved:] */
    if (!mouse->relative_mode) {
        return;
    }

    /* Ignore events that aren't inside the client area (i.e. title bar.) */
    if ([event window]) {
        NSRect windowRect = [[[event window] contentView] frame];
        if (!NSPointInRect([event locationInWindow], windowRect)) {
            return;
        }
    }

    float deltaX = [event deltaX];
    float deltaY = [event deltaY];

    if (seenWarp)
    {
        deltaX += (lastMoveX - driverdata->lastWarpX);
        deltaY += ((CGDisplayPixelsHigh(kCGDirectMainDisplay) - lastMoveY) - driverdata->lastWarpY);

        DLog("Motion was (%g, %g), offset to (%g, %g)", [event deltaX], [event deltaY], deltaX, deltaY);
    }

    SDL_SendMouseMotion(mouse->focus, mouse->mouseID, 1, (int)deltaX, (int)deltaY);
}

void
Cocoa_HandleMouseWheel(SDL_Window *window, NSEvent *event)
{
    SDL_Mouse *mouse = SDL_GetMouse();

    float x = -[event deltaX];
    float y = [event deltaY];

    if (x > 0) {
        x += 0.9f;
    } else if (x < 0) {
        x -= 0.9f;
    }
    if (y > 0) {
        y += 0.9f;
    } else if (y < 0) {
        y -= 0.9f;
    }
    SDL_SendMouseWheel(window, mouse->mouseID, (int)x, (int)y);
}

void
Cocoa_HandleMouseWarp(CGFloat x, CGFloat y)
{
    /* This makes Cocoa_HandleMouseEvent ignore the delta caused by the warp,
     * since it gets included in the next movement event.
     */
    SDL_MouseData *driverdata = (SDL_MouseData*)SDL_GetMouse()->driverdata;
    driverdata->lastWarpX = x;
    driverdata->lastWarpY = y;
    driverdata->seenWarp = SDL_TRUE;

    DLog("(%g, %g)", x, y);
}

void
Cocoa_QuitMouse(_THIS)
{
    SDL_Mouse *mouse = SDL_GetMouse();
    if (mouse) {
        if (mouse->driverdata) {
            Cocoa_QuitMouseEventTap(((SDL_MouseData*)mouse->driverdata));
        }

        SDL_free(mouse->driverdata);
    }
}

#endif /* SDL_VIDEO_DRIVER_COCOA */

/* vi: set ts=4 sw=4 expandtab: */
