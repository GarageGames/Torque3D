# -----------------------------------------------------------------------------
# Copyright (c) 2014 GarageGames, LLC
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

project(libtheora)

addPath( "${libDir}/libtheora" )
addPathRec( "${libDir}/libtheora/include" )
addPath( "${libDir}/libtheora/lib" )
addPath( "${libDir}/libtheora/lib/dec" )
addPath( "${libDir}/libtheora/lib/enc" )

if(WIN32)
	addPath( "${libDir}/libtheora/lib/dec/x86_vc" )
	addPath( "${libDir}/libtheora/lib/enc/x86_32_vs" )
else()
	addPath( "${libDir}/libtheora/lib/dec/x86" )
	addPath( "${libDir}/libtheora/lib/enc/x86_32" )
endif()

addDef(TORQUE_OGGTHEORA)
addDef(TORQUE_OGGVORIBS)
addInclude(${libDir}/libogg/include)
addInclude(${libDir}/libtheora/include)

finishLibrary()
