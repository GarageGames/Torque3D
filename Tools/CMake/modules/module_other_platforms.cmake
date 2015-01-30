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

# Other platforms module

###############################################################################
# platform specific things
###############################################################################
if(APPLE)
    addPath("${srcDir}/platformMac")
    addPath("${srcDir}/platformMac/menus")
    addPath("${srcDir}/platformPOSIX")
    addPath("${srcDir}/windowManager/mac")
    addPath("${srcDir}/gfx/gl")
    addPath("${srcDir}/gfx/gl/ggl")
    addPath("${srcDir}/gfx/gl/ggl/mac")
    addPath("${srcDir}/gfx/gl/ggl/generated")
    addPath("${srcDir}/shaderGen/GLSL")
    addPath("${srcDir}/terrain/glsl")
    addPath("${srcDir}/forest/glsl")    
endif()

if(XBOX360)
    addPath("${srcDir}/platformXbox")
    addPath("${srcDir}/platformXbox/threads")
    addPath("${srcDir}/windowManager/360")
    addPath("${srcDir}/gfx/D3D9")
    addPath("${srcDir}/gfx/D3D9/360")
    addPath("${srcDir}/shaderGen/HLSL")
    addPath("${srcDir}/shaderGen/360")
    addPath("${srcDir}/ts/arch/360")
    addPath("${srcDir}/terrain/hlsl")
    addPath("${srcDir}/forest/hlsl")
endif()

if(PS3)
    addPath("${srcDir}/platformPS3")
    addPath("${srcDir}/platformPS3/threads")
    addPath("${srcDir}/windowManager/ps3")
    addPath("${srcDir}/gfx/gl")
    addPath("${srcDir}/gfx/gl/ggl")
    addPath("${srcDir}/gfx/gl/ggl/ps3")
    addPath("${srcDir}/gfx/gl/ggl/generated")
    addPath("${srcDir}/shaderGen/GLSL")
    addPath("${srcDir}/ts/arch/ps3")
    addPath("${srcDir}/terrain/glsl")
    addPath("${srcDir}/forest/glsl")    
endif()