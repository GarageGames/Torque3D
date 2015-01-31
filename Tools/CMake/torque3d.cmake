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

project(${TORQUE_APP_NAME})

# TODO: fmod support

###############################################################################
# modules
###############################################################################
option(TORQUE_THEORA "Theora Video Support" ON)
mark_as_advanced(TORQUE_THEORA)
option(TORQUE_ADVANCED_LIGHTING "Advanced Lighting" ON)
mark_as_advanced(TORQUE_ADVANCED_LIGHTING)
option(TORQUE_BASIC_LIGHTING "Basic Lighting" ON)
mark_as_advanced(TORQUE_BASIC_LIGHTING)
option(TORQUE_SFX_OPENAL "OpenAL Sound" ON)
mark_as_advanced(TORQUE_SFX_OPENAL)
option(TORQUE_HIFI "HIFI? support" OFF)
mark_as_advanced(TORQUE_HIFI)
option(TORQUE_EXTENDED_MOVE "Extended move support" OFF)
mark_as_advanced(TORQUE_EXTENDED_MOVE)

###############################################################################
# modular paths
###############################################################################
# lighting
if(TORQUE_ADVANCED_LIGHTING)
    addPath("${srcDir}/lighting/advanced")
    addPathRec("${srcDir}/lighting/shadowMap")
    if(WIN32)
		addPathRec("${srcDir}/lighting/advanced/hlsl")
	endif()
    addDef(TORQUE_ADVANCED_LIGHTING)
endif()
if(TORQUE_BASIC_LIGHTING)
    addPathRec("${srcDir}/lighting/basic")
    addPathRec("${srcDir}/lighting/shadowMap")
    addDef(TORQUE_BASIC_LIGHTING)
endif()

# DirectX Sound
if(TORQUE_SFX_DirectX)
    addLib(x3daudio.lib)
    addPathRec("${srcDir}/sfx/dsound")
    addPathRec("${srcDir}/sfx/xaudio")
endif()

# OpenAL
if(TORQUE_SFX_OPENAL AND NOT TORQUE_DEDICATED)
    addPath("${srcDir}/sfx/openal")
    #addPath("${srcDir}/sfx/openal/mac")
    if(WIN32)
		addPath("${srcDir}/sfx/openal/win32")
		addInclude("${libDir}/openal/win32")
    endif()
	if(UNIX)
		addPath("${srcDir}/sfx/openal/linux")
	endif()
    
endif()

# Theora
if(TORQUE_THEORA)
    addPath("${srcDir}/core/ogg")
    addPath("${srcDir}/gfx/video")
    addPath("${srcDir}/gui/theora")
    
    addDef(TORQUE_OGGTHEORA)
    addDef(TORQUE_OGGVORIBS)
    addInclude(${libDir}/libtheora/include)
    addLib(libtheora)
endif()

# Include tools for non-tool builds (or define player if a tool build)
if(TORQUE_TOOLS)
    addPath("${srcDir}/gui/worldEditor")
    addPath("${srcDir}/environment/editors")
    addPath("${srcDir}/forest/editor")
    addPath("${srcDir}/gui/editor")
    addPath("${srcDir}/gui/editor/inspector")
endif()

if(TORQUE_HIFI)
    addPath("${srcDir}/T3D/gameBase/hifi")
    addDef(TORQUE_HIFI_NET)
endif()
    
if(TORQUE_EXTENDED_MOVE)
    addPath("${srcDir}/T3D/gameBase/extended")
    addDef(TORQUE_EXTENDED_MOVE)
else()
    addPath("${srcDir}/T3D/gameBase/std")
endif()

if(TORQUE_DEDICATED)
    addDef(TORQUE_DEDICATED)
endif()

#modules dir
file(GLOB modules "modules/*.cmake")
foreach(module ${modules})
    string(FIND ${module} ".target.cmake" FIND_RESULT)
    if( ${FIND_RESULT} STREQUAL "-1")
        include(${module})
    endif()
endforeach()

###############################################################################
###############################################################################
finishExecutable()
###############################################################################
###############################################################################

#modules dir
file(GLOB modules "modules/*.target.cmake")
foreach(module ${modules})
    include(${module})
endforeach()

message(STATUS "writing ${projectSrcDir}/torqueConfig.h")
CONFIGURE_FILE("${cmakeDir}/torqueConfig.h.in" "${projectSrcDir}/torqueConfig.h")

# configure the relevant files only once
if(NOT EXISTS "${projectSrcDir}/torque.ico")
    CONFIGURE_FILE("${cmakeDir}/torque.ico" "${projectSrcDir}/torque.ico" COPYONLY)
endif()
if(NOT EXISTS "${projectOutDir}/${PROJECT_NAME}.torsion")
    CONFIGURE_FILE("${cmakeDir}/template.torsion.in" "${projectOutDir}/${PROJECT_NAME}.torsion")
endif()
if(EXISTS "${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/game/main.cs.in" AND NOT EXISTS "${projectOutDir}/main.cs")
    CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/game/main.cs.in" "${projectOutDir}/main.cs")
endif()


###############################################################################
# Installation
###############################################################################

if(TORQUE_TEMPLATE)
    message("Prepare Template(${TORQUE_TEMPLATE}) install...")
    INSTALL(DIRECTORY "${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/game"                 DESTINATION "${TORQUE_APP_DIR}")
endif()
