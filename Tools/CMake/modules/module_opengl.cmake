# -----------------------------------------------------------------------------
# Copyright (c) 2015 GarageGames, LLC
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

# OpenGL module

if(WIN32)
	option(TORQUE_OPENGL "Allow OpenGL render" OFF)
	#mark_as_advanced(TORQUE_OPENGL)
else()
	set(TORQUE_OPENGL ON) # we need OpenGL to render on Linux/Mac
endif()

if( NOT TORQUE_OPENGL )
    return()
endif()

addDef(TORQUE_OPENGL)
addInclude("${libDir}/glew/include")
addPath("${srcDir}/shaderGen/GLSL")

# lighting
if(TORQUE_ADVANCED_LIGHTING)
    addPathRec("${srcDir}/lighting/advanced/glsl")
endif()

if( NOT TORQUE_DEDICATED )
    addPath("${srcDir}/gfx/gl")
    addPath("${srcDir}/gfx/gl/tGL")        
    addPath("${srcDir}/shaderGen/GLSL")
    addPath("${srcDir}/terrain/glsl")
    addPath("${srcDir}/forest/glsl")    

    # glew
    LIST(APPEND ${PROJECT_NAME}_files "${libDir}/glew/src/glew.c")
endif()

if(WIN32)
    addDef(GLEW_STATIC)
    addLib(OpenGL32.lib)

    if(NOT TORQUE_SDL)
        addPath("${srcDir}/gfx/gl/win32")
    endif()
endif()
