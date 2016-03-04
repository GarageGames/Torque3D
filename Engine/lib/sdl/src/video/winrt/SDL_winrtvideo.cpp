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

#if SDL_VIDEO_DRIVER_WINRT

/* WinRT SDL video driver implementation

   Initial work on this was done by David Ludwig (dludwig@pobox.com), and
   was based off of SDL's "dummy" video driver.
 */

/* Windows includes */
#include <agile.h>
using namespace Windows::UI::Core;


/* SDL includes */
extern "C" {
#include "SDL_video.h"
#include "SDL_mouse.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"
#include "../../render/SDL_sysrender.h"
#include "SDL_syswm.h"
#include "SDL_winrtopengles.h"
}

#include "../../core/winrt/SDL_winrtapp_direct3d.h"
#include "../../core/winrt/SDL_winrtapp_xaml.h"
#include "SDL_winrtvideo_cpp.h"
#include "SDL_winrtevents_c.h"
#include "SDL_winrtmouse_c.h"
#include "SDL_main.h"
#include "SDL_system.h"


/* Initialization/Query functions */
static int WINRT_VideoInit(_THIS);
static int WINRT_InitModes(_THIS);
static int WINRT_SetDisplayMode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode);
static void WINRT_VideoQuit(_THIS);


/* Window functions */
static int WINRT_CreateWindow(_THIS, SDL_Window * window);
static void WINRT_DestroyWindow(_THIS, SDL_Window * window);
static SDL_bool WINRT_GetWindowWMInfo(_THIS, SDL_Window * window, SDL_SysWMinfo * info);


/* SDL-internal globals: */
SDL_Window * WINRT_GlobalSDLWindow = NULL;
SDL_VideoDevice * WINRT_GlobalSDLVideoDevice = NULL;


/* WinRT driver bootstrap functions */

static int
WINRT_Available(void)
{
    return (1);
}

static void
WINRT_DeleteDevice(SDL_VideoDevice * device)
{
    if (device == WINRT_GlobalSDLVideoDevice) {
        WINRT_GlobalSDLVideoDevice = NULL;
    }
    SDL_free(device);
}

static SDL_VideoDevice *
WINRT_CreateDevice(int devindex)
{
    SDL_VideoDevice *device;

    /* Initialize all variables that we clean on shutdown */
    device = (SDL_VideoDevice *) SDL_calloc(1, sizeof(SDL_VideoDevice));
    if (!device) {
        SDL_OutOfMemory();
        if (device) {
            SDL_free(device);
        }
        return (0);
    }

    /* Set the function pointers */
    device->VideoInit = WINRT_VideoInit;
    device->VideoQuit = WINRT_VideoQuit;
    device->CreateWindow = WINRT_CreateWindow;
    device->DestroyWindow = WINRT_DestroyWindow;
    device->SetDisplayMode = WINRT_SetDisplayMode;
    device->PumpEvents = WINRT_PumpEvents;
    device->GetWindowWMInfo = WINRT_GetWindowWMInfo;
#ifdef SDL_VIDEO_OPENGL_EGL
    device->GL_LoadLibrary = WINRT_GLES_LoadLibrary;
    device->GL_GetProcAddress = WINRT_GLES_GetProcAddress;
    device->GL_UnloadLibrary = WINRT_GLES_UnloadLibrary;
    device->GL_CreateContext = WINRT_GLES_CreateContext;
    device->GL_MakeCurrent = WINRT_GLES_MakeCurrent;
    device->GL_SetSwapInterval = WINRT_GLES_SetSwapInterval;
    device->GL_GetSwapInterval = WINRT_GLES_GetSwapInterval;
    device->GL_SwapWindow = WINRT_GLES_SwapWindow;
    device->GL_DeleteContext = WINRT_GLES_DeleteContext;
#endif
    device->free = WINRT_DeleteDevice;
    WINRT_GlobalSDLVideoDevice = device;

    return device;
}

#define WINRTVID_DRIVER_NAME "winrt"
VideoBootStrap WINRT_bootstrap = {
    WINRTVID_DRIVER_NAME, "SDL WinRT video driver",
    WINRT_Available, WINRT_CreateDevice
};

int
WINRT_VideoInit(_THIS)
{
    if (WINRT_InitModes(_this) < 0) {
        return -1;
    }
    WINRT_InitMouse(_this);
    WINRT_InitTouch(_this);

    return 0;
}

int
WINRT_CalcDisplayModeUsingNativeWindow(SDL_DisplayMode * mode)
{
    SDL_DisplayModeData * driverdata;

    using namespace Windows::Graphics::Display;

    // Go no further if a native window cannot be accessed.  This can happen,
    // for example, if this function is called from certain threads, such as
    // the SDL/XAML thread.
    if (!CoreWindow::GetForCurrentThread()) {
        return SDL_SetError("SDL/WinRT display modes cannot be calculated outside of the main thread, such as in SDL's XAML thread");
    }

    // Calculate the display size given the window size, taking into account
    // the current display's DPI:
#if NTDDI_VERSION > NTDDI_WIN8
    const float currentDPI = DisplayInformation::GetForCurrentView()->LogicalDpi;
#else
    const float currentDPI = Windows::Graphics::Display::DisplayProperties::LogicalDpi;
#endif
    const float dipsPerInch = 96.0f;
    const int w = (int) ((CoreWindow::GetForCurrentThread()->Bounds.Width * currentDPI) / dipsPerInch);
    const int h = (int) ((CoreWindow::GetForCurrentThread()->Bounds.Height * currentDPI) / dipsPerInch);
    if (w == 0 || w == h) {
        return SDL_SetError("Unable to calculate the WinRT window/display's size");
    }

    // Create a driverdata field:
    driverdata = (SDL_DisplayModeData *) SDL_malloc(sizeof(*driverdata));
    if (!driverdata) {
        return SDL_OutOfMemory();
    }
    SDL_zerop(driverdata);

    // Fill in most fields:
    SDL_zerop(mode);
    mode->format = SDL_PIXELFORMAT_RGB888;
    mode->refresh_rate = 0;  // TODO, WinRT: see if refresh rate data is available, or relevant (for WinRT apps)
    mode->w = w;
    mode->h = h;
    mode->driverdata = driverdata;
#if NTDDI_VERSION > NTDDI_WIN8
    driverdata->currentOrientation = DisplayInformation::GetForCurrentView()->CurrentOrientation;
#else
    driverdata->currentOrientation = DisplayProperties::CurrentOrientation;
#endif

#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
    // On Windows Phone, the native window's size is always in portrait,
    // regardless of the device's orientation.  This is in contrast to
    // Windows 8/RT, which will resize the native window as the device's
    // orientation changes.  In order to compensate for this behavior,
    // on Windows Phone, the mode's width and height will be swapped when
    // the device is in a landscape (non-portrait) mode.
    switch (DisplayProperties::CurrentOrientation) {
        case DisplayOrientations::Landscape:
        case DisplayOrientations::LandscapeFlipped:
        {
            const int tmp = mode->h;
            mode->h = mode->w;
            mode->w = tmp;
            break;
        }

        default:
            break;
    }
#endif

    return 0;
}

int
WINRT_DuplicateDisplayMode(SDL_DisplayMode * dest, const SDL_DisplayMode * src)
{
    SDL_DisplayModeData * driverdata;
    driverdata = (SDL_DisplayModeData *) SDL_malloc(sizeof(*driverdata));
    if (!driverdata) {
        return SDL_OutOfMemory();
    }
    SDL_memcpy(driverdata, src->driverdata, sizeof(SDL_DisplayModeData));
    SDL_memcpy(dest, src, sizeof(SDL_DisplayMode));
    dest->driverdata = driverdata;
    return 0;
}

int
WINRT_InitModes(_THIS)
{
    // Retrieve the display mode:
    SDL_DisplayMode mode, desktop_mode;
    if (WINRT_CalcDisplayModeUsingNativeWindow(&mode) != 0) {
        return -1;	// If WINRT_CalcDisplayModeUsingNativeWindow fails, it'll already have set the SDL error
    }

    if (WINRT_DuplicateDisplayMode(&desktop_mode, &mode) != 0) {
        return -1;
    }
    if (SDL_AddBasicVideoDisplay(&desktop_mode) < 0) {
        return -1;
    }

    SDL_AddDisplayMode(&_this->displays[0], &mode);
    return 0;
}

static int
WINRT_SetDisplayMode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode)
{
    return 0;
}

void
WINRT_VideoQuit(_THIS)
{
    WINRT_QuitMouse(_this);
}

int
WINRT_CreateWindow(_THIS, SDL_Window * window)
{
    // Make sure that only one window gets created, at least until multimonitor
    // support is added.
    if (WINRT_GlobalSDLWindow != NULL) {
        SDL_SetError("WinRT only supports one window");
        return -1;
    }

    SDL_WindowData *data = new SDL_WindowData;
    if (!data) {
        SDL_OutOfMemory();
        return -1;
    }
    window->driverdata = data;
    data->sdlWindow = window;

    /* To note, when XAML support is enabled, access to the CoreWindow will not
       be possible, at least not via the SDL/XAML thread.  Attempts to access it
       from there will throw exceptions.  As such, the SDL_WindowData's
       'coreWindow' field will only be set (to a non-null value) if XAML isn't
       enabled.
    */
    if (!WINRT_XAMLWasEnabled) {
        data->coreWindow = CoreWindow::GetForCurrentThread();
    }

#if SDL_VIDEO_OPENGL_EGL
    /* Setup the EGL surface, but only if OpenGL ES 2 was requested. */
    if (!(window->flags & SDL_WINDOW_OPENGL)) {
        /* OpenGL ES 2 wasn't requested.  Don't set up an EGL surface. */
        data->egl_surface = EGL_NO_SURFACE;
    } else {
        /* OpenGL ES 2 was reuqested.  Set up an EGL surface. */

        /* HACK: ANGLE/WinRT currently uses non-pointer, C++ objects to represent
           native windows.  The object only contains a single pointer to a COM
           interface pointer, which on x86 appears to be castable to the object
           without apparant problems.  On other platforms, notable ARM and x64,
           doing so will cause a crash.  To avoid this crash, we'll bypass
           SDL's normal call to eglCreateWindowSurface, which is invoked from C
           code, and call it here, where an appropriate C++ object may be
           passed in.
         */
        typedef EGLSurface (*eglCreateWindowSurfaceFunction)(EGLDisplay dpy, EGLConfig config,
            Microsoft::WRL::ComPtr<IUnknown> win,
            const EGLint *attrib_list);
        eglCreateWindowSurfaceFunction WINRT_eglCreateWindowSurface =
            (eglCreateWindowSurfaceFunction) _this->egl_data->eglCreateWindowSurface;

        Microsoft::WRL::ComPtr<IUnknown> nativeWindow = reinterpret_cast<IUnknown *>(data->coreWindow.Get());
        data->egl_surface = WINRT_eglCreateWindowSurface(
            _this->egl_data->egl_display,
            _this->egl_data->egl_config,
            nativeWindow, NULL);
        if (data->egl_surface == NULL) {
            // TODO, WinRT: see if eglCreateWindowSurface, or its callee(s), sets an error message.  If so, attach it to the SDL error.
            return SDL_SetError("eglCreateWindowSurface failed");
        }
    }
#endif

    /* Make sure the window is considered to be positioned at {0,0},
       and is considered fullscreen, shown, and the like.
    */
    window->x = 0;
    window->y = 0;
    window->flags =
        SDL_WINDOW_FULLSCREEN |
        SDL_WINDOW_SHOWN |
        SDL_WINDOW_BORDERLESS |
        SDL_WINDOW_MAXIMIZED |
        SDL_WINDOW_INPUT_GRABBED;

#if SDL_VIDEO_OPENGL_EGL
    if (data->egl_surface) {
        window->flags |= SDL_WINDOW_OPENGL;
    }
#endif

    /* WinRT does not, as of this writing, appear to support app-adjustable
       window sizes.  Set the window size to whatever the native WinRT
       CoreWindow is set at.

       TODO, WinRT: if and when non-fullscreen XAML control support is added to SDL, consider making those resizable via SDL_Window's interfaces.
    */
    window->w = _this->displays[0].current_mode.w;
    window->h = _this->displays[0].current_mode.h;

    /* For now, treat WinRT apps as if they always have focus.
       TODO, WinRT: try tracking keyboard and mouse focus state with respect to snapped apps
     */
    SDL_SetMouseFocus(window);
    SDL_SetKeyboardFocus(window);
 
    /* Make sure the WinRT app's IFramworkView can post events on
       behalf of SDL:
    */
    WINRT_GlobalSDLWindow = window;

    /* All done! */
    return 0;
}

void
WINRT_DestroyWindow(_THIS, SDL_Window * window)
{
    SDL_WindowData * data = (SDL_WindowData *) window->driverdata;

    if (WINRT_GlobalSDLWindow == window) {
        WINRT_GlobalSDLWindow = NULL;
    }

    if (data) {
        // Delete the internal window data:
        delete data;
        data = NULL;
    }
}

SDL_bool
WINRT_GetWindowWMInfo(_THIS, SDL_Window * window, SDL_SysWMinfo * info)
{
    SDL_WindowData * data = (SDL_WindowData *) window->driverdata;

    if (info->version.major <= SDL_MAJOR_VERSION) {
        info->subsystem = SDL_SYSWM_WINRT;
        info->info.winrt.window = reinterpret_cast<IInspectable *>(data->coreWindow.Get());
        return SDL_TRUE;
    } else {
        SDL_SetError("Application not compiled with SDL %d.%d\n",
                     SDL_MAJOR_VERSION, SDL_MINOR_VERSION);
        return SDL_FALSE;
    }
    return SDL_FALSE;
}

#endif /* SDL_VIDEO_DRIVER_WINRT */

/* vi: set ts=4 sw=4 expandtab: */
