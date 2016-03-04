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

#ifndef _SDL_androidwindow_h
#define _SDL_androidwindow_h

#include "../../core/android/SDL_android.h"
#include "../SDL_egl_c.h"

extern int Android_CreateWindow(_THIS, SDL_Window * window);
extern void Android_SetWindowTitle(_THIS, SDL_Window * window);
extern void Android_DestroyWindow(_THIS, SDL_Window * window);

typedef struct
{
    EGLSurface egl_surface;
    EGLContext egl_context; /* We use this to preserve the context when losing focus */
    ANativeWindow* native_window;
    
} SDL_WindowData;

#endif /* _SDL_androidwindow_h */

/* vi: set ts=4 sw=4 expandtab: */
