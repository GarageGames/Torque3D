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

# Unix module

if( NOT UNIX )
    return()
endif()

addDef(LINUX)

if(NOT CXX_FLAG32)
    set(CXX_FLAG32 "")
endif()
#set(CXX_FLAG32 "-m32") #uncomment for build x32 on OSx64

# default compiler flags
# force compile 32 bit
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_FLAG32} -Wall -Wundef -msse -pipe -Wfatal-errors ${TORQUE_ADDITIONAL_LINKER_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CXX_FLAG32} -Wall -Wundef -msse -pipe -Wfatal-errors ${TORQUE_ADDITIONAL_LINKER_FLAGS}")

# for asm files
SET (CMAKE_ASM_NASM_OBJECT_FORMAT "elf")
ENABLE_LANGUAGE (ASM_NASM)

# linux_dedicated
if(TORQUE_DEDICATED)
    addPath("${srcDir}/windowManager/dedicated")
    # ${srcDir}/platformX86UNIX/*.client.* files are not needed	
    # @todo: move to separate file
    file( GLOB tmp_files
         ${srcDir}/platformX86UNIX/*.cpp
         ${srcDir}/platformX86UNIX/*.c
         ${srcDir}/platformX86UNIX/*.cc
         ${srcDir}/platformX86UNIX/*.h )
    file( GLOB tmp_remove_files ${srcDir}/platformX86UNIX/*client.* )
    LIST( REMOVE_ITEM tmp_files ${tmp_remove_files} )
    foreach( f ${tmp_files} )
        addFile( ${f} )
    endforeach()
else()
    addPath("${srcDir}/platformX86UNIX")
    addPath("${srcDir}/platformX86UNIX/nativeDialogs")
endif()

addPath("${srcDir}/platformX86UNIX/threads")
addPath("${srcDir}/platformPOSIX")

addInclude("/usr/include/freetype2/freetype")
addInclude("/usr/include/freetype2")

# copy pasted from T3D build system, some might not be needed
set(TORQUE_EXTERNAL_LIBS "rt dl Xxf86vm Xext X11 Xft stdc++ pthread GL" CACHE STRING "external libs to link against")
mark_as_advanced(TORQUE_EXTERNAL_LIBS)

string(REPLACE " " ";" TORQUE_EXTERNAL_LIBS_LIST ${TORQUE_EXTERNAL_LIBS})
addLib( "${TORQUE_EXTERNAL_LIBS_LIST}" )
