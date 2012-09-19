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

// Windows WGL functions
GL_GROUP_BEGIN(ARB_win32)
GL_FUNCTION(wglCopyContext, BOOL, (HGLRC, HGLRC, UINT))
GL_FUNCTION(wglCreateContext, HGLRC, (HDC))
GL_FUNCTION(wglCreateLayerContext, HGLRC, (HDC, GLint))
GL_FUNCTION(wglDeleteContext, BOOL, (HGLRC))
GL_FUNCTION(wglGetCurrentContext, HGLRC, (VOID))
GL_FUNCTION(wglGetCurrentDC, HDC, (VOID))
GL_FUNCTION(wglGetProcAddress, PROC, (LPCSTR))
GL_FUNCTION(wglMakeCurrent, BOOL, (HDC, HGLRC))
GL_FUNCTION(wglShareLists, BOOL, (HGLRC, HGLRC))
GL_FUNCTION(wglDescribeLayerPlane, BOOL, (HDC, GLint, GLint, UINT, LPLAYERPLANEDESCRIPTOR))
GL_FUNCTION(wglSetLayerPaletteEntries, GLint, (HDC, GLint, GLint, GLint, CONST COLORREF *))
GL_FUNCTION(wglGetLayerPaletteEntries, GLint, (HDC, GLint, GLint, GLint, COLORREF *))
GL_FUNCTION(wglRealizeLayerPalette, BOOL, (HDC, GLint, BOOL))
GL_FUNCTION(wglSwapLayerBuffers, BOOL, (HDC, UINT))

// Ascii and Unicode versions
GL_FUNCTION(wglUseFontBitmapsA, BOOL, (HDC, DWORD, DWORD, DWORD))
GL_FUNCTION(wglUseFontOutlinesA, BOOL, (HDC, DWORD, DWORD, DWORD, FLOAT, FLOAT, GLint, LPGLYPHMETRICSFLOAT))
GL_FUNCTION(wglUseFontBitmapsW, BOOL, (HDC, DWORD, DWORD, DWORD))
GL_FUNCTION(wglUseFontOutlinesW, BOOL, (HDC, DWORD, DWORD, DWORD, FLOAT, FLOAT, GLint, LPGLYPHMETRICSFLOAT))

GL_GROUP_END()

