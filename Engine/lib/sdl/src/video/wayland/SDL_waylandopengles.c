/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2019 Sam Lantinga <slouken@libsdl.org>

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

#if SDL_VIDEO_DRIVER_WAYLAND && SDL_VIDEO_OPENGL_EGL

#include "../SDL_sysvideo.h"
#include "../../events/SDL_windowevents_c.h"
#include "SDL_waylandvideo.h"
#include "SDL_waylandopengles.h"
#include "SDL_waylandwindow.h"
#include "SDL_waylandevents_c.h"
#include "SDL_waylanddyn.h"

#include "xdg-shell-client-protocol.h"
#include "xdg-shell-unstable-v6-client-protocol.h"

/* EGL implementation of SDL OpenGL ES support */

int
Wayland_GLES_LoadLibrary(_THIS, const char *path) {
    int ret;
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;
    
    ret = SDL_EGL_LoadLibrary(_this, path, (NativeDisplayType) data->display, 0);

    Wayland_PumpEvents(_this);
    WAYLAND_wl_display_flush(data->display);
    
    return ret;
}


SDL_GLContext
Wayland_GLES_CreateContext(_THIS, SDL_Window * window)
{
    SDL_GLContext context;
    context = SDL_EGL_CreateContext(_this, ((SDL_WindowData *) window->driverdata)->egl_surface);
    WAYLAND_wl_display_flush( ((SDL_VideoData*)_this->driverdata)->display );
    
    return context;
}

int
Wayland_GLES_SwapWindow(_THIS, SDL_Window *window)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    struct wl_region *region;

    if (SDL_EGL_SwapBuffers(_this, data->egl_surface) < 0) {
        return -1;
    }

    // Wayland-EGL forbids drawing calls in-between SwapBuffers and wl_egl_window_resize
    if (data->resize.pending) {
        if (data->scale_factor != data->resize.scale_factor) {
            window->w = 0;
            window->h = 0;
        }
        SDL_SendWindowEvent(window, SDL_WINDOWEVENT_RESIZED, data->resize.width, data->resize.height);
        window->w = data->resize.width;
        window->h = data->resize.height;
        data->scale_factor = data->resize.scale_factor;
        wl_surface_set_buffer_scale(data->surface, data->scale_factor);
        WAYLAND_wl_egl_window_resize(data->egl_window, window->w * data->scale_factor, window->h * data->scale_factor, 0, 0);

        if (data->resize.configure) {
           if (data->waylandData->shell.xdg) {
              xdg_surface_ack_configure(data->shell_surface.xdg.surface, data->resize.serial);
           } else if (data->waylandData->shell.zxdg) {
              zxdg_surface_v6_ack_configure(data->shell_surface.zxdg.surface, data->resize.serial);
           }
           data->resize.configure = SDL_FALSE;
        }

        region = wl_compositor_create_region(data->waylandData->compositor);
        wl_region_add(region, 0, 0, window->w, window->h);
        wl_surface_set_opaque_region(data->surface, region);
        wl_region_destroy(region);

        data->resize.pending = SDL_FALSE;
    }

    WAYLAND_wl_display_flush( data->waylandData->display );

    return 0;
}

int
Wayland_GLES_MakeCurrent(_THIS, SDL_Window * window, SDL_GLContext context)
{
    int ret;
    
    if (window && context) {
        ret = SDL_EGL_MakeCurrent(_this, ((SDL_WindowData *) window->driverdata)->egl_surface, context);
    }
    else {
        ret = SDL_EGL_MakeCurrent(_this, NULL, NULL);
    }
    
    WAYLAND_wl_display_flush( ((SDL_VideoData*)_this->driverdata)->display );
    
    return ret;
}

void
Wayland_GLES_GetDrawableSize(_THIS, SDL_Window * window, int * w, int * h)
{
    SDL_WindowData *data;
    if (window->driverdata) {
        data = (SDL_WindowData *) window->driverdata;

        if (w) {
            *w = window->w * data->scale_factor;
        }

        if (h) {
            *h = window->h * data->scale_factor;
        }
    }
}

void 
Wayland_GLES_DeleteContext(_THIS, SDL_GLContext context)
{
    SDL_EGL_DeleteContext(_this, context);
    WAYLAND_wl_display_flush( ((SDL_VideoData*)_this->driverdata)->display );
}

#endif /* SDL_VIDEO_DRIVER_WAYLAND && SDL_VIDEO_OPENGL_EGL */

/* vi: set ts=4 sw=4 expandtab: */
