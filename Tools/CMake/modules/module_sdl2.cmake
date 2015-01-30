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

# SDL2 module
if(WIN32)
	option(TORQUE_SDL "Use SDL for window and input" OFF)
	mark_as_advanced(TORQUE_SDL)
else()
	set(TORQUE_SDL ON) # we need sdl to work on Linux/Mac
endif()

if( NOT TORQUE_SDL )
    return()
endif()


addDef(TORQUE_SDL)
addInclude(${libDir}/sdl/include)
addLib(SDL2)
addPathRec("${srcDir}/windowManager/sdl")
addPathRec("${srcDir}/platformSDL")

if(TORQUE_OPENGL)
  addPathRec("${srcDir}/gfx/gl/sdl")
endif()

if(UNIX)
   #set(CMAKE_SIZEOF_VOID_P 4) #force 32 bit
   set(ENV{CFLAGS} "${CXX_FLAG32} -g -O3")
   if("${TORQUE_ADDITIONAL_LINKER_FLAGS}" STREQUAL "")
     set(ENV{LDFLAGS} "${CXX_FLAG32}")
   else()
     set(ENV{LDFLAGS} "${CXX_FLAG32} ${TORQUE_ADDITIONAL_LINKER_FLAGS}")
   endif()
endif()

#override and hide SDL2 cache variables
set(SDL_SHARED ON CACHE INTERNAL "" FORCE)
set(SDL_STATIC OFF CACHE INTERNAL "" FORCE)
add_subdirectory( ${libDir}/sdl ${CMAKE_CURRENT_BINARY_DIR}/sdl2)
