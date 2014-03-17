cmake_minimum_required (VERSION 2.6)

# TODO: fmod support

#project(torque3d)

# Prefs
add_definitions(-DTORQUE_SHADERGEN)
add_definitions(-DINITGUID)
add_definitions(-DNTORQUE_SHARED)

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE) # for VS
add_definitions(-DTORQUE_UNICODE)
#add_definitions(-DTORQUE_SHARED)

# for libTomCrypt
add_definitions(-DLTC_NO_PROTOTYPES)


add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D_CRT_SECURE_NO_DEPRECATE)

# warning C4800: 'XXX' : forcing value to bool 'true' or 'false' (performance warning)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4800")

# warning C4018: '<' : signed/unsigned mismatch
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4018")

# warning C4244: 'initializing' : conversion from 'XXX' to 'XXX', possible loss of data
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4244")

include_directories(.)
include_directories(main) # temporary for now
include_directories(${Torque3DEngine_SOURCE_DIR}/lib/lmpg)
include_directories(${Torque3DEngine_SOURCE_DIR}/lib/lpng)
include_directories(${Torque3DEngine_SOURCE_DIR}/lib/ljpeg)
include_directories(${Torque3DEngine_SOURCE_DIR}/lib/lungif)
include_directories(${Torque3DEngine_SOURCE_DIR}/lib/zlib)
include_directories(${Torque3DEngine_SOURCE_DIR}/lib/) # for tinyxml
include_directories(${Torque3DEngine_SOURCE_DIR}/lib/tinyxml)
include_directories(${Torque3DEngine_SOURCE_DIR}/lib/squish)
include_directories(${Torque3DEngine_SOURCE_DIR}/lib/convexDecomp)
include_directories(${Torque3DEngine_SOURCE_DIR}/lib/libogg/include)

set(t3dLibs "")
set(t3dLibDirs "")

# external things
include_directories($ENV{DXSDK_DIR}/Include)
set(t3dLibDirs "${t3dLibs};$ENV{DXSDK_DIR}/Lib/x86")

# module helper

set(t "")
addLibPath(t "/app" GLOB_RECURSE)

# lighting
add_definitions(-DTORQUE_ADVANCED_LIGHTING)
addLibPath(t "/lighting/advanced" GLOB)
addLibPath(t "/lighting/shadowMap" GLOB_RECURSE)
addLibPath(t "/lighting/advanced/hlsl" GLOB_RECURSE)
#addLibPath(t "/lighting/advanced/glsl" GLOB_RECURSE)
add_definitions(-DTORQUE_BASIC_LIGHTING)
addLibPath(t "/lighting/basic" GLOB_RECURSE)

# opcode
include_directories(${Torque3DEngine_SOURCE_DIR}/lib/opcode)
add_definitions(-DBAN_OPCODE_AUTOLINK)
add_definitions(-DICE_NO_DLL)
add_definitions(-DTORQUE_OPCODE)


# collada
add_definitions(-DTORQUE_COLLADA)
add_definitions(-DDOM_INCLUDE_TINYXML)
add_definitions(-DPCRE_STATIC)
addLibPath(t "/ts/collada" GLOB_RECURSE)
addLibPath(t "/ts/loader" GLOB_RECURSE)
include_directories(${Torque3DEngine_SOURCE_DIR}/lib/collada/include)
include_directories(${Torque3DEngine_SOURCE_DIR}/lib/collada/include/1.4)

# DirectX Sound
addLibPath(t "/sfx/dsound" GLOB_RECURSE)
addLibPath(t "/sfx/xaudio" GLOB_RECURSE)
set(t3dLibs "${t3dLibs};x3daudio.lib")

# OpenAL
addLibPath(t "/sfx/openal" GLOB)
#addLibPath(t "/sfx/openal/mac" GLOB)
addLibPath(t "/sfx/openal/win32" GLOB)
include_directories(${Torque3DEngine_SOURCE_DIR}/lib/openal/win32)

# vorbis
add_definitions(-DTORQUE_OGGVORBIS)
include_directories(${Torque3DEngine_SOURCE_DIR}/lib/libvorbis/include)

# Theora
add_definitions(-DTORQUE_OGGTHEORA)
add_definitions(-DTORQUE_OGGVORIBS)
addLibPath(t "/core/ogg" GLOB)
addLibPath(t "/gfx/video" GLOB)
addLibPath(t "/gui/theora" GLOB)
include_directories(${Torque3DEngine_SOURCE_DIR}/lib/libtheora/include)

# Tools
addLibPath(t "/gui/worldEditor" GLOB)
addLibPath(t "/environment/editors" GLOB)
addLibPath(t "/forest/editor" GLOB)
addLibPath(t "/gui/editor" GLOB)
addLibPath(t "/gui/editor/inspector" GLOB)


# Always on things
addLibPath(t "/sfx/media" GLOB)
addLibPath(t "/sfx/null" GLOB)
addLibPath(t "/sfx" GLOB)

# Components
addLibPath(t "/component" GLOB)
addLibPath(t "/component/interfaces" GLOB)

   
addLibPath(t "/console" GLOB)
addLibPath(t "/core" GLOB)
addLibPath(t "/core/stream" GLOB)
addLibPath(t "/core/strings" GLOB)
addLibPath(t "/core/util" GLOB)
addLibPath(t "/core/util/test" GLOB)
addLibPath(t "/core/util/journal" GLOB)
addLibPath(t "/core/util/journal/test" GLOB)
addLibPath(t "/core/util/zip" GLOB)
addLibPath(t "/core/util/zip/unitTests" GLOB)
addLibPath(t "/core/util/zip/compressors" GLOB)
addLibPath(t "/i18n" GLOB)
addLibPath(t "/sim" GLOB)
addLibPath(t "/unit/tests" GLOB)
addLibPath(t "/unit" GLOB)
addLibPath(t "/util" GLOB)
addLibPath(t "/windowManager" GLOB)
addLibPath(t "/windowManager/torque" GLOB)
addLibPath(t "/windowManager/test" GLOB)
addLibPath(t "/math" GLOB)
addLibPath(t "/math/util" GLOB)
addLibPath(t "/math/test" GLOB)
addLibPath(t "/platform" GLOB)
addLibPath(t "/cinterface" GLOB)

addLibPath(t "/platform/nativeDialogs" GLOB)
addLibPath(t "/platform/menus" GLOB)
addLibPath(t "/platform/test" GLOB)

addLibPath(t "/platform/threads" GLOB)
addLibPath(t "/platform/async" GLOB)
addLibPath(t "/platform/input" GLOB)
addLibPath(t "/platform/output" GLOB)
addLibPath(t "/app" GLOB)
addLibPath(t "/app/net" GLOB)

addLibPath(t "/util/messaging" GLOB)

# win32
addLibPath(t "/platformWin32" GLOB)
addLibPath(t "/platformWin32/nativeDialogs" GLOB)
addLibPath(t "/platformWin32/menus" GLOB)
addLibPath(t "/platformWin32/threads" GLOB)
addLibPath(t "/platformWin32/videoInfo" GLOB)
addLibPath(t "/platformWin32/minidump" GLOB)
addLibPath(t "/windowManager/win32" GLOB)
     
# mac
#addLibPath(t "/platformMac" GLOB)
#addLibPath(t "/platformMac/menus" GLOB)
#addLibPath(t "/platformPOSIX" GLOB)
#addLibPath(t "/windowManager/mac" GLOB)

# xbox360
#addLibPath(t "/platformXbox" GLOB)
#addLibPath(t "/platformXbox/threads" GLOB)
#addLibPath(t "/windowManager/360" GLOB)

# ps3
#addLibPath(t "/platformPS3" GLOB)
#addLibPath(t "/platformPS3/threads" GLOB)
#addLibPath(t "/windowManager/ps3" GLOB)

# linux_dedicated
#addLibPath(t "/windowManager/dedicated" GLOB)

# linux
#addLibPath(t "/platformX86UNIX" GLOB)
#addLibPath(t "/platformX86UNIX/threads" GLOB)
#addLibPath(t "/platformPOSIX" GLOB)

# GFX
addLibPath(t "/gfx/Null" GLOB)
addLibPath(t "/gfx/test" GLOB)
addLibPath(t "/gfx/bitmap" GLOB)
addLibPath(t "/gfx/bitmap/loaders" GLOB)
addLibPath(t "/gfx/util" GLOB)
addLibPath(t "/gfx/video" GLOB)
addLibPath(t "/gfx" GLOB)
addLibPath(t "/shaderGen" GLOB)

# win32
#addLibPath(t "/gfx/D3D8" GLOB)
addLibPath(t "/gfx/D3D" GLOB)
addLibPath(t "/gfx/D3D9" GLOB)
addLibPath(t "/gfx/D3D9/pc" GLOB)
addLibPath(t "/shaderGen/HLSL" GLOB)

# xbox360
#addLibPath(t "/gfx/D3D9" GLOB)
#addLibPath(t "/gfx/D3D9/360" GLOB)
#addLibPath(t "/shaderGen/HLSL" GLOB)
#addLibPath(t "/shaderGen/360" GLOB)

# mac
#addLibPath(t "/gfx/gl/ggl/mac" GLOB)

# GFX - GGL
#addLibPath(t "/gfx/gl" GLOB)
#addLibPath(t "/gfx/gl/ggl" GLOB)
#addLibPath(t "/gfx/gl/ggl/generated" GLOB)
#addLibPath(t "/shaderGen/GLSL" GLOB)

# linux 
# linux_dedicated":
#addLibPath(t "/gfx/gl/ggl/x11" GLOB) // This one is not yet implemented!
# GFX - GGL
#addLibPath(t "/gfx/gl" GLOB)
#addLibPath(t "/gfx/gl/ggl" GLOB)
#addLibPath(t "/gfx/gl/ggl/generated" GLOB)
#addLibPath(t "/shaderGen/GLSL" GLOB)

# ps3
#addLibPath(t "/gfx/gl/ggl/ps3" GLOB)

# GFX - GGL
#addLibPath(t "/gfx/gl" GLOB)
#addLibPath(t "/gfx/gl/ggl" GLOB)
#addLibPath(t "/gfx/gl/ggl/generated" GLOB)
#addLibPath(t "/shaderGen/GLSL" GLOB)

# GFX - Sim dependent 
addLibPath(t "/gfx/sim" GLOB)

# GUI
addLibPath(t "/gui/buttons" GLOB)
addLibPath(t "/gui/containers" GLOB)
addLibPath(t "/gui/controls" GLOB)

addLibPath(t "/gui/core" GLOB)
addLibPath(t "/gui/game" GLOB)
addLibPath(t "/gui/shiny" GLOB)
addLibPath(t "/gui/utility" GLOB)
addLibPath(t "/gui" GLOB)

# Include tools for non-tool builds (or define player if a tool build)
addLibPath(t "/gui/worldEditor" GLOB)
addLibPath(t "/environment/editors" GLOB)
addLibPath(t "/forest/editor" GLOB)
addLibPath(t "/gui/editor" GLOB)
addLibPath(t "/gui/editor/inspector" GLOB)

# T3D
# 3D
addLibPath(t "/collision" GLOB)
addLibPath(t "/materials" GLOB)
addLibPath(t "/lighting" GLOB)
addLibPath(t "/lighting/common" GLOB)
addLibPath(t "/renderInstance" GLOB)
addLibPath(t "/scene" GLOB)
addLibPath(t "/scene/culling" GLOB)
addLibPath(t "/scene/zones" GLOB)
addLibPath(t "/scene/mixin" GLOB)
addLibPath(t "/shaderGen" GLOB)
addLibPath(t "/terrain" GLOB)
addLibPath(t "/environment" GLOB)

addLibPath(t "/forest" GLOB)
addLibPath(t "/forest/ts" GLOB)

addLibPath(t "/ts" GLOB)
addLibPath(t "/ts/arch" GLOB)
addLibPath(t "/physics" GLOB)
addLibPath(t "/gui/3d" GLOB)
addLibPath(t "/postFx" GLOB)

# 3D game
addLibPath(t "/T3D" GLOB)
addLibPath(t "/T3D/examples" GLOB)
addLibPath(t "/T3D/fps" GLOB)
addLibPath(t "/T3D/fx" GLOB)
addLibPath(t "/T3D/vehicles" GLOB)
addLibPath(t "/T3D/physics" GLOB)
addLibPath(t "/T3D/decal" GLOB)
addLibPath(t "/T3D/sfx" GLOB)
addLibPath(t "/T3D/gameBase" GLOB)
addLibPath(t "/T3D/turret" GLOB)


# HIFI NET
#add_definitions(-DTORQUE_HIFI_NET)
#addLibPath(t "/T3D/gameBase/hifi" GLOB)

# extended move
#add_definitions(-DTORQUE_EXTENDED_MOVE)
#addLibPath(t "/T3D/gameBase/extended" GLOB)
addLibPath(t "/T3D/gameBase/std" GLOB)

# Platform specific stuff.
# xbox 360
#addLibPath(t "/ts/arch/360" GLOB)
# Fall through
# win32
#addLibPath(t "/terrain/hlsl" GLOB)
#addLibPath(t "/forest/hlsl" GLOB)

# ps3
#addLibPath(t "/ts/arch/ps3" GLOB)
# Fall through

# mac
# linux
# linux_dedicated
#addLibPath(t "/ts/arch/ps3" GLOB)
#addLibPath(t "/terrain/glsl" GLOB)
#addLibPath(t "/forest/glsl" GLOB)


addLibraryFinal("${t}" "libtorque3d")
set(t3dLibs "${t3dLibs};libtorque3d")
link_directories(${t3dLibDirs})

# copy pasted from T3D build system, some might not be needed
set(t3dLibs "${t3dLibs};COMCTL32.LIB;COMDLG32.LIB;USER32.LIB;ADVAPI32.LIB;GDI32.LIB;WINMM.LIB;WSOCK32.LIB;vfw32.lib;Imm32.lib;d3d9.lib;d3dx9.lib;DxErr.lib;ole32.lib;shell32.lib;oleaut32.lib;version.lib")

# internal libs
set(t3dLibs "${t3dLibs};mng;png;ungif;jpeg;zlib;tinyxml;opcode;squish;collada;pcre;convexDecomp;vorbis;ogg;theora")
		  
add_executable(torque3d main/main.cpp main/Torque.rc)

target_link_libraries(torque3d ${t3dLibs})

