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

if(UNIX)
    if(NOT CXX_FLAG32)
        set(CXX_FLAG32 "")
    endif()
    #set(CXX_FLAG32 "-m32") #uncomment for build x32 on OSx64
    
    # default compiler flags
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_FLAG32} -Wundef -msse -pipe -Wfatal-errors ${TORQUE_ADDITIONAL_LINKER_FLAGS} -Wl,-rpath,'$$ORIGIN'")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CXX_FLAG32} -Wundef -msse -pipe -Wfatal-errors ${TORQUE_ADDITIONAL_LINKER_FLAGS} -Wl,-rpath,'$$ORIGIN'")

	# for asm files
	SET (CMAKE_ASM_NASM_OBJECT_FORMAT "elf")
	ENABLE_LANGUAGE (ASM_NASM)
    
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# TODO: fmod support

###############################################################################
# modules
###############################################################################
option(TORQUE_SFX_VORBIS "Vorbis Sound" ON)
mark_as_advanced(TORQUE_SFX_VORBIS)
option(TORQUE_THEORA "Theora Video Support" ON)
mark_as_advanced(TORQUE_THEORA)
option(TORQUE_ADVANCED_LIGHTING "Advanced Lighting" ON)
mark_as_advanced(TORQUE_ADVANCED_LIGHTING)
option(TORQUE_BASIC_LIGHTING "Basic Lighting" ON)
mark_as_advanced(TORQUE_BASIC_LIGHTING)
if(WIN32)
	option(TORQUE_SFX_DirectX "DirectX Sound" ON)
	mark_as_advanced(TORQUE_SFX_DirectX)
else()
	set(TORQUE_SFX_DirectX OFF)
endif()
option(TORQUE_SFX_OPENAL "OpenAL Sound" ON)
mark_as_advanced(TORQUE_SFX_OPENAL)
option(TORQUE_HIFI "HIFI? support" OFF)
mark_as_advanced(TORQUE_HIFI)
option(TORQUE_EXTENDED_MOVE "Extended move support" OFF)
mark_as_advanced(TORQUE_EXTENDED_MOVE)
if(WIN32)
	option(TORQUE_SDL "Use SDL for window and input" OFF)
else()
	set(TORQUE_SDL ON) # we need sdl to work on Linux/Mac
endif()
if(WIN32)
	option(TORQUE_OPENGL "Allow OpenGL render" OFF)
	#mark_as_advanced(TORQUE_OPENGL)
else()
	set(TORQUE_OPENGL ON) # we need OpenGL to render on Linux/Mac
endif()

if(WIN32)
	option(TORQUE_OPENGL "Allow OpenGL render" OFF)
	#mark_as_advanced(TORQUE_OPENGL)
else()
	set(TORQUE_OPENGL ON) # we need OpenGL to render on Linux/Mac
	option(TORQUE_DEDICATED "Torque dedicated" OFF)
endif()

if(WIN32)
	option(TORQUE_D3D11 "Allow Direct3D 11 render" OFF)
endif()

option(TORQUE_EXPERIMENTAL_EC "Experimental Entity/Component systems" OFF)
mark_as_advanced(TORQUE_EXPERIMENTAL_EC)

###############################################################################
# options
###############################################################################
if(NOT MSVC) # handle single-configuration generator
    set(TORQUE_BUILD_TYPE "Debug" CACHE STRING "Select one of Debug, Release and RelWithDebInfo")
    set_property(CACHE TORQUE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")
    
    set(TORQUE_ADDITIONAL_LINKER_FLAGS "" CACHE STRING "Additional linker flags")
    mark_as_advanced(TORQUE_ADDITIONAL_LINKER_FLAGS)
endif()

option(TORQUE_MULTITHREAD "Multi Threading" ON)
mark_as_advanced(TORQUE_MULTITHREAD)

option(TORQUE_DISABLE_MEMORY_MANAGER "Disable memory manager" ON)
mark_as_advanced(TORQUE_DISABLE_MEMORY_MANAGER)

option(TORQUE_DISABLE_VIRTUAL_MOUNT_SYSTEM "Disable virtual mount system" OFF)
mark_as_advanced(TORQUE_DISABLE_VIRTUAL_MOUNT_SYSTEM)

option(TORQUE_PLAYER "Playback only?" OFF)
mark_as_advanced(TORQUE_PLAYER)

option(TORQUE_TOOLS "Enable or disable the tools" ON)
mark_as_advanced(TORQUE_TOOLS)

option(TORQUE_ENABLE_PROFILER "Enable or disable the profiler" OFF)
mark_as_advanced(TORQUE_ENABLE_PROFILER)

option(TORQUE_DEBUG "T3D Debug mode" OFF)
mark_as_advanced(TORQUE_DEBUG)

option(TORQUE_SHIPPING "T3D Shipping build?" OFF)
mark_as_advanced(TORQUE_SHIPPING)

option(TORQUE_DEBUG_NET "debug network" OFF)
mark_as_advanced(TORQUE_DEBUG_NET)

option(TORQUE_DEBUG_NET_MOVES "debug network moves" OFF)
mark_as_advanced(TORQUE_DEBUG_NET_MOVES)

option(TORQUE_ENABLE_ASSERTS "enables or disable asserts" OFF)
mark_as_advanced(TORQUE_ENABLE_ASSERTS)

option(TORQUE_DEBUG_GFX_MODE "triggers graphics debug mode" OFF)
mark_as_advanced(TORQUE_DEBUG_GFX_MODE)

#option(DEBUG_SPEW "more debug" OFF)
set(TORQUE_NO_DSO_GENERATION ON)

if(WIN32)
    # warning C4800: 'XXX' : forcing value to bool 'true' or 'false' (performance warning)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4800")
    # warning C4018: '<' : signed/unsigned mismatch
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4018")
    # warning C4244: 'initializing' : conversion from 'XXX' to 'XXX', possible loss of data
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4244")

    if( TORQUE_CPU_X64 )
        link_directories($ENV{DXSDK_DIR}/Lib/x64)
    else()
        link_directories($ENV{DXSDK_DIR}/Lib/x86)
    endif()
endif()

# build types
if(NOT MSVC) # handle single-configuration generator
	set(CMAKE_BUILD_TYPE ${TORQUE_BUILD_TYPE})
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(TORQUE_DEBUG TRUE)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(TORQUE_RELEASE TRUE)
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        set(TORQUE_RELEASE TRUE)
    else()
		message(FATAL_ERROR "Please select Debug, Release or RelWithDebInfo for TORQUE_BUILD_TYPE")
	endif()
endif()

###############################################################################
# Always enabled paths first
###############################################################################
addPath("${srcDir}/") # must come first :)
addPathRec("${srcDir}/app")
addPath("${srcDir}/sfx/media")
addPath("${srcDir}/sfx/null")
addPath("${srcDir}/sfx")
addPath("${srcDir}/console")
addPath("${srcDir}/core")
addPath("${srcDir}/core/stream")
addPath("${srcDir}/core/strings")
addPath("${srcDir}/core/util")
addPath("${srcDir}/core/util/test")
addPath("${srcDir}/core/util/journal")
addPath("${srcDir}/core/util/journal/test")
addPath("${srcDir}/core/util/zip")
addPath("${srcDir}/core/util/zip/test")
addPath("${srcDir}/core/util/zip/compressors")
addPath("${srcDir}/i18n")
addPath("${srcDir}/sim")
addPath("${srcDir}/util")
addPath("${srcDir}/windowManager")
addPath("${srcDir}/windowManager/torque")
addPath("${srcDir}/windowManager/test")
addPath("${srcDir}/math")
addPath("${srcDir}/math/util")
addPath("${srcDir}/math/test")

addPath("${srcDir}/platform")
if(NOT TORQUE_SDL) 
   set(BLACKLIST "fileDialog.cpp" )
endif()
addPath("${srcDir}/platform/nativeDialogs")
set(BLACKLIST "" )

addPath("${srcDir}/cinterface")

if( NOT TORQUE_DEDICATED )
    addPath("${srcDir}/platform/menus")
endif()
addPath("${srcDir}/platform/test")
addPath("${srcDir}/platform/threads")
addPath("${srcDir}/platform/threads/test")
addPath("${srcDir}/platform/async")
addPath("${srcDir}/platform/async/test")
addPath("${srcDir}/platform/input")
addPath("${srcDir}/platform/output")
addPath("${srcDir}/app")
addPath("${srcDir}/app/net")
addPath("${srcDir}/util/messaging")
addPath("${srcDir}/gfx/Null")
addPath("${srcDir}/gfx/test")
addPath("${srcDir}/gfx/bitmap")
addPath("${srcDir}/gfx/bitmap/loaders")
addPath("${srcDir}/gfx/util")
addPath("${srcDir}/gfx/video")
addPath("${srcDir}/gfx")
addPath("${srcDir}/shaderGen")
addPath("${srcDir}/gfx/sim")
addPath("${srcDir}/gui/buttons")
addPath("${srcDir}/gui/containers")
addPath("${srcDir}/gui/controls")
addPath("${srcDir}/gui/core")
addPath("${srcDir}/gui/game")
addPath("${srcDir}/gui/shiny")
addPath("${srcDir}/gui/utility")
addPath("${srcDir}/gui")
addPath("${srcDir}/collision")
addPath("${srcDir}/materials")
addPath("${srcDir}/lighting")
addPath("${srcDir}/lighting/common")
addPath("${srcDir}/renderInstance")
addPath("${srcDir}/scene")
addPath("${srcDir}/scene/culling")
addPath("${srcDir}/scene/zones")
addPath("${srcDir}/scene/mixin")
addPath("${srcDir}/shaderGen")
addPath("${srcDir}/terrain")
addPath("${srcDir}/environment")
addPath("${srcDir}/forest")
addPath("${srcDir}/forest/ts")
addPath("${srcDir}/ts")
addPath("${srcDir}/ts/arch")
addPath("${srcDir}/physics")
addPath("${srcDir}/gui/3d")
addPath("${srcDir}/postFx")

if(NOT TORQUE_EXPERIMENTAL_EC) 
   set(BLACKLIST "entity.cpp;entity.h" )
endif()
addPath("${srcDir}/T3D")
set(BLACKLIST "" )

addPath("${srcDir}/T3D/examples")
addPath("${srcDir}/T3D/fps")
addPath("${srcDir}/T3D/fx")
addPath("${srcDir}/T3D/vehicles")
addPath("${srcDir}/T3D/physics")
addPath("${srcDir}/T3D/decal")
addPath("${srcDir}/T3D/sfx")
addPath("${srcDir}/T3D/gameBase")
addPath("${srcDir}/T3D/turret")

if( TORQUE_EXPERIMENTAL_EC )
	addPath("${srcDir}/T3D/components/")
	addPath("${srcDir}/T3D/components/animation")
	addPath("${srcDir}/T3D/components/camera")
	addPath("${srcDir}/T3D/components/collision")
	addPath("${srcDir}/T3D/components/game")
	addPath("${srcDir}/T3D/components/physics")
	addPath("${srcDir}/T3D/components/render")
endif()

addPath("${srcDir}/main/")
addPath("${srcDir}/assets")
addPath("${srcDir}/module")
addPath("${srcDir}/T3D/assets")
addPathRec("${srcDir}/persistence")
addPathRec("${srcDir}/ts/collada")
addPathRec("${srcDir}/ts/loader")
addPathRec("${projectSrcDir}")

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
	if(TORQUE_OPENGL)
		addPathRec("${srcDir}/lighting/advanced/glsl")
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

# Vorbis
if(TORQUE_SFX_VORBIS)
    addInclude(${libDir}/libvorbis/include)
    addDef(TORQUE_OGGVORBIS)
    addLib(libvorbis)
    addLib(libogg)
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
    if(NOT TORQUE_EXPERIMENTAL_EC) 
        set(BLACKLIST "entityGroup.cpp;entityGroup.h;mountingGroup.cpp;mountingGroup.h;componentGroup.cpp;componentGroup.h" )
    endif()
    addPath("${srcDir}/gui/editor/inspector")
    set(BLACKLIST "" )
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

if(TORQUE_SDL)
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

       find_package(PkgConfig REQUIRED)
       pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

       # Setup CMake to use GTK+, tell the compiler where to look for headers
       # and to the linker where to look for libraries
       include_directories(${GTK3_INCLUDE_DIRS})
       link_directories(${GTK3_LIBRARY_DIRS})

       # Add other flags to the compiler
       add_definitions(${GTK3_CFLAGS_OTHER})

       set(BLACKLIST "nfd_win.cpp"  )
       addLib(nativeFileDialogs)

       set(BLACKLIST ""  )
       target_link_libraries(nativeFileDialogs ${GTK3_LIBRARIES})
 	else()
 	   set(BLACKLIST "nfd_gtk.c" )
 	   addLib(nativeFileDialogs)
       set(BLACKLIST ""  )
 	   addLib(comctl32)	   
    endif()
    
    #override and hide SDL2 cache variables
    set(SDL_SHARED ON CACHE INTERNAL "" FORCE)
    set(SDL_STATIC OFF CACHE INTERNAL "" FORCE)
    add_subdirectory( ${libDir}/sdl ${CMAKE_CURRENT_BINARY_DIR}/sdl2)
endif()

if(TORQUE_DEDICATED)
    addDef(TORQUE_DEDICATED)
endif()

if(TORQUE_EXPERIMENTAL_EC)
	addDef(TORQUE_EXPERIMENTAL_EC)
endif()

#modules dir
file(GLOB modules "modules/*.cmake")
foreach(module ${modules})
	include(${module})
endforeach()

###############################################################################
# platform specific things
###############################################################################
if(WIN32)
    addPath("${srcDir}/platformWin32")
    if(TORQUE_SDL) 
 		set(BLACKLIST "fileDialog.cpp" )
 	endif()
    addPath("${srcDir}/platformWin32/nativeDialogs")
    set(BLACKLIST "" )
    addPath("${srcDir}/platformWin32/menus")
    addPath("${srcDir}/platformWin32/threads")
    addPath("${srcDir}/platformWin32/videoInfo")
    addPath("${srcDir}/platformWin32/minidump")
    addPath("${srcDir}/windowManager/win32")
	if(TORQUE_D3D11)
		addPath("${srcDir}/gfx/D3D11")
	endif()
    addPath("${srcDir}/gfx/D3D9")
    addPath("${srcDir}/gfx/D3D9/pc")
    addPath("${srcDir}/shaderGen/HLSL")    
    addPath("${srcDir}/terrain/hlsl")
    addPath("${srcDir}/forest/hlsl")
    # add windows rc file for the icon
    addFile("${projectSrcDir}/torque.rc")
endif()

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

if(UNIX)
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
    # linux
    addPath("${srcDir}/platformX86UNIX/threads")
    addPath("${srcDir}/platformPOSIX")
endif()

if( TORQUE_OPENGL )
    addPath("${srcDir}/shaderGen/GLSL")
    if( TORQUE_OPENGL AND NOT TORQUE_DEDICATED )
        addPath("${srcDir}/gfx/gl")
        addPath("${srcDir}/gfx/gl/tGL")        
        addPath("${srcDir}/shaderGen/GLSL")
        addPath("${srcDir}/terrain/glsl")
        addPath("${srcDir}/forest/glsl")    
    endif()
    
    if(WIN32 AND NOT TORQUE_SDL)
      addPath("${srcDir}/gfx/gl/win32")
    endif()
endif()

###############################################################################
###############################################################################
finishExecutable()
###############################################################################
###############################################################################

# Set Visual Studio startup project
if((${CMAKE_VERSION} VERSION_EQUAL 3.6.0) OR (${CMAKE_VERSION} VERSION_GREATER 3.6.0) AND MSVC)
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${TORQUE_APP_NAME})
endif()

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
if(WIN32)
    if(NOT EXISTS "${projectSrcDir}/torque.rc")
        CONFIGURE_FILE("${cmakeDir}/torque-win.rc.in" "${projectSrcDir}/torque.rc")
    endif()
    if(NOT EXISTS "${projectOutDir}/${PROJECT_NAME}-debug.bat")
        CONFIGURE_FILE("${cmakeDir}/app-debug-win.bat.in" "${projectOutDir}/${PROJECT_NAME}-debug.bat")
    endif()
    if(NOT EXISTS "${projectOutDir}/cleanup.bat")
        CONFIGURE_FILE("${cmakeDir}/cleanup-win.bat.in" "${projectOutDir}/cleanup.bat")
    endif()
endif()

###############################################################################
# Common Libraries
###############################################################################
addLib(lmng)
addLib(lpng)
addLib(lungif)
addLib(ljpeg)
addLib(zlib)
addLib(tinyxml)
addLib(opcode)
addLib(squish)
addLib(collada)
addLib(pcre)
addLib(convexDecomp)
if (TORQUE_OPENGL)
   addLib(epoxy)
endif()

if(WIN32)
    # copy pasted from T3D build system, some might not be needed
    set(TORQUE_EXTERNAL_LIBS "COMCTL32.LIB;COMDLG32.LIB;USER32.LIB;ADVAPI32.LIB;GDI32.LIB;WINMM.LIB;WSOCK32.LIB;vfw32.lib;Imm32.lib;d3d9.lib;d3dx9.lib;DxErr.lib;ole32.lib;shell32.lib;oleaut32.lib;version.lib" CACHE STRING "external libs to link against")
    mark_as_advanced(TORQUE_EXTERNAL_LIBS)
    addLib("${TORQUE_EXTERNAL_LIBS}")
   
   if(TORQUE_OPENGL)
      addLib(OpenGL32.lib)
   endif()
		
   # JTH: DXSDK is compiled with older runtime, and MSVC 2015+ is when __vsnprintf is undefined.
   # This is a workaround by linking with the older legacy library functions.
   # See this for more info: http://stackoverflow.com/a/34230122
   if (MSVC14)
      addLib(legacy_stdio_definitions.lib)
   endif()
endif()

if(UNIX)
    # copy pasted from T3D build system, some might not be needed
	set(TORQUE_EXTERNAL_LIBS "dl Xxf86vm Xext X11 Xft stdc++ pthread GL" CACHE STRING "external libs to link against")
	mark_as_advanced(TORQUE_EXTERNAL_LIBS)
    
    string(REPLACE " " ";" TORQUE_EXTERNAL_LIBS_LIST ${TORQUE_EXTERNAL_LIBS})
    addLib( "${TORQUE_EXTERNAL_LIBS_LIST}" )
endif()

if(UNIX)
    # copy pasted from T3D build system, some might not be needed
	set(TORQUE_EXTERNAL_LIBS "rt dl Xxf86vm Xext X11 Xft stdc++ pthread GL" CACHE STRING "external libs to link against")
	mark_as_advanced(TORQUE_EXTERNAL_LIBS)
    
    string(REPLACE " " ";" TORQUE_EXTERNAL_LIBS_LIST ${TORQUE_EXTERNAL_LIBS})
    addLib( "${TORQUE_EXTERNAL_LIBS_LIST}" )
endif()

###############################################################################
# Always enabled Definitions
###############################################################################
addDef(TORQUE_DEBUG Debug)
addDef(TORQUE_ENABLE_ASSERTS "Debug;RelWithDebInfo")
addDef(TORQUE_DEBUG_GFX_MODE "RelWithDebInfo")
addDef(TORQUE_SHADERGEN)
addDef(INITGUID)
addDef(NTORQUE_SHARED)
addDef(UNICODE)
addDef(_UNICODE) # for VS
addDef(TORQUE_UNICODE)
#addDef(TORQUE_SHARED) # not used anymore as the game is the executable directly
addDef(LTC_NO_PROTOTYPES) # for libTomCrypt
addDef(BAN_OPCODE_AUTOLINK)
addDef(ICE_NO_DLL)
addDef(TORQUE_OPCODE)
addDef(TORQUE_COLLADA)
addDef(DOM_INCLUDE_TINYXML)
addDef(PCRE_STATIC)
addDef(_CRT_SECURE_NO_WARNINGS)
addDef(_CRT_SECURE_NO_DEPRECATE)

if(UNIX)
	addDef(LINUX)	
endif()

if(TORQUE_OPENGL)
	addDef(TORQUE_OPENGL)
endif()

if(TORQUE_SDL)
    addDef(TORQUE_SDL)
    addInclude(${libDir}/sdl/include)
    addLib(SDL2)
endif()

if(TORQUE_STATIC_CODE_ANALYSIS)
    addDef( "ON_FAIL_ASSERTFATAL=exit(1)" )
endif()

###############################################################################
# Include Paths
###############################################################################
addInclude("${projectSrcDir}")
addInclude("${srcDir}/")
addInclude("${libDir}/lmpg")
addInclude("${libDir}/lpng")
addInclude("${libDir}/ljpeg")
addInclude("${libDir}/lungif")
addInclude("${libDir}/zlib")
addInclude("${libDir}/") # for tinyxml
addInclude("${libDir}/tinyxml")
addInclude("${libDir}/squish")
addInclude("${libDir}/convexDecomp")
addInclude("${libDir}/libogg/include")
addInclude("${libDir}/opcode")
addInclude("${libDir}/collada/include")
addInclude("${libDir}/collada/include/1.4")
if(TORQUE_SDL)
   addInclude("${libDir}/nativeFileDialogs/include")
endif()
if(TORQUE_OPENGL)
	addInclude("${libDir}/epoxy/include")
	addInclude("${libDir}/epoxy/src")
endif()

if(UNIX)
	addInclude("/usr/include/freetype2/freetype")
	addInclude("/usr/include/freetype2")
endif()

# external things
if(WIN32)
    set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY INCLUDE_DIRECTORIES $ENV{DXSDK_DIR}/Include)
endif()

if(UNIX)
	addInclude("/usr/include/freetype2/freetype")
	addInclude("/usr/include/freetype2")
endif()

if(MSVC)
    # Match projectGenerator naming for executables
    set(OUTPUT_CONFIG DEBUG MINSIZEREL RELWITHDEBINFO)
    set(OUTPUT_SUFFIX DEBUG MINSIZE    OPTIMIZEDDEBUG)
    foreach(INDEX RANGE 2)
        list(GET OUTPUT_CONFIG ${INDEX} CONF)
        list(GET OUTPUT_SUFFIX ${INDEX} SUFFIX)
        set_property(TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME_${CONF} ${PROJECT_NAME}_${SUFFIX})
    endforeach()
endif()

###############################################################################
# Installation
###############################################################################

if(TORQUE_TEMPLATE)
    message("Prepare Template(${TORQUE_TEMPLATE}) install...")
    file(GLOB_RECURSE INSTALL_FILES_AND_DIRS "${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/game/*")
    
    IF( NOT TORQUE_EXPERIMENTAL_EC)
        list(REMOVE_ITEM INSTALL_FILES_AND_DIRS "${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/game/art/art.module.taml")
        list(REMOVE_ITEM INSTALL_FILES_AND_DIRS "${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/game/art/shapes/actors/Soldier/soldier.asset.taml")
        list(REMOVE_ITEM INSTALL_FILES_AND_DIRS "${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/game/scripts/scripts.module.taml")
        
        foreach(ITEM ${INSTALL_FILES_AND_DIRS})
            get_filename_component( dir ${ITEM} DIRECTORY )
            get_filename_component( fileName ${ITEM} NAME )
            if( ${dir} STREQUAL ${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/game/scripts/server/components 
                OR ${dir} STREQUAL ${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/game/scripts/server/components/game
                OR ${dir} STREQUAL ${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/game/scripts/server/components/input
                OR ${dir} STREQUAL ${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/game/scripts/server/gameObjects
                OR ${dir} STREQUAL ${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/game/tools/componentEditor
                OR ${dir} STREQUAL ${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/game/tools/componentEditor/gui
                OR ${dir} STREQUAL ${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/game/tools/componentEditor/scripts )
                list(REMOVE_ITEM INSTALL_FILES_AND_DIRS ${dir}/${fileName})
            ENDIF()
        endforeach()
    ENDIF()
    
    foreach(ITEM ${INSTALL_FILES_AND_DIRS})
        get_filename_component( dir ${ITEM} DIRECTORY )
        STRING(REGEX REPLACE "${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/" "${TORQUE_APP_DIR}/" INSTALL_DIR ${dir})
        install( FILES ${ITEM} DESTINATION ${INSTALL_DIR} )
    endforeach()
    
    if(WIN32)
        INSTALL(FILES "${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/cleanShaders.bat"     DESTINATION "${TORQUE_APP_DIR}")
        INSTALL(FILES "${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/DeleteCachedDTSs.bat" DESTINATION "${TORQUE_APP_DIR}")
        INSTALL(FILES "${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/DeleteDSOs.bat"       DESTINATION "${TORQUE_APP_DIR}")
        INSTALL(FILES "${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/DeletePrefs.bat"      DESTINATION "${TORQUE_APP_DIR}")
    endif()
endif()
