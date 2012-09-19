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

// X11 gl functions
GL_GROUP_BEGIN(ARB_glx)
GL_FUNCTION(glXQueryExtension, Bool, (Display *dpy, int *errorBase, int *eventBase))
GL_FUNCTION(glXQueryVersion, Bool, (Display *dpy, int *major, int *minor))
GL_FUNCTION(glXGetConfig, int, (Display *dpy, XVisualInfo *vis, int attrib, int *value))
GL_FUNCTION(glXChooseVisual, XVisualInfo*, (Display *dpy, int screen, int *attribList))
GL_FUNCTION(glXCreateGLXPixmap, GLXPixmap, (Display *dpy, XVisualInfo *vis, Pixmap pixmap))
GL_FUNCTION(glXDestroyGLXPixmap, void, (Display *dpy, GLXPixmap pix))
GL_FUNCTION(glXCreateContext, GLXContext, (Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct))
GL_FUNCTION(glXDestroyContext, void, (Display *dpy, GLXContext ctx))
GL_FUNCTION(glXIsDirect, Bool, (Display *dpy, GLXContext ctx))
GL_FUNCTION(glXCopyContext, void, (Display *dpy, GLXContext src, GLXContext dst, GLuint mask))
GL_FUNCTION(glXMakeCurrent, Bool, (Display *dpy, GLXDrawable drawable, GLXContext ctx))
GL_FUNCTION(glXGetCurrentContext, GLXContext, (void))
GL_FUNCTION(glXGetCurrentDrawable, GLXDrawable, (void))
GL_FUNCTION(glXWaitGL, void, (void))
GL_FUNCTION(glXWaitX, void, (void))
GL_FUNCTION(glXSwapBuffers, void, (Display *dpy, GLXDrawable drawable))
GL_FUNCTION(glXUseXFont, void, (Font font, int first, int count, int listBase))
GL_GROUP_END()

