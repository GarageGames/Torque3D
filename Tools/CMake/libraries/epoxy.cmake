# -----------------------------------------------------------------------------
# Copyright (c) 2016 GarageGames, LLC
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
# -----------------------------------------------------------------------------

project(epoxy)

addPath("${libDir}/epoxy/src")

# TODO EGL support if we ever use EGL instead of GLX
if (WIN32)
   addPath("${libDir}/epoxy/src/wgl")
   addDef(BUILD_WGL)
else()
   addPath("${libDir}/epoxy/src/glx")
   addDef(BUILD_GLX)   
endif()

addInclude("${libDir}/epoxy/include")
addInclude("${libDir}/epoxy/src")

finishLibrary()
# VS 2015 has a problem with sdl and epoxy together and requires optimizations to be disabled
if (MSVC14)
	target_compile_options(epoxy PRIVATE "/Od")
endif()
