/*
 * Copyright © 2013 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/** @file gl.h
 *
 * Provides an implementation of a GL dispatch layer using either
 * global function pointers or a hidden vtable.
 */

#ifndef EPOXY_GL_H
#define EPOXY_GL_H

#if    defined(__glplatform_h_)  || defined(__gl_h_)  || defined(__glext_h_)  \
	|| defined(__gl2platform_h_) || defined(__gl2_h_) || defined(__gl2ext_h_) \
	|| defined(__gl3platform_h_) || defined(__gl3_h_) || defined(__gl31_h_)
#	error "epoxy/gl.h" must be included before (or in place of) the desktop OpenGL / OpenGL ES headers.
#else
#	define __glplatform_h_
#	define __gl_h_
#	define __glext_h_
#	define __gl2platform_h
#	define __gl2_h_ 1
#	define __gl2ext_h_ 1
#	define __gl3platform_h_
#	define __gl3_h_ 1
#	define __gl31_h_ 1
#endif

#include <stdbool.h>

#ifndef _WIN32
#	define APIENTRY
#	define GLAPIENTRY
#	ifndef EPOXY_IMPORTEXPORT
#		define EPOXY_IMPORTEXPORT
#	endif
#	define EPOXY_CALLSPEC
#	define GLAPI
#else
#	ifndef APIENTRY
#		define APIENTRY __stdcall
#	endif
#	ifndef GLAPIENTRY
#		define GLAPIENTRY APIENTRY
#	endif
#	ifndef EPOXY_CALLSPEC
#		define EPOXY_CALLSPEC __stdcall
#	endif
#	ifndef EPOXY_IMPORTEXPORT
#		ifdef EPOXY_DLL
#			define EPOXY_IMPORTEXPORT __declspec(dllimport)
#		else
#			define EPOXY_IMPORTEXPORT
#		endif
#	endif
#	ifndef GLAPI
#		define GLAPI extern
#	endif
#endif /* _WIN32 */

#ifndef APIENTRYP
#	define APIENTRYP APIENTRY *
#endif
#ifndef GLAPIENTRYP
#	define GLAPIENTRYP GLAPIENTRY *
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "epoxy/gl_generated.h"

EPOXY_IMPORTEXPORT bool epoxy_has_gl_extension(const char *extension);
EPOXY_IMPORTEXPORT bool epoxy_is_desktop_gl(void);
EPOXY_IMPORTEXPORT int epoxy_gl_version(void);
EPOXY_IMPORTEXPORT bool epoxy_current_context_is_egl(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* EPOXY_GL_H */
